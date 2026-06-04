idf_component_register(
    SRCS
        "color_code_tft.cpp"
    INCLUDE_DIRS
        "include"
    REQUIRES
        tft_camera_common
)

file(STRINGS "include/color_code_tft_config.h" COLOR_CODE_FULL_FEATURES_LINE
     REGEX "^[ \t]*#define[ \t]+COLOR_CODE_FULL_FEATURES[ \t]+1")

if(COLOR_CODE_FULL_FEATURES_LINE)
    target_include_directories(${COMPONENT_LIB} PRIVATE "opencv")

    add_prebuilt_library(opencv_imgcodecs "opencv/libopencv_imgcodecs.a")
    add_prebuilt_library(libpng "opencv/3rdparty/liblibpng.a")
    add_prebuilt_library(libzlib "opencv/3rdparty/libzlib.a")
    add_prebuilt_library(opencv_imgproc "opencv/libopencv_imgproc.a")
    add_prebuilt_library(opencv_core "opencv/libopencv_core.a")
    add_prebuilt_library(ade "opencv/libade.a")

    target_link_libraries(${COMPONENT_LIB} PRIVATE opencv_imgcodecs)
    target_link_libraries(${COMPONENT_LIB} PRIVATE libpng)
    target_link_libraries(${COMPONENT_LIB} PRIVATE libzlib)
    target_link_libraries(${COMPONENT_LIB} PRIVATE opencv_imgproc)
    target_link_libraries(${COMPONENT_LIB} PRIVATE opencv_core)
    target_link_libraries(${COMPONENT_LIB} PRIVATE ade)
endif()
