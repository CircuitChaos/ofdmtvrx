#if defined(PLATFORM_POSIX) && defined(WITH_X)

#include "viewresources_x.h"

ViewResourcesX::~ViewResourcesX()
{
	// TODO should we also free winDelMsg here?

	if(colormap != None) {
		XFreeColormap(dpy, colormap);
	}

	if(dpy) {
		XCloseDisplay(dpy);
	}
}

#endif
