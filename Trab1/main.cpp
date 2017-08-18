#include "structures.hpp"
#include <cmath>
#include <map>
#include <sstream>

static cairo_surface_t *surface = nullptr;

GtkWidget *drawing_area, *window_widget, *combo;
GtkBuilder *builder;

matrix m_transfer(const coord& c) {
  return matrix({{1, 0, 0}, {0, 1, 0}, {c.x, c.y, 1}});
}

matrix m_rotate(double a) {
  return matrix({{cos(a), -sin(a), 0}, {sin(a), cos(a), 0}, {0, 0, 1}});
}

matrix m_scale(const coord& c) {
  return matrix({{c.x, 0, 0}, {0, c.y, 0}, {0, 0, 1}});
}

std::map<std::string, drawable> objects;
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
        c.emplace_back(coord(tmp.front(), tmp.back()));

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

    if (surface != nullptr) {
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

void update() {

    clear_surface();
    cairo_t *cr = cairo_create(surface);
    for (auto& obj : objects) {
        obj.second.viewport(drawing_area, w);
        obj.second.draw(cr);
    }
    gtk_widget_queue_draw(window_widget);
    cairo_destroy(cr);

}

extern "C" G_MODULE_EXPORT void btn_trans_clk(
    GtkWidget *widget, GtkWidget *entry) {

  GtkEntry *data = GTK_ENTRY(gtk_builder_get_object(builder, "transferVector"));
  std::list<coord> c = split(gtk_entry_get_text(data));
  gchar *obj = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));

  if (obj && c.size() == 1) {
    objects.find(obj)->second.transform(m_transfer(c.front()));
    update();
  }

}

extern "C" G_MODULE_EXPORT void btn_scale_clk(
    GtkWidget *widget, GtkWidget *entry) {

  GtkEntry *data = GTK_ENTRY(gtk_builder_get_object(builder, "scaleFactor"));
  std::list<coord> c = split(gtk_entry_get_text(data));
  gchar *obj = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));

  if (obj && c.size() == 1) {
    drawable& d = objects.find(obj)->second;
    d.transform(
        m_transfer(-d.center()) * m_scale(c.front()) * m_transfer(d.center()));
    update();
  }

}


extern "C" G_MODULE_EXPORT void btn_rotate_clk(
    GtkWidget *widget, GtkWidget *entry) {

  GtkEntry *data = GTK_ENTRY(gtk_builder_get_object(builder, "rotationDegree"));
  gchar *obj = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));

  GtkRadioButton *rotate_bycenter = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "buttonByCenter"));
  GtkRadioButton *rotate_bypoint = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "buttonByPoint"));
  GtkRadioButton *rotate_byworld = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "buttonByWorld"));

  if (obj) {
    try {
      double a = M_PI * std::stod(gtk_entry_get_text(data)) / 180;
      drawable& d = objects.find(obj)->second;
      if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rotate_bycenter))) {
        d.transform(
          m_transfer(-d.center()) * m_rotate(a) * m_transfer(d.center()));
      } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rotate_bypoint))) {
        GtkEntry *data_point = GTK_ENTRY(gtk_builder_get_object(builder, "transferVector"));
        std::list<coord> c = split(gtk_entry_get_text(data_point));

        d.transform(
          m_transfer(-c.front()) * m_rotate(a) * m_transfer(c.front()));
      } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rotate_byworld))) {

        d.transform(
          m_rotate(a));
      } else {
          std::cout << "Select an option: " << std::endl;
      }
      update();
    } catch (const std::invalid_argument& ia) {}

    // TODO Rotate based on any point.
    // TODO utils.hpp (split and matrix functions)

  }

}

extern "C" G_MODULE_EXPORT void btn_draw_figure_clk() {

  GtkEntry *name = GTK_ENTRY(gtk_builder_get_object(builder, "name")),
           *coor = GTK_ENTRY(gtk_builder_get_object(builder, "coord"));

  std::string name_entry = std::string(gtk_entry_get_text(name));
  std::list<coord> c = split(gtk_entry_get_text(coor));

  if (!name_entry.empty() && !c.empty()) {
    std::string s(name_entry);
    objects.insert(std::pair<std::string, drawable>(s, drawable(s, c)));
    update();

    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, s.c_str());
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
    objects.clear();
    w.reset();
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(combo));
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

    combo = GTK_WIDGET(gtk_builder_get_object(builder, "comboBox"));

    g_signal_connect(drawing_area, "draw",
            G_CALLBACK(draw_cb), nullptr);
    g_signal_connect(drawing_area, "configure-event",
            G_CALLBACK(configure_event_cb), nullptr);

    gtk_builder_connect_signals(builder, nullptr);
    gtk_widget_show_all(window_widget);
    gtk_main();

    return 0;

}
