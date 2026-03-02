#ifdef DISPLAY_AXS15231B_LONG
#include "Axs15231bLong.h"

#include "fonts/tiny4x6.h"

///// LILYGO T-Display-S3-Long DRIVER
///// Uses AXS15231B LCD controller with QSPI interface
///// Display: 180x640 pixels, used in landscape mode (640x180)
///// No 24 BIT rendering supported, internally everything will be decoded to 16
/// bit.

Axs15231bLong::Axs15231bLong() : tft(), sprite(&tft), currentScalingMode(0) {
  // Sprite for fullscreen stuff - landscape mode 640x180
  sprite.createSprite(640, 180);
  sprite.setSwapBytes(1);

  axs15231b_init();
  lcd_setRotation(1);  // Landscape mode
}

const char *Axs15231bLong::scalingModes[4] = {
    "5x5 square pixels (5x Upscale)", "3x3 square pixels (DMD Effect #1)",
    "4x4 square pixels (DMD Effect #2)",
    "Argyle (Diamond) pixels (DMD Effect #3)"};

bool Axs15231bLong::HasScalingModes() {
  return true;  // This display supports subpixel scaling
}

const char **Axs15231bLong::GetScalingModes() {
  return scalingModes;  // Return the static array of mode names
}

uint8_t Axs15231bLong::GetScalingModeCount() {
  return sizeof(scalingModes) / sizeof(scalingModes[0]);
}

uint8_t Axs15231bLong::GetCurrentScalingMode() { return currentScalingMode; }

void Axs15231bLong::SetCurrentScalingMode(uint8_t mode) {
  if (mode >= 0 && mode <= 3) {
    currentScalingMode = mode;
  }
}

void Axs15231bLong::DrawPixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g,
                              uint8_t b) {
  uint16_t color = sprite.color565(r, g, b);

  sprite.fillRect(x * DISPLAY_SCALE + DISPLAY_X_OFFSET,
                  y * DISPLAY_SCALE + DISPLAY_Y_OFFSET, DISPLAY_SCALE,
                  DISPLAY_SCALE, color);
}

void Axs15231bLong::DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
  sprite.fillRect(x * DISPLAY_SCALE + DISPLAY_X_OFFSET,
                  y * DISPLAY_SCALE + DISPLAY_Y_OFFSET, DISPLAY_SCALE,
                  DISPLAY_SCALE, color);
}

void Axs15231bLong::ClearScreen() {
  sprite.fillSprite(TFT_BLACK);
  lcd_PushColors(0, 0, 640, 180, (uint16_t *)sprite.getPointer());
}

void Axs15231bLong::SetBrightness(uint8_t level) {
  lcd_brightness(lumval[level]);
}

void Axs15231bLong::FillScreen(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t color = sprite.color565(r, g, b);
  sprite.fillScreen(color);
  lcd_PushColors(0, 0, 640, 180, (uint16_t *)sprite.getPointer());
}

void Axs15231bLong::DisplayText(const char *text, uint16_t x, uint16_t y,
                                uint8_t r, uint8_t g, uint8_t b,
                                bool transparent, bool inverted) {
  for (uint8_t ti = 0; ti < strlen(text); ti++) {
    for (uint8_t tj = 0; tj <= 5; tj++) {
      uint8_t fourPixels = getFontLine(text[ti], tj);
      for (uint8_t pixel = 0; pixel < 4; pixel++) {
        bool p = (fourPixels >> (3 - pixel)) & 0x1;
        if (inverted) {
          p = !p;
        }
        if (transparent && !p) {
          continue;
        }
        uint16_t color = sprite.color565(r * p, g * p, b * p);

        sprite.fillRect(
            (x + pixel + (ti * 4)) * DISPLAY_SCALE + DISPLAY_X_OFFSET,
            (y + tj) * DISPLAY_SCALE + DISPLAY_Y_OFFSET, DISPLAY_SCALE,
            DISPLAY_SCALE, color);
      }
    }
  }
  lcd_PushColors(0, 0, 640, 180, (uint16_t *)sprite.getPointer());
}

