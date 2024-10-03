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

#ifdef WITH_X
private:
	XResources m_res;
	int m_fd{-1};

	void createWindow(std::unique_ptr<XResources::Win> &win, const std::string &name, unsigned width, unsigned height, XSetWindowAttributes *attrs, bool preventResizing);
	void createImage(XResources::Win *win);
	void redraw(XResources::Win *win);
	void copyImageMono(XResources::Win *win, const std::vector<uint32_t> &src, unsigned scale);
	void copyImageColor(XResources::Win *win, const std::vector<uint32_t> &src, unsigned scale);
	void createOscilloscope(const std::vector<int16_t> &audioBuffer);
	void beautifySpectrum();
	XResources::Win *getWindow(Window w);
#endif
};
