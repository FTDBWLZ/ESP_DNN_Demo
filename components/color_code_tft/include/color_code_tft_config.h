#ifndef COLOR_CODE_TFT_CONFIG_H
#define COLOR_CODE_TFT_CONFIG_H

/*
 * 0: high-performance local preview path.
 *    RGB565 camera frames are drawn directly to TFT, with center color readout.
 *
 * 1: original full-feature path.
 *    Enables OpenCV BGR888 conversion, histogram drawing, center mark, and LVGL image display.
 */
#ifndef COLOR_CODE_FULL_FEATURES
#define COLOR_CODE_FULL_FEATURES 0
#endif

#endif
