#pragma once

#if defined(PLATFORM_WIN) || (defined(PLATFORM_POSIX) && defined(WITH_X))

#include <cinttypes>

class ViewWindow {
public:
	ViewWindow(unsigned width, unsigned height);
	virtual ~ViewWindow();

	unsigned getWidth() const;
	unsigned getHeight() const;

	void setPixel(unsigned x, unsigned y, uint32_t rgb);
	void drawLine(unsigned x1, unsigned y1, unsigned x2, unsigned y2, uint8_t red, uint8_t green, uint8_t blue);

	virtual void setPixel(unsigned x, unsigned y, uint8_t red, uint8_t green, uint8_t blue) = 0;
	virtual void clear()                                                                    = 0;
	virtual void resize(unsigned width, unsigned height)                                    = 0;
	virtual void redraw()                                                                   = 0;

protected:
	unsigned m_width;
	unsigned m_height;
};

#endif
