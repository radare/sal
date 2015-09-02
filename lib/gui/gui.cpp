#define SAL_GUI_LIBRARY "libsalgui.so"
#define GuiInit() gui.init
#define GuiLoop() gui.loop
#define GuiButton(x,y) push y push x gui.button
#define GuiWindow(x,y) push y push x gui.window
#define GuiAdd(x,y) push y push x gui.add
#define GuiAddPack(x,y) push y push x gui.add.pack
#define GuiAddPackEnd(x,y) push y push x gui.add.packend
#define GuiHButtonBox() gui.hbbox
#define GuiShow(x) push x gui.show
#define GuiLabel(x) push x gui.label
#define GuiLabelSet(x,y) push y push x gui.label.set
#define GuiVBox() gui.vbox
#define GuiHBox() gui.hbox
#define GuiProgress() gui.progress
#define GuiProgressSet(x,y) push y push x gui.progress.set
