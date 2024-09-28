#pragma once

#include <string>

class Logger {
public:
	enum Level {
		LL_DBG,
		LL_VERB,
		LL_NORM,
		LL_ERR,
	};

	void setLevel(Level level);
	void operator()(Level level, const std::string &file, int line, const char *fmt, ...) __attribute__((format(printf, 5, 6)));

private:
	Level m_level{LL_NORM};

	static char getLevelChar(Level level);
};

extern Logger logger;

#define logx(level, ...)                                \
	do {                                                \
		logger(level, __FILE__, __LINE__, __VA_ARGS__); \
	} while(0)

#define logd(...) logx(Logger::LL_DBG, __VA_ARGS__)
#define logv(...) logx(Logger::LL_VERB, __VA_ARGS__)
#define logn(...) logx(Logger::LL_NORM, __VA_ARGS__)
#define loge(...) logx(Logger::LL_ERR, __VA_ARGS__)
