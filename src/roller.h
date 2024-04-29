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

#include <touch.h>
#include <choice.h>
#include <font.h>
#include <static.h>
#include <limits.h>
#include "libopenui_config.h"

constexpr WindowFlags ROLLER_SEPARATION_LINES = (FORM_FLAGS_LAST << 1u);
constexpr coord_t SCROLL_POSITION_INVALIDATED = INT_MIN;

namespace ui {

class Roller: public Choice
{
  public:
    Roller(FormGroup * parent, const rect_t & rect, const char * label, const char * const * values, int vmin, int vmax, std::function<int()> getValue, std::function<void(int)> setValue = nullptr, WindowFlags windowFlags = ROLLER_SEPARATION_LINES):
      Choice(parent, rect, values, vmin, vmax, std::move(getValue), std::move(setValue), windowFlags | NO_SCROLLBAR)
    {
      init(label);
    }

    Roller(FormGroup * parent, const rect_t & rect, const char * label, std::vector<std::string> values, int vmin, int vmax, std::function<int()> getValue, std::function<void(int)> setValue = nullptr, WindowFlags windowFlags = ROLLER_SEPARATION_LINES):
      Choice(parent, rect, std::move(values), vmin, vmax, std::move(getValue), std::move(setValue), windowFlags | NO_SCROLLBAR)
    {
      init(label);
    }

    void enable(bool value = true)
    {
      if (value != enabled) {
        enabled = value;
        setInnerHeight(value ? INFINITE_HEIGHT : height());
        invalidate();
      }
    }

    void init(const char * label)
    {
      if (label) {
        // the label is another window, could be changed, but is it needed?
        new StaticText(parent, {rect.x, rect.y - ROLLER_LINE_HEIGHT, rect.w, ROLLER_LINE_HEIGHT}, label, 0, CENTERED);
      }

      setHeight(ROLLER_LINE_HEIGHT * 3 - 1);
      setPageHeight(ROLLER_LINE_HEIGHT);
      setInnerHeight(INFINITE_HEIGHT);
      updateScrollPositionFromValue();
    }

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "Roller";
    }
#endif

    void paint(BitmapBuffer * dc) override
    {
      int32_t value = getValue();
      auto valuesCount = (int)getValuesCount();
      if (valuesCount == 0) {
        return;
      }

      int index = (scrollPositionY - ROLLER_LINE_HEIGHT + 1)  / ROLLER_LINE_HEIGHT;
      coord_t y = index * ROLLER_LINE_HEIGHT;
      coord_t yMax = scrollPositionY + 3 * ROLLER_LINE_HEIGHT;
      index = mod(index, valuesCount);

      while (y < yMax) {
        auto displayedValue = getValueFromIndex(index);

        auto fgColor = DISABLE_COLOR;
        if (value == displayedValue) {
          fgColor = FOCUS_COLOR | FONT(STD);
        }

        unsigned valueIndex = displayedValue - vmin;
        if (valueIndex < values.size()) {
          dc->drawText(width() / 2, y, values[valueIndex].c_str(), fgColor, CENTERED);
        }
        else {
          dc->drawNumber(width() / 2, y, displayedValue, fgColor, CENTERED);
        }

        if (windowFlags & ROLLER_SEPARATION_LINES) {
          dc->drawPlainHorizontalLine(0, y - 10, width(), DISABLE_COLOR);
        }

        if (++index == valuesCount)
          index = 0;

        y += ROLLER_LINE_HEIGHT;
      }
    }

    void checkEvents() override
    {
      Window::checkEvents();

      if (lastEditMode != editMode) {
        updateScrollPositionFromValue();
        lastEditMode = editMode;
      }

      if (lastScrollPositionY == SCROLL_POSITION_INVALIDATED) {
        updateScrollPositionFromValue();
      }

#if defined(HARDWARE_TOUCH)
      if (touchState.state != TE_SLIDE) {
        if (scrollPositionY != lastScrollPositionY) {
          if (isEnabled()) {
            updateValueFromScrollPosition();
          }
          else {
            updateScrollPositionFromValue();
          }
        }
      }
#endif
    }

    // virtual not needed until now
    void setMin(int value)
    {
      Choice::setMin(value);
      invalidateScrollPosition();
    }

    // virtual not needed until now
    void setMax(int value)
    {
      Choice::setMax(value);
      invalidateScrollPosition();
    }

    // virtual not needed until now
    void setAvailableHandler(std::function<bool(int)> handler)
    {
      Choice::setAvailableHandler(std::move(handler));
      invalidateScrollPosition();
    }

    void invalidateScrollPosition()
    {
      lastScrollPositionY = SCROLL_POSITION_INVALIDATED;
    }

  protected:
    coord_t lastScrollPositionY = SCROLL_POSITION_INVALIDATED;
    bool lastEditMode = false;

    void updateScrollPositionFromValue()
    {
      setScrollPositionY(ROLLER_LINE_HEIGHT * (getIndexFromValue(this->getValue()) - 1));
      lastScrollPositionY = scrollPositionY;
    }

    void updateValueFromScrollPosition()
    {
      lastScrollPositionY = scrollPositionY;
      auto valuesCount = getValuesCount();
      auto newValue = getValueFromIndex(mod((scrollPositionY / ROLLER_LINE_HEIGHT) + 1, valuesCount));
      setValue(newValue);
      invalidate();
    }
};

}
