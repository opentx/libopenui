/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it underresult the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include <math.h>
#include "bitmapbuffer.h"
#include "libopenui_depends.h"
#include "libopenui_helpers.h"
#include "libopenui_file.h"
#include "font.h"
#include "file_reader.h"
#include "intconversions.h"

void BitmapBuffer::drawBitmap(coord_t x, coord_t y, const Bitmap * bitmap, coord_t srcx, coord_t srcy, coord_t srcw, coord_t srch, float scale)
{
  if (!data || !bitmap)
    return;

  APPLY_OFFSET();

  if (x >= xmax || y >= ymax)
    return;

  coord_t bmpw = bitmap->width();
  coord_t bmph = bitmap->height();

  if (srcw == 0)
    srcw = bmpw;
  if (srch == 0)
    srch = bmph;
  if (srcx + srcw > bmpw)
    srcw = bmpw - srcx;
  if (srcy + srch > bmph)
    srch = bmph - srcy;

  if (scale == 0) {
    if (x < xmin) {
      srcw += x - xmin;
      srcx -= x - xmin;
      x = xmin;
    }
    if (y < ymin) {
      srch += y - ymin;
      srcy -= y - ymin;
      y = ymin;
    }
    if (x + srcw > xmax) {
      srcw = xmax - x;
    }
    if (y + srch > ymax) {
      srch = ymax - y;
    }

    if (srcw <= 0 || srch <= 0) {
      return;
    }

    if (bitmap->getFormat() == BMP_ARGB4444 || _format == BMP_ARGB4444)
      DMACopyAlphaBitmap(data, _format == BMP_ARGB4444, _width, _height, x, y, bitmap->getData(), bitmap->getFormat() == BMP_ARGB4444, bmpw, bmph, srcx, srcy, srcw, srch);
    else
      DMACopyBitmap(data, _width, _height, x, y, bitmap->getData(), bmpw, bmph, srcx, srcy, srcw, srch);
  }
  else {
    if (x < xmin) {
      srcw += (x - xmin) / scale;
      srcx -= (x - xmin) / scale;
      x = xmin;
    }
    if (y < ymin) {
      srch += (y - ymin) / scale;
      srcy -= (y - ymin) / scale;
      y = ymin;
    }
    if (x + srcw * scale > xmax) {
      srcw = int(ceil((xmax - x) / scale));
    }
    if (y + srch * scale > ymax) {
      srch = int(ceil((ymax - y) / scale));
    }

    if (srcw <= 0 || srch <= 0) {
      return;
    }

    auto scaledw = min<int>(xmax - xmin, ceil(scale * srcw));
    auto scaledh = min<int>(ymax - ymin, ceil(scale * srch));

    if (x + scaledw > _width)
      scaledw = _width - x;
    if (y + scaledh > _height)
      scaledh = _height - y;

    if (getFormat() == BMP_ARGB4444)  {
      for (int i = 0; i < scaledh; i++) {
        pixel_t * p = getPixelPtrAbs(x, y + i);
        const pixel_t * qstart = bitmap->getPixelPtrAbs(srcx, srcy + int(i / scale));
        for (int j = 0; j < scaledw; j++) {
          const pixel_t * q = qstart;
          q = bitmap->getNextPixel(q, j / scale);
          if (bitmap->getFormat() == BMP_RGB565) {
            RGB_SPLIT(*q, r, g, b);
            drawPixel(p, ARGB_JOIN(0xF, r>>1, g>>2, b>>1));
          }
          else {  // bmp->getFormat() == BMP_ARGB4444
            drawPixel(p, *q);
          }
          p = getNextPixel(p);
        }
      }
    }
    else {
      for (int i = 0; i < scaledh; i++) {
        pixel_t * p = getPixelPtrAbs(x, y + i);
        const pixel_t * qstart = bitmap->getPixelPtrAbs(srcx, srcy + int(i / scale));
        for (int j = 0; j < scaledw; j++) {
          const pixel_t * q = qstart;
          q = bitmap->getNextPixel(q, j / scale);
          if (bitmap->getFormat() == BMP_ARGB4444) {
            ARGB_SPLIT(*q, a, r, g, b);
            drawAlphaPixel(p, a, RGB_JOIN(r<<1, g<<2, b<<1));
          }
          else {
            drawPixel(p, *q);
          }
          p = getNextPixel(p);
        }
      }
    }
  }
}

void BitmapBuffer::drawScaledBitmap(const Bitmap * bitmap, coord_t x, coord_t y, coord_t w, coord_t h)
{
  if (bitmap) {
    auto scale = bitmap->getScale(w, h);
    int xshift = (w - (bitmap->width() * scale)) / 2;
    int yshift = (h - (bitmap->height() * scale)) / 2;
    drawBitmap(x + xshift, y + yshift, bitmap, 0, 0, 0, 0, scale);
  }
}

