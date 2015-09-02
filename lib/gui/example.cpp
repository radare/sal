/*
 * Simple application using cpp macros
 */

#include "sal.cpp"
#include "gui.cpp"

loadlib(SAL_GUI_LIBRARY)

GuiInit()
#define _window %r0
#define _vbox %r1
#define _label %r2
#define _progress %r3
#define _hbox %r4
#define _button %r5
#define _msg %r6
#define _msg_i %r7

#define _tmp %r9

 set(_msg_i, 0)
 array_new(5) is(_msg)
 array_set(_msg, 0, "Click to make fun")
 array_set(_msg, 1, "Oooh Yeeeess!")
 array_set(_msg, 2, "Stop. Dont click me more!")
 array_set(_msg, 3, "I'll segfault...")
 array_set(_msg, 4, "NO MOOORE!!")

 GuiWindow("Sal::Gtk", vm.exit) is(_window)
 GuiVBox() is (_vbox)

  ;-- add label
  GuiHBox() is (_hbox)
   GuiLabel("Click the button to make the progress happy") is(_label)
   GuiAdd(_hbox, _label)
   GuiButton("click me!", ClickMe) is (_button)
   GuiAddPackEnd(_hbox, _button)
  GuiAddPack(_vbox, _hbox)

  ;-- add button
  GuiProgress() is(_progress)
  GuiAddPack(_vbox, _progress)

 GuiAdd(_window, _vbox)
 GuiShow(_window)

GuiLoop()

exit(0)

SetNextLabelString:
  array_get(_msg, _msg_i) is(_tmp)
  GuiLabelSet(_label, _tmp)
  ret

ClickMe:
  ; _msg_i ++ 
  push _msg_i push 1 add 2 pop _msg_i

  ; _tmp = _msg_i*23
  push 23 push _msg_i mul pop _tmp

  GuiProgressSet(_progress, _tmp)
  SetNextLabelString
