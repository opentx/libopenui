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

#include "mainwindow.h"
#include "keyboard_base.h"

#if defined(HARDWARE_TOUCH)
#include "touch.h"
#endif

MainWindow * MainWindow::_instance = nullptr;

#if defined(HARDWARE_TOUCH)
TouchState touchState;
Keyboard * Keyboard::activeKeyboard = nullptr;
#endif

void MainWindow::emptyTrash()
{
  for (auto window: trash) {
    delete window;
  }
  trash.clear();
}

void MainWindow::checkEvents()
{
#if defined(HARDWARE_TOUCH)
  auto touchEvent = touchState.popEvent();

  if (touchEvent == TE_DOWN) {
    onTouchStart(touchState.x + scrollPositionX, touchState.y + scrollPositionY);
    slidingWindow = nullptr;
  }
  else if (touchEvent == TE_LONG) {
    onTouchLong(touchState.x + scrollPositionX, touchState.y + scrollPositionY);
    slidingWindow = nullptr;
  }
  else if (touchEvent == TE_UP) {
    onTouchEnd(touchState.startX + scrollPositionX, touchState.startY + scrollPositionY);
  }
  else if (touchEvent == TE_SLIDE) {
    if (touchState.deltaX || touchState.deltaY) {
      onTouchSlide(touchState.x, touchState.y, touchState.startX, touchState.startY, touchState.deltaX, touchState.deltaY);
      touchState.lastDeltaX = touchState.deltaX;
      touchState.lastDeltaY = touchState.deltaY;
      touchState.deltaX = 0;
      touchState.deltaY = 0;
    }
  }
  else if (touchEvent == TE_SLIDE_END && slidingWindow) {
    if (touchState.lastDeltaX > SLIDE_SPEED_REDUCTION)
      touchState.lastDeltaX -= SLIDE_SPEED_REDUCTION;
    else if (touchState.lastDeltaX < -SLIDE_SPEED_REDUCTION)
      touchState.lastDeltaX += SLIDE_SPEED_REDUCTION;
    else
      touchState.lastDeltaX = 0;
    if (touchState.lastDeltaY > SLIDE_SPEED_REDUCTION)
      touchState.lastDeltaY -= SLIDE_SPEED_REDUCTION;
    else if (touchState.lastDeltaY < -SLIDE_SPEED_REDUCTION)
      touchState.lastDeltaY += SLIDE_SPEED_REDUCTION;
    else
      touchState.lastDeltaY = 0;
    if (touchState.lastDeltaX || touchState.lastDeltaY) {
      onTouchSlide(touchState.x, touchState.y, touchState.startX, touchState.startY, touchState.lastDeltaX, touchState.lastDeltaY);
    }
  }
#endif

  Window::checkEvents();
}

void MainWindow::invalidate(const rect_t & rect)
{
  if (invalidatedRect.w) {
    auto left = limit<coord_t>(0, rect.left(), invalidatedRect.left());
    auto right = limit<coord_t>(invalidatedRect.right(), rect.right(), LCD_W);
    auto top = limit<coord_t>(0, rect.top(), invalidatedRect.top());
    auto bottom = limit<coord_t>(rect.bottom(), invalidatedRect.bottom(), LCD_H);
    invalidatedRect = {left, top, right - left, bottom - top};
  }
  else {
    invalidatedRect = rect;
  }
}

bool MainWindow::refresh()
{
  if (invalidatedRect.w) {
    if (invalidatedRect.x > 0 || invalidatedRect.y > 0 || invalidatedRect.w < LCD_W || invalidatedRect.h < LCD_H) {
      TRACE_WINDOWS("Refresh rect: left=%d top=%d width=%d height=%d", invalidatedRect.left(), invalidatedRect.top(), invalidatedRect.w, invalidatedRect.h);
      const BitmapBuffer * previous = lcd;
      lcdNextLayer();
      lcdCopy(lcd->getData(), previous->getData());
    }
    else {
      TRACE_WINDOWS("Refresh full screen");
      lcdNextLayer();
    }
    lcd->setOffset(0, 0);
    lcd->setClippingRect(invalidatedRect.left(), invalidatedRect.right(), invalidatedRect.top(), invalidatedRect.bottom());
    fullPaint(lcd);
    invalidatedRect.w = 0;
    return true;
  }
  else {
    return false;
  }
}

void MainWindow::run(bool trash)
{
  auto start = ticksNow();

  checkEvents();

  if (trash) {
    emptyTrash();
  }
  
  if (refresh()) {
    lcdRefresh();
  }

  auto delta = ticksNow() - start;
  if (delta > 10 * SYSTEM_TICKS_1MS) {
    TRACE_WINDOWS("MainWindow::run took %dms", (ticksNow() - start) / SYSTEM_TICKS_1MS);
  }
}
