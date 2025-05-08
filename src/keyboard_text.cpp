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

#include "keyboard_text.h"
#include "font.h"

using namespace ui;

TextKeyboard * TextKeyboard::_instance = nullptr;

const uint8_t LBM_KEY_UPPERCASE[] = {
#include "bitmaps/mask_key_uppercase.lbm"
};

const uint8_t LBM_KEY_LOWERCASE[] = {
#include "bitmaps/mask_key_lowercase.lbm"
};

const uint8_t LBM_KEY_BACKSPACE[] = {
#include "bitmaps/mask_key_backspace.lbm"
};

const uint8_t LBM_KEY_LETTERS[] = {
#include "bitmaps/mask_key_letters.lbm"
};

const uint8_t LBM_KEY_NUMBERS[] = {
#include "bitmaps/mask_key_numbers.lbm"
};

const uint8_t LBM_KEY_SPACEBAR[] = {
#include "bitmaps/mask_key_spacebar.lbm"
};

const uint8_t * const LBM_SPECIAL_KEYS[] = {
  LBM_KEY_BACKSPACE,
  LBM_KEY_UPPERCASE,
  LBM_KEY_LOWERCASE,
  LBM_KEY_LETTERS,
  LBM_KEY_NUMBERS,
};

const char * const KEYBOARD_QWERTY_LOWERCASE[] = {
  "qwertyuiop",
  " asdfghjkl",
  KEYBOARD_SET_UPPERCASE "zxcvbnm" KEYBOARD_BACKSPACE,
  KEYBOARD_SET_NUMBERS KEYBOARD_SPACEBAR KEYBOARD_ENTER
};

const char * const KEYBOARD_AZERTY_LOWERCASE[] = {
  "azertyuiop",
  " qsdfghjklm",
  KEYBOARD_SET_UPPERCASE "wxcvbn," KEYBOARD_BACKSPACE,
  KEYBOARD_SET_NUMBERS KEYBOARD_SPACEBAR KEYBOARD_ENTER
};

const char * const KEYBOARD_QWERTZ_LOWERCASE[] = {
  "qwertzuiop",
  " asdfghjkl",
  KEYBOARD_SET_UPPERCASE "yxcvbnm" KEYBOARD_BACKSPACE,
  KEYBOARD_SET_NUMBERS KEYBOARD_SPACEBAR KEYBOARD_ENTER
};

const char * const KEYBOARD_QWERTY_UPPERCASE[] = {
  "QWERTYUIOP",
  " ASDFGHJKL",
  KEYBOARD_SET_LOWERCASE "ZXCVBNM" KEYBOARD_BACKSPACE,
  KEYBOARD_SET_NUMBERS KEYBOARD_SPACEBAR KEYBOARD_ENTER
};

const char * const KEYBOARD_AZERTY_UPPERCASE[] = {
  "AZERTYUIOP",
  " QSDFGHJKLM",
  KEYBOARD_SET_LOWERCASE "WXCVBN," KEYBOARD_BACKSPACE,
  KEYBOARD_SET_NUMBERS KEYBOARD_SPACEBAR KEYBOARD_ENTER
};

const char * const KEYBOARD_QWERTZ_UPPERCASE[] = {
  "QWERTZUIOP",
  " ASDFGHJKL",
  KEYBOARD_SET_LOWERCASE "YXCVBNM" KEYBOARD_BACKSPACE,
  KEYBOARD_SET_NUMBERS KEYBOARD_SPACEBAR KEYBOARD_ENTER
};

const char * const KEYBOARD_NUMBERS[] = {
  "1234567890",
  KEYBOARD_SPECIAL_CHAR_1 KEYBOARD_SPECIAL_CHAR_2 KEYBOARD_SPECIAL_CHAR_3 KEYBOARD_SPECIAL_CHAR_4 KEYBOARD_SPECIAL_CHAR_5 KEYBOARD_SPECIAL_CHAR_6 KEYBOARD_SPECIAL_CHAR_7 KEYBOARD_SPECIAL_CHAR_8 KEYBOARD_SPECIAL_CHAR_9 KEYBOARD_SPECIAL_CHAR_10,
  KEYBOARD_SPECIAL_CHAR_11 KEYBOARD_SPECIAL_CHAR_12 KEYBOARD_SPECIAL_CHAR_13 KEYBOARD_SPECIAL_CHAR_14 KEYBOARD_SPECIAL_CHAR_15 KEYBOARD_SPECIAL_CHAR_16 KEYBOARD_SPECIAL_CHAR_17 KEYBOARD_SPECIAL_CHAR_18 KEYBOARD_SPECIAL_CHAR_19 KEYBOARD_BACKSPACE,
  KEYBOARD_SET_LETTERS KEYBOARD_SPACEBAR KEYBOARD_ENTER
};

