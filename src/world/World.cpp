#include "World.hpp"
#include "Noise.hpp"
#include "IConfig.hpp"

namespace World {

void createWorld(const IConfig &config, int p, int q, WorldFunc func, void* arg) {
	int pad = 1;
	// Utilisation de la taille de chunk issue de la configuration.
	int chunkSize = config.getChunkSize();
	
	for (int dx = -pad; dx < chunkSize + pad; dx++) {
		for (int dz = -pad; dz < chunkSize + pad; dz++) {
			int flag = 1;
			if (dx < 0 || dz < 0 || dx >= chunkSize || dz >= chunkSize) {
				flag = -1;
			}
			int x = p * chunkSize + dx;
			int z = q * chunkSize + dz;
			float f = Noise::simplex2(x * 0.01f, z * 0.01f, 4, 0.5f, 2);
			float g = Noise::simplex2(-x * 0.01f, -z * 0.01f, 2, 0.9f, 2);
			int mh = static_cast<int>(g * 32) + 16;
			int h = static_cast<int>(f * mh);
			int w = 1;
			int t = 12;
			if (h <= t) {
				h = t;
				w = 2;
			}
			// Génération du terrain (sable et herbe)
			for (int y = 0; y < h; y++) {
				func(x, y, z, w * flag, arg);
			}
			if (w == 1) {
				if (config.showPlants()) {
					// Herbe
					if (Noise::simplex2(-x * 0.1f, z * 0.1f, 4, 0.8f, 2) > 0.6f) {
						func(x, h, z, 17 * flag, arg);
					}
					// Fleurs
					if (Noise::simplex2(x * 0.05f, -z * 0.05f, 4, 0.8f, 2) > 0.7f) {
						int wFlower = 18 + static_cast<int>(Noise::simplex2(x * 0.1f, z * 0.1f, 4, 0.8f, 2) * 7);
						func(x, h, z, wFlower * flag, arg);
					}
				}
				// Arbres
				int ok = config.showTrees() ? 1 : 0;
				if (dx - 4 < 0 || dz - 4 < 0 || dx + 4 >= chunkSize || dz + 4 >= chunkSize) {
					ok = 0;
				}
				if (ok && Noise::simplex2(x, z, 6, 0.5f, 2) > 0.84f) {
					for (int y = h + 3; y < h + 8; y++) {
						for (int ox = -3; ox <= 3; ox++) {
							for (int oz = -3; oz <= 3; oz++) {
								int d = (ox * ox) + (oz * oz) + (y - (h + 4)) * (y - (h + 4));
								if (d < 11) {
									func(x + ox, y, z + oz, 15, arg);
								}
							}
						}
					}
					for (int y = h; y < h + 7; y++) {
						func(x, y, z, 5, arg);
					}
				}
			}
			// Nuages
			if (config.showClouds()) {
				for (int y = 64; y < 72; y++) {
					if (Noise::simplex3(x * 0.01f, y * 0.1f, z * 0.01f, 8, 0.5f, 2) > 0.75f) {
						func(x, y, z, 16 * flag, arg);
					}
				}
			}
		}
	}
}

} // namespace World
