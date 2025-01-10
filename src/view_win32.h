#pragma once

#if defined(PLATFORM_WIN)

#include "view.h"
#include "viewwindow.h"

class ViewWin32 : public View {
public:
	ViewWin32();
	virtual ~ViewWin32();

	virtual int getFD() const;
	virtual void readHandler();
	virtual void reopenWindows();
};

#endif
