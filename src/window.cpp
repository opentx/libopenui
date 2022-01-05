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

#include <algorithm>
#include "window.h"
#include "touch.h"

Window * Window::focusWindow = nullptr;
Window * Window::slidingWindow = nullptr;
std::list<Window *> Window::trash;

Window::Window(Window * parent, const rect_t & rect, WindowFlags windowFlags, LcdFlags textFlags):
  parent(parent),
  rect(rect),
  innerWidth(rect.w),
  innerHeight(rect.h),
  windowFlags(windowFlags),
  textFlags(textFlags)
{
  if (parent) {
    parent->addChild(this, windowFlags & PUSH_FRONT);
    if (!(windowFlags & TRANSPARENT)) {
      invalidate();
    }
  }
}

Window::~Window()
{
  TRACE_WINDOWS("Destroy %p %s", this, getWindowDebugString().c_str());

  if (focusWindow == this) {
    focusWindow = nullptr;
  }

  deleteChildren();
}

void Window::attach(Window * newParent)
{
  if (parent)
    detach();
  parent = newParent;
  newParent->addChild(this);
}

void Window::detach()
{
  if (parent) {
    parent->removeChild(this);
    parent = nullptr;
  }
}

void Window::deleteLater(bool detach, bool trash)
{
  if (_deleted)
    return;

  TRACE_WINDOWS("Delete %p %s", this, getWindowDebugString().c_str());

  _deleted = true;

  if (static_cast<Window *>(focusWindow) == static_cast<Window *>(this)) {
    focusWindow = nullptr;
  }

  if (detach)
    this->detach();
  else
    parent = nullptr;

  deleteChildren();

  if (closeHandler) {
    closeHandler();
  }

  if (trash) {
    Window::trash.push_back(this);
  }
}

void Window::clear()
{
  scrollPositionX = 0;
  scrollPositionY = 0;
  innerWidth = rect.w;
  innerHeight = rect.h;
  deleteChildren();
  invalidate();
}

void Window::deleteChildren()
{
  for (auto window: children) {
    window->deleteLater(false);
  }
  children.clear();
}

void Window::setFocus(uint8_t flag, Window * from)
{
  if (deleted())
    return;

  TRACE_WINDOWS("%s setFocus()", getWindowDebugString().c_str());

  if (focusWindow != this) {
    // scroll before calling focusHandler so that the window can adjust the scroll position if needed
    Window * parent = this->parent;
    while (parent && parent->getWindowFlags() & FORWARD_SCROLL) {
      parent = parent->parent;
    }
    if (parent) {
      parent->scrollTo(this);
      invalidate();
    }

    clearFocus();
    focusWindow = this;
    if (focusHandler) {
      focusHandler(true);
    }
  }
}

void Window::setScrollPositionX(coord_t value)
{
  auto newScrollPosition = max<coord_t>(0, min<coord_t>(innerWidth - width(), value));
  if (newScrollPosition != scrollPositionX) {
    scrollPositionX = newScrollPosition;
    invalidate();
  }
}

void Window::setScrollPositionY(coord_t value)
{
  auto newScrollPosition = min<coord_t>(innerHeight - height(), value);

  if (newScrollPosition < 0 && innerHeight != INFINITE_HEIGHT) {
    newScrollPosition = 0;
  }

  if (newScrollPosition != scrollPositionY) {
    scrollPositionY = newScrollPosition;
    invalidate();
  }
}

void Window::scrollTo(Window * child, bool bottom)
{
  TRACE_WINDOWS("%s scrollTo(%s)", getWindowDebugString().c_str(), child->getWindowDebugString().c_str());

  coord_t offsetX = 0;
  coord_t offsetY = 0;

  Window * parentWindow = child->getParent();
  while (parentWindow && parentWindow != this) {
    offsetX += parentWindow->left();
    offsetY += parentWindow->top();
    parentWindow = parentWindow->getParent();
  }

  const rect_t scrollRect = {
    offsetX + child->left(),
    offsetY + (bottom ? child->bottom() : child->top()),
    min(child->width(), width()),
    min(child->height(), bottom ? 0 : height())
  };

  scrollTo(scrollRect);
}

