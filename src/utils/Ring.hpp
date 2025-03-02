#ifndef RING_HPP
#define RING_HPP

#include <vector>
#include <cstddef>

namespace Utils {

enum class RingEntryType {
	BLOCK,
	LIGHT,
	KEY,
	COMMIT,
	EXIT
};

struct RingEntry {
	RingEntryType type;
	int p = 0;
	int q = 0;
	int x = 0;
	int y = 0;
	int z = 0;
	int w = 0;
	int key = 0;
};

class Ring {
public:
	explicit Ring(unsigned int capacity = 16);
	~Ring() = default;

	bool empty() const;
	bool full() const;
	int size() const;
	void grow();
	void put(const RingEntry& entry);
	bool get(RingEntry &entry);

	void putBlock(int p, int q, int x, int y, int z, int w);
	void putLight(int p, int q, int x, int y, int z, int w);
	void putKey(int p, int q, int key);
	void putCommit();
	void putExit();

private:
	unsigned int capacity_;
	unsigned int start_;
	unsigned int end_;
	std::vector<RingEntry> data_;
};

} // namespace Utils

#endif // RING_HPP
