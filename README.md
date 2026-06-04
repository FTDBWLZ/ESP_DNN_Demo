# ESP-DNN TFT AI Demo Cluster

基于 ESP32-S3 的深度学习 AI 视觉演示项目，使用 ESP-DL（Deep Learning）框架，在 TFT 显示屏上实时运行多种 AI 推理任务。

## 功能概述

本项目集成了 4 种 AI 视觉演示模式，通过摄像头采集图像并在 TFT 屏幕上实时显示推理结果：

| 模式 | 应用名称 | 描述 |
|------|----------|------|
| 1 | Color Code TFT | 颜色识别检测 |
| 2 | Palm Keypoint TFT | 手掌关键点检测 |
| 3 | Face Detect TFT | 人脸检测 |
| 4 | Body Detect TFT | 人体/行人检测 |

默认应用为模式 4（人体检测）。

## 硬件要求

- **芯片**: ESP32-S3
- **摄像头**: 兼容 ESP32-CAM 接口的摄像头模块
- **显示屏**: SPI TFT LCD（通过 LVGL 驱动）
- **PSRAM**: 需启用（模型推理需要较大内存）

## 技术栈

- **框架**: ESP-IDF v5.5.4
- **深度学习**: [ESP-DL](https://github.com/espressif/esp-dl) v3.3.2
- **GUI 库**: LVGL
- **DSP 库**: esp-dsp v1.8.0
- **图像处理**: esp_new_jpeg v0.6.1, dl_fft v0.4.0

## 项目结构

```
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

## 构建

### 前置条件

1. 安装 [ESP-IDF v5.5+](https://docs.espressif.com/projects/esp-idf/) 并正确配置环境变量
2. 确保 `IDF_PATH` 环境变量指向 ESP-IDF 安装目录

### 编译

```powershell
# 使用项目自带的 IDF 环境脚本
cmd /c tools\idf_env.cmd idf.py build

# 或使用共享的 IDF 环境
cmd /c D:\IDF_PRO\IDF_PROJECT\tools\idf_env.cmd idf.py build
```

### 切换应用模式

通过 `ACTIVE_APP` 编译选项选择演示模式：

```powershell
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=1 reconfigure build   # 颜色识别
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=2 reconfigure build   # 手掌关键点
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=3 reconfigure build   # 人脸检测
cmd /c tools\idf_env.cmd idf.py -DACTIVE_APP=4 reconfigure build   # 人体检测
```

### 烧录

```powershell
cmd /c tools\idf_env.cmd idf.py -p <端口> flash monitor
```

## 分区表

| 分区名   | 类型 | 子类型  | 偏移     | 大小     |
|----------|------|---------|----------|----------|
| nvs      | data | nvs     | 0x9000   | 0x5000   |
| otadata  | data | ota     | 0xe000   | 0x2000   |
| app0     | app  | ota_0   | 0x10000  | 0xA00000 |
| spiffs   | data | spiffs  | 0xA10000 | 0x5E0000 |
| coredump | data | coredump| 0xFF0000 | 0x10000  |

## 许可证

各组件分别遵循其原有的许可证。ESP-DL、esp-dsp 等组件版权归 Espressif 所有。
