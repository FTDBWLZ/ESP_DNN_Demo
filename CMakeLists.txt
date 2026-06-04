# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly.
cmake_minimum_required(VERSION 3.16)

set(COMPONENTS
    main
    color_code_tft
    palm_keypoint_tft
    face_detect_tft
    body_detect_tft
    tft_camera_common
    hand_track_detect
    hand_detect
    face_detect
    human_face_detect
    pose_detect
    pedestrian_detect
    lvgl_gui
    lcd
    esp32_camera
    esp-dl
    esp_psram
    spi_flash
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESP_DNN)