// Speed optimized version
void IRAM_ATTR Axs15231bLong::FillZoneRaw(uint8_t idx, uint8_t *pBuffer) {
  uint16_t yOffset =
      ((idx / ZONES_PER_ROW) * ZONE_HEIGHT * DISPLAY_SCALE) + DISPLAY_Y_OFFSET;
  uint16_t xOffset =
      (idx % ZONES_PER_ROW) * ZONE_WIDTH * DISPLAY_SCALE + DISPLAY_X_OFFSET;

  // Buffer to store the pixel data in byte format for SPI (2 bytes per pixel)
  uint8_t
      pixelBuffer[ZONE_WIDTH * ZONE_HEIGHT * DISPLAY_SCALE * DISPLAY_SCALE * 2];

  for (uint16_t y = 0; y < ZONE_HEIGHT; y++) {
    for (uint16_t x = 0; x < ZONE_WIDTH; x++) {
      // Extract the RGB888 color and convert to RGB565
      uint16_t pos = (y * ZONE_WIDTH + x) * 3;
      uint16_t color = ((pBuffer[pos] & 0xF8) << 8) |
                       ((pBuffer[pos + 1] & 0xFC) << 3) |
                       (pBuffer[pos + 2] >> 3);
      uint16_t black = 0x0000;

      // Precompute pixel block colors based on currentScalingMode
      uint16_t drawColors[DISPLAY_SCALE][DISPLAY_SCALE];

      switch (currentScalingMode) {
        case 0:
          // 5x5 all pixels are color
          for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
            for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
              drawColors[i][j] = color;
            }
          }
          break;
        case 1:
          // 3x3 center pixels in color, rest black
          for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
            for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
              if (i >= 1 && i <= 3 && j >= 1 && j <= 3) {
                drawColors[i][j] = color;
              } else {
                drawColors[i][j] = black;
              }
            }
          }
          break;
        case 2:
          // 4x4 center pixels in color, rest black (1 pixel border)
          for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
            for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
              if (i > 0 && i < DISPLAY_SCALE - 1 && j > 0 &&
                  j < DISPLAY_SCALE - 1) {
                drawColors[i][j] = color;
              } else {
                drawColors[i][j] = black;
              }
            }
          }
          break;
        case 3:
          // Argyle diamond pattern
          for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
            for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
              if ((i == j) || (i + j == DISPLAY_SCALE - 1)) {
                drawColors[i][j] = color;
              } else {
                drawColors[i][j] = black;
              }
            }
          }
          break;
        default:
          // Fallback to normal 5x5 block
          for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
            for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
              drawColors[i][j] = color;
            }
          }
          break;
      }

      // Scale the block according to DISPLAY_SCALE
      for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
        for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
          uint16_t scaledX = x * DISPLAY_SCALE + i;
          uint16_t scaledY = y * DISPLAY_SCALE + j;

          uint16_t pixelPos =
              (scaledY * ZONE_WIDTH * DISPLAY_SCALE + scaledX) * 2;

          pixelBuffer[pixelPos] = (drawColors[i][j] >> 8) & 0xFF;
          pixelBuffer[pixelPos + 1] = drawColors[i][j] & 0xFF;
        }
      }
    }
  }

  lcd_PushColors(xOffset, yOffset, ZONE_WIDTH * DISPLAY_SCALE,
                 ZONE_HEIGHT * DISPLAY_SCALE, (uint16_t *)pixelBuffer);
}

