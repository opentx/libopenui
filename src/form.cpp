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

#include "form.h"
#include "bitmapbuffer.h"
#include "libopenui_config.h"

FormField::FormField(Window * parent, const rect_t & rect, WindowFlags windowFlags, LcdFlags textFlags) :
  Window(parent, rect, windowFlags, textFlags)
{
  if (!(windowFlags & NO_FOCUS)) {
    auto * form = dynamic_cast<FormGroup *>(parent);
    if (form) {
      form->addField(this, windowFlags & PUSH_FRONT);
    }
  }
}

#if defined(HARDWARE_KEYS)
void FormField::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("FormField").c_str(), event);

  if (event == EVT_ROTARY_RIGHT/*EVT_KEY_BREAK(KEY_DOWN)*/) {
    if (next && next->setFocus(SET_FOCUS_FORWARD, this)) {
      onKeyPress();
    }
    else {
      Window::onEvent(event);
    }
  }
  else if (event == EVT_ROTARY_LEFT/*EVT_KEY_BREAK(KEY_UP)*/) {
    if (previous && previous->setFocus(SET_FOCUS_BACKWARD, this)) {
      onKeyPress();
    }
    else {
      Window::onEvent(event);
    }
  }
  else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    onKeyPress();
    setEditMode(!editMode);
    invalidate();
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) && editMode) {
    onKeyPress();
    setEditMode(false);
    invalidate();
  }
  else {
    Window::onEvent(event);
  }
}
#endif

bool FormField::setFocus(uint8_t flag, Window * from)
{
  TRACE_WINDOWS("%s setFocus(%d)", getWindowDebugString("FormField").c_str(), flag);

  if (enabled) {
    return Window::setFocus(flag, from);
  }
  else {
    if (flag == SET_FOCUS_BACKWARD) {
      return previous ? previous->setFocus(flag, this) : false;
    }
    else {
      return next ? next->setFocus(flag, this) : false;
    }
  }
}

void FormField::paint(BitmapBuffer * dc)
{
  if (editMode) {
    dc->drawPlainFilledRectangle(0, 0, rect.w, rect.h, FOCUS_BGCOLOR);
  }
  else if (!(windowFlags & FORM_NO_BORDER)) {
    if (hasFocus()) {
      dc->drawPlainRectangle(0, 0, rect.w, rect.h, FOCUS_BGCOLOR, 2);
    }
    else if (!(windowFlags & FORM_BORDER_FOCUS_ONLY)) {
      dc->drawPlainRectangle(0, 0, rect.w, rect.h, DISABLE_COLOR, 1);
    }
  }
}

void FormGroup::addField(FormField * field, bool front)
{
  if (field->getWindowFlags() & FORM_DETACHED)
    return;

  if (front) {
    if (first)
      link(field, first);
    first = field;
    if (!last)
      last = field;
  }
  else {
    if (last)
      link(last, field);
    last = field;
    if (!first)
      first = field;
  }

  if (WRAP_FORM_FIELDS_WITHIN_PAGE) {
    if (previous && (windowFlags & FORM_FORWARD_FOCUS)) {
      last->setNextField(this);
      link(previous, first);
    }
    else {
      link(last, first);
    }
  }
  else {
    if (windowFlags & FORM_FORWARD_FOCUS) {
      last->setNextField(this);
      if (previous)
        link(previous, first);
    }
  }

  if (!focusWindow && !(field->getWindowFlags() & FORM_FORWARD_FOCUS)) {
    field->setFocus(SET_FOCUS_DEFAULT);
  }
  else if (focusWindow == this && (windowFlags & FORM_FORWARD_FOCUS)) {
    field->setFocus(SET_FOCUS_DEFAULT);
  }
}

void FormGroup::removeField(FormField * field)
{
  FormField * prev = field->getPreviousField();
  FormField * next = field->getNextField();

  if (prev) {
    prev->setNextField(next);
  }
  if (next) {
    next->setPreviousField(prev);
  }

  if (first == field) {
    if (prev && (prev != field))
      first = prev;
    else if (next && (next != field))
      first = next;
    else
      first = nullptr;
  }

  if (last == field) {
    if (next && (next != field))
      last = next;
    else if (prev && (prev != field))
      last = prev;
    else
      last = nullptr;
  }
}

