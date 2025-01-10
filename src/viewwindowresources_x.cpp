#if defined(PLATFORM_POSIX) && defined(WITH_X)

#include "viewwindowresources_x.h"

ViewWindowResourcesX::ViewWindowResourcesX(Display *dpy)
    : dpy(dpy)
{
}

ViewWindowResourcesX::~ViewWindowResourcesX()
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
