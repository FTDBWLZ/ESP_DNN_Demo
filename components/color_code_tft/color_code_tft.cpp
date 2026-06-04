/////////////////////////////////////////////////////////////////
/*
  RGB Pixel Detector & Histogram (Running OpenCV on ESP32)
  For More Information: https://youtu.be/DNQuCkPtzYA
  Created by Eric N. (ThatProject)
*/
/////////////////////////////////////////////////////////////////

#include "color_code_tft_config.h"

#if COLOR_CODE_FULL_FEATURES
#undef EPS
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#define EPS 192
#endif

#include <esp_err.h>
#include <esp_log.h>
#include <esp_timer.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

#include "app_camera.h"
#include "app_screen.h"
#include "board_def.h"
#include "color_code_tft.h"
#include "iot_lvgl.h"
#include "system.h"

#if COLOR_CODE_FULL_FEATURES
using namespace cv;
#endif

#define TAG "color_code_tft"

extern CEspLcd *tft;

#if COLOR_CODE_FULL_FEATURES
static lv_obj_t *s_lv_camera_image;
static const int kHistSize = 240;
static const int kHistW = 240;
static const int kHistH = 240;
#endif

static lv_obj_t *s_lv_container_rgb;
static lv_obj_t *s_lv_bar_red;
static lv_obj_t *s_lv_label_red;
static lv_obj_t *s_lv_bar_green;
static lv_obj_t *s_lv_label_green;
static lv_obj_t *s_lv_bar_blue;
static lv_obj_t *s_lv_label_blue;

static inline uint8_t expand5(uint16_t value)
{
    return (uint8_t)((value << 3) | (value >> 2));
}

static inline uint8_t expand6(uint16_t value)
{
    return (uint8_t)((value << 2) | (value >> 4));
}

static inline void set_rgb565_be_pixel(camera_fb_t *fb, int x, int y, uint16_t color)
{
    if (x < 0 || y < 0 || x >= fb->width || y >= fb->height)
    {
        return;
    }

    uint8_t *pixel = fb->buf + ((y * fb->width + x) * 2);
    pixel[0] = (uint8_t)(color >> 8);
    pixel[1] = (uint8_t)(color & 0xFF);
}

static void read_center_rgb(const camera_fb_t *fb, int *red, int *green, int *blue)
{
    const int pos_x = fb->width / 2;
    const int pos_y = fb->height / 2;
    const uint8_t *pixel = fb->buf + ((pos_y * fb->width + pos_x) * 2);
    const uint16_t rgb565 = ((uint16_t)pixel[0] << 8) | pixel[1];

    *red = expand5((rgb565 >> 11) & 0x1F);
    *green = expand6((rgb565 >> 5) & 0x3F);
    *blue = expand5(rgb565 & 0x1F);
}

static void draw_center_mark_rgb565_be(camera_fb_t *fb)
{
    const int cx = fb->width / 2;
    const int cy = fb->height / 2;
    const int half_len = 20;

    for (int y = cy - half_len; y <= cy + half_len; y++)
    {
        set_rgb565_be_pixel(fb, cx, y, 0xFFFF);
    }

    for (int x = cx - half_len; x <= cx + half_len; x++)
    {
        set_rgb565_be_pixel(fb, x, cy, 0xFFFF);
    }
}

static void display_camera_rgb565_be(camera_fb_t *fb)
{
    const int x = (TFT_WITDH - fb->width) / 2;
    const int y = (TFT_HEIGHT - fb->height) / 2;

    tft->drawBitmapnotswap(x, y, (const uint16_t *)fb->buf, fb->width, fb->height);
}

#if COLOR_CODE_FULL_FEATURES
static void rgb565_be_to_bgr888(const camera_fb_t *fb, Mat &dst)
{
    dst.create(fb->height, fb->width, CV_8UC3);

    const uint8_t *src = fb->buf;
    for (int y = 0; y < fb->height; y++)
    {
        Vec3b *row = dst.ptr<Vec3b>(y);
        for (int x = 0; x < fb->width; x++)
        {
            uint16_t rgb565 = ((uint16_t)src[0] << 8) | src[1];
            src += 2;

            uint8_t r = expand5((rgb565 >> 11) & 0x1F);
            uint8_t g = expand6((rgb565 >> 5) & 0x3F);
            uint8_t b = expand5(rgb565 & 0x1F);

            row[x] = Vec3b(b, g, r);
        }
    }
}

