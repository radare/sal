#include "vm.h"
#include "file.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>

vm_t *_vm = NULL;

#define USE_GTK 1
char *salsyms = "gui.init,gui.add,gui.label,gui.label.set,gui.add.pack,gui.dialog,gui.dialog.file,gui.add.packend,gui.vbox,gui.hbox,gui.hbbox,gui.window,gui.window.size,gui.button,gui.combo,gui.combo.get,gui.entry,gui.loop,gui.show,gui.entry.get,gui.entry.set,gui.progress,gui.progress.set";

/*
TODO

gui.object {
  magic   ; magic number of a gui widget
  widget  ; widget id
  data    ; pointer to data
}
*/

/* initialize graphical user interface */
void sal_gui_init(vm_t *vm)
{
	_vm = vm;

#if USE_GTK
	gtk_init(NULL, NULL);
#endif
}


void sal_gui_vbox(vm_t *vm)
{
	GtkWidget *vbox;
	obj_t *ptr;

	vbox = gtk_vbox_new(FALSE, 5);

	ptr = pointer_new("gui-vbox",NULL);
	ptr->data = vbox;

	sal_stack_push(vm->stack, ptr);
}

void sal_gui_hbox(vm_t *vm)
{
	GtkWidget *hbox;
	obj_t *ptr;

	hbox = gtk_hbox_new(FALSE, 5);

	ptr = pointer_new("gui-hbox",NULL);
	ptr->data = hbox;

	sal_stack_push(vm->stack, ptr);
}

void sal_gui_hbbox(vm_t *vm)
{
	GtkWidget *hbbox;
	obj_t *ptr;

	hbbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(hbbox), 5);

	ptr = pointer_new("gui-hbbox",NULL);
	ptr->data = hbbox;

	sal_stack_push(vm->stack, ptr);
}

void sal_gui_combo(vm_t *vm)
{
	GtkWidget *combo;
	obj_t *ptr;
	char *word = file_read(vm->file, vm->output);
	int j, i, items = integer_get_value_i(word);

	combo = gtk_combo_box_new_text();
	
	if (items<0)
		items = 0;

	for(i=0;i<items;i++) {
		ptr = sal_stack_pop(vm->stack);
		switch(ptr->type) {
		case OBJECT_STRING:
			gtk_combo_box_insert_text( GTK_COMBO_BOX(combo), i, ptr->data);
			break;
		case OBJECT_ARRAY:
			for(j=0;j<array_size(ptr);j++) {
				obj_t *obj = (obj_t *)array_get(ptr, j);
				switch(obj->type) {
				case OBJECT_STRING:
					gtk_combo_box_insert_text( GTK_COMBO_BOX(combo), j, string_get_value(obj));
					break;
				default:
					fprintf(stderr, "Invalid object in array\n");
					break;
				}
			}
			break;
		default:
			fprintf(stderr, "Invalid object in stack (%d). wanna string or string array.\n",ptr->type);
			break;
		}
	}

	gtk_combo_box_set_active( GTK_COMBO_BOX(combo), 0 );

	ptr = pointer_new("gui-combo",NULL);
	ptr->data = combo;

	sal_stack_push(vm->stack, ptr);
}

void sal_gui_combo_get(vm_t *vm)
{
	obj_t *str;
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack); /* widget */

	str = string_new((char *)gtk_combo_box_get_active_text(GTK_COMBO_BOX(obj->data)));
	sal_stack_push(vm->stack, str);
}

/*
 * push 2
 * push "You're going to die"
 * push "About"
 * gui.dialog
 */
enum {
	DIALOG_POPUP = 0,
	DIALOG_YESNO = 2
} dialog_t;

void sal_gui_dialog(vm_t *vm)
{
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack); /* title */
	obj_t *str = (obj_t *)sal_stack_pop(vm->stack); /* string */
	obj_t *typ = (obj_t *)sal_stack_pop(vm->stack); /* type */
	obj_t *ptr;

#if USE_GTK
        GtkDialog *d = (GtkDialog *)gtk_dialog_new();
        int ret;

        gtk_window_set_title(GTK_WINDOW(d), string_get_value(obj));
//      gtk_window_set_default_size(GTK_WINDOW(d), 300,140);
        gtk_window_set_position(GTK_WINDOW(d), GTK_WIN_POS_CENTER);
	switch(integer_get_value(typ)) {
	case DIALOG_YESNO:
        	gtk_dialog_add_button(d, "gtk-cancel", FALSE);
	case DIALOG_POPUP:
	default:
		gtk_dialog_add_button(d, "gtk-ok", TRUE);
		break;
	}
        gtk_dialog_set_default_response(d, FALSE);
        gtk_window_set_modal(GTK_WINDOW(d), TRUE);
        // XXX no access to w pointer
        //gtk_window_set_transient_for(GTK_WINDOW(d), GTK_WINDOW(w->window));
        gtk_container_add(
                GTK_CONTAINER(d->vbox),
                gtk_label_new( string_get_value(str) ));
        gtk_widget_show_all(GTK_WIDGET(d));

        ret = gtk_dialog_run(d);
        gtk_widget_destroy(GTK_WIDGET(d));
