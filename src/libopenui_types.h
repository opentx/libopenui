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

#include "libopenui_helpers.h"

typedef int coord_t;

struct point_t
{
  coord_t x;
  coord_t y;

  bool operator != (const point_t & b) const
  {
    return x != b.x || y != b.y;
  }
};

struct range_t
{
  coord_t left;
  coord_t right;
};

struct rect_t
{
  coord_t x, y, w, h;

  constexpr coord_t left() const
  {
    return x;
  }

  constexpr coord_t right() const
  {
    return x + w;
  }

  constexpr coord_t top() const
  {
    return y;
  }

  constexpr coord_t bottom() const
  {
    return y + h;
  }

  bool contains(const point_t & point) const
  {
    return point.x >= left() && point.x < right() && point.y >= top() && point.y < bottom();
  }

  bool contains(const rect_t & other) const
  {
    return left() <= other.left() && right() >= other.right() && top() <= other.top() && bottom() >= other.bottom();
  }

  rect_t operator & (const rect_t & other) const
  {
    auto leftMax = max<coord_t>(left(), other.left());
    auto rightMin = min<coord_t>(right(), other.right());
    auto width = rightMin - leftMax;
    if (width <= 0)
      return {0, 0, 0, 0};
    auto topMax = max<coord_t>(top(), other.top());
    auto bottomMin = min<coord_t>(bottom(), other.bottom());
    auto height = bottomMin - topMax;
    if (height <= 0)
      return {0, 0, 0, 0};
    return { leftMax, topMax, width, height };
  }

  std::string toString() const
  {
    char result[32];
    sprintf(result, "[%d, %d, %d, %d]", x, y, w, h);
    return result;
  }
};

static const rect_t nullRect = {0, 0, 0, 0};

typedef uint32_t LcdFlags;
typedef uint16_t event_t;
typedef uint16_t Color565;
typedef uint32_t LcdColor;
typedef uint32_t WindowFlags;