void BitmapBuffer::drawAlphaPixel(pixel_t * p, uint8_t alpha, Color565 color)
{
  if (_format == BMP_RGB565) {
    if (alpha == ALPHA_MAX) {
      drawPixel(p, color);
    }
    else if (alpha != 0) {
      uint8_t bgAlpha = ALPHA_MAX - alpha;
      RGB_SPLIT(color, red, green, blue);
      RGB_SPLIT(*p, bgRed, bgGreen, bgBlue);
      uint16_t r = (bgRed * bgAlpha + red * alpha) / ALPHA_MAX;
      uint16_t g = (bgGreen * bgAlpha + green * alpha) / ALPHA_MAX;
      uint16_t b = (bgBlue * bgAlpha + blue * alpha) / ALPHA_MAX;
      drawPixel(p, RGB_JOIN(r, g, b));
    }
  }
  else if (_format == BMP_ARGB4444) {
    if (alpha == ALPHA_MAX) {
      drawPixel(p, RGB565_TO_ARGB4444(color, 0xFF));
    }
    else if (alpha != 0) {
      // https://en.wikipedia.org/wiki/Alpha_compositing
      ARGB_SPLIT(*p, bgAlpha, bgRed, bgGreen, bgBlue);
      if (bgAlpha == 0) {
        drawPixel(p, RGB565_TO_ARGB4444(color, alpha << 4));
      }
      else {
        RGB_SPLIT(color, red, green, blue);
        red >>= 1;
        green >>= 2;
        blue >>= 1;
        uint16_t a = alpha + (bgAlpha * (ALPHA_MAX - alpha)) / ALPHA_MAX;
        uint16_t r = min<uint8_t>(0x0F, (red * alpha + bgRed * bgAlpha) / a);
        uint16_t g = min<uint8_t>(0x0F, (green * alpha + bgGreen * bgAlpha) / a);
        uint16_t b = min<uint8_t>(0x0F, (blue * alpha + bgBlue * bgAlpha) / a);
        drawPixel(p, ARGB_JOIN(a, r, g, b));
      }
    }
  }
}

void BitmapBuffer::drawHorizontalLine(coord_t x, coord_t y, coord_t w, LcdColor color, uint8_t pat)
{
  APPLY_OFFSET();

  coord_t h = 1;
  if (!applyClippingRect(x, y, w, h))
    return;

  drawHorizontalLineAbs(x, y, w, color, pat);
}

void BitmapBuffer::drawHorizontalLineAbs(coord_t x, coord_t y, coord_t w, LcdColor color, uint8_t pat)
{
  pixel_t * p = getPixelPtrAbs(x, y);
  auto rgb565 = COLOR_TO_RGB565(color);
  uint8_t alpha = GET_COLOR_ALPHA(color);

  if (pat == SOLID) {
    while (w--) {
      drawAlphaPixel(p, alpha, rgb565);
      p = getNextPixel(p, 1);
    }
  }
  else {
    while (w--) {
      if (pat & 1) {
        drawAlphaPixel(p, alpha, rgb565);
        pat = (pat >> 1) | 0x80;
      }
      else {
        pat = pat >> 1;
      }
      p = getNextPixel(p, 1);
    }
  }
}

template <class T>
T ror(T n, unsigned d)
{
   return (n >> d) | (n << (sizeof(n) * 8 - d));
}

void BitmapBuffer::drawVerticalLine(coord_t x, coord_t y, coord_t h, LcdColor color, uint8_t pat)
{
  APPLY_OFFSET();

  coord_t w = 1;
  coord_t h0 = h;
  if (!applyClippingRect(x, y, w, h))
    return;

  if (h0 != h) {
    pat = ror<uint8_t>(pat, (h0 - h) & 7);
  }

  auto rgb565 = COLOR_TO_RGB565(color);
  uint8_t alpha = GET_COLOR_ALPHA(color);

  if (pat == SOLID) {
    while (h--) {
      drawAlphaPixelAbs(x, y, alpha, rgb565);
      y++;
    }
  }
  else {
    if (pat == DOTTED && !(y & 1)) {
      pat = ~pat;
    }
    while (h--) {
      if (pat & 1) {
        drawAlphaPixelAbs(x, y, alpha, color);
        pat = (pat >> 1) | 0x80;
      }
      else {
        pat = pat >> 1;
      }
      y++;
    }
  }
}

//
// Liang-barsky clipping algo
//
static float lb_max(float arr[], int n)
{
  float m = 0;
  for (int i = 0; i < n; ++i)
    if (m < arr[i]) m = arr[i];
  return m;
}

// this function gives the minimum
static float lb_min(float arr[], int n)
{
  float m = 1;
  for (int i = 0; i < n; ++i)
    if (m > arr[i]) m = arr[i];
  return m;
}

bool BitmapBuffer::clipLine(coord_t & x1, coord_t & y1, coord_t & x2, coord_t & y2)
{
  // defining variables
  float p1 = -(x2 - x1);
  float p2 = -p1;
  float p3 = -(y2 - y1);
  float p4 = -p3;

  float q1 = x1 - xmin;
  float q2 = xmax - 1 - x1;
  float q3 = y1 - ymin;
  float q4 = ymax - 1 - y1;

  float posarr[5], negarr[5];
  int posind = 1, negind = 1;
  posarr[0] = 1;
  negarr[0] = 0;

  if ((p1 == 0 && q1 < 0) || (p2 == 0 && q2 < 0) || (p3 == 0 && q3 < 0) ||
      (p4 == 0 && q4 < 0)) {
    return false;
  }

  if (p1 != 0) {
    float r1 = q1 / p1;
    float r2 = q2 / p2;
    if (p1 < 0) {
      negarr[negind++] = r1;  // for negative p1, add it to negative array
      posarr[posind++] = r2;  // and add p2 to positive array
    } else {
      negarr[negind++] = r2;
      posarr[posind++] = r1;
    }
  }
  if (p3 != 0) {
    float r3 = q3 / p3;
    float r4 = q4 / p4;
    if (p3 < 0) {
      negarr[negind++] = r3;
      posarr[posind++] = r4;
    } else {
      negarr[negind++] = r4;
      posarr[posind++] = r3;
    }
  }

  float xn1, yn1, xn2, yn2;
  float rn1, rn2;
  rn1 = lb_max(negarr, negind);  // maximum of negative array
  rn2 = lb_min(posarr, posind);  // minimum of positive array

  if (rn1 > rn2) {  // reject
    return false;
  }

  xn1 = x1 + p2 * rn1;
  yn1 = y1 + p4 * rn1;  // computing new points

  xn2 = x1 + p2 * rn2;
  yn2 = y1 + p4 * rn2;

  x1 = coord_t(xn1);
  y1 = coord_t(yn1);
  x2 = coord_t(xn2);
  y2 = coord_t(yn2);

  return true;
}

