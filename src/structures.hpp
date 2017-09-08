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
  coord(double _x = 0, double _y = 0, double _z = 1) : x(_x), y(_y), z(_z) {}

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
  window(double width = 1, double height = 1, double b = 1.0 / 20.0)
      : wid(width - b), hei(height - b), center(coord()) {}

  double wid, hei, angle;
  coord center;
};

class drawable {
public:
  explicit drawable(std::string _name, const std::list<coord> &_orig,
                    bool f = false)
      : name(std::move(_name)), orig(_orig), scn(_orig), fill(f) {
    faces = matrix<int>(1, orig.size());
    std::iota(faces[0].begin(), faces[0].end(), 1);
  }

  explicit drawable(std::string _name, const std::list<coord> &_orig,
                    const matrix<int> &_faces)
      : name(std::move(_name)), orig(_orig), scn(_orig), faces(_faces) {}

  friend std::ostream &operator<<(std::ostream &os, const drawable &d) {
    for (auto &i : d.orig) {
      os << i;
    }
    return os << d.faces;
  }

  void draw(cairo_t *cr, const std::list<coord> &points) {
    auto it = std::begin(points), end = std::end(points);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, (*it).x, (*it).y);
    while (it++ != end) {
      cairo_line_to(cr, (*it).x, (*it).y);
    }
    cairo_close_path(cr);
    if (fill) {
      cairo_fill(cr);
    }
    cairo_stroke(cr);
  }

  gint16 type() {
    if (orig.size() > 2) {
      return 3;
    }
    return orig.size();
  }

  coord center() {
    return std::accumulate(orig.begin(), orig.end(), coord()) / orig.size();
  }

  void point_clipping(const window &w) {
    coord &c = *std::begin(scn);
    c.x = (c.x >= w.wid) ? w.wid : c.x;
    c.x = (c.x <= -w.wid) ? w.wid : c.x;
    c.y = (c.y >= w.hei) ? w.hei : c.y;
    c.y = (c.y <= -w.hei) ? -w.hei : c.y;
  }

  void liang_barsky(const window &w) {
    coord &s = *std::begin(scn), &t = *(--std::end(scn));
    std::vector<double> lu1({s.x - t.x, t.x - s.x, s.y - t.y, t.y - s.y}),
        lu2({s.x + w.wid, w.wid - s.x, s.y + w.hei, w.hei - s.y}), r;
    double u1 = 0.0, u2 = 1.0, x1 = s.x, x2 = s.y;

    for (unsigned int i = 0; i < lu1.size(); ++i) {
      if (lu1[i] == 0 && lu2[i] < 0) {
        for (auto &i : scn)
          i = coord(w.wid, w.wid);
        return;
      }
      r.emplace_back(lu2[i] / lu1[i]);
      u1 = (lu1[i] < 0 && r[i] > u1) ? r[i] : u1;
      u2 = (lu1[i] > 0 && r[i] < u2) ? r[i] : u2;
    }

    if (u1 > u2) {
      for (auto &i : scn)
        i = coord(w.wid, w.wid);
      return;
    }

    if (u1 > 0) {
      s.x += u1 * lu1[1];
      s.y += u1 * lu1[3];
    }

    if (u2 < 1) {
      t.x = x1 + u2 * lu1[1];
      t.y = x2 + u2 * lu1[3];
    }
  }

  inline int region_code(const window &w, const coord &c) {
    return ((1 << ((c.y > w.hei) * 4)) | (1 << ((c.y < -w.hei) * 3)) |
            (1 << ((c.x > w.wid) * 2)) | (1 << ((c.x < -w.wid)) * 1)) >>
           1;
  }

  inline void cs_inters(coord &t, coord &s, const window &w, int r) {
    double m = (t.y - s.y) / (t.x - s.x);
    if (r & 4) {
      t.x = s.x + (-w.hei - s.y) / m;
      t.y = -w.hei;
    }
    if (r & 8) {
      t.x = s.x + (w.hei - s.y) / m;
      t.y = w.hei;
    }
    if (r & 1) {
      t.x = -w.wid;
      t.y = m * (-w.wid - s.x) + s.y;
    }
    if (r & 2) {
      t.x = w.wid;
      t.y = m * (w.wid - s.x) + s.y;
    }
  }

  void cohen_sutherland(const window &w) {
    coord &s = *std::begin(scn), &t = *(--std::end(scn));
    int RC1 = region_code(w, s), RC2 = region_code(w, t);

    if ((RC1 + RC2) == 0) {
      return;
    }

    if (RC1 & RC2) {
      for (auto &i : scn)
        i = coord(w.wid, w.wid);
      return;
    }

    cs_inters(s, t, w, RC1);
    cs_inters(t, s, w, RC2);
  }

  void clip(int l) {
    window ww;
    switch (type()) {
    case 1:
      point_clipping(ww);
      break;
    case 2:
      (l == -1) ? liang_barsky(ww) : cohen_sutherland(ww);
      break;
    }
  }

  const std::string name;
  std::list<coord> orig, scn;
  matrix<int> faces;
  bool fill;
};

#endif // STRUCTURES_HPP
