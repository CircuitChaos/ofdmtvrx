#pragma once

#if defined(PLATFORM_WIN)

#include <string>
#include <vector>
#include <windows.h>
#include "viewwindow.h"

class ViewWindowWin32 : public ViewWindow {
public:
	ViewWindowWin32(unsigned width, unsigned height, const std::string &name, bool canResize);
	virtual ~ViewWindowWin32();

	virtual void setPixel(unsigned x, unsigned y, uint8_t red, uint8_t green, uint8_t blue);
	virtual void clear();
	virtual void resize(unsigned width, unsigned height);
	virtual void redraw();

	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	const bool m_canResize;
	HWND m_hwnd;
	std::vector<BYTE> m_pixels;
	PAINTSTRUCT m_paintStruct{};
	BITMAPINFOHEADER m_bmpHeader{};

	void recreateImage();
};

#endif
