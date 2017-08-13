#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <gtk/gtk.h>
#include <iostream>
#include <list>
#include <string>

class window {
 public:
  explicit window(double _xmax = 100, double _xmin = 0,
      double _ymax = 100, double _ymin = 0) noexcept
    : xmax(_xmax), xmin(_xmin), ymax(_ymax), ymin(_ymin)
  {
    set_limits(0, 0, 0, 0);
  }

  void set_limits(double l1, double l2, double l3, double l4) {
    this->xmax += l1;
    this->xmin += l2;
    this->ymax += l3;
    this->ymin += l4;
  }

 void zoom(double r) {
    this->xmax *= r;
    this->xmin *= r;
    this->ymax *= r;
    this->ymin *= r;
  }

  void reset() {
    this->xmax = 100;
    this->xmin = 0;
    this->ymax = 100;
    this->ymin = 0;
  }

  double xmax, xmin, ymax, ymin;
};

class coord {
 public:
  coord(double _x, double _y, double _z = 1)
    : x(_x), y(_y), z(_z) {}

  double x, y, z;

  bool operator!=(const coord& rhs) {
    return this->x != rhs.x || this->y != rhs.y;
  }

  friend std::ostream& operator<<(std::ostream& os, const coord& c) {
    return os << "x: " << c.x << " y: " << c.y << std::endl;
  }
};

class drawable {
 public:
  explicit drawable(std::string _name, const std::list<coord>& _orig)
    : name(std::move(_name)), orig(_orig), actual(_orig) {}

  void draw(cairo_t* cr) {
    auto it = std::begin(actual), end = --std::end(actual);
    while (it != end) {
      cairo_move_to(cr, (*it).x, (*it).y);
      ++it;
      cairo_line_to(cr, (*it).x, (*it).y);
    }

    cairo_set_line_width(cr, 2);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, actual.front().x, actual.front().y);
    cairo_line_to(cr, actual.back().x, actual.back().y);
    cairo_stroke(cr);
  }

  gint16 type() {
    if (orig.size() > 2) {
      return 2;
    }
    return orig.size();
  }

  void viewport(GtkWidget* area, const window& w) {
    int wid, hei;
    gtk_widget_get_size_request(area, &wid, &hei);

    auto it1 = orig.begin(), it2 = actual.begin();
    for (; it1 != orig.end(); ++it1, ++it2) {
      (*it2).x = (((*it1).x - w.xmin) / (w.xmax - w.xmin)) * (wid - 0);
      (*it2).y = (1 - (((*it1).y - w.ymin) / (w.ymax - w.ymin))) * (hei - 0);
    }
  }

  const std::string name;
  std::list<coord> orig, actual;
};

#endif // STRUCTURES_HPP
