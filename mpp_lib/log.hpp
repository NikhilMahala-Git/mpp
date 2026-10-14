#ifndef _MPP_LIB_LOG_HPP_
#define _MPP_LIB_LOG_HPP_
#include "./defs.hpp"

MPP_LIB_NAMESPACE_BEGIN

enum class LogLevel_E {
    INFO = 0,
    WARN,
    ERROR,
};

PROJECT_PLATFORM_IMPORT_METHOD(Log, Log_Line)
void log_line(LogLevel_E log_level, const char *message);

// TODO Implement something like logf that does not take in anything exepct format_string, format_args

MPP_LIB_NAMESPACE_END

#endif // _MPP_LIB_LOG_HPP_