void BitmapBuffer::drawLine(coord_t x1, coord_t y1, coord_t x2, coord_t y2, LcdColor color, uint8_t pat)
{
  // Offsets
  x1 += offsetX;
  y1 += offsetY;
  x2 += offsetX;
  y2 += offsetY;

  if (!clipLine(x1, y1, x2, y2))
    return;

  auto rgb565 = COLOR_TO_RGB565(color);
  uint8_t alpha = GET_COLOR_ALPHA(color);

  int dx = x2 - x1;      /* the horizontal distance of the line */
  int dy = y2 - y1;      /* the vertical distance of the line */
  int dxabs = abs(dx);
  int dyabs = abs(dy);
  int sdx = sgn(dx);
  int sdy = sgn(dy);
  int x = dyabs >> 1;
  int y = dxabs >> 1;
  int px = x1;
  int py = y1;

  if (dxabs >= dyabs) {
    /* the line is more horizontal than vertical */
    for (int i = 0; i <= dxabs; i++) {
      if ((1 << (px % 8)) & pat) {
        drawAlphaPixelAbs(px, py, alpha, rgb565);
      }
      y += dyabs;
      if (y >= dxabs) {
        y -= dxabs;
        py += sdy;
      }
      px += sdx;
    }
  }
  else {
    /* the line is more vertical than horizontal */
    for (int i = 0; i <= dyabs; i++) {
      if ((1 << (py % 8)) & pat) {
        drawAlphaPixelAbs(px, py, alpha, rgb565);
      }
      x += dxabs;
      if (x >= dyabs) {
        x -= dyabs;
        px += sdx;
      }
      py += sdy;
    }
  }
}

void BitmapBuffer::fillBottomFlatTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t y12, coord_t x2, LcdColor color)
{
  auto dy = y12 - y0;
  auto slopex = float(x1 - x0) / dy;
  auto slopew = float(x2 - x1) / dy;

  float x = x0;
  float w = 1;

  for (int y = y0; y <= y12; y++) {
    drawHorizontalLine(round(x), y, round(w), color, SOLID);
    x += slopex;
    w += slopew;
  }
}

void BitmapBuffer::fillTopFlatTriangle(coord_t x0, coord_t y01, coord_t x1, coord_t x2, coord_t y2, LcdColor color)
{
  auto dy = y2 - y01;
  auto slopex = float(x0 - x2) / dy;
  auto slopew = float(x1 - x0) / dy;

  float x = x2;
  float w = 1;

  for (int y = y2; y >= y01; y--) {
    drawHorizontalLine(round(x), y, round(w), color, SOLID);
    x += slopex;
    w += slopew;
  }
}

void BitmapBuffer::drawFilledTriangle(coord_t x0, coord_t y0, coord_t x1, coord_t y1, coord_t x2, coord_t y2, LcdColor color)
{
  // Sort the points so that y0 <= y1 <= y2
  if (y1 < y0) {
    std::swap(x1, x0);
    std::swap(y1, y0);
  }
  if (y2 < y0) {
    std::swap(x2, x0);
    std::swap(y2, y0);
  }
  if (y2 < y1) {
    std::swap(x2, x1);
    std::swap(y2, y1);
  }

  if (y1 == y2) {
    // bottom-flat triangle
    fillBottomFlatTriangle(x0, y0, x1, y1, x2, color);
  }
  else if (y0 == y1) {
    // top-flat triangle
    fillTopFlatTriangle(x0, y0, x1, x2, y2, color);
  }
  else {
    // general case: split the triangle in a top-flat and bottom-flat triangles
    coord_t x4 = x0 + multDivRoundClosest(x2 - x0, y1 - y0, y2 - y0);
    fillBottomFlatTriangle(x0, y0, x1, y1, x4, color);
    fillTopFlatTriangle(x1, y1 + 1, x4, x2, y2, color);
  }
}

void BitmapBuffer::drawRectangle(coord_t x, coord_t y, coord_t w, coord_t h, LcdColor color, uint8_t thickness, uint8_t pat)
{
  for (unsigned i = 0; i < thickness; i++) {
    drawVerticalLine(x + i, y, h, color, pat);
    drawVerticalLine(x + w - 1 - i, y, h, color, pat);
    drawHorizontalLine(x, y + h - 1 - i, w, color, pat);
    drawHorizontalLine(x, y + i, w, color, pat);
  }
}

void BitmapBuffer::fillRectangle(coord_t x, coord_t y, coord_t w, coord_t h, pixel_t pixel)
{
  APPLY_OFFSET();

  if (!applyClippingRect(x, y, w, h))
    return;

  DMAFillRect(data, _width, _height, x, y, w, h, pixel);
}

void BitmapBuffer::drawPlainFilledRectangle(coord_t x, coord_t y, coord_t w, coord_t h, Color565 color)
{
  if (_format == BMP_RGB565)
    fillRectangle(x, y, w, h, color);
  else
    fillRectangle(x, y, w, h, RGB565_TO_ARGB4444(color, 0xFF));
}

void BitmapBuffer::drawMaskFilledRectangle(coord_t x, coord_t y, coord_t w, coord_t h, const Mask * mask, Color565 color)
{
  coord_t maskHeight = mask->height();
  while (h > 0) {
    if (maskHeight > h)
      maskHeight = h;
    coord_t rowx = x;
    coord_t roww = w;
    coord_t maskWidth = mask->width();
    while (roww > 0) {
      if (maskWidth > roww)
        maskWidth = roww;
      drawMask(rowx, y, mask, color, 0, 0, maskWidth, maskHeight);
      rowx += maskWidth;
      roww -= maskWidth;
    }
    y += maskHeight;
    h -= maskHeight;
  }
}

