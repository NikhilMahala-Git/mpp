#ifndef _DEFS_HPP_
#define _DEFS_HPP_

#if !defined(__cplusplus) || __cplusplus < 202302L
#   error "C++23 or above is required."
#endif

#define PROJECT_COMPILER_CLANG   0
#define PROJECT_COMPILER_GCC     1
#define PROJECT_COMPILER_MINGW   2
#define PROJECT_COMPILER_MSVC    3

#if defined(__clang__)
#   define PROJECT_COMPILER PROJECT_COMPILER_CLANG
#   define PROJECT_COMPILER_STRING "Clang"
#elif defined(__GNUC__) && defined(_WIN32)
#   define PROJECT_COMPILER PROJECT_COMPILER_MINGW
#   define PROJECT_COMPILER_STRING "MinGW"
#elif defined(__GNUC__)
#   define PROJECT_COMPILER PROJECT_COMPILER_GCC
#   define PROJECT_COMPILER_STRING "GCC"
#elif defined(_MSC_VER)
#   define PROJECT_COMPILER PROJECT_COMPILER_MSVC
#   define PROJECT_COMPILER_STRING "MSVC"
#else
#   error "Compiler not recognized."
#endif

#define PROJECT_PLATFORM_LINUX   0
#define PROJECT_PLATFORM_WINDOWS 1
#define PROJECT_PLATFORM_WEB     2

#if defined(PLATFORM_WEB)
#   if defined(__clang__) && defined(__wasm32__)
#       define PROJECT_PLATFORM PROJECT_PLATFORM_WEB
#       define PROJECT_PLATFORM_STRING "Web-Wasm32"
#   else
#       error "Failed to target web platform. Either compiler is not clang or __wasm32__ macro is not defined."
#   endif
#elif defined(__linux__)
#   define PROJECT_PLATFORM PROJECT_PLATFORM_LINUX
#   define PROJECT_PLATFORM_STRING "Linux"
#elif defined(_WIN32)
#   define PROJECT_PLATFORM PROJECT_PLATFORM_WINDOWS
#   define PROJECT_PLATFORM_STRING "Windows"
#else
#   error "Platform not recognized."
#endif

#if PROJECT_PLATFORM == PROJECT_PLATFORM_WEB
#   define PROJECT_PLATFORM_IMPORT_METHOD(module_name, method_name) __attribute__((import_module(#module_name), import_name(#method_name))) extern
#   define PROJECT_PLATFORM_EXPORT_METHOD(             method_name) __attribute__((                             export_name(#method_name))) extern
#else
#   define PROJECT_PLATFORM_IMPORT_METHOD(module_name, method_name) extern "C"
#   define PROJECT_PLATFORM_EXPORT_METHOD(             method_name) extern "C"
#endif

#include <stddef.h>
#include <stdint.h>

#endif // _DEFS_HPP_
