#ifndef UTILS_HPP
#define UTILS_HPP

#include "structures.hpp"

#include <cmath>
#include <map>
#include <sstream>

static cairo_surface_t *surface = nullptr;
static GtkBuilder *builder;

static std::map<std::string, drawable> objects;
static window w;

inline matrix m_transfer(const coord &c) {
  return matrix({{1, 0, 0}, {0, 1, 0}, {c.x, c.y, 1}});
}

inline matrix m_rotate(double a) {
  return matrix({{cos(a), -sin(a), 0}, {sin(a), cos(a), 0}, {0, 0, 1}});
}

inline matrix m_scale(const coord &c) {
  return matrix({{c.x, 0, 0}, {0, c.y, 0}, {0, 0, 1}});
}

inline std::list<coord> split(const char *input) {
  std::list<coord> c;
  std::list<double> tmp;

  std::istringstream iss1(input);
  std::string s1, s2;

  while (getline(iss1, s1, ' ')) {
    std::istringstream iss2(s1);
    while (getline(iss2, s2, ';')) {
      try {
        tmp.push_back(std::stod(s2));
      } catch (const std::invalid_argument &ia) {
      }
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

inline void clear_surface() {
  cairo_t *cr = cairo_create(surface);
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_paint(cr);
  cairo_destroy(cr);
}

inline void update() {
  clear_surface();
  cairo_t *cr = cairo_create(surface);

  for (auto &obj : objects) {
    obj.second.viewport(
        GTK_WIDGET(gtk_builder_get_object(builder, "drawing_area")), w);
    obj.second.draw(cr);
  }

  gtk_widget_queue_draw(
      GTK_WIDGET(gtk_builder_get_object(builder, "main_window")));
  cairo_destroy(cr);
}

#endif // UTILS_HPP
