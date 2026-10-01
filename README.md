# ESP32 CN Font

**语言 / Language:** [中文](#中文) | [English](#english)

---

<a name="中文"></a>
# 中文

一个用于 **ESP32 + TFT_eSPI** 的中英文混合显示库，支持内置 8x16 ASCII 字库、内置中文备用字符，以及可选的 SD 卡外部字库加载。

## 功能特性

- **中英文混合排版**：一个 `drawMixedText()` 函数即可同时绘制中文、英文、数字、标点。
- **内置 8x16 ASCII 字库**：数字、大小写字母、标点全部内置，无需外部文件。
- **内置中文备用字符**：内置少量测试用汉字（你、好、世、界、中、文），开箱即用。
- **SD 卡字库可选加载**：支持从 SD 卡加载完整字库文件，自动优先使用 SD 字库，找不到时回退到内置字库。
- **全角标点支持**：ASCII 标点可自动映射为全角标点，同时支持中文特有标点（《》""''…—￥ 等）。
- **颜色反相开关**：通过一个全局布尔变量即可控制颜色是否取反，适配不同屏幕驱动。
```
## 目录结构
ESP32_CN_Font/
├── src/
│ ├── cn.h # 中文字库 + 混合绘制入口
│ ├── FontManager.h # 字库管理器
│ └── screen.h # 屏幕基础函数 + 颜色定义
├── examples/
│ └── test/
│ └── test.ino # 示例程序
├── library.properties # Arduino 库元数据
└── README.md

```

## 快速开始

### 1. 依赖

- ESP32 (Arduino core)
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
- `SD_MMC`（ESP32 内置库）

### 2. 引入头文件

```
#include <Arduino.h>
#include <SD_MMC.h>
#include <driver/ledc.h>
#include "screen.h"
#include "FontManager.h"
#include "cn.h"

// ============================================================
// Pin config (user defined)
// ============================================================
#define SD_CLK    39
#define SD_CMD    38
#define SD_D0     40
#define TFT_BL    10

// ============================================================
// Globals
// ============================================================
TFT_eSPI tft = TFT_eSPI();
SemaphoreHandle_t sdMutex = NULL;
bool g_colorInvert = true;
// ============================================================
// Backlight control
// ============================================================
void setBacklight(uint8_t brightness) {
    #ifdef TFT_BL
        ledc_timer_config_t timer_conf = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_8_BIT,
            .timer_num = LEDC_TIMER_0,
            .freq_hz = 5000,
            .clk_cfg = LEDC_AUTO_CLK
        };
        ledc_timer_config(&timer_conf);
        
        ledc_channel_config_t channel_conf = {
            .gpio_num = TFT_BL,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_0,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = brightness,
            .hpoint = 0
        };
        ledc_channel_config(&channel_conf);
    #endif
}

// ============================================================
// Init SD card
// ============================================================
bool initSDCard() {
    SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0);
    delay(100);
    for (int attempt = 0; attempt < 3; attempt++) {
        if (attempt > 0) {
            delay(500);
            SD_MMC.end();
            delay(100);
            SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0);
            delay(100);
        }
        if (SD_MMC.begin("/sdcard", true, false, 20000000)) {
            return true;
        }
    }
    return false;
}

void setup() {
    g_colorInvert = true;
    
    sdMutex = xSemaphoreCreateMutex();
    setBacklight(200);
    tft.init();
    tft.setRotation(1);
    
    initSDCard();
    initInternalFont();
    fontManager.loadSDFont("/font/font.bin");
    drawtext(20, 20, "你好，世界！Hello World!", COLOR_WHITE, 2, COLOR_ORANGE);
}
void loop() {
    delay(1000);
    yield();
}
```
### 3. 你的项目结构

```

ESP32_CN_Font/
├─ cn.h # 中文字库 + 混合绘制入口
├─ FontManager.h # 字库管理器
├─ screen.h # 屏幕基础函数 + 颜色定义
└─ test.ino # 示例程序

```
主要 API
函数	说明
```
initInternalFont()	//注册内置中文字库
fontManager.loadSDFont(path)	//从 SD 卡加载字库
drawtext(x, y, str, color, size, bgcolor)	//绘制中英混合文本
drawtext8x16(x, y, c, color, size, bgcolor)	
drawtext5x7(x, y, d, color, size)	
fixColor(color)	//根据开关返回颜色
```
颜色反相开关
在 setup() 里改一行即可：

```
g_colorInvert = false;   // false = 不取反
                         // true  = 取反
```                  
SD 卡字库格式
路径：/font/font.bin

汉字区：UTF-8 码点 E4-E9，每字 32 字节 (16x16)

ASCII 区：从偏移 6 * 4096 * 32 开始，每字符 16 字节 (8x16)

许可证
本项目基于 [MIT License](LICENSE) 开源。

致谢
显示驱动：TFT_eSPI

字体数据：自制

<a name="english"></a>
# English

A mixed Chinese/English text display library for **ESP32 + TFT_eSPI**. It supports a built-in 8x16 ASCII font, built-in Chinese fallback characters, and optional external font loading from an SD card.

## Features

- **Mixed Chinese/English layout**: a single `drawMixedText()` call can draw Chinese, English, digits, and punctuation together.
- **Built-in 8x16 ASCII font**: digits, upper/lower case letters, and punctuation are all built in, no external file needed.
- **Built-in Chinese fallback characters**: a few test characters (你、好、世、界、中、文) work out of the box.
- **Optional SD card font loading**: supports loading a full font file from an SD card. The SD font is used with priority, and falls back to the internal font if not found.
- **Full-width punctuation support**: ASCII punctuation can be automatically mapped to full-width punctuation, and native Chinese punctuation is also supported (《》""''…—￥ etc.).
- **Color inversion switch**: a single global boolean controls whether colors are inverted, adapting to different display drivers.

## Directory Structure

```
ESP32_CN_Font/
├── src/
│ ├── cn.h # Chinese font + mixed draw entry
│ ├── FontManager.h # Font manager
│ └── screen.h # Screen basics + color definitions
├── examples/
│ └── test/
│ └── test.ino # Example sketch
├── library.properties # Arduino library metadata
└── README.md
```
## Quick Start

### 1. Dependencies

- ESP32 (Arduino core)
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
- `SD_MMC` (ESP32 built-in)

### 2. Include headers
```
#include <Arduino.h>
#include <SD_MMC.h>
#include <driver/ledc.h>
#include "screen.h"
#include "FontManager.h"
#include "cn.h"

// ============================================================
// Pin config (user defined)
// ============================================================
#define SD_CLK 39
#define SD_CMD 38
#define SD_D0 40
#define TFT_BL 10

// ============================================================
// Globals
// ============================================================
TFT_eSPI tft = TFT_eSPI();
SemaphoreHandle_t sdMutex = NULL;
bool g_colorInvert = true;

// ============================================================
// Backlight control
// ============================================================
void setBacklight(uint8_t brightness) {
#ifdef TFT_BL
ledc_timer_config_t timer_conf = {
.speed_mode = LEDC_LOW_SPEED_MODE,
.duty_resolution = LEDC_TIMER_8_BIT,
.timer_num = LEDC_TIMER_0,
.freq_hz = 5000,
.clk_cfg = LEDC_AUTO_CLK
};
ledc_timer_config(&timer_conf);

ledc_channel_config_t channel_conf = {
.gpio_num = TFT_BL,
.speed_mode = LEDC_LOW_SPEED_MODE,
.channel = LEDC_CHANNEL_0,
.intr_type = LEDC_INTR_DISABLE,
.timer_sel = LEDC_TIMER_0,
.duty = brightness,
.hpoint = 0
};
ledc_channel_config(&channel_conf);
#endif
}

// ============================================================
// Init SD card
// ============================================================
bool initSDCard() {
SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0);
delay(100);
for (int attempt = 0; attempt < 3; attempt++) {
if (attempt > 0) {
delay(500);
SD_MMC.end();
delay(100);
SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0);
delay(100);
}
if (SD_MMC.begin("/sdcard", true, false, 20000000)) {
return true;
}
}
return false;
}

void setup() {
g_colorInvert = true;

sdMutex = xSemaphoreCreateMutex();
setBacklight(200);
tft.init();
tft.setRotation(1);

initSDCard();
initInternalFont();
fontManager.loadSDFont("/font/font.bin");
drawtext(20, 20, "你好，世界！Hello World!", COLOR_WHITE, 2, COLOR_ORANGE);
}
void loop() {
delay(1000);
yield();
}

```

### 3. Your project structure

```
ESP32_CN_Font/
├─ cn.h # Chinese font + mixed draw entry
├─ FontManager.h # Font manager
├─ screen.h # Screen basics + color definitions
└─ test.ino # Example sketch

```

## Main API
```

| Function | Description |
|----------|-------------|
| `initInternalFont()` | Register the internal Chinese font |
| `fontManager.loadSDFont(path)` | Load font from SD card |
| `drawtext(x, y, str, color, size, bgcolor)` | Draw mixed Chinese/English text |
| `drawtext8x16(x, y, c, color, size, bgcolor)` | Draw 8x16 ASCII text |
| `drawtext5x7(x, y, d, color, size)` | Draw 5x7 ASCII text |
| `fixColor(color)` | Return color according to the invert switch |
```
## Color Invert Switch

Change one line in `setup()`:
```
g_colorInvert = false; // false = no invert
// true = invert
```             

## SD Font Format

- Path: `/font/font.bin`
- CJK region: UTF-8 code points `E4-E9`, 32 bytes per char (16x16)
- ASCII region: starts at offset `6 * 4096 * 32`, 16 bytes per char (8x16)

## License

This project is licensed under the [MIT License](LICENSE).

## Credits

- Display driver: TFT_eSPI
- Font data: self-made


---
