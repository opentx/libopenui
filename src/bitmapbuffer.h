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

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <type_traits>
#include "libopenui_types.h"
#include "libopenui_defines.h"
#include "libopenui_depends.h"
#include "libopenui_helpers.h"
#include "debug.h"

class FontGlyph;

constexpr uint8_t SOLID = 0xFF;
constexpr uint8_t DOTTED  = 0x55;
constexpr uint8_t DASHED = 0x33;

#define MOVE_OFFSET() coord_t offsetX = this->offsetX; x += offsetX; this->offsetX = 0; coord_t offsetY = this->offsetY; y += offsetY; this->offsetY = 0
#define APPLY_OFFSET() x += this->offsetX; y += this->offsetY
#define RESTORE_OFFSET()  this->offsetX = offsetX, this->offsetY = offsetY

extern char * numberToString(int32_t val, uint8_t len, const char * prefix, const char * suffix, LcdFlags flags);

typedef uint16_t pixel_t;

enum BitmapFormat : uint8_t
{
  BMP_RGB565,
  BMP_ARGB4444
};

template<class T>
class Raster
{
  public:
    using DataType = T;

    // Raster, Mask and Bitmap are literal types (constexpr constructors, trivial destructor): the embedded bitmaps
    // are constexpr objects, in flash instead of RAM

    constexpr Raster(const Raster & other) = default;

    constexpr Raster(uint16_t width, uint16_t height, const T * data):
      _width(width),
      _height(height),
      data(const_cast<T *>(data)),
      dataEnd(const_cast<T *>(data) + (width * height))
    {
    }

    // the allocated objects (allocate()) and their pixels are in the same block, freed by delete
    static void * operator new(size_t size)
    {
      return malloc(size);
    }

    static void * operator new(size_t, void * where)
    {
      return where;
    }

    static void operator delete(void * ptr)
    {
      free(ptr);
    }

    [[nodiscard]] inline uint16_t width() const
    {
      return _width;
    }

    [[nodiscard]] inline uint16_t height() const
    {
      return _height;
    }

    [[nodiscard]] float getScale(coord_t w, coord_t h) const
    {
      auto vscale = float(h) / height();
      auto hscale = float(w) / width();
      return vscale < hscale ? vscale : hscale;
    }

    [[nodiscard]] inline T * getData() const
    {
      return data;
    }

    [[nodiscard]] inline T * getDataEnd() const
    {
      return dataEnd;
    }

    [[nodiscard]] inline bool isValid() const
    {
      return data != nullptr;
    }

    [[nodiscard]] uint32_t getDataSize() const
    {
      return (const uint8_t *)dataEnd - (const uint8_t *)data;
    }

    [[nodiscard]] inline const T * getPixelPtrAbs(coord_t x, coord_t y) const
    {
#if LCD_ORIENTATION == 180
      x = _width - x - 1;
      y = _height - y - 1;
#elif LCD_ORIENTATION == 270
  #if defined(LTDC_OFFSET_X)
      if (isLcdFrameBuffer(data))
        return &data[x * (_height + LTDC_OFFSET_X) + y + LTDC_OFFSET_X];
      else
        return &data[x * _height + y];
  #else
      return &data[x * _height + y];
  #endif
#endif
      return &data[y * _width + x];
    }

    inline T * getPixelPtrAbs(coord_t x, coord_t y)
    {
#if LCD_ORIENTATION == 180
      x = _width - x - 1;
      y = _height - y - 1;
#elif LCD_ORIENTATION == 270
  #if defined(LTDC_OFFSET_X)
      if (isLcdFrameBuffer(data))
        return &data[x * (_height + LTDC_OFFSET_X) + y + LTDC_OFFSET_X];
      else
        return &data[x * _height + y];
  #else
      return &data[x * _height + y];
  #endif
#endif
      return &data[y * _width + x];
    }

