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

#include "libopenui_config.h"
#include "button.h"
#include "font.h"
#include "theme.h"

using namespace ui;

void Button::onPress()
{
  bool check = (pressHandler && pressHandler());
  if (check != bool(windowFlags & BUTTON_CHECKED)) {
    windowFlags ^= BUTTON_CHECKED;
    invalidate();
  }
}

#if defined(HARDWARE_KEYS)
void Button::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("Button").c_str(), event);

  if (enabled && event == EVT_KEY_BREAK(KEY_ENTER)) {
    onKeyPress();
    onPress();
  }
  else {
    FormField::onEvent(event);
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool Button::onTouchEnd(coord_t x, coord_t y)
{
  if (enabled) {
    onKeyPress();
    if (!(windowFlags & NO_FOCUS)) {
      setFocus(SET_FOCUS_DEFAULT);
    }
    onPress();
  }
  return true;
}
#endif

void Button::checkEvents()
{
  Window::checkEvents();
  if (checkHandler)
    checkHandler();
}

void TextButton::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);

  auto textColor = DEFAULT_COLOR;

  if (checked()) {
    if (hasFocus()) {
      dc->drawPlainRectangle(0, 0, rect.w, rect.h, FOCUS_BGCOLOR, 2);
      dc->drawPlainFilledRectangle(3, 3, rect.w - 6, rect.h - 6, FOCUS_BGCOLOR);
    }
    else {
      dc->drawPlainRectangle(0, 0, rect.w, rect.h, DISABLE_COLOR, 1);
      dc->drawPlainFilledRectangle(2, 2, rect.w - 4, rect.h - 4, FOCUS_BGCOLOR);
    }
    textColor = FOCUS_COLOR;
  }
  else {
    if (windowFlags & BUTTON_BACKGROUND) {
      if (hasFocus()) {
        dc->drawPlainRectangle(0, 0, rect.w, rect.h, FOCUS_BGCOLOR, 2);
        dc->drawPlainFilledRectangle(2, 2, rect.w - 4, rect.h - 4, DISABLE_COLOR);
      }
      else {
        dc->drawPlainFilledRectangle(0, 0, rect.w, rect.h, DISABLE_COLOR);
      }
    }
    else {
      if (hasFocus()) {
        dc->drawPlainRectangle(0, 0, rect.w, rect.h, FOCUS_BGCOLOR, 2);
      }
      else {
        dc->drawPlainRectangle(0, 0, rect.w, rect.h, DISABLE_COLOR, 1);
      }
    }
  }

  dc->drawText(rect.w / 2, 1 + (rect.h - getFontHeight(textFlags)) / 2, text.c_str(), textColor, CENTERED | textFlags);
}

void IconButton::paint(BitmapBuffer * dc)
{
  dc->drawBitmap(0, 0, theme->getIcon(icon, checked() ? STATE_PRESSED : STATE_DEFAULT));
}
