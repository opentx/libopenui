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

class ColorEdit : public FormGroup {
  public:
    ColorEdit(FormGroup * parent, const rect_t & rect, std::function<uint16_t()> getValue, std::function<void(uint16_t)> setValue = nullptr);

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "ColorEdit";
    }
#endif
};

