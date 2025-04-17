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

#include <cinttypes>

inline bool isUnicodeContinuationByte(uint8_t c)
{
  return (c & 0b11000000) == 0b10000000;
}

template <class T>
inline uint32_t getNextUnicodeChar(T & s)
{
  uint8_t c1 = uint8_t(*s++);
  if (c1 < 0x80) {
    return c1;
  }
  else if (c1 < 0xE0) {
    uint8_t c2 = uint8_t(*s++);
    return ((c1 & 0b11111) << 6) + (c2 & 0b111111);
  }
  else if (c1 < 0xF0) {
    uint8_t c2 = uint8_t(*s++);
    uint8_t c3 = uint8_t(*s++);
    return ((c1 & 0b1111) << 12) + ((c2 & 0b111111) << 6) + (c3 & 0b111111);
  }
  else {
    uint8_t c2 = uint8_t(*s++);
    uint8_t c3 = uint8_t(*s++);
    uint8_t c4 = uint8_t(*s++);
    return ((c1 & 0b111) << 18) + ((c2 & 0b111111) << 12) + ((c3 & 0b111111) << 6) + (c4 & 0b111111);
  }
}

template <class T>
inline uint32_t getPreviousUnicodeChar(T * & s)
{
  while (isUnicodeContinuationByte(*(--s)));
  auto tmp = s;
  return getNextUnicodeChar(tmp);
}

inline uint32_t getUnicodeStringLength(const char * s, uint8_t size = 0)
{
  const char * pos = s;
  uint32_t result = 0;
  while (true) {
    uint8_t c = *pos;
    if (c == 0)
      return result;
    else if (c < 0x80)
      pos += 1;
    else if (c < 0xE0)
      pos += 2;
    else if (c < 0xF0) 
      pos += 3;
    else
      pos += 4;
    if (size && pos >= s + size)
      return result;
    result++;
  }
  return result;
}

template <class T>
inline T * getUnicodeStringAtPosition(T * s, uint8_t index)
{
  while (index-- > 0) {
    uint8_t c = *s;
    if (c == 0)
      return nullptr;
    else if (c < 0x80)
      s += 1;
    else if (c < 0xE0)
      s += 2;
    else if (c < 0xF0)
      s += 3;
    else
      s += 4;
  }
  return s;
}

template <class T>
inline uint32_t getUnicodeCharAtPosition(T * s, uint8_t index)
{
  auto pos = getUnicodeStringAtPosition(s, index);
  return getNextUnicodeChar(pos);
}


inline uint8_t getUnicodeCharLength(uint32_t c)
{
  if (c < 0x80)
    return 1;
  else if (c <= 0b11111111111)
    return 2;
  else if (c <= 0b1111111111111111)
    return 3;
  else
    return 4;
}

inline void writeUnicodeChar(char * s, uint32_t c)
{
  if (c < 0x80) {
    *s = c;
  }
  else if (c <= 0b11111111111) {
    *s++ = 0xC0 | ((c >> 6) & 0b11111);
    *s = 0b10000000 | (c & 0b111111);
  }
  else if (c <= 0b1111111111111111) {
    *s++ = 0xE0 | ((c >> 12) & 0b1111);
    *s++ = 0b10000000 | ((c >> 6) & 0b111111);
    *s = 0b10000000 | (c & 0b111111);
  }
  else {
    *s++ = 0xF0 | ((c >> 18) & 0b111);
    *s++ = 0b10000000 | ((c >> 12) & 0b111111);
    *s++ = 0b10000000 | ((c >> 6) & 0b111111);
    *s = 0b10000000 | (c & 0b111111);
  }
}

inline void insertUnicodeChar(char * s, uint8_t position, uint32_t c, uint8_t maxLength)
{
  auto len = getUnicodeCharLength(c);
  auto pos = getUnicodeStringAtPosition(s, position);
  memmove(pos + len, pos, s + maxLength - pos - len);
}
