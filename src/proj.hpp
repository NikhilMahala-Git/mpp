#ifndef _PROJECT_HPP_
#define _PROJECT_HPP_
#include "../defs.hpp"

PROJECT_PLATFORM_EXPORT_METHOD(Init)
int32_t project_init();

PROJECT_PLATFORM_EXPORT_METHOD(Callback)
uint8_t project_frame_callback();

PROJECT_PLATFORM_EXPORT_METHOD(Close)
void    project_close();

#endif // _PROJECT_HPP_
