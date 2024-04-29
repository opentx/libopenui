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

#include "expansion_panel.h"

using namespace ui;

void ExpansionPanel::updateHeight(bool move)
{
  coord_t newHeight = (isOpen() ? header->height() + body->height() : header->height());
  if (move) {
    parent->moveWindowsTop(bottom(), newHeight - height());
  }
  setHeight(newHeight);
}

bool ExpansionPanel::setFocus(uint8_t flag, Window * from) // NOLINT(google-default-arguments)
{
  if (!enabled) {
    if (flag == SET_FOCUS_BACKWARD) {
      auto previous = getPreviousField();
      return previous ? previous->setFocus(SET_FOCUS_BACKWARD, this) : false;
    }
    else {
      auto next = getNextField();
      return next ? next->setFocus(SET_FOCUS_FORWARD, this) : false;
    }
  }
  else if (isOpen()) {
    return FormGroup::setFocus(flag, from);
  }
  else {
    return header->setFocus(flag, from);
  }
}

ExpansionPanelHeader::ExpansionPanelHeader(ExpansionPanel * parent):
  FormGroup(parent, {0, 0, parent->width(), parent->height()}, FORWARD_SCROLL)
{
}

void ExpansionPanelHeader::onEvent(event_t event)
{
  auto panel = static_cast<ExpansionPanel *>(parent);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    panel->toggle();
  }
  else if (event == EVT_ROTARY_RIGHT && !panel->isOpen()) {
    auto next = panel->getNextField();
    if (next)
      next->setFocus(SET_FOCUS_FORWARD, this);
  }
  else if (event == EVT_ROTARY_LEFT) {
    auto previous = panel->getPreviousField();
    if (previous) {
      previous->setFocus(SET_FOCUS_BACKWARD, this);
    }
  }
  else {
    FormGroup::onEvent(event);
  }
}

bool ExpansionPanelHeader::setFocus(uint8_t flag, Window * from) // NOLINT(google-default-arguments)
{
  auto panel = static_cast<ExpansionPanel *>(parent);

  if (enabled || panel->isOpen()) {
    return FormGroup::setFocus(flag, from);
  }
  else {
    if (flag == SET_FOCUS_BACKWARD) {
      auto previous = panel->getPreviousField();
      return previous ? previous->setFocus(SET_FOCUS_BACKWARD, this) : false;
    }
    else {
      auto next = panel->getNextField();
      return next ? next->setFocus(SET_FOCUS_FORWARD, this) : false;
    }
  }
}

#if defined(HARDWARE_TOUCH)
bool ExpansionPanelHeader::onTouchEnd(coord_t, coord_t)
{
  if (enabled) {
    static_cast<ExpansionPanel *>(parent)->toggle();
    setFocus(SET_FOCUS_DEFAULT);
  }
  return true;
}
#endif
