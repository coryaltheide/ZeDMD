#pragma once

#include "stdint.h"

// T-Display-S3-Long specific configuration
#define LCD_USB_QSPI_DREVER 1

#define SPI_FREQUENCY 32000000  // 32MHz - safe upper limit for AXS15231B
#define TFT_SPI_MODE SPI_MODE0
#define TFT_SPI_HOST SPI2_HOST

#define TFT_WIDTH 180
#define TFT_HEIGHT 640
#define SEND_BUF_SIZE (28800 / 2)  // 16bit RGB565

// Pin definitions for T-Display-S3-Long
#define TFT_QSPI_CS 12
#define TFT_QSPI_SCK 17
#define TFT_QSPI_D0 13
#define TFT_QSPI_D1 18
#define TFT_QSPI_D2 21
#define TFT_QSPI_D3 14
#define TFT_QSPI_RST 16
#define TFT_BL 1

#define PIN_BAT_VOLT 8
#define PIN_BUTTON_1 0
#define PIN_BUTTON_2 21

// Touch pins
#define TOUCH_IICSCL 10
#define TOUCH_IICSDA 15
#define TOUCH_INT 11
#define TOUCH_RES 16

// Display control macros
#define TFT_MADCTL 0x36
#define TFT_MAD_MY 0x80
#define TFT_MAD_MX 0x40
#define TFT_MAD_MV 0x20
#define TFT_MAD_ML 0x10
#define TFT_MAD_BGR 0x08
#define TFT_MAD_MH 0x04
#define TFT_MAD_RGB 0x00

#define TFT_INVOFF 0x20
#define TFT_INVON 0x21

#define TFT_RES_H digitalWrite(TFT_QSPI_RST, 1);
#define TFT_RES_L digitalWrite(TFT_QSPI_RST, 0);
#define TFT_CS_H digitalWrite(TFT_QSPI_CS, 1);
#define TFT_CS_L digitalWrite(TFT_QSPI_CS, 0);

typedef struct {
  uint8_t cmd;
  uint8_t data[36];
  uint8_t len;
} lcd_cmd_t;

void axs15231b_init(void);
void lcd_address_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_setRotation(uint8_t r);
void lcd_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void lcd_fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend,
              uint16_t color);
void lcd_PushColors(uint16_t x, uint16_t y, uint16_t width, uint16_t high,
                    uint16_t *data);
void lcd_PushColors(uint16_t *data, uint32_t len);
void lcd_sleep();
void lcd_brightness(uint8_t bright);
