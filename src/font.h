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

#include <list>
#include "libopenui_types.h"
#include "libopenui_config.h"
#include "bitmapbuffer.h"
#include "unicode.h"

constexpr uint8_t LEN_FONT_NAME = 8;

inline const char * findNextLine(const char * str)
{
  while (true) {
    auto current = str;
    auto c = getNextUnicodeChar(str);
    if (c == '\0')
      return nullptr;
    else if (c == '\n')
      return current;
  }
}

class FontGlyph
{
  public:
    const Mask * data;
    unsigned offset;
    uint8_t width;
};

class Font
{
  public:
    struct GlyphRange
    {
      uint32_t begin;
      uint32_t end;
      const Mask * data;
      const uint16_t * specs;
    };

    Font() = default;

    Font(const char * name)
    {
      strlcpy(this->name, name, sizeof(this->name));
    }

    Font(const char * name, uint8_t spacing, uint8_t spaceWidth, std::list<GlyphRange> ranges):
      spacing(spacing),
      spaceWidth(spaceWidth),
      ranges(std::move(ranges))
    {
      strlcpy(this->name, name, sizeof(this->name));
    }

    bool loadFile(const char * path);

    void addGlyphs(const Font * other)
    {
      for (auto range: other->ranges) {
        addRange(range);
      }
    }

    void addRange(const GlyphRange & range)
    {
      ranges.push_back(range);
    }

    coord_t getHeight() const
    {
      return ranges.empty() ? 0 : ranges.front().data->height();
    }

    const char * getName() const
    {
      return name;
    }

    FontGlyph getGlyph(wchar_t c) const
    {
      for (auto & range: ranges) {
        if (range.begin <= uint32_t(c) && uint32_t(c) < range.end) {
          auto index = c - range.begin;
          unsigned offset = range.specs[index];
          uint8_t width = range.specs[index + 1] - offset;
          if (width > 0) {
            return {range.data, offset, width};
          }
        }
      }
      return {};
    }

    uint8_t getGlyphWidth(wchar_t c) const
    {
      if (c == ' ') {
        return spaceWidth;
      }
      else {
        auto glyph = getGlyph(c);
        return glyph.width + spacing;
      }
    }

    coord_t getTextWidth(const char * s, int len = 0) const
    {
      int currentLineWidth = 0;
      int result = 0;

      auto curr = s;
      while (len == 0 || curr - s < len) {
        auto c = getNextUnicodeChar(curr);
        if (!c) {
          break;
        }
        if (c == '\n') {
          if (currentLineWidth > result)
            result = currentLineWidth;
          currentLineWidth = 0;
        }
        else {
          currentLineWidth += getGlyphWidth(c);
        }
      }

      return currentLineWidth > result ? currentLineWidth : result;
    }

    coord_t getTextWidth(const wchar_t * s, int len = 0) const
    {
      int currentLineWidth = 0;
      int result = 0;

      auto curr = s;
      while (len == 0 || curr - s < len) {
        auto c = *curr++;
        if (!c) {
          break;
        }
        if (c == '\n') {
          if (currentLineWidth > result)
            result = currentLineWidth;
          currentLineWidth = 0;
        }
        else {
          currentLineWidth += getGlyphWidth(c);
        }
      }

      return currentLineWidth > result ? currentLineWidth : result;
    }

    uint8_t getSpacing() const
    {
      return spacing;
    }

    uint8_t getSpaceWidth() const
    {
      return spaceWidth;
    }

    wchar_t begin() const 
    {
      wchar_t result = ' ';
      for (auto & range: ranges) {
        if (range.begin < uint32_t(result)) {
          result = range.begin;
        }
      }
      return result;
    }

    wchar_t end() const 
    {
      wchar_t result = ' ';
      for (auto & range: ranges) {
        if (range.end > uint32_t(result)) {
          result = range.end;
        }
      }
      return result;
    }

    uint8_t isSubsetLoaded(uint8_t index) const
    {
      return subsetsMask & (1 << index);
    }

    void setSubsetLoaded(uint8_t index)
    {
      subsetsMask |= 1 << index;
    }

  protected:
    char name[LEN_FONT_NAME + 1];
    uint8_t subsetsMask = 0;
    uint8_t spacing = 1;
    uint8_t spaceWidth = 4;
    std::list<GlyphRange> ranges;
};

const Font * getFont(LcdFlags flags);
int getFontIndex(const char * name);

inline coord_t getFontHeight(LcdFlags flags)
{
  return getFont(flags)->getHeight();
}

template <class T>
inline coord_t getTextWidth(const T * s, int len = 0, LcdFlags flags = 0)
{
  return getFont(flags)->getTextWidth(s, len);
}
