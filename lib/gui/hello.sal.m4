#!/usr/pkg/bin/sal -lgui
/*
 *
 * Hello World example with sal+m4+gui
 *
**/

include(sal.m4)
include(gui.m4)

main:
  ; local variables
  define(win,%r0)
  define(but,%r1)
  define(but2,%r2)
  define(butbox,%r3)
  define(vebox, %r4) ; vertical box

  GuiInit()

  GuiWindow("Hello World", bye) is(win)

  GuiVBox() is(vebox)
  GuiButton("gtk-ok", click_ok) is(but)
  GuiButton("gtk-cancel", click_ok) is(but2)
  GuiHButtonBox() is(butbox)
  GuiAdd(butbox,but2)
  GuiAdd(butbox,but)

  GuiLabel("Hello World") is(%r9)
  GuiAdd(vebox, %r9)
  GuiAddPackEnd(vebox, butbox)

  GuiAdd(win, vebox)
  GuiShow(win)

  GuiLoop()

click_ok:
  push "Button clicked!"
  println 1
  push 0 vm.exit

bye:
  push "Bye!"
  println 1
  push 0 vm.exit
