#include "pose_detect.h"

#include <list>

#include "dl_detect_define.hpp"
#include "dl_image_define.hpp"
#include "pedestrian_detect.hpp"

static bool s_pose_detect_enabled;
static PedestrianDetect *s_detector;

static PedestrianDetect *ensure_detector(void)
{
    if (s_detector == nullptr) {
        s_detector = new PedestrianDetect(PedestrianDetect::PICO_S8_V1, true);
    }
    return s_detector;
}

static void compute_tracking_point(const dl::detect::result_t &prediction, int *target_x, int *target_y)
{
    int box_width = prediction.box[2] - prediction.box[0];
    int box_height = prediction.box[3] - prediction.box[1];

    *target_x = prediction.box[0] + (box_width / 2);
    *target_y = prediction.box[1] + (box_height / 3);
}

static bool is_full_frame_false_positive(const dl::detect::result_t &prediction, size_t width, size_t height)
{
    if (prediction.box.size() < 4 || width == 0 || height == 0) {
        return true;
    }

    int box_width = prediction.box[2] - prediction.box[0] + 1;
    int box_height = prediction.box[3] - prediction.box[1] + 1;
    int frame_width = static_cast<int>(width);
    int frame_height = static_cast<int>(height);

    bool touches_all_edges = prediction.box[0] <= 1 &&
                             prediction.box[1] <= 1 &&
                             prediction.box[2] >= frame_width - 2 &&
                             prediction.box[3] >= frame_height - 2;
    bool covers_most_frame = box_width * 100 >= frame_width * 92 &&
                             box_height * 100 >= frame_height * 92;

    return touches_all_edges && covers_most_frame;
}

static bool fill_best_result(std::list<dl::detect::result_t> &results, pose_detect_result_t *result, size_t width, size_t height)
{
    dl::detect::result_t *best = nullptr;
    int best_area = -1;

    for (auto &prediction : results) {
        if (prediction.box.size() < 4) {
            continue;
        }
        if (is_full_frame_false_positive(prediction, width, height)) {
            continue;
        }

        int area = (prediction.box[2] - prediction.box[0] + 1) * (prediction.box[3] - prediction.box[1] + 1);
        if (area > best_area) {
            best_area = area;
            best = &prediction;
        }
    }

    if (best == nullptr) {
        return false;
    }
    if (result == nullptr) {
        return true;
    }

    result->x1 = best->box[0];
    result->y1 = best->box[1];
    result->x2 = best->box[2];
    result->y2 = best->box[3];
    result->score = best->score;
    compute_tracking_point(*best, &result->target_x, &result->target_y);
    return true;
}

extern "C" bool pose_detect_get_enabled(void)
{
    return s_pose_detect_enabled;
}

extern "C" void pose_detect_set_enabled(bool enabled)
{
    s_pose_detect_enabled = enabled;
}

extern "C" bool pose_detect_process_bgr888(uint8_t *pixels, size_t width, size_t height, pose_detect_result_t *result)
{
    PedestrianDetect *detector = ensure_detector();
    dl::image::img_t img = {
        .data = pixels,
        .width = static_cast<uint16_t>(width),
        .height = static_cast<uint16_t>(height),
        .pix_type = dl::image::DL_IMAGE_PIX_TYPE_BGR888,
    };

    std::list<dl::detect::result_t> &results = detector->run(img);
    return fill_best_result(results, result, width, height);
}
