#ifdef WITH_X

#include "xwindowres.h"

XWindowRes::XWindowRes(Display *dpy)
    : dpy(dpy)
{
}

XWindowRes::~XWindowRes()
{
	if(image) {
		/* Frees also imagePixels */
		XDestroyImage(image);
	}

	if(sizeHints) {
		XFree(sizeHints);
	}

	if(win != None) {
		XDestroyWindow(dpy, win);
	}
}

#endif
