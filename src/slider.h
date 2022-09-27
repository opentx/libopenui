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

#include "form.h"

class Slider: public FormField
{
  public:
    Slider(Window * parent, const rect_t & rect, int32_t vmin, int32_t vmax, std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags = 0):
      FormField(parent, rect, windowFlags),
      vmin(vmin),
      vmax(vmax),
      getValue(std::move(getValue)),
      _setValue(std::move(setValue))
    {
    }

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "Slider";
    }
#endif

    void setValue(int value)
    {
      _setValue(limit(vmin, value, vmax));
      invalidate();
    }

    void setMin(int value)
    {
      vmin = value;
      invalidate();
    }

    void setMax(int value)
    {
      vmax = value;
      invalidate();
    }

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchStart(coord_t x, coord_t y) override;

    bool onTouchEnd(coord_t x, coord_t y) override;

    bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY) override;
#endif

  protected:
    int value(coord_t x) const;
    int vmin;
    int vmax;
    bool sliding = false;
    std::function<int()> getValue;
    std::function<void(int)> _setValue;
};

