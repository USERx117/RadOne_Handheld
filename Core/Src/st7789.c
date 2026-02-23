/**
  ******************************************************************************
  * @file    st7789.c
  * @author  Florian Gotschim
  * @brief   ST7789V2 Display Driver - DMA accelerated for RadOne Handheld
  ******************************************************************************
  */

#include "st7789.h"
#include "spi.h"
#include "fonts.h"
#include "tx_api.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define ST7789_CS_LOW()   HAL_GPIO_WritePin(ST7789_CS_PORT,  ST7789_CS_PIN,  GPIO_PIN_RESET)
#define ST7789_CS_HIGH()  HAL_GPIO_WritePin(ST7789_CS_PORT,  ST7789_CS_PIN,  GPIO_PIN_SET)
#define ST7789_DC_LOW()   HAL_GPIO_WritePin(ST7789_DC_PORT,  ST7789_DC_PIN,  GPIO_PIN_RESET)
#define ST7789_DC_HIGH()  HAL_GPIO_WritePin(ST7789_DC_PORT,  ST7789_DC_PIN,  GPIO_PIN_SET)
#define ST7789_RST_LOW()  HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_RESET)
#define ST7789_RST_HIGH() HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_SET)

#define ST7789_NOP      0x00
#define ST7789_SWRESET  0x01
#define ST7789_SLPIN    0x10
#define ST7789_SLPOUT   0x11
#define ST7789_INVOFF   0x20
#define ST7789_INVON    0x21
#define ST7789_DISPOFF  0x28
#define ST7789_DISPON   0x29
#define ST7789_CASET    0x2A
#define ST7789_RASET    0x2B
#define ST7789_RAMWR    0x2C
#define ST7789_MADCTL   0x36
#define ST7789_COLMOD   0x3A

#define ST7789_Y_OFFSET 20

#define LINE_BUF_PIXELS ST7789_WIDTH
static uint8_t s_line_buf[LINE_BUF_PIXELS * 2];

/* ==========================================================================
 * DMA synchronisation semaphore
 * Created once in ST7789_Init, used by ST7789_WriteDataDMA
 * ========================================================================== */
static TX_SEMAPHORE s_dma_sem;
static uint8_t      s_dma_ready = 0;

/* ==========================================================================
 * HAL DMA completion callback - called from SPI1 IRQ context
 * ========================================================================== */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1) {
        ST7789_CS_HIGH();
        /* Release semaphore from ISR context */
        tx_semaphore_put(&s_dma_sem);
    }
}

/* ==========================================================================
 * Private: blocking 1-byte SPI (commands, single data bytes)
 * ========================================================================== */
static void ST7789_WriteCommand(uint8_t cmd)
{
    ST7789_DC_LOW();
    ST7789_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    ST7789_CS_HIGH();
}

static void ST7789_WriteData(uint8_t data)
{
    ST7789_DC_HIGH();
    ST7789_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    ST7789_CS_HIGH();
}

/* ==========================================================================
 * Private: DMA transfer - non-blocking, waits on semaphore for completion
 * CS is pulled LOW here, raised in HAL_SPI_TxCpltCallback
 * ========================================================================== */
static void ST7789_WriteDataDMA(uint8_t *data, uint32_t len)
{
    ST7789_DC_HIGH();
    ST7789_CS_LOW();
    HAL_SPI_Transmit_DMA(&hspi1, data, (uint16_t)len);
    /* Block this thread until DMA completes (callback puts semaphore) */
    tx_semaphore_get(&s_dma_sem, TX_WAIT_FOREVER);
}

/* ==========================================================================
 * Private: small blocking transfer (< 8 bytes, not worth DMA overhead)
 * ========================================================================== */
static void ST7789_WriteDataMulti(uint8_t *data, uint32_t len)
{
    ST7789_DC_HIGH();
    ST7789_CS_LOW();
    HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);
    ST7789_CS_HIGH();
}

static void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0,
                                     uint16_t x1, uint16_t y1)
{
    uint16_t y0o = y0 + ST7789_Y_OFFSET;
    uint16_t y1o = y1 + ST7789_Y_OFFSET;

    ST7789_WriteCommand(ST7789_CASET);
    ST7789_WriteData(x0 >> 8);  ST7789_WriteData(x0 & 0xFF);
    ST7789_WriteData(x1 >> 8);  ST7789_WriteData(x1 & 0xFF);

    ST7789_WriteCommand(ST7789_RASET);
    ST7789_WriteData(y0o >> 8); ST7789_WriteData(y0o & 0xFF);
    ST7789_WriteData(y1o >> 8); ST7789_WriteData(y1o & 0xFF);

    ST7789_WriteCommand(ST7789_RAMWR);
}

