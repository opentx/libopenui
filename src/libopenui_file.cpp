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

#include <inttypes.h>
#include <string.h>
#include "libopenui_file.h"
#include "libopenui_defines.h"

/**
  Check if given extension exists in a list of extensions.
  @param extension The extension to search for, including leading period.
  @param pattern One or more file extensions concatenated together, including the periods.
    The list is searched backwards and the first match, if any, is returned.
    eg: ".gif.jpg.jpeg.png"
  @param match Optional container to hold the matched file extension (wide enough to hold LEN_FILE_EXTENSION_MAX + 1).
  @retval true if a extension was found in the lost, false otherwise.
*/
bool isExtensionMatching(const char * extension, const char * pattern, char * match)
{
  const char * ext;
  uint8_t extLen, filenameLen;
  int plen;

  ext = getFileExtension(pattern, 0, 0, &filenameLen, &extLen);
  plen = (int)filenameLen;
  while (plen > 0 && ext) {
    if (!strncasecmp(extension, ext, extLen)) {
      if (match != nullptr) strncat(&(match[0]='\0'), ext, extLen);
      return true;
    }
    plen -= extLen;
    if (plen > 0) {
      ext = getFileExtension(pattern, plen, 0, nullptr, &extLen);
    }
  }
  return false;
}

// returns true if current working dir is at the root level
bool isCwdAtRoot()
{
  char path[10];
  if (f_getcwd(path, sizeof(path)-1) == FR_OK) {
    return (strcasecmp("/", path) == 0);
  }
  return false;
}

/*
  Wrapper around the f_readdir() function which
  also returns ".." entry for sub-dirs. (FatFS 0.12 does
  not return ".", ".." dirs anymore)
*/
FRESULT sdReadDir(DIR * dir, FILINFO * fno, bool & firstTime)
{
  FRESULT res;
  if (firstTime && !isCwdAtRoot()) {
    // fake parent directory entry
    strcpy(fno->fname, "..");
    fno->fattrib = AM_DIR;
    res = FR_OK;
  }
  else {
    res = f_readdir(dir, fno);                   /* Read a directory item */
  }
  firstTime = false;
  return res;
}
