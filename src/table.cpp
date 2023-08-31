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

#include "table.h"
#include "font.h"

void Table::Header::paint(BitmapBuffer * dc)
{
  coord_t x = TABLE_HORIZONTAL_PADDING;
  if (!cells.empty()) {
    dc->clear(TABLE_HEADER_BGCOLOR);
    for (unsigned i = 0; i < cells.size(); i++) {
      auto cell = cells[i];
      if (cell) {
        cell->paint(dc, x, 0, DEFAULT_COLOR, TABLE_HEADER_FONT);
      }
      x += static_cast<Table *>(parent)->columnsWidth[i];
    }
  }
}

void Table::Body::checkEvents()
{
  Window::checkEvents();

  if (_deleted)
    return;

  coord_t y = 0;
  for (auto line: lines) {
    if (y > scrollPositionY - TABLE_LINE_HEIGHT) {
      if (y >= scrollPositionY + height()) {
        break;
      }
      coord_t x = TABLE_HORIZONTAL_PADDING;
      for (unsigned i = 0; i < line->cells.size(); i++) {
        auto cell = line->cells[i];
        auto width = static_cast<Table *>(parent)->columnsWidth[i];
        if (cell && cell->needsInvalidate()) {
          invalidate({x, y - scrollPositionY, width, TABLE_LINE_HEIGHT - TABLE_LINE_BORDER});
        }
        x += width;
      }
    }
    y += TABLE_LINE_HEIGHT;
  }
}

void Table::Body::paint(BitmapBuffer * dc)
{
  coord_t y = 0;
  int lineIndex = 0;
  dc->clear(DEFAULT_BGCOLOR);
  for (auto line: lines) {
    bool highlight = (lineIndex == selection);
    dc->drawSolidFilledRect(0, y, width(), TABLE_LINE_HEIGHT - TABLE_LINE_BORDER, highlight ? MENU_HIGHLIGHT_BGCOLOR : TABLE_BGCOLOR);
    coord_t x = TABLE_HORIZONTAL_PADDING;
    for (unsigned i = 0; i < line->cells.size(); i++) {
      auto cell = line->cells[i];
      if (cell) {
        cell->paint(dc, x, y, highlight ? MENU_HIGHLIGHT_COLOR : line->color, line->font);
      }
      x += static_cast<Table *>(parent)->columnsWidth[i];
    }
    y += TABLE_LINE_HEIGHT;
    lineIndex += 1;
  }
}

#if defined(HARDWARE_TOUCH)
bool Table::Body::onTouchEnd(coord_t x, coord_t y)
{
  unsigned lineIndex = y / TABLE_LINE_HEIGHT;
  if (lineIndex < lines.size()) {
    onKeyPress();
    setFocus(SET_FOCUS_DEFAULT);
    auto onPress = lines[lineIndex]->onPress;
    if (onPress)
      onPress();
  }
  return true;
}
#endif

#if defined(HARDWARE_KEYS)
void Table::Body::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    if (selection >= 0) {
      onKeyPress();
      auto onPress = lines[selection]->onPress;
      if (onPress)
        onPress();
    }
  }
  if (event == EVT_ROTARY_RIGHT) {
    onKeyPress();
    auto table = static_cast<Table *>(parent);
    if (table->getWindowFlags() & FORWARD_SCROLL) {
      auto lineIndex = selection + 1;
      if (lineIndex < int(lines.size())) {
        select(lineIndex, true);
      }
      else {
        auto next = table->getNextField();
        if (next) {
          next->setFocus(SET_FOCUS_FORWARD, this);
          if (!hasFocus()) {
            select(-1, false);
          }
        }
      }
    }
    else {
      if (!lines.empty()) {
        select((selection + 1) % lines.size(), true);
      }
    }
  }
  else if (event == EVT_ROTARY_LEFT) {
    onKeyPress();
    auto table = static_cast<Table *>(parent);
    if (table->getWindowFlags() & FORWARD_SCROLL) {
      auto lineIndex = selection - 1;
      if (lineIndex >= 0) {
        select(lineIndex, true);
      }
      else {
        auto previous = table->getPreviousField();
        if (previous) {
          select(-1, false);
          previous->setFocus(SET_FOCUS_BACKWARD);
        }
      }
    }
    else {
      if (!lines.empty()) {
        select(selection <= 0 ? lines.size() - 1 : selection - 1, true);
      }
    }
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) && selection >= 0) {
    select(-1, true);
    Window::onEvent(event);
  }
  else {
    Window::onEvent(event);
  }
}
#endif