// Speed optimized version for RGB565
void IRAM_ATTR Axs15231bLong::FillZoneRaw565(uint8_t idx, uint8_t *pBuffer) {
  uint16_t yOffset =
      ((idx / ZONES_PER_ROW) * ZONE_HEIGHT * DISPLAY_SCALE) + DISPLAY_Y_OFFSET;
  uint16_t xOffset =
      (idx % ZONES_PER_ROW) * ZONE_WIDTH * DISPLAY_SCALE + DISPLAY_X_OFFSET;

  uint8_t
      pixelBuffer[ZONE_WIDTH * ZONE_HEIGHT * DISPLAY_SCALE * DISPLAY_SCALE * 2];

  for (uint16_t y = 0; y < ZONE_HEIGHT; y++) {
    for (uint16_t x = 0; x < ZONE_WIDTH; x++) {
      uint16_t pos = (y * ZONE_WIDTH + x) * 2;
      uint16_t color = ((((uint16_t)pBuffer[pos + 1]) << 8) + pBuffer[pos]);
      uint16_t black = 0x0000;

      uint16_t drawColors[DISPLAY_SCALE][DISPLAY_SCALE];

      switch (currentScalingMode) {
        case 0:
          for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
            for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
              drawColors[i][j] = color;
            }
          }
          break;
        case 1:
          for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
            for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
              if (i >= 1 && i <= 3 && j >= 1 && j <= 3) {
                drawColors[i][j] = color;
              } else {
                drawColors[i][j] = black;
              }
            }
          }
          break;
        case 2:
          for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
            for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
              if (i > 0 && i < DISPLAY_SCALE - 1 && j > 0 &&
                  j < DISPLAY_SCALE - 1) {
                drawColors[i][j] = color;
              } else {
                drawColors[i][j] = black;
              }
            }
          }
          break;
        case 3:
          for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
            for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
              if ((i == j) || (i + j == DISPLAY_SCALE - 1)) {
                drawColors[i][j] = color;
              } else {
                drawColors[i][j] = black;
              }
            }
          }
          break;
        default:
          for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
            for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
              drawColors[i][j] = color;
            }
          }
          break;
      }

      for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
        for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
          uint16_t scaledX = x * DISPLAY_SCALE + i;
          uint16_t scaledY = y * DISPLAY_SCALE + j;

          uint16_t pixelPos =
              (scaledY * ZONE_WIDTH * DISPLAY_SCALE + scaledX) * 2;

          pixelBuffer[pixelPos] = (drawColors[i][j] >> 8) & 0xFF;
          pixelBuffer[pixelPos + 1] = drawColors[i][j] & 0xFF;
        }
      }
    }
  }

  lcd_PushColors(xOffset, yOffset, ZONE_WIDTH * DISPLAY_SCALE,
                 ZONE_HEIGHT * DISPLAY_SCALE, (uint16_t *)pixelBuffer);
}

void IRAM_ATTR Axs15231bLong::ClearZone(uint8_t idx) {
  FillZoneRaw(idx, (uint8_t *)blackZone);
}

void IRAM_ATTR Axs15231bLong::FillPanelRaw(uint8_t *pBuffer) {
  uint16_t pos;

  for (uint16_t y = 0; y < TOTAL_HEIGHT; y++) {
    for (uint16_t x = 0; x < TOTAL_WIDTH; x++) {
      pos = (y * TOTAL_WIDTH + x) * 3;
      uint16_t color =
          sprite.color565(pBuffer[pos], pBuffer[pos + 1], pBuffer[pos + 2]);
      uint16_t black = sprite.color565(0, 0, 0);

      for (uint16_t i = 0; i < DISPLAY_SCALE; i++) {
        for (uint16_t j = 0; j < DISPLAY_SCALE; j++) {
          uint16_t drawColor = black;

          switch (currentScalingMode) {
            case 0:
              drawColor = color;
              break;
            case 1:
              if (i >= 1 && i <= 3 && j >= 1 && j <= 3) {
                drawColor = color;
              }
              break;
            case 2:
              if (i > 0 && i < DISPLAY_SCALE - 1 && j > 0 &&
                  j < DISPLAY_SCALE - 1) {
                drawColor = color;
              }
              break;
            case 3:
              if ((i == j) || (i + j == DISPLAY_SCALE - 1)) {
                drawColor = color;
              }
              break;
            default:
              drawColor = color;
          }

          sprite.drawPixel(x * DISPLAY_SCALE + i + DISPLAY_X_OFFSET,
                           y * DISPLAY_SCALE + j + DISPLAY_Y_OFFSET, drawColor);
        }
      }
    }
  }

  lcd_PushColors(0, 0, 640, 180, (uint16_t *)sprite.getPointer());
}

void Axs15231bLong::Render() {}

Axs15231bLong::~Axs15231bLong() {
  // Clean up resources if necessary
}
#endif
