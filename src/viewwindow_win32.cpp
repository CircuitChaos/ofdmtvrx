#if defined(PLATFORM_WIN)

#include "viewwindow_win32.h"
#include <cstring>
#include "throw.h"

ViewWindowWin32::ViewWindowWin32(unsigned width, unsigned height, const std::string &name, bool canResize)
    : ViewWindow(width, height), m_canResize(canResize)
{
	m_pixels.resize(width * height * 3);

	m_bmpHeader.biSize          = sizeof(m_bmpHeader);
	m_bmpHeader.biWidth         = width;
	m_bmpHeader.biHeight        = -height;
	m_bmpHeader.biPlanes        = 1;
	m_bmpHeader.biBitCount      = 24;
	m_bmpHeader.biCompression   = BI_RGB;
	m_bmpHeader.biSizeImage     = 0;
	m_bmpHeader.biXPelsPerMeter = 0;
	m_bmpHeader.biYPelsPerMeter = 0;
	m_bmpHeader.biClrUsed       = 0;
	m_bmpHeader.biClrImportant  = 0;

	WNDCLASSEX wndclass    = {};
	wndclass.cbSize        = sizeof(WNDCLASSEX);
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = StaticWindowProc;
	wndclass.hInstance     = GetModuleHandle(0);
	wndclass.hIcon         = 0;
	wndclass.hCursor       = 0;
	wndclass.lpszClassName = name.c_str();

	xassert(RegisterClassEx(&wndclass), "RegisterClassEx() failed");

	RECT windowRect = {0, 0, (LONG) width, (LONG) height};
	DWORD dwStyle   = WS_OVERLAPPEDWINDOW;
	if(!m_canResize) {
		dwStyle &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
	}

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, 0);

	m_hwnd = CreateWindow(name.c_str(), name.c_str(), dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, GetModuleHandle(nullptr), this);
	ShowWindow(m_hwnd, SW_SHOWDEFAULT);
	UpdateWindow(m_hwnd);
	recreateImage();
	redraw(); // TODO needed?
}

ViewWindowWin32::~ViewWindowWin32()
{
}

LRESULT CALLBACK ViewWindowWin32::StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ViewWindowWin32 *targetWindow = nullptr;
	if(message == WM_NCCREATE) {
		CREATESTRUCT *cs = (CREATESTRUCT *) lParam;
		targetWindow     = (ViewWindowWin32 *) cs->lpCreateParams;

		SetLastError(0);
		if(SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) targetWindow) == 0 && GetLastError() != 0) {
			return FALSE;
		}
	}
	else {
		targetWindow = (ViewWindowWin32 *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if(targetWindow && (targetWindow->m_hwnd == hwnd)) {
			return targetWindow->WindowProc(message, wParam, lParam);
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT ViewWindowWin32::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
		case WM_PAINT: {
			HDC wndDC = BeginPaint(m_hwnd, &m_paintStruct);
			SetDIBitsToDevice(wndDC, 0, 0, m_width, m_height, 0, 0, 0, m_height, &m_pixels[0], reinterpret_cast<BITMAPINFO *>(&m_bmpHeader), DIB_RGB_COLORS);
			EndPaint(m_hwnd, &m_paintStruct);
			break;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			break;
	}

	return DefWindowProc(m_hwnd, message, wParam, lParam);
}

void ViewWindowWin32::setPixel(unsigned x, unsigned y, uint8_t red, uint8_t green, uint8_t blue)
{
	xassert(x < m_width && y < m_height, "Pixel coords out of range (pixel at %ux%u, size is %ux%u)", x, y, m_width, m_height);
	m_pixels[(y * m_width + x) * 3]     = blue;
	m_pixels[(y * m_width + x) * 3 + 1] = green;
	m_pixels[(y * m_width + x) * 3 + 2] = red;
}

void ViewWindowWin32::clear()
{
	memset(&m_pixels[0], 0, m_width * m_height * 3);
}

void ViewWindowWin32::resize(unsigned width, unsigned height)
{
	m_width  = width;
	m_height = height;
	recreateImage();
	redraw();
}

void ViewWindowWin32::redraw()
{
	InvalidateRect(m_hwnd, nullptr, true);
}

void ViewWindowWin32::recreateImage()
{
	m_bmpHeader.biWidth  = m_width;
	m_bmpHeader.biHeight = -m_height;
	m_pixels.resize(m_width * m_height * 3);
}

#endif