    inline const T * getNextPixel(const T * pixel, coord_t count = 1) const
    {
#if LCD_ORIENTATION == 180
      return pixel - count;
#elif LCD_ORIENTATION == 270
      coord_t lineHeight = _height;
  #if defined(LTDC_OFFSET_X)
      if (isLcdFrameBuffer(data)) {
        lineHeight += LTDC_OFFSET_X;
      }
  #endif
      auto result = pixel + (count * lineHeight);
      while (result > dataEnd)
        result -= lineHeight - 1;
      return result;
#else
      return pixel + count;
#endif
    }

    inline T * getNextPixel(T * pixel, coord_t count = 1)
    {
#if LCD_ORIENTATION == 180
      return pixel - count;
#elif LCD_ORIENTATION == 270
      coord_t lineHeight = _height;
  #if defined(LTDC_OFFSET_X)
      if (isLcdFrameBuffer(data)) {
        lineHeight += LTDC_OFFSET_X;
      }
  #endif
      auto result = pixel + (count * lineHeight);
      while (result > dataEnd)
        result -= lineHeight - 1;
      return result;
#else
      return pixel + count;
#endif
    }

  protected:
    uint16_t _width;
    uint16_t _height;
    T * data;
    T * dataEnd;

    // allocates the object (class C) and its pixels in one block, C(args..., width, height, pixels)
    template <class C, class... Args>
    static C * allocateWithPixels(uint16_t width, uint16_t height, Args... args)
    {
      auto offset = align32(sizeof(C));
      auto block = (uint8_t *)malloc(offset + align32(width * height * sizeof(T)));
      if (!block) {
        return nullptr;
      }
      return new (block) C(args..., width, height, (T *)(block + offset));
    }
};

class Mask: public Raster<uint8_t>
{
  public:
    constexpr Mask(uint16_t width, uint16_t height, const uint8_t * data):
      Raster(width, height, data)
    {
    }

    // data with the header (width and height), not constexpr
    Mask(const uint8_t * data):
      Raster(*((uint16_t *)(data + 0)), *((uint16_t *)(data + 2)), (uint8_t *)(data + 4))
    {
    }

    static Mask * allocate(uint16_t width, uint16_t height)
    {
      return allocateWithPixels<Mask>(width, height);
    }

    static Mask * load(const char * path, int maxSize = -1);

    static Mask * decodeRle(const uint8_t * data);

    void clear()
    {
      memset(data, 0, getDataSize());
    }

    void drawFilledRectangle(coord_t x, coord_t y, coord_t w, coord_t h, uint8_t mask)
    {
      for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
          *getPixelPtrAbs(x + j, y + i) = mask;
        }
      }
    }
};

class Bitmap: public Raster<pixel_t>
{
  public:
    constexpr Bitmap(BitmapFormat format, uint16_t width, uint16_t height, const pixel_t * data):
      Raster(width, height, data),
      _format(format)
    {
    }

    // data with the header (format, width and height), not constexpr
    Bitmap(const uint8_t * data):
      Bitmap((BitmapFormat)*((uint32_t *)data), *((uint16_t *)(data + 4)), *((uint16_t *)(data + 6)), (pixel_t *)(data + 8))
    {
    }

    static Bitmap * allocate(BitmapFormat format, uint16_t width, uint16_t height)
    {
      return allocateWithPixels<Bitmap>(width, height, format);
    }

    static Bitmap * load(const char * path, int maxSize = -1);

    [[nodiscard]] BitmapFormat getFormat() const
    {
      return _format;
    }

    void setFormat(BitmapFormat format)
    {
      _format = format;
    }

  protected:
    BitmapFormat _format;

    static Bitmap * load_stb(const char * filename, int maxSize = -1);
    static Bitmap * load_bmp(const char * filename, int maxSize = -1);
#if defined(LIBOPENUI_EXTERNAL_LOAD_JPG)
    static Bitmap * load_jpg(const char * filename, int maxSize = -1);
#endif
};