void BitmapBuffer::drawFilledRectangle(coord_t x, coord_t y, coord_t w, coord_t h, LcdColor color, uint8_t pat)
{
  APPLY_OFFSET();

  if (!applyClippingRect(x, y, w, h))
    return;

  for (coord_t i = y; i < y + h; i++) {
    drawHorizontalLineAbs(x, i, w, color, pat);
  }
}

void BitmapBuffer::drawCircle(coord_t x, coord_t y, coord_t radius, LcdColor color)
{
  int x1 = radius;
  int y1 = 0;
  int decisionOver2 = 1 - x1;
  auto rgb565 = COLOR_TO_RGB565(color);
  uint8_t alpha = GET_COLOR_ALPHA(color);

  while (y1 <= x1) {
    drawAlphaPixel(x1 + x, y1 + y, alpha, color);
    drawAlphaPixel(y1 + x, x1 + y, alpha, rgb565);
    drawAlphaPixel(-x1 + x, y1 + y, alpha, rgb565);
    drawAlphaPixel(-y1 + x, x1 + y, alpha, rgb565);
    drawAlphaPixel(-x1 + x, -y1 + y, alpha, rgb565);
    drawAlphaPixel(-y1 + x, -x1 + y, alpha, rgb565);
    drawAlphaPixel(x1 + x, -y1 + y, alpha, rgb565);
    drawAlphaPixel(y1 + x, -x1 + y, alpha, rgb565);
    y1++;
    if (decisionOver2 <= 0) {
      decisionOver2 += 2 * y1 + 1;
    }
    else {
      x1--;
      decisionOver2 += 2 * (y1 - x1) + 1;
    }
  }
}

void BitmapBuffer::drawPlainFilledCircle(coord_t x, coord_t y, coord_t radius, Color565 color)
{
  coord_t imax = (radius * 707) / 1000 + 1;
  coord_t sqmax = radius * radius + radius / 2;
  coord_t x1 = radius;
  drawPlainHorizontalLine(x - radius, y, radius * 2 + 1, color);
  for (coord_t i = 1; i <= imax; i++) {
    if ((i * i + x1 * x1) > sqmax) {
      // Draw lines from outside
      if (x1 > imax) {
        drawPlainHorizontalLine(x - i + 1, y + x1, (i - 1) * 2 + 1, color);
        drawPlainHorizontalLine(x - i + 1, y - x1, (i - 1) * 2 + 1, color);
      }
      x1--;
    }
    // Draw lines from inside (center)
    drawPlainHorizontalLine(x - x1, y + i, x1 * 2 + 1, color);
    drawPlainHorizontalLine(x - x1, y - i, x1 * 2 + 1, color);
  }
}

void BitmapBuffer::drawFilledCircle(coord_t x, coord_t y, coord_t radius, LcdColor color, uint8_t pat)
{
  coord_t imax = ((coord_t)((coord_t)radius * 707)) / 1000 + 1;
  coord_t sqmax = (coord_t)radius * (coord_t)radius + (coord_t)radius / 2;
  coord_t x1 = radius;
  drawHorizontalLine(x - radius, y, radius * 2, color, pat);
  for (coord_t i = 1; i <= imax; i++) {
    if ((i * i + x1 * x1) > sqmax) {
      // Draw lines from outside
      if (x1 > imax) {
        drawHorizontalLine(x - i + 1, y + x1, (i - 1) * 2, color, pat);
        drawHorizontalLine(x - i + 1, y - x1, (i - 1) * 2, color, pat);
      }
      x1--;
    }
    // Draw lines from inside (center)
    drawHorizontalLine(x - x1, y + i, x1 * 2, color, pat);
    drawHorizontalLine(x - x1, y - i, x1 * 2, color, pat);
  }
}

class Slope
{
  public:
    explicit Slope(int angle)
    {
      if (angle < 0)
        angle += 360;
      if (angle > 360)
        angle %= 360;
      auto radians = degrees2radians<float>(angle);
      if (angle == 0) {
        left = false;
        value = 100000;
      }
      else if (angle == 360) {
        left = true;
        value = 100000;
      }
      else if (angle >= 180) {
        left = true;
        value = -(cosf(radians) * 100 / sinf(radians));
      }
      else {
        left = false;
        value = (cosf(radians) * 100 / sinf(radians));
      }
    }

    Slope(bool left, int value):
      left(left),
      value(value)
    {
    }

    bool isBetween(const Slope & start, const Slope & end) const
    {
      if (left) {
        if (start.left) {
          if (end.left)
            return end.value > start.value ? (value <= end.value && value >= start.value) : (value <= end.value || value >= start.value);
          else
            return value >= start.value;
        }
        else {
          if (end.left)
            return value <= end.value;
          else
            return end.value > start.value;
        }
      }
      else {
        if (start.left) {
          if (end.left)
            return start.value > end.value;
          else
            return value >= end.value;
        }
        else {
          if (end.left)
            return value <= start.value;
          else
            return end.value < start.value ? (value >= end.value && value <= start.value) : (value <= start.value || value >= end.value);
        }
      }
    }

    Slope & invertVertical()
    {
      value = -value;
      return *this;
    }

    Slope & invertHorizontal()
    {
      left = !left;
      return *this;
    }

  protected:
    bool left;
    int value;
};

