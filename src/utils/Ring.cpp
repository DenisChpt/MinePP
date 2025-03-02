#include "Ring.hpp"

namespace Utils {

Ring::Ring(unsigned int capacity)
	: capacity_(capacity), start_(0), end_(0), data_(capacity) {}

bool Ring::empty() const {
	return start_ == end_;
}

bool Ring::full() const {
	return (end_ + 1) % capacity_ == start_;
}

int Ring::size() const {
	if (end_ >= start_) {
		return end_ - start_;
	} else {
		return capacity_ - (start_ - end_);
	}
}

void Ring::grow() {
	unsigned int newCapacity = capacity_ * 2;
	std::vector<RingEntry> newData(newCapacity);
	int currentSize = size();
	for (int i = 0; i < currentSize; i++) {
		newData[i] = data_[(start_ + i) % capacity_];
	}
	data_ = std::move(newData);
	capacity_ = newCapacity;
	start_ = 0;
	end_ = currentSize;
}

void Ring::put(const RingEntry& entry) {
	if (full()) {
		grow();
	}
	data_[end_] = entry;
	end_ = (end_ + 1) % capacity_;
}

bool Ring::get(RingEntry &entry) {
	if (empty()) {
		return false;
	}
	entry = data_[start_];
	start_ = (start_ + 1) % capacity_;
	return true;
}

void Ring::putBlock(int p, int q, int x, int y, int z, int w) {
	RingEntry entry;
	entry.type = RingEntryType::BLOCK;
	entry.p = p;
	entry.q = q;
	entry.x = x;
	entry.y = y;
	entry.z = z;
	entry.w = w;
	put(entry);
}

void Ring::putLight(int p, int q, int x, int y, int z, int w) {
	RingEntry entry;
	entry.type = RingEntryType::LIGHT;
	entry.p = p;
	entry.q = q;
	entry.x = x;
	entry.y = y;
	entry.z = z;
	entry.w = w;
	put(entry);
}

void Ring::putKey(int p, int q, int key) {
	RingEntry entry;
	entry.type = RingEntryType::KEY;
	entry.p = p;
	entry.q = q;
	entry.key = key;
	put(entry);
}

void Ring::putCommit() {
	RingEntry entry;
	entry.type = RingEntryType::COMMIT;
	put(entry);
}

void Ring::putExit() {
	RingEntry entry;
	entry.type = RingEntryType::EXIT;
	put(entry);
}

} // namespace Utils
