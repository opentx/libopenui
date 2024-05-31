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

#include "numberedit.h"

#if defined(SOFTWARE_KEYBOARD)
#include "keyboard_number.h"
#endif

using namespace ui;

NumberEdit::NumberEdit(Window * parent, const rect_t & rect, int vmin, int vmax, std::function<int()> getValue, std::function<void(int)> setValue, WindowFlags windowFlags, LcdFlags textFlags):
  BaseNumberEdit(parent, rect, vmin, vmax, std::move(getValue), std::move(setValue), windowFlags, textFlags)
{
}

std::string NumberEdit::getStringValue(int value)
{
  if (value == 0 && !zeroText.empty())
    return zeroText;
  else if (_getStringValue)
    return _getStringValue(value);
  else
    return numberToString(value, 0, prefix.c_str(), suffix.c_str(), textFlags);
}

void NumberEdit::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);

  auto value = getValue();

  LcdFlags textColor;
  if (editMode)
    textColor = FOCUS_COLOR;
  else if (hasFocus())
    textColor = FOCUS_BGCOLOR;
  else if (enabled && (value != 0 || zeroText.empty()))
    textColor = DEFAULT_COLOR;
  else
    textColor = DISABLE_COLOR;

  if (displayFunction) {
    displayFunction(dc, textColor, value);
  }
  else {
    auto s = numberToString(value, 0, prefix.c_str(), suffix.c_str(), textFlags);
    if (textFlags & RIGHT)
      dc->drawText(rect.w - FIELD_PADDING_LEFT, FIELD_PADDING_TOP, s, textColor, textFlags);
    else
      dc->drawText(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, s, textColor, textFlags);
  }
}

void NumberEdit::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (editMode) {
    switch (event) {
#if defined(HARDWARE_KEYS)
      case EVT_ROTARY_RIGHT: {
        int value = getValue();
        if (value < vmax) {
          do {
            value += ROTARY_ENCODER_SPEED() * step;
          } while (isValueAvailable && !isValueAvailable(value) && value <= vmax);
          setValue(value);
          onKeyPress();
        }
        else {
          onKeyError();
        }
        return;
      }

      case EVT_ROTARY_LEFT: {
        int value = getValue();
        if (value > vmin) {
          do {
            value -= ROTARY_ENCODER_SPEED() * step;
          } while (isValueAvailable && !isValueAvailable(value) && value >= vmin);
          setValue(value);
          onKeyPress();
        }
        else {
          onKeyError();
        }
        return;
      }
#endif
    }
  }

  FormField::onEvent(event);
}

#if defined(HARDWARE_TOUCH)
bool NumberEdit::onTouchEnd(coord_t, coord_t)
{
  if (enabled) {
    if (!hasFocus()) {
      setFocus(SET_FOCUS_DEFAULT);
    }
    setEditMode(true);
  }
  return true;
}
#endif

void NumberEdit::onFocusLost()
{
#if defined(SOFTWARE_KEYBOARD)
  KeyboardBase::hide();
#endif

  FormField::onFocusLost();
}

#if defined(SOFTWARE_KEYBOARD)
void NumberEdit::setEditMode(bool newEditMode)
{
  BaseNumberEdit::setEditMode(newEditMode);
  if (editMode && keyboardEnabled) {
    NumberKeyboard::show(this);
  }
}
#endif

#if defined(SOFTWARE_KEYBOARD)
void NumberEdit::deleteLater(bool detach, bool trash)
{
  if (hasFocus()) {
    KeyboardBase::hide();
  }
  BaseNumberEdit::deleteLater(detach, trash);
}
#endif
