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

uint8_t getMappedChar(uint8_t c);
uint8_t getFontHeight(LcdFlags flags);
int getCharWidth(uint8_t c, const uint16_t * spec);
int getTextWidth(const char * s, int len = 0, LcdFlags flags = 0);

constexpr uint8_t CJK_BYTE1_MIN = 0xFD;

inline unsigned getCJKChar(uint8_t byte1, uint8_t byte2)
{
  unsigned result = byte2 + ((byte1 - CJK_BYTE1_MIN) << 8u) - 1;
  if (result >= 0x200)
    result -= 1;
  if (result >= 0x100)
    result -= 1;
  return CJK_FIRST_LETTER_INDEX + result;
}
