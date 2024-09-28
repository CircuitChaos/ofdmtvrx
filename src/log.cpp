#include <cstdio>
#include <cstdarg>
#include "log.h"
#include "util.h"

Logger logger;

void Logger::setLevel(Level level)
{
	m_level = level;
}

void Logger::operator()(Level level, const std::string &file, int line, const char *fmt, ...)
{
	if(level < m_level) {
		return;
	}

	printf("%s %c ", util::getTS().c_str(), getLevelChar(level));

	if(m_level == LL_DBG) {
		printf("%s:%d ", file.c_str(), line);
	}

	printf("| ");

	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	printf("\n");
}

char Logger::getLevelChar(Level level)
{
	switch(level) {
		case LL_DBG:
			return 'D';

		case LL_VERB:
			return 'V';

		case LL_NORM:
			return 'N';

		case LL_ERR:
			return 'E';
	}

	return '?';
}
