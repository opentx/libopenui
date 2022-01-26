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

#include <utility>
#include "layer.h"
#include "bitmapbuffer.h"

class MainWindow: public Window
{
  protected:
    // singleton
    MainWindow():
      Window(nullptr, {0, 0, LCD_W, LCD_H}),
      invalidatedRect(rect)
    {
      Layer::push(this);
    }

  public:
    ~MainWindow() override
    {
      Layer::pop(this);
    }

    static MainWindow * instance()
    {
      if (!_instance)
        _instance = new MainWindow();

      return _instance;
    }

#if defined(SIMULATION)
    static void create()
    {
      _instance = new MainWindow();
    }

    static void destroy()
    {
      if (_instance) {
        _instance->deleteLater();
        _instance = nullptr;
        emptyTrash();
      }
    }
#endif

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "MainWindow";
    }
#endif

    void checkEvents() override;

    void invalidate()
    {
      invalidate({0, 0, rect.w, rect.h});
    }

    void invalidate(const rect_t & rect) override;

    bool needsRefresh() const
    {
      return invalidatedRect.w > 0;
    }

    bool refresh();

    void run(bool trash=true);

  protected:
    static MainWindow * _instance;
    static void emptyTrash();
    rect_t invalidatedRect;
    const char * shutdown = nullptr;
};
