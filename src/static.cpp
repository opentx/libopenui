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

using namespace ui;

coord_t StaticText::drawText(BitmapBuffer * dc, const rect_t & rect, const std::string & text, LcdColor textColor, LcdFlags textFlags)
{
  coord_t x = rect.x;
  if (textFlags & CENTERED)
    x += rect.w / 2;
  else if (textFlags & RIGHT)
    x += rect.w;

  auto fontHeight = getFontHeight(textFlags);

  coord_t y = rect.y;
  if (textFlags & VCENTERED) {
    y += (rect.h - fontHeight) / 2;
  }
  
  auto start = text.c_str();
  auto current = start;

  // TODO pas du code dupliqué ici ???
  
  // auto nextline = findNextLine(start);
  // if (nextline) {
  //   do {
  //     dc->drawText(x, y, text.substr(current - start, nextline - current).c_str(), textColor, textFlags);
  //     current = nextline + 1;
  //     nextline = findNextLine(current);
  //     y += fontHeight + 2;
  //   } while (nextline);
  // }
  dc->drawText(x, y, current, textColor, textFlags);
  y += fontHeight + 2;
  return y;
}

void StaticText::paint(BitmapBuffer * dc)
{
  if (bgColor) {
    dc->drawPlainFilledRectangle(0, 0, rect.w, rect.h, bgColor);
  }

  drawText(dc, {horizontalPadding, verticalPadding, rect.w - horizontalPadding * 2, rect.h - verticalPadding * 2}, text, textColor, textFlags);
}
