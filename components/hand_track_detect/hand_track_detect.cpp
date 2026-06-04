#include "hand_track_detect.h"

#include <algorithm>
#include <list>

#include "dl_detect_define.hpp"
#include "dl_image_define.hpp"
#include "hand_detect.hpp"

static HandDetect *s_detector;

static HandDetect *ensure_detector(void)
{
    if (s_detector == nullptr) {
        s_detector = new HandDetect(HandDetect::ESPDET_PICO_224_224_HAND, true);
    }
    return s_detector;
}

static bool fill_best_result(std::list<dl::detect::result_t> &results, hand_track_result_t *result)
{
    dl::detect::result_t *best = nullptr;
    int best_area = -1;

    for (auto &prediction : results) {
        if (prediction.box.size() < 4) {
            continue;
        }

        int area = (prediction.box[2] - prediction.box[0] + 1) * (prediction.box[3] - prediction.box[1] + 1);
        if (area > best_area) {
            best_area = area;
            best = &prediction;
        }
    }

    if (best == nullptr || result == nullptr) {
        return best != nullptr;
    }

    result->x1 = best->box[0];
    result->y1 = best->box[1];
    result->x2 = best->box[2];
    result->y2 = best->box[3];
    result->target_x = (best->box[0] + best->box[2]) / 2;
    result->target_y = (best->box[1] + best->box[3]) / 2;
    result->score = best->score;
    result->keypoint_count = 0;
    result->has_model_keypoints = false;

    if (best->keypoint.size() >= 2) {
        int point_count = std::min(static_cast<int>(best->keypoint.size() / 2), HAND_TRACK_MAX_KEYPOINTS);
        for (int i = 0; i < point_count * 2; i++) {
            result->keypoints[i] = best->keypoint[i];
        }
        result->keypoint_count = point_count;
        result->has_model_keypoints = true;
    }

    return true;
}

extern "C" bool hand_track_detect_process_bgr888(uint8_t *pixels, size_t width, size_t height, hand_track_result_t *result)
{
    HandDetect *detector = ensure_detector();
    dl::image::img_t img = {
        .data = pixels,
        .width = static_cast<uint16_t>(width),
        .height = static_cast<uint16_t>(height),
        .pix_type = dl::image::DL_IMAGE_PIX_TYPE_BGR888,
    };

    std::list<dl::detect::result_t> &results = detector->run(img);
    return fill_best_result(results, result);
}
