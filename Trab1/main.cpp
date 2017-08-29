#include "utils.hpp"

#include <map>

static cairo_surface_t *surface = nullptr;
GtkBuilder *builder;
GtkWidget *drawing_area, *window_widget;

std::map<std::string, drawable> objects;
window *w;
int vport_wid, vport_hei;

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

void draw_border(int borderSize) {
  clear_surface();
  cairo_t *cr = cairo_create(surface);
  drawable border("border",
                  {coord(borderSize, borderSize),
                   coord(vport_wid - borderSize, borderSize),
                   coord(vport_wid - borderSize, vport_hei - borderSize),
                   coord(borderSize, vport_hei - borderSize)});
  border.transform_normalize(
      m_transfer(-coord(w->wCenterX, w->wCenterY, 1)) *
      m_rotate(-1 * w->angle) *
      m_scale(coord(2 / (w->xmax - w->xmin), 2 / (w->ymax - w->ymin), 1)));

  border.draw(cr);
  gtk_widget_queue_draw(window_widget);
  cairo_destroy(cr);
}

void update() {
  clear_surface();
  draw_border(10);
  cairo_t *cr = cairo_create(surface);
  for (auto &obj : objects) {
    obj.second.transform_normalize(
        m_transfer(-coord(w->wCenterX, w->wCenterY, 1)) *
        m_rotate(-1 * w->angle) *
        m_scale(coord(2 / (w->xmax - w->xmin), 2 / (w->ymax - w->ymin), 1)));
    obj.second.viewport(drawing_area);
    if (obj.second.line_clipping(vport_wid, vport_hei))
      obj.second.draw(cr);
  }
  gtk_widget_queue_draw(window_widget);
  cairo_destroy(cr);
}

extern "C" G_MODULE_EXPORT void btn_draw_figure_clk() {
  GtkEntry *name = GTK_ENTRY(gtk_builder_get_object(builder, "name")),
           *coor = GTK_ENTRY(gtk_builder_get_object(builder, "coord"));
  GtkComboBoxText *combo =
      GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "combo"));

  std::string s(gtk_entry_get_text(name));
  std::list<coord> c = split(gtk_entry_get_text(coor));

  if (!s.empty() && !c.empty() && objects.count(s) == 0) {
    gtk_combo_box_text_append(combo, nullptr, s.c_str());
    objects.insert({s, drawable(s, c)});
    update();
  }

  gtk_entry_set_text(name, "");
  gtk_entry_set_text(coor, "");
}

extern "C" G_MODULE_EXPORT void btn_rotateleft_clk(GtkWidget *widget,
                                                   GtkWidget *entry) {
  const double rate =
      M_PI * std::stod(gtk_entry_get_text(GTK_ENTRY(entry))) / 180;
  w->rotate(rate);
  update();
}

extern "C" G_MODULE_EXPORT void btn_rotateright_clk(GtkWidget *widget,
                                                    GtkWidget *entry) {
  const double rate =
      M_PI * std::stod(gtk_entry_get_text(GTK_ENTRY(entry))) / 180;
  w->rotate(-rate);
  update();
}

extern "C" G_MODULE_EXPORT void btn_pan_up_clk(GtkWidget *widget,
                                               GtkWidget *entry) {
  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w->transform(m_transfer(-coord(w->wCenterX, w->wCenterY)) *
               m_rotate(-w->angle) * m_transfer(coord(0, rate)) *
               m_rotate(w->angle) *
               m_transfer(coord(w->wCenterX, w->wCenterY)));
  update();
}

extern "C" G_MODULE_EXPORT void btn_pan_left_clk(GtkWidget *widget,
                                                 GtkWidget *entry) {
  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w->transform(m_transfer(-coord(w->wCenterX, w->wCenterY)) *
               m_rotate(-w->angle) * m_transfer(coord(-rate, 0)) *
               m_rotate(w->angle) *
               m_transfer(coord(w->wCenterX, w->wCenterY)));
  update();
}

extern "C" G_MODULE_EXPORT void btn_pan_right_clk(GtkWidget *widget,
                                                  GtkWidget *entry) {
  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w->transform(m_transfer(-coord(w->wCenterX, w->wCenterY)) *
               m_rotate(-w->angle) * m_transfer(coord(rate, 0)) *
               m_rotate(w->angle) *
               m_transfer(coord(w->wCenterX, w->wCenterY)));
  update();
}

