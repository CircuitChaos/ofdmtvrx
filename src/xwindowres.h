#pragma once

#ifdef WITH_X

#include <X11/Xlib.h>
#include <X11/Xutil.h>

struct XWindowRes {
public:
	XWindowRes(Display *dpy);
	~XWindowRes();

	Display *const dpy;
	Window win{None};
	XSizeHints *sizeHints{nullptr};
	char *pixels{nullptr};
	XImage *image{nullptr};
};

#endif
