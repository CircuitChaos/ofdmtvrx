#pragma once

#include <vector>
#include <cinttypes>
#include <ctime>

class AudioLevelPrinter {
public:
	void process(const std::vector<int16_t> &buf);

private:
	uint16_t m_peak{0};
	time_t m_oldTime{0};
};
