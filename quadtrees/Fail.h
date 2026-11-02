#pragma once

#include <iostream>

[[noreturn]] inline void fatal(const char *message) {
    std::cerr << "fatal: " << message << std::endl;
    abort();
}

inline void warn(const char *message) {
    std::cerr << "warning: " << message << std::endl;
}
