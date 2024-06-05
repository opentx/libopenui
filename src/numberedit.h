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

#include "basenumberedit.h"
#include <string>

namespace ui {

class NumberEdit: public BaseNumberEdit
{
  public:
    NumberEdit(Window * parent, const rect_t & rect, int vmin, int vmax, std::function<int()> getValue, std::function<void(int)> setValue = nullptr, WindowFlags windowFlags = 0, LcdFlags textFlags = 0);

#if defined(HARDWARE_TOUCH)
    void deleteLater(bool detach = true, bool trash = true) override;
#endif

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      if (deleted())
        return "NumberEdit()";
      else
        return "NumberEdit(" + std::to_string(getValue()) + ")";
    }
#endif

    void paint(BitmapBuffer * dc) override;

    void setAvailableHandler(std::function<bool(int)> handler)
    {
      isValueAvailable = std::move(handler);
    }

    void setGetStringValueHandler(std::function<std::string(int)> handler)
    {
      _getStringValue = std::move(handler);
    }

    void setPrefix(std::string value)
    {
      prefix = std::move(value);
    }

    void setSuffix(std::string value)
    {
      suffix = std::move(value);
    }

    void setZeroText(std::string value)
    {
      zeroText = std::move(value);
    }

    void setDisplayHandler(std::function<void(BitmapBuffer *, LcdFlags, int)> handler)
    {
      displayFunction = std::move(handler);
    }

    void onEvent(event_t event) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    void enableKeyboard(bool enable)
    {
#if defined(SOFTWARE_KEYBOARD)
      keyboardEnabled = enable;
#endif
    }

    void disableKeyboard()
    {
      enableKeyboard(false);
    }

    void onFocusLost() override;

#if defined(SOFTWARE_KEYBOARD)
    void setEditMode(bool newEditMode) override;
#endif

    std::string getStringValue(int value) const
    {
      if (value == 0 && !zeroText.empty())
        return zeroText;
      else if (_getStringValue)
        return _getStringValue(value);
      else
        return getDefaultStringValue(value);
    }

    std::string getDefaultStringValue(int value) const
    {
      return numberToString(value, 0, prefix.c_str(), suffix.c_str(), textFlags);
    }

  protected:
    std::function<void(BitmapBuffer *, LcdFlags, int)> displayFunction;
    std::string prefix;
    std::string suffix;
    std::string zeroText;
    std::function<bool(int)> isValueAvailable;
    std::function<std::string(int)> _getStringValue;
#if defined(SOFTWARE_KEYBOARD)
    bool keyboardEnabled = true;
#endif
};

}
