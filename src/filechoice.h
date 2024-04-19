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

#include <string>
#include "choice.h"

namespace ui {

class FileChoice: public ChoiceBase
{
  public:
    FileChoice(FormGroup * parent, const rect_t & rect, std::string folder, const char * extension, int maxlen, std::function<std::string()> getValue, std::function<void(std::string)> setValue);

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "FileChoice";
    }
#endif

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

  protected:
    std::string folder;
    const char * extension;
    int maxlen;
    std::function<std::string()> getValue;
    std::function<void(std::string)> setValue;

    bool openMenu();
};

}