void Window::scrollTo(const rect_t & rect)
{
  if (rect.top() < scrollPositionY) {
    setScrollPositionY(pageHeight ? rect.top() - (rect.top() % pageHeight) : rect.top() - 5);
  }
  else if (rect.bottom() > scrollPositionY + height() - 5) {
    setScrollPositionY(pageHeight ? rect.top() - (rect.top() % pageHeight) : rect.bottom() - height() + 5);
  }

  if (rect.left() < scrollPositionX) {
    setScrollPositionX(pageWidth ? rect.left() - (rect.left() % pageWidth) : rect.left() - 5);
  }
  else if (rect.right() > scrollPositionX + width() - 5) {
    setScrollPositionX(pageWidth ? rect.left() - (rect.left() % pageWidth) : rect.right() - width() + 5);
  }
}

void Window::scrollToPage(unsigned pageIndex)
{
  scrollTo({coord_t(pageWidth * pageIndex), 0, pageWidth, 0});
}

bool Window::hasOpaqueRect(const rect_t & testRect) const
{
  if (!rect.contains(testRect))
    return false;

  if (windowFlags & OPAQUE) {
    return true;
  }

  const rect_t relativeRect = {testRect.x - rect.x + getScrollPositionX(), testRect.y - rect.y + getScrollPositionY(), testRect.w, testRect.h};
  return std::any_of(children.begin(), children.end(), [relativeRect](Window * child) { return child->hasOpaqueRect(relativeRect); });
}

void Window::fullPaint(BitmapBuffer * dc)
{
  bool paintNeeded = true;
  std::list<Window *>::iterator firstChild;

  coord_t xmin, xmax, ymin, ymax;
  dc->getClippingRect(xmin, xmax, ymin, ymax);
  coord_t x = dc->getOffsetX();
  coord_t y = dc->getOffsetY();

  if (windowFlags & PAINT_CHILDREN_FIRST) {
    paintChildren(dc, children.begin());
    dc->setOffset(x, y);
    dc->setClippingRect(xmin, xmax, ymin, ymax);
  }
  else {
    firstChild = children.end();
    rect_t relativeRect = {xmin - x, ymin - y, xmax - xmin, ymax - ymin};
    while (firstChild != children.begin()) {
      auto child = *(--firstChild);
      if (child->hasOpaqueRect(relativeRect)) {
        paintNeeded = false;
        break;
      }
    }
  }

  if (paintNeeded) {
    TRACE_WINDOWS_INDENT("%s%s", getWindowDebugString().c_str(), hasFocus() ? " (*)" : "");
    paint(dc);
#if defined(WINDOWS_INSPECT_BORDER_COLOR)
    dc->drawSolidRect(0, 0, width(), height(), WINDOWS_INSPECT_BORDER_COLOR, 1);
#endif
  }
  else {
    TRACE_WINDOWS_INDENT("%s (skipped)", getWindowDebugString().c_str());
  }

  if (!(windowFlags & NO_SCROLLBAR)) {
    drawVerticalScrollbar(dc);
  }

  if (!(windowFlags & PAINT_CHILDREN_FIRST)) {
    paintChildren(dc, firstChild);
  }
}

bool Window::isChildFullSize(const Window * child) const
{
  return child->top() == 0 && child->height() == height() && child->left() == 0 && child->width() == width();
}

bool Window::isChildVisible(const Window * window) const
{
  for (auto rit = children.rbegin(); rit != children.rend(); rit++) {
    auto child = *rit;
    if (child == window) {
      return true;
    }
    if ((child->getWindowFlags() & OPAQUE) & isChildFullSize(child)) {
      return false;
    }
  }
  return false;
}

void Window::setInsideParentScrollingArea(bool bottom)
{
  Window * parent = getParent();
  while (parent && parent->getWindowFlags() & FORWARD_SCROLL) {
    parent = parent->parent;
  }
  if (parent) {
    parent->scrollTo(this, bottom);
    invalidate();
  }
}

