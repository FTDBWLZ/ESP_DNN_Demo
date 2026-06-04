#include <esp_err.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_timer.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <stdlib.h>

#include "app_camera.h"
#include "app_screen.h"
#include "board_def.h"
#include "hand_track_detect.h"
#include "iot_lvgl.h"
#include "palm_keypoint_tft.h"
#include "system.h"

#define TAG "palm_keypoint_tft"

extern CEspLcd *tft;

static lv_obj_t *s_lv_container;
static lv_obj_t *s_lv_bar_hand;
static lv_obj_t *s_lv_label_hand;
static lv_obj_t *s_lv_bar_score;
static lv_obj_t *s_lv_label_score;
static lv_obj_t *s_lv_bar_keypoint;
static lv_obj_t *s_lv_label_keypoint;

static uint8_t *s_bgr888 = NULL;
static size_t s_bgr888_size = 0;

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

static esp_err_t ensure_bgr888_buffer(size_t width, size_t height)
{
    size_t needed = width * height * 3U;
    if (s_bgr888 != NULL && s_bgr888_size >= needed)
    {
        return ESP_OK;
    }

    free(s_bgr888);
    s_bgr888 = (uint8_t *)heap_caps_malloc(needed, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (s_bgr888 == NULL)
    {
        s_bgr888 = (uint8_t *)malloc(needed);
    }
    s_bgr888_size = s_bgr888 == NULL ? 0 : needed;

    return s_bgr888 == NULL ? ESP_ERR_NO_MEM : ESP_OK;
}

static void rgb565_be_to_bgr888_buffer(const camera_fb_t *fb, uint8_t *dst)
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

static void draw_line_rgb565_be(camera_fb_t *fb, int x0, int y0, int x1, int y1, uint16_t color)
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

static void draw_rect_rgb565_be(camera_fb_t *fb, int x1, int y1, int x2, int y2, uint16_t color)
{
    x1 = clip_int(x1, 0, fb->width - 1);
    y1 = clip_int(y1, 0, fb->height - 1);
    x2 = clip_int(x2, 0, fb->width - 1);
    y2 = clip_int(y2, 0, fb->height - 1);

    for (int i = 0; i < 2; i++)
    {
        draw_line_rgb565_be(fb, x1, y1 + i, x2, y1 + i, color);
        draw_line_rgb565_be(fb, x1, y2 - i, x2, y2 - i, color);
        draw_line_rgb565_be(fb, x1 + i, y1, x1 + i, y2, color);
        draw_line_rgb565_be(fb, x2 - i, y1, x2 - i, y2, color);
    }
}

static void draw_point_rgb565_be(camera_fb_t *fb, int x, int y, uint16_t color)
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

static void draw_hand_keypoints(camera_fb_t *fb, const hand_track_result_t *result)
{
    if (!result->has_model_keypoints || result->keypoint_count <= 0)
    {
        return;
    }

    int point_count = result->keypoint_count;
    if (point_count > HAND_TRACK_MAX_KEYPOINTS)
    {
        point_count = HAND_TRACK_MAX_KEYPOINTS;
    }

    static const uint8_t bones[][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 4},
        {0, 5}, {5, 6}, {6, 7}, {7, 8},
        {0, 9}, {9, 10}, {10, 11}, {11, 12},
        {0, 13}, {13, 14}, {14, 15}, {15, 16},
        {0, 17}, {17, 18}, {18, 19}, {19, 20},
        {5, 9}, {9, 13}, {13, 17},
    };

    for (int i = 0; i < (int)(sizeof(bones) / sizeof(bones[0])); i++)
    {
        int a = bones[i][0];
        int b = bones[i][1];
        if (a < point_count && b < point_count)
        {
            draw_line_rgb565_be(fb,
                                result->keypoints[a * 2],
                                result->keypoints[a * 2 + 1],
                                result->keypoints[b * 2],
                                result->keypoints[b * 2 + 1],
                                0x07E0);
        }
    }

    for (int i = 0; i < point_count; i++)
    {
        draw_point_rgb565_be(fb, result->keypoints[i * 2], result->keypoints[i * 2 + 1], i == 0 ? 0xF800 : 0xFFE0);
    }
}

static void draw_hand_center(camera_fb_t *fb, const hand_track_result_t *result)
{
    draw_point_rgb565_be(fb, result->target_x, result->target_y, 0xFFE0);
    draw_line_rgb565_be(fb, result->target_x - 8, result->target_y, result->target_x + 8, result->target_y, 0xFFE0);
    draw_line_rgb565_be(fb, result->target_x, result->target_y - 8, result->target_x, result->target_y + 8, 0xFFE0);
}

static void display_camera_rgb565_be(camera_fb_t *fb)
{
    const int x = (TFT_WITDH - fb->width) / 2;
    const int y = (TFT_HEIGHT - fb->height) / 2;

    tft->drawBitmapnotswap(x, y, (const uint16_t *)fb->buf, fb->width, fb->height);
}