static esp_err_t update_camera_image(const Mat &img)
{
    static Mat img_copy;
    static lv_img_dsc_t img_dsc;

    if (img.empty())
    {
        ESP_LOGW(TAG, "Can't display empty image");
        return ESP_ERR_INVALID_ARG;
    }

    if (img.type() == CV_8UC1)
    {
        cvtColor(img, img_copy, COLOR_GRAY2BGR565, 1);
    }
    else if (img.type() == CV_8UC3)
    {
        cvtColor(img, img_copy, COLOR_BGR2BGR565, 1);
    }
    else if (img.type() == CV_8UC2)
    {
        img.copyTo(img_copy);
    }

    img_dsc.header.always_zero = 0;
    img_dsc.header.w = img_copy.cols;
    img_dsc.header.h = img_copy.rows;
    img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    img_dsc.data_size = img_copy.total() * img_copy.elemSize();
    img_dsc.data = img_copy.ptr<uchar>(0);

    lv_img_set_src(s_lv_camera_image, &img_dsc);
    lv_obj_set_pos(s_lv_camera_image, (TFT_WITDH - img_copy.cols) / 2, (TFT_HEIGHT - img_copy.rows) / 2);

    return ESP_OK;
}

static void draw_center_mark(Mat &src)
{
    const int cx = src.cols / 2;
    const int cy = src.rows / 2;
    line(src, Point(cx, cy - 20), Point(cx, cy + 20), Scalar(255, 255, 255), 2, LINE_8);
    line(src, Point(cx - 20, cy), Point(cx + 20, cy), Scalar(255, 255, 255), 2, LINE_8);
}

static void draw_histogram(Mat &b_hist, Mat &g_hist, Mat &r_hist, Mat &src)
{
    int bin_w = cvRound((double)kHistW / kHistSize);

    normalize(b_hist, b_hist, 0, src.rows, NORM_MINMAX, -1, Mat());
    normalize(g_hist, g_hist, 0, src.rows, NORM_MINMAX, -1, Mat());
    normalize(r_hist, r_hist, 0, src.rows, NORM_MINMAX, -1, Mat());

    for (int i = 1; i < kHistSize; i++)
    {
        line(src, Point(bin_w * (i - 1), kHistH - cvRound(b_hist.at<float>(i - 1))),
             Point(bin_w * i, kHistH - cvRound(b_hist.at<float>(i))), Scalar(255, 0, 0), 1, LINE_AA);
        line(src, Point(bin_w * (i - 1), kHistH - cvRound(g_hist.at<float>(i - 1))),
             Point(bin_w * i, kHistH - cvRound(g_hist.at<float>(i))), Scalar(0, 255, 0), 1, LINE_AA);
        line(src, Point(bin_w * (i - 1), kHistH - cvRound(r_hist.at<float>(i - 1))),
             Point(bin_w * i, kHistH - cvRound(r_hist.at<float>(i))), Scalar(0, 0, 255), 1, LINE_AA);
    }
}
#endif

static void gui_screen(void)
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
    lv_label_set_text(obj, "RGB Pixel\nDetector");
    lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, 0);
    wait_msec(1200);
    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

#if COLOR_CODE_FULL_FEATURES
    s_lv_camera_image = lv_img_create(lv_scr_act(), NULL);
