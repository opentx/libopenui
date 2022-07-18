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

struct BitmapData
{
  uint16_t _width;
  uint16_t _height;
  uint8_t data[];

  uint16_t width() const
  {
    return _width;
  }

  uint16_t height() const
  {
    return _height;
  }

  const uint8_t * getData() const
  {
    return data;
  }
};
