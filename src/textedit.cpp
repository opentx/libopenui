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

#if defined(SOFTWARE_KEYBOARD) || defined(SIMULATION)
void TextEdit::setEditMode(bool newEditMode)
{
  FormField::setEditMode(newEditMode);

#if defined(SOFTWARE_KEYBOARD)
  if (editMode) {
    TextKeyboard::show(this);
  }
#endif

#if defined(SIMULATION)
  enableSimulatorKeyboard(newEditMode);
#endif
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

#if defined(SOFTWARE_KEYBOARD) || defined(SIMULATION)
void TextEdit::onVirtualKeyEvent(event_t event)
{
  auto c = event & MSK_VIRTUAL_KEY;
  if (c == SPECIAL_KEY_BACKSPACE) {
    if (cursorPos > 0) {
      auto pos = getUnicodeStringAtPosition(value, cursorPos - 1);
      char * tmp = pos;
      auto len = getUnicodeCharLength(getNextUnicodeChar(tmp));
      memmove(pos, pos + len, value + length - pos - len);
      memset(value + length - len, 0, len);
      --cursorPos;
      invalidate();
      changed = true;
    }
  }
  else {
    auto len = getUnicodeCharLength(c);
    if (strlen(value) + len <= length) {
      insertUnicodeChar(value, cursorPos, c, length);
      cursorPos++;
      invalidate();
      changed = true;
    }
  }
}
#endif

void TextEdit::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

#if defined(SOFTWARE_KEYBOARD) || defined(SIMULATION)
  if (IS_VIRTUAL_KEY_EVENT(event)) {
    onVirtualKeyEvent(event);
    return;
  }
#endif

#if defined(HARDWARE_KEYS)
  if (editMode) {
    auto currentChar = getUnicodeCharAtPosition(value, cursorPos);
    auto nextChar = currentChar;

    switch (event) {
      case EVT_ROTARY_RIGHT:
        for (int i = 0; i < ROTARY_ENCODER_SPEED(); i++) {
          nextChar = getNextAvailableChar(nextChar);
        }
        break;

      case EVT_ROTARY_LEFT:
        for (int i = 0; i < ROTARY_ENCODER_SPEED(); i++) {
          nextChar = getPreviousAvailableChar(nextChar);
        }
        break;

      case EVT_KEY_BREAK(KEY_LEFT):
        if (cursorPos > 0) {
          setCursorPos(cursorPos - 1);
        }
        break;

      case EVT_KEY_BREAK(KEY_RIGHT):
        if (cursorPos < getUnicodeStringLength(value)) {
          cursorPos++;
          invalidate();
        }
        break;

      case EVT_KEY_BREAK(KEY_ENTER):
        if (cursorPos < length - 1) {
          if (value[cursorPos] == '\0') {
            value[cursorPos] = ' ';
            changed = true;
          }
          cursorPos++;
          if (value[cursorPos] == '\0') {
            value[cursorPos] = ' ';
            changed = true;
          }
          invalidate();
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
        nextChar = toggleCase(nextChar);
        break;

      case EVT_KEY_LONG(KEY_LEFT):
      case EVT_KEY_LONG(KEY_RIGHT):
        nextChar = toggleCase(nextChar);
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

    if (cursorPos < length && currentChar != nextChar) {
      // TRACE("value[%d] = %d", cursorPos, v);
      // write(value[cursorPos] = nextChar;
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
    uint8_t w = font->getGlyph(c).width + 1;
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