#endif

    static lv_style_t style_container;
    lv_style_init(&style_container);
    lv_style_set_bg_color(&style_container, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_bg_opa(&style_container, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_border_width(&style_container, LV_STATE_DEFAULT, 0);

    s_lv_container_rgb = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_add_style(s_lv_container_rgb, LV_CONT_PART_MAIN, &style_container);
    lv_obj_align(s_lv_container_rgb, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_obj_set_size(s_lv_container_rgb, 240, 40);

    static lv_style_t label_style;
    lv_style_init(&label_style);
    lv_style_set_text_opa(&label_style, LV_STATE_DEFAULT, LV_OPA_COVER);

    s_lv_label_red = lv_label_create(s_lv_container_rgb, NULL);
    lv_obj_add_style(s_lv_label_red, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_recolor(s_lv_label_red, true);
    lv_label_set_text(s_lv_label_red, "#ff0000 R: --");
    lv_obj_align(s_lv_label_red, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 1);

    s_lv_label_green = lv_label_create(s_lv_container_rgb, NULL);
    lv_obj_add_style(s_lv_label_green, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_recolor(s_lv_label_green, true);
    lv_label_set_text(s_lv_label_green, "#00ff00 G: --");
    lv_obj_align(s_lv_label_green, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 14);

    s_lv_label_blue = lv_label_create(s_lv_container_rgb, NULL);
    lv_obj_add_style(s_lv_label_blue, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_recolor(s_lv_label_blue, true);
    lv_label_set_text(s_lv_label_blue, "#0000ff B: --");
    lv_obj_align(s_lv_label_blue, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 27);

    static lv_style_t style_bar;
    lv_style_init(&style_bar);
    lv_style_set_radius(&style_bar, LV_STATE_DEFAULT, 4);
    lv_style_set_bg_color(&style_bar, LV_STATE_DEFAULT, LV_COLOR_RED);

    s_lv_bar_red = lv_bar_create(s_lv_container_rgb, NULL);
    lv_obj_add_style(s_lv_bar_red, LV_BAR_PART_INDIC, &style_bar);
    lv_obj_set_size(s_lv_bar_red, 160, 6);
    lv_obj_align(s_lv_bar_red, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 5);
    lv_bar_set_anim_time(s_lv_bar_red, 0);
    lv_bar_set_range(s_lv_bar_red, 0, 255);
    lv_obj_set_style_local_bg_opa(s_lv_bar_red, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    s_lv_bar_green = lv_bar_create(s_lv_container_rgb, NULL);
    lv_obj_add_style(s_lv_bar_green, LV_BAR_PART_INDIC, &style_bar);
    lv_obj_set_size(s_lv_bar_green, 160, 6);
    lv_obj_align(s_lv_bar_green, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 18);
    lv_bar_set_anim_time(s_lv_bar_green, 0);
    lv_bar_set_range(s_lv_bar_green, 0, 255);
    lv_obj_set_style_local_bg_color(s_lv_bar_green, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_obj_set_style_local_bg_opa(s_lv_bar_green, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    s_lv_bar_blue = lv_bar_create(s_lv_container_rgb, NULL);
    lv_obj_add_style(s_lv_bar_blue, LV_BAR_PART_INDIC, &style_bar);
    lv_obj_set_size(s_lv_bar_blue, 160, 6);
    lv_obj_align(s_lv_bar_blue, NULL, LV_ALIGN_IN_TOP_LEFT, 60, 31);
    lv_bar_set_anim_time(s_lv_bar_blue, 0);
    lv_bar_set_range(s_lv_bar_blue, 0, 255);
    lv_obj_set_style_local_bg_color(s_lv_bar_blue, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_BLUE);
    lv_obj_set_style_local_bg_opa(s_lv_bar_blue, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
}

static void update_color_code(int red, int green, int blue)
{
    static int last_red = -1;
    static int last_green = -1;
    static int last_blue = -1;
    char str[24];

    lv_obj_move_foreground(s_lv_container_rgb);

    if (red != last_red)
    {
        lv_bar_set_value(s_lv_bar_red, red, LV_ANIM_OFF);
        snprintf(str, sizeof(str), "#ff0000 R: %d", red);
        lv_label_set_text(s_lv_label_red, str);
        last_red = red;
    }

    if (green != last_green)
    {
        lv_bar_set_value(s_lv_bar_green, green, LV_ANIM_OFF);
        snprintf(str, sizeof(str), "#00ff00 G: %d", green);
        lv_label_set_text(s_lv_label_green, str);
        last_green = green;
    }

    if (blue != last_blue)
    {
        lv_bar_set_value(s_lv_bar_blue, blue, LV_ANIM_OFF);
        snprintf(str, sizeof(str), "#0000ff B: %d", blue);
        lv_label_set_text(s_lv_label_blue, str);
        last_blue = blue;
    }
}

static void find_color(void *arg)
{
    ESP_LOGI(TAG, "Starting color_code_tft");

    sensor_t *s = esp_camera_sensor_get();
    int64_t last_fps_us = esp_timer_get_time();
    int frame_count = 0;

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
            int red = 0;
            int green = 0;
            int blue = 0;

#if COLOR_CODE_FULL_FEATURES
            Mat src;
            rgb565_be_to_bgr888(fb, src);
            Vec3b center = src.at<Vec3b>(src.rows / 2, src.cols / 2);
            blue = center[0];
            green = center[1];
            red = center[2];

            std::vector<Mat> bgr_planes;
            split(src, bgr_planes);
            float range[] = {0, 256};
            const float *hist_range = {range};
            Mat b_hist;
            Mat g_hist;
            Mat r_hist;
            calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &kHistSize, &hist_range, true, false);
            calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &kHistSize, &hist_range, true, false);
            calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &kHistSize, &hist_range, true, false);
            draw_histogram(b_hist, g_hist, r_hist, src);
            draw_center_mark(src);
            update_camera_image(src);
#else
            read_center_rgb(fb, &red, &green, &blue);
            draw_center_mark_rgb565_be(fb);
            display_camera_rgb565_be(fb);
#endif

            update_color_code(red, green, blue);
            frame_count++;

            const int64_t now = esp_timer_get_time();
            if (now - last_fps_us >= 1000000)
            {
                const float fps = (float)frame_count * 1000000.0f / (float)(now - last_fps_us);
                ESP_LOGI(TAG, "fps=%.2f rgb=(%d,%d,%d)", fps, red, green, blue);
                frame_count = 0;
                last_fps_us = now;
            }

            esp_camera_fb_return(fb);
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

extern "C" void color_code_tft_start(void)
{
    ESP_LOGI(TAG, "Starting main");

    app_camera_init();
    lvgl_init();
    gui_screen();

    xTaskCreatePinnedToCore(find_color, "color_code_tft", 1024 * 7, nullptr, 5, nullptr, 0);
}
