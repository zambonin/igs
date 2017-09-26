#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include "structures.hpp"

#include <list>
#include <numeric>
#include <string>

class drawable {
public:
  drawable(const std::string &_name, const std::list<coord> &_orig)
      : name(std::move(_name)), orig(_orig), scn(_orig) {
    faces = matrix<int>(1, orig.size());
    std::iota(faces[0].begin(), faces[0].end(), 1);
  }

  drawable(const std::string &_name, const std::list<coord> &_orig,
           const matrix<int> &_faces)
      : name(std::move(_name)), orig(_orig), scn(_orig), faces(_faces) {}

  friend std::ostream &operator<<(std::ostream &os, const drawable &d) {
    for (auto &i : d.orig) {
      os << i;
    }
    return os << d.faces;
  }

  virtual void draw(cairo_t *cr, const std::list<coord> &points) {}

  virtual std::list<coord> clip(const window &w = window()) { return scn; }

  coord center() {
    return std::accumulate(orig.begin(), orig.end(), coord()) / orig.size();
  }

  const std::string name{};
  std::list<coord> orig{}, scn{};
  matrix<int> faces;
};

class point : public drawable {
public:
  using drawable::drawable;

  void draw(cairo_t *cr, const std::list<coord> &points) override {
    auto p = *std::begin(points);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, p.x, p.y);
    cairo_line_to(cr, p.x, p.y);
    cairo_stroke(cr);
  }

  std::list<coord> clip(const window &w) override {
    coord &c = *std::begin(scn);
    c.x = (c.x >= w.wid) ? w.wid : c.x;
    c.x = (c.x <= -w.wid) ? w.wid : c.x;
    c.y = (c.y >= w.hei) ? w.hei : c.y;
    c.y = (c.y <= -w.hei) ? -w.hei : c.y;
    return scn;
  }
};

class line : public drawable {
public:
  line(const std::string &n, const std::list<coord> &cs, bool clip = false)
      : drawable(n, cs), clip_with_lb(clip) {}

  void draw(cairo_t *cr, const std::list<coord> &points) override {
    auto it = std::begin(points), end = --std::end(points);
    cairo_move_to(cr, (*it).x, (*it).y);
    while (it++ != end) {
      cairo_line_to(cr, (*it).x, (*it).y);
    }
    cairo_stroke(cr);
  }

