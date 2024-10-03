#pragma once

#ifdef WITH_X

#include <memory>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <vector>

struct XResources {
	struct Win {
		Win(Display *dpy, Window &win);
		~Win();

		Display *const dpy;
		Window win;
		unsigned width{0};
		unsigned height{0};
		XSizeHints *sizeHints{nullptr};
		char *imagePixels{nullptr}; // this is freed by XDestroyImage
		XImage *image{nullptr};
	};

	~XResources();

	Display *dpy{nullptr};
	Atom winDelMsg{None};
	std::unique_ptr<Win> spectrumWin;
	std::unique_ptr<Win> spectrogramWin;
	std::unique_ptr<Win> constellationWin;
	std::unique_ptr<Win> peakMeterWin;
	std::unique_ptr<Win> oscilloscopeWin;
	XVisualInfo vi;
};

#endif
