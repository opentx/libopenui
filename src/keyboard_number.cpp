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
#include "libopenui_globals.h"

NumberKeyboard * NumberKeyboard::_instance = nullptr;

void NumberKeyboard::show(FormField * field)
{
  if (!_instance)
    _instance = new DefaultNumberKeyboard();
  _instance->setField(field);
}

DefaultNumberKeyboard::DefaultNumberKeyboard() :
  NumberKeyboard()
{
  new TextButton(this, {LCD_W / 2 - 115, 10, 50, 30}, "<<",
                 [=]() {
                     pushEvent(EVT_VIRTUAL_KEY_BACKWARD);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(this, {LCD_W / 2 - 55, 10, 50, 30}, "-",
                 [=]() {
                     pushEvent(EVT_VIRTUAL_KEY_MINUS);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(this, {LCD_W / 2 + 5, 10, 50, 30}, "+",
                 [=]() {
                     pushEvent(EVT_VIRTUAL_KEY_PLUS);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(this, {LCD_W / 2 + 65, 10, 50, 30}, ">>",
                 [=]() {
                     pushEvent(EVT_VIRTUAL_KEY_FORWARD);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(this, {LCD_W / 2 - 115, 50, 50, 30}, "MIN",
                 [=]() {
                     pushEvent(EVT_VIRTUAL_KEY_MIN);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(this, {LCD_W / 2 + 65, 50, 50, 30}, "MAX",
                 [=]() {
                     pushEvent(EVT_VIRTUAL_KEY_MAX);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);

  new TextButton(this, {LCD_W / 2 - 55, 50, 110, 30}, "DEFAULT",
                 [=]() {
                     pushEvent(EVT_VIRTUAL_KEY_DEFAULT);
                     return 0;
                 }, BUTTON_BACKGROUND | OPAQUE | NO_FOCUS);
}

void DefaultNumberKeyboard::paint(BitmapBuffer * dc)
{
  dc->clear(RGB565(0xE0, 0xE0, 0xE0));
}
