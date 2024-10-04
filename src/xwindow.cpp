#ifdef WITH_X

#include <cstring>
#include "xresources.h"
#include "xwindow.h"
#include "throw.h"

XWindow::XWindow(XResources &res, const std::string &name, unsigned width, unsigned height, bool canResize)
    : m_winRes(res.dpy), m_visual(res.vi.visual), m_depth(res.vi.depth), m_width(width), m_height(height)
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

void XWindow::recreateImage()
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

Window XWindow::getWindow() const
{
	return m_winRes.win;
}

unsigned XWindow::getWidth() const
{
	return m_width;
}

unsigned XWindow::getHeight() const
{
	return m_height;
}

void XWindow::setPixel(unsigned x, unsigned y, uint8_t red, uint8_t green, uint8_t blue)
{
	xassert(x < m_width && y < m_height, "Pixel coords out of range (pixel at %ux%u, size is %ux%u)", x, y, m_width, m_height);
	m_winRes.pixels[(y * m_width + x) * 4]     = blue;
	m_winRes.pixels[(y * m_width + x) * 4 + 1] = green;
	m_winRes.pixels[(y * m_width + x) * 4 + 2] = red;
}

void XWindow::setPixel(unsigned x, unsigned y, uint32_t rgb)
{
	xassert(x < m_width && y < m_height, "Pixel coords out of range (pixel at %ux%u, size is %ux%u)", x, y, m_width, m_height);
	m_winRes.pixels[(y * m_width + x) * 4]     = (rgb >> 16) & 0xff;
	m_winRes.pixels[(y * m_width + x) * 4 + 1] = (rgb >> 8) & 0xff;
	m_winRes.pixels[(y * m_width + x) * 4 + 2] = rgb >> 24;
}

void XWindow::drawLine(unsigned x1, unsigned y1, unsigned x2, unsigned y2, uint8_t red, uint8_t green, uint8_t blue)
{
	xassert(x1 < m_width && y1 < m_height, "Line start out of range (point at %ux%u, size is %ux%u)", x1, y1, m_width, m_height);
	xassert(x2 < m_width && y2 < m_height, "Line end out of range (point at %ux%u, size is %ux%u)", x2, y2, m_width, m_height);

	if(abs(x2 - x1) > abs(y2 - y1)) {
		if(x1 > x2) {
			unsigned tmp(x1);
			x1 = x2;
			x2 = tmp;

			tmp = y1;
			y1  = y2;
			y2  = tmp;
		}

		const unsigned dx(x2 - x1);
		int dy(y2 - y1);

		int yi(1);
		if(dy < 0) {
			yi = -1;
			dy = -dy;
		}

		int d(2 * dy - dx);
		unsigned y(y1);

		for(unsigned x(x1); x <= x2; ++x) {
			setPixel(x, y, red, green, blue);

			if(d > 0) {
				y += yi;
				d -= 2 * dx;
			}

			d += 2 * dy;
		}
	}
	else {
		if(y1 > y2) {
			unsigned tmp(x1);
			x1 = x2;
			x2 = tmp;

			tmp = y1;
			y1  = y2;
			y2  = tmp;
		}

		int dx(x2 - x1);
		const unsigned dy(y2 - y1);

		int xi(1);
		if(dx < 0) {
			xi = -1;
			dx = -dx;
		}

		int d(2 * dx - dy);
		unsigned x(x1);

		for(unsigned y(y1); y <= y2; ++y) {
			setPixel(x, y, red, green, blue);

			if(d > 0) {
				x += xi;
				d -= 2 * dy;
			}

			d += 2 * dx;
		}
	}
}

void XWindow::clear()
{
	memset(m_winRes.pixels, 0, m_width * m_height * 4);
}

void XWindow::resize(unsigned width, unsigned height)
{
	m_width  = width;
	m_height = height;
	recreateImage();
	redraw();
}

void XWindow::redraw()
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

#endif
