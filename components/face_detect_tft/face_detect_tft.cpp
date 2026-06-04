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
#include "face_detect.h"
#include "face_detect_tft.h"
#include "iot_lvgl.h"
#include "system.h"
#include "tft_draw_utils.h"

#define TAG "face_detect_tft"

static lv_obj_t *s_lv_container;
static lv_obj_t *s_lv_label_state;
static lv_obj_t *s_lv_label_box;
static lv_obj_t *s_lv_label_fps;

static uint8_t *s_bgr888 = NULL;
static size_t s_bgr888_size = 0;

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

static void face_gui_screen(void)
{
    if (!lvgl_port_lock(portMAX_DELAY))
    {
        return;
    }

    lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

    static lv_style_t splash_style;
    lv_style_init(&splash_style);
    lv_style_set_bg_opa(&splash_style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&splash_style, LV_STATE_DEFAULT, LV_COLOR_MAKE(190, 190, 190));
    lv_style_set_text_color(&splash_style, LV_STATE_DEFAULT, LV_COLOR_MAKE(70, 70, 70));
    lv_style_set_pad_top(&splash_style, LV_STATE_DEFAULT, 60);
    lv_style_set_pad_bottom(&splash_style, LV_STATE_DEFAULT, 60);
    lv_style_set_pad_left(&splash_style, LV_STATE_DEFAULT, 50);
    lv_style_set_pad_right(&splash_style, LV_STATE_DEFAULT, 50);

    lv_obj_t *splash = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(splash, LV_LABEL_PART_MAIN, &splash_style);
    lv_label_set_text(splash, "Face Detect");
    lv_obj_align(splash, NULL, LV_ALIGN_CENTER, 0, 0);
    lvgl_port_unlock();

    wait_msec(1000);

    if (!lvgl_port_lock(portMAX_DELAY))
    {
        return;
    }

    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

    static lv_style_t container_style;
    lv_style_init(&container_style);
    lv_style_set_bg_opa(&container_style, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_border_width(&container_style, LV_STATE_DEFAULT, 0);

    s_lv_container = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_add_style(s_lv_container, LV_CONT_PART_MAIN, &container_style);
    lv_obj_align(s_lv_container, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_obj_set_size(s_lv_container, 240, 42);

    s_lv_label_state = lv_label_create(s_lv_container, NULL);
    lv_label_set_recolor(s_lv_label_state, true);
    lv_label_set_text(s_lv_label_state, "#ff0000 FACE: --");
    lv_obj_align(s_lv_label_state, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 1);

    s_lv_label_box = lv_label_create(s_lv_container, NULL);
    lv_label_set_recolor(s_lv_label_box, true);
    lv_label_set_text(s_lv_label_box, "#00ff00 BOX: --");
    lv_obj_align(s_lv_label_box, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 15);

    s_lv_label_fps = lv_label_create(s_lv_container, NULL);
    lv_label_set_recolor(s_lv_label_fps, true);
    lv_label_set_text(s_lv_label_fps, "#0000ff FPS: --");
    lv_obj_align(s_lv_label_fps, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 29);

    lvgl_port_unlock();
}

static void update_face_status(bool found, const face_detect_box_t *box, float fps)
{
    char str[48];

    if (!lvgl_port_lock(20))
    {
        return;
    }

    lv_obj_move_foreground(s_lv_container);
    lv_label_set_text(s_lv_label_state, found ? "#ff0000 FACE: YES" : "#ff0000 FACE: NO");

    if (found && box != NULL)
    {
        snprintf(str, sizeof(str), "#00ff00 BOX:%d,%d,%d,%d", box->x1, box->y1, box->x2, box->y2);
    }
    else
    {
        snprintf(str, sizeof(str), "#00ff00 BOX: --");
    }
    lv_label_set_text(s_lv_label_box, str);

    snprintf(str, sizeof(str), "#0000ff FPS: %.2f", fps);
    lv_label_set_text(s_lv_label_fps, str);

    lvgl_port_unlock();
}

static void face_detect_task(void *arg)
{
    ESP_LOGI(TAG, "Starting face_detect_tft");

    sensor_t *s = esp_camera_sensor_get();
    int64_t last_ui_us = 0;
    int64_t last_fps_us = esp_timer_get_time();
    int frame_count = 0;
    int hit_count = 0;
    float last_fps = 0.0f;

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
            face_detect_box_t box = {};
            bool found = false;

            if (ensure_bgr888_buffer(fb->width, fb->height) == ESP_OK)
            {
                tft_common_rgb565_be_to_bgr888(fb, s_bgr888);
                found = face_detect_process_bgr888_with_box(s_bgr888, fb->width, fb->height, &box);
            }
            else
            {
                ESP_LOGE(TAG, "face detect buffer alloc failed");
            }

            if (found)
            {
                int cx = (box.x1 + box.x2) / 2;
                int cy = (box.y1 + box.y2) / 2;
                tft_common_draw_rect_rgb565_be(fb, box.x1, box.y1, box.x2, box.y2, 0xF800);
                tft_common_draw_cross_rgb565_be(fb, cx, cy, 8, 0xFFE0);
                hit_count++;
            }

            tft_common_display_rgb565_be(fb);

            frame_count++;
            if (now - last_fps_us >= 1000000)
            {
                last_fps = (float)frame_count * 1000000.0f / (float)(now - last_fps_us);
                ESP_LOGI(TAG,
                         "fps=%.2f face_hits=%d box=(%d,%d)-(%d,%d)",
                         last_fps,
                         hit_count,
                         found ? box.x1 : 0,
                         found ? box.y1 : 0,
                         found ? box.x2 : 0,
                         found ? box.y2 : 0);
                frame_count = 0;
                hit_count = 0;
                last_fps_us = now;
            }

            if (now - last_ui_us >= 150000)
            {
                update_face_status(found, found ? &box : NULL, last_fps);
                last_ui_us = now;
            }

            esp_camera_fb_return(fb);
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

extern "C" void face_detect_tft_start(void)
{
    ESP_LOGI(TAG, "Starting main");

    app_camera_init();
    lvgl_init();
    face_gui_screen();

    xTaskCreatePinnedToCore(face_detect_task, "face_detect_tft", 1024 * 9, nullptr, 5, nullptr, 0);
}
