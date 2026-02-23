/**
  ******************************************************************************
  * @file    gfx_fonts.c
  * @author  Florian Gotschim
  * @brief   GFX Font bitmap data - compiled exactly once
  *
  *          PROGMEM is an AVR/Arduino keyword for explicit Flash storage.
  *          On STM32 all const data lands in Flash automatically.
  *          We define it empty so the font headers compile without changes.
  *
  *          To add a font:
  *          1. Place .h file in Core/Inc/
  *          2. Add #include below
  *          3. Add extern const GFXfont YourFont; in gfx_fonts.h
  ******************************************************************************
  */

#include "gfx_fonts.h"

#ifndef PROGMEM
#define PROGMEM
#endif

#include "FreeSans12pt7b.h"
#include "FreeSans18pt7b.h"
#include "FreeSansBold12pt7b.h"
#include "FreeSansBold18pt7b.h"
#include "FreeSans24pt7b.h"
#include "FreeSansBold24pt7b.h"
