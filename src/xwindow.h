#pragma once

#ifdef WITH_X

#include <X11/Xlib.h>
#include <string>
#include "xwindowres.h"

class XResources;

class XWindow {
public:
	XWindow(XResources &res, const std::string &name, unsigned width, unsigned height, bool canResize);

	Window getWindow() const;
	unsigned getWidth() const;
	unsigned getHeight() const;

	/* TODO: This approach might be slow. If it is, rethink. */
	void setPixel(unsigned x, unsigned y, uint8_t red, uint8_t green, uint8_t blue);
	void setPixel(unsigned x, unsigned y, uint32_t rgb);
	void drawLine(unsigned x1, unsigned y1, unsigned x2, unsigned y2, uint8_t red, uint8_t green, uint8_t blue);
	void clear();
	void resize(unsigned width, unsigned height);
	void redraw();

private:
	XWindowRes m_winRes;
	Visual *const m_visual;
	int m_depth;
	unsigned m_width{0};
	unsigned m_height{0};

	void recreateImage();
};

#endif
