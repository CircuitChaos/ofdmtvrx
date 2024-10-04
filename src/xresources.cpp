#ifdef WITH_X

#include "xresources.h"

XResources::~XResources()
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
