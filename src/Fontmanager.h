// ============================================================
// FontManager.h - SD font manager + internal font fallback
// Version: 2.0
// ============================================================

#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include "screen.h"

#define FONT_DEBUG_ENABLE   1

#if FONT_DEBUG_ENABLE
    #define FONT_LOG(...) Serial.printf(__VA_ARGS__)
#else
    #define FONT_LOG(...)
#endif

// ============================================================
// Config
// ============================================================
#define FONT_FILE_PATH      "/font/font.bin"

struct InternalChar {
    const char* code;
    const unsigned char* data;
    uint8_t width;
    uint8_t height;
};

class FontManager {
private:
    uint8_t* sdFontData;
    uint32_t sdFontSize;
    bool sdFontLoaded;
    
    const InternalChar* internalTable;
    int internalCount;
    
    uint8_t* charBuffer;
    
public:
    FontManager() {
        sdFontData = NULL;
        sdFontSize = 0;
        sdFontLoaded = false;
        internalTable = NULL;
        internalCount = 0;
        charBuffer = NULL;
        charBuffer = (uint8_t*)malloc(32);
        FONT_LOG("FontManager init done\n");
    }
    
    ~FontManager() {
        if (sdFontData) { free(sdFontData); sdFontData = NULL; }
        if (charBuffer) { free(charBuffer); charBuffer = NULL; }
    }

    // Register internal font table
    void init(const InternalChar* table, int count) {
        internalTable = table;
        internalCount = count;
        FONT_LOG("Internal font registered: %d chars\n", count);
    }
    
    // Load SD font file into PSRAM/heap
    bool loadSDFont(const char* path = FONT_FILE_PATH) {
        FONT_LOG("Trying to load SD font: %s\n", path);
        
        if (!SD_MMC.cardSize()) {
            FONT_LOG(">SD card not ready\n");
            return false;
        }
        
        if (!SD_MMC.exists(path)) {
            FONT_LOG(">Font file not found: %s\n", path);
            return false;
        }
        
        File fontFile = SD_MMC.open(path);
        if (!fontFile) {
            FONT_LOG(">Cannot open font file\n");
            return false;
        }
        
        sdFontSize = fontFile.size();
        FONT_LOG("Font file size: %u bytes\n", (unsigned int)sdFontSize);
        
        sdFontData = (uint8_t*)ps_malloc(sdFontSize);
        if (!sdFontData) {
            sdFontData = (uint8_t*)malloc(sdFontSize);
        }
        
        if (!sdFontData) {
            FONT_LOG(">Memory alloc failed\n");
            fontFile.close();
            return false;
        }
        
        size_t readBytes = fontFile.read(sdFontData, sdFontSize);
        fontFile.close();
        
        if (readBytes != sdFontSize) {
            FONT_LOG(">Incomplete read\n");
            free(sdFontData);
            sdFontData = NULL;
            return false;
        }
        
        sdFontLoaded = true;
        FONT_LOG(">SD font loaded: %u bytes\n", (unsigned int)sdFontSize);
        printFontInfo();
        return true;
    }

    void printFontInfo() {
        if (!sdFontLoaded) {
            FONT_LOG("error FONT not found\n");
            return;
        }
        FONT_LOG("  Size: %u bytes\n", (unsigned int)sdFontSize);
        FONT_LOG("  Chars: %u\n", (unsigned int)(sdFontSize / 32));
        FONT_LOG("  Free heap: %u KB\n", (unsigned int)(esp_get_free_heap_size() / 1024));
        if (sdFontSize >= 4) {
            uint32_t magic = *(uint32_t*)sdFontData;
            FONT_LOG("  Header: 0x%08X\n", (unsigned int)magic);
        }
    }
    
    bool isFontReady() {
        return sdFontLoaded && sdFontData != NULL && SD_MMC.cardSize() > 0;
    }

    // Get SD glyph by UTF-8 (CJK range E4-E9)
    const uint8_t* getSDFontCharByUTF8(const char* utf8) {
        if (!sdFontLoaded || !sdFontData || !utf8 || strlen(utf8) < 3) return NULL;
        
        unsigned char b1 = (unsigned char)utf8[0];
        unsigned char b2 = (unsigned char)utf8[1];
        unsigned char b3 = (unsigned char)utf8[2];
        
        if (b1 < 0xE4 || b1 > 0xE9) return NULL;
        if (b2 < 0x80 || b2 > 0xBF || b3 < 0x80 || b3 > 0xBF) return NULL;
        
        uint32_t offset = ((b1 - 0xE4) * 4096 + (b2 - 0x80) * 64 + (b3 - 0x80)) * 32;
        if (offset + 32 > sdFontSize) return NULL;
        return sdFontData + offset;
    }
    
