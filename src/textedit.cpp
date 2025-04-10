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

#include "textedit.h"
#include "font.h"
#include "libopenui_config.h"

#if defined(SOFTWARE_KEYBOARD)
#include "keyboard_text.h"
#endif

#if !defined(STR_EDIT)
#define STR_EDIT "Edit"
#endif

#if !defined(STR_CLEAR)
#define STR_CLEAR "Clear"
#endif

#if defined(HARDWARE_KEYS)
#include "menu.h"
#endif

#if defined(CLIPBOARD)
#include "clipboard.h"
#endif

#if defined(SOFTWARE_KEYBOARD)
void TextEdit::setEditMode(bool newEditMode)
{
  FormField::setEditMode(newEditMode);
  if (editMode) {
    TextKeyboard::show(this);
  }
}
#endif

void TextEdit::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);

  if (editMode) {
    dc->drawSizedText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value, length, FOCUS_COLOR);
    coord_t left = (cursorPos == 0 ? 0 : getTextWidth(value, cursorPos));
#if defined(SOFTWARE_KEYBOARD)
    dc->drawPlainFilledRectangle(left + 2, 2, 2, height() - 4, FOCUS_COLOR);
#else
    char s[] = { value[cursorPos], '\0' };
    dc->drawPlainFilledRectangle(FIELD_PADDING_LEFT + left - 1, FIELD_PADDING_TOP - 1, getTextWidth(s, 1) + 1, height() - 2, FOCUS_COLOR);
    dc->drawText(FIELD_PADDING_LEFT + left, FIELD_PADDING_TOP, s, DEFAULT_COLOR);
#endif
  }
  else {
    const char * displayedValue = value;
    LcdFlags textColor;
    if (hasFocus()) {
      if (strlen(value) == 0) {
        displayedValue = "---";
      }
      textColor = FOCUS_BGCOLOR;
    }
    else {
      if (strlen(value) == 0) {
        displayedValue = "---";
        textColor = DISABLE_COLOR;
      }
      else {
        textColor = DEFAULT_COLOR;
      }
    }
    dc->drawSizedText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, displayedValue, length, textColor);
  }
}

void TextEdit::trim()
{
  for (int i = length - 1; i >= 0; i--) {
    if (value[i] == ' ' || value[i] == '\0')
      value[i] = '\0';
    else
      break;
  }
}

void TextEdit::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

#if defined(SOFTWARE_KEYBOARD)
  if (IS_VIRTUAL_KEY_EVENT(event)) {
    uint8_t c = event & 0xFF;
    if (c == (uint8_t)KEYBOARD_BACKSPACE[0]) {
      if (cursorPos > 0) {
        memmove(value + cursorPos - 1, value + cursorPos, length - cursorPos);
        value[length - 1] = '\0';
        --cursorPos;
        invalidate();
        changed = true;
      }
    }
#if defined(KEYBOARD_DELETE)
    else if (c == SPECIAL_KEY_DELETE) {
      if (cursorPos < length - 1) {
        memmove(value + cursorPos, value + cursorPos + 1, length - cursorPos - 1);
        value[length - 1] = '\0';
        invalidate();
        changed = true;
      }
    }
#endif
#if defined(KEYBOARD_HOME)
    else if (c == SPECIAL_KEY_HOME) {
      setCursorPos(0);
    }
#endif
#if defined(KEYBOARD_END)
    else if (c == SPECIAL_KEY_END) {
      setCursorPos(length);
    }
#endif
    else if (cursorPos < length) {
      memmove(value + cursorPos + 1, value + cursorPos, length - cursorPos - 1);
      value[cursorPos++] = c;
      invalidate();
      changed = true;
    }
  }
#endif

