#include "tft_draw_utils.h"

#include <stdlib.h>

#include "board_def.h"
#include "iot_lcd.h"

extern CEspLcd *tft;

static inline uint8_t expand5(uint16_t value)
{
    return (uint8_t)((value << 3) | (value >> 2));
}

static inline uint8_t expand6(uint16_t value)
{
    return (uint8_t)((value << 2) | (value >> 4));
}

static inline int clip_int(int value, int min_value, int max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

static void set_rgb565_be_pixel(camera_fb_t *fb, int x, int y, uint16_t color)
{
    if (x < 0 || y < 0 || x >= fb->width || y >= fb->height)
    {
        return;
    }

    uint8_t *pixel = fb->buf + ((y * fb->width + x) * 2);
    pixel[0] = (uint8_t)(color >> 8);
    pixel[1] = (uint8_t)(color & 0xFF);
}

extern "C" void tft_common_display_rgb565_be(camera_fb_t *fb)
{
    const int x = (TFT_WITDH - fb->width) / 2;
    const int y = (TFT_HEIGHT - fb->height) / 2;

    tft->drawBitmapnotswap(x, y, (const uint16_t *)fb->buf, fb->width, fb->height);
}

extern "C" void tft_common_rgb565_be_to_bgr888(const camera_fb_t *fb, uint8_t *dst)
{
    const uint8_t *src = fb->buf;
    for (int i = 0; i < fb->width * fb->height; i++)
    {
        uint16_t rgb565 = ((uint16_t)src[0] << 8) | src[1];
        src += 2;

        uint8_t r = expand5((rgb565 >> 11) & 0x1F);
        uint8_t g = expand6((rgb565 >> 5) & 0x3F);
        uint8_t b = expand5(rgb565 & 0x1F);

        *dst++ = b;
        *dst++ = g;
        *dst++ = r;
    }
}

extern "C" void tft_common_draw_line_rgb565_be(camera_fb_t *fb, int x0, int y0, int x1, int y1, uint16_t color)
{
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true)
    {
        set_rgb565_be_pixel(fb, x0, y0, color);
        if (x0 == x1 && y0 == y1)
        {
            break;
        }

        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

extern "C" void tft_common_draw_rect_rgb565_be(camera_fb_t *fb, int x1, int y1, int x2, int y2, uint16_t color)
{
    x1 = clip_int(x1, 0, fb->width - 1);
    y1 = clip_int(y1, 0, fb->height - 1);
    x2 = clip_int(x2, 0, fb->width - 1);
    y2 = clip_int(y2, 0, fb->height - 1);

    for (int i = 0; i < 2; i++)
    {
        tft_common_draw_line_rgb565_be(fb, x1, y1 + i, x2, y1 + i, color);
        tft_common_draw_line_rgb565_be(fb, x1, y2 - i, x2, y2 - i, color);
        tft_common_draw_line_rgb565_be(fb, x1 + i, y1, x1 + i, y2, color);
        tft_common_draw_line_rgb565_be(fb, x2 - i, y1, x2 - i, y2, color);
    }
}

extern "C" void tft_common_draw_point_rgb565_be(camera_fb_t *fb, int x, int y, uint16_t color)
{
    for (int yy = -2; yy <= 2; yy++)
    {
        for (int xx = -2; xx <= 2; xx++)
        {
            if ((xx * xx + yy * yy) <= 5)
            {
                set_rgb565_be_pixel(fb, x + xx, y + yy, color);
            }
        }
    }
}

extern "C" void tft_common_draw_cross_rgb565_be(camera_fb_t *fb, int x, int y, int radius, uint16_t color)
{
    tft_common_draw_point_rgb565_be(fb, x, y, color);
    tft_common_draw_line_rgb565_be(fb, x - radius, y, x + radius, y, color);
    tft_common_draw_line_rgb565_be(fb, x, y - radius, x, y + radius, color);
}
