#pragma once

#include <vector>
#include <cinttypes>

#if defined(PLATFORM_WIN) || (defined(PLATFORM_POSIX) && defined(WITH_X))
#include <memory>
#include "viewwindow.h"
#endif

class View {
public:
	View();
	virtual ~View();

	void update(const std::vector<uint32_t> &spectrum, const std::vector<uint32_t> &spectrogram, const std::vector<uint32_t> &constellation, const std::vector<uint32_t> &peakMeter, const std::vector<int16_t> &audioBuffer);

	virtual int getFD() const    = 0;
	virtual void readHandler()   = 0;
	virtual void reopenWindows() = 0;

#if defined(PLATFORM_WIN) || (defined(PLATFORM_POSIX) && defined(WITH_X))
protected:
	std::unique_ptr<ViewWindow> m_oscilloscope;
	std::unique_ptr<ViewWindow> m_spectrum;
	std::unique_ptr<ViewWindow> m_constellation;

private:
	void createOscilloscope(const std::vector<int16_t> &audioBuffer);
	void createSpectrum(const std::vector<uint32_t> &spectrum, const std::vector<uint32_t> &spectrogram);
	void createConstellation(const std::vector<uint32_t> &constellation, const std::vector<uint32_t> &peakMeter);
#endif
};
