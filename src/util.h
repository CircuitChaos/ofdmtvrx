#pragma once

#include <string>

namespace util {

std::string getTS();
std::string format(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

} // namespace util
