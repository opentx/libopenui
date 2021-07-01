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

#include "keyboard_base.h"

class NumberKeyboard: public Keyboard
{
  public:
    NumberKeyboard();

    ~NumberKeyboard() override;

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "NumberKeyboard";
    }
#endif

    static void setInstance(NumberKeyboard * keyboard)
    {
      _instance = keyboard;
    }

    static void show(FormField * field)
    {
      if (!_instance)
        _instance = new NumberKeyboard();
      _instance->setField(field);
    }

    void paint(BitmapBuffer * dc) override;

  protected:
    static NumberKeyboard * _instance;
};
