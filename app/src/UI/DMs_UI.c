#include "UI/DMs_UI.h"

#include "UI/UI.h"

#define SCENE_MENU_UI "res/gtk_resources_pages/scene_menu.ui"
#define SCENE_UI "res/gtk_resources_pages/scene.ui"

/*Scene menu*/
static void on_file_open_ready(GObject* obj, GAsyncResult* res, gpointer userdata);

/*Side menu btns*/
void on_side_menu_btn_clicked(GtkButton* btn, gpointer userdata)
{
  gtk_widget_set_visible(GTK_WIDGET(btn), false);

  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  gtk_revealer_set_reveal_child(dms_ui->side_menu_revealer, true);
  gtk_widget_set_can_target(GTK_WIDGET(dms_ui->side_box_for_btn), false);
}

void on_side_hide_btn_clicked(GtkButton* btn, gpointer userdata)
{
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  gtk_revealer_set_reveal_child(dms_ui->side_menu_revealer, false);
  gtk_widget_set_can_target(GTK_WIDGET(dms_ui->side_box_for_btn), true);
  gtk_widget_set_visible(GTK_WIDGET(dms_ui->side_menu_btn), true);
}

/*Top menu btns*/
void on_scene_menu_btn_clicked(GtkButton* btn, gpointer userdata)
{
  GtkBuilder* scene_menu_builder = gtk_builder_new_from_file(SCENE_MENU_UI);
  GtkWidget* scene_menu_win = GTK_WIDGET(gtk_builder_get_object(scene_menu_builder, "main_scene_menu"));
  /*Set like transient*/
  gtk_window_set_transient_for(GTK_WINDOW(scene_menu_win), GTK_WINDOW(userdata));
  gtk_window_set_destroy_with_parent(GTK_WINDOW(scene_menu_win), TRUE);
  /*Linking scene menu window with main window*/
  g_object_set_data(G_OBJECT(userdata), "scene_menu_win", scene_menu_win);
  /*Linking other scene menu widget with scene menu window*/
  GtkWidget* scene_add_scene_btn = GTK_WIDGET(gtk_builder_get_object(scene_menu_builder, "add_scene_btn"));
  g_object_set_data(G_OBJECT(scene_menu_win), "add_scene_btn", scene_add_scene_btn);
  GtkWidget* scene_list_box = GTK_WIDGET(gtk_builder_get_object(scene_menu_builder, "scene_list_box"));
  g_object_set_data(G_OBJECT(scene_menu_win), "scene_list_box", scene_list_box);
  /*Connect signal to scene_add_scene_btn*/
  g_signal_connect(scene_add_scene_btn, "clicked", G_CALLBACK(on_add_scene_btn_clicked), scene_menu_win);
  /*Present scene menu window*/
  gtk_window_present(GTK_WINDOW(scene_menu_win));
  g_object_unref(scene_menu_builder);
}

/*Scene menu*/
void on_add_scene_btn_clicked(GtkButton* btn, gpointer userdata)
{
  GtkWindow* main_scene_menu_win = GTK_WINDOW(userdata);
  GtkFileDialog* dialog = gtk_file_dialog_new();
  /*Add file filter to dialog*/
  GtkFileFilter* filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "PNG Images (*.png)");
  gtk_file_filter_add_suffix(filter, "png");
  gtk_file_filter_add_suffix(filter, "jpg");
  gtk_file_dialog_set_default_filter(dialog, filter);
  g_object_unref(filter);
  /*Open file dialog*/
  gtk_file_dialog_open(dialog, main_scene_menu_win, nullptr, on_file_open_ready, main_scene_menu_win);
  g_object_unref(dialog);
}

static void on_file_open_ready(GObject* obj, GAsyncResult* res, gpointer userdata)
{
  GtkFileDialog* dialog = GTK_FILE_DIALOG(obj);
  GError* err = nullptr;
  GFile* file = gtk_file_dialog_open_finish(dialog, res, &err);
  if(err){
    g_print("Error:File is not open: %s\n", err->message);
    g_clear_error(&err);
    return;
  }
  if(file != nullptr){
    char* path = g_file_get_path(file);
    /*Get scene*/
    GtkBuilder* scene_builder = gtk_builder_new_from_file(SCENE_UI);
    GtkWidget* scene = GTK_WIDGET(gtk_builder_get_object(scene_builder, "scene_list_element"));
    GtkWidget* scene_label = GTK_WIDGET(gtk_builder_get_object(scene_builder, "scene_label"));
    GtkWidget* scene_img = GTK_WIDGET(gtk_builder_get_object(scene_builder, "scene_img"));
    /*Set scene name and img*/
    gchar* filename = g_file_get_basename(file);
    gtk_label_set_text(GTK_LABEL(scene_label), filename);
    g_free(filename);
    gtk_picture_set_file(GTK_PICTURE(scene_img), file);
    /*Add scene to list box*/
    GtkBox* list_box = GTK_BOX(g_object_get_data(G_OBJECT(userdata), "scene_list_box"));
    gtk_box_append(list_box, scene);
    /*Free*/
    g_object_unref(scene_builder);
    g_free(path);
    g_object_unref(file);
  }
}
