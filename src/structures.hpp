#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <gtk/gtk.h>
#include <iostream>
#include <list>
#include <numeric>
#include <string>
#include <vector>

class coord {
public:
  coord(double _x, double _y, double _z = 1) : x(_x), y(_y), z(_z) {}

  bool operator!=(const coord &rhs) {
    return this->x != rhs.x || this->y != rhs.y;
  }

  coord operator+(const coord &c) { return coord(x + c.x, y + c.y, z + c.z); }

  coord operator+=(const coord &c) { return (*this) + c; }

  coord operator-() { return coord(-x, -y, -z); }

  coord operator/(double s) { return coord(x / s, y / s, z / s); }

  friend std::ostream &operator<<(std::ostream &os, const coord &c) {
    return os << "x: " << c.x << " y: " << c.y << std::endl;
  }

  double x, y, z;
};

class matrix {
public:
  explicit matrix(int _l = 3, int _c = 3)
      : l(_l), c(_c), elem(_l, std::vector<double>(_c, 0)) {}

  explicit matrix(std::vector<std::vector<double>> e)
      : l(e.size()), c(e[0].size()), elem(e) {}

  std::vector<double> &operator[](int i) { return elem[i]; }

  const std::vector<double> &operator[](int i) const { return elem[i]; }

  matrix operator+(const matrix &m) {
    if (l == m.l && c == m.c) {
      matrix r(3, 3);
      for (int i = 0; i < l; ++i) {
        for (int j = 0; i < c; ++i) {
          r[i][j] += elem[i][j] + m[i][j];
        }
      }
      return r;
    }
    return *this;
  }

  matrix operator*(const matrix &m) {
    matrix r(l, m.c);
    for (int i = 0; i < l; ++i) {
      for (int j = 0; j < m.c; ++j) {
        double mul = 0;
        for (int k = 0; k < m.l; ++k) {
          mul += elem[i][k] * m[k][j];
        }
        r[i][j] = mul;
      }
    }
    return r;
  }

  friend std::ostream &operator<<(std::ostream &os, const matrix &m) {
    for (int i = 0; i < m.l; ++i) {
      for (int j = 0; j < m.c; ++j) {
        os << m[i][j] << " ";
      }
      os << std::endl;
    }
    return os;
  }

  int l, c;
  std::vector<std::vector<double>> elem;
};

class window {
public:
  void update_coord() {
    auto frst = this->coords.front();
    auto last = this->coords.back();
    this->xmax = frst.x;
    this->ymax = frst.y;
    this->xmin = last.x;
    this->ymin = last.y;
  }

  void reset(int wid, int hei) {
    this->angle = 0;
    this->xmax = wid;
    this->xmin = -wid;
    this->ymax = hei;
    this->ymin = -hei;
  }

  coord center() {
    return coord(this->xmax + this->xmin, this->ymax + this->ymin) / 2;
  }

  double xmax, xmin, ymax, ymin, angle;
  std::list<coord> coords;
};

class drawable {
public:
  explicit drawable(std::string _name, const std::list<coord> &_orig)
      : name(std::move(_name)), orig(_orig), actual(_orig) {}

  void draw(cairo_t *cr, const std::list<coord> &points) {
    auto it = std::begin(points), end = --std::end(points);
    while (it != end) {
      cairo_move_to(cr, (*it).x, (*it).y);
      ++it;
      cairo_line_to(cr, (*it).x, (*it).y);
    }

    cairo_set_line_width(cr, 2);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, points.front().x, points.front().y);
    cairo_line_to(cr, points.back().x, points.back().y);
    cairo_stroke(cr);
  }

  gint16 type() {
    if (orig.size() > 2) {
      return 2;
    }
    return orig.size();
  }

  coord center() {
    coord sum = std::accumulate(orig.begin(), orig.end(), coord(0, 0));
    return sum / orig.size();
  }

  const std::string name;
  std::list<coord> orig, actual;
};

#endif // STRUCTURES_HPP
