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

#include <utility>
#include <vector>
#include "form.h"
#include "libopenui_config.h"
#include "font.h"

namespace ui {

class Table: public FormField
{
  public:
    class Cell
    {
      public:
        virtual ~Cell() = default;

        virtual void paint(BitmapBuffer * dc, const rect_t & rect, LcdColor color, LcdFlags flags) = 0;

        [[nodiscard]] virtual bool needsInvalidate() = 0;
    };

    class StringCell: public Cell
    {
      public:
        StringCell() = default;

        explicit StringCell(std::string value):
          value(std::move(value))
        {
        }

        void paint(BitmapBuffer * dc, const rect_t & rect, LcdColor color, LcdFlags flags) override
        {
          dc->drawText(rect.x, rect.y + (rect.h - getFontHeight(TABLE_BODY_FONT)) / 2, value.c_str(), color, flags);
        }

        [[nodiscard]] bool needsInvalidate() override
        {
          return valueChanged;
        }

        [[nodiscard]] std::string getValue() const
        {
          return value;
        }

        void setValue(std::string newValue)
        {
          value = std::move(newValue);
          valueChanged = true;
        }

      protected:
        std::string value;
        bool valueChanged = false;
    };

    class DynamicStringCell: public Cell
    {
      public:
        explicit DynamicStringCell(std::function<std::string()> getText):
          getText(std::move(getText))
        {
        }

        void paint(BitmapBuffer * dc, const rect_t & rect, LcdColor color, LcdFlags flags) override
        {
          auto text = getText();
          dc->drawText(rect.x, rect.y  + (rect.h - getFontHeight(TABLE_BODY_FONT)) / 2, text.c_str(), color, flags);
        }

        [[nodiscard]] bool needsInvalidate() override
        {
          auto newText = getText();
          if (newText != currentText) {
            currentText = newText;
            return true;
          }
          else {
            return false;
          }
        }

      protected:
        std::function<std::string()> getText;
        std::string currentText;
    };

    class CustomCell: public Cell
    {
      public:
        explicit CustomCell(std::function<void(BitmapBuffer * /*dc*/, const rect_t & rect, LcdColor /*color*/, LcdFlags /*flags*/)> paintFunction, std::function<bool()> isInvalidateNeededFunction = nullptr):
          paintFunction(std::move(paintFunction)),
          isInvalidateNeededFunction(std::move(isInvalidateNeededFunction))
        {
        }

        void paint(BitmapBuffer * dc, const rect_t & rect, LcdColor color, LcdFlags flags) override
        {
          paintFunction(dc, rect, color, flags);
        }

        [[nodiscard]] bool needsInvalidate() override
        {
          return isInvalidateNeededFunction ? isInvalidateNeededFunction() : false;
        }

      protected:
        std::function<void(BitmapBuffer * dc, const rect_t & rect, LcdColor color, LcdFlags flags)> paintFunction;
        std::function<bool()> isInvalidateNeededFunction;
    };

    class Line: public Window
    {
      public:
        explicit Line(uint8_t columnsCount):
          Window(nullptr, {0, 0, 0, 0}, OPAQUE),
          cells(columnsCount, nullptr)
        {
        }

        Line(Table * parent, const rect_t & rect, uint8_t columnsCount):
          Window(parent, rect, OPAQUE),
          cells(columnsCount, nullptr)
        {
        }

        virtual ~Line()
        {
          for (auto cell: cells) {
            delete cell;
          }
        }

        coord_t lineHeight = TABLE_DEFAULT_LINE_HEIGHT;
        std::vector<Cell *> cells;
        std::function<void()> onPress;
        std::function<void()> onSelect;
        LcdFlags font = TABLE_BODY_FONT;
        LcdColor color = DEFAULT_COLOR;
    };

    class Header: public Line
    {
      public:
        Header(Table * parent, const rect_t & rect, uint8_t columnsCount):
          Line(parent, rect, columnsCount)
        {
        }

#if defined(DEBUG_WINDOWS)
        [[nodiscard]] std::string getName() const override
        {
          return "Table::Header";
        }
#endif

        void paint(BitmapBuffer * dc) override;
    };

    class Body: public Window
    {
      friend class Table;

      public:
        Body(Table * parent, const rect_t & rect, WindowFlags windowFlags):
          Window(parent, rect, windowFlags)
        {
        }

        ~Body() override
        {
          clear();
        }

#if defined(DEBUG_WINDOWS)
        [[nodiscard]] std::string getName() const override
        {
          return "Table::Body";
        }
#endif

        void addLine(Line * line)
        {
          line->attach(this);
          line->setRect({0, lines.size() > 0 ? lines[lines.size() - 1]->bottom() : 0, width(), line->lineHeight});
          lines.push_back(line);
          setInnerHeight(line->bottom() - TABLE_LINE_BORDER);
          if (hasFocus() && selection < 0) {
            select(0, true);
          }
        }

        void setLineFont(unsigned lineIndex, LcdFlags font)
        {
          auto line = lines[lineIndex];
          if (line->font != font) {
            line->font = font;
            line->invalidate();
          }
        }

        void setLineColor(unsigned lineIndex, LcdColor color)
        {
          auto line = lines[lineIndex];
          if (line->color != color) {
            line->color = color;
            line->invalidate();
          }
        }

        void clear()
        {
          Window::clear();
          lines.clear();
        }

