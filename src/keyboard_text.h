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

enum SpecialKey
{
  SPECIAL_KEY_BACKSPACE = 1,
  SPECIAL_KEY_SET_UPPERCASE = 2,
  SPECIAL_KEY_SET_LOWERCASE = 3,
  SPECIAL_KEY_SET_LETTERS = 4,
  SPECIAL_KEY_SET_NUMBERS = 5,
  SPECIAL_KEY_SPACE = '\t',
  SPECIAL_KEY_ENTER = '\n',
};

#define KEYBOARD_BACKSPACE     "\001"
#define KEYBOARD_SET_UPPERCASE "\002"
#define KEYBOARD_SET_LOWERCASE "\003"
#define KEYBOARD_SET_LETTERS   "\004"
#define KEYBOARD_SET_NUMBERS   "\005"
#define KEYBOARD_SPACE         "\t"
#define KEYBOARD_ENTER         "\n"

extern const uint8_t LBM_KEY_SPACEBAR[];
extern const uint8_t * const LBM_SPECIAL_KEYS[];
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

  KEYBOARD_LAYOUT_NUMBERS,
  KEYBOARD_LAYOUT_COUNT
};

class TextKeyboard: public Keyboard
{
  public:
    TextKeyboard();

    ~TextKeyboard() override;

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "TextKeyboard";
    }
#endif

    static void setInstance(TextKeyboard * keyboard)
    {
      _instance = keyboard;
    }

    static TextKeyboard * instance()
    {
      return _instance;
    }

    static void show(FormField * field)
    {
      if (!_instance) {
        _instance = new TextKeyboard();
      }
      _instance->layoutIndex = KEYBOARD_LAYOUT_INDEX_START;
      _instance->setField(field);
    }

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

  protected:
    static TextKeyboard * _instance;
    unsigned layoutIndex = KEYBOARD_LAYOUT_INDEX_START;
};
