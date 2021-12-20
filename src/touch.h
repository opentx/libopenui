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

enum TouchEvent
{
  TE_NONE,
  TE_DOWN,
  TE_LONG,
  TE_UP,
  TE_SLIDE,
  TE_SLIDE_END
};

struct TouchState
{
  unsigned char state;
  unsigned char event;
  int x;
  int y;
  int startX;
  int startY;
  int deltaX;
  int deltaY;
  int lastDeltaX;
  int lastDeltaY;
  unsigned startTime;

  void setState(unsigned char value)
  {
    if (value >= TE_SLIDE || value != state) {
      event = value;
    }
    state = value;
  }

  void killEvents()
  {
    event = state = TE_NONE;
  }

  unsigned char popEvent()
  {
    auto result = event;
    if (result < TE_SLIDE) {
      event = TE_NONE;
    }
    return result;
  }

  bool isScrolling() const
  {
    return state == TE_SLIDE || lastDeltaX != 0 || lastDeltaY != 0;
  }
};

extern TouchState touchState;
