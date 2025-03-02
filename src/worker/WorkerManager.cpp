#include "WorkerManager.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

// ========================================================
// WorkerManager Implementation
// ========================================================

namespace WorkerManager {
WorkerManager::WorkerManager(int workerCount)
	: running_(false)
{
	workers_.resize(workerCount);
	for (int i = 0; i < workerCount; i++) {
		workers_[i].index = i;
		workers_[i].state = WORKER_IDLE;
		// Initialisation de l'item par défaut.
		workers_[i].item.p = 0;
		workers_[i].item.q = 0;
		workers_[i].item.load = 0;
		workers_[i].item.blockMap = nullptr;
		workers_[i].item.lightMap = nullptr;
		workers_[i].item.miny = 256;
		workers_[i].item.maxy = 0;
		workers_[i].item.faces = 0;
		workers_[i].item.data = nullptr;
	}
}

WorkerManager::~WorkerManager() {
	stop();
}

void WorkerManager::start() {
	running_ = true;
	// Pour chaque worker, démarrer le thread en passant config et db par référence.
	// Note : Ici, nous ne possédons pas directement le Config et Database*,
	// ils seront transmis via la méthode update() en appelant workerRun.
	// Nous pouvons démarrer les threads en passant des copies de références.
	// Pour simplifier, nous supposons que les workers recevront ces paramètres à chaque boucle.
	for (auto &worker : workers_) {
		worker.thrd = std::thread(workerRun, &worker, std::ref(*new Config()), nullptr);
		// Le passage de Config et Database* sera utilisé dans update(), mais ici nous lançons le thread.
		// Dans une intégration réelle, vous pourriez stocker ces pointeurs globalement ou les passer autrement.
	}
}

void WorkerManager::stop() {
	running_ = false;
	// Notifier tous les workers pour qu'ils se réveillent et vérifient la condition d'arrêt.
	for (auto &worker : workers_) {
		{
			std::lock_guard<std::mutex> lock(worker.mtx);
			worker.state = WORKER_EXIT;
		}
		worker.cnd.notify_one();
	}
	for (auto &worker : workers_) {
		if (worker.thrd.joinable())
			worker.thrd.join();
	}
}

void WorkerManager::workerRun(Worker* worker, const Config &config, Database::DB* db) {
	// La boucle de chaque worker.
	while (true) {
		std::unique_lock<std::mutex> lock(worker->mtx);
		// Attendre que le worker soit marqué BUSY ou qu'il doive quitter.
		worker->cnd.wait(lock, [&](){ return worker->state == WORKER_BUSY || worker->state == WORKER_EXIT; });
		if (worker->state == WORKER_EXIT) {
			break;
		}
		lock.unlock();
		// Si la tâche demande de charger les données, on les charge.
		if (worker->item.load && worker->item.blockMap && worker->item.lightMap) {
			// Charger le chunk depuis la base de données.
			WorldManager::loadChunk(*worker->item.blockMap, config, db);
		}
		// Calculer la géométrie du chunk.
		WorldManager::computeChunk(worker->item, config);
		// Une fois le travail terminé, passer à l'état DONE.
		lock.lock();
		worker->state = WORKER_DONE;
		lock.unlock();
	}
}

void WorkerManager::checkWorkers(std::vector<Chunk>& chunks, const Config &config, Database::DB* db) {
	for (auto &worker : workers_) {
		std::lock_guard<std::mutex> lock(worker.mtx);
		if (worker.state == WORKER_DONE) {
			// Trouver le chunk correspondant aux indices stockés dans worker->item.
			Chunk* chunk = nullptr;
			for (auto &ch : chunks) {
				if (ch.p == worker.item.p && ch.q == worker.item.q) {
					chunk = &ch;
					break;
				}
			}
			if (chunk) {
				// Générer le buffer OpenGL du chunk à partir des données calculées.
				WorldManager::generateChunk(*chunk, worker.item, config);
			}
			// Libérer la mémoire allouée pour les copies des maps.
			if (worker.item.blockMap) {
				delete worker.item.blockMap;
				worker.item.blockMap = nullptr;
			}
			if (worker.item.lightMap) {
				delete worker.item.lightMap;
				worker.item.lightMap = nullptr;
			}
			worker.state = WORKER_IDLE;
		}
	}
}

void WorkerManager::ensureChunks(InputManager::State* playerState, std::vector<Chunk>& chunks,
								 const Config &config, Database::DB* db) {
	int chunkSize = config.getChunkSize();
	// Détermine la position du joueur en indices de chunk.
	int p = static_cast<int>(std::floor(playerState->x / chunkSize));
	int q = static_cast<int>(std::floor(playerState->z / chunkSize));
	int createRadius = config.getCreateChunkRadius(); // Supposons qu'il existe un getter dans Config.
	
	// Parcourt la zone autour du joueur.
	for (int dp = -createRadius; dp <= createRadius; dp++) {
		for (int dq = -createRadius; dq <= createRadius; dq++) {
			int cp = p + dp;
			int cq = q + dq;
			// Vérifie si le chunk existe, sinon l'initialise.
			Chunk* chunk = findChunk(chunks, cp, cq);
			if (!chunk) {
				Chunk newChunk;
				WorldManager::initChunk(newChunk, cp, cq, config);
				// Charger le chunk initialement.
				WorldManager::loadChunk(newChunk, config, db);
				chunks.push_back(newChunk);
				chunk = &chunks.back();
			}
			// Si le chunk est marqué dirty, on demande sa régénération.
			if (chunk->dirty) {
				// Pour chaque worker idle, on affecte une tâche.
				for (auto &worker : workers_) {
					std::unique_lock<std::mutex> lock(worker.mtx);
					if (worker.state == WORKER_IDLE) {
						worker.item.p = cp;
						worker.item.q = cq;
						worker.item.load = (chunk->map.getSize() == 0) ? 1 : 0;
						// Créer des copies des maps pour la génération.
						worker.item.blockMap = new World::Map(chunk->map);
						worker.item.lightMap = new World::Map(chunk->lights);
						lock.unlock();
						worker.state = WORKER_BUSY;
						worker.cnd.notify_one();
						break; // Affecter un chunk par worker.
					}
				}
			}
		}
	}
}

void WorkerManager::update(InputManager::State* playerState, std::vector<Chunk>& chunks,
							 const Config &config, Database::DB* db) {
	// Vérifier les workers terminés et mettre à jour les chunks correspondants.
	checkWorkers(chunks, config, db);
	// Affecter de nouvelles tâches aux workers idle en fonction de la position du joueur.
	ensureChunks(playerState, chunks, config, db);
}

} // namespace WorkerManager
