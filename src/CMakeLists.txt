if(HARDWARE_KEYS)
  add_definitions(-DHARDWARE_KEYS)
endif()

if(HARWARE_TOUCH)
  add_definitions(-DHARDWARE_TOUCH)
endif()

if(SOFTWARE_KEYBOARD)
  add_definitions(-DSOFTWARE_KEYBOARD)
endif()

add_definitions(-DSPNG_USE_MINIZ)
include_directories(
  ${PROJECT_SOURCE_DIR}/thirdparty/libopenui/src/thirdparty/miniz
  ${PROJECT_SOURCE_DIR}/thirdparty/libopenui/src/thirdparty/libspng
  )

set(LIBOPENUI_SRC
  thirdparty/miniz/miniz.c
  thirdparty/libspng/spng.c
  libopenui_globals.cpp
  libopenui_file.cpp
  bitmapbuffer.cpp
  window.cpp
  layer.cpp
  form.cpp
  button.cpp
  static.cpp
  checkbox.cpp
  carousel.cpp
  slider.cpp
  mainwindow.cpp
  menu.cpp
  menutoolbar.cpp
  choice.cpp
  filechoice.cpp
  numberedit.cpp
  textedit.cpp
  coloredit.cpp
  progress.cpp
  table.cpp
  modal_window.cpp
  dialog.cpp
  expansion_panel.cpp
  )

if(SOFTWARE_KEYBOARD)
  set(LIBOPENUI_SRC
    ${LIBOPENUI_SRC}
    keyboard_text.cpp
    keyboard_number.cpp
    )
endif()
