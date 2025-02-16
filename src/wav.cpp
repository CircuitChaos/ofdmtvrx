#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include "wav.h"
#include "throw.h"
#include "log.h"
#include "platform.h"

Wav::Wav(const std::string &file)
{
	if(!file.empty()) {
		m_fd = open(file.c_str(), WAV_OPEN_FLAGS);
		xassert(m_fd >= 0, "Could not open %s: %s", file.c_str(), strerror(errno));
		m_close = true;
	}
#ifdef PLATFORM_WIN
	else {
		/* stdin has to be open in binary mode on Windows */
		_setmode(_fileno(stdin), _O_BINARY);
	}
#endif

	if(!readHeader()) {
		if(m_close) {
			close(m_fd);
		}

		xthrow("WAV header error");
	}
}

Wav::~Wav()
{
	if(m_close) {
		close(m_fd);
	}
}

bool Wav::readHeaderPart(void *data, size_t size)
{
	char *p((char *) data);
	size_t pos(0);
	while(pos != size) {
		const ssize_t rs(read(m_fd, p + pos, size - pos));
		if(rs < 0) {
			loge("Error reading WAV header: %s", strerror(errno));
			return false;
		}

		if(rs == 0) {
			loge("EOF when reading WAV header");
			return false;
		}

		if((size_t) rs > (size - pos)) {
			loge("read() returned nonsense (" SSIZET_FMT " gt " SIZET_FMT ")", rs, size - pos);
			return false;
		}

		pos += rs;
	}

	return true;
}

bool Wav::skip(size_t size)
{
	while(size) {
		const size_t bufsz((size > 16384) ? 16384 : size);
		char buf[bufsz];
		if(!readHeaderPart(buf, bufsz)) {
			return false;
		}

		size -= bufsz;
	}

	return true;
}

bool Wav::readFormatSubchunk()
{
	struct FmtSubchunk {
		uint16_t audioFormat;
		uint16_t numChannels;
		uint32_t sampleRate;
		uint32_t byteRate;
		uint16_t blockAlign;
		uint16_t bitsPerSample;
	} __attribute__((packed));

	FmtSubchunk fmt;
	if(!readHeaderPart(&fmt, sizeof(fmt))) {
		return false;
	}

	fmt.audioFormat   = le16toh(fmt.audioFormat);
	fmt.numChannels   = le16toh(fmt.numChannels);
	fmt.sampleRate    = le32toh(fmt.sampleRate);
	fmt.byteRate      = le32toh(fmt.byteRate);
	fmt.blockAlign    = le16toh(fmt.blockAlign);
	fmt.bitsPerSample = le16toh(fmt.bitsPerSample);

	if(fmt.audioFormat != 1) {
		loge("WAV audio format %u unknown (must be 1), perhaps file is compressed?", fmt.audioFormat);
		return false;
	}

	if(fmt.numChannels < 1 || fmt.numChannels > 8) {
		loge("WAV has %u channels, doesn't make sense", fmt.numChannels);
		return false;
	}

	if(fmt.sampleRate < 8000 || fmt.sampleRate > 96000) {
		loge("WAV probably has bogus sample rate %u, report if it's incorrect", fmt.sampleRate);
		return false;
	}

	if(fmt.bitsPerSample != 16) {
		loge("WAV has unsupported number of bits per sample %u (must be 16)", fmt.bitsPerSample);
		return false;
	}

	if(fmt.byteRate != fmt.numChannels * fmt.sampleRate * fmt.bitsPerSample / 8) {
		loge("WAV byte rate %u bad (must be %u)", fmt.byteRate, fmt.numChannels * fmt.sampleRate * fmt.bitsPerSample / 8);
		return false;
	}

	if(fmt.blockAlign != fmt.numChannels * fmt.bitsPerSample / 8) {
		loge("WAV has unsupported block align value %u (must be %u)", fmt.blockAlign, fmt.numChannels * fmt.bitsPerSample / 8);
		return false;
	}

	m_rate     = fmt.sampleRate;
	m_channels = fmt.numChannels;
	return true;
}

bool Wav::readHeader()
{
	struct Hdr {
		char chunkID[4];
		uint32_t chunkSize;
		char format[4];
	} __attribute__((packed));

	Hdr h;
	if(!readHeaderPart(&h, sizeof(h))) {
		return false;
	}

	// Warning: if h.chunkSize is to be used, make sure to do le32toh on it

	if(memcmp(h.chunkID, "RIFF", 4)) {
		loge("WAV chunk ID bad (must be RIFF)");
		return false;
	}

	if(memcmp(h.format, "WAVE", 4)) {
		loge("WAV format bad (must be WAVE)");
		return false;
	}

	/* - find fmt subchunk and decode it
	 * - ignore other subchunks
	 * - stop at data subchunk
	 */

	bool haveFmt(false);
	for(;;) {
		struct SubchunkHdr {
			char id[4];
			uint32_t size;
		} __attribute__((packed));

		SubchunkHdr sh;
		if(!readHeaderPart(&sh, sizeof(sh))) {
			return false;
		}

		sh.size = le32toh(sh.size);
		logd("Got subchunk \"%c%c%c%c\" (%u bytes)", sh.id[0], sh.id[1], sh.id[2], sh.id[3], sh.size);

		if(!memcmp(sh.id, "fmt ", 4)) {
			/* Got fmt subchunk */
			if(sh.size != 16) {
				loge("WAV subchunk 1 size %u bad (must be 16)", sh.size);
				return false;
			}

			if(!readFormatSubchunk()) {
				return false;
			}

			haveFmt = true;
			continue;
		}

		if(!memcmp(sh.id, "data", 4)) {
			break;
		}

		skip(sh.size);
	}

	/* In theory, there might be other subchunks after data, but we ignore it,
	 * as we can't rely on data subchunks size (because it might be a real-time
	 * stream and data size isn't known yet). So we just process everything
	 * after data subchunk header.
	 */

	if(!haveFmt) {
		loge("\"fmt \" chunk not found");
		return false;
	}

	logd("WAV sample rate: %u Hz, %u channel(s)", m_rate, m_channels);
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
	if(m_rawBuffer.size() < audioBuffer.size() * 2 * m_channels) {
		return false;
	}

	for(size_t i(0); i != audioBuffer.size(); ++i) {
		int32_t sum(0);
		for(size_t chan(0); chan < m_channels; ++chan) {
			const uint16_t *le((const uint16_t *) &m_rawBuffer[(i * m_channels + chan) * 2]);
			const uint16_t he(le16toh(*le));
			int16_t heSigned;
			memcpy(&heSigned, &he, 2);
			sum += heSigned;
		}

		audioBuffer[i] = sum / m_channels;
	}

	m_rawBuffer.erase(m_rawBuffer.begin(), m_rawBuffer.begin() + audioBuffer.size() * 2 * m_channels);
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

	xassert(rs > 0, "Error reading WAV: %s", strerror(errno));
	xassert((size_t) rs <= sizeof(buf), "WAV read returned nonsense (" SSIZET_FMT " gt " SIZET_FMT ")", rs, sizeof(buf));

	const size_t curSize(m_rawBuffer.size());
	m_rawBuffer.resize(curSize + rs);
	memcpy(&m_rawBuffer[curSize], buf, rs);
}
