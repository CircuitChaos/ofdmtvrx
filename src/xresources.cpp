#ifdef WITH_X

#include "xresources.h"

XResources::Win::Win(Display *dpy, Window &win)
    : dpy(dpy), win(win)
{
}

XResources::Win::~Win()
{
	if(image) {
		XDestroyImage(image);
	}

	if(sizeHints) {
		XFree(sizeHints);
	}

	XDestroyWindow(dpy, win);
}

XResources::~XResources()
{
	// TODO should we also free winDelMsg here?

	if(dpy) {
		XCloseDisplay(dpy);
	}
}

#endif