// Allocates a new object of the same kind as 'source' (same pixel format for the bitmaps) with another size
template <class C>
C * allocateLike(const C * source, uint16_t width, uint16_t height)
{
  if constexpr (std::is_same_v<C, Mask>) {
    UNUSED(source);
    return C::allocate(width, height);
  }
  else {
    return C::allocate(source->getFormat(), width, height);
  }
}

// Bilinear interpolation of 4 pixels: p00 (x0, y0), p10 (x1, y0), p01 (x0, y1), p11 (x1, y1), fx and fy in [0, 256]
// are the distances from (x0, y0). Each channel is interpolated separately, in the format of the source.
template <class C>
typename C::DataType bilinear(const C * source, typename C::DataType p00, typename C::DataType p10, typename C::DataType p01, typename C::DataType p11, int fx, int fy)
{
  const int w00 = (256 - fx) * (256 - fy);
  const int w10 = fx * (256 - fy);
  const int w01 = (256 - fx) * fy;
  const int w11 = fx * fy;   // the sum of the weights is 65536
  auto mix = [&](int c00, int c10, int c01, int c11) -> int {
    return (c00 * w00 + c10 * w10 + c01 * w01 + c11 * w11 + 32768) >> 16;
  };

  if constexpr (std::is_same_v<C, Mask>) {
    UNUSED(source);
    return mix(p00, p10, p01, p11);
  }
  else if (source->getFormat() == BMP_ARGB4444) {
    auto channel = [&](int shift) {
      return mix((p00 >> shift) & 0xF, (p10 >> shift) & 0xF, (p01 >> shift) & 0xF, (p11 >> shift) & 0xF);
    };
    return (channel(12) << 12) | (channel(8) << 8) | (channel(4) << 4) | channel(0);
  }
  else {
    // RGB565
    auto channel = [&](int shift, int mask) {
      return mix((p00 >> shift) & mask, (p10 >> shift) & mask, (p01 >> shift) & mask, (p11 >> shift) & mask);
    };
    return (channel(11, 0x1F) << 11) | (channel(5, 0x3F) << 5) | channel(0, 0x1F);
  }
}

template <class C>
C * invert(const C * source)
{
  auto result = allocateLike(source, source->width(), source->height());
  if (result) {
    auto * srcData = source->getData();
    auto * destData = result->getData();
    for (coord_t y = 0; y < source->height(); y++) {
      for (coord_t x = 0; x < source->width(); x++) {
        destData[x] = 0xFF - srcData[x];
      }
      srcData += source->width();
      destData += source->width();
    }
  }
  return result;
}

template <class C>
C * verticalFlip(const C * source)
{
  auto * result = allocateLike(source, source->width(), source->height());
  if (!result) {
    return nullptr;
  }
  
#if LCD_ORIENTATION == 270
  auto sourceData = source->getData();
  auto sourceWidth = source->width();
  auto sourceHeight = source->height();
  auto resultData = result->getData();
  for (coord_t y = 0; y < sourceHeight; y++) {
    for (coord_t x = 0; x < sourceWidth; x++) {
      auto * p = &sourceData[x * sourceHeight + sourceHeight - 1 - y];
      auto * q = &resultData[x * sourceHeight + y];
      *q = *p;
    }
  }
#else
  auto * srcData = source->getData() + source->width() * (source->height() - 1);
  auto * destData = result->getData();
  for (coord_t y = 0; y < source->height(); y++) {
    for (coord_t x = 0; x < source->width(); x++) {
      *(destData++) = *(srcData++);
    }
    srcData -= 2 * source->width();
  }
#endif
  return result;
}

