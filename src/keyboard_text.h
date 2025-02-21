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

#define KEYBOARD_BACKSPACE     "\x01"
#define KEYBOARD_SET_UPPERCASE "\x02"
#define KEYBOARD_SET_LOWERCASE "\x03"
#define KEYBOARD_SET_LETTERS   "\x04"
#define KEYBOARD_SET_NUMBERS   "\x05"
#define KEYBOARD_SPACE         "\x06"
#define KEYBOARD_ENTER         "\x07"
#define KEYBOARD_SPECIAL_CHAR_1 "\x08"
#define KEYBOARD_SPECIAL_CHAR_2 "\x09"
#define KEYBOARD_SPECIAL_CHAR_3 "\x0A"
#define KEYBOARD_SPECIAL_CHAR_4 "\x0B"
#define KEYBOARD_SPECIAL_CHAR_5 "\x0C"
#define KEYBOARD_SPECIAL_CHAR_6 "\x0D"
#define KEYBOARD_SPECIAL_CHAR_7 "\x0E"
#define KEYBOARD_SPECIAL_CHAR_8 "\x0F"
#define KEYBOARD_SPECIAL_CHAR_9 "\x10"
#define KEYBOARD_SPECIAL_CHAR_10 "\x11"
#define KEYBOARD_SPECIAL_CHAR_11 "\x12"
#define KEYBOARD_SPECIAL_CHAR_12 "\x13"
#define KEYBOARD_SPECIAL_CHAR_13 "\x14"
#define KEYBOARD_SPECIAL_CHAR_14 "\x15"
#define KEYBOARD_SPECIAL_CHAR_15 "\x16"
#define KEYBOARD_SPECIAL_CHAR_16 "\x17"
#define KEYBOARD_SPECIAL_CHAR_17 "\x18"
#define KEYBOARD_SPECIAL_CHAR_18 "\x19"
#define KEYBOARD_SPECIAL_CHAR_19 "\x1A"
#define KEYBOARD_SPECIAL_CHAR_LAST KEYBOARD_SPECIAL_CHAR_19

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

  KEYBOARD_LAYOUT_QWERTZ,
  KEYBOARD_LAYOUT_QWERTZ_UPPERCASE = KEYBOARD_LAYOUT_QWERTZ,
  KEYBOARD_LAYOUT_QWERTZ_LOWERCASE = KEYBOARD_LAYOUT_QWERTZ + LOWERCASE_OPTION,

  KEYBOARD_LAYOUT_NUMBERS,
  KEYBOARD_LAYOUT_COUNT
};

namespace ui {

class TextKeyboard: public Keyboard<FormField>
{
  public:
    TextKeyboard();

    ~TextKeyboard() override;

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
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
      if (activeKeyboard != _instance) {
        _instance->layoutIndex = KEYBOARD_LAYOUT_INDEX_START;
      }
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

}
