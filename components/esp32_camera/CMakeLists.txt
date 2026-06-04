idf_component_register(
    SRCS
        "esp32_camera_compat.c"
    INCLUDE_DIRS
        "include"
    REQUIRES
        driver
        esp_driver_i2c
        esp_driver_ledc
        esp_ringbuf
        esp_timer
        esp_system
        esp_hw_support
        esp_psram
        hal
        soc
        nvs_flash
)

set(arduino_sdk_dir "$ENV{USERPROFILE}/.platformio/packages/framework-arduinoespressif32/tools/sdk/${IDF_TARGET}")

target_include_directories(${COMPONENT_LIB} PUBLIC
    "${arduino_sdk_dir}/include/esp32-camera/driver/include"
    "${arduino_sdk_dir}/include/esp32-camera/conversions/include"
)

target_link_directories(${COMPONENT_LIB} PUBLIC
    "${arduino_sdk_dir}/lib"
)

target_link_libraries(${COMPONENT_LIB} PUBLIC
    esp32-camera
    "${CMAKE_BINARY_DIR}/esp-idf/driver/libdriver.a"
    "${CMAKE_BINARY_DIR}/esp-idf/esp_driver_i2c/libesp_driver_i2c.a"
    "${CMAKE_BINARY_DIR}/esp-idf/esp_driver_ledc/libesp_driver_ledc.a"
    "${CMAKE_BINARY_DIR}/esp-idf/esp_hw_support/libesp_hw_support.a"
    "${CMAKE_BINARY_DIR}/esp-idf/esp_ringbuf/libesp_ringbuf.a"
    "${CMAKE_BINARY_DIR}/esp-idf/hal/libhal.a"
    "${CMAKE_BINARY_DIR}/esp-idf/soc/libsoc.a"
)
