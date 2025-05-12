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

#include <ff.h>
#include <cinttypes>
#include "bitmapbuffer.h"
#include "font.h"
#include "debug.h"

/* Font format
 * 'F', 'N', 'T', '1'
 * begin: 4bytes (glyphs index start)
 * end: 4bytes (glyphs index end, not included)
 * specs: 2bytes * (count + 1)
 * data: bitmap data in RLE format
 */
bool Font::loadFile(const char * path)
{
  TRACE("Font::loadFile('%s')", path);

  auto file = (FIL *)malloc(sizeof(FIL));
  if (!file) {
    return false;
  }

  FRESULT result = f_open(file, path, FA_READ);
  if (result != FR_OK) {
    free(file);
    return false;
  }

  struct {
    char fmt[4];
    char name[LEN_FONT_NAME + 1];
    uint8_t rangesCount;
    uint8_t spacing;
    uint8_t spaceWidth;
  } header;
  UINT read;
  result = f_read(file, (uint8_t *)&header, sizeof(header), &read);
  if (result != FR_OK || read != sizeof(header) || strncmp(header.fmt, "FNT1", sizeof(header.fmt)) != 0) {
    TRACE("loadFont('%s'): invalid header", path);
    f_close(file);
    free(file);
    return false;
  }

  strlcpy(this->name, header.name, sizeof(this->name));
  this->spacing = header.spacing;
  this->spaceWidth = header.spaceWidth;

  for (auto i = 0; i < header.rangesCount; i++) {
    struct {
      uint32_t begin;
      uint32_t end;
      uint32_t dataSize;
    } rangeHeader;

    result = f_read(file, (uint8_t *)&rangeHeader, sizeof(rangeHeader), &read);
    if (result != FR_OK || read != sizeof(rangeHeader) || rangeHeader.begin >= rangeHeader.end) {
      TRACE("loadFont('%s'): invalid range header", path);
      f_close(file);
      free(file);
      return false;
    }

    uint32_t specsSize = sizeof(uint16_t) * (rangeHeader.end - rangeHeader.begin + 1);
    auto specs = (uint16_t *)malloc(specsSize);
    if (!specs) {
      f_close(file);
      free(file);
      return false;
    }

    result = f_read(file, (uint8_t *)specs, specsSize, &read);
    if (result != FR_OK || read != specsSize) {
      free(specs);
      f_close(file);
      free(file);
      return false;
    }

    auto data = (uint8_t *)malloc(rangeHeader.dataSize);
    if (!data) {
      free(specs);
      f_close(file);
      free(file);
      return false;
    }

    result = f_read(file, (uint8_t *)data, rangeHeader.dataSize, &read);
    if (result != FR_OK || read != rangeHeader.dataSize) {
      free(specs);
      free(data);
      f_close(file);
      free(file);
      return false;
    }

    auto mask = Mask::decodeRle(data);
    free(data);
    if (mask) {
      ranges.push_back({rangeHeader.begin, rangeHeader.end, mask, specs});
    }
  }

  // TRACE("Ranges...");
  // for (auto & range: ranges) {
  //   TRACE("  %d-%d", range.begin, range.end);
  // }

  f_close(file);
  free(file);
  
  return true;
}
