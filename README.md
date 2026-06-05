# ESP-DNN TFT AI Demo Cluster

[中文](#中文) | [English](#english)

---

## 中文

基于 ESP32-S3 的深度学习 AI 视觉演示项目，使用 ESP-DL（Deep Learning）框架，在 TFT 显示屏上实时运行多种 AI 推理任务。

### 功能概述

本项目集成了 4 种 AI 视觉演示模式，通过摄像头采集图像并在 TFT 屏幕上实时显示推理结果：

| 模式 | 应用名称 | 描述 |
|------|----------|------|
| 1 | Color Code TFT | 颜色识别检测 |
| 2 | Palm Keypoint TFT | 手掌关键点检测 |
| 3 | Face Detect TFT | 人脸检测 |
| 4 | Body Detect TFT | 人体/行人检测 |

默认应用为模式 4（人体检测）。

### 项目特点

- 基于 ESP32-S3 的端侧 AI 推理演示，不依赖 PC 或云端推理。
- 集成摄像头采集、TFT 实时显示和 AI 检测结果绘制。
- 通过 `ACTIVE_APP` 编译选项切换不同视觉检测 Demo。
- 适合作为 ESP-DL、LVGL、ESP32 摄像头和嵌入式视觉部署的学习参考。
- 项目结构按功能组件拆分，便于单独调试、替换或扩展不同检测模块。

### 演示效果

Demo 图片将在后续补充。建议展示以下内容：

| Color Code TFT | Palm Keypoint TFT | Face Detect TFT | Body Detect TFT |
|----------------|-------------------|-----------------|-----------------|
| 待补充 | 待补充 | 待补充 | 待补充 |

### 硬件要求

- **芯片**: ESP32-S3
- **摄像头**: 兼容 ESP32-CAM 接口的摄像头模块
- **显示屏**: SPI TFT LCD（通过 LVGL 驱动）
- **PSRAM**: 需启用（模型推理需要较大内存）

### 已测试 / 推荐硬件

> 具体引脚和外设配置请以源码中的配置文件为准。

- **开发板**: ESP32-S3 开发板或自制 ESP32-S3 PCB
- **摄像头**: ESP32 Camera 接口兼容模块
- **显示屏**: SPI TFT LCD，推荐使用已适配 LVGL 的 ST7735 / ST7789 类屏幕
- **存储**: 建议使用带 PSRAM 的 ESP32-S3 模组

### 技术栈

- **框架**: ESP-IDF v5.5.4
- **深度学习**: [ESP-DL](https://github.com/espressif/esp-dl) v3.3.2
- **GUI 库**: LVGL
- **DSP 库**: esp-dsp v1.8.0
- **图像处理**: esp_new_jpeg v0.6.1, dl_fft v0.4.0

### 项目结构

```text
Esp_DNN_Demo/
├── main/                   # 主程序入口
│   ├── main.c              # 应用选择与调度
│   └── CMakeLists.txt      # 主组件构建配置
├── components/             # 自定义组件
│   ├── body_detect_tft/    # 人体检测 TFT 展示
│   ├── color_code_tft/     # 颜色识别 TFT 展示
│   ├── face_detect/        # 人脸检测
│   ├── face_detect_tft/    # 人脸检测 TFT 展示
│   ├── hand_detect/        # 手部检测
│   ├── hand_track_detect/  # 手部追踪检测
│   ├── human_face_detect/  # 人面检测
│   ├── palm_keypoint_tft/  # 手掌关键点 TFT 展示
│   ├── pedestrian_detect/  # 行人检测
│   ├── pose_detect/        # 姿态检测
│   ├── tft_camera_common/  # TFT 相机通用工具
│   ├── lcd/                # LCD/SPI 显示驱动
│   ├── lvgl_gui/           # LVGL GUI 配置
│   ├── esp32_camera/       # ESP32 摄像头驱动适配
│   └── esp-dl/             # ESP-DL 深度学习库
├── managed_components/     # ESP-IDF 托管依赖
├── tools/                  # 构建工具脚本
├── partitions.csv          # 分区表配置
├── CMakeLists.txt          # 顶层 CMake 配置
└── dependencies.lock       # 依赖锁定文件
```

### 构建

#### 前置条件

1. 安装 [ESP-IDF v5.5+](https://docs.espressif.com/projects/esp-idf/) 并正确配置环境变量
2. 确保 `IDF_PATH` 环境变量指向 ESP-IDF 安装目录

#### 编译

```powershell
# 使用项目自带的 IDF 环境脚本
cmd /c tools\idf_env.cmd idf.py build

# 或使用共享的 IDF 环境
cmd /c D:\IDF_PRO\IDF_PROJECT\tools\idf_env.cmd idf.py build
```

#### 切换应用模式

通过 `ACTIVE_APP` 编译选项选择演示模式：

```powershell
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=1 reconfigure build   # 颜色识别
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=2 reconfigure build   # 手掌关键点
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=3 reconfigure build   # 人脸检测
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=4 reconfigure build   # 人体检测
```

#### 烧录

```powershell
cmd /c tools\idf_env.cmd idf.py -p <端口> flash monitor
```

烧录完成后，固件会启动当前编译选中的 Demo。默认情况下会进入人体检测模式，并在 TFT 屏幕上显示摄像头预览和检测结果。

### 常见问题

#### 编译失败

请确认 ESP-IDF v5.5+ 已正确安装，并且 `IDF_PATH` 环境变量指向有效的 ESP-IDF 路径。

#### 摄像头初始化失败

请检查摄像头模块供电、排线方向、引脚配置以及摄像头型号是否与当前驱动适配。

#### 屏幕无显示或显示异常

请检查 SPI TFT 的供电、背光引脚、SPI 连接、屏幕驱动型号以及 LVGL 显示缓冲区配置。

#### 内存分配失败

请确认 PSRAM 已启用。AI 模型推理、摄像头帧缓存和 TFT 显示缓冲都会占用较多内存。

### 模型与组件说明

本项目使用 ESP-DL 及相关示例组件进行端侧视觉推理。不同检测 Demo 的模型文件、预处理流程和后处理逻辑分布在对应组件目录中。若需要替换模型，请同时检查输入尺寸、颜色格式、量化方式和后处理代码。

### 分区表

| 分区名   | 类型 | 子类型  | 偏移     | 大小     |
|----------|------|---------|----------|----------|
| nvs      | data | nvs     | 0x9000   | 0x5000   |
| otadata  | data | ota     | 0xe000   | 0x2000   |
| app0     | app  | ota_0   | 0x10000  | 0xA00000 |
| spiffs   | data | spiffs  | 0xA10000 | 0x5E0000 |
| coredump | data | coredump| 0xFF0000 | 0x10000  |

### 后续计划

- 补充各 Demo 的实际运行截图或短视频。
- 补充更详细的硬件接线说明。
- 优化 TFT 刷新性能和界面展示效果。
- 增加更多嵌入式视觉检测 Demo。
- 整理模型替换和自定义训练部署流程。

### 许可证

各组件分别遵循其原有的许可证。ESP-DL、esp-dsp 等组件版权归 Espressif 所有。

---

## English

An ESP32-S3 deep-learning AI vision demo project based on the ESP-DL framework. It captures camera frames, runs multiple AI inference tasks, and displays real-time results on a TFT screen.

### Features

This project integrates four AI vision demo modes. Each mode captures images from the camera and displays inference results on the TFT screen in real time:

| Mode | Application | Description |
|------|-------------|-------------|
| 1 | Color Code TFT | Color recognition and detection |
| 2 | Palm Keypoint TFT | Palm keypoint detection |
| 3 | Face Detect TFT | Face detection |
| 4 | Body Detect TFT | Human body / pedestrian detection |

The default application is mode 4, the body detection demo.

### Highlights

- Runs edge AI inference directly on ESP32-S3 without PC-side or cloud inference.
- Combines camera capture, TFT real-time display, and AI detection result rendering.
- Supports switching between different vision demos through the `ACTIVE_APP` build option.
- Useful as a learning reference for ESP-DL, LVGL, ESP32 camera integration, and embedded vision deployment.
- Organized into functional components, making each detection module easier to debug, replace, or extend.

### Demo Preview

Demo images will be added later. Suggested preview items:

| Color Code TFT | Palm Keypoint TFT | Face Detect TFT | Body Detect TFT |
|----------------|-------------------|-----------------|-----------------|
| To be added | To be added | To be added | To be added |

### Hardware Requirements

- **Chip**: ESP32-S3
- **Camera**: Camera module compatible with the ESP32-CAM interface
- **Display**: SPI TFT LCD driven through LVGL
- **PSRAM**: Required, because model inference needs relatively large memory

### Tested / Recommended Hardware

> Please refer to the source configuration files for the actual pin and peripheral settings.

- **Board**: ESP32-S3 development board or custom ESP32-S3 PCB
- **Camera**: ESP32 Camera interface compatible module
- **Display**: SPI TFT LCD, preferably an LVGL-adapted ST7735 / ST7789-style panel
- **Memory**: ESP32-S3 module with PSRAM is recommended

### Tech Stack

- **Framework**: ESP-IDF v5.5.4
- **Deep Learning**: [ESP-DL](https://github.com/espressif/esp-dl) v3.3.2
- **GUI Library**: LVGL
- **DSP Library**: esp-dsp v1.8.0
- **Image Processing**: esp_new_jpeg v0.6.1, dl_fft v0.4.0

### Project Structure

```text
Esp_DNN_Demo/
├── main/                   # Main application entry
│   ├── main.c              # Application selection and dispatch
│   └── CMakeLists.txt      # Main component build configuration
├── components/             # Custom components
│   ├── body_detect_tft/    # Body detection TFT demo
│   ├── color_code_tft/     # Color recognition TFT demo
│   ├── face_detect/        # Face detection
│   ├── face_detect_tft/    # Face detection TFT demo
│   ├── hand_detect/        # Hand detection
│   ├── hand_track_detect/  # Hand tracking detection
│   ├── human_face_detect/  # Human face detection
│   ├── palm_keypoint_tft/  # Palm keypoint TFT demo
│   ├── pedestrian_detect/  # Pedestrian detection
│   ├── pose_detect/        # Pose detection
│   ├── tft_camera_common/  # Common TFT camera utilities
│   ├── lcd/                # LCD/SPI display driver
│   ├── lvgl_gui/           # LVGL GUI configuration
│   ├── esp32_camera/       # ESP32 camera driver adapter
│   └── esp-dl/             # ESP-DL deep learning library
├── managed_components/     # ESP-IDF managed dependencies
├── tools/                  # Build helper scripts
├── partitions.csv          # Partition table configuration
├── CMakeLists.txt          # Top-level CMake configuration
└── dependencies.lock       # Dependency lock file
```

### Build

#### Prerequisites

1. Install [ESP-IDF v5.5+](https://docs.espressif.com/projects/esp-idf/) and configure the environment variables correctly.
2. Make sure the `IDF_PATH` environment variable points to the ESP-IDF installation directory.

#### Compile

```powershell
# Use the IDF environment script included in this project
cmd /c tools\idf_env.cmd idf.py build

# Or use a shared IDF environment
cmd /c D:\IDF_PRO\IDF_PROJECT\tools\idf_env.cmd idf.py build
```

#### Switch Application Mode

Use the `ACTIVE_APP` CMake option to select the demo mode:

```powershell
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=1 reconfigure build   # Color recognition
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=2 reconfigure build   # Palm keypoint
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=3 reconfigure build   # Face detection
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=4 reconfigure build   # Body detection
```

#### Flash

```powershell
cmd /c tools\idf_env.cmd idf.py -p <PORT> flash monitor
```

After flashing, the firmware starts the demo selected at build time. By default, it starts the body detection demo and displays the camera preview and detection results on the TFT screen.

### Troubleshooting

#### Build fails

Make sure ESP-IDF v5.5+ is installed correctly and that the `IDF_PATH` environment variable points to a valid ESP-IDF path.

#### Camera initialization fails

Check the camera power supply, ribbon cable direction, pin configuration, and whether the camera model is supported by the current driver.

#### Display is blank or abnormal

Check the SPI TFT power supply, backlight pin, SPI wiring, display driver model, and LVGL display buffer configuration.

#### Memory allocation fails

Make sure PSRAM is enabled. AI model inference, camera frame buffers, and TFT display buffers all require relatively large memory.

### Model and Component Notes

This project uses ESP-DL and related example components for edge vision inference. Model files, preprocessing, and postprocessing logic are distributed in the corresponding component directories. When replacing a model, check the input size, color format, quantization method, and postprocessing code together.

### Partition Table

| Name     | Type | Subtype | Offset   | Size     |
|----------|------|---------|----------|----------|
| nvs      | data | nvs     | 0x9000   | 0x5000   |
| otadata  | data | ota     | 0xe000   | 0x2000   |
| app0     | app  | ota_0   | 0x10000  | 0xA00000 |
| spiffs   | data | spiffs  | 0xA10000 | 0x5E0000 |
| coredump | data | coredump| 0xFF0000 | 0x10000  |

### Roadmap

- Add actual screenshots or short videos for each demo.
- Add more detailed hardware wiring documentation.
- Optimize TFT refresh performance and UI presentation.
- Add more embedded vision detection demos.
- Document the model replacement and custom training deployment workflow.

### License

Each component follows its original license. ESP-DL, esp-dsp, and related components are copyrighted by Espressif.
