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

class BitmapData
{
  public:
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

    // TODO duplicated code
    void flip()
    {
      auto ptr1 = &data[0];
      auto ptr2 = &data[height() * width() - 1];
      while (ptr2 > ptr1) {
        std::swap(*ptr1++, *ptr2--);
      }
    }

    // TODO duplicated code
    void rotate()
    {
      auto dataSize = width() * height();
      auto * srcData = (uint8_t *)malloc(dataSize);
      memcpy(srcData, data, dataSize);
      auto * destData = &data[0];
      for (uint16_t y = 0; y < height(); y++) {
        for (uint16_t x = 0; x < width(); x++) {
          destData[x * height() + y] = srcData[y * width() + x];
        }
      }
      free(srcData);
    }

  protected:
    uint32_t format; // alignment
    uint16_t _width;
    uint16_t _height;
    uint8_t data[];
};