/* ==========================================================================
 * Init
 * ========================================================================== */
void ST7789_Init(void)
{
    /* Create DMA semaphore - starts at 0, callback puts it */
    tx_semaphore_create(&s_dma_sem, "ST7789_DMA", 0);
    s_dma_ready = 1;

    ST7789_RST_LOW();
    tx_thread_sleep(1);
    ST7789_RST_HIGH();
    tx_thread_sleep(12);
    ST7789_WriteCommand(ST7789_SWRESET);
    tx_thread_sleep(15);
    ST7789_WriteCommand(ST7789_SLPOUT);
    tx_thread_sleep(1);
    ST7789_WriteCommand(ST7789_COLMOD);
    ST7789_WriteData(0x55);
    ST7789_WriteCommand(ST7789_MADCTL);
    ST7789_WriteData(0x00);
    ST7789_WriteCommand(ST7789_INVON);
    ST7789_WriteCommand(ST7789_DISPON);
    tx_thread_sleep(1);
    ST7789_FillScreen(ST7789_BLACK);
}

void ST7789_InitBlocking(void)
{
    ST7789_RST_LOW();
    HAL_Delay(10);
    ST7789_RST_HIGH();
    HAL_Delay(120);
    ST7789_WriteCommand(ST7789_SWRESET);
    HAL_Delay(150);
    ST7789_WriteCommand(ST7789_SLPOUT);
    HAL_Delay(120);
    ST7789_WriteCommand(ST7789_COLMOD);
    ST7789_WriteData(0x55);
    ST7789_WriteCommand(ST7789_MADCTL);
    ST7789_WriteData(0x00);
    ST7789_WriteCommand(ST7789_INVON);

    /* Fill black blocking - no DMA, no semaphore needed */
    ST7789_SetAddressWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);
    ST7789_DC_HIGH();
    ST7789_CS_LOW();
    uint8_t black[2] = {0x00, 0x00};
    for (uint32_t i = 0; i < ST7789_WIDTH * ST7789_HEIGHT; i++) {
        HAL_SPI_Transmit(&hspi1, black, 2, HAL_MAX_DELAY);
    }
    ST7789_CS_HIGH();

    ST7789_WriteCommand(ST7789_DISPON);
    HAL_Delay(10);
}

/* ==========================================================================
 * Fill operations - use DMA for full line transfers
 * ========================================================================== */
void ST7789_FillScreen(uint16_t color)
{
    ST7789_FillRect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                     uint16_t color)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    if (x + w > ST7789_WIDTH)  w = ST7789_WIDTH  - x;
    if (y + h > ST7789_HEIGHT) h = ST7789_HEIGHT - y;

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    for (uint16_t i = 0; i < w; i++) {
        s_line_buf[i * 2]     = hi;
        s_line_buf[i * 2 + 1] = lo;
    }

    ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    /* Use DMA for each row - CPU free during transfer */
    ST7789_DC_HIGH();
    for (uint16_t row = 0; row < h; row++) {
        ST7789_WriteDataDMA(s_line_buf, w * 2);
    }
}
void ST7789_FillRect_blocking(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);
    ST7789_DC_HIGH();
    ST7789_CS_LOW();
    uint8_t hi = color >> 8, lo = color & 0xFF;
    for (uint32_t i = 0; i < w * h; i++) {
        HAL_SPI_Transmit(&hspi1, &hi, 1, HAL_MAX_DELAY);
        HAL_SPI_Transmit(&hspi1, &lo, 1, HAL_MAX_DELAY);
    }
    ST7789_CS_HIGH();
}

/* ==========================================================================
 * Pixel
 * ========================================================================== */
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    ST7789_SetAddressWindow(x, y, x, y);
    uint8_t data[2] = {color >> 8, color & 0xFF};
    ST7789_WriteDataMulti(data, 2);
}

/* ==========================================================================
 * Character rendering - DMA per row
 * ========================================================================== */
