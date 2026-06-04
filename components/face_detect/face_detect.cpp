#include "face_detect.h"

#include <list>

#include "dl_image_define.hpp"
#include "human_face_detect.hpp"

static bool s_face_detect_enabled;
static HumanFaceDetect *s_detector;

static HumanFaceDetect *ensure_detector(void)
{
    if (s_detector == nullptr) {
        s_detector = new HumanFaceDetect(HumanFaceDetect::MSRMNP_S8_V1, true);
    }
    return s_detector;
}

static bool fill_best_box(std::list<dl::detect::result_t> &results, face_detect_box_t *box)
{
    if (results.empty()) {
        return false;
    }

    if (box == nullptr) {
        return true;
    }

    int best_area = -1;
    for (auto &prediction : results) {
        if (prediction.box.size() < 4) {
            continue;
        }

        int area = (prediction.box[2] - prediction.box[0] + 1) * (prediction.box[3] - prediction.box[1] + 1);
        if (area > best_area) {
            best_area = area;
            box->x1 = prediction.box[0];
            box->y1 = prediction.box[1];
            box->x2 = prediction.box[2];
            box->y2 = prediction.box[3];
        }
    }

    return best_area >= 0;
}

static bool run_face_detect(void *pixels, size_t width, size_t height, dl::image::pix_type_t pix_type, face_detect_box_t *box)
{
    HumanFaceDetect *detector = ensure_detector();
    dl::image::img_t img = {
        .data = pixels,
        .width = static_cast<uint16_t>(width),
        .height = static_cast<uint16_t>(height),
        .pix_type = pix_type,
    };

    std::list<dl::detect::result_t> &results = detector->run(img);
    return fill_best_box(results, box);
}

extern "C" bool face_detect_get_enabled(void)
{
    return s_face_detect_enabled;
}

extern "C" void face_detect_set_enabled(bool enabled)
{
    s_face_detect_enabled = enabled;
}

extern "C" bool face_detect_process_rgb565(uint16_t *pixels, size_t width, size_t height)
{
    return run_face_detect(pixels, width, height, dl::image::DL_IMAGE_PIX_TYPE_RGB565LE, nullptr);
}

extern "C" bool face_detect_process_bgr888(uint8_t *pixels, size_t width, size_t height)
{
    return run_face_detect(pixels, width, height, dl::image::DL_IMAGE_PIX_TYPE_BGR888, nullptr);
}

extern "C" bool face_detect_process_bgr888_with_box(uint8_t *pixels, size_t width, size_t height, face_detect_box_t *box)
{
    return run_face_detect(pixels, width, height, dl::image::DL_IMAGE_PIX_TYPE_BGR888, box);
}
