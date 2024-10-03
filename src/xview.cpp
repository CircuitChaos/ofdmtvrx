#include "xview.h"

#ifndef WITH_X

XView::XView()
{
}

int XView::getFD() const
{
	return -1;
}

void XView::readHandler()
{
}

void XView::update(const std::vector<uint32_t> & /* spectrum */, const std::vector<uint32_t> & /* spectrogram */, const std::vector<uint32_t> & /* constellation */, const std::vector<uint32_t> & /* peakMeter */, const std::vector<int16_t> & /* audioBuffer */)
{
}

#else

#include <cstring>
#include "decoder_constants.h"
#include "throw.h"
#include "log.h"

// TODO add scale for all meters
// TODO allow scale to be floating-point (some scaler)
static const unsigned PEAK_METER_SCALE            = 8;
static const unsigned OSCILLOSCOPE_INITIAL_WIDTH  = 640;
static const unsigned OSCILLOSCOPE_INITIAL_HEIGHT = 240;

void XView::createWindow(std::unique_ptr<XResources::Win> &win, const std::string &name, unsigned width, unsigned height, XSetWindowAttributes *attrs, bool preventResizing)
{
	// TODO do something with the fact that all windows are created on the same position

	Window w = XCreateWindow(
	    m_res.dpy,                                // display
	    DefaultRootWindow(m_res.dpy),             // parent
	    0, 0,                                     // initial position
	    width, height,                            // size
	    0,                                        // border width
	    m_res.vi.depth,                           // depth
	    InputOutput,                              // class
	    m_res.vi.visual,                          // visual
	    CWColormap | CWBackPixel | CWBorderPixel, // value mask
	    attrs);                                   // attributes

	xassert(w, "Could not create window");
	win.reset(new XResources::Win(m_res.dpy, w));

	XSetWMProtocols(m_res.dpy, win->win, &m_res.winDelMsg, 1);
	XStoreName(m_res.dpy, win->win, name.c_str());
	XSelectInput(m_res.dpy, win->win, ExposureMask | StructureNotifyMask);

	if(preventResizing) {
		win->sizeHints = XAllocSizeHints();
		xassert(win->sizeHints, "Could not allocate size hints");

		memset(win->sizeHints, 0, sizeof(XSizeHints));
		win->sizeHints->flags     = PMinSize | PMaxSize;
		win->sizeHints->min_width = win->sizeHints->max_width = width;
		win->sizeHints->min_height = win->sizeHints->max_height = height;

		XSetWMNormalHints(m_res.dpy, win->win, win->sizeHints);
	}

	XWindowAttributes a;
	XGetWindowAttributes(m_res.dpy, win->win, &a);
	xassert((unsigned) a.width == width && (unsigned) a.height == height, "X created a window (%s) with a wrong size (expected %ux%u, got %dx%d); use -n to disable X",
	    name.c_str(), width, height, a.width, a.height);

	win->width  = a.width;
	win->height = a.height;

	createImage(win.get());
	XMapWindow(m_res.dpy, win->win);
}

void XView::createImage(XResources::Win *win)
{
	if(win->image) {
		XDestroyImage(win->image);
	}

	// TODO check sanity of these values
	logd("Resizing image to %ux%u", win->width, win->height);
	win->imagePixels = (char *) malloc(win->width * win->height * 4);
	xassert(win->imagePixels, "Could not allocate image; use -n to disable X");
	memset(win->imagePixels, 0, win->width * win->height * 4);

	win->image = XCreateImage(
	    m_res.dpy,        // display
	    m_res.vi.visual,  // visual
	    m_res.vi.depth,   // depth
	    ZPixmap,          // format
	    0,                // offset
	    win->imagePixels, // data
	    win->width,       // width
	    win->height,      // height
	    32,               // bitmap_pad (what's this?)
	    0);               // bytes per line

	xassert(win->image, "Could not create image; use -n to disable X");
}

