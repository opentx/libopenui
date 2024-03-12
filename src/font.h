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
#include "libopenui_config.h"
#include "bitmapdata.h"

constexpr uint8_t CJK_BYTE1_MIN = 0xFD;

inline bool hasChineseChars(const char * str)
{
  while (*str) {
    uint8_t c = *str++;
    if (c >= CJK_BYTE1_MIN) {
      return true;
    }
  }
  return false;
}

inline const char * findNextLine(const char * stack)
{
  while (true) {
    const char * pos = strchr(stack, '\n');
    if (!pos)
      return nullptr;
    if (pos == stack || *((uint8_t *)(pos - 1)) < CJK_BYTE1_MIN)
      return pos;
    stack = pos + 1;
  }
}

class Font
{
  public:
    struct Glyph
    {
      const Font * font;
      unsigned offset;
      uint8_t width;
    };

    Font(const char * name, uint16_t count, const BitmapData * data, const uint16_t * specs):
      name(name),
      count(count),
      data(data),
      specs(specs)
    {
    }

    coord_t getHeight() const
    {
      return specs[0];
    }

    const char * getName() const
    {
      return name;
    }

    Glyph getGlyph(unsigned index) const
    {
      auto offset = specs[index + 1];
      return {this, offset, uint8_t(specs[index + 2] - offset)};
    }

    Glyph getChar(uint8_t c) const
    {
      if (c >= 0x20) {
        return getGlyph(c - 0x20);
      }
      else {
        return {this, 0, 0};
      }
    }

    [[nodiscard]] Glyph getCJKChar(uint8_t byte1, uint8_t byte2) const
    {
      unsigned result = byte2 + ((byte1 - CJK_BYTE1_MIN) << 8u) - 1;
      if (result >= 0x200)
        result -= 1;
      if (result >= 0x100)
        result -= 1;
      return getGlyph(CJK_FIRST_LETTER_INDEX + result);
    }

    coord_t getTextWidth(const char * s, int len = 0) const
    {
      int currentLineWidth = 0;
      int result = 0;
      for (int i = 0; len == 0 || i < len; ++i) {
        unsigned c = uint8_t(*s);
        if (!c) {
          break;
        }
        else if (c >= CJK_BYTE1_MIN) {
          currentLineWidth += getCJKChar(c, *++s).width + CHAR_SPACING;
        }
        else if (c >= 0x20) {
          if (c >= '0' && c <= '9')
            currentLineWidth += getChar('9').width + CHAR_SPACING;
          else
            currentLineWidth += getChar(c).width + CHAR_SPACING;
        }
        else if (c == '\n') {
          if (currentLineWidth > result)
            result = currentLineWidth;
          currentLineWidth = 0;
        }

        ++s;
      }

      return currentLineWidth > result ? currentLineWidth : result;
    }

    [[nodiscard]] const BitmapData * getBitmapData() const
    {
      return data;
    }

    [[nodiscard]] bool hasCJKChars() const
    {
      return count > CJK_FIRST_LETTER_INDEX;
    }

  protected:
    const char * name;
    uint16_t count;
    const BitmapData * data;
    const uint16_t * specs;
};

const Font * getFont(LcdFlags flags);
int getFontIndex(const char * name);

inline coord_t getFontHeight(LcdFlags flags)
{
  return getFont(flags)->getHeight();
}

inline coord_t getTextWidth(const char * s, int len = 0, LcdFlags flags = 0)
{
  return getFont(flags)->getTextWidth(s, len);
}
