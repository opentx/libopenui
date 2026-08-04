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

#pragma once

#include "form.h"

namespace ui {

class ExpansionPanel;

template<class T = FormGroup>
class ExpansionPanelHeader: public T
{
  public:
    using T::T;

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "ExpansionPanelHeader";
    }
#endif

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t, coord_t) override;
#endif

    bool setFocus(uint8_t flag = SET_FOCUS_DEFAULT, Window * from = nullptr) override; // NOLINT(google-default-arguments)
};

class ExpansionPanel: public FormGroup
{
  // friend class ExpansionPanelHeader;

  public:
    ExpansionPanel(Window * parent, const rect_t & rect):
      FormGroup(parent, rect, FORM_NO_BORDER | FORM_FORWARD_FOCUS | FORWARD_SCROLL)
    {
    }

    void deleteLater(bool detach = true, bool trash = true) override
    {
      FormGroup::deleteLater(detach, trash);
      if (body && !isOpen()) {
        body->deleteLater(detach, trash);
      }
    }

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "ExpansionPanel";
    }
#endif

    void toggle()
    {
      open(!_isOpen);
    }

    bool isOpen() const
    {
      return _isOpen;
    }

    virtual void open(bool state = true, bool handler = true)
    {
      if (_isOpen != state) {
        _isOpen = state;
        body->attach(state ? this : nullptr);
        updateHeight();
        invalidate();
        if (handler && openHandler) {
          openHandler(state);
        }
      }
    }

    void setCloseAllowed(bool value = true)
    {
      header->enable(value);
    }

    void setOpenHandler(std::function<void(bool)> handler)
    {
      openHandler = std::move(handler);
    }

    void enable(bool value = true)
    {
      if (!value && isOpen()) {
        open(false);
      }
      FormGroup::enable(value);
      header->enable(value);
    }

    void disable()
    {
      enable(false);
    }

    void updateHeight(bool move = true);

    bool setFocus(uint8_t flag = SET_FOCUS_DEFAULT, Window * from = nullptr) override; // NOLINT(google-default-arguments)

    FormGroup * getHeader()
    {
      return header;
    }

    FormGroup * getBody()
    {
      return body;
    }

  protected:
    bool _isOpen = false;
    FormGroup * header = nullptr;
    FormGroup * body = nullptr;
    std::function<void(bool)> openHandler;
};


template<class T>
bool ExpansionPanelHeader<T>::setFocus(uint8_t flag, Window * from) // NOLINT(google-default-arguments)
{
  auto panel = static_cast<ExpansionPanel *>(T::parent);

  if (T::enabled || panel->isOpen()) {
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

#if defined(HARDWARE_KEYS)
template<class T>
void ExpansionPanelHeader<T>::onEvent(event_t event)
{
  auto panel = static_cast<ExpansionPanel *>(T::parent);

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
#endif

#if defined(HARDWARE_TOUCH)
template<class T>
bool ExpansionPanelHeader<T>::onTouchEnd(coord_t, coord_t)
{
  if (T::enabled) {
    static_cast<ExpansionPanel *>(T::parent)->toggle();
    setFocus(SET_FOCUS_DEFAULT);
  }
  return true;
}
#endif

}
