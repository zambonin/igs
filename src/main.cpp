#include "handler.hpp"

static gboolean configure_event_cb(GtkWidget *widget, GdkEventConfigure *event,
                                   gpointer data) {
  if (surface != nullptr) {
    cairo_surface_destroy(surface);
  }

  surface = gdk_window_create_similar_surface(
      gtk_widget_get_window(widget), CAIRO_CONTENT_COLOR,
      gtk_widget_get_allocated_width(widget),
      gtk_widget_get_allocated_height(widget));
  clear_surface();

  return TRUE;
}

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data) {
  cairo_set_source_surface(cr, surface, 0, 0);
  cairo_paint(cr);

  return FALSE;
}

int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);
  builder = gtk_builder_new_from_file("src/window.glade");

  GtkWidget *drawing_area =
      GTK_WIDGET(gtk_builder_get_object(builder, "drawing_area"));

  g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_cb), nullptr);
  g_signal_connect(drawing_area, "configure-event",
                   G_CALLBACK(configure_event_cb), nullptr);

  gtk_widget_get_size_request(drawing_area, &vp_width, &vp_height);
  w.coords = std::list<coord>(
      {coord(vp_width, vp_height), coord(-vp_width, vp_height),
       coord(vp_width, -vp_height), coord(-vp_width, -vp_height)});

  gtk_builder_connect_signals(builder, nullptr);
  gtk_widget_show_all(
      GTK_WIDGET(gtk_builder_get_object(builder, "main_window")));
  gtk_main();

  return 0;
}