void ST7789_DrawChar(uint16_t x, uint16_t y, char c,
                     uint16_t color, uint16_t bg, const FontDef *font)
{
    if (font == NULL) font = ST7789_DEFAULT_FONT;
    if (c < font->first || c > font->last) c = font->first;

    uint8_t  idx  = c - font->first;
    uint8_t  w    = font->width;
    uint8_t  h    = font->height;
    uint8_t  bpr  = font->bytes_per_row;
    const uint8_t *glyph = font->data + idx * h * bpr;

    ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);
    ST7789_DC_HIGH();

    for (uint8_t row = 0; row < h; row++) {
        uint8_t row_buf[24 * 2];
        for (uint8_t col = 0; col < w; col++) {
            uint8_t byte_idx = col / 8;
            uint8_t bit_idx  = col % 8;
            uint8_t byte_val = glyph[row * bpr + byte_idx];
            uint16_t px = (byte_val & (0x01 << bit_idx)) ? color : bg;
            row_buf[col * 2]     = px >> 8;
            row_buf[col * 2 + 1] = px & 0xFF;
        }
        ST7789_WriteDataDMA(row_buf, w * 2);
    }
}

void ST7789_DrawString(uint16_t x, uint16_t y, const char *str,
                       uint16_t color, uint16_t bg, const FontDef *font)
{
    if (font == NULL) font = ST7789_DEFAULT_FONT;
    uint16_t cx = x, cy = y;
    while (*str) {
        if (*str == '\n') { cy += font->height; cx = x; }
        else if (*str != '\r') {
            ST7789_DrawChar(cx, cy, *str, color, bg, font);
            cx += font->width;
            if (cx + font->width > ST7789_WIDTH) { cx = x; cy += font->height; }
        }
        str++;
    }
}

/* ==========================================================================
 * Shapes
 * ========================================================================== */
void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                     uint16_t color)
{
    int16_t dx  =  abs((int16_t)x1 - (int16_t)x0);
    int16_t dy  = -abs((int16_t)y1 - (int16_t)y0);
    int16_t sx  = (x0 < x1) ? 1 : -1;
    int16_t sy  = (y0 < y1) ? 1 : -1;
    int16_t err = dx + dy;
    while (1) {
        ST7789_DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void ST7789_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                     uint16_t color)
{
    ST7789_FillRect(x,         y,         w, 1, color);
    ST7789_FillRect(x,         y + h - 1, w, 1, color);
    ST7789_FillRect(x,         y,         1, h, color);
    ST7789_FillRect(x + w - 1, y,         1, h, color);
}

void ST7789_DrawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                             uint8_t percent, uint16_t color, uint16_t bg)
{
    if (percent > 100) percent = 100;
    ST7789_DrawRect(x, y, w, h, ST7789_WHITE);
    ST7789_FillRect(x + 2, y + 2, w - 4, h - 4, bg);
    uint16_t fill_w = ((w - 4) * percent) / 100;
    if (fill_w > 0)
        ST7789_FillRect(x + 2, y + 2, fill_w, h - 4, color);
}

void ST7789_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    int16_t f = 1 - (int16_t)r, ddx = 1, ddy = -2 * (int16_t)r;
    int16_t x = 0, y = (int16_t)r;
    ST7789_DrawPixel(x0, y0 + r, color); ST7789_DrawPixel(x0, y0 - r, color);
    ST7789_DrawPixel(x0 + r, y0, color); ST7789_DrawPixel(x0 - r, y0, color);
    while (x < y) {
        if (f >= 0) { y--; ddy += 2; f += ddy; }
        x++; ddx += 2; f += ddx;
        ST7789_DrawPixel(x0+x, y0+y, color); ST7789_DrawPixel(x0-x, y0+y, color);
        ST7789_DrawPixel(x0+x, y0-y, color); ST7789_DrawPixel(x0-x, y0-y, color);
        ST7789_DrawPixel(x0+y, y0+x, color); ST7789_DrawPixel(x0-y, y0+x, color);
        ST7789_DrawPixel(x0+y, y0-x, color); ST7789_DrawPixel(x0-y, y0-x, color);
    }
}

