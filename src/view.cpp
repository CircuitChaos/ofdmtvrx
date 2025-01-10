#include "view.h"

View::View()
{
}

View::~View()
{
}

#if defined(PLATFORM_WIN) || (defined(PLATFORM_POSIX) && defined(WITH_X))

#include "decoder_constants.h"

void View::update(const std::vector<uint32_t> &spectrum, const std::vector<uint32_t> &spectrogram, const std::vector<uint32_t> &constellation, const std::vector<uint32_t> &peakMeter, const std::vector<int16_t> &audioBuffer)
{
	if(m_oscilloscope) {
		createOscilloscope(audioBuffer);
		m_oscilloscope->redraw();
	}

	if(m_spectrum) {
		createSpectrum(spectrum, spectrogram);
		m_spectrum->redraw();
	}

	if(m_constellation) {
		createConstellation(constellation, peakMeter);
		m_constellation->redraw();
	}

	// TODO call readHandler here? check if works without it
}

void View::createOscilloscope(const std::vector<int16_t> &audioBuffer)
{
	m_oscilloscope->clear();

	const unsigned width(m_oscilloscope->getWidth());
	const unsigned height(m_oscilloscope->getHeight());
	unsigned oldx(width), oldy(height);

	for(size_t i(0); i < audioBuffer.size(); ++i) {
		const unsigned x((double) i * m_oscilloscope->getWidth() / audioBuffer.size());
		const uint16_t sample(65535 - (audioBuffer[i] + 32768));
		const unsigned y((double) sample * m_oscilloscope->getHeight() / 65535);

		/* Can it happen? */
		if(x >= m_oscilloscope->getWidth() || y >= m_oscilloscope->getHeight()) {
			continue;
		}

		if(oldx != width) {
			m_oscilloscope->drawLine(oldx, oldy, x, y, 0xff, 0xff, 0xff);
		}

		oldx = x;
		oldy = y;
	}
}

void View::createSpectrum(const std::vector<uint32_t> &spectrum, const std::vector<uint32_t> &spectrogram)
{
	for(unsigned x(0); x < decoder_constants::spectrum_width; ++x) {
		bool drawingBlue(false);
		for(unsigned y(0); y < decoder_constants::spectrum_height; ++y) {
			if(spectrum[y * decoder_constants::spectrum_width + x]) {
				m_spectrum->setPixel(x, y, 0xff, 0xff, 0xff);
				drawingBlue = true;
			}
			else if(drawingBlue) {
				m_spectrum->setPixel(x, y, 0, 0, 0x30);
			}
			else {
				m_spectrum->setPixel(x, y, 0, 0, 0);
			}
		}
	}

	for(unsigned y(0); y < decoder_constants::spectrogram_height; ++y) {
		for(unsigned x(0); x < decoder_constants::spectrogram_width; ++x) {
			m_spectrum->setPixel(x, y + decoder_constants::spectrum_height, spectrogram[y * decoder_constants::spectrogram_width + x]);
		}
	}
}

void View::createConstellation(const std::vector<uint32_t> &constellation, const std::vector<uint32_t> &peakMeter)
{
	for(unsigned y(0); y < decoder_constants::constellation_height; ++y) {
		for(unsigned x(0); x < decoder_constants::constellation_width; ++x) {
			const uint8_t rgb(constellation[y * decoder_constants::constellation_width + x] ? 0xff : 0);
			m_constellation->setPixel(x, y, rgb, rgb, rgb);
		}
	}

	for(unsigned y(0); y < decoder_constants::peak_meter_height; ++y) {
		for(unsigned x(0); x < decoder_constants::constellation_width; ++x) {
			m_constellation->setPixel(x, y + decoder_constants::constellation_height, peakMeter[x / (decoder_constants::constellation_width / decoder_constants::peak_meter_width)]);
		}
	}
}

#else

void View::update(const std::vector<uint32_t> & /* spectrum */, const std::vector<uint32_t> & /* spectrogram */, const std::vector<uint32_t> & /* constellation */, const std::vector<uint32_t> & /* peakMeter */, const std::vector<int16_t> & /* audioBuffer */)
{
}

#endif
