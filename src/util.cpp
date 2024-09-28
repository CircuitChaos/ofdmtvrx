#include <ctime>
#include <cstdio>
#include <cstdarg>
#include <stdexcept>
#include "util.h"

std::string util::getTS()
{
	const time_t t(time(NULL));

	struct tm tm;
	localtime_r(&t, &tm);

	char buf[64];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);

	return buf;
}

std::string util::format(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	char *p;
	if(vasprintf(&p, fmt, ap) == -1) {
		throw std::runtime_error("Memory allocation error (format)");
	}

	va_end(ap);

	std::string s(p);
	free(p);

	return s;
}
