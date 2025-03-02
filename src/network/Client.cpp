#include "Client.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define close closesocket
// Pour Windows, initialisation de Winsock à prévoir dans le main.
#else
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace Network
{

	Client::Client()
		: sd_(-1),
		  clientEnabled_(false),
		  running_(false),
		  bytesSent_(0),
		  bytesReceived_(0)
	{
	}

	Client::~Client()
	{
		stop();
	}

	void Client::enable()
	{
		clientEnabled_ = true;
	}

	void Client::disable()
	{
		clientEnabled_ = false;
	}

	bool Client::isEnabled() const
	{
		return clientEnabled_;
	}

	bool Client::connect(const std::string &hostname, int port)
	{
		if (!clientEnabled_)
		{
			return false;
		}

		struct hostent *host = gethostbyname(hostname.c_str());
		if (!host)
		{
			perror("gethostbyname");
			return false;
		}

		struct sockaddr_in address;
		std::memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = ((struct in_addr *)(host->h_addr_list[0]))->s_addr;
		address.sin_port = htons(port);

		sd_ = socket(AF_INET, SOCK_STREAM, 0);
		if (sd_ == -1)
		{
			perror("socket");
			return false;
		}
		if (::connect(sd_, (struct sockaddr *)&address, sizeof(address)) == -1)
		{
			perror("connect");
			return false;
		}
		return true;
	}

	void Client::start()
	{
		if (!clientEnabled_)
		{
			return;
		}
		running_ = true;
		{
			std::lock_guard<std::mutex> lock(queueMutex_);
			recvQueue_.clear();
		}
		// Démarrage du thread de réception.
		recvThread_ = std::thread(&Client::recvWorker, this);
	}

	void Client::stop()
	{
		if (!clientEnabled_)
		{
			return;
		}
		running_ = false;
		if (sd_ != -1)
		{
			close(sd_);
			sd_ = -1;
		}
		if (recvThread_.joinable())
		{
			recvThread_.join();
		}
		{
			std::lock_guard<std::mutex> lock(queueMutex_);
			recvQueue_.clear();
		}
	}

	int Client::sendAll(const char *data, int length)
	{
		if (!clientEnabled_)
		{
			return 0;
		}
		int totalSent = 0;
		while (totalSent < length)
		{
			int n = ::send(sd_, data + totalSent, length - totalSent, 0);
			if (n == -1)
			{
				perror("send");
				return -1;
			}
			totalSent += n;
			bytesSent_ += n;
		}
		return totalSent;
	}

	void Client::send(const std::string &data)
	{
		if (!clientEnabled_)
		{
			return;
		}
		if (sendAll(data.c_str(), static_cast<int>(data.size())) == -1)
		{
			perror("sendAll");
			std::exit(1);
		}
	}

	std::string Client::recv()
	{
		std::lock_guard<std::mutex> lock(queueMutex_);
		size_t pos = recvQueue_.find('\n');
		if (pos != std::string::npos)
		{
			std::string result = recvQueue_.substr(0, pos + 1);
			recvQueue_.erase(0, pos + 1);
			bytesReceived_ += static_cast<int>(result.size());
			return result;
		}
		return "";
	}

	void Client::version(int version)
	{
		if (!clientEnabled_)
		{
			return;
		}
		char buffer[1024];
		std::snprintf(buffer, sizeof(buffer), "V,%d\n", version);
		send(buffer);
	}

	void Client::login(const std::string &username, const std::string &identityToken)
	{
		if (!clientEnabled_)
		{
			return;
		}
		char buffer[1024];
		std::snprintf(buffer, sizeof(buffer), "A,%s,%s\n", username.c_str(), identityToken.c_str());
		send(buffer);
	}

	void Client::position(float x, float y, float z, float rx, float ry)
	{
		if (!clientEnabled_)
		{
			return;
		}
		static float px = 0, py = 0, pz = 0, prx = 0, pry = 0;
		float distance =
			(px - x) * (px - x) +
			(py - y) * (py - y) +
			(pz - z) * (pz - z) +
			(prx - rx) * (prx - rx) +
			(pry - ry) * (pry - ry);
		if (distance < 0.0001f)
		{
			return;
		}
		px = x;
		py = y;
		pz = z;
		prx = rx;
		pry = ry;
		char buffer[1024];
		std::snprintf(buffer, sizeof(buffer), "P,%.2f,%.2f,%.2f,%.2f,%.2f\n", x, y, z, rx, ry);
		send(buffer);
	}

	void Client::chunk(int p, int q, int key)
	{
		if (!clientEnabled_)
		{
			return;
		}
		char buffer[1024];
		std::snprintf(buffer, sizeof(buffer), "C,%d,%d,%d\n", p, q, key);
		send(buffer);
	}

	void Client::block(int x, int y, int z, int w)
	{
		if (!clientEnabled_)
		{
			return;
		}
		char buffer[1024];
		std::snprintf(buffer, sizeof(buffer), "B,%d,%d,%d,%d\n", x, y, z, w);
		send(buffer);
	}

	void Client::light(int x, int y, int z, int w)
	{
		if (!clientEnabled_)
		{
			return;
		}
		char buffer[1024];
		std::snprintf(buffer, sizeof(buffer), "L,%d,%d,%d,%d\n", x, y, z, w);
		send(buffer);
	}

	void Client::sign(int x, int y, int z, int face, const std::string &text)
	{
		if (!clientEnabled_)
		{
			return;
		}
		char buffer[1024];
		std::snprintf(buffer, sizeof(buffer), "S,%d,%d,%d,%d,%s\n", x, y, z, face, text.c_str());
		send(buffer);
	}

	void Client::talk(const std::string &text)
	{
		if (!clientEnabled_ || text.empty())
		{
			return;
		}
		char buffer[1024];
		std::snprintf(buffer, sizeof(buffer), "T,%s\n", text.c_str());
		send(buffer);
	}

	void Client::recvWorker()
	{
		char buffer[RECV_SIZE];
		while (running_)
		{
			int length = ::recv(sd_, buffer, RECV_SIZE - 1, 0);
			if (length <= 0)
			{
				if (running_)
				{
					perror("recv");
					std::exit(1);
				}
				else
				{
					break;
				}
			}
			buffer[length] = '\0';
			{
				std::lock_guard<std::mutex> lock(queueMutex_);
				// On ajoute les données reçues au tampon.
				recvQueue_.append(buffer, length);
				// (On pourrait limiter la taille de recvQueue_ à QUEUE_SIZE si nécessaire.)
			}
			std::this_thread::yield(); // cède le temps d'exécution
		}
	}

} // namespace Network
