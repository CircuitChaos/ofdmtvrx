#pragma once

#if defined(PLATFORM_POSIX) && defined(WITH_X)

#include "view.h"
#include "viewwindow.h"
#include "viewresources_x.h"

class ViewX : public View {
public:
	ViewX();
	virtual ~ViewX();

	virtual int getFD() const;
	virtual void readHandler();
	virtual void reopenWindows();

	std::unique_ptr<ViewWindow> *getWindow(Window w);

private:
	ViewResourcesX m_res;
};

#endif
