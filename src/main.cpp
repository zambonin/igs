#include <gtk/gtk.h>
#include <iostream>
#include <vector>

static cairo_surface_t *surface = nullptr;
GtkBuilder *builder;
GtkWidget *drawing_area, *window_widget, *viewport, *draw_widget;

static void clear_surface() {
  cairo_t *cr = cairo_create(surface);
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_paint(cr);
  cairo_destroy(cr);
}

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

extern "C" G_MODULE_EXPORT void btn_ok_clicked_cb() {
  cairo_t *cr = cairo_create(surface);
  cairo_move_to(cr, 200, 100);
  cairo_line_to(cr, 300, 50);
  cairo_stroke(cr);
  gtk_widget_queue_draw(window_widget);
}

extern "C" G_MODULE_EXPORT void btn_addFigure_clk() {
  GtkBuilder *b = gtk_builder_new_from_file("src/window.glade");
  draw_widget = GTK_WIDGET(gtk_builder_get_object(b, "windowAddFigure"));
  gtk_builder_connect_signals(b, NULL);
  gtk_widget_show(GTK_WIDGET(draw_widget));
}

extern "C" G_MODULE_EXPORT void btn_w2cancel_clk() {
  gtk_widget_destroy(GTK_WIDGET(draw_widget));
}

extern "C" G_MODULE_EXPORT void btn_w2ok_clk() {}
extern "C" G_MODULE_EXPORT void btn_w2addCoord_clk() {}
extern "C" G_MODULE_EXPORT void btn_left_clk() {}
extern "C" G_MODULE_EXPORT void btn_zoomOut_clk() {}
extern "C" G_MODULE_EXPORT void btn_zoomIn_clk() {}
extern "C" G_MODULE_EXPORT void btn_up_clk() {}

int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);
  builder = gtk_builder_new_from_file("src/window.glade");

  window_widget = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
  viewport = GTK_WIDGET(gtk_builder_get_object(builder, "viewport1"));
  drawing_area = GTK_WIDGET(gtk_builder_get_object(builder, "drawing_area"));

  g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_cb), nullptr);
  g_signal_connect(drawing_area, "configure-event",
                   G_CALLBACK(configure_event_cb), nullptr);

  gtk_builder_connect_signals(builder, nullptr);
  gtk_widget_show_all(window_widget);
  gtk_main();

  return 0;
}