// Rotation around the center, the corners left empty are transparent / black.
// antialias: bilinear interpolation of the pixels instead of the nearest one
template <class C>
C * rotate(const C * source, float radians, bool antialias = true)
{
  float sine = sinf(radians);
  float cosine = cosf(radians);

  auto w = source->width();
  auto h = source->height();

  auto x0 = w / 2;
  auto y0 = h / 2;

  auto * result = allocateLike(source, w, h);
  if (!result) {
    return nullptr;
  }

  auto * srcData = source->getData();
  auto * destData = result->getData();

  for (unsigned rX = 0; rX < w; rX++) {
    coord_t dX = rX - x0;
    for (unsigned rY = 0; rY < h; rY++) {
      coord_t dY = rY - y0;
#if LCD_ORIENTATION == 270
      // the bitmaps are stored column by column: pixel (x, y) is at data[x * h + y]
      auto * dest = &destData[rX * h + rY];
#else
      auto * dest = &destData[rY * w + rX];
#endif
      if (antialias) {
        float fSrcX = dX * cosine + dY * sine + x0;
        float fSrcY = dY * cosine - dX * sine + y0;
        // same area as the nearest pixel version: the rounded coordinates are inside the source
        if (fSrcX >= -0.5f && fSrcX < w - 0.5f && fSrcY >= -0.5f && fSrcY < h - 0.5f) {
          float flX = floorf(fSrcX);
          float flY = floorf(fSrcY);
          int fx = int((fSrcX - flX) * 256);
          int fy = int((fSrcY - flY) * 256);
          coord_t xa = max<coord_t>(0, coord_t(flX)), xb = min<coord_t>(w - 1, coord_t(flX) + 1);
          coord_t ya = max<coord_t>(0, coord_t(flY)), yb = min<coord_t>(h - 1, coord_t(flY) + 1);
#if LCD_ORIENTATION == 270
          *dest = bilinear<C>(source, srcData[xa * h + ya], srcData[xb * h + ya], srcData[xa * h + yb], srcData[xb * h + yb], fx, fy);
#else
          *dest = bilinear<C>(source, srcData[ya * w + xa], srcData[ya * w + xb], srcData[yb * w + xa], srcData[yb * w + xb], fx, fy);
#endif
        }
        else {
          *dest = 0;
        }
        continue;
      }
      coord_t x = round(dX * cosine + dY * sine + x0);
      coord_t y = round(dY * cosine - dX * sine + y0);
      if (x >= 0 && x < w && y >= 0 && y < h) {
#if LCD_ORIENTATION == 270
        *dest = srcData[x * h + y];
#else
        *dest = srcData[y * w + x];
#endif
      }
      else {
        *dest = 0;
      }
    }
  }

  return result;
}

template <class C>
static C * horizontalFlip(const C * source)
{
  auto * result = allocateLike(source, source->width(), source->height());
  if (!result) {
    return nullptr;
  }

#if LCD_ORIENTATION == 270
  auto sourceData = source->getData();
  auto sourceWidth = source->width();
  auto sourceHeight = source->height();
  auto resultData = result->getData();
  for (coord_t y = 0; y < sourceHeight; y++) {
    for (coord_t x = 0; x < sourceWidth; x++) {
      auto * p = &sourceData[(sourceWidth - 1 - x) * sourceHeight + y];
      auto * q = &resultData[x * sourceHeight + y];
      *q = *p;
    }
  }
#else
  auto sourceWidth = source->width();
  auto * srcData = source->getData() + sourceWidth;
  auto * destData = result->getData();
  for (coord_t y = 0; y < source->height(); y++) {
    for (coord_t x = 0; x < sourceWidth; x++) {
      *(destData++) = *(--srcData);
    }
    srcData += 2 * sourceWidth;
  }
#endif
  return result;
}

template <class C>
C * rotate90(const C * source)
{
  auto * result = allocateLike(source, source->height(), source->width());
  if (!result) {
    return nullptr;
  }

#if LCD_ORIENTATION == 270
  auto * srcData = source->getData();
  auto * destData = result->getData();
  for (coord_t y = 0; y < source->width(); y++) {
    for (coord_t x = 0; x < source->height(); x++) {
      destData[x * source->width() + y] = srcData[(y + 1) * source->height() - (x + 1)];
    }
  }
#else
  auto * destData = result->getData() + source->height();
  for (coord_t y = 0; y < source->width(); y++) {
    auto * srcData = source->getData() + y;
    for (coord_t x = 0; x < source->height(); x++) {
      destData--;
      *destData = *srcData;
      srcData += source->width();
    }
    destData += 2 * source->height();
  }
#endif
  return result;
}