  std::list<coord> liang_barsky(const window &w) {
    coord &s = *std::begin(scn), &t = *(--std::end(scn));
    std::vector<double> lu1(
        {-1 * (t.x - s.x), t.x - s.x, -1 * (t.y - s.y), t.y - s.y}),
        lu2({s.x + w.wid, w.wid - s.x, s.y + w.hei, w.hei - s.y}), r;
    double u1 = 0.0, u2 = 1.0, x1 = s.x, x2 = s.y;

    for (unsigned int i = 0; i < lu1.size(); ++i) {
      if (lu1[i] == 0 && lu2[i] < 0) {
        std::list<coord> emp;
        return emp;
      }
      r.emplace_back(lu2[i] / lu1[i]);
      u1 = (lu1[i] < 0 && r[i] > u1) ? r[i] : u1;
      u2 = (lu1[i] > 0 && r[i] < u2) ? r[i] : u2;
    }

    if (u1 > u2) {
      std::list<coord> emp;
      return emp;
    }

    std::list<coord> coords;
    coords.assign(std::begin(scn), std::end(scn));
    auto source = std::begin(coords), target = --std::end(coords);

    if (u2 < 1) {
      (*target).x = x1 + u2 * lu1[1];
      (*target).y = x2 + u2 * lu1[3];
    }

    if (u1 > 0) {
      (*source).x = x1 + u1 * lu1[1];
      (*source).y = x2 + u1 * lu1[3];
    }

    return coords;
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
      if (t.x > w.wid || t.y < -w.wid) {
        t.x = w.wid;
        t.y = m * (w.wid - s.x) + s.y;
      }
      if (t.x < -w.wid || t.y < -w.wid) {
        t.x = -w.wid;
        t.y = m * (-w.wid - s.x) + s.y;
      }
    } else if ((r & 8) != 0) {
      t.x = s.x + (w.hei - s.y) / m;
      t.y = w.hei;
      if (t.x > w.wid || t.y > w.wid) {
        t.x = w.wid;
        t.y = m * (w.wid - s.x) + s.y;
      }
      if (t.x < -w.wid || t.y > w.wid) {
        t.x = -w.wid;
        t.y = m * (-w.wid - s.x) + s.y;
      }
    } else if ((r & 1) != 0) {
      t.x = -w.wid;
      t.y = m * (-w.wid - s.x) + s.y;
    } else if ((r & 2) != 0) {
      t.x = w.wid;
      t.y = m * (w.wid - s.x) + s.y;
    }
  }

  std::list<coord> cohen_sutherland(const window &w) {
    coord &s = *std::begin(scn), &t = *(--std::end(scn));
    int RC1 = region_code(w, s), RC2 = region_code(w, t), RCr = RC1 & RC2;

    if ((RC1 == RC2) && RC1 == 0) {
      return scn;
    }

    if (RCr != 0) {
      std::list<coord> emp;
      return emp;
    }

    std::list<coord> coords;
    std::list<coord> aux;
    coords.assign(std::begin(scn), std::end(scn));

    if (RC1 != RC2) {
      if (RCr == 0) {
        if (RC1 != 0) {
          cs_inters(s, t, w, RC1);
        }
        if (RC2 != 0) {
          cs_inters(t, s, w, RC2);
        }

        if ((s.x > w.wid || s.x < -w.wid || s.y > w.wid || s.y < -w.wid) ||
            (t.x > w.wid || t.x < -w.wid || t.y > w.wid || t.y < -w.wid)) {
          std::list<coord> emp;
          return emp;
        }

        aux = scn;
        scn = coords;
        return aux;
      }
    }
    std::list<coord> emp;
    return emp;
  }

  std::list<coord> clip(const window &w) override {
    return (clip_with_lb) ? liang_barsky(w) : cohen_sutherland(w);
  }

  bool clip_with_lb{false};
};

class polygon : public drawable {
public:
  polygon(const std::string &n, const std::list<coord> &cs, bool _fill = false)
      : drawable(n, cs), fill(_fill) {}

  polygon(const std::string &n, const std::list<coord> &cs,
          const matrix<int> &f, bool _fill = false)
      : drawable(n, cs, f), fill(_fill) {}

  void draw(cairo_t *cr, const std::list<coord> &points) override {
    auto it = std::begin(points), end = --std::end(points);
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

  std::list<coord> clip(const window &w) override {
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

  bool fill{false};
};

class curve : public line {
public:
  curve(const std::string &name, const std::list<coord> &cs, bool cm,
        double _step = 1000)
      : line(name, cs, cm), step(_step) {}

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

  std::list<coord> clip(const window &w) override {
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
          if (!((j + 1) % old_coords.size() == 0))
            coords.emplace_back(line_inters(c3, c4, c1, c2));
          if (s_out) {
            if (!((j + 1) % old_coords.size() == 0)) {
              coords.push_back(c4);
            }
          }
        } else if (!t_out && !s_out) {
          if (!((j + 1) % old_coords.size() == 0)) {
            coords.push_back(c4);
          }
        }
      }
      old_coords.assign(coords.begin(), coords.end());
    }
    if (coords.empty()) {
      coords.push_back(coord(w.wid, w.wid));
    }
    return coords;
  }

  double step{0};
};