static void palm_gui_screen(void)
{
    lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, LV_STATE_DEFAULT, 2);
    lv_style_set_bg_opa(&style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&style, LV_STATE_DEFAULT, LV_COLOR_MAKE(190, 190, 190));
    lv_style_set_border_width(&style, LV_STATE_DEFAULT, 2);
    lv_style_set_border_color(&style, LV_STATE_DEFAULT, LV_COLOR_MAKE(142, 142, 142));
    lv_style_set_pad_top(&style, LV_STATE_DEFAULT, 60);
    lv_style_set_pad_bottom(&style, LV_STATE_DEFAULT, 60);
    lv_style_set_pad_left(&style, LV_STATE_DEFAULT, 60);
    lv_style_set_pad_right(&style, LV_STATE_DEFAULT, 60);
    lv_style_set_text_color(&style, LV_STATE_DEFAULT, LV_COLOR_MAKE(102, 102, 102));
    lv_style_set_text_letter_space(&style, LV_STATE_DEFAULT, 5);
    lv_style_set_text_line_space(&style, LV_STATE_DEFAULT, 20);

    lv_obj_t *obj = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(obj, LV_LABEL_PART_MAIN, &style);
    lv_label_set_text(obj, "Palm Detect\nKeypoints");
    lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, 0);
    wait_msec(1200);
    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

    static lv_style_t style_container;
    lv_style_init(&style_container);
    lv_style_set_bg_color(&style_container, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_bg_opa(&style_container, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_border_width(&style_container, LV_STATE_DEFAULT, 0);

    s_lv_container = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_add_style(s_lv_container, LV_CONT_PART_MAIN, &style_container);
    lv_obj_align(s_lv_container, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_obj_set_size(s_lv_container, 240, 40);

    static lv_style_t label_style;
    lv_style_init(&label_style);
    lv_style_set_text_opa(&label_style, LV_STATE_DEFAULT, LV_OPA_COVER);

    s_lv_label_hand = lv_label_create(s_lv_container, NULL);
    lv_obj_add_style(s_lv_label_hand, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_recolor(s_lv_label_hand, true);
    lv_label_set_text(s_lv_label_hand, "#ff0000 HAND: --");
    lv_obj_align(s_lv_label_hand, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 1);

    s_lv_label_score = lv_label_create(s_lv_container, NULL);
    lv_obj_add_style(s_lv_label_score, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_recolor(s_lv_label_score, true);
    lv_label_set_text(s_lv_label_score, "#00ff00 SCORE: --");
    lv_obj_align(s_lv_label_score, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 14);

    s_lv_label_keypoint = lv_label_create(s_lv_container, NULL);
    lv_obj_add_style(s_lv_label_keypoint, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_recolor(s_lv_label_keypoint, true);
    lv_label_set_text(s_lv_label_keypoint, "#0000ff KP: --");
    lv_obj_align(s_lv_label_keypoint, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 27);

    static lv_style_t style_bar;
    lv_style_init(&style_bar);
    lv_style_set_radius(&style_bar, LV_STATE_DEFAULT, 4);
    lv_style_set_bg_color(&style_bar, LV_STATE_DEFAULT, LV_COLOR_RED);

    s_lv_bar_hand = lv_bar_create(s_lv_container, NULL);
    lv_obj_add_style(s_lv_bar_hand, LV_BAR_PART_INDIC, &style_bar);
    lv_obj_set_size(s_lv_bar_hand, 160, 6);
    lv_obj_align(s_lv_bar_hand, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 5);
    lv_bar_set_anim_time(s_lv_bar_hand, 0);
    lv_bar_set_range(s_lv_bar_hand, 0, 255);
    lv_obj_set_style_local_bg_opa(s_lv_bar_hand, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    s_lv_bar_score = lv_bar_create(s_lv_container, NULL);
    lv_obj_add_style(s_lv_bar_score, LV_BAR_PART_INDIC, &style_bar);
    lv_obj_set_size(s_lv_bar_score, 160, 6);
    lv_obj_align(s_lv_bar_score, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 18);
    lv_bar_set_anim_time(s_lv_bar_score, 0);
    lv_bar_set_range(s_lv_bar_score, 0, 255);
    lv_obj_set_style_local_bg_color(s_lv_bar_score, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_obj_set_style_local_bg_opa(s_lv_bar_score, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    s_lv_bar_keypoint = lv_bar_create(s_lv_container, NULL);
    lv_obj_add_style(s_lv_bar_keypoint, LV_BAR_PART_INDIC, &style_bar);
    lv_obj_set_size(s_lv_bar_keypoint, 160, 6);
    lv_obj_align(s_lv_bar_keypoint, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 31);
    lv_bar_set_anim_time(s_lv_bar_keypoint, 0);
    lv_bar_set_range(s_lv_bar_keypoint, 0, 255);
    lv_obj_set_style_local_bg_color(s_lv_bar_keypoint, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_BLUE);
    lv_obj_set_style_local_bg_opa(s_lv_bar_keypoint, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
}

static void update_hand_status(bool found, const hand_track_result_t *result)
{
    static bool last_found = false;
    static int last_score = -1;
    static int last_kp = -1;
    char str[32];

    lv_obj_move_foreground(s_lv_container);

    int score = 0;
    int kp = 0;
    bool model_kp = false;
    if (found && result != NULL)
    {
        score = (int)(result->score * 100.0f);
        kp = result->has_model_keypoints ? result->keypoint_count : 0;
        model_kp = result->has_model_keypoints;
    }

    if (found != last_found)
    {
        lv_bar_set_value(s_lv_bar_hand, found ? 255 : 0, LV_ANIM_OFF);
        lv_label_set_text(s_lv_label_hand, found ? "#ff0000 HAND: YES" : "#ff0000 HAND: NO");
        last_found = found;
    }

    if (score != last_score)
    {
        lv_bar_set_value(s_lv_bar_score, score * 255 / 100, LV_ANIM_OFF);
        snprintf(str, sizeof(str), "#00ff00 SCORE: %d%%", score);
        lv_label_set_text(s_lv_label_score, str);
        last_score = score;
    }

    if (kp != last_kp || model_kp)
    {
        lv_bar_set_value(s_lv_bar_keypoint, kp * 255 / HAND_TRACK_MAX_KEYPOINTS, LV_ANIM_OFF);
        if (model_kp)
        {
            snprintf(str, sizeof(str), "#0000ff KP: %d M", kp);
        }
        else
        {
            snprintf(str, sizeof(str), "#0000ff KP: --");
        }
        lv_label_set_text(s_lv_label_keypoint, str);
        last_kp = kp;
    }
}

static void palm_detect_task(void *arg)
{
    ESP_LOGI(TAG, "Starting palm_keypoints");

    sensor_t *s = esp_camera_sensor_get();
    int64_t last_ui_us = 0;
    int64_t last_fps_us = esp_timer_get_time();
    int frame_count = 0;
    int hit_count = 0;

    while (true)
    {
        camera_fb_t *fb = esp_camera_fb_get();

        if (!fb)
        {
            ESP_LOGE(TAG, "Camera capture failed");
        }
        else if (s->pixformat == PIXFORMAT_JPEG)
        {
            TFT_jpg_image(CENTER, CENTER, 0, -1, NULL, fb->buf, fb->len);
            esp_camera_fb_return(fb);
        }
        else
        {
            const int64_t now = esp_timer_get_time();
            hand_track_result_t hand = {};
            bool found = false;

            if (ensure_bgr888_buffer(fb->width, fb->height) == ESP_OK)
            {
                rgb565_be_to_bgr888_buffer(fb, s_bgr888);
                found = hand_track_detect_process_bgr888(s_bgr888, fb->width, fb->height, &hand);
            }
            else
            {
                ESP_LOGE(TAG, "hand detect buffer alloc failed");
            }

            if (found)
            {
                draw_rect_rgb565_be(fb, hand.x1, hand.y1, hand.x2, hand.y2, 0x001F);
                if (hand.has_model_keypoints)
                {
                    draw_hand_keypoints(fb, &hand);
                }
                else
                {
                    draw_hand_center(fb, &hand);
                }
                hit_count++;
            }

            display_camera_rgb565_be(fb);

            if (now - last_ui_us >= 150000)
            {
                update_hand_status(found, found ? &hand : NULL);
                last_ui_us = now;
            }

            frame_count++;
            if (now - last_fps_us >= 1000000)
            {
                const float fps = (float)frame_count * 1000000.0f / (float)(now - last_fps_us);
                ESP_LOGI(TAG,
                         "fps=%.2f hand_hits=%d score=%.2f box=(%d,%d)-(%d,%d) kp=%d%s",
                         fps,
                         hit_count,
                         found ? hand.score : 0.0f,
                         found ? hand.x1 : 0,
                         found ? hand.y1 : 0,
                         found ? hand.x2 : 0,
                         found ? hand.y2 : 0,
                         found && hand.has_model_keypoints ? hand.keypoint_count : 0,
                         found ? (hand.has_model_keypoints ? " model" : " detect_only") : " none");
                frame_count = 0;
                hit_count = 0;
                last_fps_us = now;
            }

            esp_camera_fb_return(fb);
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

extern "C" void palm_keypoint_tft_start(void)
{
    ESP_LOGI(TAG, "Starting main");

    app_camera_init();
    lvgl_init();
    palm_gui_screen();

    xTaskCreatePinnedToCore(palm_detect_task, "palm_keypoint_tft", 1024 * 9, nullptr, 5, nullptr, 0);
}
