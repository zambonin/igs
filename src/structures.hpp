#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <gtk/gtk.h>
#include <iomanip>
#include <iostream>
#include <list>
#include <numeric>
#include <string>
#include <vector>

class coord {
public:
  coord(double _x, double _y, double _z = 1) : x(_x), y(_y), z(_z) {}

  coord(std::vector<double> c) : x(c[0]), y(c[1]), z(1) {}

  bool operator!=(const coord &rhs) {
    return this->x != rhs.x || this->y != rhs.y;
  }

  coord operator+(const coord &c) { return coord(x + c.x, y + c.y, z + c.z); }

  coord operator+=(const coord &c) { return (*this) + c; }

  coord operator-() { return coord(-x, -y, -z); }

  coord operator/(double s) { return coord(x / s, y / s, z / s); }

  friend std::ostream &operator<<(std::ostream &os, const coord &c) {
    os << std::setprecision(5) << std::fixed;
    return os << "v " << c.x << " " << c.y << " " << c.z << std::endl;
  }

  double x, y, z;
};

template <typename T> class matrix {
public:
  explicit matrix(int _l = 3, int _c = 3)
      : l(_l), c(_c), elem(_l, std::vector<T>(_c, 0)) {}

  explicit matrix(std::vector<std::vector<T>> e)
      : l(e.size()), c(e[0].size()), elem(e) {}

  std::vector<T> &operator[](int i) { return elem[i]; }

  const std::vector<T> &operator[](int i) const { return elem[i]; }

  matrix<T> operator+(const matrix &m) {
    if (l == m.l && c == m.c) {
      matrix<T> r(3, 3);
      for (int i = 0; i < l; ++i) {
        for (int j = 0; i < c; ++i) {
          r[i][j] += elem[i][j] + m[i][j];
        }
      }
      return r;
    }
    return *this;
  }

  matrix<T> operator*(const matrix &m) {
    matrix<T> r(l, m.c);
    for (int i = 0; i < l; ++i) {
      for (int j = 0; j < m.c; ++j) {
        T mul = 0;
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
      os << "f ";
      for (int j = 0; j < m.c; ++j) {
        os << m[i][j] << " ";
      }
      os << std::endl;
    }
    return os;
  }

  int l, c;
  std::vector<std::vector<T>> elem;
};

class window {
public:
  explicit window(int width = 1, int height = 1)
      : xmax(width), xmin(-width), ymax(height), ymin(-height),
        coords({coord(width, height), coord(-width, height),
                coord(width, -height), coord(-width, -height)}) {}

  void update_coord() {
    auto frst = this->coords.front();
    auto last = this->coords.back();
    this->xmax = frst.x;
    this->ymax = frst.y;
    this->xmin = last.x;
    this->ymin = last.y;
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
      : name(std::move(_name)), orig(_orig), actual(_orig) {
    faces = matrix<int>(1, orig.size());
    std::iota(faces[0].begin(), faces[0].end(), 1);
  }

  explicit drawable(std::string _name, const std::list<coord> &_orig,
                    const matrix<int> &_faces)
      : name(std::move(_name)), orig(_orig), actual(_orig), faces(_faces) {}

  friend std::ostream &operator<<(std::ostream &os, const drawable &d) {
    for (auto &i : d.orig) {
      os << i;
    }
    return os << d.faces;
  }

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
  matrix<int> faces;
};

#endif // STRUCTURES_HPP
