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

#include <vector>
#include <functional>
#include <utility>
#include "modal_window.h"
#include "form.h"

constexpr coord_t MENUS_HORIZONTAL_PADDING = 10;

namespace ui {

class Menu;
class MenuWindowContent;

class MenuBody: public Window
{
  friend class Menu;

  class MenuLine
  {
    friend class MenuBody;

    public:
      MenuLine(std::string text, const Mask * icon, std::function<void()> onPress, std::function<void()> onSelect, std::function<bool()> isChecked):
        text(std::move(text)),
        icon(icon),
        onPress(std::move(onPress)),
        onSelect(std::move(onSelect)),
        isChecked(std::move(isChecked))
      {
      }

      MenuLine(std::function<void(BitmapBuffer * /*dc*/, coord_t /*x*/, coord_t /*y*/, LcdColor /*color*/)> drawLine, std::function<void()> onPress, std::function<void()> onSelect, std::function<bool()> isChecked):
        drawLine(std::move(drawLine)),
        onPress(std::move(onPress)),
        onSelect(std::move(onSelect)),
        isChecked(std::move(isChecked))
      {
      }

      MenuLine(MenuLine &) = delete;

      MenuLine(MenuLine &&) = default;

    protected:
      std::string text;
      const Mask * icon;
      std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdColor color)> drawLine;
      std::function<void()> onPress;
      std::function<void()> onSelect;
      std::function<bool()> isChecked;
  };

  public:
    MenuBody(Window * parent, const rect_t & rect, bool multiple):
      Window(parent, rect, OPAQUE),
      multiple(multiple)
    {
      setPageHeight(MENUS_LINE_HEIGHT);
    }

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "MenuBody";
    }
#endif

    void select(int index);

    int selection() const
    {
      return selectedIndex;
    }

    int count() const
    {
      return lines.size();
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    void addLine(const std::string & text, const Mask * icon = nullptr, std::function<void()> onPress = nullptr, std::function<void()> onSelect = nullptr, std::function<bool()> isChecked = nullptr)
    {
      lines.emplace_back(text, icon, std::move(onPress), std::move(onSelect), std::move(isChecked));
      if (icon)
        displayIcons = true;
      invalidate();
    }

    void addCustomLine(std::function<void(BitmapBuffer * /*dc*/, coord_t /*x*/, coord_t /*y*/, LcdFlags /*flags*/)> drawLine, std::function<void()> onPress, std::function<void()> onSelect = nullptr, std::function<bool()> isChecked = nullptr)
    {
      lines.emplace_back(std::move(drawLine), std::move(onPress), std::move(onSelect), std::move(isChecked));
      invalidate();
    }

    void removeLines()
    {
      lines.clear();
      invalidate();
    }

    void setCancelHandler(std::function<bool()> handler)
    {
      onCancel = std::move(handler);
    }

    void setDefaultSelection(int index)
    {
      defaultSelection = index;
    }

    int getDefaultSelection()
    {
      return defaultSelection;
    }

    void setAutoClose(bool value)
    {
      autoClose = value;
    }

    void paint(BitmapBuffer * dc) override;

    std::vector<MenuLine> lines;

  protected:
#if defined(HARDWARE_TOUCH)
    int selectedIndex = -1;
#else
    int selectedIndex = 0;
#endif
    int defaultSelection = 0;
    std::function<bool()> onCancel;
    bool displayIcons = false;
    bool autoClose = true;
    bool multiple = false;

    inline Window * getParentMenu()
    {
      return getParent()->getParent();
    }
};

class MenuWindowContent: public ModalWindowContent
{
  friend class Menu;

  public:
    explicit MenuWindowContent(ModalWindow * parent, const rect_t & rect, bool multiple, bool footer);

    void deleteLater(bool detach = true, bool trash = true) override
    {
      if (deleted())
        return;

      body.deleteLater(true, false);
      ModalWindowContent::deleteLater(detach, trash);
    }

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "MenuWindowContent";
    }
#endif

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

    void paint(BitmapBuffer * dc) override;

    MenuBody * getBody()
    {
      return &body;
    }

    FormGroup * getFooter() const
    {
      return footer;
    }

  protected:
    MenuBody body;
    FormGroup * footer = nullptr;
};

class Menu: public ModalWindow
{
  public:
    explicit Menu(Window * parent, bool multiple = false, bool footer = false);

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "Menu";
    }
#endif
    
    void setCancelHandler(std::function<bool()> handler)
    {
      content->body.setCancelHandler(std::move(handler));
    }

    void setWaitHandler(std::function<void()> handler)
    {
      waitHandler = std::move(handler);
    }

    void setTitle(std::string text);

    void addLine(const std::string & text, const Mask * icon, std::function<void()> onPress, std::function<void()> onSelect = nullptr, std::function<bool()> isChecked = nullptr);
    void addLine(const std::string & text, std::function<void()> onPress, std::function<void()> onSelect = nullptr, std::function<bool()> isChecked = nullptr)
    {
      addLine(text, nullptr, std::move(onPress), std::move(onSelect), std::move(isChecked));
    }

    void addCustomLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdColor color)> drawLine, std::function<void()> onPress, std::function<void()> onSelect = nullptr, std::function<bool()> isChecked = nullptr);

    void removeLines();

    unsigned count() const
    {
      return content->body.count();
    }

    void select(int index)
    {
      content->body.select(index);
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

    void checkEvents() override
    {
      ModalWindow::checkEvents();
      if (waitHandler) {
        waitHandler();
      }
    }

  protected:
    MenuWindowContent * content;
    std::function<void()> waitHandler;
    void updatePosition();
};

class MultiMenu: public ModalWindow
{
  public:
    explicit MultiMenu(Window * parent):
      ModalWindow(parent, true)
    {
    }

    void addColumn(MenuWindowContent * column)
    {
      columns.emplace_back(column);
    }

    MenuWindowContent * getColumn(uint8_t index)
    {
      return columns[index];
    }

  protected:
    std::vector<MenuWindowContent *> columns;
};

}
