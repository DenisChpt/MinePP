#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <thread>
#include <mutex>
#include <atomic>

namespace Network {

class Client {
public:
	Client();
	~Client();

	// Active ou désactive le client.
	void enable();
	void disable();
	bool isEnabled() const;

	// Se connecte au serveur en fournissant l'hôte et le port.
	bool connect(const std::string &hostname, int port);

	// Démarre le thread de réception.
	void start();

	// Arrête le client (ferme la connexion et arrête le thread).
	void stop();

	// Envoie une chaîne de caractères sur la connexion.
	void send(const std::string &data);

	// Récupère une ligne complète (terminée par '\n') depuis le tampon de réception.
	// Renvoie une chaîne vide si aucun message complet n'est disponible.
	std::string recv();

	// Envoie divers messages formatés :
	void version(int version);
	void login(const std::string &username, const std::string &identityToken);
	void position(float x, float y, float z, float rx, float ry);
	void chunk(int p, int q, int key);
	void block(int x, int y, int z, int w);
	void light(int x, int y, int z, int w);
	void sign(int x, int y, int z, int face, const std::string &text);
	void talk(const std::string &text);

private:
	// Envoie toutes les données (boucle jusqu'à transmission complète).
	int sendAll(const char *data, int length);

	// Fonction exécutée par le thread de réception.
	void recvWorker();

	// Socket de communication.
	int sd_;

	// Indique si le client est activé.
	std::atomic<bool> clientEnabled_;

	// Indique si le thread de réception doit continuer à tourner.
	std::atomic<bool> running_;

	// Tampon de réception (queue).
	std::string recvQueue_;
	mutable std::mutex queueMutex_;

	// Thread de réception.
	std::thread recvThread_;

	// Statistiques (optionnelles)
	int bytesSent_;
	int bytesReceived_;

	// Constantes pour la gestion du tampon.
	static constexpr int QUEUE_SIZE = 1048576;
	static constexpr int RECV_SIZE = 4096;
};

} // namespace Network

#endif // CLIENT_HPP