XView::XView()
{
	m_res.dpy = XOpenDisplay(nullptr);
	xassert(m_res.dpy, "Cannot open display; set DISPLAY or use -n to disable X");
	m_fd = ConnectionNumber(m_res.dpy);

	const int screen(DefaultScreen(m_res.dpy));
	xassert(XMatchVisualInfo(m_res.dpy, screen, 24, TrueColor, &m_res.vi) != 0, "This program needs at least 24-bit display; use -n to disable X");

	m_res.winDelMsg = XInternAtom(m_res.dpy, "WM_DELETE_WINDOW", false);
	xassert(m_res.winDelMsg != None, "Could not obtain WM_DELETE_WINDOW atom; use -n to disable X");

	XSetWindowAttributes attrs;
	attrs.colormap         = XCreateColormap(m_res.dpy, DefaultRootWindow(m_res.dpy), m_res.vi.visual, AllocNone);
	attrs.background_pixel = 0;
	attrs.border_pixel     = 0;

	createWindow(m_res.spectrumWin, "Spectrum", decoder_constants::spectrum_width, decoder_constants::spectrum_height, &attrs, true);
	createWindow(m_res.spectrogramWin, "Spectrogram", decoder_constants::spectrogram_width, decoder_constants::spectrogram_height, &attrs, true);
	createWindow(m_res.constellationWin, "Constellation", decoder_constants::constellation_width, decoder_constants::constellation_height, &attrs, true);
	createWindow(m_res.peakMeterWin, "Peak meter", decoder_constants::peak_meter_width * PEAK_METER_SCALE, PEAK_METER_SCALE, &attrs, true);
	createWindow(m_res.oscilloscopeWin, "Oscilloscope", OSCILLOSCOPE_INITIAL_WIDTH, OSCILLOSCOPE_INITIAL_HEIGHT, &attrs, false);
}

int XView::getFD() const
{
	return m_fd;
}

void XView::readHandler()
{
	while(XPending(m_res.dpy) > 0) {
		XEvent e;
		XNextEvent(m_res.dpy, &e);

		logd("Got X event type %d", e.type);
		switch(e.type) {
			case Expose:
				logd("Expose -- redrawing");
				xassert(e.xexpose.type == Expose, "Invalid event type: %d", e.xexpose.type);
				redraw(getWindow(e.xexpose.window));
				break;

			case ConfigureNotify: {
				logd("ConfigureNotify");
				xassert(e.xconfigure.type == ConfigureNotify, "Invalid event type: %d", e.xconfigure.type);

				XResources::Win *win(getWindow(e.xconfigure.window));
				xassert(win, "ConfigureNotify for nonexisting window");

				if((unsigned) e.xconfigure.width != win->width || (unsigned) e.xconfigure.height != win->height) {
					logd("Window resized");
					// TODO check if m_res.oscilloscopeWin is not null?
					if(win != m_res.oscilloscopeWin.get()) {
						xthrow("Non-oscilloscope window resized, this shouldn't happen");
					}

					// TODO check sanity of these values
					win->width  = e.xconfigure.width;
					win->height = e.xconfigure.height;
					createImage(win);
				}

				redraw(getWindow(e.xconfigure.window));
				break;
			}

			case MapNotify:
				logd("MapNotify -- redrawing");
				xassert(e.xmap.type == MapNotify, "Invalid event type: %d", e.xmap.type);
				redraw(getWindow(e.xmap.window));
				break;

			case ClientMessage:
				logd("ClientMessage");
				xassert(e.xclient.type == ClientMessage, "Invalid event type: %d", e.xclient.type);
				// TODO allow closing the window
				break;

			default:
				logd("Ignoring this event");
				break;
		}
	}
}

void XView::update(const std::vector<uint32_t> &spectrum, const std::vector<uint32_t> &spectrogram, const std::vector<uint32_t> &constellation, const std::vector<uint32_t> &peakMeter, const std::vector<int16_t> &audioBuffer)
{
#if 0
	std::set<uint32_t> values;
	for(std::vector<uint32_t>::const_iterator i(peakMeter.begin()); i != peakMeter.end(); ++i) {
		if(values.find(*i) == values.end()) {
			values.insert(*i);
		}
	}

	logd("Dumping %zu values", values.size());
	for(std::set<uint32_t>::const_iterator i(values.begin()); i != values.end(); ++i) {
		logd("Found value: 0x%08x", *i);
	}
#endif

	// TODO peak meter doesn't look quite right
	copyImageMono(m_res.spectrumWin.get(), spectrum, 1);
	copyImageColor(m_res.spectrogramWin.get(), spectrogram, 1);
	copyImageMono(m_res.constellationWin.get(), constellation, 1);
	copyImageColor(m_res.peakMeterWin.get(), peakMeter, PEAK_METER_SCALE);
	beautifySpectrum();
	createOscilloscope(audioBuffer);

	redraw(m_res.spectrumWin.get());
	redraw(m_res.spectrogramWin.get());
	redraw(m_res.constellationWin.get());
	redraw(m_res.peakMeterWin.get());
	redraw(m_res.oscilloscopeWin.get());
}