void ST7789_FillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    ST7789_FillRect(x0 - r, y0, 2 * r + 1, 1, color);
    int16_t f = 1-(int16_t)r, ddx=1, ddy=-2*(int16_t)r, x=0, y=(int16_t)r;
    while (x < y) {
        if (f >= 0) { y--; ddy += 2; f += ddy; }
        x++; ddx += 2; f += ddx;
        ST7789_FillRect(x0-x, y0+y, 2*x+1, 1, color);
        ST7789_FillRect(x0-x, y0-y, 2*x+1, 1, color);
        ST7789_FillRect(x0-y, y0+x, 2*y+1, 1, color);
        ST7789_FillRect(x0-y, y0-x, 2*y+1, 1, color);
    }
}

void ST7789_DrawArc(uint16_t x0, uint16_t y0, uint16_t r,
                    int16_t start_deg, int16_t end_deg,
                    uint8_t thickness, uint16_t color)
{
    float step = 1.0f / (float)r;
    float start_rad = start_deg * 3.14159f / 180.0f;
    float end_rad   = end_deg   * 3.14159f / 180.0f;
    for (float a = start_rad; a <= end_rad; a += step) {
        for (uint8_t t = 0; t < thickness; t++) {
            uint16_t rt = r - t;
            int16_t px = (int16_t)(x0 + rt * sinf(a));
            int16_t py = (int16_t)(y0 - rt * cosf(a));
            if (px >= 0 && px < ST7789_WIDTH && py >= 0 && py < ST7789_HEIGHT)
                ST7789_DrawPixel((uint16_t)px, (uint16_t)py, color);
        }
    }
}

uint16_t ST7789_Color565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void ST7789_DisplayOn(void)  { ST7789_WriteCommand(ST7789_DISPON); }
void ST7789_DisplayOff(void) { ST7789_WriteCommand(ST7789_DISPOFF); }

/* ==========================================================================
 * GFX Font rendering - DMA per pixel row
 * ========================================================================== */
void ST7789_DrawGFXChar(uint16_t x, uint16_t y, unsigned char c,
                         uint16_t color, uint16_t bg, const GFXfont *font)
{
    if (!font) return;
    if (c < font->first || c > font->last) return;

    GFXglyph *glyph = &font->glyph[c - font->first];
    uint8_t  *bmap  = font->bitmap;

    uint8_t  gw  = glyph->width;
    uint8_t  gh  = glyph->height;
    int8_t   xo  = glyph->xOffset;
    int8_t   yo  = glyph->yOffset;
    uint16_t bo  = glyph->bitmapOffset;

    if (bg != color)
        ST7789_FillRect(x, (int16_t)y + yo, glyph->xAdvance, gh, bg);

    uint8_t bits = 0, bit = 0;
    for (uint8_t row = 0; row < gh; row++)
    {
        for (uint8_t col = 0; col < gw; col++)
        {
            if (!(bit++ & 7)) bits = bmap[bo++];
            if (bits & 0x80)
            {
                int16_t px = (int16_t)x + xo + col;
                int16_t py = (int16_t)y + yo + row;
                if (px >= 0 && px < ST7789_WIDTH &&
                    py >= 0 && py < ST7789_HEIGHT)
                    ST7789_DrawPixel((uint16_t)px, (uint16_t)py, color);
            }
            bits <<= 1;
        }
    }
}

void ST7789_DrawGFXString(uint16_t x, uint16_t y, const char *str,
                           uint16_t color, uint16_t bg, const GFXfont *font)
{
    if (!font || !str) return;
    uint16_t cx = x;
    uint16_t cy = y;
    while (*str)
    {
        if (*str == '\n') {
            cy += font->yAdvance;
            cx  = x;
        } else if (*str != '\r') {
            if (*str >= font->first && *str <= font->last) {
                ST7789_DrawGFXChar(cx, cy, (unsigned char)*str,
                                   color, bg, font);
                cx += font->glyph[*str - font->first].xAdvance;
            }
        }
        str++;
    }
}

uint16_t ST7789_GFXStringWidth(const char *str, const GFXfont *font)
{
    if (!font || !str) return 0;
    uint16_t w = 0;
    while (*str) {
        if (*str >= font->first && *str <= font->last)
            w += font->glyph[*str - font->first].xAdvance;
        str++;
    }
    return w;
}

void ST7789_Sleep(void) {
    ST7789_WriteCommand(0x28);  /* DISPOFF */
    ST7789_WriteCommand(0x10);  /* SLPIN   */
}
