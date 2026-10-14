#include "./window.hpp"
#if PROJECT_PLATFORM != PROJECT_PLATFORM_WEB
#include "../vendor/raylib/include/raylib.h"

MPP_LIB_NAMESPACE_BEGIN

uint32_t resource_width;
uint32_t resource_height;
uint32_t *resource_pixel_buffer;
Texture2D resource_texture_2d;

void init_window(uint32_t width, uint32_t height, uint32_t *pixel_buffer, const char *title) {
    InitWindow(width, height, title);

    resource_width = width;
    resource_height = height;
    resource_pixel_buffer = pixel_buffer;

    Image _image = GenImageColor(width, height, BLANK);
    resource_texture_2d = LoadTextureFromImage(_image);
    UnloadImage(_image);

    SetTargetFPS(60);
}

void close_window() {
    UnloadTexture(resource_texture_2d);
    CloseWindow();
}

bool window_should_close() {
    return WindowShouldClose();
}

void put_pixel_buffer() {
    UpdateTexture(resource_texture_2d, resource_pixel_buffer);

    BeginDrawing();
    DrawTexture(resource_texture_2d, 0, 0, WHITE);
    EndDrawing();
}

MPP_LIB_NAMESPACE_END
#endif // PROJECT_PLATFORM != PROJECT_PLATFORM_WEB
