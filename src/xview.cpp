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

void XView::reopenWindows()
{
}

#else

#include <cstring>
#include "decoder_constants.h"
#include "throw.h"
#include "log.h"

static const unsigned OSCILLOSCOPE_INITIAL_WIDTH  = 640;
static const unsigned OSCILLOSCOPE_INITIAL_HEIGHT = 240;
static const unsigned PEAK_METER_HEIGHT           = 16;

XView::XView()
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

int XView::getFD() const
{
	return ConnectionNumber(m_res.dpy);
}

void XView::readHandler()
{
	while(XPending(m_res.dpy) > 0) {
		XEvent e;
		XNextEvent(m_res.dpy, &e);

		switch(e.type) {
			case Expose: {
				logd("Got Expose event");
				xassert(e.xexpose.type == Expose, "Invalid event type: %d", e.xexpose.type);
				std::unique_ptr<XWindow> *w(getWindow(e.xexpose.window));
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
				std::unique_ptr<XWindow> *w(getWindow(e.xconfigure.window));
				if(!w) {
					loge("Got ConfigureNotify event for nonexisting window. Ignoring, but this probably shouldn't happen");
					break;
				}

				if((unsigned) e.xconfigure.width != (*w)->getWidth() || (unsigned) e.xconfigure.height != (*w)->getHeight()) {
					logd("Window resized");

					if(*w != m_res.oscilloscope) {
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
				std::unique_ptr<XWindow> *w(getWindow(e.xmap.window));
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
					std::unique_ptr<XWindow> *w(getWindow(e.xclient.window));
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

void XView::update(const std::vector<uint32_t> &spectrum, const std::vector<uint32_t> &spectrogram, const std::vector<uint32_t> &constellation, const std::vector<uint32_t> &peakMeter, const std::vector<int16_t> &audioBuffer)
{
	if(m_res.oscilloscope) {
		createOscilloscope(audioBuffer);
		m_res.oscilloscope->redraw();
	}

	if(m_res.spectrum) {
		createSpectrum(spectrum, spectrogram);
		m_res.spectrum->redraw();
	}

	if(m_res.constellation) {
		createConstellation(constellation, peakMeter);
		m_res.constellation->redraw();
	}
}

void XView::createOscilloscope(const std::vector<int16_t> &audioBuffer)
{
	m_res.oscilloscope->clear();

	const unsigned width(m_res.oscilloscope->getWidth());
	const unsigned height(m_res.oscilloscope->getHeight());
	unsigned oldx(width), oldy(height);

	for(size_t i(0); i < audioBuffer.size(); ++i) {
		const unsigned x((double) i * m_res.oscilloscope->getWidth() / audioBuffer.size());
		const uint16_t sample(65535 - (audioBuffer[i] + 32768));
		const unsigned y((double) sample * m_res.oscilloscope->getHeight() / 65535);

		/* Can it happen? */
		if(x >= m_res.oscilloscope->getWidth() || y >= m_res.oscilloscope->getHeight()) {
			continue;
		}

		if(oldx != width) {
			m_res.oscilloscope->drawLine(oldx, oldy, x, y, 0xff, 0xff, 0xff);
		}

		oldx = x;
		oldy = y;
	}
}

void XView::createSpectrum(const std::vector<uint32_t> &spectrum, const std::vector<uint32_t> &spectrogram)
{
	for(unsigned x(0); x < decoder_constants::spectrum_width; ++x) {
		bool drawingBlue(false);
		for(unsigned y(0); y < decoder_constants::spectrum_height; ++y) {
			if(spectrum[y * decoder_constants::spectrum_width + x]) {
				m_res.spectrum->setPixel(x, y, 0xff, 0xff, 0xff);
				drawingBlue = true;
			}
			else if(drawingBlue) {
				m_res.spectrum->setPixel(x, y, 0, 0, 0x30);
			}
			else {
				m_res.spectrum->setPixel(x, y, 0, 0, 0);
			}
		}
	}

	for(unsigned y(0); y < decoder_constants::spectrogram_height; ++y) {
		for(unsigned x(0); x < decoder_constants::spectrogram_width; ++x) {
			m_res.spectrum->setPixel(x, y + decoder_constants::spectrum_height, spectrogram[y * decoder_constants::spectrogram_width + x]);
		}
	}
}

void XView::createConstellation(const std::vector<uint32_t> &constellation, const std::vector<uint32_t> &peakMeter)
{
	for(unsigned y(0); y < decoder_constants::constellation_height; ++y) {
		for(unsigned x(0); x < decoder_constants::constellation_width; ++x) {
			const uint8_t rgb(constellation[y * decoder_constants::constellation_width + x] ? 0xff : 0);
			m_res.constellation->setPixel(x, y, rgb, rgb, rgb);
		}
	}

	for(unsigned y(0); y < PEAK_METER_HEIGHT; ++y) {
		for(unsigned x(0); x < decoder_constants::constellation_width; ++x) {
			m_res.constellation->setPixel(x, y + decoder_constants::constellation_height, peakMeter[x / (decoder_constants::constellation_width / decoder_constants::peak_meter_width)]);
		}
	}
}

void XView::reopenWindows()
{
	if(!m_res.oscilloscope) {
		m_res.oscilloscope.reset(new XWindow(m_res, "Oscilloscope", OSCILLOSCOPE_INITIAL_WIDTH, OSCILLOSCOPE_INITIAL_HEIGHT, true));
	}

	if(!m_res.spectrum) {
		m_res.spectrum.reset(new XWindow(m_res, "Spectrum", decoder_constants::spectrum_width, decoder_constants::spectrum_height + decoder_constants::spectrogram_height, false));
	}

	if(!m_res.constellation) {
		m_res.constellation.reset(new XWindow(m_res, "Constellation", decoder_constants::constellation_width, decoder_constants::constellation_height + PEAK_METER_HEIGHT, false));
	}
}

std::unique_ptr<XWindow> *XView::getWindow(Window w)
{
	if(m_res.oscilloscope && m_res.oscilloscope->getWindow() == w) {
		return &m_res.oscilloscope;
	}

	if(m_res.spectrum && m_res.spectrum->getWindow() == w) {
		return &m_res.spectrum;
	}

	if(m_res.constellation && m_res.constellation->getWindow() == w) {
		return &m_res.constellation;
	}

	return nullptr;
}

#endif
