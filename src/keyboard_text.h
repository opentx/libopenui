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

#include "keyboard_base.h"
#include "textedit.h"

extern const char * const * KEYBOARDS[];

constexpr uint8_t LOWERCASE_OPTION = 1;

enum KeyboardLayout
{
  KEYBOARD_LAYOUT_QWERTY,
  KEYBOARD_LAYOUT_QWERTY_UPPERCASE = KEYBOARD_LAYOUT_QWERTY,
  KEYBOARD_LAYOUT_QWERTY_LOWERCASE = KEYBOARD_LAYOUT_QWERTY + LOWERCASE_OPTION,

  KEYBOARD_LAYOUT_AZERTY,
  KEYBOARD_LAYOUT_AZERTY_UPPERCASE = KEYBOARD_LAYOUT_AZERTY,
  KEYBOARD_LAYOUT_AZERTY_LOWERCASE = KEYBOARD_LAYOUT_AZERTY + LOWERCASE_OPTION,

  KEYBOARD_LAYOUT_QWERTZ,
  KEYBOARD_LAYOUT_QWERTZ_UPPERCASE = KEYBOARD_LAYOUT_QWERTZ,
  KEYBOARD_LAYOUT_QWERTZ_LOWERCASE = KEYBOARD_LAYOUT_QWERTZ + LOWERCASE_OPTION,

  KEYBOARD_LAYOUT_NUMBERS,
  KEYBOARD_LAYOUT_COUNT
};

namespace ui {

class TextKeyboardBase: public Keyboard<FormField>
{
  public:
    TextKeyboardBase():
      Keyboard(TEXT_KEYBOARD_HEIGHT)
    {
    }

    ~TextKeyboardBase() override
    {
      _instance = nullptr;
    }

    static void show(FormField * field)
    {
      if (activeKeyboard != _instance) {
        _instance->layoutIndex = KEYBOARD_LAYOUT_INDEX_START;
      }
      _instance->setField(field);
    }

    static void setInstance(TextKeyboardBase * keyboard)
    {
      _instance = keyboard;
    }

    static TextKeyboardBase * instance()
    {
      return _instance;
    }

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "TextKeyboard";
    }
#endif

  protected:
    static TextKeyboardBase * _instance;
    unsigned layoutIndex = KEYBOARD_LAYOUT_INDEX_START;
};

class TextKeyboard: public TextKeyboardBase
{
  protected:
    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif
};

}
