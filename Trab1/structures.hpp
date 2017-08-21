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
  coord(double _x, double _y, double _z = 1)
    : x(_x), y(_y), z(_z) {}

  bool operator!=(const coord& rhs) {
    return this->x != rhs.x || this->y != rhs.y;
  }

  coord operator+(const coord& c) {
    return coord(x + c.x, y + c.y, z + c.z);
  }

  coord operator+=(const coord& c) {
    return (*this) + c;
  }

  coord operator-() {
    return coord(-x, -y, -z);
  }

  coord operator/(double s) {
    return coord(x / s, y / s, z / s);
  }

  friend std::ostream& operator<<(std::ostream& os, const coord& c) {
    return os << "x: " << c.x << " y: " << c.y << std::endl;
  }

  double x, y, z;
};

class matrix {
 public:
  matrix(int _l = 3, int _c = 3)
    : l(_l), c(_c), elem(_l, std::vector<double>(_c, 0)) {}

  matrix(std::vector<std::vector<double>> e)
    : l(e.size()), c(e[0].size()), elem(e) {}

  std::vector<double>& operator[](int i) {
    return elem[i];
  }

  const std::vector<double>& operator[](int i) const {
    return elem[i];
  }

  matrix operator+(matrix& m) {
    if (l == m.l && c == m.c) {
      matrix r(3, 3);
      for (int i = 0; i < l; ++i) {
        for (int j = 0; i < c; ++i) {
          r[i][j] += elem[i][j] + m[i][j];
        }
      }
      return r;
    } else {
      return *this;
    }
  }

  matrix operator*(matrix m) {
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

  friend std::ostream& operator<<(std::ostream& os, const matrix& m) {
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
  explicit window(std::list<coord> _coords, double _xmax = 200, double _xmin = -200,
      double _ymax = 200, double _ymin = -200) noexcept
    : xmax(_xmax), xmin(_xmin), ymax(_ymax), ymin(_ymin), coords(_coords)
  {
    this->angle = 0;
    updateMax();
    set_limits(0, 0, 0, 0);
  }

  void transform(matrix m) {
     for (auto& i : coords) {
       matrix res = matrix({{i.x, i.y, i.z}}) * m;
       i = coord(res[0][0], res[0][1], res[0][2]);
     }
     updateMax();
  }

  void updateMax() {
    auto frst = this->coords.front();
    auto last = this->coords.back();
    this->xmax = frst.x;
    this->ymax = frst.y;
    this->xmin = last.x;
    this->ymin = last.y;
    this->wCenterX = (xmax+xmin)/2;
    this->wCenterY = (ymax+ymin)/2;

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
    this->xmax = 200;
    this->xmin = -200;
    this->ymax = 200;
    this->ymin = -200;
  }

  double xmax, xmin, ymax, ymin;
  std::list<coord> coords;
  double wCenterX, wCenterY;
  double angle;
};


class drawable {
 public:
  explicit drawable(std::string _name, const std::list<coord>& _orig)
    : name(std::move(_name)), orig(_orig), vp(_orig), normCoord(_orig) {}

  void draw(cairo_t* cr) {
    auto it = std::begin(vp), end = --std::end(vp);
    while (it != end) {
      cairo_move_to(cr, (*it).x, (*it).y);
      ++it;
      cairo_line_to(cr, (*it).x, (*it).y);
    }

    cairo_set_line_width(cr, 2);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, vp.front().x, vp.front().y);
    cairo_line_to(cr, vp.back().x, vp.back().y);
    cairo_stroke(cr);
  }

  gint16 type() {
    if (orig.size() > 2) {
      return 2;
    }
    return orig.size();
  }

  void viewport(GtkWidget* area, const window* w) {
    int wid, hei;
    gtk_widget_get_size_request(area, &wid, &hei);
    auto it1 = normCoord.begin(), it2 = vp.begin();
    for (; it1 != normCoord.end(); ++it1, ++it2) {
      std::cout << (*it1).x << (*it1).y << std::endl;
      (*it2).x = (((*it1).x +1) / (2)) * (wid - 0);
      (*it2).y = (1 - (((*it1).y +1) / (2))) * (hei - 0);
    }
  }

  void transform(matrix m) {
    for (auto& i : orig) {
      matrix res = matrix({{i.x, i.y, i.z}}) * m;
      i = coord(res[0][0], res[0][1], res[0][2]);
    }

  }

  void transform_normalize(matrix m) {
    auto it = normCoord.begin();
    for (auto& i : orig) {
      matrix res = matrix({{i.x, i.y, i.z}}) * m;
      *it = coord(res[0][0], res[0][1], res[0][2]);
      std::cout << " NormCoord: " << (*it).x << (*it).y << std::endl;
      it++;
    }
  }

  coord center() {
    coord sum = std::accumulate(normCoord.begin(), normCoord.end(), coord(0, 0));
        return sum / normCoord.size();
  }

  const std::string name;
  std::list<coord> orig, vp, normCoord;
};

#endif // STRUCTURES_HPP
