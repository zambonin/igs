#include <cmath>
#include <gtk/gtk.h>
#include <iostream>
#include <sstream>
#include <vector>

static cairo_surface_t *surface = nullptr;
GtkBuilder *builder;
GtkWidget *drawing_area, *window_widget, *viewport, *draw_widget;

std::vector<float> split(GtkWidget *entry) {
  std::vector<float> vect;
  std::stringstream ss(gtk_entry_get_text(GTK_ENTRY(entry)));
  float i;

  while (ss >> i) {
    vect.push_back(i);
    if (ss.peek() == ';') {
      ss.ignore();
    }
  }

  return vect;
}

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

extern "C" G_MODULE_EXPORT void btn_addFigure_clk() {
  GtkBuilder *b = gtk_builder_new_from_file("window.glade");
  draw_widget = GTK_WIDGET(gtk_builder_get_object(b, "windowAddFigure"));
  gtk_builder_connect_signals(b, NULL);
  gtk_widget_show(GTK_WIDGET(draw_widget));
}

extern "C" G_MODULE_EXPORT void btn_w2cancel_clk() {
  gtk_widget_destroy(GTK_WIDGET(draw_widget));
}

extern "C" G_MODULE_EXPORT void btn_w2okDot_clk(GtkWidget *widget,
                                                GtkWidget *ud) {
  std::vector<float> vect = split(ud);
  cairo_t *cr = cairo_create(surface);

  cairo_translate(cr, vect.at(0), vect.at(1));
  cairo_arc(cr, 0, 0, 1, 0, 2 * M_PI);
  cairo_fill(cr);

  gtk_widget_queue_draw(window_widget);
  gtk_widget_destroy(draw_widget);
}

extern "C" G_MODULE_EXPORT void btn_w2okLine_clk(GtkWidget *widget,
                                                 GtkWidget *ud) {
  std::vector<float> vect = split(ud);
  cairo_t *cr = cairo_create(surface);

  cairo_move_to(cr, vect.at(0), vect.at(1));
  cairo_line_to(cr, vect.at(2), vect.at(3));
  cairo_stroke(cr);

  gtk_widget_queue_draw(window_widget);
  gtk_widget_destroy(draw_widget);
}

extern "C" G_MODULE_EXPORT void btn_w2okPolygon_clk() {}
extern "C" G_MODULE_EXPORT void btn_w2ok_clk() {}
extern "C" G_MODULE_EXPORT void btn_w2addCoord_clk() {}
extern "C" G_MODULE_EXPORT void btn_left_clk() {}
extern "C" G_MODULE_EXPORT void btn_zoomOut_clk() {}
extern "C" G_MODULE_EXPORT void btn_zoomIn_clk() {}
extern "C" G_MODULE_EXPORT void btn_up_clk() {}
extern "C" G_MODULE_EXPORT void btn_exit_clk() { gtk_main_quit(); }

int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);
  builder = gtk_builder_new_from_file("window.glade");

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
