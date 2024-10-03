#pragma once

#include <cinttypes>
#include <string>
#include <vector>
#include "file.h"

/* For simplicity, this class supports only:
 * - single-channel wav
 * - s16_le format
 */

class Wav {
public:
	/* file can be empty string to read from stdin */
	Wav(const std::string &file);
	~Wav();

	unsigned getRate() const;
	int getFD() const;
	void readHandler();
	bool isEOF() const;
	bool getBuffer(std::vector<int16_t> &audioBuffer);

private:
	int m_fd{0};
	unsigned m_rate{0};
	std::vector<uint8_t> m_rawBuffer;
	bool m_close{false};
	bool m_eof{false};

	bool readHeader();
};