template <class C>
C * rotate180(const C * source)
{
  auto * result = allocateLike(source, source->width(), source->height());
  if (!result) {
    return nullptr;
  }

  auto * srcData = source->getData();
  auto * destData = result->getDataEnd();
  while (destData > result->getData()) {
    *(--destData) = *srcData++;
  }

  return result;
}

// Resize to width x height, bilinear interpolation (antialias) or nearest neighbour
template <class C>
C * scale(const C * source, coord_t width, coord_t height, bool antialias = true)
{
  auto w = source->width();
  auto h = source->height();
  if (width <= 0 || height <= 0 || w <= 0 || h <= 0) {
    return nullptr;
  }

  auto * result = allocateLike(source, width, height);
  if (!result) {
    return nullptr;
  }

  auto * srcData = source->getData();
  auto * destData = result->getData();

  if (antialias) {
    // the center of the destination pixel (x + 0.5) is projected on the source
    const float ratioX = float(w) / width;
    const float ratioY = float(h) / height;
    for (coord_t y = 0; y < height; y++) {
      float fSrcY = (y + 0.5f) * ratioY - 0.5f;
      float flY = floorf(fSrcY);
      int fy = int((fSrcY - flY) * 256);
      coord_t ya = max<coord_t>(0, coord_t(flY)), yb = min<coord_t>(h - 1, coord_t(flY) + 1);
      for (coord_t x = 0; x < width; x++) {
        float fSrcX = (x + 0.5f) * ratioX - 0.5f;
        float flX = floorf(fSrcX);
        int fx = int((fSrcX - flX) * 256);
        coord_t xa = max<coord_t>(0, coord_t(flX)), xb = min<coord_t>(w - 1, coord_t(flX) + 1);
#if LCD_ORIENTATION == 270
        // the destination is written column by column
        destData[x * height + y] = bilinear<C>(source, srcData[xa * h + ya], srcData[xb * h + ya], srcData[xa * h + yb], srcData[xb * h + yb], fx, fy);
#else
        destData[y * width + x] = bilinear<C>(source, srcData[ya * w + xa], srcData[ya * w + xb], srcData[yb * w + xa], srcData[yb * w + xb], fx, fy);
#endif
      }
    }
    return result;
  }

  // The source index of a destination index i is i * srcSize / dstSize, computed without any
  // division in the loops: the integer part and the remainder are incremented step by step.
  const coord_t stepX = w / width, remX = w % width;
  const coord_t stepY = h / height, remY = h % height;

#if LCD_ORIENTATION == 270
  // the bitmaps are stored column by column: pixel (x, y) is at data[x * h + y]
  // the destination is written column by column, sequentially
  coord_t sx = 0, errX = 0;
  for (coord_t x = 0; x < width; x++) {
    const auto * srcColumn = srcData + sx * h;
    coord_t sy = 0, errY = 0;
    for (coord_t y = 0; y < height; y++) {
      *destData++ = srcColumn[sy];
      sy += stepY;
      errY += remY;
      if (errY >= height) {
        errY -= height;
        sy++;
      }
    }
    sx += stepX;
    errX += remX;
    if (errX >= width) {
      errX -= width;
      sx++;
    }
  }
#else
  coord_t sy = 0, errY = 0;
  for (coord_t y = 0; y < height; y++) {
    const auto * srcRow = srcData + sy * w;
    coord_t sx = 0, errX = 0;
    for (coord_t x = 0; x < width; x++) {
      *destData++ = srcRow[sx];
      sx += stepX;
      errX += remX;
      if (errX >= width) {
        errX -= width;
        sx++;
      }
    }
    sy += stepY;
    errY += remY;
    if (errY >= height) {
      errY -= height;
      sy++;
    }
  }
#endif

  return result;
}

