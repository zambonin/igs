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
  explicit window(double width = 1, double height = 1,
                  double b = 1.0 / 20.0) noexcept
      : wid(width - b), hei(height - b), center(coord()) {}

  double wid, hei, angle{0};
  coord center;
};

class drawable {
public:
  explicit drawable(std::string _name, const std::list<coord> &_orig,
                    bool f = false, bool c = false)
      : name(std::move(_name)), orig(_orig), scn(_orig), fill(f), curve(c) {
    faces = matrix<int>(1, orig.size());
    std::iota(faces[0].begin(), faces[0].end(), 1);

    if (curve) {
      int n = ((this->orig.size() - 4) / 3) + 1;

      std::vector<coord> _orig;
      _orig.assign(std::begin(this->orig), std::end(this->orig));
      this->orig.clear();

      for (int i = 0; i < n; i++) {
        for (double j = 0; j < 1; j += 0.001) {
          double j2 = j * j;
          double j3 = j * j * j;

          double x, y;
          x = (-j3 + 3 * j2 - 3 * j + 1) * _orig[i * 3 + 0].x +
              (3 * j3 - 6 * j2 + 3 * j) * _orig[i * 3 + 1].x +
              (-3 * j3 + 3 * j2) * _orig[i * 3 + 2].x + (j3)*_orig[i * 3 + 3].x;
          y = (-j3 + 3 * j2 - 3 * j + 1) * _orig[i * 3 + 0].y +
              (3 * j3 - 6 * j2 + 3 * j) * _orig[i * 3 + 1].y +
              (-3 * j3 + 3 * j2) * _orig[i * 3 + 2].y + (j3)*_orig[i * 3 + 3].y;

          this->orig.emplace_back(coord(x, y));
        }
      }
      this->scn = orig;
    }
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
    auto it = std::begin(points), end = --std::end(points);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, (*it).x, (*it).y);
    while (it++ != end) {
      cairo_line_to(cr, (*it).x, (*it).y);
    }
    if (!curve) {
      cairo_close_path(cr);
    }
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

  std::list<coord> point_clipping(const window &w) {
    coord &c = *std::begin(scn);
    c.x = (c.x >= w.wid) ? w.wid : c.x;
    c.x = (c.x <= -w.wid) ? w.wid : c.x;
    c.y = (c.y >= w.hei) ? w.hei : c.y;
    c.y = (c.y <= -w.hei) ? -w.hei : c.y;
    return scn;
  }

  std::list<coord> liang_barsky(const window &w) {
    coord &s = *std::begin(scn), &t = *(--std::end(scn));
    std::vector<double> lu1({s.x - t.x, t.x - s.x, s.y - t.y, t.y - s.y}),
        lu2({s.x + w.wid, w.wid - s.x, s.y + w.hei, w.hei - s.y}), r;
    double u1 = 0.0, u2 = 1.0, x1 = s.x, x2 = s.y;

    for (unsigned int i = 0; i < lu1.size(); ++i) {
      if (lu1[i] == 0 && lu2[i] < 0) {
        for (auto &i : scn)
          i = coord(w.wid, w.wid);
        return scn;
      }
      r.emplace_back(lu2[i] / lu1[i]);
      u1 = (lu1[i] < 0 && r[i] > u1) ? r[i] : u1;
      u2 = (lu1[i] > 0 && r[i] < u2) ? r[i] : u2;
    }

    if (u1 > u2) {
      for (auto &i : scn)
        i = coord(w.wid, w.wid);
      return scn;
    }

    if (u1 > 0) {
      s.x += u1 * lu1[1];
      s.y += u1 * lu1[3];
    }

    if (u2 < 1) {
      t.x = x1 + u2 * lu1[1];
      t.y = x2 + u2 * lu1[3];
    }

    return scn;
  }

  inline int region_code(const window &w, const coord &c) {
    return ((1 << ((c.y > w.hei) * 4)) | (1 << ((c.y < -w.hei) * 3)) |
            (1 << ((c.x > w.wid) * 2)) | (1 << ((c.x < -w.wid)) * 1)) >>
           1;
  }

