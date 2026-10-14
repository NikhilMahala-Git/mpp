#ifndef _BUILDER_UTILITY_H_
#define _BUILDER_UTILITY_H_
#include "../vendor/nob.hpp"

#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#if defined(__linux__)
#   include <sys/ioctl.h>
#   include <unistd.h>
#endif

namespace Builder::Helper {
    struct TerminalSize_T {
        uint16_t row, col;
    };

    inline TerminalSize_T __helper_get_terminal_size() {
        TerminalSize_T _tsize {};
#if defined(__linux__)
        struct winsize _ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &_ws) == 0) {
            _tsize.col = _ws.ws_col;
            _tsize.row = _ws.ws_row;
        }
#endif
        return _tsize;
    }
}

namespace Builder {
    enum LogLevel_E {
        INFO = 0,
        WARN,
        ERROR
    };

    template<typename... Args>
    void log_fmt(LogLevel_E log_level, const std::format_string<Args...> &fmt_string, Args&&... args) {
        std::string _label;
        switch(log_level) {
            using enum LogLevel_E;
            case INFO : _label = "[INFO ] "; break;
            case WARN : _label = "[WARN ] "; break;
            case ERROR: _label = "[ERROR] "; break;
            default   : break;
        }
        std::string _label_indent(_label.length(), ' ');
        const std::string _fmt_msg = std::vformat(fmt_string.get(), std::make_format_args(args...));

        auto _tsize = Helper::__helper_get_terminal_size();
        if (_tsize.col <= _label_indent.length()) {
            std::cout << _label << _fmt_msg << std::endl;
            return;
        }

        size_t _offset = _label_indent.length();
        std::string _message;

        for (size_t i = 0; i < _fmt_msg.length(); ++i) {
            char _c = _fmt_msg[i];
            if (_offset >= (size_t)_tsize.col || _c == '\n') {
                _message += '\n';
                _message += _label_indent;
                _offset = _label_indent.length();
                if (_c != '\n') {
                    i--;
                }
                continue;
            }

            _message += _c;
            _offset++;
        }

        std::cout << _label << _message << std::endl;
    }
}

namespace Builder::Fs {
    inline std::error_code Builder_Error_Code; // Inline variable

    inline bool collect_files_with_extension(const std::filesystem::path &directory, const std::string &extension, bool collect_recursively, std::vector<std::filesystem::path> &collection, bool clear_collection_before_run) {
        if (clear_collection_before_run) {
            collection.clear();
        }

        try {
            if (collect_recursively) {
                for (const auto &entry : std::filesystem::recursive_directory_iterator(directory)) {
                    if (entry.is_regular_file() && entry.path().extension() == extension) {
                        collection.emplace_back(entry.path());
                    }
                }
            } else {
                for (const auto &entry : std::filesystem::directory_iterator(directory)) {
                    if (entry.is_regular_file() && entry.path().extension() == extension) {
                        collection.emplace_back(entry.path());
                    }
                }
            }
        } catch (const std::exception &e) {
            log_fmt(LogLevel_E::ERROR, 
                    "Could not collect files with extension: '{}' from directory: {} ({})\n{}"
                    "",
                    extension,
                    directory.string(),
                    collect_recursively ? "Recursively" : "Not-Recursively",
                    e.what()
                   );
            return false;
        }
        return true;
    }

    // @returns true if path does not exist
    inline bool remove(const std::filesystem::path &path, bool remove_all) {
        if (!std::filesystem::exists(path)) {
            return true;
        }

        bool _result;
        if (remove_all) {
            _result = std::filesystem::remove_all(path, Builder_Error_Code);
        } else {
            _result = std::filesystem::remove(path, Builder_Error_Code);
        }

        if (Builder_Error_Code) {
            log_fmt(LogLevel_E::ERROR, "Could not remove {} ({})\n{}",
                    path.string(), remove_all ? "Remove-All" : "Not-Remove-All", Builder_Error_Code.message()
                   );
            return false;
        }
        return _result;
    }
}

namespace Builder::Nob {
    inline void custom_nob_log_handler(Nob_Log_Level level, const char *fmt, va_list args) {
        va_list _args_copy;
        va_copy(_args_copy, args);

        int _len = vsnprintf(nullptr, 0, fmt, _args_copy);
        va_end(_args_copy);
        if (_len <= 0) {
            va_end(args);
            return;
        }

        std::string _fmt_msg(_len * 2, 0);
        vsnprintf(&_fmt_msg[0], _fmt_msg.length(), fmt, args);
        va_end(args);

        log_fmt((LogLevel_E)level, "{}", _fmt_msg.c_str());
    }

    inline void nob_init() {
        ::nob_set_log_handler(&custom_nob_log_handler);
    }

    inline bool nob_cmd_run_sync_vec(const std::string &cmd, const std::vector<std::string> &cmd_args) {
        Nob_Cmd _cmd{};
        ::nob_cmd_append(&_cmd, cmd.c_str());

        for (const auto &arg : cmd_args) {
            ::nob_cmd_append(&_cmd, arg.c_str());
        }

        bool _result = nob_cmd_run_sync(_cmd);
        nob_da_free(_cmd);

        return _result;
    }
}

#endif // _BUILDER_UTILITY_H_
