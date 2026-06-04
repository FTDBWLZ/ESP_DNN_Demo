#ifndef POSE_DETECT_H
#define POSE_DETECT_H

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
    int target_x;
    int target_y;
    float score;
} pose_detect_result_t;

bool pose_detect_get_enabled(void);
void pose_detect_set_enabled(bool enabled);
bool pose_detect_process_bgr888(uint8_t *pixels, size_t width, size_t height, pose_detect_result_t *result);

#ifdef __cplusplus
}
#endif

#endif
