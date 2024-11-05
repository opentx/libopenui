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

#include "mainwindow.h"
#include "form.h"

namespace ui {

class KeyboardBase: public FormWindow
{
  public:
    explicit KeyboardBase(coord_t height):
      FormWindow(nullptr, {0, LCD_H - height, LCD_W, height}, OPAQUE)
    {
    }

    static void hide()
    {
      if (activeKeyboard) {
        auto keyboard = activeKeyboard;
        activeKeyboard = nullptr;
        keyboard->clearField();
      }
    }

    virtual void clearField() = 0;

  protected:
    static KeyboardBase * activeKeyboard;
    Window * fieldContainer = nullptr;
    coord_t fieldContainerOriginalHeight = 0;
};

template <class T>
class Keyboard: public KeyboardBase
{
  public:
    using KeyboardBase::KeyboardBase;

    virtual void setField(T * newField)
    {
      if (field != newField) {
        if (activeKeyboard) {
          activeKeyboard->clearField();
        }
        activeKeyboard = this;
        attach(MainWindow::instance());
        fieldContainer = getFieldContainer(newField);
        fieldContainerOriginalHeight = fieldContainer->height();
        fieldContainer->setHeight(fieldContainer->height() - height());
        fieldContainer->scrollTo(newField);
        fieldContainer->disableScroll();
        invalidate();
        field = newField;
      }
    }

    void clearField() override
    {
      detach();
      if (fieldContainer) {
        fieldContainer->setHeight(fieldContainerOriginalHeight);
        fieldContainer->enableScroll();
        fieldContainer = nullptr;
      }
      if (field) {
        field->setEditMode(false);
        field = nullptr;
      }
    }

  protected:
    T * field = nullptr;

    static Window * getFieldContainer(FormField * field)
    {
      Window * parent = field;
      while (true) {
        parent = parent->getParent();
        if (!(parent->getWindowFlags() & FORWARD_SCROLL)) {
          return parent;
        }
      }
    }
};

}
