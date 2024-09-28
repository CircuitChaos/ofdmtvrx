#include <cstring>
#include "crsdecoder.h"
#include "throw.h"
#include "decoder_constants.h"
#include "file.h"
#include "log.h"

void CRSDecoder::usePayload(const std::vector<uint8_t> &payload)
{
	xassert(payload.size() == decoder_constants::data_bits / 8, "Invalid payload size (%zu bytes)", payload.size());
	if(memcmp(&payload[0], "CRS", 3)) {
		logn("Invalid payload (not CRS)");
		return;
	}

	const uint16_t blockCount(((payload[4] & 255) << 8) + (payload[3] & 255) + 1);
	const uint16_t blockIdent(((payload[6] & 255) << 8) + (payload[5] & 255));
	const uint32_t imageBytes(((payload[9] & 255) << 16) + ((payload[8] & 255) << 8) + (payload[7] & 255) + 1);
	const uint32_t imageCRC32(((payload[13] & 255L) << 24) + ((payload[12] & 255L) << 16) + ((payload[11] & 255L) << 8) + (payload[10] & 255L));
	logv("Block count: %u, block ident: %04X, image bytes: %u, image CRC32: %08X", blockCount, blockIdent, imageBytes, imageCRC32);

	/* Some (in)sane limit to protect precious memory */
	if(imageBytes > 10485760) {
		loge("Image size too big (%u bytes); if this is a valid image, please report bug", imageBytes);
		return;
	}

	if(!m_currentsValid || blockCount != m_currentBlockCount || imageBytes != m_currentImageBytes || imageCRC32 != m_currentImageCRC32) {
		logd("New image!");
		m_identList.clear();
		m_currentBlockCount = blockCount;
		m_currentImageBytes = imageBytes;
		m_currentImageCRC32 = imageCRC32;
		m_currentsValid     = true;
	}

	if(m_identList.find(blockIdent) != m_identList.end()) {
		logv("Duplicate chunk received");
		return;
	}

	if(m_identList.size() == blockCount) {
		logv("Redundant chunk received");
		return;
	}

	m_crsec.chunk(&payload[0], m_identList.size(), blockIdent);
	m_identList.insert(blockIdent);
	if(m_identList.size() < blockCount) {
		return;
	}

	m_data.resize(imageBytes);
	if(m_crsec.recover(&m_data[0], m_data.size(), m_identList.size()) != imageCRC32) {
		logv("Image corrupted");
		m_data.clear();
		m_currentsValid = false;
		// TODO add CLI option to save anyway
		return;
	}

	if(!m_deduper(imageCRC32)) {
		logn("Refusing to save duplicate image");
		m_data.clear();
		m_currentsValid = false;
		return;
	}
}

bool CRSDecoder::fileReady(uint32_t &crc) const
{
	if(m_data.empty()) {
		return false;
	}

	xassert(m_currentsValid, "Currents must be valid here");
	crc = m_currentImageCRC32;
	return true;
}

std::string CRSDecoder::getExtension() const
{
	// TODO some decoding to determine extension
	return "jpg";
}

void CRSDecoder::save(const std::string &path)
{
	logn("Saving file: %s", path.c_str());

	xassert(!m_data.empty(), "save() called on empty data");

	File f(fopen(path.c_str(), "wb"));
	xassert(f, "Could not open file %s: %m", path.c_str());
	xassert(fwrite(&m_data[0], 1, m_data.size(), f) == m_data.size(), "Write error, disk full?");
	xassert(fclose(f) == 0, "Could not close file %s: %m", path.c_str());
	f.release();

	m_data.clear();
	m_currentsValid = false;
}
