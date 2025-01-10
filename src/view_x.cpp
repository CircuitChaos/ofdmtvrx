#if defined(PLATFORM_POSIX) && defined(WITH_X)

#include "view_x.h"
#include "viewwindow_x.h"
#include "throw.h"
#include "decoder_constants.h"
#include "log.h"

ViewX::ViewX()
{
	m_res.dpy = XOpenDisplay(nullptr);
	xassert(m_res.dpy, "Cannot open display; set DISPLAY or use -n to disable X");

	const int screen(DefaultScreen(m_res.dpy));
	xassert(XMatchVisualInfo(m_res.dpy, screen, 24, TrueColor, &m_res.vi) != 0, "This program needs at least 24-bit display; use -n to disable X");

	m_res.winDelMsg = XInternAtom(m_res.dpy, "WM_DELETE_WINDOW", false);
	xassert(m_res.winDelMsg != None, "Could not obtain WM_DELETE_WINDOW atom; use -n to disable X");

	m_res.colormap = XCreateColormap(m_res.dpy, DefaultRootWindow(m_res.dpy), m_res.vi.visual, AllocNone);
	reopenWindows();
}

ViewX::~ViewX()
{
}

int ViewX::getFD() const
{
	return ConnectionNumber(m_res.dpy);
}

void ViewX::readHandler()
{
	while(XPending(m_res.dpy) > 0) {
		XEvent e;
		XNextEvent(m_res.dpy, &e);

		switch(e.type) {
			case Expose: {
				logd("Got Expose event");
				xassert(e.xexpose.type == Expose, "Invalid event type: %d", e.xexpose.type);
				std::unique_ptr<ViewWindow> *w(getWindow(e.xexpose.window));
				if(!w) {
					loge("Got Expose event for nonexisting window. Ignoring, but this probably shouldn't happen");
					break;
				}
				(*w)->redraw();
				break;
			}

			case ConfigureNotify: {
				logd("ConfigureNotify");
				xassert(e.xconfigure.type == ConfigureNotify, "Invalid event type: %d", e.xconfigure.type);
				std::unique_ptr<ViewWindow> *w(getWindow(e.xconfigure.window));
				if(!w) {
					loge("Got ConfigureNotify event for nonexisting window. Ignoring, but this probably shouldn't happen");
					break;
				}

				if((unsigned) e.xconfigure.width != (*w)->getWidth() || (unsigned) e.xconfigure.height != (*w)->getHeight()) {
					logd("Window resized");

					if(*w != m_oscilloscope) {
						xthrow("Non-oscilloscope window resized, this shouldn't happen");
					}

					(*w)->resize(e.xconfigure.width, e.xconfigure.height);
				}

				(*w)->redraw();
				break;
			}

			case MapNotify: {
				logd("Got MapNotify event");
				xassert(e.xmap.type == MapNotify, "Invalid event type: %d", e.xmap.type);
				std::unique_ptr<ViewWindow> *w(getWindow(e.xmap.window));
				if(!w) {
					loge("Got MapNotify event for nonexisting window. Ignoring, but this probably shouldn't happen");
					break;
				}
				(*w)->redraw();
				break;
			}

			case ClientMessage:
				logd("Got ClientMessage event");
				xassert(e.xclient.type == ClientMessage, "Invalid event type: %d", e.xclient.type);
				if((Atom) e.xclient.data.l[0] == m_res.winDelMsg) {
					std::unique_ptr<ViewWindow> *w(getWindow(e.xclient.window));
					if(!w) {
						/* This can happen if there's a lag and user clicks X more than once before we handle the first message */
						logd("Got ClientMessage window close event, but window is already closed");
						break;
					}

					logd("Window was closed");
					w->reset(nullptr);
				}
				else {
					logd("Ignoring ClientMessage event as atom isn't known");
				}
				break;

			default:
				logd("Got unsupported X event %d, ignoring", e.type);
				break;
		}
	}
}

void ViewX::reopenWindows()
{
	if(!m_oscilloscope) {
		m_oscilloscope.reset(new ViewWindowX(decoder_constants::oscilloscope_initial_width, decoder_constants::oscilloscope_initial_height, "Oscilloscope", true, m_res));
	}

	if(!m_spectrum) {
		m_spectrum.reset(new ViewWindowX(decoder_constants::spectrum_width, decoder_constants::spectrum_height + decoder_constants::spectrogram_height, "Spectrum", false, m_res));
	}

	if(!m_constellation) {
		m_constellation.reset(new ViewWindowX(decoder_constants::constellation_width * 2, (decoder_constants::constellation_height + decoder_constants::peak_meter_height) * 2, "Constellation", false, m_res));
	}
}

std::unique_ptr<ViewWindow> *ViewX::getWindow(Window w)
{
	if(m_oscilloscope && dynamic_cast<ViewWindowX *>(m_oscilloscope.get())->getWindow() == w) {
		return &m_oscilloscope;
	}

	if(m_spectrum && dynamic_cast<ViewWindowX *>(m_spectrum.get())->getWindow() == w) {
		return &m_spectrum;
	}

	if(m_constellation && dynamic_cast<ViewWindowX *>(m_constellation.get())->getWindow() == w) {
		return &m_constellation;
	}

	return nullptr;
}

#endif
