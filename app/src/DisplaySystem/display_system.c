#include "DisplaySystem/display_system.h"

#include <stdlib.h>

#include <epoxy/gl.h>

#include "Map/map.h"

void onrealize(GtkGLArea* area, gpointer userdata)
{
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
}

gboolean onrender(GtkGLArea* area, GdkGLContext* context, gpointer userdata)
{
  /*Make area current*/
  gtk_gl_area_make_current(area);
  if(gtk_gl_area_get_error(area) != NULL){
    return FALSE;
  }
  /*GL NEEDS*/
  glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
  return TRUE;
}

gboolean ontick(GtkWidget* widget, GdkFrameClock* clock, gpointer userdata)
{
  /*Gets*/
  struct map** m = (struct map**)g_object_get_data(G_OBJECT(userdata), "map_p");
  /*Get time*/

  /*Update*/
  updatemap(*m);
  /*Let's DRAW*/
  gtk_widget_queue_draw(widget);
  return TRUE;
}
