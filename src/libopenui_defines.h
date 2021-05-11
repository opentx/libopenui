/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#pragma once

#include "libopenui_types.h"
#include "libopenui_compat.h"

/* obsolete flags */
#define BLINK                          0
#define EXPANDED                       0
#define TIMEHOUR                       0

/* drawText flags */
#define LEFT                           0x00u /* align left */
#define VCENTERED                      0x02u /* align center vertically */
#define CENTERED                       0x04u /* align center */
#define RIGHT                          0x08u /* align right */
#define SHADOWED                       0x80u /* black copy at +1 +1 */
// unused 0x1000u
#define SPACING_NUMBERS_CONST          0x2000u
#define VERTICAL                       0x4000u
// unused 0x8000u

/* drawNumber flags */
#define PREC1                          0x10u
#define PREC2                          0x20u
#define PREC3                          0x30u
#define MODE(flags)                    (uint8_t((flags) & 0x30u) >> 4u)

#define LEADING0                       0x40u

#define FONT_MASK                      0x0F00u
#define FONT_INDEX(flags)              (((flags) & FONT_MASK) >> 8u)
#define FONT(xx)                       (unsigned(FONT_ ## xx ## _INDEX) << 8u)

#define ARGB_SPLIT(color, a, r, g, b) \
  uint16_t a = ((color) & 0xF000) >> 12; \
  uint16_t r = ((color) & 0x0F00) >> 8; \
  uint16_t g = ((color) & 0x00F0) >> 4; \
  uint16_t b = ((color) & 0x000F)

#define RGB_SPLIT(color, r, g, b) \
  uint16_t r = ((color) & 0xF800) >> 11; \
  uint16_t g = ((color) & 0x07E0) >> 5; \
  uint16_t b = ((color) & 0x001F)

#define RGB_JOIN(r, g, b) \
  (((r) << 11) + ((g) << 5) + (b))

#define GET_RED(color) \
  (((color) & 0xF800) >> 8)

#define GET_GREEN(color) \
  (((color) & 0x07E0) >> 3)

#define GET_BLUE(color) \
  (((color) & 0x001F) << 3)

#define OPACITY_MAX                    0x0Fu
#define OPACITY(value)                 ((value) << 24u)

#define RGB(r, g, b)                   (uint16_t)((((r) & 0xF8) << 8) + (((g) & 0xFC) << 3) + (((b) & 0xF8) >> 3))
#define ARGB(a, r, g, b)               (uint16_t)((((a) & 0xF0) << 8) + (((r) & 0xF0) << 4) + (((g) & 0xF0) << 0) + (((b) & 0xF0) >> 4))

#define COLOR(index)                   LcdFlags(unsigned(index) << 16u)
#define COLOR_IDX(flags)               uint8_t((flags) >> 16u)
#define COLOR_MASK(flags)              ((flags) & 0xFFFF0000u)

#define DEFAULT_COLOR                  COLOR(DEFAULT_COLOR_INDEX)
#define DEFAULT_BGCOLOR                COLOR(DEFAULT_BGCOLOR_INDEX)
#define FOCUS_COLOR                    COLOR(FOCUS_COLOR_INDEX)
#define FOCUS_BGCOLOR                  COLOR(FOCUS_BGCOLOR_INDEX)
#define DISABLE_COLOR                  COLOR(DISABLE_COLOR_INDEX)
#define HIGHLIGHT_COLOR                COLOR(HIGHLIGHT_COLOR_INDEX)
#define CHECKBOX_COLOR                 COLOR(CHECKBOX_COLOR_INDEX)
#define SCROLLBAR_COLOR                COLOR(SCROLLBAR_COLOR_INDEX)
#define MENU_COLOR                     COLOR(MENU_COLOR_INDEX)
#define MENU_BGCOLOR                   COLOR(MENU_BGCOLOR_INDEX)
#define MENU_TITLE_BGCOLOR             COLOR(MENU_TITLE_BGCOLOR_INDEX)
#define MENU_LINE_COLOR                COLOR(MENU_LINE_COLOR_INDEX)
#define MENU_HIGHLIGHT_COLOR           COLOR(MENU_HIGHLIGHT_COLOR_INDEX)
#define MENU_HIGHLIGHT_BGCOLOR         COLOR(MENU_HIGHLIGHT_BGCOLOR_INDEX)
#define OVERLAY_COLOR                  COLOR(OVERLAY_COLOR_INDEX)
#define TABLE_BGCOLOR                  COLOR(TABLE_BGCOLOR_INDEX)
#define TABLE_HEADER_BGCOLOR           COLOR(TABLE_HEADER_BGCOLOR_INDEX)
#define CUSTOM_COLOR                   COLOR(CUSTOM_COLOR_INDEX)

