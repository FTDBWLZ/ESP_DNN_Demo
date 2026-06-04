#include "body_detect_tft.h"
#include "color_code_tft.h"
#include "face_detect_tft.h"
#include "palm_keypoint_tft.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ACTIVE_APP_COLOR_CODE_TFT 1
#define ACTIVE_APP_PALM_KEYPOINT_TFT 2
#define ACTIVE_APP_FACE_DETECT_TFT 3
#define ACTIVE_APP_BODY_DETECT_TFT 4

#ifndef ACTIVE_APP
#define ACTIVE_APP ACTIVE_APP_BODY_DETECT_TFT
#endif

void app_main(void)
{
#if ACTIVE_APP == ACTIVE_APP_COLOR_CODE_TFT
    color_code_tft_start();
#elif ACTIVE_APP == ACTIVE_APP_PALM_KEYPOINT_TFT
    palm_keypoint_tft_start();
#elif ACTIVE_APP == ACTIVE_APP_FACE_DETECT_TFT
    face_detect_tft_start();
#elif ACTIVE_APP == ACTIVE_APP_BODY_DETECT_TFT
    body_detect_tft_start();
#else
#error "Unknown ACTIVE_APP selection"
#endif

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