void BitmapBuffer::drawBitmapPatternPie(coord_t x, coord_t y, const Mask * mask, LcdColor color, int startAngle, int endAngle)
{
  if (endAngle == startAngle) {
    endAngle += 1;
  }

  Slope startSlope(startAngle);
  Slope endSlope(endAngle);

  auto rgb565 = COLOR_TO_RGB565(color);

  coord_t width = mask->width();
  coord_t height = mask->height();
  const uint8_t * q = mask->getData();

  int w2 = width / 2;
  int h2 = height / 2;

  for (int y1 = h2 - 1; y1 >= 0; y1--) {
    for (int x1 = w2 - 1; x1 >= 0; x1--) {
      Slope slope(false, x1 == 0 ? 99000 : y1 * 100 / x1);
      if (slope.isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 + x1, y + h2 - y1, q[(h2 - y1) * width + w2 + x1] >> 4, rgb565);
      }
      if (slope.invertVertical().isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 + x1, y + h2 + y1, q[(h2 + y1) * width + w2 + x1] >> 4, rgb565);
      }
      if (slope.invertHorizontal().isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 - x1, y + h2 + y1, q[(h2 + y1) * width + w2 - x1] >> 4, rgb565);
      }
      if (slope.invertVertical().isBetween(startSlope, endSlope)) {
        drawAlphaPixel(x + w2 - x1, y + h2 - y1, q[(h2 - y1) * width + w2 - x1] >> 4, rgb565);
      }
    }
  }
}

void BitmapBuffer::drawAnnulusSector(coord_t x, coord_t y, coord_t internalRadius, coord_t externalRadius, LcdColor color, int startAngle, int endAngle)
{
  if (endAngle == startAngle) {
    endAngle += 1;
  }

  Slope startSlope(startAngle);
  Slope endSlope(endAngle);

  auto rgb565 = COLOR_TO_RGB565(color);
  APPLY_OFFSET();

  coord_t internalDist = internalRadius * internalRadius;
  coord_t externalDist = externalRadius * externalRadius;

  for (int y1 = 0; y1 <= externalRadius; y1++) {
    for (int x1 = 0; x1 <= externalRadius; x1++) {
      auto dist = x1 * x1 + y1 * y1;
      if (dist >= internalDist && dist <= externalDist) {
        Slope slope(false, x1 == 0 ? 99000 : y1 * 100 / x1);
        if (slope.isBetween(startSlope, endSlope))
          drawPixelAbsWithClipping(x + x1, y - y1, rgb565);
        if (slope.invertVertical().isBetween(startSlope, endSlope))
          drawPixelAbsWithClipping(x + x1, y + y1, rgb565);
        if (slope.invertHorizontal().isBetween(startSlope, endSlope))
          drawPixelAbsWithClipping(x - x1, y + y1, rgb565);
        if (slope.invertVertical().isBetween(startSlope, endSlope))
          drawPixelAbsWithClipping(x - x1, y - y1, rgb565);
      }
    }
  }
}

void BitmapBuffer::drawMask(coord_t x, coord_t y, const Mask * mask, Color565 color, coord_t srcx, coord_t srcy, coord_t srcw, coord_t srch)
{
  if (!mask)
    return;

  APPLY_OFFSET();

  coord_t maskWidth = mask->width();
  coord_t maskHeight = mask->height();

  if (!srcw) {
    srcw = maskWidth;
  }

  if (!srch) {
    srch = maskHeight;
  }

  if (srcx + srcw > maskWidth) {
    srcw = maskWidth - srcx;
  }

  if (srcy + srch > maskHeight) {
    srch = maskHeight - srcy;
  }

  if (x < xmin) {
    srcw += x - xmin;
    srcx -= x - xmin;
    x = xmin;
  }
  if (y < ymin) {
    srch += y - ymin;
    srcy -= y - ymin;
    y = ymin;
  }
  if (x + srcw > xmax) {
    srcw = xmax - x;
  }
  if (y + srch > ymax) {
    srch = ymax - y;
  }

  if (srcw <= 0 || srch <= 0) {
    return;
  }

  auto rgb565 = COLOR_TO_RGB565(color);
  DMACopyAlphaMask(data, _format == BMP_ARGB4444, _width, _height, x, y, mask->getData(), maskWidth, maskHeight, srcx, srcy, srcw, srch, rgb565);
}

void BitmapBuffer::drawMask(coord_t x, coord_t y, const Mask * mask, const Bitmap * srcBitmap, coord_t offsetX, coord_t offsetY, coord_t width, coord_t height)
{
  if (!mask || !srcBitmap)
    return;

  APPLY_OFFSET();

  coord_t maskWidth = mask->width();
  coord_t maskHeight = mask->height();

  if (!width || width > maskWidth) {
    width = maskWidth;
  }

  if (!height || height > maskHeight) {
    height = maskHeight;
  }

  if (x + width > xmax) {
    width = xmax - x;
  }

  if (x < xmin) {
    width += x - xmin;
    offsetX -= x - xmin;
    x = xmin;
  }

  if (y >= ymax || x >= xmax || width <= 0 || x + width < xmin || y + height < ymin)
    return;

  for (coord_t yCur = 0; yCur < height; yCur++) {
    if (y + yCur < ymin || y + yCur >= ymax)
      continue;
    auto * p = getPixelPtrAbs(x, y + yCur);
    const auto * q = mask->getPixelPtrAbs(offsetX, offsetY + yCur);
    for (coord_t xCur = 0; xCur < width; xCur++) {
      drawAlphaPixel(p, (*q) >> 4, *srcBitmap->getPixelPtrAbs(xCur, yCur));
      p = getNextPixel(p);
      q = mask->getNextPixel(q);
    }
  }
}

void BitmapBuffer::drawChar(coord_t x, coord_t y, const FontGlyph & glyph, LcdColor color)
{
  drawMask(x, y, glyph.data, color, glyph.offset, 0, glyph.width);
}

#define INCREMENT_POS(delta)    do { if (flags & VERTICAL) y -= delta; else x += delta; } while(0)

