idf_component_register(
    SRCS "pose_detect.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp-dl pedestrian_detect
)
