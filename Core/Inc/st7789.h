/**
  ******************************************************************************
  * @file    st7789.h
  * @author  Florian Gotschim
  ******************************************************************************
  */

#ifndef INC_ST7789_H_
#define INC_ST7789_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32u3xx_hal.h"
#include "fonts.h"
#include <stdint.h>
#include "gfx_fonts.h"

#define ST7789_WIDTH    240
#define ST7789_HEIGHT   280
#define ST7789_CS_PIN    GPIO_PIN_2
#define ST7789_CS_PORT   GPIOA
#define ST7789_DC_PIN    GPIO_PIN_3
#define ST7789_DC_PORT   GPIOA
#define ST7789_RST_PIN   GPIO_PIN_4
#define ST7789_RST_PORT  GPIOA

#define ST7789_DEFAULT_FONT   (&Font_8x8)

#define ST7789_BLACK       0x0000
#define ST7789_WHITE       0xFFFF
#define ST7789_RED         0xF800
#define ST7789_GREEN       0x07E0
#define ST7789_BLUE        0x001F
#define ST7789_CYAN        0x07FF
#define ST7789_MAGENTA     0xF81F
#define ST7789_YELLOW      0xFFE0
#define ST7789_ORANGE      0xFC00
#define ST7789_GRAY        0x8410
#define ST7789_DARKGRAY    0x4208
#define ST7789_LIGHTGRAY   0xC618

void ST7789_Init(void);
void ST7789_FillScreen(uint16_t color);
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7789_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void ST7789_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void ST7789_FillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void ST7789_DrawArc(uint16_t x0, uint16_t y0, uint16_t r,
                    int16_t start_deg, int16_t end_deg,
                    uint8_t thickness, uint16_t color);
void ST7789_DrawChar  (uint16_t x, uint16_t y, char c,
                       uint16_t color, uint16_t bg, const FontDef *font);
void ST7789_DrawString(uint16_t x, uint16_t y, const char *str,
                       uint16_t color, uint16_t bg, const FontDef *font);
void ST7789_DrawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                             uint8_t percent, uint16_t color, uint16_t bg);
uint16_t ST7789_Color565(uint8_t r, uint8_t g, uint8_t b);
void     ST7789_DisplayOn(void);
void     ST7789_DisplayOff(void);


/* ============================================================================
 * GFX Font rendering (Adafruit GFXfont compatible, proportional fonts)
 *
 * y = BASELINE position (not top of character)
 * Use y = font->yAdvance + margin for first text line
 *
 * Example:
 *   uint16_t w = ST7789_GFXStringWidth("0.000", &FreeSans18pt7b);
 *   ST7789_DrawGFXString((240-w)/2, 80, "0.000", WHITE, BLACK, &FreeSans18pt7b);
 * ============================================================================ */
void     ST7789_DrawGFXChar  (uint16_t x, uint16_t y, unsigned char c,
                               uint16_t color, uint16_t bg,
                               const GFXfont *font);
void     ST7789_DrawGFXString(uint16_t x, uint16_t y, const char *str,
                               uint16_t color, uint16_t bg,
                               const GFXfont *font);
uint16_t ST7789_GFXStringWidth(const char *str, const GFXfont *font);

#define ST7789_PutChar(x,y,c,col,bg)  ST7789_DrawChar(x,y,c,col,bg,ST7789_DEFAULT_FONT)
#define ST7789_Print(x,y,str,col,bg)  ST7789_DrawString(x,y,str,col,bg,ST7789_DEFAULT_FONT)

#ifdef __cplusplus
}
#endif

#endif /* INC_ST7789_H_ */
