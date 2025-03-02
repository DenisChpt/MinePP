#ifndef WORKER_MANAGER_HPP
#define WORKER_MANAGER_HPP

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "WorldManager.hpp"
#include "Config.hpp"
#include "Database.hpp"
#include "InputManager.hpp" // Pour la structure State, utilisée par Player

namespace WorkerManager {

// États possibles pour un worker
enum WorkerState {
	WORKER_IDLE = 0,
	WORKER_BUSY = 1,
	WORKER_DONE = 2,
	WORKER_EXIT = 3
};

// Structure représentant un worker (thread) de génération de chunks.
struct Worker {
	int index;
	int state; // Utilise l'enum WorkerState
	std::thread thrd;
	std::mutex mtx;
	std::condition_variable cnd;
	// WorkerItem tel que défini dans WorldManager.hpp, qui contient :
	//   int p, q; // coordonnées du chunk
	//   WorldManager::WorkerItem members: blockMap, lightMap, miny, maxy, faces, data
	WorldManager::WorkerItem item;
};

class WorkerManager {
public:
	// Constructeur : on passe le nombre de workers souhaité.
	WorkerManager(int workerCount);
	~WorkerManager();

	// Démarre tous les workers.
	void start();
	// Met à jour les workers :
	// - Vérifie les workers terminés et met à jour les chunks correspondants.
	// - Affecte de nouvelles tâches aux workers idle en fonction de la position du joueur.
	void update(InputManager::State* playerState, std::vector<Chunk>& chunks,
				const Config &config, Database::DB* db);
	// Arrête tous les workers.
	void stop();
private:
	std::vector<Worker> workers_;
	bool running_;

	// Fonction exécutée par chaque worker dans son thread.
	static void workerRun(Worker* worker, const Config &config, Database::DB* db);
	
	// Parcourt les workers et, pour ceux terminés (WORKER_DONE), met à jour les chunks via WorldManager.
	void checkWorkers(std::vector<Chunk>& chunks, const Config &config, Database::DB* db);
	// Affecte à un worker idle une tâche de génération de chunk si un chunk autour du joueur est marqué dirty.
	void ensureChunks(InputManager::State* playerState, std::vector<Chunk>& chunks,
					  const Config &config, Database::DB* db);
};

#endif // WORKER_MANAGER_HPP

} // namespace WorkerManager