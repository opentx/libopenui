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


#include "modal_window.h"
#include "font.h"
#include "layer.h"

using namespace ui;

ModalWindow::ModalWindow(Window * parent, bool closeWhenClickOutside):
  Window(parent->getFullScreenWindow(), {0, 0, LCD_W, LCD_H}),
  closeWhenClickOutside(closeWhenClickOutside)
{
  Layer::push(this);
}

void ModalWindow::deleteLater(bool detach, bool trash) // NOLINT(google-default-arguments)
{
  if (deleted())
    return;

  Layer::pop(this);

  Window::deleteLater(detach, trash);
}

void ModalWindow::paint(BitmapBuffer * dc)
{
  dc->drawFilledRectangle(0, 0, width(), height(), OVERLAY_COLOR, SOLID);
}

void ModalWindowContent::paint(BitmapBuffer * dc)
{
  dc->drawPlainFilledRectangle(0, 0, width(), POPUP_HEADER_HEIGHT, FOCUS_BGCOLOR);
  dc->drawText(FIELD_PADDING_LEFT, (POPUP_HEADER_HEIGHT - getFontHeight(FONT(STD))) / 2, title.c_str(), FOCUS_COLOR);
  dc->drawPlainFilledRectangle(0, POPUP_HEADER_HEIGHT, width(), height() - POPUP_HEADER_HEIGHT, DEFAULT_BGCOLOR);
}