void Window::paintChildren(BitmapBuffer * dc, std::list<Window *>::iterator it)
{
  coord_t x = dc->getOffsetX();
  coord_t y = dc->getOffsetY();
  coord_t xmin, xmax, ymin, ymax;
  dc->getClippingRect(xmin, xmax, ymin, ymax);

  for (; it != children.end(); it++) {
    auto child = *it;

    coord_t child_xmin = x + child->rect.x;
    if (child_xmin >= xmax)
      continue;
    coord_t child_ymin = y + child->rect.y;
    if (child_ymin >= ymax)
      continue;
    coord_t child_xmax = child_xmin + child->rect.w;
    if (child_xmax <= xmin)
      continue;
    coord_t child_ymax = child_ymin + child->rect.h;
    if (child_ymax <= ymin)
      continue;

    dc->setOffset(x + child->rect.x - child->scrollPositionX, y + child->rect.y - child->scrollPositionY);
    dc->setClippingRect(max(xmin, x + child->rect.left()),
                        min(xmax, x + child->rect.right()),
                        max(ymin, y + child->rect.top()),
                        min(ymax, y + child->rect.bottom()));
    child->fullPaint(dc);
  }
}

#if defined(HARDWARE_TOUCH)
coord_t Window::getSnapStep(coord_t relativeScrollPosition, coord_t pageSize)
{
  coord_t result = 0;
  if (relativeScrollPosition > pageSize / 2) {
    // closer to next page
    result = (pageSize - relativeScrollPosition);
  }
  else {
    // closer to previous page
    result = (0 - relativeScrollPosition);
  }

  // do not get too slow
  if (abs(result) > 32)
    result /= 2;

  return result;
}
#endif

void Window::checkEvents()
{
  auto copy = children;
  for (auto child: copy) {
    if (!child->deleted()) {
      child->checkEvents();
    }
  }

  if (this == Window::focusWindow) {
    event_t event = getWindowEvent();
    if (event) {
      this->onEvent(event);
    }
  }

  if (windowFlags & REFRESH_ALWAYS) {
    invalidate();
  }

#if defined(HARDWARE_TOUCH)
  if (!touchState.isScrolling()) {
    if (pageWidth) {
      coord_t relativeScrollPosition = getScrollPositionX() % pageWidth;
      if (relativeScrollPosition) {
        setScrollPositionX(getScrollPositionX() + getSnapStep(relativeScrollPosition, pageWidth));
      }
    }
    if (pageHeight) {
      coord_t relativeScrollPosition = getScrollPositionY() % pageHeight;
      if (relativeScrollPosition) {
        setScrollPositionY(getScrollPositionY() + getSnapStep(relativeScrollPosition, pageHeight));
      }
    }
  }
#endif
}

void Window::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", Window::getWindowDebugString("Window").c_str(), event);
  if (parent) {
    parent->onEvent(event);
  }
}

#if defined(HARDWARE_TOUCH)
bool Window::onTouchStart(coord_t x, coord_t y)
{
  TRACE_WINDOWS("%s touch start", Window::getWindowDebugString("Window").c_str());

  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    auto child = *it;
    if (child->rect.contains(x, y)) {
      if (child->onTouchStart(x - child->rect.x + child->scrollPositionX, y - child->rect.y + child->scrollPositionY)) {
        return true;
      }
    }
  }

  return windowFlags & OPAQUE;
}

bool Window::onTouchLong(coord_t x, coord_t y)
{
  TRACE_WINDOWS("%s touch long", Window::getWindowDebugString("Window").c_str());

  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    auto child = *it;
    if (child->rect.contains(x, y)) {
      if (child->onTouchLong(x - child->rect.x + child->scrollPositionX, y - child->rect.y + child->scrollPositionY)) {
        return true;
      }
    }
  }

  return windowFlags & OPAQUE;
}

bool Window::forwardTouchEnd(coord_t x, coord_t y)
{
  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    auto child = *it;
    if (child->rect.contains(x, y)) {
      if (child->onTouchEnd(x - child->rect.x + child->scrollPositionX, y - child->rect.y + child->scrollPositionY)) {
        return true;
      }
    }
  }

  return false;
}

