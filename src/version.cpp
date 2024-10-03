#include "version.h"

#define TO_STR2(x) #x
#define TO_STR(x)  TO_STR2(x)

std::string version::getVersion()
{
	return std::string("git/") + TO_STR(GIT_HASH);
}

std::string version::getBuild()
{
	return std::string(__DATE__) + " " + __TIME__;
}

bool version::withX()
{
#ifdef WITH_X
	return true;
#else
	return false;
#endif
}
