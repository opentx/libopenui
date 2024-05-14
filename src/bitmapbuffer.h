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
#include "bitmapdata.h"
#include "libopenui_types.h"
#include "libopenui_defines.h"
#include "libopenui_depends.h"
#include "libopenui_helpers.h"
#include "font.h"
#include "debug.h"

constexpr uint8_t SOLID = 0xFF;
constexpr uint8_t DOTTED  = 0x55;
constexpr uint8_t STASHED = 0x33;

#define MOVE_OFFSET() coord_t offsetX = this->offsetX; x += offsetX; this->offsetX = 0; coord_t offsetY = this->offsetY; y += offsetY; this->offsetY = 0
#define APPLY_OFFSET() x += this->offsetX; y += this->offsetY
#define RESTORE_OFFSET()  this->offsetX = offsetX, this->offsetY = offsetY

typedef uint16_t pixel_t;

enum BitmapFormat
{
  BMP_RGB565,
  BMP_ARGB4444
};

template<class T>
class BitmapBufferBase
{
  public:
    BitmapBufferBase(uint8_t format, uint16_t width, uint16_t height, T * data):
      format(format),
      _width(width),
      _height(height),
      xmax(width),
      ymax(height),
      data(data),
      dataEnd(data + (width * height))
    {
    }

    BitmapBufferBase(uint8_t format, T * data):
      format(format),
      _width(*((uint16_t*)data)),
      _height(*(((uint16_t*)data) + 1)),
      xmax(_width),
      ymax(_height),
      data((T *)(((uint16_t *)data) + 2)),
      dataEnd((T *)(((uint16_t *)data) + 2) + (_width * _height))
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
      this->xmax = min<coord_t>(_width, xmax);
      this->ymin = max<coord_t>(0, ymin);
      this->ymax = min<coord_t>(_height, ymax);
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

    [[nodiscard]] inline uint8_t getFormat() const
    {
      return format;
    }

    [[nodiscard]] inline uint16_t width() const
    {
      return _width;
    }

    [[nodiscard]] inline uint16_t height() const
    {
      return _height;
    }

    [[nodiscard]] inline T * getData() const
    {
      return data;
    }

    [[nodiscard]] inline T * getDataEnd() const
    {
      return dataEnd;
    }

    [[nodiscard]] uint32_t getDataSize() const
    {
      return _width * _height * sizeof(T);
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
        return &data[x * (_height) + y];
  #else
      return &data[x * (_height) + y];
  #endif
#endif
      return &data[y * _width + x];
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
        return &data[x * (_height) + y];
  #else
      return &data[x * (_height) + y];
  #endif
#endif
      return &data[y * _width + x];
    }

    template <class C>
    C * horizontalFlip() const
    {
      auto * result = new C(format, width(), height());
#if LCD_ORIENTATION == 270
      for (uint8_t y = 0; y < height(); y++) {
        for (uint8_t x = 0; x < width(); x++) {
          auto * destData = &result->data[x * height() + y];
          auto * srcData = &data[(width() - 1 - x) * height() + y];
          *destData = *srcData;
        }
      }
#else
      auto * srcData = data + width();
      auto * destData = result->data;
      for (uint8_t y = 0; y < height(); y++) {
        for (uint8_t x = 0; x < width(); x++) {
          *(destData++) = *(--srcData);
        }
        srcData += 2 * width();
      }
#endif
      return result;
    }

    template <class C>
    C * verticalFlip() const
    {
      auto * result = new C(format, width(), height());
#if LCD_ORIENTATION == 270
      for (uint8_t y = 0; y < height(); y++) {
        for (uint8_t x = 0; x < width(); x++) {
          auto * destData = &result->data[x * height() + y];
          auto * srcData = &data[x * height() + height() - 1 - y];
          *destData = *srcData;
        }
      }
#else
      auto * srcData = getDataEnd() - width();
      auto * destData = result->data;
      for (uint8_t y = 0; y < height(); y++) {
        for (uint8_t x = 0; x < width(); x++) {
          *(destData++) = *(srcData++);
        }
        srcData -= 2 * width();
      }
#endif
      return result;
    }