        void select(int lineIndex, bool scroll)
        {
          selection = lineIndex;
          if (scroll) {
            scrollTo(lineIndex);
          }
          invalidate();
          if (lineIndex >= 0 && lineIndex < (int)lines.size()) {
            auto onSelect = lines[lineIndex]->onSelect;
            if (onSelect) {
              onSelect();
            }
          }
        }

        void scrollTo(int lineIndex)
        {
          if (lineIndex >= 0 && lineIndex < (int)lines.size()) {
            auto line = lines[lineIndex];
            coord_t y = line->top();
            Window * window = this;
            while (window->getWindowFlags() & FORWARD_SCROLL) {
              y += window->top();
              window = window->getParent();
            }
            const rect_t rect = {
              0,
              y,
              width(),
              line->height()
            };
            window->scrollTo(rect);
          }
        }

        void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_KEYS)
        void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
        bool onTouchEnd(coord_t x, coord_t y) override;
#endif

        void checkEvents() override;

      protected:
        std::vector<Line *> lines;
        int selection = -1;
    };

  public:
    Table(Window * parent, const rect_t & rect, uint8_t columnsCount, const std::function<Header *(Table *)> & createHeader = nullptr, const std::function<Body *(Table *)> & createBody = nullptr, WindowFlags windowFlags = OPAQUE | FORM_NO_BORDER):
      FormField(parent, rect, windowFlags),
      columnsCount(columnsCount),
      columnsWidth(columnsCount, width() / columnsCount),
      header(createHeader ? createHeader(this) : new Header(this, {0, 0, width(), 0}, columnsCount)),
      body(createBody ? createBody(this) : new Body(this, {0, 0, width(), height()}, windowFlags))
    {
    }

#if defined(DEBUG_WINDOWS)
    [[nodiscard]] std::string getName() const override
    {
      return "Table";
    }
#endif

    void setColumnsWidth(const coord_t values[])
    {
      int restColumn = -1;
      coord_t restWidth = width() - 2 * TABLE_HORIZONTAL_PADDING;
      for (auto i = 0; i < columnsCount; i++) {
        auto columnWidth = values[i];
        columnsWidth[i] = columnWidth;
        if (columnWidth == 0) {
          restColumn = i;
        }
        else {
          restWidth -= columnWidth;
        }
      }
      if (restColumn >= 0) {
        columnsWidth[restColumn] = restWidth;
      }
    }

    [[nodiscard]] coord_t getColumnWidth(uint8_t columnIndex) const
    {
      return columnsWidth[columnIndex];
    }

    [[nodiscard]] int getSelection() const
    {
      return body->selection;
    }

    void clearSelection()
    {
      body->selection = -1;
      body->invalidate();
    }

    bool setFocus(uint8_t flag = SET_FOCUS_DEFAULT, Window * from = nullptr) override // NOLINT(google-default-arguments)
    {
      if (body->lines.empty()) {
        if (flag == SET_FOCUS_BACKWARD) {
          if (previous) {
            return previous->setFocus(flag, this);
          }
          else {
            return false;
          }
        }
        else {
          if (next) {
            return next->setFocus(flag, this);
          }
          else {
            return false;
          }
        }
      }
      else {
        body->setFocus(flag, from);
        if (body->selection < 0) {
          select(flag == SET_FOCUS_BACKWARD ? (int)body->lines.size() - 1 : 0);
        }
        return true;
      }
    }

    void select(int lineIndex, bool scroll = true)
    {
      body->select(lineIndex, scroll);
    }

    void setLineFont(unsigned lineIndex, LcdFlags font)
    {
      body->setLineFont(lineIndex, font);
    }

    void setLineColor(unsigned lineIndex, LcdColor color)
    {
      body->setLineColor(lineIndex, color);
    }

    void setHeader(const char * const values[])
    {
      header->setHeight(TABLE_HEADER_HEIGHT);
      body->setTop(TABLE_HEADER_HEIGHT);
      if (windowFlags & FORWARD_SCROLL)
        setHeight(header->height() + body->height());
      else
        body->setHeight(height() - TABLE_HEADER_HEIGHT);
      for (auto i = 0; i < columnsCount; i++) {
        delete header->cells[i];
        header->cells[i] = new StringCell(values[i]);
      }
    }

    void addLine(Line * line, std::function<void()> onPress = nullptr, std::function<void()> onSelect = nullptr)
    {
      line->onPress = std::move(onPress);
      line->onSelect = std::move(onSelect);
      body->addLine(line);
    }

    void addLine(const char * const values[], std::function<void()> onPress = nullptr, std::function<void()> onSelect = nullptr)
    {
      Line * line = new Line(columnsCount);
      for (auto i = 0; i < columnsCount; i++) {
        line->cells[i] = new StringCell(values[i]);
      }
      addLine(line, std::move(onPress), std::move(onSelect));
    }

    [[nodiscard]] Cell * getCell(unsigned row, unsigned column) const
    {
      return body->lines[row]->cells[column];
    }

    void clear()
    {
      clearSelection();
      body->clear();
    }

    [[nodiscard]] unsigned size() const
    {
      return body->lines.size();
    }

    [[nodiscard]] Header * getHeader()
    {
      return header;
    }

    [[nodiscard]] Body * getBody()
    {
      return body;
    }

  protected:
    uint8_t columnsCount;
    std::vector<coord_t> columnsWidth;
    Header * header;
    Body * body;
};

}
