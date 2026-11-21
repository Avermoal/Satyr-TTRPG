#include "UI/Menu_UI.h"

#include <stdlib.h>
#include <string.h>

#include "UI/UIs_info.h"
#include "UI/DMs_UI.h"

typedef struct{
  GtkWidget* box;
  char* basename;
}campaign_to_delete;

static void set_new_campaign_page_callbacks(GtkBuilder* builder, gpointer userdata);
static void on_letsgo_btn_clicked(GtkButton* btn, gpointer userdata);
static void on_back_btn_clicked(GtkButton* btn, gpointer userdata);
static void on_back_from_load_btn_clicked(GtkButton* btn, gpointer userdata);

static void on_campaign_btn_clicked(GtkButton* btn, gpointer userdata);
static void on_delete_campaign_btn_clicked(GtkButton* btn, gpointer userdata);

void on_to_new_campaign_btn_clicked(GtkButton* btn, gpointer userdata)
{
  /*New campaign*/
  GError* err = nullptr;
  GtkBuilder* new_campaign_builder = gtk_builder_new();
  set_new_campaign_page_callbacks(new_campaign_builder, userdata);
  gtk_builder_add_from_file(new_campaign_builder, NEW_CAMPAIGN_SOURCES, &err);
  if(err){
    g_print("New campaign builder error: %s\n", err->message);
    g_object_unref(new_campaign_builder);
    g_error_free(err);
    return;
  }
  GtkWidget* new_campaign_page = GTK_WIDGET(gtk_builder_get_object(new_campaign_builder, "new_campaign_page"));
  /*Get Entry Buffer and set like obj data*/
  GtkEntryBuffer* entrybuffer = GTK_ENTRY_BUFFER(gtk_builder_get_object(new_campaign_builder, "campaign_name_buffer"));
  g_object_ref(entrybuffer);
  g_object_set_data(G_OBJECT(userdata), "campaign_name_buffer", entrybuffer);
  /*Interfacestack*/
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(userdata), "interfacestack");
  gtk_stack_add_named(interfacestack, new_campaign_page, "new_campaign_page");
  gtk_stack_set_visible_child_name(interfacestack, "new_campaign_page");
  g_object_unref(new_campaign_builder);
}

void on_to_load_save_btn_clicked(GtkButton* btn, gpointer userdata)
{
  GtkWidget* scrolledwin = gtk_scrolled_window_new();
  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
  gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolledwin), box);
  /*Get all game camapigns in saves*/
  GFile* saves = g_file_new_for_path("saves/");
  GError* err = nullptr;
  GFileEnumerator* enumerator = g_file_enumerate_children(
    saves,
    G_FILE_ATTRIBUTE_STANDARD_NAME ","
    G_FILE_ATTRIBUTE_STANDARD_TYPE,
    G_FILE_QUERY_INFO_NONE,
    nullptr,
    &err
  );
  if(err){
    g_print("Can't open saves dir\n");
    g_error_free(err);
    g_object_unref(saves);
    g_object_unref(scrolledwin);
    return;
  }
  GFileInfo* info;
  while((info = g_file_enumerator_next_file(enumerator, nullptr, &err))){
    if(err){
      g_print("Can't open file in saves dir");
      g_object_unref(info);
      continue;
    }
    GFileType type = g_file_info_get_file_type(info);
    if(type == G_FILE_TYPE_DIRECTORY){
      const char* basename = g_file_info_get_name(info);
      GtkWidget* campaignbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
      campaign_to_delete* deldata = (campaign_to_delete*)g_new0(campaign_to_delete, 1);
      if(!deldata){
        g_object_unref(campaignbox);
        continue;
      }
      deldata->box = campaignbox;
      deldata->basename = strdup(basename);
      GtkWidget* campaignbtn = gtk_button_new_with_label(basename);
      g_signal_connect(campaignbtn, "clicked", G_CALLBACK(on_campaign_btn_clicked), userdata);
      /*Delete button*/
      GtkWidget* delbtn = gtk_button_new_from_icon_name("list-remove-symbolic");
      g_signal_connect_data(delbtn, "clicked", G_CALLBACK(on_delete_campaign_btn_clicked),
                            (gpointer)deldata, (GClosureNotify)(g_free), 0);
      /*Append to main box*/
      gtk_box_append(GTK_BOX(campaignbox), campaignbtn);
      gtk_box_append(GTK_BOX(campaignbox), delbtn);
      gtk_box_append(GTK_BOX(box), campaignbox);
    }
    /*Add saved btn*/
    g_object_unref(info);
  }
  /*Free enumerator*/
  g_file_enumerator_close(enumerator, nullptr, nullptr);
  g_object_unref(enumerator);
  g_object_unref(saves);
  /*Set back btn*/
  GtkWidget* back_btn = gtk_button_new_with_label("Back");
  g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_from_load_btn_clicked), userdata);
  gtk_box_append(GTK_BOX(box), back_btn);
  /*Interfacestack*/
  GtkStack* interfacestack = GTK_STACK(g_object_get_data(G_OBJECT(userdata), "interfacestack"));
  GtkWidget* child = gtk_stack_get_child_by_name(interfacestack, "load_campaign_page");
  if(child){
    gtk_stack_remove(interfacestack, child);
  }
  gtk_stack_add_named(interfacestack, scrolledwin, "load_campaign_page");
  gtk_stack_set_visible_child_name(interfacestack, "load_campaign_page");
}

