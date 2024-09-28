#include <algorithm>
#include "deduper.h"
#include "log.h"

static const size_t MAX_HITS = 1024;

bool Deduper::operator()(uint32_t crc)
{
	if(std::find(m_crcs.begin(), m_crcs.end(), crc) != m_crcs.end()) {
		return false;
	}

	if(m_crcs.size() == MAX_HITS) {
		loge("Deduper cache size exceeded; are you running this program that long? Expect duplicate images");
		m_crcs.erase(m_crcs.begin());
	}

	m_crcs.push_back(crc);
	return true;
}
