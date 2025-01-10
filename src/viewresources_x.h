#pragma once

#if defined(PLATFORM_POSIX) && defined(WITH_X)

#include <memory>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <vector>

struct ViewResourcesX {
	~ViewResourcesX();

	Display *dpy{nullptr};
	Atom winDelMsg{None};
	XVisualInfo vi;
	Colormap colormap{None};
};

#endif
