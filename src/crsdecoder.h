#pragma once

#include <vector>
#include <string>
#include <cinttypes>
#include <set>
#include "assempix/crsec.hh"
#include "deduper.h"

class CRSDecoder {
public:
	void usePayload(const std::vector<uint8_t> &payload);

	bool fileReady(uint32_t &crc) const;
	std::string getExtension() const;
	void save(const std::string &path);

private:
	bool m_currentsValid{false};
	uint16_t m_currentBlockCount{0};
	uint32_t m_currentImageBytes{0};
	uint32_t m_currentImageCRC32{0};
	std::set<uint16_t> m_identList;

	CauchyReedSolomonErasureCoding m_crsec;
	std::vector<uint8_t> m_data;

	Deduper m_deduper;
};