coord_t BitmapBuffer::drawSizedText(coord_t x, coord_t y, const char * s, uint8_t len, LcdColor color, LcdFlags flags)
{
  MOVE_OFFSET();

  auto font = getFont(flags);
  int height = font->getHeight();

  if (y + height <= ymin || y >= ymax) {
    RESTORE_OFFSET();
    return x;
  }

  if (flags & (RIGHT | CENTERED)) {
    int width = font->getTextWidth(s, len);
    if (flags & RIGHT) {
      INCREMENT_POS(-width);
    }
    else if (flags & CENTERED) {
      INCREMENT_POS(-width / 2);
    }
  }

  coord_t & pos = (flags & VERTICAL) ? y : x;
  const coord_t orig_pos = pos;

  auto curr = s;
  while (len == 0 || curr - s < len) {
    auto c = getNextUnicodeChar(curr);

    if (!c) {
      break;
    }

    if (c == ' ') {
      INCREMENT_POS(font->getSpaceWidth());
      continue;
    }

    if (c == '\n') {
      pos = orig_pos;
      if (flags & VERTICAL)
        x += height;
      else
        y += height;
      continue;
    }

    auto glyph = font->getGlyph(c);
    // TRACE("c = '%lc' 0x%X offset=%d width=%d", c, c, glyph.offset, glyph.width);
    if (glyph.width) {
      drawChar(x, y, glyph, color);
      INCREMENT_POS(glyph.width + font->getSpacing());
    }
    else {
      TRACE("Missing glyph '%lc' hex=0x%X", c, c);
      INCREMENT_POS(font->getSpacing());
    }
  }

  RESTORE_OFFSET();

  return ((flags & RIGHT) ? orig_pos : pos) - offsetX;
}


coord_t BitmapBuffer::drawSizedText(coord_t x, coord_t y, const wchar_t * s, uint8_t len, LcdColor color, LcdFlags flags)
{
  MOVE_OFFSET();

  auto font = getFont(flags);
  int height = font->getHeight();

  if (y + height <= ymin || y >= ymax) {
    RESTORE_OFFSET();
    return x;
  }

  if (flags & (RIGHT | CENTERED)) {
    int width = font->getTextWidth(s, len);
    if (flags & RIGHT) {
      INCREMENT_POS(-width);
    }
    else if (flags & CENTERED) {
      INCREMENT_POS(-width / 2);
    }
  }

  coord_t & pos = (flags & VERTICAL) ? y : x;
  const coord_t orig_pos = pos;

  auto curr = s;
  while (len == 0 || curr - s < len) {
    auto c = *curr++;

    if (!c) {
      break;
    }

    if (c == ' ') {
      INCREMENT_POS(font->getSpaceWidth());
      continue;
    }

    if (c == '\n') {
      pos = orig_pos;
      if (flags & VERTICAL)
        x += height;
      else
        y += height;
      continue;
    }

    auto glyph = font->getGlyph(c);
    // TRACE("c = '%lc' 0x%X offset=%d width=%d", c, c, glyph.offset, glyph.width);
    if (glyph.width) {
      drawChar(x, y, glyph, color);
      INCREMENT_POS(glyph.width + font->getSpacing());
    }
    else {
      TRACE("Missing glyph '%lc' hex=0x%X", c, c);
      INCREMENT_POS(font->getSpacing());
    }
  }

  RESTORE_OFFSET();

  return ((flags & RIGHT) ? orig_pos : pos) - offsetX;
}

char * numberToString(int32_t val, uint8_t len, const char * prefix, const char * suffix, LcdFlags flags)
{
  static char str[48+1]; // max=16 for the prefix, 16 chars for the number, 16 chars for the suffix
  char * s = str + 32;
  *s = '\0';
  int idx = 0;
  int prec = FLAGS_TO_DECIMALS(flags);
  bool neg = false;
  if (val < 0) {
    val = -val;
    neg = true;
  }
  do {
    *--s = '0' + (val % 10);
    ++idx;
    val /= 10;
    if (prec != 0 && idx == prec) {
      prec = 0;
      *--s = '.';
      if (val == 0)
        *--s = '0';
    }
  } while (val != 0 || prec > 0);
  if (neg) *--s = '-';

  // TODO needs check on all string lengths ...
  if (prefix) {
    int len = strlen(prefix);
    if (len <= 16) {
      s -= len;
      memcpy(s, prefix, len);
    }
  }
  if (suffix) {
    strncpy(&str[32], suffix, 16);
  }
  return s;
}

coord_t BitmapBuffer::drawNumber(coord_t x, coord_t y, int32_t val, LcdColor color, LcdFlags flags, uint8_t len, const char * prefix, const char * suffix)
{
  auto s = numberToString(val, len, prefix, suffix, flags);
  return drawText(x, y, s, color, flags);
}

//void BitmapBuffer::drawBitmapPie(int x0, int y0, const uint16_t * img, int startAngle, int endAngle)
//{
//  const uint16_t * q = img;
//  coord_t width = *q++;
//  coord_t height = *q++;
//
//  int slopes[4];
//  if (!evalSlopes(slopes, startAngle, endAngle))
//    return;
//
//  int w2 = width/2;
//  int h2 = height/2;
//
//  for (int y=h2-1; y>=0; y--) {
//    for (int x=w2-1; x>=0; x--) {
//      int slope = (x==0 ? 99000 : y*100/x);
//      if (slope >= slopes[0] && slope < slopes[1]) {
//        *getPixelPtr(x0+w2+x, y0+h2-y) = q[(h2-y)*width + w2+x];
//      }
//      if (-slope >= slopes[0] && -slope < slopes[1]) {
//        *getPixelPtr(x0+w2+x, y0+h2+y) = q[(h2+y)*width + w2+x];
//      }
//      if (slope >= slopes[2] && slope < slopes[3]) {
//        *getPixelPtr(x0+w2-x, y0+h2-y) = q[(h2-y)*width + w2-x];
//      }
//      if (-slope >= slopes[2] && -slope < slopes[3]) {
//        *getPixelPtr(x0+w2-x, y0+h2+y)  = q[(h2+y)*width + w2-x];
//      }
//    }
//  }
//}
//

