#if defined(PLATFORM_POSIX) && defined(WITH_X)

#include "viewwindow_x.h"
#include <cstring>
#include "throw.h"

ViewWindowX::ViewWindowX(unsigned width, unsigned height, const std::string &name, bool canResize, ViewResourcesX &res)
    : ViewWindow(width, height), m_winRes(res.dpy), m_visual(res.vi.visual), m_depth(res.vi.depth)
{
	// TODO do something with the fact that all windows are created on the same position

	XSetWindowAttributes attrs;
	attrs.colormap         = res.colormap;
	attrs.background_pixel = 0;
	attrs.border_pixel     = 0;

	m_winRes.win = XCreateWindow(
	    res.dpy,                                  // display
	    DefaultRootWindow(res.dpy),               // parent
	    0, 0,                                     // initial position
	    width, height,                            // size
	    0,                                        // border width
	    m_depth,                                  // depth
	    InputOutput,                              // class
	    m_visual,                                 // visual
	    CWColormap | CWBackPixel | CWBorderPixel, // value mask
	    &attrs);                                  // attributes

	xassert(m_winRes.win, "Could not create window");

	XSetWMProtocols(res.dpy, m_winRes.win, &res.winDelMsg, 1);
	XStoreName(res.dpy, m_winRes.win, name.c_str());
	XSelectInput(res.dpy, m_winRes.win, ExposureMask | StructureNotifyMask);

	if(!canResize) {
		m_winRes.sizeHints = XAllocSizeHints();
		xassert(m_winRes.sizeHints, "Could not allocate size hints");

		memset(m_winRes.sizeHints, 0, sizeof(XSizeHints));
		m_winRes.sizeHints->flags     = PMinSize | PMaxSize;
		m_winRes.sizeHints->min_width = m_winRes.sizeHints->max_width = width;
		m_winRes.sizeHints->min_height = m_winRes.sizeHints->max_height = height;

		XSetWMNormalHints(res.dpy, m_winRes.win, m_winRes.sizeHints);
	}

	XWindowAttributes a;
	XGetWindowAttributes(res.dpy, m_winRes.win, &a);
	xassert((unsigned) a.width == width && (unsigned) a.height == height, "X created a window (%s) with a wrong size (expected %ux%u, got %dx%d); use -n to disable X",
	    name.c_str(), width, height, a.width, a.height);

	recreateImage();
	XMapWindow(res.dpy, m_winRes.win);
}

ViewWindowX::~ViewWindowX()
{
}

void ViewWindowX::setPixel(unsigned x, unsigned y, uint8_t red, uint8_t green, uint8_t blue)
{
	xassert(x < m_width && y < m_height, "Pixel coords out of range (pixel at %ux%u, size is %ux%u)", x, y, m_width, m_height);
	m_winRes.pixels[(y * m_width + x) * 4]     = blue;
	m_winRes.pixels[(y * m_width + x) * 4 + 1] = green;
	m_winRes.pixels[(y * m_width + x) * 4 + 2] = red;
}

void ViewWindowX::clear()
{
	memset(m_winRes.pixels, 0, m_width * m_height * 4);
}

void ViewWindowX::resize(unsigned width, unsigned height)
{
	m_width  = width;
	m_height = height;
	recreateImage();
	redraw();
}

void ViewWindowX::redraw()
{
	XPutImage(
	    m_winRes.dpy,                                         // display
	    m_winRes.win,                                         // window
	    DefaultGC(m_winRes.dpy, DefaultScreen(m_winRes.dpy)), // GC
	    m_winRes.image,                                       // image
	    0, 0,                                                 // src_x, src_y,
	    0, 0,                                                 // dest_x, dest_y,
	    m_width, m_height);                                   // width, height
}

Window ViewWindowX::getWindow() const
{
	return m_winRes.win;
}

void ViewWindowX::recreateImage()
{
	if(m_winRes.image) {
		XDestroyImage(m_winRes.image);
		m_winRes.image = nullptr;
	}

	// TODO check sanity of these values
	m_winRes.pixels = (char *) malloc(m_width * m_height * 4);
	xassert(m_winRes.pixels, "Could not allocate image; use -n to disable X");
	clear();

	m_winRes.image = XCreateImage(
	    m_winRes.dpy,    // display
	    m_visual,        // visual
	    m_depth,         // depth
	    ZPixmap,         // format
	    0,               // offset
	    m_winRes.pixels, // data
	    m_width,         // width
	    m_height,        // height
	    32,              // bitmap_pad (what's this?)
	    0);              // bytes per line

	xassert(m_winRes.image, "Could not create image; use -n to disable X");
}

#endif
