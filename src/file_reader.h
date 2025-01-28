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

#include <ff.h>

class FileReaderBase
{
  public:
    FileReaderBase(const char * path)
    {
      open(path);
    }

    ~FileReaderBase()
    {
      free(file);
    }

    bool open(const char * path)
    {
      file = (FIL *)malloc(sizeof(FIL));
      if (!file) {
        return false;
      }

      if (f_open(file, path, FA_OPEN_EXISTING | FA_READ) != FR_OK) {
        TRACE("File::load(%s) failed: read error", path);
        free(file);
        file = nullptr;
        return false;
      }

      fileSize = f_size(file);
      return true;
    }

    size_t size() const
    {
      return fileSize;
    }

    size_t read(uint8_t * data, size_t size)
    {
      UINT count;
      return f_read(file, data, size, &count) == FR_OK ? count : 0;
    }

  protected:
    FIL * file = nullptr;
    size_t fileSize = 0;
};

class FileReader: public FileReaderBase
{
  public:
    using FileReaderBase::FileReaderBase;

    ~FileReader()
    {
      free(data);
    }

    const uint8_t * read()
    {
      data = (uint8_t *)malloc(fileSize);
      if (data) {
        auto result = FileReaderBase::read(data, fileSize);
        free(file);
        file = nullptr;
        return result ? data : nullptr;
      }
      else {
        return nullptr;
      }
    }

  protected:
    uint8_t * data = nullptr;
};

class FileBufferedReader: public FileReaderBase
{
  public:
    FileBufferedReader(const char * path, size_t bufferSize = 4096):
      FileReaderBase(path)
    {
      open(path, bufferSize);
    }

    bool open(const char * path, size_t bufferSize = 4096)
    {
      bool result = FileReaderBase::open(path);
      if (result) {
        dataSize = bufferSize;
        data = (uint8_t *)malloc(bufferSize);
        dataRemaining = fileSize;
        ptr = data;
        dataAvailable = 0;
      }
      return result;
    }

    ~FileBufferedReader()
    {
      free(data);
    }

    bool skip(size_t size)
    {
      size_t read;
      FileBufferedReader::read(size, &read);
      return read == size;
    }

    const uint8_t * read(size_t size, size_t * read)
    {
      if (dataAvailable < size) {
        fill();
      }
      auto result = ptr;
      *read = min(size, dataAvailable);
      dataAvailable -= *read;
      ptr += *read;
      return result;
    }

    void fill()
    {
      if (dataRemaining > 0 && dataAvailable < dataSize / 2) {
        if (dataAvailable) {
          auto rest = dataAvailable & 0x03;
          auto newdata = rest ? data + 4 - rest : data;
          memmove(newdata, ptr, dataAvailable);
          ptr = newdata;
        }
        else {
          ptr = data;
        }
        auto dataNeeded = (data + dataSize - ptr - dataAvailable) & 0xFFFFFFFC;
        auto result = FileReaderBase::read(ptr + dataAvailable, dataNeeded);
        dataAvailable += result;
        if (result < dataNeeded)
          dataRemaining = 0;
        else
          dataRemaining -= result;
      }
    }

  protected:
    uint8_t * data = nullptr;
    size_t dataSize = 0;
    uint8_t * ptr = nullptr;
    size_t dataAvailable = 0;
    size_t dataRemaining = 0;
};