class bezier : public curve {
public:
  bezier(std::string name, const std::list<coord> &cs, bool cm, double step)
      : curve(name, cs, cm, step) {
    int n = ((orig.size() - 4) / 3);

    std::vector<coord> _orig;
    _orig.assign(std::begin(orig), std::end(orig));
    orig.clear();

    for (int i = 0; i < n + 1; ++i) {
      for (int k = 0; k < step; ++k) {
        double j = k / step;
        double j2 = j * j;
        double j3 = j * j * j;

        double x = _orig[i * 3 + 0].x * (-j3 + (3 * j2) + (-3 * j) + 1) +
                   _orig[i * 3 + 1].x * ((3 * j3) + (-6 * j2) + (3 * j)) +
                   _orig[i * 3 + 2].x * ((-3 * j3) + (3 * j2)) +
                   _orig[i * 3 + 3].x * (j3);

        double y = _orig[i * 3 + 0].y * (-j3 + (3 * j2) + (-3 * j) + 1) +
                   _orig[i * 3 + 1].y * ((3 * j3) + (-6 * j2) + (3 * j)) +
                   _orig[i * 3 + 2].y * ((-3 * j3) + (3 * j2)) +
                   _orig[i * 3 + 3].y * (j3);
        orig.emplace_back(coord(x, y));
      }
    }
    scn = orig;
  }
};

class bspline : public curve {
public:
  bspline(std::string name, const std::list<coord> &cs, bool cm, double step)
      : curve(name, cs, cm, step) {
    std::vector<coord> coords;
    coords.assign(std::begin(orig), std::end(orig));
    orig.clear();

    double t = 1.0 / step;
    double t2 = t * t;
    double t3 = t2 * t;

    for (unsigned int i = 0; i < coords.size() - 3; ++i) {
      coord coef1 = coords[i];
      coord coef2 = coords[i + 1];
      coord coef3 = coords[i + 2];
      coord coef4 = coords[i + 3];

      double a = -(1.0 / 6.0) * coef1.x + 0.5 * coef2.x - 0.5 * coef3.x +
                 (1.0 / 6.0) * coef4.x;
      double b = 0.5 * coef1.x - coef2.x + 0.5 * coef3.x;
      double c = -0.5 * coef1.x + 0.5 * coef3.x;
      double d =
          (1.0 / 6.0) * coef1.x + (2.0 / 3.0) * coef2.x + (1.0 / 6.0) * coef3.x;
      double x = d;

      double deltaX = a * t3 + b * t2 + c * t;
      double deltaX2 = (6 * a * t3) + (2 * b * t2);
      double deltaX3 = (6 * a * t3);

      a = -(1.0 / 6.0) * coef1.y + 0.5 * coef2.y - 0.5 * coef3.y +
          (1.0 / 6.0) * coef4.y;
      b = 0.5 * coef1.y - coef2.y + 0.5 * coef3.y;
      c = -0.5 * coef1.y + 0.5 * coef3.y;
      d = (1.0 / 6.0) * coef1.y + (2.0 / 3.0) * coef2.y + (1.0 / 6.0) * coef3.y;
      double y = d;

      double deltaY = a * t3 + b * t2 + c * t;
      double deltaY2 = (6 * a * t3) + (2 * b * t2);
      double deltaY3 = (6 * a * t3);

      orig.emplace_back(coord(x, y));
      forward_diff(x, deltaX, deltaX2, deltaX3, y, deltaY, deltaY2, deltaY3);
    }
    scn = orig;
  }

  void forward_diff(double _x, double deltaX, double deltaX2, double deltaX3,
                    double _y, double deltaY, double deltaY2, double deltaY3) {
    for (int t = 0; t < step; ++t) {
      double x = _x, y = _y;

      x += deltaX;
      deltaX += deltaX2;
      deltaX2 += deltaX3;

      y += deltaY;
      deltaY += deltaY2;
      deltaY2 += deltaY3;

      orig.emplace_back(coord(x, y));
      _x = x;
      _y = y;
    }
  }
};

#endif // DRAWABLE_HPP
