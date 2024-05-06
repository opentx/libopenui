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

#include "libopenui_compat.h" // for strcasecmp / VC++
#include <string>
#include <cstring>
#include <cinttypes>
#include "ff.h"

constexpr uint8_t LEN_FILE_EXTENSION_MAX = 5;  // longest used, including the dot, excluding null term.

template <typename T>
T * getFileExtension(T * filename, uint8_t size = 0, uint8_t extMaxLen = 0, uint8_t * filenameLen = nullptr, uint8_t * extLen = nullptr)
{
  int len = (size == 0 ? strlen(filename) : size);
  if (!extMaxLen) {
    extMaxLen = LEN_FILE_EXTENSION_MAX;
  }
  if (filenameLen != nullptr) {
    *filenameLen = (uint8_t)len;
  }
  for (int i = len - 1; i >= 0 && len - i <= extMaxLen; --i) {
    if (filename[i] == '.') {
      if (extLen) {
        *extLen = len - i;
      }
      return &filename[i];
    }
  }
  if (extLen != nullptr) {
    *extLen = 0;
  }
  return nullptr;
}

bool isExtensionMatching(const char * extension, const char * pattern, char * match = nullptr);
FRESULT sdReadDir(DIR * dir, FILINFO * fno, bool & firstTime);

// comparison, not case sensitive.
inline bool compare_nocase(const std::string & first, const std::string & second)
{
  return strcasecmp(first.c_str(), second.c_str()) < 0;
}
