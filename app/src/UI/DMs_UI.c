#include "UI/DMs_UI.h"

void on_to_menu(GtkButton* btn, GtkWindow* win, gpointer userdata)
{
  GtkStack* interfacestack = GTK_STACK(g_object_get_data(G_OBJECT(win), "interfacestack"));
  gtk_stack_set_visible_child_name(interfacestack, "menu_page");
}
