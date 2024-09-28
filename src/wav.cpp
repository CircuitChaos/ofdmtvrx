#include <endian.h>
#include <cstring>
#include "wav.h"
#include "throw.h"
#include "log.h"

Wav::Wav(const std::string &file)
{
	struct Hdr {
		char chunkID[4];
		uint32_t chunkSize;
		char format[4];
		char subchunk1ID[4];
		uint32_t subchunk1Size;
		uint16_t audioFormat;
		uint16_t numChannels;
		uint32_t sampleRate;
		uint32_t byteRate;
		uint16_t blockAlign;
		uint16_t bitsPerSample;
		char subchunk2ID[4];
		uint32_t subchunk2Size;
	} __attribute__((packed));

	if(!file.empty()) {
		m_file = fopen(file.c_str(), "rb");
		xassert(m_file, "Could not open %s: %m", file.c_str());
	}

	Hdr h;
	xassert(read(&h, sizeof(h)) == sizeof(h), "Could not read WAV header");
	// h.chunkSize     = le32toh(h.chunkSize);
	h.subchunk1Size = le32toh(h.subchunk1Size);
	h.audioFormat   = le16toh(h.audioFormat);
	h.numChannels   = le16toh(h.numChannels);
	h.sampleRate    = le32toh(h.sampleRate);
	h.byteRate      = le32toh(h.byteRate);
	h.blockAlign    = le16toh(h.blockAlign);
	h.bitsPerSample = le16toh(h.bitsPerSample);
	// h.subchunk2Size = le32toh(h.subchunk2Size);

	xassert(!memcmp(h.chunkID, "RIFF", 4), "WAV chunk ID bad (must be RIFF)");
	xassert(!memcmp(h.format, "WAVE", 4), "WAV format bad (must be WAVE)");
	xassert(!memcmp(h.subchunk1ID, "fmt ", 4), "WAV subchunk 1 ID bad (must be fmt)");
	xassert(!memcmp(h.subchunk2ID, "data", 4), "WAV subchunk 2 ID bad (must be data), perhaps extra params exist? Report it");
	xassert(h.subchunk1Size == 16, "WAV subchunk 1 size %u bad (must be 16)", h.subchunk1Size);
	xassert(h.audioFormat == 1, "WAV audio format %u unknown (must be 1), perhaps file is compressed?", h.audioFormat);
	xassert(h.numChannels == 1, "WAV has %u channels, but only mono files are supported", h.numChannels);
	xassert(h.sampleRate >= 8000 && h.sampleRate <= 96000, "WAV probably has bogus sample rate %u, report if it's incorrect", h.sampleRate);
	xassert(h.bitsPerSample == 16, "WAV has unsupported number of bits per sample %u (must be 16)", h.bitsPerSample);
	xassert(h.byteRate == h.numChannels * h.sampleRate * h.bitsPerSample / 8, "WAV byte rate %u bad (must be %u)", h.byteRate, h.numChannels * h.sampleRate * h.bitsPerSample / 8);
	xassert(h.blockAlign == h.numChannels * h.bitsPerSample / 8, "WAV has unsupported block align value %u (must be %u)", h.blockAlign, h.numChannels * h.bitsPerSample / 8);

	m_rate = h.sampleRate;
	logd("WAV sample rate: %u Hz", m_rate);
}

unsigned Wav::getRate() const
{
	return m_rate;
}

bool Wav::getSamples(int16_t *samples, size_t size)
{
	const size_t rs(read(samples, size * 2));
	if(rs != size * 2) {
		return false;
	}

	for(size_t i(0); i < size; ++i) {
		const uint16_t he(le16toh(samples[i]));
		memcpy(&samples[i], &he, 2);
	}

	return true;
}

size_t Wav::read(void *ptr, size_t sz)
{
	FILE *const fp(m_file ? (FILE *) m_file : stdin);
	size_t rs(fread(ptr, 1, sz, fp));
	if(rs == 0) {
		xassert(!ferror(fp), "Read error: %m");
	}

	xassert(rs <= sz, "fread() returned nonsense");
	return rs;
}
