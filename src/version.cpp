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
#if (defined(PLATFORM_POSIX) && defined(WITH_X)) || defined(PLATFORM_WIN)
	return true;
#else
	return false;
#endif
}
