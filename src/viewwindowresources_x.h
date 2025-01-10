#pragma once

#if defined(PLATFORM_POSIX) && defined(WITH_X)

#include <X11/Xlib.h>
#include <X11/Xutil.h>

struct ViewWindowResourcesX {
public:
	ViewWindowResourcesX(Display *dpy);
	~ViewWindowResourcesX();

	Display *const dpy;
	Window win{None};
	XSizeHints *sizeHints{nullptr};
	char *pixels{nullptr};
	XImage *image{nullptr};
};

#endif
