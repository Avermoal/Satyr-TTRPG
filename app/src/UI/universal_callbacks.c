#include "UI/universal_callbacks.h"

#include "UI/DMs_UI.h"
#include "UI/Players_UI.h"

/*ESCAPE*/
gboolean on_escape_pressed(GtkEventControllerKey* controller, guint keyval,
                       guint keycode, GdkModifierType state, gpointer userdata)
{
  if(keyval != GDK_KEY_Escape){
    return FALSE;
  }
  /*Cancell server thread*/
  GCancellable* servercancellable = G_CANCELLABLE(g_object_get_data(G_OBJECT(userdata), "servercancellable"));
  g_cancellable_cancel(servercancellable);
  g_object_unref(servercancellable);
  servercancellable = nullptr;
  g_object_set_data(G_OBJECT(userdata), "servercancellable", nullptr);
  /*Get interfacestack*/
  GtkStack* interfacestack = GTK_STACK(g_object_get_data(G_OBJECT(userdata), "interfacestack"));
  if(interfacestack){
    /*Set visible menu*/
    gtk_stack_set_visible_child_name(interfacestack, "menu_page");
    /*Remove dms_page from interfacestack*/
    GtkWidget* dms_page = GTK_WIDGET(gtk_stack_get_child_by_name(interfacestack, "dms_page"));
    if(dms_page){
      gtk_stack_remove(interfacestack, dms_page);
    }
    /*Remove players_page from interfacestack*/
    GtkWidget* players_page = GTK_WIDGET(gtk_stack_get_child_by_name(interfacestack, "players_page"));
    if(players_page){
      gtk_stack_remove(interfacestack, players_page);
    }
  }
  /*DMs_UI free*/
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  if(dms_ui){
    free(dms_ui);
    dms_ui = nullptr;
    g_object_set_data(G_OBJECT(userdata), "dms_ui_data", nullptr);
  }
  /*Entry buffer free*/
  GtkEntryBuffer* entrybuffer = GTK_ENTRY_BUFFER(g_object_get_data(G_OBJECT(userdata), "campaign_name_buffer"));
  if(entrybuffer){
    g_object_unref(entrybuffer);
    g_object_set_data(G_OBJECT(userdata), "campaign_name_buffer", nullptr);
  }
  /*Name entry buffer free*/
  GtkEntryBuffer* player_name_buffer = GTK_ENTRY_BUFFER(g_object_get_data(G_OBJECT(userdata), "player_name_buffer"));
  if(player_name_buffer){
    g_object_unref(player_name_buffer);
    g_object_set_data(G_OBJECT(userdata), "player_name_buffer", nullptr);
  }
  /*IP Port buffer free*/
  GtkEntryBuffer* ipportbuf = GTK_ENTRY_BUFFER(g_object_get_data(G_OBJECT(userdata), "ipportbuf"));
  if(ipportbuf){
    g_object_unref(ipportbuf);
    g_object_set_data(G_OBJECT(userdata), "ipportbuf", nullptr);
  }
  /*Players_UI free*/
  struct Players_UI* players_ui = (struct Players_UI*)g_object_get_data(G_OBJECT(userdata), "players_ui_data");
  if(players_ui){
    free(players_ui);
    players_ui = nullptr;
    g_object_set_data(G_OBJECT(userdata), "players_ui_data", nullptr);
  }
  return TRUE;
}
