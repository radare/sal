#!/usr/pkg/bin/sal -lgui

include(gui.m4)

GuiInit()

 GuiWindow("Sal::Gtk", vm.exit) is(%r0)
 GuiLabel("Hello World") is(%r1)
 GuiAdd(%r0, %r1)
 GuiShow(%r0)

GuiLoop()
