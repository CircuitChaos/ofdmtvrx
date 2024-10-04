#pragma once

#include <vector>
#include <cinttypes>
#include "xresources.h"

#ifdef WITH_X
#include <string>
#endif

class XView {
public:
	XView();

	int getFD() const;
	void readHandler();
	void update(const std::vector<uint32_t> &spectrum, const std::vector<uint32_t> &spectrogram, const std::vector<uint32_t> &constellation, const std::vector<uint32_t> &peakMeter, const std::vector<int16_t> &audioBuffer);
	void reopenWindows();

#ifdef WITH_X
private:
	XResources m_res;
	std::unique_ptr<XWindow> *getWindow(Window w);

	void createOscilloscope(const std::vector<int16_t> &audioBuffer);
	void createSpectrum(const std::vector<uint32_t> &spectrum, const std::vector<uint32_t> &spectrogram);
	void createConstellation(const std::vector<uint32_t> &constellation, const std::vector<uint32_t> &peakMeter);
#endif
};
