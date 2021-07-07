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

class Keyboard: public FormWindow
{
  public:
    explicit Keyboard(coord_t height):
      FormWindow(nullptr, {0, LCD_H - height, LCD_W, height}, OPAQUE)
    {
    }

    static void hide()
    {
      if (activeKeyboard) {
        activeKeyboard->clearField();
        activeKeyboard = nullptr;
      }
    }

  protected:
    static Keyboard * activeKeyboard;
    FormField * field = nullptr;
    Window * fieldContainer = nullptr;

    virtual void setField(FormField * newField)
    {
      if (activeKeyboard) {
        if (activeKeyboard == this)
          return;
        activeKeyboard->clearField();
      }
      activeKeyboard = this;
      attach(MainWindow::instance());
      fieldContainer = getFieldContainer(newField);
      auto backupHeight = fieldContainer->height();
      fieldContainer->setHeight(fieldContainer->height() - height());
      fieldContainer->scrollTo(newField);
      fieldContainer->setHeight(backupHeight);
      fieldContainer->disableScroll();
      invalidate();
      newField->setEditMode(true);
      field = newField;
    }

    void clearField()
    {
      detach();
      if (fieldContainer) {
        fieldContainer->enableScroll();
        fieldContainer = nullptr;
      }
      if (field) {
        field->setEditMode(false);
        field = nullptr;
      }
    }

    static Window * getFieldContainer(FormField * field)
    {
      Window * parent = field;
      while (true) {
        parent = parent->getParent();
        if (!(parent->getWindowFlags() & FORWARD_SCROLL) /*&& tmp->width() == LCD_W && tmp->height() == LCD_H*/) {
          return parent;
        }
      }
    }
};
