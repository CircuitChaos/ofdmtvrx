#include <endian.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include "wav.h"
#include "throw.h"
#include "log.h"

Wav::Wav(const std::string &file)
{
	if(!file.empty()) {
		m_fd = open(file.c_str(), O_RDONLY | O_LARGEFILE);
		xassert(m_fd >= 0, "Could not open %s: %m", file.c_str());
		m_close = true;
	}

	if(!readHeader()) {
		if(m_close) {
			close(m_fd);
			xthrow("WAV header error");
		}
	}
}

Wav::~Wav()
{
	if(m_close) {
		close(m_fd);
	}
}

bool Wav::readHeader()
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

	char buf[sizeof(Hdr)];
	size_t pos(0);
	while(pos != sizeof(Hdr)) {
		const ssize_t rs(read(m_fd, buf + pos, sizeof(Hdr) - pos));
		if(rs < 0) {
			loge("Error reading WAV header: %m");
			return false;
		}

		if(rs == 0) {
			loge("EOF when reading WAV header");
			return false;
		}

		if((size_t) rs > (sizeof(Hdr) - pos)) {
			loge("read() returned nonsense (%zd gt %zu)", rs, sizeof(Hdr) - pos);
			return false;
		}

		pos += rs;
	}

	Hdr h;
	memcpy(&h, buf, sizeof(Hdr));

	// not done, as this field is unused by us
	// h.chunkSize     = le32toh(h.chunkSize);
	h.subchunk1Size = le32toh(h.subchunk1Size);
	h.audioFormat   = le16toh(h.audioFormat);
	h.numChannels   = le16toh(h.numChannels);
	h.sampleRate    = le32toh(h.sampleRate);
	h.byteRate      = le32toh(h.byteRate);
	h.blockAlign    = le16toh(h.blockAlign);
	h.bitsPerSample = le16toh(h.bitsPerSample);
	// not done, as this field is unused by us
	// h.subchunk2Size = le32toh(h.subchunk2Size);

	if(memcmp(h.chunkID, "RIFF", 4)) {
		loge("WAV chunk ID bad (must be RIFF)");
		return false;
	}

	if(memcmp(h.format, "WAVE", 4)) {
		loge("WAV format bad (must be WAVE)");
		return false;
	}

	if(memcmp(h.subchunk1ID, "fmt ", 4)) {
		loge("WAV subchunk 1 ID bad (must be fmt)");
		return false;
	}

	if(memcmp(h.subchunk2ID, "data", 4)) {
		loge("WAV subchunk 2 ID bad (must be data), perhaps extra params exist? Report it");
		return false;
	}

	if(h.subchunk1Size != 16) {
		loge("WAV subchunk 1 size %u bad (must be 16)", h.subchunk1Size);
		return false;
	}

	if(h.audioFormat != 1) {
		loge("WAV audio format %u unknown (must be 1), perhaps file is compressed?", h.audioFormat);
		return false;
	}

	if(h.numChannels != 1) {
		loge("WAV has %u channels, but only mono files are supported", h.numChannels);
		return false;
	}

	if(h.sampleRate < 8000 || h.sampleRate > 96000) {
		loge("WAV probably has bogus sample rate %u, report if it's incorrect", h.sampleRate);
		return false;
	}

	if(h.bitsPerSample != 16) {
		loge("WAV has unsupported number of bits per sample %u (must be 16)", h.bitsPerSample);
		return false;
	}

	if(h.byteRate != h.numChannels * h.sampleRate * h.bitsPerSample / 8) {
		loge("WAV byte rate %u bad (must be %u)", h.byteRate, h.numChannels * h.sampleRate * h.bitsPerSample / 8);
		return false;
	}

	if(h.blockAlign != h.numChannels * h.bitsPerSample / 8) {
		loge("WAV has unsupported block align value %u (must be %u)", h.blockAlign, h.numChannels * h.bitsPerSample / 8);
		return false;
	}

	m_rate = h.sampleRate;
	logd("WAV sample rate: %u Hz", m_rate);
	return true;
}

unsigned Wav::getRate() const
{
	return m_rate;
}

int Wav::getFD() const
{
	return m_fd;
}

bool Wav::isEOF() const
{
	return m_eof;
}

bool Wav::getBuffer(std::vector<int16_t> &audioBuffer)
{
	if(m_rawBuffer.size() < audioBuffer.size() * 2) {
		return false;
	}

	for(size_t i(0); i != audioBuffer.size(); ++i) {
		const uint16_t *le((const uint16_t *) &m_rawBuffer[i * 2]);
		const uint16_t he(le16toh(*le));
		memcpy(&audioBuffer[i], &he, 2);
	}

	m_rawBuffer.erase(m_rawBuffer.begin(), m_rawBuffer.begin() + audioBuffer.size() * 2);
	return true;
}

void Wav::readHandler()
{
	char buf[1024];
	const ssize_t rs(read(m_fd, buf, sizeof(buf)));
	if(rs == 0) {
		m_eof = true;
		return;
	}

	xassert(rs > 0, "Error reading WAV: %m");
	xassert((size_t) rs <= sizeof(buf), "WAV read returned nonsense (%zd gt %zu)", rs, sizeof(buf));

	const size_t curSize(m_rawBuffer.size());
	m_rawBuffer.resize(curSize + rs);
	memcpy(&m_rawBuffer[curSize], buf, rs);
}