#if defined(HARDWARE_KEYS)
  if (editMode) {
    char previousChar = (cursorPos > 0 ? value[cursorPos - 1] : 0);
    int c = (cursorPos < length ? value[cursorPos] : 0);
    int v = c;

    switch (event) {
      case EVT_ROTARY_RIGHT:
        for (int i = 0; i < ROTARY_ENCODER_SPEED(); i++) {
          v = getNextChar(v, previousChar);
        }
        break;

      case EVT_ROTARY_LEFT:
        for (int i = 0; i < ROTARY_ENCODER_SPEED(); i++) {
          v = getPreviousChar(v);
        }
        break;

      case EVT_KEY_BREAK(KEY_LEFT):
        if (cursorPos > 0) {
          setCursorPos(cursorPos - 1);
        }
        break;

      case EVT_KEY_BREAK(KEY_RIGHT):
      {
#if defined(SOFTWARE_KEYBOARD)
        if (cursorPos < length && value[cursorPos] != '\0') {
          setCursorPos(cursorPos + 1);
        }
#else
        if (cursorPos < length - 1 && value[cursorPos + 1] != '\0') {
          setCursorPos(cursorPos + 1);
        }
#endif
        break;
      }

      case EVT_KEY_BREAK(KEY_ENTER):
        if (cursorPos < length - 1) {
          if (value[cursorPos] == '\0') {
            value[cursorPos] = ' ';
            changed = true;
          }
          setCursorPos(cursorPos + 1);
          if (value[cursorPos] == '\0') {
            value[cursorPos] = ' ';
            changed = true;
          }
        }
        else {
          changeEnd();
          FormField::onEvent(event);
        }
        break;

      case EVT_KEY_BREAK(KEY_EXIT):
        changeEnd();
        FormField::onEvent(event);
#if defined(HARDWARE_TOUCH)
        TextKeyboard::hide();
#endif
        break;

      case EVT_KEY_LONG(KEY_ENTER):
      {
        killEvents(event);
        auto menu = new Menu(this);
        menu->setTitle(STR_EDIT);
#if defined(CLIPBOARD)
        menu->addLine(STR_COPY, [=] {
          clipboard.write((uint8_t *)value, length, Clipboard::CONTENT_TEXT);
        });
        if (clipboard.contentType == Clipboard::CONTENT_TEXT) {
          menu->addLine(STR_PASTE, [=] {
            clipboard.read((uint8_t *)value, length);
            changeEnd(true);
          });
        }
#endif
        menu->addLine(STR_CLEAR, [=] {
          memset(value, 0, length);
          changeEnd(true);
        });
        break;
      }

      case EVT_KEY_BREAK(KEY_UP):
        v = toggleCase(v);
        break;

      case EVT_KEY_LONG(KEY_LEFT):
      case EVT_KEY_LONG(KEY_RIGHT):
        v = toggleCase(v);
        if (event == EVT_KEY_LONG(KEY_LEFT)) {
          killEvents(KEY_LEFT);
        }
        break;

      case EVT_KEY_BREAK(KEY_PGDN):
        if (cursorPos < length) {
          memmove(&value[cursorPos], &value[cursorPos + 1], length - cursorPos - 1);
          value[length - 1] = '\0';
          changed = true;
          if (cursorPos > 0 && value[cursorPos] == '\0') {
            cursorPos = cursorPos - 1;
          }
          invalidate();
        }
        break;
    }

    if (cursorPos < length && c != v) {
      // TRACE("value[%d] = %d", cursorPos, v);
      value[cursorPos] = v;
      invalidate();
      changed = true;
    }
  }
  else {
    cursorPos = 0;
    FormField::onEvent(event);
  }
#endif
}

#if defined(HARDWARE_TOUCH)
bool TextEdit::onTouchEnd(coord_t x, coord_t y)
{
  if (!isEnabled()) {
    return true;
  }

  if (!hasFocus()) {
    setFocus(SET_FOCUS_DEFAULT);
  }

#if defined(SOFTWARE_KEYBOARD)
  TextKeyboard::show(this);
#endif

  auto font = getFont(FONT(STD));

  coord_t rest = x;
  for (cursorPos = 0; cursorPos < length; cursorPos++) {
    char c = value[cursorPos];
    if (c == '\0')
      break;
    uint8_t w = font->getChar(c).width + 1;
    if (rest < w)
      break;
    rest -= w;
  }

  invalidate();
  return true;
}
#endif

void TextEdit::onFocusLost()
{
#if defined(SOFTWARE_KEYBOARD)
  TextKeyboard::hide();
#endif

  changeEnd();
  FormField::onFocusLost();
}
