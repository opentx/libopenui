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

#include <vector>
#include "form.h"

enum ChoiceType {
  CHOICE_TYPE_DROPOWN,
  CHOICE_TYPE_FOLDER,
};

class ChoiceBase : public FormField
{
  public:
    ChoiceBase(FormGroup * parent, const rect_t & rect, ChoiceType type = CHOICE_TYPE_DROPOWN, WindowFlags windowFlags = 0):
      FormField(parent, rect, windowFlags),
      type(type)
    {
    }

    inline ChoiceType getType() const
    {
      return type;
    }

  protected:
    ChoiceType type;
};

class Choice: public ChoiceBase
{
  public:
    Choice(FormGroup * parent, const rect_t & rect, int vmin, int vmax, std::function<int()> getValue, std::function<void(int)> setValue = nullptr, WindowFlags windowFlags = 0);
    Choice(FormGroup * parent, const rect_t & rect, std::vector<std::string> values, int vmin, int vmax, std::function<int()> getValue, std::function<void(int)> setValue = nullptr, WindowFlags windowFlags = 0);
    Choice(FormGroup * parent, const rect_t & rect, const char * const values[], int vmin, int vmax, std::function<int()> getValue, std::function<void(int)> setValue = nullptr, WindowFlags windowFlags = 0);
    Choice(FormGroup * parent, const rect_t & rect, const char * values, int vmin, int vmax, std::function<int()> getValue, std::function<void(int)> setValue = nullptr, WindowFlags windowFlags = 0);

    void addValue(const char * value);

    void addValues(const char * const values[], uint8_t count);

    void setValues(std::vector<std::string> values);

    void setValues(const char * const values[]);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "Choice";
    }
#endif

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    void setSetValueHandler(std::function<void(int)> handler)
    {
      setValue = std::move(handler);
    }

    void setAvailableHandler(std::function<bool(int)> handler)
    {
      isValueAvailable = std::move(handler);
    }

    unsigned getIndexFromValue(int value) const
    {
      if (!isValueAvailable) {
        return value - vmin;
      }

      unsigned index = 0;
      for (int i = vmin; i < value; i++) {
        if (isValueAvailable(i)) {
          index++;
        }
      }
      return index;
    }

    int getValueFromIndex(int index) const
    {
      if (!isValueAvailable) {
        return vmin + index;
      }

      int value = vmin - 1;
      while (index >= 0) {
        index--;
        value++;
        while (value < vmax && !isValueAvailable(value)) {
          value++;
        }
      }
      return value;
    }

    unsigned getValuesCount() const
    {
      return getIndexFromValue(vmax + 1);
    }

    void setTextHandler(std::function<std::string(int)> handler)
    {
      textHandler = std::move(handler);
    }

    void setMenuTitle(std::string value)
    {
      menuTitle = std::move(value);
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

  protected:
    std::vector<std::string> values;
    int vmin = 0;
    int vmax = 0;
    std::string menuTitle;
    std::function<int()> getValue;
    std::function<void(int)> setValue;
    std::function<bool(int)> isValueAvailable;
    std::function<std::string(int)> textHandler;

    virtual void openMenu();
};
