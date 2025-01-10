#pragma once

#if defined(PLATFORM_POSIX) && defined(WITH_X)

#include <string>
#include <X11/Xlib.h>
#include "viewwindow.h"
#include "viewresources_x.h"
#include "viewwindowresources_x.h"

class ViewWindowX : public ViewWindow {
public:
	ViewWindowX(unsigned width, unsigned height, const std::string &name, bool canResize, ViewResourcesX &res);
	virtual ~ViewWindowX();

	virtual void setPixel(unsigned x, unsigned y, uint8_t red, uint8_t green, uint8_t blue);
	virtual void clear();
	virtual void resize(unsigned width, unsigned height);
	virtual void redraw();

	Window getWindow() const;

private:
	ViewWindowResourcesX m_winRes;
	Visual *const m_visual;
	int m_depth;

	void recreateImage();
};

#endif