    template <class C>
    C * rotate(double radians) const
    {
      double sinA = sin(radians);
      double cosA = cos(radians);

      auto w = width();
      auto h = height();

      double x1 = -h * sinA; 
      double y1 = h * cosA; 
      double x3 = w * cosA; 
      double y3 = w * sinA; 
      double x2 = x1 + x3; 
      double y2 = y1 + y3;

      float minx = min(double(0), min(x1, min(x2, x3))); 
      float miny = min(double(0), min(y1, min(y2, y3))); 
      float maxx = max(x1, max(x2, x3)); 
      float maxy = max(y1, max(y2, y3));

      coord_t rW = ceil(fabs(maxx) - minx); 
      coord_t rH = ceil(fabs(maxy) - miny); 

      auto x0 = (w - 1) / 2;
      auto y0 = (h - 1) / 2;

      auto rX0 = (rW - 1) / 2;
      auto rY0 = (rH - 1) / 2;

      auto * result = new C(format, rW, rH);

      auto * srcData = data;
      auto * destData = result->data;

      for (unsigned rX = 0; rX < rW; rX++) {
        coord_t dX = rX - rX0;
        for (unsigned rY = 0; rY < rH; rY++) {
          coord_t dY = rY - rY0;
          coord_t x = (double)dX * cosA + (double)dY * sinA + x0;
          if (x >= 0 && x < w) {
            coord_t y = (double)dY * cosA - (double)dX * sinA + y0;
            if (y >= 0 && y < h) {
              destData[rY * rW + rX] = srcData[y * w + x];
            }
            else {
              destData[rY * rW + rX] = 0;
            }
          }
          else {
            destData[rY * rW + rX] = 0;
          }
        }
      }

      return result;
    }

    template <class C>
    C * rotate90() const
    {
      auto * result = new C(format, height(), width());
  #if LCD_ORIENTATION == 270
      auto * srcData = data;
      auto * destData = result->data;
      for (uint8_t y = 0; y < width(); y++) {
        for (uint8_t x = 0; x < height(); x++) {
          destData[x * width() + y] = srcData[y * height() + x];
        }
      }
  #else
      auto * destData = result->data + height();
      for (uint8_t y = 0; y < width(); y++) {
        auto * srcData = data + y;
        for (uint8_t x = 0; x < height(); x++) {
          destData--;
          *destData = *srcData;
          srcData += width();
        }
        destData += 2 * height();
      }
  #endif
      return result;
    }

  protected:
    uint8_t format;
    coord_t _width;
    coord_t _height;
    coord_t xmin = 0;
    coord_t xmax;
    coord_t ymin = 0;
    coord_t ymax;
    coord_t offsetX = 0;
    coord_t offsetY = 0;
    T * data;
    T * dataEnd;
};

typedef BitmapBufferBase<const uint16_t> Bitmap;
typedef BitmapBufferBase<const uint8_t> StaticMask;

class RLEBitmap: public BitmapBufferBase<uint16_t>
{
  public:
    RLEBitmap(uint8_t format, const uint8_t* rle_data) :
      BitmapBufferBase<uint16_t>(format, 0, 0, nullptr)
    {
      _width = *((uint16_t *)rle_data);
      _height = *(((uint16_t *)rle_data)+1);
      uint32_t pixels = _width * _height;
      data = (uint16_t*)malloc(align32(pixels * sizeof(uint16_t)));
      decode((uint8_t *)data, pixels * sizeof(uint16_t), rle_data+4);
      dataEnd = data + pixels;
    }

    ~RLEBitmap()
    {
      free(data);
    }

    static int decode(uint8_t * dest, unsigned int destSize, const uint8_t * src)
    {
      uint8_t prevByte = 0;
      bool prevByteValid = false;

      const uint8_t * destEnd = dest + destSize;
      uint8_t * d = dest;

      while (d < destEnd) {
        uint8_t byte = *src++;
        *d++ = byte;

        if (prevByteValid && byte == prevByte) {
          uint8_t count = *src++;

          if (d + count > destEnd) {
            TRACE("rle_decode_8bit: destination overflow!\n");
            return -1;
          }

          memset(d, byte, count);
          d += count;
          prevByteValid = false;
        }
        else {
          prevByte = byte;
          prevByteValid = true;
        }
      }

      return d - dest;
    }
};

class BitmapMask: public BitmapBufferBase<uint8_t>
{
  public:
    BitmapMask(uint8_t format, uint16_t width, uint16_t height):
      BitmapBufferBase<uint8_t>(format, width, height, (uint8_t *)malloc(align32(width * height)))
    {
    }

    ~BitmapMask()
    {
      free(data);
    }

    [[nodiscard]] BitmapMask * invert() const
    {
      auto result = new BitmapMask(format, width(), height());
      auto * srcData = data;
      auto * destData = result->data;
      for (auto y = 0; y < height(); y++) {
        for (auto x = 0; x < width(); x++) {
          destData[x] = 0xFF - srcData[x];
        }
        srcData += width();
        destData += width();
      }
      return result;
    }

    static BitmapMask * load(const char * filename, int maxSize = -1);
};

class BitmapBuffer: public BitmapBufferBase<pixel_t>
{
  public:
    BitmapBuffer(uint8_t format, uint16_t width, uint16_t height);
    BitmapBuffer(uint8_t format, uint16_t width, uint16_t height, uint16_t * data);

    ~BitmapBuffer();

    [[nodiscard]] inline bool isValid() const
    {
      return data != nullptr;
    }

