#ifndef TFT_DRAW_UTILS_H
#define TFT_DRAW_UTILS_H

#include <stdint.h>

#include "esp_camera.h"

#ifdef __cplusplus
extern "C" {
#endif

void tft_common_display_rgb565_be(camera_fb_t *fb);
void tft_common_rgb565_be_to_bgr888(const camera_fb_t *fb, uint8_t *dst);
void tft_common_draw_line_rgb565_be(camera_fb_t *fb, int x0, int y0, int x1, int y1, uint16_t color);
void tft_common_draw_rect_rgb565_be(camera_fb_t *fb, int x1, int y1, int x2, int y2, uint16_t color);
void tft_common_draw_point_rgb565_be(camera_fb_t *fb, int x, int y, uint16_t color);
void tft_common_draw_cross_rgb565_be(camera_fb_t *fb, int x, int y, int radius, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif
