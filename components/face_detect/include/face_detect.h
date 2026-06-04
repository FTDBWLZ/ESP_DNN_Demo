#ifndef FACE_DETECT_H
#define FACE_DETECT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
} face_detect_box_t;

bool face_detect_get_enabled(void);
void face_detect_set_enabled(bool enabled);
bool face_detect_process_rgb565(uint16_t *pixels, size_t width, size_t height);
bool face_detect_process_bgr888(uint8_t *pixels, size_t width, size_t height);
bool face_detect_process_bgr888_with_box(uint8_t *pixels, size_t width, size_t height, face_detect_box_t *box);

#ifdef __cplusplus
}
#endif

#endif
