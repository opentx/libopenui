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

#include "menu.h"
#include "font.h"
#include "theme.h"
#include "layer.h"

using namespace ui;

void MenuBody::select(int index)
{
  selectedIndex = index;
  if (innerHeight > height()) {
    if (scrollPositionY + height() < MENUS_LINE_HEIGHT * (index + 1)) {
      setScrollPositionY(MENUS_LINE_HEIGHT * (index + 1) - height());
    }
    else if (scrollPositionY > MENUS_LINE_HEIGHT * index) {
      setScrollPositionY(MENUS_LINE_HEIGHT * index);
    }
  }

  if (lines[index].onSelect) {
    lines[index].onSelect();
  }

  invalidate();
}

#if defined(HARDWARE_KEYS)
void MenuBody::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_ROTARY_RIGHT) {
    if (!lines.empty()) {
      select(int((selectedIndex + 1) % lines.size()));
      onKeyPress();
    }
  }
  else if (event == EVT_ROTARY_LEFT) {
    if (!lines.empty()) {
      select(int(selectedIndex <= 0 ? lines.size() - 1 : selectedIndex - 1));
      onKeyPress();
    }
  }
  else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    if (!lines.empty()) {
      onKeyPress();
      if (selectedIndex < 0) {
        select(0);
      }
      else {
        auto menu = getParentMenu();
        if (menu->multiple) {
          lines[selectedIndex].onPress();
          menu->invalidate();
        }
        else {
          Layer::pop(menu);
          lines[selectedIndex].onPress();
          menu->deleteLater(); // called at the end in case onPress changes the closeHandler
        }
      }
    }
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    onKeyPress();
    if (onCancel) {
      onCancel();
    }
    Window::onEvent(event);
  }
  else {
    Window::onEvent(event);
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool MenuBody::onTouchEnd(coord_t /*x*/, coord_t y)
{
  Menu * menu = getParentMenu();
  int index = y / MENUS_LINE_HEIGHT;
  if (index < (int)lines.size()) {
    onKeyPress();
    if (menu->multiple) {
      if (selectedIndex == index && lines[index].onPress)
        lines[index].onPress();
      else
        select(index);
      menu->invalidate();
    }
    else {
      Layer::pop(menu);
      lines[index].onPress();
      menu->deleteLater(); // called at the end in case onPress changes the closeHandler
    }
  }
  return true;
}
#endif

#if !defined(IS_TRANSLATION_RIGHT_TO_LEFT)
  #define IS_TRANSLATION_RIGHT_TO_LEFT() false
#endif

void MenuBody::paint(BitmapBuffer * dc)
{
  dc->clear(MENU_BGCOLOR);

  for (unsigned i = 0; i < lines.size(); i++) {
    auto & line = lines[i];
    LcdColor color = MENU_COLOR;
    LcdColor iconColor = GREY(0xA0);
    if (selectedIndex == (int)i) {
      color = EDIT_COLOR;
      iconColor = EDIT_COLOR;
      if (FOCUS_COLOR != MENU_BGCOLOR) {
        dc->drawPlainFilledRectangle(0, i * MENUS_LINE_HEIGHT, width(), MENUS_LINE_HEIGHT, FOCUS_COLOR);
      }
    }
    if (line.drawLine) {
      line.drawLine(dc, 0, i * MENUS_LINE_HEIGHT, color);
    }
    else {
      const char * text = line.text.data();
      if (IS_TRANSLATION_RIGHT_TO_LEFT()) {
        dc->drawText(width() - MENUS_HORIZONTAL_PADDING, i * MENUS_LINE_HEIGHT + (MENUS_LINE_HEIGHT - getFontHeight(MENU_FONT)) / 2, text[0] == '\0' ? "---" : text, color, MENU_FONT | RIGHT);
      }
      else {
        if (line.icon) {
          dc->drawMask(MENUS_HORIZONTAL_PADDING, i * MENUS_LINE_HEIGHT + (MENUS_LINE_HEIGHT - line.icon->height()) / 2, line.icon, iconColor);
        }
        if (displayIcons)
          dc->drawText(MENUS_HORIZONTAL_PADDING + MENUS_ICON_WIDTH, i * MENUS_LINE_HEIGHT + (MENUS_LINE_HEIGHT - getFontHeight(MENU_FONT)) / 2, text[0] == '\0' ? "---" : text, color, MENU_FONT);
        else
          dc->drawText(MENUS_HORIZONTAL_PADDING, i * MENUS_LINE_HEIGHT + (MENUS_LINE_HEIGHT - getFontHeight(MENU_FONT)) / 2, text[0] == '\0' ? "---" : text, color, MENU_FONT);
      }
    }

    Menu * menu = getParentMenu();
    if (menu->multiple && line.isChecked) {
      theme->drawCheckBox(dc, line.isChecked(), IS_TRANSLATION_RIGHT_TO_LEFT() ? MENUS_HORIZONTAL_PADDING : width() - MENUS_HORIZONTAL_PADDING - CHECKBOX_WIDTH, i * MENUS_LINE_HEIGHT + (MENUS_LINE_HEIGHT - CHECKBOX_WIDTH) / 2, 0);
    }

    if (i > 0) {
      dc->drawPlainHorizontalLine(0, i * MENUS_LINE_HEIGHT - 1, width(), MENU_LINE_COLOR);
    }
  }
}

MenuWindowContent::MenuWindowContent(Menu * parent, bool footer):
  ModalWindowContent(parent, {(LCD_W - MIN_MENUS_WIDTH) / 2, (LCD_H - MIN_MENUS_WIDTH) / 2, MIN_MENUS_WIDTH, 0}),
  body(this, {0, 0, MIN_MENUS_WIDTH, 0})
{
  body.setFocus(SET_FOCUS_DEFAULT);
  if (footer) {
    this->footer = new FormGroup(this, {0, 0, MIN_MENUS_WIDTH, POPUP_FOOTER_HEIGHT}, FORM_NO_BORDER | FORM_FORWARD_FOCUS);
  }
}

#if defined(HARDWARE_KEYS)
void MenuWindowContent::onEvent(event_t event)
{
  if (event == EVT_KEY_BREAK(KEY_PAGE) && this->footer) {
    onKeyPress();
    if (body.hasFocus()) {
      footer->setFocus();
    }
    else {
      body.setFocus();
    }
  }
  else {
    Window::onEvent(event);
  }
}
#endif


void MenuWindowContent::paint(BitmapBuffer * dc)
{
  // the background
  dc->clear(MENU_BGCOLOR);

  // the title
  if (!title.empty()) {
    dc->drawText(MIN_MENUS_WIDTH / 2, (POPUP_HEADER_HEIGHT - getFontHeight(MENU_HEADER_FONT)) / 2, title.c_str(), DEFAULT_COLOR, CENTERED | MENU_HEADER_FONT);
    dc->drawPlainHorizontalLine(0, POPUP_HEADER_HEIGHT - 1, MIN_MENUS_WIDTH, MENU_LINE_COLOR);
  }
}

Menu::Menu(Window * parent, bool multiple, bool footer):
  ModalWindow(parent, true),
  content(createMenuWindow(this, footer)),
  multiple(multiple)
{
}

void Menu::updatePosition()
{
  auto headerHeight = content->title.empty() ? 0 : POPUP_HEADER_HEIGHT;
  auto footerHeight = content->footer ? POPUP_FOOTER_HEIGHT : 0;
  auto bodyHeight = limit<coord_t>(MENUS_MIN_HEIGHT, content->body.lines.size() * MENUS_LINE_HEIGHT - 1, MENUS_MAX_HEIGHT);
  content->setHeight(headerHeight + bodyHeight + footerHeight);
  content->setTop((LCD_H - content->height()) / 2 + MENUS_OFFSET_TOP);
  content->body.setTop(headerHeight);
  content->body.setHeight(bodyHeight);
  content->body.setInnerHeight(content->body.lines.size() * MENUS_LINE_HEIGHT - 1);
  if (content->footer) {
    content->footer->setTop(content->body.bottom());
  }
}

void Menu::setTitle(std::string text)
{
  content->setTitle(std::move(text));
  updatePosition();
}

void Menu::addLine(const std::string & text, const BitmapMask * mask, std::function<void()> onPress, std::function<void()> onSelect, std::function<bool()> isChecked)
{
  content->body.addLine(text, mask, std::move(onPress), std::move(onSelect), std::move(isChecked));
  if (content->width() < MAX_MENUS_WIDTH) {
    auto lineWidth = min(MAX_MENUS_WIDTH, getTextWidth(text.c_str(), 0, MENU_FONT) + 2 * MENUS_HORIZONTAL_PADDING);
    if (lineWidth > content->width()) {
      content->setWidth(lineWidth);
      content->body.setWidth(lineWidth);
    }
  }
  updatePosition();
}

void Menu::addCustomLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine, std::function<void()> onPress, std::function<void()> onSelect, std::function<bool()> isChecked)
{
  content->body.addCustomLine(std::move(drawLine), std::move(onPress), std::move(onSelect), std::move(isChecked));
  updatePosition();
}

void Menu::removeLines()
{
  content->body.removeLines();
  updatePosition();
}

#if defined(HARDWARE_KEYS)
void Menu::onEvent(event_t event)
{
  if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    deleteLater();
  }
  else if (event == EVT_KEY_BREAK(KEY_ENTER) && !multiple) {
    deleteLater();
  }
}
#endif
