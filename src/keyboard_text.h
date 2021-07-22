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

#define KEYBOARD_SPACE         "\t"
#define KEYBOARD_ENTER         "\n"
#define KEYBOARD_BACKSPACE     "\200"
#define KEYBOARD_SET_UPPERCASE "\201"
#define KEYBOARD_SET_LOWERCASE "\202"
#define KEYBOARD_SET_LETTERS   "\203"
#define KEYBOARD_SET_NUMBERS   "\204"

extern const uint8_t LBM_KEY_SPACEBAR[];
extern const uint8_t * const LBM_SPECIAL_KEYS[];
extern const char * const * KEYBOARDS[];

enum LayoutIndex
{
  LAYOUT_INDEX_UPPERCASE,
  LAYOUT_INDEX_LOWERCASE,
  LAYOUT_INDEX_NUMBERS,
};

static const uint8_t layoutIndexes[] = {
  LAYOUT_INDEX_UPPERCASE,
  LAYOUT_INDEX_LOWERCASE,
  LAYOUT_INDEX_LOWERCASE,
  LAYOUT_INDEX_NUMBERS,
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
      if (!_instance)
        _instance = new TextKeyboard();
      _instance->setField(field);
    }

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

  protected:
    static TextKeyboard * _instance;
    unsigned layoutIndex = LAYOUT_INDEX_LOWERCASE;
};
