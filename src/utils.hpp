#ifndef UTILS_HPP
#define UTILS_HPP

#include "drawable.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>
#include <unordered_map>

static cairo_surface_t *surface = nullptr;
static GtkBuilder *builder;

static std::unordered_map<std::string, std::unique_ptr<drawable>> objects;
static window w;
static int vp_height, vp_width, lclip;

matrix<double> m_transfer(const coord &c) {
  return matrix<double>({{1, 0, 0}, {0, 1, 0}, {c.x, c.y, 1}});
}

matrix<double> m_rotate(double a) {
  return matrix<double>({{cos(a), -sin(a), 0}, {sin(a), cos(a), 0}, {0, 0, 1}});
}

matrix<double> m_scale(const coord &c) {
  return matrix<double>({{c.x, 0, 0}, {0, c.y, 0}, {0, 0, 1}});
}

std::list<coord> viewport(const std::list<coord> &cs) {
  std::list<coord> vp_coord;
  double x, y;
  for (auto &i : cs) {
    x = ((i.x + 1) / (1 - -1)) * (vp_width - 0);
    y = (1 - ((i.y + 1) / (1 - -1))) * (vp_height - 0);
    vp_coord.emplace_back(coord(x, y));
  }
  return vp_coord;
}

void transform(const matrix<double> &m, std::list<coord> &from,
               std::list<coord> &to) {
  auto it = to.begin();
  for (auto &i : from) {
    *it++ = coord((matrix<double>({{i.x, i.y, i.z}}) * m)[0]);
  }
}

void transform(const matrix<double> &m, std::list<coord> &cs) {
  transform(m, cs, cs);
}

void transform(const matrix<double> &m, coord &c) {
  c = coord((matrix<double>({{c.x, c.y, c.z}}) * m)[0]);
}

template <typename Out>
void split(const std::string &s, char delim, Out result) {
  std::stringstream ss;
  ss.str(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

std::list<coord> read_coord(const char *input) {
  std::list<coord> points;
  try {
    std::vector<std::string> cs = split(std::string(input), ' ');
    for (auto &i : cs) {
      std::vector<std::string> p = split(i, ';');
      std::vector<double> point;
      std::transform(p.begin(), p.end(), std::back_inserter(point),
                     [](const std::string &str) { return std::stod(str); });
      points.emplace_back(point);
    }
  } catch (const std::invalid_argument &ia) {
    points.clear();
  }
  return points;
}

polygon read_obj(const std::string &path) {
  std::ifstream file(path);
  std::string l;
  std::vector<std::string> line;

  const std::string name = split(path, '/').back();
  std::vector<std::vector<int>> faces;
  std::list<coord> points;

  try {
    while (std::getline(file, l)) {
      line = split(l, ' ');
      if (line.size() == 0) {
        continue;
      } else if (line[0] == "f") {
        std::vector<int> face;
        std::transform(line.begin() + 1, line.end(), std::back_inserter(face),
                       [](const std::string &str) { return std::stoi(str); });
        faces.emplace_back(face);
      } else if (line[0] == "v") {
        std::vector<double> point;
        std::transform(line.begin() + 1, line.end(), std::back_inserter(point),
                       [](const std::string &str) { return std::stod(str); });
        points.emplace_back(point);
      }
    }
    faces.at(0);
  } catch (const std::invalid_argument &ia) {
    return polygon(name, {});
  } catch (const std::out_of_range &o) {
    return polygon(name, {});
  }
  return polygon(name, points, matrix<int>(faces));
}

void write_obj(const drawable &d) { std::ofstream(d.name + ".obj") << d; }

void clear_surface() {
  cairo_t *cr = cairo_create(surface);
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_paint(cr);
  cairo_destroy(cr);
}

void update() {
  clear_surface();
  cairo_t *cr = cairo_create(surface);
  polygon border("__window", {coord(10, 10), coord(vp_width - 10, 10),
                              coord(vp_width - 10, vp_height - 10),
                              coord(10, vp_height - 10)});
  border.draw(cr, border.orig);

  for (auto &obj : objects) {
    transform(m_transfer(-w.center) * m_rotate(-w.angle) *
                  m_scale(coord(1 / w.wid, 1 / w.hei)),
              obj.second->orig, obj.second->scn);
    obj.second->draw(cr, viewport(obj.second->clip()));
  }

  gtk_widget_queue_draw(
      GTK_WIDGET(gtk_builder_get_object(builder, "main_window")));
  cairo_destroy(cr);
}

void pan(const coord &c) {
  transform(m_transfer(c), w.center);
  update();
}

void rotate(const double angle) {
  w.angle += (M_PI / 180) * angle;
  update();
}

void zoom(const double rate) {
  w.wid *= rate;
  w.hei *= rate;
  update();
}

#endif // UTILS_HPP
