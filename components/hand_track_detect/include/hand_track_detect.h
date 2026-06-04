#ifndef HAND_TRACK_DETECT_H
#define HAND_TRACK_DETECT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HAND_TRACK_MAX_KEYPOINTS 21

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
    int keypoint_count;
    int keypoints[HAND_TRACK_MAX_KEYPOINTS * 2];
    bool has_model_keypoints;
} hand_track_result_t;

bool hand_track_detect_process_bgr888(uint8_t *pixels, size_t width, size_t height, hand_track_result_t *result);

#ifdef __cplusplus
}
#endif

#endif
