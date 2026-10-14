// Code below is for demo purpose only.
// Does not represent anything.

#include "./proj.hpp"
#include "../platform/module.window.hpp"

constexpr uint32_t WIDTH  = 800;
constexpr uint32_t HEIGHT = 600;
uint32_t PIXEL_BUFFER[WIDTH * HEIGHT] = { 0 };

int32_t project_init() {
    Platform::init_window(WIDTH, HEIGHT, PIXEL_BUFFER, "Hello from Mpp");
    return 0;
}

void clear() {
    for (uint32_t i = 0; i < HEIGHT * WIDTH; ++i) {
        PIXEL_BUFFER[i] = 0xFF181818;
    }
}

#define MIN(a, b)      ((a) < (b) ? (a) : (b))
#define MAX(a, b)      ((a) > (b) ? (a) : (b))
#define CLAMP(a, b, c) MAX(a, MIN(b, c))

void lossy_draw_square(int32_t x_pos, int32_t y_pos, uint32_t s) {
    const uint32_t _xmin = (x_pos < 0) ? 0 : MIN(WIDTH, (uint32_t)x_pos);
    const uint32_t _xmax = (x_pos + (int32_t)s < 0) ? 0 : MIN(WIDTH, (uint32_t)(x_pos + s));

    const uint32_t _ymin = (y_pos < 0) ? 0 : MIN(HEIGHT, (uint32_t)y_pos);
    const uint32_t _ymax = (y_pos + (int32_t)s < 0) ? 0 : MIN(HEIGHT, (uint32_t)(y_pos + s));

    for (uint32_t r = _ymin; r < _ymax; ++r) {
        for (uint32_t c = _xmin; c < _xmax; ++c) {
            PIXEL_BUFFER[r * WIDTH + c] = 0xFF0000FF;
        }
    }
}

int32_t x = 0, y = 100, vx = 10, vy = -10, s = 20;

uint8_t project_frame_callback() {
    if (Platform::window_should_close()) { return false; }

    clear();
    lossy_draw_square(x, y, s);

    x += vx;
    y += vy;

    if (x < 0 || x + s >= (int32_t)WIDTH) {
        x = CLAMP(0, x, (int32_t)WIDTH - s);
        vx *= -1;
    }

    if (y < 0 || y + s >= (int32_t)HEIGHT) {
        y = CLAMP(0, y, (int32_t)HEIGHT - s);
        vy *= -1;
    }

    Platform::put_pixel_buffer();
    return true;
}

void project_close() {
    Platform::close_window();
}