void on_to_join_btn_clicked(GtkButton* btn, gpointer userdata)
{

}

void on_exit_btn_clicked(GtkButton* btn, gpointer userdata)
{
  gtk_window_destroy(GTK_WINDOW(userdata));
}

static void on_letsgo_btn_clicked(GtkButton* btn, gpointer userdata)
{
  create_DMs_UI(1, userdata);
}

static void on_back_btn_clicked(GtkButton* btn, gpointer userdata)
{
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(userdata), "interfacestack");
  GtkWidget* child = gtk_stack_get_child_by_name(interfacestack, "new_campaign_page");
  gtk_stack_remove(interfacestack, child);
  gtk_stack_set_visible_child_name(interfacestack, "menu_page");
}

static void set_new_campaign_page_callbacks(GtkBuilder* builder, gpointer userdata)
{
  GtkBuilderScope* scope = gtk_builder_cscope_new();

  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_letsgo_btn_clicked",
                                         G_CALLBACK(on_letsgo_btn_clicked));
  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_back_btn_clicked",
                                         G_CALLBACK(on_back_btn_clicked));
  gtk_builder_set_scope(builder, scope);
  gtk_builder_set_current_object(builder, G_OBJECT(userdata));
  g_object_unref(scope);
}

static void on_back_from_load_btn_clicked(GtkButton* btn, gpointer userdata)
{
  GtkEntryBuffer* entrybuffer = GTK_ENTRY_BUFFER(g_object_get_data(G_OBJECT(userdata), "campaign_name_buffer"));
  if(entrybuffer){
    g_object_unref(entrybuffer);
  }
  g_object_set_data(G_OBJECT(userdata), "campaign_name_buffer", nullptr);
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(userdata), "interfacestack");
  GtkWidget* child = gtk_stack_get_child_by_name(interfacestack, "load_campaign_page");
  gtk_stack_remove(interfacestack, child);
  gtk_stack_set_visible_child_name(interfacestack, "menu_page");
}

static void on_campaign_btn_clicked(GtkButton* btn, gpointer userdata)
{
  const char* name = gtk_button_get_label(btn);
  GtkEntryBuffer* entrybuffer = gtk_entry_buffer_new(name, -1);
  g_object_ref(entrybuffer);
  g_object_set_data(G_OBJECT(userdata), "campaign_name_buffer", entrybuffer);
  /*Swith to dms_page*/
  create_DMs_UI(0, userdata);
}

static void on_delete_campaign_btn_clicked(GtkButton* btn, gpointer userdata)
{
  campaign_to_delete* deldata = (campaign_to_delete*)userdata;
  if(deldata && !deldata->basename){
    return;
  }
  /*Delete campaign*/
  char path_to_dir[512];
  snprintf(path_to_dir, sizeof(path_to_dir), "saves/%s", deldata->basename);
  GFile* dir = g_file_new_for_path(path_to_dir);
  GError* err = nullptr;
  gboolean success = g_file_trash(dir, nullptr, &err);
  if(!success){
    g_print("Campaign %s not move to trash by error: %s\n", deldata->basename, err ? err->message : "UNKNOWN");
    g_error_free(err);
    g_object_unref(dir);
    return;
  }
  gtk_widget_unparent(deldata->box);
  g_object_unref(dir);
  return;
}
