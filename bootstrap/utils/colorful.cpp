#include "utils/colorful.h"

#ifdef _WIN32
    #include <io.h>
    #define isatty(fd) _isatty(fd)
    #define fileno(fd) _fileno(fd)
#else
    #include <unistd.h>
#endif

namespace colgm {

static bool stdout_is_tty() {
    static bool init = false;
    static bool tty = false;
    if (!init) {
        init = true;
        tty = isatty(fileno(stdout));
    }
    return tty;
}

static bool stderr_is_tty() {
    static bool init = false;
    static bool tty = false;
    if (!init) {
        init = true;
        tty = isatty(fileno(stderr));
    }
    return tty;
}

static bool enable_color(std::ostream& s) {
    if (s.rdbuf() == std::cout.rdbuf()) {
        return stdout_is_tty();
    }
    if (s.rdbuf() == std::cerr.rdbuf()) {
        return stderr_is_tty();
    }
    return false;
}

std::ostream& back_white(std::ostream& s) {
    if (!enable_color(s)) {
        return s;
    }
    s << "\033[7m";
    return s;
}

std::ostream& red(std::ostream& s) {
    if (!enable_color(s)) {
        return s;
    }
    s << "\033[91;1m";
    return s;
}

std::ostream& cyan(std::ostream& s) {
    if (!enable_color(s)) {
        return s;
    }
    s << "\033[36;1m";
    return s;
}

std::ostream& green(std::ostream& s) {
    if (!enable_color(s)) {
        return s;
    }
    s << "\033[92;1m";
    return s;
}

std::ostream& orange(std::ostream& s) {
    if (!enable_color(s)) {
        return s;
    }
    s << "\033[93;1m";
    return s;
}

std::ostream& white(std::ostream& s) {
    if (!enable_color(s)) {
        return s;
    }
    s << "\033[97;1m";
    return s;
}

std::ostream& reset(std::ostream& s) {
    if (!enable_color(s)) {
        return s;
    }
    s << "\033[0m";
    return s;
}

}
