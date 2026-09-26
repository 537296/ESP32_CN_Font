// ============================================================
// User sketch - include 3 libs to use
// ============================================================

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

// ============================================================
// Main
// ============================================================
void setup() {
    Serial.begin(115200);
    delay(100);
    g_colorInvert = true; 
    // 1. create mutex
    sdMutex = xSemaphoreCreateMutex();
    
    // 2. init screen
    setBacklight(200);
    tft.init();
    tft.setRotation(1);
    
    // 3. init SD card
    initSDCard();
    
    // 4. init internal font
    initInternalFont();
    
    // 5. try load SD font
    fontManager.loadSDFont("/font/font.bin");
    
    // 6. draw text
    fillscreen(COLOR_BLACK);
    
    int y = 20;
    drawMixedText(20, y, "你好，世界！Hello World!", COLOR_WHITE, 2);
    y += 40;
    drawMixedText(20, y, "这是 ESP32S3", COLOR_CYAN, 2);
    y += 40;
    drawMixedText(20, y, "ABCDEFG 1234567890", COLOR_YELLOW, 2);
    y += 40;
    drawMixedText(20, y, "Punct: , . ! ? : ; ( ) [ ]", COLOR_GREEN, 1);
    y += 25;
    drawMixedText(20, y, "Symbol: =+-*/%#@$~￥<>—…’‘”“》《＇＂", COLOR_ORANGE, 1);
    
    Serial.println("Init done");
}

void loop() {
    delay(1000);
    yield();
}