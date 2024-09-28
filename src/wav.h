#pragma once

#include <cinttypes>
#include <string>
#include "file.h"

/* For simplicity, this class supports only:
 * - single-channel wav
 * - s16_le format
 */

class Wav {
public:
	/* file can be empty string to read from stdin */
	Wav(const std::string &file);

	unsigned getRate() const;
	bool getSamples(int16_t *samples, size_t size);

private:
	/* If nullptr, then read from stdin */
	File m_file{nullptr};
	unsigned m_rate{0};

	size_t read(void *ptr, size_t sz);
};
