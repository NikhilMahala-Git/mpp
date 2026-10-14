#include "./proj.hpp"
#include "../mpp_lib/window.hpp"

constexpr uint32_t WIDTH  = 800;
constexpr uint32_t HEIGHT = 600;
uint32_t PIXEL_BUFFER[WIDTH * HEIGHT] = { 0 };

int32_t project_init() {
    mpp::init_window(WIDTH, HEIGHT, PIXEL_BUFFER, "Hello from Mpp");
    return 0;
}

uint8_t project_frame_callback() {
    if (mpp::window_should_close()) { return false; }

    for (uint32_t y = 0; y < HEIGHT; ++y) {
        for (uint32_t x = 0; x < WIDTH; ++x) {
            PIXEL_BUFFER[y * WIDTH + x] = x < WIDTH / 2 ? 0xFF0000FF : 0xFF000000;
        }
    }

    mpp::put_pixel_buffer();
    return true;
}

void project_close() {
    mpp::close_window();
}