Bitmap * Bitmap::load(const char * path, int maxSize)
{
  auto ext = getFileExtension(path);
  if (ext && !strcmp(ext, ".bmp"))
    return load_bmp(path, maxSize);
  else
    return load_stb(path, maxSize);
}

Mask * Mask::load(const char * path, int maxSize)
{
  auto bitmap = Bitmap::load(path, maxSize);
  if (bitmap) {
    Mask * result = Mask::allocate(bitmap->width(), bitmap->height());
    if (result) {
      auto * q = result->getData();
      for (const auto * p = bitmap->getData(); p < bitmap->getDataEnd(); p++) {
        *q++ = (ALPHA_MAX - ((*p) >> 12)) << 4;
      }
    }
    delete bitmap;
    return result;
  }
  return nullptr;
}

Mask * Mask::decodeRle(const uint8_t * data)
{
  auto width = *((uint16_t *)data);
  auto height = *((uint16_t *)(data + 2));

  auto result = Mask::allocate(width, height);

  uint8_t prevByte = 0;
  bool prevByteValid = false;

  auto d = result->getData();
  auto destDataEnd = result->getDataEnd();

  while (d < destDataEnd) {
    uint8_t byte = *data++;
    *d++ = byte;

    if (prevByteValid && byte == prevByte) {
      uint8_t count = *data++;

      if (d + count > destDataEnd) {
        TRACE("RLE decode error: destination overflow!");
        delete result;
        return nullptr;
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

  return result;
}

BitmapBuffer * BitmapBuffer::loadMaskOnBackground(const char * filename, Color565 foreground, Color565 background, int maxSize)
{
  BitmapBuffer * result = nullptr;
  const auto * mask = Mask::load(filename, maxSize);
  if (mask) {
    result = BitmapBuffer::allocate(BMP_RGB565, mask->width(), mask->height());
    if (result) {
      result->clear(background);
      result->drawMask(0, 0, mask, foreground);
    }
    delete mask;
  }
  return result;
}

Bitmap * Bitmap::load_bmp(const char * filename, int maxSize)
{
  uint8_t palette[16];
  
  FileReader fileReader(filename);
  auto dataSize = fileReader.size();
  if (maxSize >= 0 && (int)dataSize > maxSize) {
    TRACE("Bitmap::load(%s) failed: malloc refused", filename);
    return nullptr;
  }

  auto data = fileReader.read();
  if (!data) {
    TRACE("Bitmap::load(%s) failed: read error", filename);
    return nullptr;
  }

  auto buf = data;
  if (dataSize < 18 || buf[0] != 'B' || buf[1] != 'M') {
    return nullptr;
  }

  uint32_t fsize = UINT32LE(buf + 2);
  uint32_t hsize = UINT32LE(buf + 10); /* header size */
  uint32_t ihsize = UINT32LE(buf + 14); /* extra header size */

  /* invalid extra header size */
  if (ihsize + 14 > hsize) {
    return nullptr;
  }

  /* sometimes file size is set to some headers size, set a real size in that case */
  if (fsize == 14 || fsize == ihsize + 14) {
    fsize = dataSize - 2;
  }

  /* declared file size less than header size */
  if (fsize <= hsize) {
    return nullptr;
  }

  buf += 14;
  uint32_t w, h;

  switch (ihsize) {
    case 40: // windib
    case 56: // windib v3
    case 64: // OS/2 v2
    case 108: // windib v4
    case 124: // windib v5
      w = UINT32LE(buf + 4);
      h = UINT32LE(buf + 8);
      buf += 12;
      break;
    case 12: // OS/2 v1
      w = UINT16LE(buf + 4);
      h = UINT16LE(buf + 6);
      buf += 8;
      break;
    default:
      return nullptr;
  }

  if (UINT16LE(buf) != 1) { /* planes */
    return nullptr;
  }

  uint16_t depth = UINT16LE(buf + 2);

  if (depth == 4) {
    buf = data + hsize - 64;
    for (uint8_t i = 0; i < 16; i++) {
      palette[i] = buf[4 * i];
    }
  }

  buf = data + hsize;

  if (maxSize >= 0 && int(w * h * 2) > maxSize) {
    TRACE("Bitmap::load(%s) failed: malloc refused", filename);
    return nullptr;
  }

  auto bmp = Bitmap::allocate(BMP_RGB565, w, h);
  if (!bmp) {
    TRACE("Bitmap::load(%s) failed: malloc error", filename);
    return nullptr;
  }

  uint32_t rowSize;
  bool hasAlpha = false;

  switch (depth) {
    case 16:
      for (int i = h - 1; i >= 0; i--) {
        pixel_t * dest = bmp->getPixelPtrAbs(0, i);
        for (unsigned int j = 0; j < w; j++) {
          *dest = UINT16LE(buf);
          buf += 2;
          dest = bmp->getNextPixel(dest);
        }
        if (w & 1) {
          buf += 2;
        }
      }
      break;

    case 32:
      for (int i = h - 1; i >= 0; i--) {
        pixel_t * dest = bmp->getPixelPtrAbs(0, i);
        for (unsigned int j = 0; j < w; j++) {
          uint32_t pixel = UINT32LE(buf);
          buf += 4;
          // result = f_read(imgFile, (uint8_t *)&pixel, 4, &read);
          // if (result != FR_OK || read != 4) {
          //   f_close(imgFile);
          //   free(imgFile);
          //   delete bmp;
          //   return nullptr;
          // }
          if (hasAlpha) {
            *dest = ARGB4444((pixel >> 24) & 0xFF, (pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, (pixel >> 0) & 0xFF);
          }
          else {
            if ((pixel & 0xFF) == 0xFF) {
              *dest = RGB565(pixel >> 24, (pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF);
            }
            else {
              hasAlpha = true;
              bmp->setFormat(BMP_ARGB4444);
              for (pixel_t * p = bmp->getPixelPtrAbs(j, i); p != dest; p = bmp->getNextPixel(p)) {
                pixel_t tmp = *p;
                *p = ((tmp >> 1) & 0x0f) + (((tmp >> 7) & 0x0f) << 4) + (((tmp >> 12) & 0x0f) << 8);
              }
              *dest = ARGB4444(pixel & 0xFF, (pixel >> 24) & 0xFF, (pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF);
            }
          }
          dest = bmp->getNextPixel(dest);
        }
      }
      break;

    case 1:
      break;

    case 4:
      rowSize = ((4*w+31)/32)*4;
      for (int32_t i = h - 1; i >= 0; i--) {
        // result = f_read(imgFile, buf, rowSize, &read);
        // if (result != FR_OK || read != rowSize) {
        //   f_close(imgFile);
        //   free(imgFile);
        //   delete bmp;
        //   return nullptr;
        // }
        pixel_t * dest = bmp->getPixelPtrAbs(0, i);
        for (uint32_t j = 0; j < w; j++) {
          uint8_t index = (buf[j/2] >> ((j & 1) ? 0 : 4)) & 0x0F;
          uint8_t val = palette[index];
          *dest = RGB565(val, val, val);
          dest = bmp->getNextPixel(dest);
        }
        buf += rowSize;
      }
      break;

    default:
      delete bmp;
      return nullptr;
  }

  return bmp;
}

#define STBI_MALLOC(sz)                     stb_malloc(sz)
#define STBI_REALLOC_SIZED(p,oldsz,newsz)   stb_realloc(p,oldsz,newsz)
#define STBI_FREE(p)                        stb_free(p)

#if !defined(TRACE_STB_MALLOC)
#define TRACE_STB_MALLOC(...)
#endif

void * stb_malloc(unsigned int size)
{
#if defined(STB_MALLOC_MAXSIZE)
  if (size > STB_MALLOC_MAXSIZE) {
    TRACE("malloc size %d refused", size);
    return nullptr;
  }
#endif
  void * res = malloc(size);
  TRACE_STB_MALLOC("malloc %d = %p", size, res);
  return res;
}

void stb_free(void *ptr)
{
  TRACE_STB_MALLOC("free %p", ptr);
  free(ptr);
}

void * stb_realloc(void *ptr, unsigned int oldsz, unsigned int newsz)
{
#if defined(STB_MALLOC_MAXSIZE)
  if (newsz > STB_MALLOC_MAXSIZE) {
    TRACE("realloc size %d refused", newsz);
    return nullptr;
  }
#endif
  void * res = realloc(ptr, newsz);
  TRACE_STB_MALLOC("realloc %p, %d -> %d = %p", ptr, oldsz, newsz, res);
  return res;
}

#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#undef __I
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

Bitmap * Bitmap::load_stb(const char * filename, int maxSize)
{
  int w, h, n;
  unsigned char * img;

  {
    FileReader fileReader(filename);
    auto dataSize = fileReader.size();

    if (dataSize == 0) {
      return nullptr;
    }

    if (maxSize >= 0 && (int)dataSize > maxSize) {
      TRACE("Bitmap::load(%s) failed: malloc refused", filename);
      return nullptr;
    }

    auto data = fileReader.read();
    if (!data) {
      TRACE("Bitmap::load(%s) failed: read error", filename);
      return nullptr;
    }

    img = stbi_load_from_memory(data, dataSize, &w, &h, &n, 4);
    if (!img) {
      TRACE("Bitmap::load(%s) failed: %s", filename, stbi_failure_reason());
      return nullptr;
    }

    if (maxSize >= 0 && w * h * 2 > maxSize) {
      TRACE("Bitmap::load(%s) malloc not allowed", filename);
      stbi_image_free(img);
      return nullptr;
    }
  }

  // convert to RGB565 or ARGB4444 format
  auto bmp = Bitmap::allocate(n == 4 ? BMP_ARGB4444 : BMP_RGB565, w, h);
  if (!bmp) {
    TRACE("Bitmap::load(%s) malloc failed", filename);
    stbi_image_free(img);
    return nullptr;
  }

#if 0
  DMABitmapConvert(bmp->data, img, w, h, n == 4 ? DMA2D_ARGB4444 : DMA2D_RGB565);
#elif LCD_ORIENTATION == 270
  const uint8_t * p = img;
  if (n == 4) {
    for (int row = 0; row < h; ++row) {
      pixel_t * dest = bmp->getPixelPtrAbs(0, row);
      for (int col = 0; col < w; ++col) {
        *dest = ARGB4444(p[3], p[0], p[1], p[2]);
        dest += bmp->height();
        p += 4;
      }
    }
  }
  else {
    for (int row = 0; row < h; ++row) {
      pixel_t * dest = bmp->getPixelPtrAbs(0, row);
      for (int col = 0; col < w; ++col) {
        *dest = RGB565(p[0], p[1], p[2]);
        dest += bmp->height();
        p += 4;
      }
    }
  }
#else
  pixel_t * dest = bmp->getPixelPtrAbs(0, 0);
  const uint8_t * p = img;
  if (n == 4) {
    for (int row = 0; row < h; ++row) {
      for (int col = 0; col < w; ++col) {
        *dest = ARGB4444(p[3], p[0], p[1], p[2]);
        dest = bmp->getNextPixel(dest);
        p += 4;
      }
    }
  }
  else {
    for (int row = 0; row < h; ++row) {
      for (int col = 0; col < w; ++col) {
        *dest = RGB565(p[0], p[1], p[2]);
        dest = bmp->getNextPixel(dest);
        p += 4;
      }
    }
  }
#endif

  stbi_image_free(img);
  return bmp;
}