class BitmapBuffer: public Bitmap
{
  public:
    BitmapBuffer(BitmapFormat format, uint16_t width, uint16_t height, pixel_t * data):
      Bitmap(format, width, height, data),
      xmax(width),
      ymax(height)
    {
    }
  
    inline void clearClippingRect()
    {
      xmin = 0;
      xmax = _width;
      ymin = 0;
      ymax = _height;
    }
  
    inline void setClippingRect(coord_t xmin, coord_t xmax, coord_t ymin, coord_t ymax)
    {
      this->xmin = max<coord_t>(0, xmin);
      this->xmax = min<coord_t>(this->_width, xmax);
      this->ymin = max<coord_t>(0, ymin);
      this->ymax = min<coord_t>(this->_height, ymax);
    }
  
    inline void setClippingRect(const rect_t & rect)
    {
      setClippingRect(rect.left(), rect.right(), rect.top(), rect.bottom());
    }
  
    inline void getClippingRect(coord_t & xmin, coord_t & xmax, coord_t & ymin, coord_t & ymax) const
    {
      xmin = this->xmin;
      xmax = this->xmax;
      ymin = this->ymin;
      ymax = this->ymax;
    }
  
    inline rect_t getClippingRect() const
    {
      return {xmin, ymin, xmax - xmin, ymax - ymin };
    }

    inline void setOffsetX(coord_t offsetX)
    {
      this->offsetX = offsetX;
    }
  
    inline void setOffsetY(coord_t offsetY)
    {
      this->offsetY = offsetY;
    }

    inline void setOffset(coord_t offsetX, coord_t offsetY)
    {
      setOffsetX(offsetX);
      setOffsetY(offsetY);
    }

    inline void clearOffset()
    {
      setOffset(0, 0);
    }
  
    inline void reset()
    {
      clearOffset();
      clearClippingRect();
    }

    [[nodiscard]] coord_t getOffsetX() const
    {
      return offsetX;
    }

    [[nodiscard]] coord_t getOffsetY() const
    {
      return offsetY;
    }

    [[nodiscard]] inline DataType * getDataEnd() const
    {
      return dataEnd;
    }

    static BitmapBuffer * allocate(BitmapFormat format, uint16_t width, uint16_t height) 
    {
      return allocateWithPixels<BitmapBuffer>(width, height, format);
    }

    inline void clear(Color565 color = 0 /*black*/)
    {
      drawPlainFilledRectangle(0, 0, _width - offsetX, _height - offsetY, color);
    }

    inline void clearTransparent()
    {
      fillRectangle(0, 0, LCD_W, LCD_H, ARGB4444(0x0F, 0, 0, 0));
    }

    [[nodiscard]] inline const pixel_t * getPixelPtr(coord_t x, coord_t y) const
    {
      APPLY_OFFSET();

      if (!applyPixelClippingRect(x, y))
        return nullptr;

      return getPixelPtrAbs(x, y);
    }

    inline void drawPixel(coord_t x, coord_t y, pixel_t value)
    {
      APPLY_OFFSET();
      drawPixelAbsWithClipping(x, y, value);
    }

    void drawAlphaPixel(pixel_t * p, uint8_t opacity, Color565 color);

    inline void drawAlphaPixel(coord_t x, coord_t y, uint8_t opacity, pixel_t value)
    {
      APPLY_OFFSET();

      if (!applyPixelClippingRect(x, y))
        return;

      drawAlphaPixelAbs(x, y, opacity, value);
    }

    void drawHorizontalLine(coord_t x, coord_t y, coord_t w, LcdColor color, uint8_t pat);
    inline void drawPlainHorizontalLine(coord_t x, coord_t y, coord_t w, Color565 color)
    {
      drawPlainFilledRectangle(x, y, w, 1, color);
    }