// TODO this might not work if window size is not a multiply of scale
void XView::copyImageColor(XResources::Win *win, const std::vector<uint32_t> &src, unsigned scale)
{
	xassert(win, "copyImage() called on nonexisting window");
	xassert((win->width / scale) * (win->height / scale) == src.size(), "Window and image sizes don't match");

	for(unsigned dsty(0); dsty < win->height; ++dsty) {
		const unsigned srcy(dsty / scale);
		for(unsigned dstx(0); dstx < win->width; ++dstx) {
			const unsigned srcx(dstx / scale);
			const uint32_t pixel(src[srcy * (win->width / scale) + srcx]);

			// Our pixels:
			// 0: blue
			// 1: green
			// 2: red
			// 3: unused?

			win->imagePixels[(dsty * win->width + dstx) * 4]     = (pixel >> 24) & 0xff;
			win->imagePixels[(dsty * win->width + dstx) * 4 + 1] = (pixel >> 8) & 0xff;
			win->imagePixels[(dsty * win->width + dstx) * 4 + 2] = (pixel >> 16) & 0xff;
		}
	}
}

void XView::copyImageMono(XResources::Win *win, const std::vector<uint32_t> &src, unsigned scale)
{
	xassert(win, "copyImage() called on nonexisting window");
	xassert((win->width / scale) * (win->height / scale) == src.size(), "Window and image sizes don't match");

	for(unsigned dsty(0); dsty < win->height; ++dsty) {
		const unsigned srcy(dsty / scale);
		for(unsigned dstx(0); dstx < win->width; ++dstx) {
			const unsigned srcx(dstx / scale);
			const uint32_t pixel(src[srcy * (win->width / scale) + srcx]);
			win->imagePixels[(dsty * win->width + dstx) * 4]     = pixel >> 24;
			win->imagePixels[(dsty * win->width + dstx) * 4 + 1] = pixel >> 24;
			win->imagePixels[(dsty * win->width + dstx) * 4 + 2] = pixel >> 24;
		}
	}
}

void XView::createOscilloscope(const std::vector<int16_t> &audioBuffer)
{
	// TODO
	// - what if each size < each resolution, draw lines?
	// - what if each size > each resolution, average?
}

void XView::beautifySpectrum()
{
	// TODO check if window is not null
	for(unsigned x(0); x < m_res.spectrumWin->width; ++x) {
		bool drawing(false);
		for(unsigned y(0); y < m_res.spectrumWin->height; ++y) {
			if(m_res.spectrumWin->imagePixels[(y * m_res.spectrumWin->width + x) * 4]) {
				drawing = true;
			}

			if(!m_res.spectrumWin->imagePixels[(y * m_res.spectrumWin->width + x) * 4] && drawing) {
				/* subtle blue tint */
				// TODO maybe make this blue tint go darker the lower we are
				m_res.spectrumWin->imagePixels[(y * m_res.spectrumWin->width + x) * 4] = 0x40;
			}
		}
	}
}

void XView::redraw(XResources::Win *win)
{
	xassert(win, "redraw() called on nonexisting window");
	XPutImage(
	    m_res.dpy,                                      // display
	    win->win,                                       // window
	    DefaultGC(m_res.dpy, DefaultScreen(m_res.dpy)), // GC
	    win->image,                                     // image
	    0, 0,                                           // src_x, src_y,
	    0, 0,                                           // dest_x, dest_y,
	    win->width, win->height);                       // width, height
}

XResources::Win *XView::getWindow(Window w)
{
	if(m_res.spectrumWin.get() && m_res.spectrumWin->win == w) {
		return m_res.spectrumWin.get();
	}

	if(m_res.spectrogramWin.get() && m_res.spectrogramWin->win == w) {
		return m_res.spectrogramWin.get();
	}

	if(m_res.constellationWin.get() && m_res.constellationWin->win == w) {
		return m_res.constellationWin.get();
	}

	if(m_res.peakMeterWin.get() && m_res.peakMeterWin->win == w) {
		return m_res.peakMeterWin.get();
	}

	if(m_res.oscilloscopeWin.get() && m_res.oscilloscopeWin->win == w) {
		return m_res.oscilloscopeWin.get();
	}

	return nullptr;
}

#endif