    // Get SD glyph for ASCII (32-126)
    const uint8_t* getSDASCIIChar(char c) {
        if (!sdFontLoaded || !sdFontData) return NULL;
        if (c < 32 || c > 126) return NULL;
        uint32_t offset = (6 * 4096 * 32) + (c - 32) * 16;
        if (offset + 16 > sdFontSize) return NULL;
        return sdFontData + offset;
    }
    
    // Find char in internal font table
    const InternalChar* findInternalChar(const char* utf8) {
        if (!internalTable || internalCount == 0) return NULL;
        for (int i = 0; i < internalCount; i++) {
            if (strcmp(internalTable[i].code, utf8) == 0) {
                return &internalTable[i];
            }
        }
        return NULL;
    }

    void drawInternalChar(int x, int y, const InternalChar* ch, uint16_t color, uint8_t size = 1) {
        if (!ch) return;
        uint16_t col = fixColor(color);
        const unsigned char* data = ch->data;
        int w = ch->width;
        int h = ch->height;
        
        for (int row = 0; row < h; row++) {
            for (int col_bit = 0; col_bit < w; col_bit++) {
                int byte_idx = row * (w / 8) + (col_bit / 8);
                int bit_idx = 7 - (col_bit % 8);
                if (data[byte_idx] & (1 << bit_idx)) {
                    for (int sx = 0; sx < size; sx++) {
                        for (int sy = 0; sy < size; sy++) {
                            tft.drawPixel(x + col_bit * size + sx, y + row * size + sy, col);
                        }
                    }
                }
            }
        }
    }
    
    // Draw placeholder box for missing glyph
    void drawPlaceholder(int x, int y, uint16_t color, uint8_t size = 1) {
        uint16_t col = fixColor(color);
        int w = 16 * size;
        int h = 16 * size;
        tft.drawRect(x, y, w, h, col);
        tft.drawLine(x, y, x + w - 1, y + h - 1, col);
        tft.drawLine(x + w - 1, y, x, y + h - 1, col);
    }
    
    // Draw ASCII char (SD first, fallback 5x7)
    bool drawASCIIChar(int x, int y, char c, uint16_t color, uint8_t size = 1) {
        const uint8_t* data = NULL;
        if (sdFontLoaded && sdFontData) {
            data = getSDASCIIChar(c);
        }
        if (!data) {
            screen_draw_char(x, y, c, color, size);
            return true;
        }
        
        uint16_t col = fixColor(color);
        for (int row = 0; row < 16; row++) {
            for (int col_bit = 0; col_bit < 8; col_bit++) {
                if (data[row] & (1 << (7 - col_bit))) {
                    for (int sx = 0; sx < size; sx++) {
                        for (int sy = 0; sy < size; sy++) {
                            tft.drawPixel(x + col_bit * size + sx, y + row * size + sy, col);
                        }
                    }
                }
            }
        }
        return true;
    }
    
    // Draw CJK char (internal table first, then SD)
    bool drawChineseChar(int x, int y, const char* utf8, uint16_t color, uint8_t size = 1) {
        if (!utf8 || strlen(utf8) < 3) return false;
        
        const uint8_t* data = NULL;
        const InternalChar* internalChar = findInternalChar(utf8);
        if (internalChar) {
            drawInternalChar(x, y, internalChar, color, size);
            return true;
        }
        
        if (sdFontLoaded && sdFontData) {
            data = getSDFontCharByUTF8(utf8);
            if (data) {
                uint16_t col = fixColor(color);
                for (int row = 0; row < 16; row++) {
                    for (int col_bit = 0; col_bit < 16; col_bit++) {
                        int byte_idx = row * 2 + (col_bit / 8);
                        int bit_idx = 7 - (col_bit % 8);
                        if (data[byte_idx] & (1 << bit_idx)) {
                            for (int sx = 0; sx < size; sx++) {
                                for (int sy = 0; sy < size; sy++) {
                                    tft.drawPixel(x + col_bit * size + sx, y + row * size + sy, col);
                                }
                            }
                        }
                    }
                }
                return true;
            }
        }
        
        drawPlaceholder(x, y, color, size);
        FONT_LOG("error UTF-8 CR %s\n", utf8);
        return false;
    }
    
    // Draw UTF-8 string (ASCII 8px, CJK 16px)
    void drawString(int x, int y, const char* str, uint16_t color, uint8_t size = 1) {
        if (!str) return;
        int posX = x;
        
        while (*str) {
            if ((unsigned char)*str < 0x80) {
                drawASCIIChar(posX, y, *str, color, size);
                posX += 8 * size;
                str++;
            } else {
                if (strlen(str) >= 3) {
                    char buf[4] = {str[0], str[1], str[2], 0};
                    drawChineseChar(posX, y, buf, color, size);
                    posX += 16 * size;
                    str += 3;
                } else {
                    break;
                }
            }
        }
    }
};

// ============================================================
// Global font manager instance
// ============================================================
extern FontManager fontManager;
FontManager fontManager;

#endif