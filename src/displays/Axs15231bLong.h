#ifndef AXS15231B_LONG_H
#define AXS15231B_LONG_H

#include <LittleFS.h>
#include <TFT_eSPI.h>

#include "axs15231b.h"
#include "displayDriver.h"
#include "panel.h"

// T-Display-S3-Long in landscape mode: 640x180 pixels
// ZeDMD content: 128x32 pixels
// Scale factor: min(640/128, 180/32) = min(5, 5.625) = 5x
#define DISPLAY_SCALE 5

// Y offset to center the 128x32 content on 180 pixel height
// (180 - 32*5) / 2 = (180 - 160) / 2 = 10
#define DISPLAY_Y_OFFSET 10

// X offset - content fits exactly in landscape mode
// (640 - 128*5) / 2 = (640 - 640) / 2 = 0
#define DISPLAY_X_OFFSET 0

// SCALING MODES:
// 0 = 5x5 pixel blocks (full upscale)
// 1 = 3x3 center pixels, other pixels black (DMD style #1)
// 2 = 4x4 center pixels, other pixels black (DMD style #2)
// 3 = Argyle (diamond) pixel blocks, other pixels black (DMD style #3)

class Axs15231bLong : public DisplayDriver {
 private:
  TFT_eSPI tft;
  TFT_eSprite sprite;
  const uint8_t lumval[16] = {0,   16,  32,  48,  64,  80,  96,  112,
                              128, 144, 160, 176, 192, 208, 224, 255};

  static const char *scalingModes[4];
  static const uint8_t modeCount = 4;

  uint8_t currentScalingMode;
  const uint8_t blackZone[384] = {0};

 public:
  Axs15231bLong();

  bool HasScalingModes();
  const char **GetScalingModes();
  uint8_t GetScalingModeCount();
  uint8_t GetCurrentScalingMode();
  void SetCurrentScalingMode(uint8_t mode);

  void DrawPixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
  void DrawPixel(uint16_t x, uint16_t y, uint16_t color);
  void ClearScreen();
  void SetBrightness(uint8_t level);
  void FillScreen(uint8_t r, uint8_t g, uint8_t b);
  void DisplayText(const char *text, uint16_t x, uint16_t y, uint8_t r,
                   uint8_t g, uint8_t b, bool transparent = false,
                   bool inverted = false);
  void FillZoneRaw(uint8_t idx, uint8_t *pBuffer);
  void FillZoneRaw565(uint8_t idx, uint8_t *pBuffer);
  void ClearZone(uint8_t idx);
  void FillPanelRaw(uint8_t *pBuffer);
  void Render() override;

  ~Axs15231bLong();
};

#endif  // AXS15231B_LONG_H
