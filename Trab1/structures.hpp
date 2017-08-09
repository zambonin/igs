#pragma once

#include <list>
#include <string>

class coord {
 public:
  coord(double _x, double _y, double _z = 1)
    : x(_x), y(_y), z(_z) {}

  double x, y, z;

  bool operator!=(const coord& rhs) {
    return this->x != rhs.x || this->y != rhs.y;
  }
};

class drawable {
 public:
  explicit drawable(std::string _name, cairo_t *_cr, std::list<coord> _clist)
    : name(_name), cr(_cr), clist(_clist) {}

  void draw() {
    auto it = std::begin(clist), end = --std::end(clist);
    while (it != end) {
      cairo_move_to(cr, (*it).x, (*it).y);
      ++it;
      cairo_line_to(cr, (*it).x, (*it).y);
    }

    cairo_move_to(cr, clist.front().x, clist.front().y);
    cairo_line_to(cr, clist.back().x, clist.back().y);
    cairo_stroke(cr);
  }

  short type() {
    return clist.size() ? (clist.size() < 3) : 2;
  }

  const std::string name;
  cairo_t *cr;
  std::list<coord> clist;
};

class window {
 public:
  window(double _xmax = 100, double _xmin = 0,
      double _ymax = 100, double _ymin = 0)
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

 void zoom(double l1, double l2, double l3, double l4) {
    this->xmax *= l1;
    this->xmin *= l2;
    this->ymax *= l3;
    this->ymin *= l4;
  }

  double xmax, xmin, ymax, ymin;
};
