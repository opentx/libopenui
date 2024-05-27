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

#include "keyboard_number.h"
#include "button.h"
#include "basenumberedit.h"

using namespace ui;

NumberKeyboard * NumberKeyboard::_instance = nullptr;

void NumberKeyboard::show(NumberEdit * field)
{
  if (!_instance) {
    _instance = new DefaultNumberKeyboard();
  }
  _instance->setField(field);
}

DefaultNumberKeyboard::DefaultNumberKeyboard() :
  NumberKeyboard()
{
  new TextButton(
      this, {LCD_W / 2 - 115, 10, 50, 30}, "<<",
      [=]() {
        int value = field->getValue();
        if (value > field->getMin())
          field->setValue(value - 10 * field->getStep());
        else
          onKeyError();
        return 0;
      },
      BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(
      this, {LCD_W / 2 - 55, 10, 50, 30}, "-",
      [=]() {
        int value = field->getValue();
        if (value > field->getMin())
          field->setValue(value - field->getStep());
        else
          onKeyError();
        return 0;
      },
      BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(
      this, {LCD_W / 2 + 5, 10, 50, 30}, "+",
      [=]() {
        int value = field->getValue();
        if (value < field->getMax())
          field->setValue(value + field->getStep());
        else
          onKeyError();
        return 0;
      },
      BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(
      this, {LCD_W / 2 + 65, 10, 50, 30}, ">>",
      [=]() {
        int value = field->getValue();
        if (value < field->getMax())
          field->setValue(value + 10 * field->getStep());
        else
          onKeyError();
        return 0;
      },
      BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(
      this, {LCD_W / 2 - 115, 50, 50, 30}, "MIN",
      [=]() {
        field->setValue(field->getMin());
        return 0;
      },
      BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(
      this, {LCD_W / 2 + 65, 50, 50, 30}, "MAX",
      [=]() {
        field->setValue(field->getMax());
        return 0;
      },
      BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(
      this, {LCD_W / 2 - 55, 50, 110, 30}, "DEFAULT",
      [=]() {
        field->setValue(field->getDefault());
        return 0;
      },
      BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);
}

void DefaultNumberKeyboard::paint(BitmapBuffer * dc)
{
  dc->clear(RGB565(0xE0, 0xE0, 0xE0));
}
