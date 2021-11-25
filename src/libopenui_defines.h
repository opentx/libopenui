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
constexpr uint32_t LEFT = 0x00u; // align left
constexpr uint32_t VCENTERED = 0x02u; // align center vertically
constexpr uint32_t CENTERED = 0x04u; // align center
constexpr uint32_t RIGHT = 0x08u; // align right
constexpr uint32_t SHADOWED = 0x80u; // black copy at +1 +1

// unused 0x1000u
constexpr uint32_t SPACING_NUMBERS_CONST = 0x2000u;
constexpr uint32_t VERTICAL = 0x4000u;
// unused 0x8000u

/* drawNumber flags */
#define PREC_MASK                      0x30u
#define PREC1                          0x10u
#define PREC2                          0x20u
#define PREC3                          0x30u
#define FLAGS_TO_PREC(flags)           (uint8_t((flags) & 0x30u) >> 4u)
#define PREC_TO_FLAGS(prec)            (uint8_t(prec) << 4u)

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

#define ARGB_JOIN(a, r, g, b) \
  (((a) << 12) + ((r) << 8) + ((g) << 4) + (b))

#define RGB_JOIN(r, g, b) \
  (((r) << 11) + ((g) << 5) + (b))

#define GET_RED(color) \
  (((color) & 0xF800) >> 8)

#define RGB_TO_ARGB(color, alpha) \
  ARGB(alpha, GET_RED(color), GET_GREEN(color), GET_BLUE(color))

#define GET_GREEN(color) \
  (((color) & 0x07E0) >> 3)

#define GET_BLUE(color) \
  (((color) & 0x001F) << 3)

#define ALPHA_MAX                      0x0Fu
#define OPACITY(value)                 ((value) << 24u)

#define RGB(r, g, b)                   (uint16_t)((((r) & 0xF8) << 8) + (((g) & 0xFC) << 3) + (((b) & 0xF8) >> 3))
#define ARGB(a, r, g, b)               (uint16_t)((((a) & 0xF0) << 8) + (((r) & 0xF0) << 4) + (((g) & 0xF0) << 0) + (((b) & 0xF0) >> 4))

#define COLOR(index)                   LcdFlags(unsigned(index) << 16u)
#define COLOR_IDX(flags)               uint8_t((flags) >> 16u)
#define COLOR_MASK                     0xFFFF0000u

inline LcdFlags replaceColor(LcdFlags flags, LcdFlags color)
{
  return (flags & ~COLOR_MASK) | color;
}

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

