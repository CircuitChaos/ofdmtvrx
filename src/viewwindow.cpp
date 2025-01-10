#if defined(PLATFORM_WIN) || (defined(PLATFORM_POSIX) && defined(WITH_X))

#include <cstdlib>
#include "viewwindow.h"
#include "throw.h"

ViewWindow::ViewWindow(unsigned width, unsigned height)
    : m_width(width),
      m_height(height)
{
}

ViewWindow::~ViewWindow()
{
}

unsigned ViewWindow::getWidth() const
{
	return m_width;
}

unsigned ViewWindow::getHeight() const
{
	return m_height;
}

void ViewWindow::setPixel(unsigned x, unsigned y, uint32_t rgb)
{
	setPixel(x, y, rgb >> 24, (rgb >> 8) & 0xff, (rgb >> 16) & 0xff);
}

void ViewWindow::drawLine(unsigned x1, unsigned y1, unsigned x2, unsigned y2, uint8_t red, uint8_t green, uint8_t blue)
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

#endif
