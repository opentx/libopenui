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

#include <inttypes.h>
#include "libopenui_config.h"

void lcdNextLayer();
void DMACopyBitmap(uint16_t * dest, int destw, int desth, int x, int y, const uint16_t * src, int srcw, int srch, int srcx, int srcy, int w, int h);
void DMACopyAlphaBitmap(uint16_t * dest, bool destAlpha, int destw, int desth, int x, int y, const uint16_t * src, bool srcAlpha, int srcw, int srch, int srcx, int srcy, int w, int h);
void DMACopyAlphaMask(uint16_t * dest, bool destAlpha, int destw, int desth, int x, int y, const uint8_t * src, int srcw, int srch, int srcx, int srcy, int w, int h, uint16_t color);
void onKeyPress();
void onKeyError();
void killEvents(event_t event);
event_t getWindowEvent();