const char * const * KEYBOARDS[] = {
  KEYBOARD_QWERTY_UPPERCASE,
  KEYBOARD_QWERTY_LOWERCASE,
  KEYBOARD_AZERTY_UPPERCASE,
  KEYBOARD_AZERTY_LOWERCASE,
  KEYBOARD_QWERTZ_UPPERCASE,
  KEYBOARD_QWERTZ_LOWERCASE,
  KEYBOARD_NUMBERS,
};

TextKeyboard::TextKeyboard():
  Keyboard(TEXT_KEYBOARD_HEIGHT)
{
}

TextKeyboard::~TextKeyboard()
{
  _instance = nullptr;
}

void TextKeyboard::paint(BitmapBuffer * dc)
{
  auto layout = KEYBOARDS[layoutIndex];

  dc->clear(RGB565(0xE0, 0xE0, 0xE0));

  for (uint8_t i = 0; i < 4; i++) {
    coord_t y = 15 + i * 40;
    coord_t x = 15;
    const char * c = layout[i];
    while (*c) {
      if (*c == ' ') {
        x += 15;
      }
      else if (uint8_t(*c) <= SPECIAL_KEY_WITH_BITMAP_LAST) {
        // special keys drawn with a bitmap
        dc->drawMask(x, y, (const BitmapData *)LBM_SPECIAL_KEYS[uint8_t(*c) - 1], DEFAULT_COLOR);
        x += 45;
      }
      else if (*c == SPECIAL_KEY_SPACEBAR) {
        // spacebar
        dc->drawMask(x, y, (const BitmapData *)LBM_KEY_SPACEBAR, DEFAULT_COLOR);
        x += 135;
      }
      else if (*c == SPECIAL_KEY_ENTER) {
        // enter
        dc->drawPlainFilledRectangle(x, y - 2, 80, 25, DISABLE_COLOR);
        dc->drawText(x + 40, y, "ENTER", DEFAULT_COLOR, CENTERED);
        x += 80;
      }
      else {
        dc->drawSizedText(x, y, c, 1, DEFAULT_COLOR);
        x += 30;
      }
      c++;
    }
  }
}

bool TextKeyboard::onTouchEnd(coord_t x, coord_t y)
{
  auto layout = KEYBOARDS[layoutIndex];

  onKeyPress();

  uint8_t row = max<coord_t>(0, y - 5) / 40;
  const uint8_t * key = (uint8_t *)layout[row];
  while (*key) {
    if (*key == ' ') {
      x -= 15;
    }
    else if (*key == SPECIAL_KEY_SPACEBAR) {
      if (x <= 135) {
        pushEvent(EVT_VIRTUAL_KEY(' '));
        return true;
      }
      x -= 135;
    }
    else if (*key == SPECIAL_KEY_ENTER) {
      if (x <= 80) {
        // enter
        hide();
        return true;
      }
      x -= 80;
    }
    else if (*key <= SPECIAL_KEY_WITH_BITMAP_LAST) {
      if (x <= 45) {
        switch (*key) {
          case SPECIAL_KEY_BACKSPACE:
            // backspace
            events.push(EVT_VIRTUAL_KEY(SPECIAL_KEY_BACKSPACE));
            break;
          case SPECIAL_KEY_SET_UPPERCASE:
            layoutIndex = getKeyboardLayout();
            invalidate();
            break;
          case SPECIAL_KEY_SET_LOWERCASE:
            layoutIndex = getKeyboardLayout() + LOWERCASE_OPTION;
            invalidate();
            break;
          case SPECIAL_KEY_SET_LETTERS:
            layoutIndex = getKeyboardLayout() + LOWERCASE_OPTION;
            invalidate();
            break;
          case SPECIAL_KEY_SET_NUMBERS:
            layoutIndex = KEYBOARD_LAYOUT_NUMBERS;
            invalidate();
            break;
        }
        break;
      }
      x -= 45;
    }
    else {
      if (x <= 30) {
        pushEvent(EVT_VIRTUAL_KEY(*key));
        return true;
      }
      x -= 30;
    }
    key++;
  }

  return true;
}