extern "C" G_MODULE_EXPORT void btn_pan_down_clk(GtkWidget *widget,
                                                 GtkWidget *entry) {
  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w->transform(m_transfer(-coord(w->wCenterX, w->wCenterY)) *
               m_rotate(-w->angle) * m_transfer(coord(0, -rate)) *
               m_rotate(w->angle) *
               m_transfer(coord(w->wCenterX, w->wCenterY)));
  update();
}

extern "C" G_MODULE_EXPORT void btn_zoom_out_clk(GtkWidget *widget,
                                                 GtkWidget *entry) {
  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w->transform(m_transfer(-coord(w->wCenterX, w->wCenterY)) *
               m_scale(coord(1 + rate, 1 + rate)) *
               m_transfer(coord(w->wCenterX, w->wCenterY)));

  update();
}

extern "C" G_MODULE_EXPORT void btn_zoom_in_clk(GtkWidget *widget,
                                                GtkWidget *entry) {
  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w->transform(m_transfer(-coord(w->wCenterX, w->wCenterY)) *
               m_scale(coord(1 - rate, 1 - rate)) *
               m_transfer(coord(w->wCenterX, w->wCenterY)));
  update();
}

extern "C" G_MODULE_EXPORT void btn_exit_clk() { gtk_main_quit(); }

extern "C" G_MODULE_EXPORT void btn_clear_clk(GtkWidget *widget,
                                              GtkWidget *combo) {
  clear_surface();
  objects.clear();
  w->reset();
  gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(combo));
  gtk_widget_queue_draw(window_widget);
}

extern "C" G_MODULE_EXPORT void btn_center_clk() {
  w->reset();
  update();
}

extern "C" G_MODULE_EXPORT void btn_trans_figure_clk(GtkWidget *widget,
                                                     GtkWidget *entry) {
  GtkComboBoxText *combo =
      GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "combo"));
  gchar *obj = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));

  GtkComboBoxText *ops =
      GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "operations"));
  int op_id = gtk_combo_box_get_active(GTK_COMBO_BOX(ops));

  GtkEntry *vector = GTK_ENTRY(gtk_builder_get_object(builder, "transfer"));
  std::list<coord> c = split(gtk_entry_get_text(vector));

  if (obj == nullptr || (c.empty() && op_id < 3)) {
    return;
  }

  const double angle =
      M_PI * std::stod(gtk_entry_get_text(GTK_ENTRY(entry))) / 180;

  drawable &d = objects.find(obj)->second;
  std::map<int, matrix<double>> bases = {
    {0, m_transfer(c.front())},
    {1, m_transfer(-d.center()) * m_scale(c.front()) * m_transfer(d.center())},
    {2, m_transfer(-c.front()) * m_rotate(angle) * m_transfer(c.front())},
    {3, m_transfer(coord(0, 0)) * m_rotate(angle)},
    {4, m_transfer(-d.center()) * m_rotate(angle) * m_transfer(d.center())},
  };

  d.transform(bases.find(op_id)->second);
  update();
}

extern "C" G_MODULE_EXPORT void btn_delete_figure_clk(GtkWidget *widget,
                                                      GtkWidget *combo) {
  gchar *obj = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
  if (obj != nullptr) {
    gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(combo),
                              gtk_combo_box_get_active(GTK_COMBO_BOX(combo)));
    objects.erase(obj);
  }
  update();
}

extern "C" G_MODULE_EXPORT void btn_add_obj_figure(GtkWidget *widget,
                                                   GtkWidget *combo) {
  gchar *file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
  drawable d = read_obj(file);
  if (objects.count(d.name) == 0) {
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), nullptr,
                              d.name.c_str());
    objects.insert({d.name, d});
    update();
  }
}

extern "C" G_MODULE_EXPORT void btn_save_obj(GtkWidget *widget,
                                             GtkWidget *combo) {
  gchar *obj = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
  if (obj != nullptr) {
    std::string p(obj);
    write_obj(p, objects.find(p)->second);
  }
}

int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);
  builder = gtk_builder_new_from_file("src/window.glade");

  window_widget = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
  drawing_area = GTK_WIDGET(gtk_builder_get_object(builder, "drawing_area"));

  g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_cb), nullptr);
  g_signal_connect(drawing_area, "configure-event",
                   G_CALLBACK(configure_event_cb), nullptr);

  gtk_widget_get_size_request(drawing_area, &vport_wid, &vport_hei);
  w = new window(
      {coord(vport_wid, vport_hei), coord(vport_wid, -vport_hei), coord(-vport_wid, vport_hei), coord(-vport_wid, -vport_hei)});
  gtk_builder_connect_signals(builder, nullptr);
  gtk_widget_show_all(window_widget);
  gtk_main();

  return 0;
}
