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

#include "static.h"
#include "font.h"

coord_t StaticText::drawText(BitmapBuffer * dc, const rect_t & rect, const std::string & text, LcdColor textColor, LcdFlags textFlags)
{
  coord_t x = rect.x;
  if (textFlags & CENTERED)
    x += rect.w / 2;
  else if (textFlags & RIGHT)
    x += rect.w;

  coord_t y = rect.y;
  if (textFlags & VCENTERED)
    y += (rect.h - getFontHeight(textFlags)) / 2;
  else
    y += FIELD_PADDING_TOP;

  auto start = text.c_str();
  auto current = start;
  auto nextline = findNextLine(start);
  if (nextline) {
    do {
      dc->drawText(x, y, text.substr(current - start, nextline - current).c_str(), textColor, textFlags);
      current = nextline + 1;
      nextline = findNextLine(current);
      y += getFontHeight(textFlags) + 2;
    } while (nextline);
  }
  dc->drawText(x, y, current, textColor, textFlags);
  y += getFontHeight(textFlags) + 2;
  return y;
}

void StaticText::paint(BitmapBuffer * dc)
{
  if (bgColor) {
    dc->drawSolidFilledRect(0, 0, rect.w, rect.h, bgColor);
  }

  drawText(dc, {0 + padding, 0, rect.w - padding * 2, rect.h}, text, textColor, textFlags);
}
