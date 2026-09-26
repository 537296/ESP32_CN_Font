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
│ └── BasicTest/
│ └── BasicTest.ino # 示例程序
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
#include "screen.h"
#include "FontManager.h"
#include "cn.h"
3. 基本使用
cpp
TFT_eSPI tft = TFT_eSPI();

void setup() {
    Serial.begin(115200);

    // 是否取反颜色
    g_colorInvert = false;

    tft.init();
    tft.setRotation(1);

    // 初始化内部字库
    initInternalFont();

    // 可选：加载 SD 卡字库
    fontManager.loadSDFont("/font/font.bin");

    // 绘制混合文本
    fillscreen(COLOR_BLACK);
    drawMixedText(20, 20, "你好，世界！Hello World!", COLOR_WHITE, 2);
    drawMixedText(20, 60, "这是 ESP32S3", COLOR_CYAN, 2);
    drawMixedText(20, 100, "ABCDEFG 1234567890", COLOR_YELLOW, 2);
}
```
主要 API
函数	说明
```
initInternalFont()	//注册内置中文字库
fontManager.loadSDFont(path)	//从 SD 卡加载字库
drawMixedText(x, y, str, color, size)	//绘制中英混合文本
cn_drawChar(x, y, c, color, size)	//绘制 8x16 字母
cn_drawDigit(x, y, d, color, size)	//绘制 8x16 数字
drawPunctChar(x, y, idx, color, size)	//绘制 8x16 标点
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

A mixed Chinese/English text drawing library for ESP32 + TFT_eSPI. Supports built-in 8x16 ASCII fonts, built-in Chinese fallback characters, and optional SD card font loading.

Features
Mixed text layout: draw Chinese, English, digits and punctuation with a single drawMixedText() call.

Built-in 8x16 ASCII font: digits, upper/lower case letters and punctuation included, no external file needed.

Built-in Chinese fallback chars: a few test characters (你、好、世、界、中、文) work out of the box.

Optional SD card font: load a full font file from SD card. SD font has priority, falls back to internal font if missing.

Full-width punctuation: ASCII punctuation can be auto-mapped to full-width, plus native CJK punctuation (《》""''…—￥ etc.).

Color invert switch: a single global bool controls color inversion, adapting to different display drivers.

Directory Structure
```
ESP32_CN_Font/
├── src/
│   ├── cn.h              # Chinese font + mixed draw entry
│   ├── FontManager.h     # Font manager
│   └── screen.h          # Screen basics + colors
├── examples/
│   └── BasicTest/
│       └── BasicTest.ino # Example sketch
├── library.properties    # Arduino library metadata
└── README.md
```
Quick Start
1. Dependencies
ESP32 (Arduino core)

TFT_eSPI

SD_MMC (ESP32 built-in)

2. Include headers
```
#include <Arduino.h>
#include <SD_MMC.h>
#include "screen.h"
#include "FontManager.h"
#include "cn.h"
```
3. Basic usage
```
TFT_eSPI tft = TFT_eSPI();

void setup() {
    Serial.begin(115200);

    // whether to invert colors
    g_colorInvert = false;

    tft.init();
    tft.setRotation(1);

    // init internal font
    initInternalFont();

    // optional: load SD font
    fontManager.loadSDFont("/font/font.bin");

    // draw mixed text
    fillscreen(COLOR_BLACK);
    drawMixedText(20, 20, "你好，世界！Hello World!", COLOR_WHITE, 2);
    drawMixedText(20, 60, "这是 ESP32S3", COLOR_CYAN, 2);
    drawMixedText(20, 100, "ABCDEFG 1234567890", COLOR_YELLOW, 2);
}
```
Main API
Function	Description
```
initInternalFont()	//register internal Chinese font
fontManager.loadSDFont(path)	l//oad font from SD card
drawMixedText(x, y, str, color, size)	//draw mixed CN/EN text
cn_drawChar(x, y, c, color, size)	//draw 8x16 letter
cn_drawDigit(x, y, d, color, size)	//draw 8x16 digit
drawPunctChar(x, y, idx, color, size)	//draw 8x16 punctuation
fixColor(color)	//color with optional invert
```
Color Invert Switch
Change one line in setup():

```
g_colorInvert = false;   // false = no invert
                         // true  = invert
```             
SD Font Format
Path: /font/font.bin

CJK region: UTF-8 code points E4-E9, 32 bytes per char (16x16)

ASCII region: starts at offset 6 * 4096 * 32, 16 bytes per char (8x16)

License
This project is licensed under the [MIT License](LICENSE).

Credits
Display driver: TFT_eSPI

Font data: self-made


---
