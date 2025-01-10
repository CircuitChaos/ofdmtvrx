#if defined(PLATFORM_WIN)

#include <windows.h>
#include "view_win32.h"
#include "viewwindow_win32.h"
#include "throw.h"
#include "decoder_constants.h"
#include "log.h"

ViewWin32::ViewWin32()
{
	reopenWindows();
}

ViewWin32::~ViewWin32()
{
}

int ViewWin32::getFD() const
{
	xassert(false, "getFD() shouldn't be called in Win32 version");
	/* NOTREACHED */
	return -1;
}

void ViewWin32::readHandler()
{
	MSG msg;
	while(PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE)) {
		GetMessage(&msg, nullptr, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void ViewWin32::reopenWindows()
{
	if(!m_oscilloscope) {
		m_oscilloscope.reset(new ViewWindowWin32(decoder_constants::oscilloscope_initial_width, decoder_constants::oscilloscope_initial_height, "Oscilloscope", true));
	}

	if(!m_spectrum) {
		m_spectrum.reset(new ViewWindowWin32(decoder_constants::spectrum_width, decoder_constants::spectrum_height + decoder_constants::spectrogram_height, "Spectrum", false));
	}

	if(!m_constellation) {
		m_constellation.reset(new ViewWindowWin32(decoder_constants::constellation_width * 2, (decoder_constants::constellation_height + decoder_constants::peak_meter_height) * 2, "Constellation", false));
	}
}

#endif
