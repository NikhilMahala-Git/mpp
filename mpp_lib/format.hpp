#ifndef _MPP_LIB_FORMAT_HPP_
#define _MPP_LIB_FORMAT_HPP_
#include "./defs.hpp"
#include "./string.hpp"
#include "./lib.hpp"

MPP_LIB_NAMESPACE_BEGIN

template<typename T>
struct formatter {
    static_assert(sizeof(T) == 0, "Implement custom formatter for your type.");
    String to_string(const T& data); 
};

using format_vfunc_t = String (*) (const void *data);
template<typename T>
String format_trampoline(const void *data) {
    const T &_true_data = *static_cast<const T*>(data);

    formatter<T> _formatter;
    return _formatter.to_string(_true_data);
}

struct basic_fmt_arg {
    const void *data;
    format_vfunc_t format_vfunc;
};

inline String vformat_engine(const String &fmt_string, size_t arg_count, const basic_fmt_arg *arg_values) {
    String _formatted_string("");

    size_t _fmt_arg_index = 0;
    for (size_t i = 0; i < fmt_string.length(); ++i) {
        if (fmt_string[i] == '{' && (i + 1) < fmt_string.length() && fmt_string[i + 1] == '}') {
            if (_fmt_arg_index >= arg_count) {
                String _assert_message("Not sufficient format arguments provided.\n");
                _assert_message += "format_string: "; _assert_message += fmt_string; _assert_message += '\n';
                _assert_message += "arg count    : "; _assert_message += String::from_uint(arg_count);
                MPP_MACRO_ASSERT(false, _assert_message.data());
            }
            _formatted_string += arg_values[_fmt_arg_index].format_vfunc(arg_values[_fmt_arg_index].data);
            _fmt_arg_index++;
            i++;
            continue;
        }
        _formatted_string += fmt_string[i];
    }

    return _formatted_string;
}

// RULES:
// (r1) - formatting placeholder begins with '{' and ends with '}' (placeholder may or may not contain chars in-between)
//        To include '{' as standalone char use \{
//
// (r2) - Inside the formatting placeholder user may provide key value pairs.
//        A key value pair is denoted like this .<key-name> <value>
//        Key value pairs can be omitted.
//
//        As of now custom key value pairs not supported, only supported key value pairs are:
//            - .max-length <size_t>
//            - .min-length <size_t>
//            - .pad-dir    <l|r>
//            - .pad-char   <char>
// (r3) - Types supported by default includes: (Other default types may fallback to these. If not provide them.)
//            - float, double                                -> Formatting library used: https://github.com/xjb714/xjb
//
//            - int8_t, int16_t, int32_t, int64_t            -> Formatting preserves correct numeric value. Includes only numbers and -
//
//            - uint8_t, uint16_t, uint32_t, uint64_t        -> Formatting preserves correct numeric value. Includes only numbers.
//
//            - bool                                         -> Formatting maps value to "true" or "false".
// TODO:
//  - Implement r1, r2.
//  - Implement pointer support.
//  - Switch to string view over string to reduce allocation for fmt_string.
//  - Provide better diagnostics
template<typename... Args>
String format(const String &fmt_string, Args&&... args) {
    basic_fmt_arg _fmt_args[] = {
        basic_fmt_arg {
            .data = &args,
            .format_vfunc = &format_trampoline<mpp::remove_cv_t<mpp::remove_reference_t<Args>>>
        }...
    };

    return vformat_engine(fmt_string, sizeof(_fmt_args) / sizeof(_fmt_args[0]), _fmt_args);
}

MPP_LIB_NAMESPACE_END

// Default formatters
MPP_LIB_NAMESPACE_BEGIN

#define IMPLEMENT_INT_FORMATTER(int_type) \
    template<>                            \
    struct formatter<int_type> {          \
        String to_string(int_type v) {    \
            return String::from_int(v);   \
        }                                 \
    }

IMPLEMENT_INT_FORMATTER(int8_t );
IMPLEMENT_INT_FORMATTER(int16_t);
IMPLEMENT_INT_FORMATTER(int32_t);
IMPLEMENT_INT_FORMATTER(int64_t);
#undef  IMPLEMENT_INT_FORMATTER

#define IMPLEMENT_UINT_FORMATTER(uint_type) \
    template<>                              \
    struct formatter<uint_type> {           \
        String to_string(uint_type v) {     \
            return String::from_uint(v);    \
        }                                   \
    }

IMPLEMENT_UINT_FORMATTER(uint8_t );
IMPLEMENT_UINT_FORMATTER(uint16_t);
IMPLEMENT_UINT_FORMATTER(uint32_t);
IMPLEMENT_UINT_FORMATTER(uint64_t);
#undef  IMPLEMENT_UINT_FORMATTER

template<>                              
struct formatter<float> {           
    String to_string(float v) {     
        return String::from_float(v);    
    }                                   
};

template<>                              
struct formatter<double> {           
    String to_string(double v) {     
        return String::from_double(v);    
    }                                   
};

template<>
struct formatter<String> {
    String to_string(const String &v) {
        return v;
    } 
};

template<>
struct formatter<bool> {
    String to_string(bool v) {
        return v ? "true" : "false";
    } 
};

MPP_LIB_NAMESPACE_END

#endif // _MPP_LIB_FORMAT_HPP_