#endif
	vm->flags = FLAGS_CLEAR;
	if (ret<0) {
		ret = -1; // avoid -4 if window closed
		vm->flags = FLAGS_NOPER;
		// nothing new in stack
		return;
	}
	ptr = integer_new_i(ret);
	sal_stack_push(vm->stack, ptr);
}

/*
 *
 * TODO: filters
 *
 * type:
 *  - open/save/selectfolder/createfolder( actions )
 *  - confirm/acceptfilename/again       ( confirmation )
 *  - one file
 *  - multiple files
 *  - directory
 * filters : Log files: *.log 
 */
/*
file chooser dialog

push type
push title-string
gui.dialog.file

*/
void sal_gui_dialog_file(vm_t *vm)
{
	obj_t *str = (obj_t *)sal_stack_pop(vm->stack); /* string */
	obj_t *typ = (obj_t *)sal_stack_pop(vm->stack); /* type */
	obj_t *ptr;
	GtkFileChooserDialog *fcd;

	if (str->type != OBJECT_STRING) {
		fprintf(stderr, "string expected\n");
		return;
	}

	if (typ->type != OBJECT_INTEGER) {
		fprintf(stderr, "integer expected\n");
		return;
	}

	fcd = (GtkFileChooserDialog *)gtk_file_chooser_dialog_new
		(
		string_get_value(str), //"Open file...",
		NULL, // parent
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		NULL
		);

	gtk_window_set_position( GTK_WINDOW(fcd), GTK_WIN_POS_CENTER);
	if ( gtk_dialog_run(GTK_DIALOG(fcd)) == GTK_RESPONSE_ACCEPT ) {
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fcd));
		ptr = string_new(filename);
		sal_stack_push(vm->stack, ptr);
		vm->flags = FLAGS_CLEAR;
	} else {
		vm->flags = FLAGS_ERROR;
	}

	gtk_widget_destroy(GTK_WIDGET(fcd));
}

void gui_entry_label(vm_t *vm)
{
	/* not yet implemented */
	// label + box widget, multiple widget.. 
	// oops. multiple widget assignement
}

void sal_gui_entry(vm_t *vm)
{
	// TODO: get descr and default text for the entry??
	// optional stuff ?
	//obj_t *obj = (obj_t *)sal_stack_pop(vm->stack); /* descr */
	//obj_t *str = (obj_t *)sal_stack_pop(vm->stack); /* entry */

	GtkWidget *entry;
	obj_t *ptr;

	entry = gtk_entry_new();
//         gtk_entry_set_text(GTK_ENTRY(speaker_delay),buf);
//         gtk_entry_set_max_length(GTK_ENTRY(speaker_delay),3);
//         gtk_entry_set_width_chars(GTK_ENTRY(speaker_delay),5);

	ptr = pointer_new("gui-entry",NULL);
	ptr->data = entry;

	sal_stack_push(vm->stack, ptr);
}

void sal_gui_entry_set(vm_t *vm)
{
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack); /* widget */
	obj_t *str = (obj_t *)sal_stack_pop(vm->stack); /* string */

	gtk_entry_set_text(
		GTK_ENTRY(pointer_get_value(obj)),
		string_get_value(str));

	object_free(obj);
	object_free(str);
}

void sal_gui_entry_get(vm_t *vm)
{
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack); /* widget */
	obj_t *str;

	str = string_new((char *)gtk_entry_get_text(GTK_ENTRY(obj->data)));
	sal_stack_push(vm->stack, str);
}

void sal_gui_label_set(vm_t *vm)
{
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack);
	obj_t *str = (obj_t *)sal_stack_pop(vm->stack);

	gtk_label_set_text(obj->data, string_get_value(str));
}

void sal_gui_label(vm_t *vm)
{
	obj_t *str = (obj_t *)sal_stack_pop(vm->stack); /* string */
	obj_t *ptr;
	GtkWidget *label;

	if ((str->type != OBJECT_STRING)
	||  (string_is_binary(str)))
		return;

	label = gtk_label_new(string_get_value(str));

	ptr = pointer_new("gui-label",NULL);
	ptr->data = label;

	sal_stack_push(vm->stack, ptr);

	object_free(str);
}

void sal_gui_add(vm_t *vm)
{
	obj_t *box = (obj_t *)sal_stack_pop(vm->stack); /* box */
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack); /* widget */

	GtkWidget *boxwidget = box->data;
	gtk_container_add(GTK_CONTAINER(boxwidget), obj->data);
}

