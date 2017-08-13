#include <algorithm>
#include <gtk/gtk.h>
#include <iostream>
#include <sstream>
#include "structures.hpp"

static cairo_surface_t *surface = nullptr;

GtkWidget *drawing_area, *window_widget;
GtkBuilder *builder;

std::list<drawable> master_list;
window w;

std::list<coord> split(const char* input) {

  std::list<coord> c;
  std::list<double> tmp;

  std::istringstream iss1(input);
  std::string s1, s2;

  while(getline(iss1, s1, ' ')) {
    std::istringstream iss2(s1);
    while(getline(iss2, s2, ';')) {
      try {
        tmp.push_back(std::stod(s2));
      } catch (const std::invalid_argument& ia) {}
    }
    c.push_back(coord(tmp.front(), tmp.back()));
    if (tmp.size() != 2) {
      c.clear();
      return c;
    }
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

static gboolean configure_event_cb(GtkWidget *widget,
    GdkEventConfigure *event, gpointer data) {

  if (surface) {
    cairo_surface_destroy(surface);
  }

  surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget),
      CAIRO_CONTENT_COLOR, gtk_widget_get_allocated_width(widget),
      gtk_widget_get_allocated_height(widget));
  clear_surface();

  return TRUE;

}

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data) {

  cairo_set_source_surface(cr, surface, 0, 0);
  cairo_paint(cr);

  return FALSE;

}

coord viewport_tr(coord& c) {

  int width, height;
  gtk_widget_get_size_request(drawing_area, &width, &height);

  double xvp = ((c.x - w.xmin) / (w.xmax - w.xmin)) * (width - 0);
  double yvp = (1 - ((c.y - w.ymin) / (w.ymax - w.ymin))) * (height - 0);

  return coord(xvp, yvp);

}

void update() {

  clear_surface();
  for (auto& obj : master_list) {
    std::list<coord> tr = obj.clist;
    std::transform(tr.begin(), tr.end(), tr.begin(), viewport_tr);
    drawable d(obj.name + "_copy", cairo_create(surface), tr);
    d.draw();
    gtk_widget_queue_draw(window_widget);
  }

}

extern "C" G_MODULE_EXPORT void btn_draw_figure_clk() {

  GtkEntry *name = GTK_ENTRY(gtk_builder_get_object(builder, "name")),
           *coor = GTK_ENTRY(gtk_builder_get_object(builder, "coord"));

  std::string name_entry = std::string(gtk_entry_get_text(name));
  const char *coord_entry = gtk_entry_get_text(coor);
  std::list<coord> c = split(coord_entry);

  if (name_entry.size() && c.size()) {
    drawable d(std::string(name_entry), cairo_create(surface), c);
    master_list.push_back(d);
    update();
  }

  gtk_entry_set_text(name, "");
  gtk_entry_set_text(coor, "");

}

extern "C" G_MODULE_EXPORT void btn_pan_up_clk(
    GtkWidget *widget, GtkWidget *entry) {

  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w.set_limits(0, 0, rate, rate);
  update();

}

extern "C" G_MODULE_EXPORT void btn_pan_left_clk(
    GtkWidget *widget, GtkWidget *entry) {

  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w.set_limits(-1 * rate, -1 * rate, 0, 0);
  update();

}

extern "C" G_MODULE_EXPORT void btn_pan_right_clk(
    GtkWidget *widget, GtkWidget *entry) {

  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w.set_limits(rate, rate, 0, 0);
  update();

}

extern "C" G_MODULE_EXPORT void btn_pan_down_clk(
    GtkWidget *widget, GtkWidget *entry) {

  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w.set_limits(0, 0, -1 * rate, -1 * rate);
  update();

}

extern "C" G_MODULE_EXPORT void btn_zoom_out_clk(
    GtkWidget *widget, GtkWidget *entry) {

  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w.zoom(1 + rate);
  update();

}

extern "C" G_MODULE_EXPORT void btn_zoom_in_clk(
    GtkWidget *widget, GtkWidget *entry) {

  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w.zoom(1 - rate);
  update();

}

extern "C" G_MODULE_EXPORT void btn_exit_clk() {
  gtk_main_quit();
}

extern "C" G_MODULE_EXPORT void btn_clear_clk() {
  clear_surface();
  master_list.clear();
  w.reset();
  gtk_widget_queue_draw(window_widget);
}

extern "C" G_MODULE_EXPORT void btn_center_clk() {
  w.reset();
  update();
}

int main(int argc, char *argv[]) {

  gtk_init(&argc, &argv);

  builder = gtk_builder_new_from_file("src/window.glade");
  window_widget = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
  drawing_area = GTK_WIDGET(gtk_builder_get_object(builder, "drawing_area"));

  g_signal_connect(drawing_area, "draw",
      G_CALLBACK(draw_cb), nullptr);
  g_signal_connect(drawing_area, "configure-event",
      G_CALLBACK(configure_event_cb), nullptr);

  gtk_builder_connect_signals(builder, nullptr);
  gtk_widget_show_all(window_widget);
  gtk_main();

  return 0;

}
