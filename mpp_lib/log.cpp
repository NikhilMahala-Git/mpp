#include "./log.hpp"
#if PROJECT_PLATFORM != PROJECT_PLATFORM_WEB

#if PROJECT_PLATFORM == PROJECT_PLATFORM_LINUX
#   include <sys/ioctl.h>
#   include <unistd.h>
#endif
#include <string>
#include <iostream>

struct TerminalSize_T {
    uint16_t row, col;
};

TerminalSize_T __helper_get_terminal_size() {
    TerminalSize_T _tsize {};
#if PROJECT_PLATFORM == PROJECT_PLATFORM_LINUX
    struct winsize _ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &_ws) == 0) {
        _tsize.col = _ws.ws_col;
        _tsize.row = _ws.ws_row;
    }
#endif
    return _tsize;
}

MPP_LIB_NAMESPACE_BEGIN

void log_line(LogLevel_E log_level, const char *message) {
    std::string _label;
    switch(log_level) {
        using enum LogLevel_E;
        case INFO : _label = "[INFO ] "; break;
        case WARN : _label = "[WARN ] "; break;
        case ERROR: _label = "[ERROR] "; break;
        default   : break;
    }
    std::string _label_indent(_label.length(), ' ');

    TerminalSize_T _tsize = __helper_get_terminal_size();
    if (_tsize.col <= _label_indent.length()) {
        std::cout << _label << message << std::endl;
        return;
    }

    size_t _offset = _label_indent.length();

    std::string _message;
    while (*message) {
        char _c = *message;
        if (_offset >= (size_t)_tsize.col || _c == '\n') {
            _message += '\n';
            _message += _label_indent;
            _offset = _label_indent.length();
            if (_c == '\n') {
                ++message;
            }
            continue;
        }

        _message += _c;
        _offset++;
        ++message;
    }

    std::cout << _label << _message << std::endl;
}

MPP_LIB_NAMESPACE_END
#endif // PROJECT_PLATFORM != PROJECT_PLATFORM_WEB
