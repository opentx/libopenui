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
uint16_t* lcdGetScratchBuffer();
void DMACopyBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h);
void DMACopyAlphaBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h);
void onKeyPress();
void onKeyError();
void killEvents(event_t event);
event_t getWindowEvent();

//
// These colors MUST be defined
//

// #define DEFAULT_COLOR                  
// #define DEFAULT_BGCOLOR                
// #define FOCUS_COLOR                    
// #define FOCUS_BGCOLOR                  
// #define DISABLE_COLOR                  
// #define HIGHLIGHT_COLOR                
// #define CHECKBOX_COLOR                 
// #define SCROLLBAR_COLOR                
// #define MENU_COLOR                     
// #define MENU_BGCOLOR                   
// #define MENU_TITLE_BGCOLOR             
// #define MENU_LINE_COLOR                
// #define MENU_HIGHLIGHT_COLOR           
// #define MENU_HIGHLIGHT_BGCOLOR         
// #define OVERLAY_COLOR                  
// #define TABLE_BGCOLOR                  
// #define TABLE_HEADER_BGCOLOR           
// #define CUSTOM_COLOR                   
