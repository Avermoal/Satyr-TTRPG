#include "UI/universal_callbacks.h"

#include "UI/DMs_UI.h"

/*ESCAPE*/
gboolean on_escape_pressed(GtkEventControllerKey* controller, guint keyval,
                       guint keycode, GdkModifierType state, gpointer userdata)
{
  if(keyval != GDK_KEY_Escape){
    return FALSE;
  }
  /*Get interfacestack*/
  GtkStack* interfacestack = GTK_STACK(g_object_get_data(G_OBJECT(userdata), "interfacestack"));
  if(!interfacestack){
    return FALSE;
  }
  /*Set visible menu*/
  gtk_stack_set_visible_child_name(interfacestack, "menu_page");
  /*Remove dms_page from interfacestack*/
  GtkWidget* dms_page = GTK_WIDGET(gtk_stack_get_child_by_name(interfacestack, "dms_page"));
  if(!dms_page){
    return FALSE;
  }
  gtk_stack_remove(interfacestack, dms_page);
  /*DMs_UI free*/
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  if(!dms_ui){
    return FALSE;
  }
  free(dms_ui);
  dms_ui = nullptr;
  g_object_set_data(G_OBJECT(userdata), "dms_ui_data", nullptr);
  /*Entry buffer free*/
  GtkEntryBuffer* entrybuffer = GTK_ENTRY_BUFFER(g_object_get_data(G_OBJECT(userdata), "campaign_name_buffer"));
  if(!entrybuffer){
    return FALSE;
  }
  g_object_unref(entrybuffer);
  g_object_set_data(G_OBJECT(userdata), "campaign_name_buffer", nullptr);
  /*Remove on_escape_pressed callback*/
  g_signal_handlers_disconnect_by_func(controller, G_CALLBACK(on_escape_pressed), userdata);
  return TRUE;
}
