#ifndef _MPP_LIB_WINDOW_HPP_
#define _MPP_LIB_WINDOW_HPP_
#include "./defs.hpp"

MPP_LIB_NAMESPACE_BEGIN

PROJECT_PLATFORM_IMPORT_METHOD(Window, Init_Window)
void init_window(uint32_t width, uint32_t height, uint32_t *pixel_buffer, const char *title);

PROJECT_PLATFORM_IMPORT_METHOD(Window, Close_Window)
void close_window();

PROJECT_PLATFORM_IMPORT_METHOD(Window, Window_Should_Close)
bool window_should_close();

PROJECT_PLATFORM_IMPORT_METHOD(Window, Put_Pixel_Buffer)
void put_pixel_buffer();

// TODO: Implement event bus and stuff like that.

MPP_LIB_NAMESPACE_END

#endif // _MPP_LIB_WINDOW_HPP_
