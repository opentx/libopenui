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

class TextEdit: public FormField
{
  friend class TextKeyboard;

  public:
    TextEdit(Window * parent, const rect_t & rect, char * value, uint8_t length, LcdFlags windowFlags = 0) :
      FormField(parent, rect, windowFlags),
      value(value),
      length(length)
    {
    }

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "TextEdit";
    }
#endif

    void setChangeHandler(std::function<void()> handler)
    {
      changeHandler = std::move(handler);
    }

#if defined(SOFTWARE_KEYBOARD) || defined(SIMULATION)
    void setEditMode(bool newEditMode) override;
#endif

    uint8_t getMaxLength() const
    {
      return length;
    }

    char * getData() const
    {
      return value;
    }

    void paint(BitmapBuffer * dc) override;

    void onEvent(event_t event) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    void onFocusLost() override;

    void setCursorPos(uint8_t value)
    {
      cursorPos = value;
      invalidate();
    }

  protected:
    char * value;
    bool changed = false;
    uint8_t length;
    uint8_t cursorPos = 0;
    std::function<void()> changeHandler = nullptr;

    void trim();

    void changeEnd(bool forceChanged = false)
    {
      cursorPos = 0;
      if (changed || forceChanged) {
        changed = false;
        trim();
        if (changeHandler) {
          changeHandler();
        }
      }
    }

    static uint32_t getNextAvailableChar(uint32_t index)
    {
      auto end = mFont.end();
      while (++index < uint32_t(end)) {
        if (mFont.getGlyph(index).width > 0) {
          return index;
        }
      }
      return index;
    }

    static uint32_t getPreviousAvailableChar(uint32_t index)
    {
      auto begin = mFont.begin();
      while (--index >= uint32_t(begin)) {
        if (mFont.getGlyph(index).width > 0) {
          return index;
        }
      }
      return index;
    }

    static wchar_t toggleCase(wchar_t c)
    {
      if (c >= 'A' && c <= 'Z')
        return c + 32; // tolower
      else if (c >= 'a' && c <= 'z')
        return c - 32; // toupper
      else
        return c;
    }

    void onVirtualKeyEvent(event_t event);
};

}
