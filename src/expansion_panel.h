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

class ExpansionPanel;

class ExpansionPanelHeader: virtual public FormGroup
{
  public:
    explicit ExpansionPanelHeader(ExpansionPanel * parent);

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "ExpansionPanelHeader";
    }
#endif

    void onEvent(event_t event) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t, coord_t) override;
#endif

    bool setFocus(uint8_t flag = SET_FOCUS_DEFAULT, Window * from = nullptr) override; // NOLINT(google-default-arguments)
};

class ExpansionPanel: public FormGroup
{
  friend class ExpansionPanelHeader;

  public:
    ExpansionPanel(Window * parent, const rect_t & rect):
      FormGroup(parent, rect, FORM_NO_BORDER | FORM_FORWARD_FOCUS | FORWARD_SCROLL)
    {
    }

    ~ExpansionPanel() override
    {
      if (body && !isOpen()) {
        body->deleteLater();
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

    virtual void open(bool state = true)
    {
      if (_isOpen != state) {
        _isOpen = state;
        body->attach(state ? this : nullptr);
        updateHeight();
        invalidate();
      }
    }

    void setCloseAllowed(bool value = true)
    {
      header->enable(value);
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

    ExpansionPanelHeader * getHeader()
    {
      return header;
    }

    FormGroup * getBody()
    {
      return body;
    }

  protected:
    bool _isOpen = false;
    ExpansionPanelHeader * header = nullptr;
    FormGroup * body = nullptr;
};
