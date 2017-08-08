#include "structures.hpp"
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>

static cairo_surface_t *surface = nullptr;
GtkBuilder *builder;
GtkWidget *drawing_area, *window_widget, *viewport, *draw_widget;

std::list<coord> split(const char *input) {
  std::list<coord> c;
  std::list<double> tmp;

  std::istringstream iss1(input);
  std::string s1, s2;

  while (getline(iss1, s1, ' ')) {
    std::istringstream iss2(s1);
    while (getline(iss2, s2, ';')) {
      tmp.push_back(std::stod(s2));
    }
    c.push_back(coord(tmp.front(), tmp.back()));
    tmp.clear();
  }

  return c;
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

extern "C" G_MODULE_EXPORT void btn_add_figure_clk() {
  // TODO can't click twice
  GtkBuilder *b = gtk_builder_new_from_file("src/window.glade");
  draw_widget = GTK_WIDGET(gtk_builder_get_object(b, "windowAddFigure"));
  gtk_builder_connect_signals(b, NULL);
  gtk_widget_show(GTK_WIDGET(draw_widget));
}

extern "C" G_MODULE_EXPORT void btn_w2cancel_clk() {
  gtk_widget_destroy(GTK_WIDGET(draw_widget));
}

extern "C" G_MODULE_EXPORT void btn_draw_figure_clk(GtkWidget *widget,
                                                    GtkWidget *entry) {
  std::list<coord> c = split(gtk_entry_get_text(GTK_ENTRY(entry)));
  cairo_t *cr = cairo_create(surface);
  cairo_set_line_width(cr, 2);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

  // TODO add entry for name
  drawable p(std::string("placeholder"), cr, c);

  // TODO add drawable master list
  // master_list.push_back(p);

  p.draw();

  gtk_widget_queue_draw(window_widget);
  gtk_widget_destroy(draw_widget);
}

extern "C" G_MODULE_EXPORT void btn_pan_up_clk() {}
extern "C" G_MODULE_EXPORT void btn_pan_left_clk() {}
extern "C" G_MODULE_EXPORT void btn_pan_right_clk() {}
extern "C" G_MODULE_EXPORT void btn_pan_down_clk() {}
extern "C" G_MODULE_EXPORT void btn_zoom_in_clk() {}
extern "C" G_MODULE_EXPORT void btn_zoom_out_clk() {}
extern "C" G_MODULE_EXPORT void btn_exit_clk() { gtk_main_quit(); }

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
