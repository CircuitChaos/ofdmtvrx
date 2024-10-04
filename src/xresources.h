#pragma once

#ifdef WITH_X

#include <memory>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <vector>
#include "xwindow.h"

struct XResources {
	~XResources();

	Display *dpy{nullptr};
	Atom winDelMsg{None};
	XVisualInfo vi;
	Colormap colormap{None};
	std::unique_ptr<XWindow> oscilloscope;
	std::unique_ptr<XWindow> spectrum;
	std::unique_ptr<XWindow> constellation;
};

#endif
