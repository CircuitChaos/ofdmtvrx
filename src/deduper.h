#pragma once

#include <cinttypes>
#include <vector>

class Deduper {
public:
	/* true -- unique, false -- duplicate */
	bool operator()(uint32_t crc);

private:
	/* TODO: implement it better (LRU) */
	/* TODO: vector is slow, but makes it easy to purge old
	 * (last recently CREATED, not used) entries */
	std::vector<uint32_t> m_crcs;
};
