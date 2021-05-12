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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <list>
#include <string>
#include <utility>
#include <functional>
#include "bitmapbuffer.h"
#include "libopenui_defines.h"
#include "libopenui_helpers.h"
#include "libopenui_config.h"

typedef uint32_t WindowFlags;

#if !defined(_GNUC_)
  #undef OPAQUE
  #undef TRANSPARENT
#endif

constexpr int INFINITE_HEIGHT = INT32_MAX;

constexpr WindowFlags OPAQUE =                1u << 0u;
constexpr WindowFlags TRANSPARENT =           1u << 1u;
constexpr WindowFlags NO_SCROLLBAR =          1u << 2u;
constexpr WindowFlags NO_FOCUS =              1u << 3u;
constexpr WindowFlags FORWARD_SCROLL =        1u << 4u;
constexpr WindowFlags REFRESH_ALWAYS =        1u << 5u;
constexpr WindowFlags PAINT_CHILDREN_FIRST =  1u << 6u;
constexpr WindowFlags PUSH_FRONT =  1u << 7u;
constexpr WindowFlags WINDOW_FLAGS_LAST =  PUSH_FRONT;

enum SetFocusFlag
{
  SET_FOCUS_DEFAULT,
  SET_FOCUS_FORWARD,
  SET_FOCUS_BACKWARD,
  SET_FOCUS_FIRST
};

class Window
{
  friend class GridLayout;

  public:
    Window(Window * parent, const rect_t & rect, WindowFlags windowFlags = 0, LcdFlags textFlags = 0);

    virtual ~Window();

#if defined(DEBUG_WINDOWS)
    virtual std::string getName() const
    {
      return "Window";
    }

    std::string getRectString() const
    {
      char result[32];
      sprintf(result, "[%d, %d, %d, %d]", left(), top(), width(), height());
      return result;
    }

    std::string getIndentString() const
    {
      std::string result;
      auto tmp = parent;
      while (tmp) {
        result += "  ";
        tmp = tmp->getParent();
      }
      return result;
    }

    std::string getWindowDebugString(const char * name = nullptr) const
    {
      return getName() + (name ? std::string(" [") + name + "] " : " ") + getRectString();
    }
#endif

    Window * getParent() const
    {
      return parent;
    }

    bool isChild(Window * window) const
    {
      return window == this || (parent && parent->isChild(window));
    }

    Window * getFullScreenWindow()
    {
      return (width() == LCD_W && height() == LCD_H) ? this : parent->getFullScreenWindow();
    }

    WindowFlags getWindowFlags() const
    {
      return windowFlags;
    }

    void setWindowFlags(WindowFlags flags)
    {
      windowFlags = flags;
    }

    LcdFlags getTextFlags() const
    {
      return textFlags;
    }

    void setTextFlags(LcdFlags flags)
    {
      textFlags = flags;
    }

    void setCloseHandler(std::function<void()> handler)
    {
      closeHandler = std::move(handler);
    }

    void setFocusHandler(std::function<void(bool)> handler)
    {
      focusHandler = std::move(handler);
    }

    virtual void deleteLater(bool detach = true, bool trash = true);

    void clear();

    void deleteChildren();

    bool hasFocus() const
    {
      return focusWindow == this;
    }

    static Window * getFocus()
    {
      return focusWindow;
    }

    void scrollTo(const rect_t & rect);

    void scrollTo(Window * child);

    static void clearFocus();

    virtual void setFocus(uint8_t flag = SET_FOCUS_DEFAULT, Window * from = nullptr);

    void setRect(rect_t value)
    {
      rect = value;
      invalidate();
    }

    void setWidth(coord_t value)
    {
      rect.w = value;
      invalidate();
    }

    void setWindowCentered()
    {
      rect.x = (parent->width() - width()) / 2;
      rect.y = (parent->height() - height()) / 2;
    }

    void setHeight(coord_t value)
    {
      rect.h = value;
      if (windowFlags & FORWARD_SCROLL)
        innerHeight = value;
      else if (innerHeight <= value) {
        setScrollPositionY(0);
      }
      invalidate();
    }

    void setLeft(coord_t x)
    {
      rect.x = x;
      invalidate();
    }

    void setTop(coord_t y)
    {
      rect.y = y;
      invalidate();
    }

    coord_t left() const
    {
      return rect.x;
    }

    coord_t right() const
    {
      return rect.x + rect.w;
    }

    coord_t top() const
    {
      return rect.y;
    }

    coord_t bottom() const
    {
      return rect.y + rect.h;
    }

    coord_t width() const
    {
      return rect.w;
    }