    [[nodiscard]] double getScale(coord_t w, coord_t h) const
    {
      double vscale = double(h) / height();
      double hscale = double(w) / width();
      return vscale < hscale ? vscale : hscale;
    }

    inline void setFormat(uint8_t format)
    {
      this->format = format;
    }

    inline void clear(Color565 color = 0 /*black*/)
    {
      fillRectangle(0, 0, _width - offsetX, _height - offsetY, color);
    }

    [[nodiscard]] inline const pixel_t * getPixelPtr(coord_t x, coord_t y) const
    {
      APPLY_OFFSET();

      if (!applyPixelClippingRect(x, y))
        return nullptr;

      return BitmapBufferBase::getPixelPtrAbs(x, y);
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

    void drawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, LcdColor color, uint8_t pat = SOLID);

    void drawTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, coord_t y2, LcdColor color, uint8_t pat = SOLID)
    {
      drawLine(x0, y0, x1, y1, color, pat);
      drawLine(x1, y1, x2, y2, color, pat);
      drawLine(x2, y2, x0, y0, color, pat);
    }

    void drawFilledTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, coord_t y2, LcdColor color);

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

    void drawCircle(coord_t x, coord_t y, coord_t radius, LcdColor color);

    void drawFilledCircle(coord_t x, coord_t y, coord_t radius, LcdColor color, uint8_t pat = SOLID);

    void drawPlainFilledCircle(coord_t x, coord_t y, coord_t radius, Color565 color);

    void drawAnnulusSector(coord_t x, coord_t y, coord_t internalRadius, coord_t externalRadius, LcdColor color, int startAngle, int endAngle);

    void drawBitmapPie(int x0, int y0, const uint16_t * img, int startAngle, int endAngle);

    void drawBitmapPatternPie(coord_t x0, coord_t y0, const uint8_t * img, LcdColor color, int startAngle, int endAngle);

    static BitmapBuffer * load(const char * filename, int maxSize = -1);

    static BitmapBuffer * loadMaskOnBackground(const char * filename, Color565 foreground, Color565 background, int maxSize = -1);

    template <class T>
    void drawMask(coord_t x, coord_t y, const T * mask, Color565 color, coord_t srcx = 0, coord_t srcw = 0);

//    void drawMask(coord_t x, coord_t y, const uint8_t * mask, Color565 color, coord_t srcx = 0, coord_t srcw = 0)
//    {
//      drawMask(x, y, (const BitmapData *)mask, flags, srcx, srcw);
//    }

    void drawMask(coord_t x, coord_t y, const BitmapMask * mask, const BitmapBuffer * srcBitmap, coord_t offsetX = 0, coord_t offsetY = 0, coord_t width = 0, coord_t height = 0);

    coord_t drawSizedText(coord_t x, coord_t y, const char * s, uint8_t len, LcdColor color, LcdFlags flags = 0);

    coord_t drawText(coord_t x, coord_t y, const char * s, LcdColor color, LcdFlags flags = 0)
    {
      return drawSizedText(x, y, s, 0, color, flags);
    }

    coord_t drawTextAtIndex(coord_t x, coord_t y, const char * s, uint8_t idx, LcdColor color, LcdFlags flags = 0)
    {
      char length = *s++;
      return drawSizedText(x, y, s+length*idx, length, color, flags);
    }

    coord_t drawNumber(coord_t x, coord_t y, int32_t val, LcdColor color, LcdFlags flags = 0, uint8_t len = 0, const char * prefix = nullptr, const char * suffix = nullptr);

    template<class T>
    void drawBitmap(coord_t x, coord_t y, const T * bmp, coord_t srcx = 0, coord_t srcy = 0, coord_t srcw = 0, coord_t srch = 0, float scale = 0);

    void copyFrom(const BitmapBuffer * other)
    {
      DMACopyBitmap(getData(), _width, _height, 0, 0, other->getData(), _width, _height, 0, 0, _width, _height);
    }

    template<class T>
    void drawScaledBitmap(const T * bitmap, coord_t x, coord_t y, coord_t w, coord_t h);

  protected:
    static BitmapBuffer * load_bmp(const char * filename, int maxSize = -1);
    static BitmapBuffer * load_stb(const char * filename, int maxSize = -1);

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

    uint8_t drawChar(coord_t x, coord_t y, const Font::Glyph & glyph, LcdColor color);

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

    bool clipLine(coord_t& x1, coord_t& y1, coord_t& x2, coord_t& y2);

    void fillRectangle(coord_t x, coord_t y, coord_t w, coord_t h, pixel_t color);

    void fillBottomFlatTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, LcdColor color);
    void fillTopFlatTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t x2, coord_t y2, LcdColor color);

  private:
    bool dataAllocated;
#if defined(DEBUG)
    bool leakReported = false;
#endif
};

extern BitmapBuffer * lcd;