    void drawVerticalLine(coord_t x, coord_t y, coord_t h, LcdColor color, uint8_t pat);
    inline void drawPlainVerticalLine(coord_t x, coord_t y, coord_t h, Color565 color)
    {
      drawPlainFilledRectangle(x, y, 1, h, color);
    }

    // antialias: smooth edges (never with a pattern)
    void drawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, LcdColor color, uint8_t pat = SOLID, bool antialias = true);

    void drawTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, coord_t y2, LcdColor color, uint8_t pat = SOLID, bool antialias = true)
    {
      drawLine(x0, y0, x1, y1, color, pat, antialias);
      drawLine(x1, y1, x2, y2, color, pat, antialias);
      drawLine(x2, y2, x0, y0, color, pat, antialias);
    }

    void drawFilledTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, coord_t y2, LcdColor color, bool antialias = true);

    void drawRectangle(coord_t x, coord_t y, coord_t w, coord_t h, LcdColor color, uint8_t thickness = 1, uint8_t pat = SOLID);

    inline void drawPlainRectangle(coord_t x, coord_t y, coord_t w, coord_t h, Color565 color, uint8_t thickness = 1)
    {
      drawPlainFilledRectangle(x, y, thickness, h, color);
      drawPlainFilledRectangle(x+w-thickness, y, thickness, h, color);
      drawPlainFilledRectangle(x, y, w, thickness, color);
      drawPlainFilledRectangle(x, y+h-thickness, w, thickness, color);
    }

    void drawFilledRectangle(coord_t x, coord_t y, coord_t w, coord_t h, LcdColor color, uint8_t pat = SOLID);

    void drawPlainFilledRectangle(coord_t x, coord_t y, coord_t w, coord_t h, Color565 color);

    void drawMaskFilledRectangle(coord_t x, coord_t y, coord_t w, coord_t h, const Mask * mask, Color565 color);

    void drawCircle(coord_t x, coord_t y, coord_t radius, LcdColor color, bool antialias = true);

    void drawFilledCircle(coord_t x, coord_t y, coord_t radius, LcdColor color, uint8_t pat = SOLID, bool antialias = true);

    void drawPlainFilledCircle(coord_t x, coord_t y, coord_t radius, Color565 color, bool antialias = true);

    void drawAnnulusSector(coord_t x, coord_t y, coord_t internalRadius, coord_t externalRadius, LcdColor color, float startAngle, float endAngle, bool antialias = true);

    void drawBitmapPie(int x0, int y0, const uint16_t * img, int startAngle, int endAngle);

    void drawBitmapPatternPie(coord_t x0, coord_t y0, const Mask * mask, LcdColor color, float startAngle, float endAngle);

    static BitmapBuffer * loadMaskOnBackground(const char * filename, Color565 foreground, Color565 background, int maxSize = -1);

    void drawMask(coord_t x, coord_t y, const Mask * mask, Color565 color, coord_t srcx = 0, coord_t srcy = 0, coord_t srcw = 0, coord_t srch = 0);
    void drawMask(coord_t x, coord_t y, const Mask * mask, const Bitmap * srcBitmap, coord_t offsetX = 0, coord_t offsetY = 0, coord_t width = 0, coord_t height = 0);

    coord_t drawSizedText(coord_t x, coord_t y, const char * s, uint8_t len, LcdColor color, LcdFlags flags = 0);
    coord_t drawSizedText(coord_t x, coord_t y, const wchar_t * s, uint8_t len, LcdColor color, LcdFlags flags = 0);

    coord_t drawText(coord_t x, coord_t y, const char * s, LcdColor color, LcdFlags flags = 0)
    {
      return drawSizedText(x, y, s, 0, color, flags);
    }

    coord_t drawText(coord_t x, coord_t y, const wchar_t * s, LcdColor color, LcdFlags flags = 0)
    {
      return drawSizedText(x, y, s, 0, color, flags);
    }

    coord_t drawTextAtIndex(coord_t x, coord_t y, const char * s, uint8_t idx, LcdColor color, LcdFlags flags = 0)
    {
      char length = *s++;
      return drawSizedText(x, y, s+length*idx, length, color, flags);
    }

    coord_t drawNumber(coord_t x, coord_t y, int32_t val, LcdColor color, LcdFlags flags = 0, uint8_t len = 0, const char * prefix = nullptr, const char * suffix = nullptr);

    void copyFrom(const BitmapBuffer * other)
    {
      DMACopyBitmap(getData(), _width, _height, 0, 0, other->getData(), _width, _height, 0, 0, _width, _height);
    }

    void drawBitmap(coord_t x, coord_t y, const Bitmap * bitmap, coord_t srcx = 0, coord_t srcy = 0, coord_t srcw = 0, coord_t srch = 0, float scale = 0);

    void drawScaledBitmap(const Bitmap * bitmap, coord_t x, coord_t y, coord_t w, coord_t h);

  protected:
    coord_t xmin = 0;
    coord_t xmax;
    coord_t ymin = 0;
    coord_t ymax;
    coord_t offsetX = 0;
    coord_t offsetY = 0;

    void fillHorizontalLineWithAlphaOnEnds(coord_t y, float x, float w, LcdColor color, bool antialias = true);

    inline bool applyClippingRect(coord_t & x, coord_t & y, coord_t & w, coord_t & h) const
    {
      if (h < 0) {
        y += h;
        h = -h;
      }

      if (w < 0) {
        x += w;
        w = -w;
      }

      if (x >= xmax || y >= ymax)
        return false;

      if (y < ymin) {
        h += y - ymin;
        y = ymin;
      }

      if (x < xmin) {
        w += x - xmin;
        x = xmin;
      }

      if (y + h > ymax)
        h = ymax - y;

      if (x + w > xmax)
        w = xmax - x;

      return data && h > 0 && w > 0;
    }

    inline bool applyPixelClippingRect(coord_t x, coord_t y) const
    {
      coord_t w = 1, h = 1;
      return applyClippingRect(x, y, w, h);
    }

    void drawChar(coord_t x, coord_t y, const FontGlyph & glyph, LcdColor color);

    inline void drawPixel(pixel_t * p, pixel_t value)
    {
      if (data && data <= p && p < dataEnd) {
        *p = value;
      }
#if defined(DEBUG)
      else if (!leakReported) {
        leakReported = true;
        TRACE("BitmapBuffer(%p).drawPixel(): buffer overrun, data: %p, written at: %p", this, data, p);
      }
#endif
    }

    inline void drawPixelAbs(coord_t x, coord_t y, pixel_t value)
    {
      pixel_t * p = getPixelPtrAbs(x, y);
      drawPixel(p, value);
    }

    inline void drawAlphaPixelAbs(coord_t x, coord_t y, uint8_t alpha, Color565 color)
    {
      pixel_t * p = getPixelPtrAbs(x, y);
      drawAlphaPixel(p, alpha, color);
    }

    inline void drawPixelAbsWithClipping(coord_t x, coord_t y, pixel_t value)
    {
      if (applyPixelClippingRect(x, y)) {
        drawPixelAbs(x, y, value);
      }
    }

    void drawHorizontalLineAbs(coord_t x, coord_t y, coord_t w, LcdColor color, uint8_t pat = SOLID);

    void drawAntialiasedCircle(coord_t x, coord_t y, coord_t radius, Color565 color, uint8_t alpha, bool filled);

    void fillRectangle(coord_t x, coord_t y, coord_t w, coord_t h, pixel_t color);

    void fillBottomFlatTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, LcdColor color, bool antialias = true);
    void fillTopFlatTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t x2, coord_t y2, LcdColor color, bool antialias = true);

    void drawAntialiasedLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, Color565 color, uint8_t alpha);

  private:
#if defined(DEBUG)
    bool leakReported = false;
#endif
};

extern BitmapBuffer * lcd;
