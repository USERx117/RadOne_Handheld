/**
  ******************************************************************************
  * @file    gfx_fonts.h
  * @author  Florian Gotschim
  * @brief   GFX Font type definitions and extern font declarations
  *
  *          Include this file anywhere you need GFXfont types or font refs.
  *          Font bitmap data is compiled once in gfx_fonts.c
  ******************************************************************************
  */

#ifndef INC_GFX_FONTS_H_
#define INC_GFX_FONTS_H_

#include <stdint.h>

/* ============================================================================
 * GFX Font descriptors (Adafruit-compatible)
 * ============================================================================ */
typedef struct {
    uint16_t bitmapOffset;  /* Offset into bitmap array              */
    uint8_t  width;         /* Glyph width in pixels                 */
    uint8_t  height;        /* Glyph height in pixels                */
    uint8_t  xAdvance;      /* Cursor advance after glyph (proportional) */
    int8_t   xOffset;       /* X offset from cursor to glyph left    */
    int8_t   yOffset;       /* Y offset from cursor to glyph top     */
} GFXglyph;

typedef struct {
    uint8_t  *bitmap;       /* Glyph bitmap data                     */
    GFXglyph *glyph;        /* Glyph descriptor array                */
    uint8_t   first;        /* First ASCII character (usually 32)    */
    uint8_t   last;         /* Last ASCII character  (usually 126)   */
    uint8_t   yAdvance;     /* Line height                           */
} GFXfont;

/* ============================================================================
 * Available fonts - defined in gfx_fonts.c
 * To add a font: place .h in Core/Inc/, #include it in gfx_fonts.c,
 * and add an extern declaration here.
 * ============================================================================ */
extern const GFXfont FreeSans12pt7b;
extern const GFXfont FreeSans18pt7b;
extern const GFXfont FreeSansBold12pt7b;
extern const GFXfont FreeSansBold18pt7b;

#endif /* INC_GFX_FONTS_H_ */