    coord_t height() const
    {
      return rect.h;
    }

    rect_t getRect() const
    {
      return rect;
    }

    coord_t getInnerWidth() const
    {
      return innerWidth;
    }

    void setInnerWidth(coord_t w)
    {
      innerWidth = w;
      if (width() >= w) {
        scrollPositionX = 0;
      }
    }

    void setPageWidth(coord_t w)
    {
      pageWidth = w;
    }

    void setPageHeight(coord_t h)
    {
      pageHeight = h;
    }

    uint8_t getPageCount() const
    {
      if (pageWidth)
        return innerWidth / pageWidth;
      else if (pageHeight)
        return innerHeight / pageHeight;
      else
        return 1;
    }

    uint8_t getPageIndex() const
    {
      if (pageWidth)
        return (getScrollPositionX() + (pageWidth / 2)) / pageWidth;
      else if (pageHeight)
        return (getScrollPositionY() + (pageHeight / 2)) / pageHeight;
      else
        return 0;
    }

    coord_t getInnerHeight() const
    {
      return innerHeight;
    }

    void setInnerHeight(coord_t h)
    {
      innerHeight = h;
      if (windowFlags & FORWARD_SCROLL) {
        rect.h = innerHeight;
        parent->adjustInnerHeight();
      }
      else if (height() >= h) {
        setScrollPositionY(0);
      }
      else {
        coord_t maxScrollPosition = h - height();
        if (scrollPositionY > maxScrollPosition) {
          setScrollPositionY(maxScrollPosition);
        }
      }
      invalidate();
    }

    coord_t getScrollPositionX() const
    {
      return scrollPositionX;
    }

    coord_t getScrollPositionY() const
    {
      return scrollPositionY;
    }

    virtual void setScrollPositionX(coord_t value);

    virtual void setScrollPositionY(coord_t value);

    bool isChildVisible(const Window * window) const;

    bool isChildFullSize(const Window * window) const;

    bool isVisible() const
    {
      return parent && parent->isChildVisible(this);
    }

    bool isInsideParentScrollingArea() const
    {
      return parent && right() >= parent->getScrollPositionX() && left() <= parent->getScrollPositionX() + parent->width();
    }

    void setInsideParentScrollingArea();

    void drawVerticalScrollbar(BitmapBuffer * dc);

    void drawHorizontalScrollbar(BitmapBuffer * dc);

    virtual void onEvent(event_t event);

    void adjustInnerHeight();

    coord_t adjustHeight();

    void moveWindowsTop(coord_t y, coord_t delta);

    void invalidate()
    {
      invalidate({0, 0, rect.w, rect.h});
    }

    void bringToTop()
    {
      attach(parent); // does a detach + attach
    }

    virtual void checkEvents();

    void attach(Window * window);

    void detach();

    bool deleted() const
    {
      return _deleted;
    }

  protected:
    Window * parent;
    std::list<Window *> children;
    rect_t rect;
    coord_t innerWidth;
    coord_t innerHeight;
    coord_t pageWidth = 0;
    coord_t pageHeight = 0;
    coord_t scrollPositionX = 0;
    coord_t scrollPositionY = 0;
    WindowFlags windowFlags;
    LcdFlags textFlags;
    bool _deleted = false;

    static Window * focusWindow;
    static Window * slidingWindow;
    static std::list<Window *> trash;

    std::function<void()> closeHandler;
    std::function<void(bool)> focusHandler;

    void addChild(Window * window, bool front = false)
    {
      if (front)
        children.push_front(window);
      else
        children.push_back(window);
    }

    void removeChild(Window * window)
    {
      children.remove(window);
      invalidate();
    }

    virtual void invalidate(const rect_t & rect);

    void paintChildren(BitmapBuffer * dc, std::list<Window *>::iterator it);

    void fullPaint(BitmapBuffer * dc);

    virtual void paint(BitmapBuffer *)
    {
    }

    virtual void onFocusLost()
    {
      TRACE_WINDOWS("%s onFocusLost()", getWindowDebugString().c_str());
      if (focusHandler) {
        focusHandler(false);
      }
      invalidate();
    }

#if defined(HARDWARE_TOUCH)
    static coord_t getSnapStep(coord_t relativeScrollPosition, coord_t pageSize);

    virtual bool onTouchStart(coord_t x, coord_t y);

    virtual bool onTouchLong(coord_t x, coord_t y);

    virtual bool onTouchEnd(coord_t x, coord_t y);

    virtual bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY);
#endif

    bool forwardTouchEnd(coord_t x, coord_t y);

    bool hasOpaqueRect(const rect_t & testRect) const;
};

