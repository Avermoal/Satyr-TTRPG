#ifndef UI_UI_H
#define UI_UI_H

#include <gtk/gtk.h>

struct UI{
  GtkWidget* menu_page;
};

void createUI(struct UI* ui, GtkWindow* win);

void change_widget_cursor(GtkWidget* widget, const char* name);

#endif/*UI_UI_H*/