  inline void cs_inters(coord &t, const coord &s, const window &w, int r) {
    double m = (t.y - s.y) / (t.x - s.x);
    if ((r & 4) != 0) {
      t.x = s.x + (-w.hei - s.y) / m;
      t.y = -w.hei;
    }
    if ((r & 8) != 0) {
      t.x = s.x + (w.hei - s.y) / m;
      t.y = w.hei;
    }
    if ((r & 1) != 0) {
      t.x = -w.wid;
      t.y = m * (-w.wid - s.x) + s.y;
    }
    if ((r & 2) != 0) {
      t.x = w.wid;
      t.y = m * (w.wid - s.x) + s.y;
    }
  }

  std::list<coord> cohen_sutherland(const window &w) {
    coord &s = *std::begin(scn), &t = *(--std::end(scn));
    int RC1 = region_code(w, s), RC2 = region_code(w, t);

    if ((RC1 + RC2) == 0) {
      return scn;
    }

    if ((RC1 & RC2) != 0) {
      for (auto &i : scn)
        i = coord(w.wid, w.wid);
      return scn;
    }

    cs_inters(s, t, w, RC1);
    cs_inters(t, s, w, RC2);

    return scn;
  }

  coord line_inters(const coord &c1, const coord &c2, const coord &c3,
                    const coord &c4) {
    double delta_x1 = c1.x - c2.x, delta_y1 = c1.y - c2.y,
           delta_x2 = c3.x - c4.x, delta_y2 = c3.y - c4.y,
           m1 = delta_y1 / delta_x1, m2 = delta_y2 / delta_x2;

    if ((delta_x1 == 0.0 && delta_x2 == 0.0) || (m1 == m2)) {
      return coord();
    }

    if (delta_x1 == 0.0) {
      return coord(c1.x, m2 * (c1.x - c3.x) + c3.y);
    }

    if (delta_x2 == 0.0) {
      return coord(c3.x, m1 * (c3.x - c1.x) + c1.y);
    }

    double x = (-m1 * c1.x + m2 * c3.x + c1.y - c3.y) / (m2 - m1),
           y = m2 * (x - c3.x) + c3.y;

    return coord(x, y);
  }

  std::list<coord> weiler_atherton(const window &w) {
    std::vector<coord> w_coord({coord(w.wid, w.hei), coord(w.wid, -w.hei),
                                coord(-w.wid, -w.hei), coord(-w.wid, w.hei)}),
        old_coords{std::begin(scn), std::end(scn)};
    std::list<coord> coords;

    for (unsigned int i = 0; i < w_coord.size(); ++i) {
      coord c1 = w_coord[i], c2 = w_coord[(i + 1) % w_coord.size()];
      coords.clear();
      for (unsigned int j = 0; j < old_coords.size(); ++j) {
        coord c3 = old_coords[j], c4 = old_coords[(j + 1) % old_coords.size()];
        bool s_out =
            ((c2.x - c1.x) * (c3.y - c1.y) > (c2.y - c1.y) * (c3.x - c1.x));
        bool t_out =
            ((c2.x - c1.x) * (c4.y - c1.y) > (c2.y - c1.y) * (c4.x - c1.x));
        if (s_out != t_out) {
          coords.emplace_back(line_inters(c3, c4, c1, c2));
          if (s_out)
            coords.push_back(c4);
        } else if (!t_out && !s_out) {
          coords.push_back(c4);
        }
      }
      old_coords.assign(coords.begin(), coords.end());
    }
    if (coords.empty()) {
      coords.push_back(coord(w.wid, w.wid));
    }
    return coords;
  }

  std::list<coord> clip(int l) {
    window ww;
    switch (type()) {
    case 1:
      return point_clipping(ww);
    case 2:
      return (l == -1) ? liang_barsky(ww) : cohen_sutherland(ww);
    case 3:
      return weiler_atherton(ww);
    }
    return {};
  }

  const std::string name;
  std::list<coord> orig, scn;
  matrix<int> faces;
  bool fill{false};
  bool curve;
};

#endif // STRUCTURES_HPP