bool FormGroup::setFocus(uint8_t flag, Window * from)
{
  TRACE_WINDOWS("%s setFocus(%d)", getWindowDebugString("FormGroup").c_str(), flag);

  if (windowFlags & FORM_FORWARD_FOCUS) {
    switch (flag) {
      case SET_FOCUS_BACKWARD:
        if (from && from->isChild(first)) {
          if (previous == this) {
            return last->setFocus(SET_FOCUS_BACKWARD, this);
          }
          else if (previous) {
            return previous->setFocus(SET_FOCUS_BACKWARD, this);
          }
          else {
            return false;
          }
        }
        else {
          if (last) {
            return last->setFocus(SET_FOCUS_BACKWARD, this);
          }
          else if (previous) {
            return previous->setFocus(SET_FOCUS_BACKWARD, this);
          }
          else {
            return false;
          }
        }
        break;

      case SET_FOCUS_FORWARD:
        if (from && from->isChild(this)) {
          if (next == this) {
            return first->setFocus(SET_FOCUS_FORWARD, this);
          }
          else if (next) {
            return next->setFocus(SET_FOCUS_FORWARD, this);
          }
          else {
            return false;
          }
        }
        else {
          if (first) {
            return first->setFocus(SET_FOCUS_FORWARD, this);
          }
          else if (next) {
            return next->setFocus(SET_FOCUS_FORWARD, this);
          }
          else {
            return false;
          }
        }
        break;

      default:
        if (from == previous) {
          if (first) {
            return first->setFocus(SET_FOCUS_DEFAULT);
          }
          else {
            clearFocus();
            focusWindow = this;
            return true;
          }
        }
        else if (next) {
          return next->setFocus(SET_FOCUS_FORWARD);
        }
        else {
          clearFocus();
          focusWindow = this;
          return true;
        }
        break;
    }
  }
  else if (!(windowFlags & NO_FOCUS)) {
    return FormField::setFocus(flag, from);
  }
  else {
    return false;
  }
}

bool FormGroup::setFocusOnFirstVisibleField() const
{
  auto field = getFirstField();
  while (field && !field->isInsideParentScrollingArea()) {
    field = field->getNextField();
  }
  return field ? field->setFocus(SET_FOCUS_FORWARD) : false;
}

bool FormGroup::setFocusOnLastVisibleField() const
{
  auto field = getLastField();
  while (field && !field->isInsideParentScrollingArea()) {
    field = field->getPreviousField();
  }
  return field ? field->setFocus(SET_FOCUS_BACKWARD) : false;
}

#if defined(HARDWARE_KEYS)
void FormGroup::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("FormGroup").c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    onKeyPress();
    setFocusOnFirstVisibleField();
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) && !hasFocus() && !(windowFlags & FORM_FORWARD_FOCUS)) {
    onKeyPress();
    setFocus(SET_FOCUS_DEFAULT); // opentx - model - timers settings
  }
  else if (event == EVT_ROTARY_RIGHT && !next) {
    onKeyPress();
    if (hasFocus()) {
      if (!setFocusOnFirstVisibleField()) {
        if (!setFocus(SET_FOCUS_FORWARD)) {
          setScrollPositionY(scrollPositionY + height() / 2);
        }
      }
    }
    else if (innerHeight > height() && scrollPositionY < innerHeight - height() / 2) {
      setScrollPositionY(scrollPositionY + height() / 2);
    }
    else {
      FormField::onEvent(event);
    }
  }
  else if (event == EVT_ROTARY_LEFT && !previous) {
    onKeyPress();
    if (hasFocus()) {
      if (!setFocusOnLastVisibleField()) {
        if (!setFocus(SET_FOCUS_BACKWARD)) {
          setScrollPositionY(scrollPositionY - height() / 2);
        }
      }
    }
    else if (scrollPositionY > 0) {
      setScrollPositionY(scrollPositionY - height() / 2);
    }
    else {
      FormField::onEvent(event);
    }
  }
  else {
    FormField::onEvent(event);
  }
}
#endif

void FormGroup::paint(BitmapBuffer * dc)
{
  if (!(windowFlags & (FORM_NO_BORDER | FORM_FORWARD_FOCUS))) {
    if (!editMode && hasFocus()) {
      dc->drawPlainRectangle(0, 0, rect.w, rect.h, FOCUS_BGCOLOR, 2);
    }
    else if (!(windowFlags & FORM_BORDER_FOCUS_ONLY)) {
      dc->drawPlainRectangle(0, 0, rect.w, rect.h, DISABLE_COLOR, 1);
    }
  }
}

#if defined(HARDWARE_KEYS)
void FormWindow::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X next=%p previous=%p", getWindowDebugString("FormWindow").c_str(), event, next, previous);

  if (event == EVT_KEY_BREAK(KEY_EXIT) && (windowFlags & FORM_FORWARD_FOCUS) && first) {
    onKeyPress();
    Window * currentFocus = getFocus();
    first->setFocus(SET_FOCUS_FORWARD);
    if (getFocus() != currentFocus) {
      return;
    }
  }

  FormGroup::onEvent(event);
}
#endif
