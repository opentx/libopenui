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

namespace ui {
  
class BaseNumberEdit: public FormField
{
  public:
    BaseNumberEdit(Window * parent, const rect_t & rect, int vmin, int vmax,
                   std::function<int()> getValue, std::function<void(int)> setValue = nullptr, WindowFlags windowFlags = 0, LcdFlags textFlags = 0):
      FormField(parent, rect, windowFlags, textFlags),
      vmin(vmin),
      vmax(vmax),
      _getValue(std::move(getValue)),
      _setValue(std::move(setValue))
    {
    }

    void setMin(int value)
    {
      vmin = value;
    }

    void setMax(int value)
    {
      vmax = value;
    }

    void setDefault(int value)
    {
      vdefault = value;
    }

    int getMin() const
    {
      return vmin;
    }

    int getMax() const
    {
      return vmax;
    }

    int getDefault() const
    {
      return vdefault;
    }

    void setStep(int value)
    {
      step = value;
    }

    int getStep() const
    {
      return step;
    }

    void setValue(int value)
    {
      currentValue = limit(vmin, value, vmax);
      if (instantChange)
        _setValue(currentValue);
      else
        dirty = true;
      invalidate();
    }

    void enableInstantChange(bool value)
    {
      instantChange = value;
    }

    void disableInstantChange()
    {
      enableInstantChange(false);
    }

    void setSetValueHandler(std::function<void(int)> handler)
    {
      _setValue = std::move(handler);
    }

    int getValue() const
    {
      return (editMode && !instantChange) ? currentValue : _getValue();
    }

    void setEditMode(bool newEditMode) override
    {
      if (editMode != newEditMode) {
        FormField::setEditMode(newEditMode);
        if (!instantChange) {
          if (newEditMode) {
            if (!dirty) {
              currentValue = _getValue();
            }
          }
          else {
            if (dirty) {
              _setValue(currentValue);
              dirty = false;
            }
          }
        }
      }
    }

    void setStepMultiplier(int value)
    {
      stepMultiplier = value;
    }

    int getStepMultiplier() const
    {
      return stepMultiplier;
    }

  protected:
    int vdefault = 0;
    int vmin;
    int vmax;
    int step = 1;
    int stepMultiplier = 10;
    int currentValue;
    bool instantChange = true;
    bool dirty = false;
    std::function<int()> _getValue;
    std::function<void(int)> _setValue;
};

}
