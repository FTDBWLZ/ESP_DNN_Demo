set(ACTIVE_APP "" CACHE STRING "Optional TFT demo selector. Valid values: 1=color, 2=palm, 3=face, 4=body. Empty uses main.c default.")
set_property(CACHE ACTIVE_APP PROPERTY STRINGS "" "1" "2" "3" "4")

idf_component_register(
    SRCS
        "main.c"
        "arduino_camera_shim.c"
    PRIV_REQUIRES
        color_code_tft
        palm_keypoint_tft
        face_detect_tft
        body_detect_tft
        esp32_camera
        esp_psram
        esp_timer
        esp_hw_support
        hal
        soc
    INCLUDE_DIRS
        "."
)

if(NOT ACTIVE_APP STREQUAL "")
    if(NOT ACTIVE_APP MATCHES "^(1|2|3|4)$")
        message(FATAL_ERROR "Invalid ACTIVE_APP '${ACTIVE_APP}'. Use 1=color, 2=palm, 3=face, 4=body.")
    endif()
    message(STATUS "Selected TFT app: ${ACTIVE_APP}")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE ACTIVE_APP=${ACTIVE_APP})
else()
    message(STATUS "Selected TFT app: main.c default")
endif()

target_link_libraries(${COMPONENT_LIB} INTERFACE
    "-u vPortEvaluateYieldFromISR"
    "-u gdma_get_channel_id"
    "${CMAKE_BINARY_DIR}/esp-idf/esp_driver_i2c/libesp_driver_i2c.a"
    "${CMAKE_BINARY_DIR}/esp-idf/hal/libhal.a"
    "${CMAKE_BINARY_DIR}/esp-idf/soc/libsoc.a")