bool Window::onTouchEnd(coord_t x, coord_t y)
{
  TRACE_WINDOWS("%s touch end", Window::getWindowDebugString("Window").c_str());

  return forwardTouchEnd(x, y) ? true : (windowFlags & OPAQUE);
}

bool Window::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  startX += getScrollPositionX();
  startY += getScrollPositionY();

  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    auto child = *it;
    if (child->rect.contains(startX, startY)) {
      if (child->onTouchSlide(x - child->rect.x, y - child->rect.y, startX - child->rect.x, startY - child->rect.y, slideX, slideY)) {
        return true;
      }
    }
  }

  if (!scrollEnabled) {
    return false;
  }

  if (slidingWindow && slidingWindow != this) {
    return false;
  }

  if (slideY && innerHeight > rect.h) {
    if (touchState.isScrollingByInertia() && (scrollPositionY == 0 || scrollPositionY == innerHeight - height())) {
      touchState.stopInertia();
    }
    else {
      setScrollPositionY(scrollPositionY - slideY);
      slidingWindow = this;
      touchState.lastDeltaX = touchState.deltaX;
      touchState.lastDeltaY = touchState.deltaY;
      touchState.deltaX = 0;
      touchState.deltaY = 0;
    }
    return true;
  }

  if (slideX && innerWidth > rect.w) {
    if (touchState.isScrollingByInertia() && (scrollPositionX == 0 || scrollPositionX == innerWidth - width())) {
      touchState.stopInertia();
    }
    else {
      setScrollPositionX(scrollPositionX - slideX);
      slidingWindow = this;
      touchState.lastDeltaX = touchState.deltaX;
      touchState.lastDeltaY = touchState.deltaY;
      touchState.deltaX = 0;
      touchState.deltaY = 0;
    }
    return true;
  }

  return false;
}
#endif

void Window::adjustInnerHeight()
{
  coord_t bottomMax = 0;
  for (auto child: children) {
    bottomMax = max(bottomMax, child->rect.y + child->rect.h);
  }
  setInnerHeight(bottomMax);
}

coord_t Window::adjustHeight()
{
  coord_t old = rect.h;
  adjustInnerHeight();
  rect.h = innerHeight;
  return rect.h - old;
}

void Window::moveWindowsTop(coord_t y, coord_t delta)
{
  if (getWindowFlags() & FORWARD_SCROLL) {
    parent->moveWindowsTop(bottom(), delta);
  }

  for (auto child: children) {
    if (child->rect.y >= y) {
      child->rect.y += delta;
      invalidate();
    }
  }
  setInnerHeight(innerHeight + delta);
}

void Window::invalidate(const rect_t & rect)
{
  if (isVisible()) {
    parent->invalidate({this->rect.x + rect.x - parent->scrollPositionX, this->rect.y + rect.y - parent->scrollPositionY, rect.w, rect.h});
  }
}

void Window::drawVerticalScrollbar(BitmapBuffer * dc)
{
  if (innerHeight > rect.h) {
    coord_t yofs = divRoundClosest(rect.h * scrollPositionY, innerHeight);
    coord_t yhgt = divRoundClosest(rect.h * rect.h, innerHeight);
    if (yhgt < 15)
      yhgt = 15;
    if (yhgt + yofs > rect.h)
      yhgt = rect.h - yofs;
    dc->drawSolidFilledRect(rect.w - SCROLLBAR_WIDTH, scrollPositionY + yofs, SCROLLBAR_WIDTH, yhgt, SCROLLBAR_COLOR);
  }
}

void Window::drawHorizontalScrollbar(BitmapBuffer * dc)
{
  if (innerWidth > rect.w) {
    coord_t xofs = divRoundClosest(rect.w * scrollPositionX, innerWidth);
    coord_t xwdth = divRoundClosest(rect.w * rect.w, innerWidth);
    if (xwdth < 15)
      xwdth = 15;
    if (xwdth + xofs > rect.w)
      xwdth = rect.w - xofs;
    dc->drawSolidFilledRect(scrollPositionX + xofs, rect.h - SCROLLBAR_WIDTH, xwdth, SCROLLBAR_WIDTH, SCROLLBAR_COLOR);
  }
}
