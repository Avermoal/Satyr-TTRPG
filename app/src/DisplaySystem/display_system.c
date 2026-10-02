#include "DisplaySystem/display_system.h"

#include <stdlib.h>
#include<stdio.h>

#include "Map/map.h"

void onrealize(GtkGLArea* area, gpointer userdata)
{
  printf("Q\n");
  /*Make area current*/
  gtk_gl_area_make_current(area);
  if(gtk_gl_area_get_error(area) != NULL){
    return;
  }
  /*Get map*/
  struct map** m = (struct map**)g_object_get_data(G_OBJECT(userdata), "map_p");
  /*Create map or load map*/
  *m = (struct map*)malloc(sizeof(struct map));
  if(!m){
    g_print("Map memory allocation failed\n");
  }
  createmap(*m);
  printf("A\n");
}

void onunrealize(GtkGLArea* area, gpointer userdata)
{
  gtk_gl_area_make_current(area);
  if(gtk_gl_area_get_error(area) != NULL){
    return;
  }
  /*Get map*/
  struct map** m = (struct map**)g_object_get_data(G_OBJECT(userdata), "map_p");
  /*Uplaod map*/
  uploadmap(*m, "SOMEPATH");
  /*Free*/
  destroymap(*m);
  printf("1\n");
}

gboolean onrender(GtkGLArea* area, GdkGLContext* context, gpointer userdata)
{
  printf("0\n");
  /*Make area current*/
  gtk_gl_area_make_current(area);
  if(gtk_gl_area_get_error(area) != NULL){
    return FALSE;
  }
  /*Get area sizes*/
  int width = gtk_widget_get_width(GTK_WIDGET(area));
  int height = gtk_widget_get_height(GTK_WIDGET(area));
  if(width <= 0 || height <= 0){
    return FALSE;
  }
  /*Get map*/
  struct map** m = (struct map**)g_object_get_data(G_OBJECT(userdata), "map_p");
  /*Render map*/
  rendermap(*m, width, height);
  /*Update map*/
  updatemap(*m);
  return TRUE;
}