void sal_gui_add_pack(vm_t *vm)
{
	obj_t *box = (obj_t *)sal_stack_pop(vm->stack); /* box */
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack); /* widget */
	// TODO: checks
	gtk_box_pack_start(GTK_BOX(box->data),
		obj->data,FALSE,FALSE,3);
}

void sal_gui_add_packend(vm_t *vm)
{
	obj_t *box = (obj_t *)sal_stack_pop(vm->stack); /* box */
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack); /* widget */
	// TODO: checks
	gtk_box_pack_end(GTK_BOX(box->data),
		obj->data,FALSE,FALSE,3);
}

void gui_callback(obj_t *obj)
{
	if (obj->type != OBJECT_LABEL) {
		printf("label expected\n");
	}

	// TODO: prepare stack here? (callback environ)
	label_t *label = (label_t *)obj->data;
	label_call_back(_vm, label);
}

void sal_gui_progress(vm_t *vm)
{
	obj_t *ptr;
	GtkWidget *pb;
	
	pb = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(
		GTK_PROGRESS_BAR(pb),
		GTK_PROGRESS_LEFT_TO_RIGHT);

	ptr = pointer_new("gui-progress", NULL);
	ptr->data = pb;

	sal_stack_push(vm->stack, ptr);
}

void sal_gui_progress_set(vm_t *vm)
{
	obj_t *box = (obj_t *)sal_stack_pop(vm->stack); /* progress bar widget */
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack); /* value */

	switch(obj->type) {
	case OBJECT_STRING: /* text */
		gtk_progress_bar_set_text(
			GTK_PROGRESS_BAR(box->data),
			string_get_value(obj));
		break;
	case OBJECT_FLOAT: { /* 0 - 1 */
		float val = float_get_value(obj);
		if (val<0) val = 0;
		if (val>1) val = 1;
		gtk_progress_bar_set_fraction(
			GTK_PROGRESS_BAR(box->data), 
			(gdouble)val); }
		break;
	case OBJECT_INTEGER: { /* 0 - 100 */
		int val = integer_get_value(obj);
		if (val<0) val = 0;
		if (val>100) val = 100;
		gtk_progress_bar_set_fraction(
			GTK_PROGRESS_BAR(box->data), 
			(gdouble)0.01*val); }
		break;
	default:
		fprintf(stderr, "Don't know how to handle this obj (%d).\n", obj->type);
	}
}

void sal_gui_button(vm_t *vm)
{
	obj_t *str = (obj_t *)sal_stack_pop(vm->stack); /* string */
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack); /* callback */
	obj_t *ptr;
	GtkWidget *button;

	if ((str->type != OBJECT_STRING)
	||  (string_is_binary(str)))
		return;

	if (obj->type != OBJECT_LABEL) {
		printf("Expect llabel\n");
		exit(1);
	}

	button = gtk_button_new_from_stock(str->data);
	gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(gui_callback), (gpointer)obj);

	ptr = pointer_new("gui-button", NULL);
	ptr->data = button;

	sal_stack_push(vm->stack, ptr);
}

void sal_gui_window_size(vm_t *vm)
{
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack);
	obj_t *width = (obj_t *)sal_stack_pop(vm->stack);
	obj_t *heigh = (obj_t *)sal_stack_pop(vm->stack);

	GtkWidget *window = obj->data;
	gtk_window_set_default_size(
		GTK_WINDOW(window),
		integer_get_value(width),
		integer_get_value(heigh));
	gtk_widget_set_usize(
		GTK_WIDGET(window),
		integer_get_value(width),
		integer_get_value(heigh));
}

void sal_gui_window(vm_t *vm)
{
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack);
	obj_t *cbo = (obj_t *)sal_stack_pop(vm->stack);
	obj_t *ptr;
#if USE_GTK
	GtkWidget *window;
#endif

	if ((obj->type != OBJECT_STRING)
	||  (string_is_binary(obj)))
		return;

	if (cbo->type != OBJECT_LABEL) {
		printf("Label expected\n");
		exit(1);
	}
#if USE_GTK
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW( window ), obj->data);
	gtk_signal_connect(GTK_OBJECT(window), "destroy",
		GTK_SIGNAL_FUNC(exit), cbo);
#endif

	ptr = pointer_new("gui-window",NULL);
	ptr->data = window;

	sal_stack_push(vm->stack, ptr);
}

void sal_gui_show(vm_t *vm)
{
	obj_t *obj = (obj_t *)sal_stack_pop(vm->stack);

	if (obj->type != OBJECT_POINTER)
		return;

#if USE_GTK
	gtk_widget_show_all(GTK_WIDGET(obj->data));
#endif
}

void sal_gui_loop(vm_t *vm)
{
	_vm = vm;
#if USE_GTK
	gtk_main();
#endif
}
