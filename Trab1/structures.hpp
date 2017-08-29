#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <gtk/gtk.h>
#include <iomanip>
#include <iostream>
#include <list>
#include <numeric>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
class coord {
 public:
  coord(double _x, double _y, double _z = 1)
    : x(_x), y(_y), z(_z) {}

  coord(std::vector<double> c)
    : x(c[0]), y(c[1]), z(c[2]) {}

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
    os << std::setprecision(5) << std::fixed;
    return os << "v " << c.x << " " << c.y << " " << c.z << std::endl;
  }

  double x, y, z;
};

template <typename T>
class matrix {
 public:
  matrix(int _l = 3, int _c = 3)
    : l(_l), c(_c), elem(_l, std::vector<T>(_c, 0)) {}

  matrix(std::vector<std::vector<T>> e)
    : l(e.size()), c(e[0].size()), elem(e) {}

  std::vector<T>& operator[](int i) {
    return elem[i];
  }

  const std::vector<T>& operator[](int i) const {
    return elem[i];
  }

  matrix<T> operator+(matrix& m) {
    if (l == m.l && c == m.c) {
      matrix<T> r(3, 3);
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

  matrix<T> operator*(matrix m) {
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

  friend std::ostream& operator<<(std::ostream& os, const matrix& m) {
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
  explicit window(std::list<coord> _coords, double _xmax = 200, double _xmin = -200,
      double _ymax = 200, double _ymin = -200) noexcept
    : xmax(_xmax), xmin(_xmin), ymax(_ymax), ymin(_ymin), coords(_coords)
  {
    this->angle = 0;
    updateMax();
    set_limits(0, 0, 0, 0);
  }

  void transform(matrix<double> m) {
     for (auto& i : coords) {
       matrix<double> res = matrix<double>({{i.x, i.y, i.z}}) * m;
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

  void rotate(double a) {
    this->angle += a;
  }

  void zoom(double r) {
    this->xmax *= r;
    this->xmin *= r;
    this->ymax *= r;
    this->ymin *= r;
  }

  void reset() {
    this->angle = 0;
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
    : name(std::move(_name)), orig(_orig), vp(_orig), normCoord(_orig) {
      faces = matrix<int>(1, orig.size());
      std::iota(faces[0].begin(), faces[0].end(), 1);
    }

  explicit drawable(std::string _name, const std::list<coord>& _orig,
      const matrix<int>& _faces)
    : name(std::move(_name)), orig(_orig), vp(_orig), normCoord(_orig),
      faces(_faces) {}

  friend std::ostream& operator<<(std::ostream& os, const drawable& d) {
    for (auto &i : d.orig) os << i;
    return os << d.faces;
  }

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


  void line_clipping(int wid, int hei) {
    int xmin= -wid+10, xmax = wid-10, ymin = -hei+10 ,ymax = hei-10;
    double r;
    double p1, p2, p3, p4, q1, q2, q3, q4;
    auto it = std::begin(vp);
    double x1 = (*it).x;
    double y1 = (*it).y;
    it++;
    double x2 = (*it).x;
    double y2 = (*it).y;
    it--;
    std::vector<double> lu1;
    std::vector<double> lu2;
    std::cout << "Point One: " << x1 << " " << y1 << std::endl;
    std::cout << "Point Two: " << x2 << " " << y2 << std::endl;
    p1 = -1*(x2 - x1);
    lu1.push_back(p1);
    p2 = x2 - x1;
    lu1.push_back(p2);
    p3 = -1*(y2 - y1);
    lu1.push_back(p3);
    p4 = y2 - y1;
    lu1.push_back(p4);

    q1 = x1 - xmin;
    lu2.push_back(q1);
    q2 = xmax - x1;
    lu2.push_back(q2);
    q3 = y1 - ymin;
    lu2.push_back(q3);
    q4 = ymax - y1;
    lu2.push_back(q4);
    std::cout << "P's and Q's: " << p1 << " " << p2 << " " << p3 << " " << p4 << " " <<q1 << " " << q2 << " " << q3 << " " << q4 << " " << std::endl;
    if ((p1 == 0 || p2 == 0 || p3 == 0 || p4 == 0) && (q1 < 0 || q2 < 0 || q3 < 0 || q4 < 0)) {
        std::cout << "Out of bounds!" << std::endl;
        return;
    }
    double u1 = 0.0;
    double u2 = 1.0;
    for(int i = 0; i < 4; i++) {
        r = lu2[i]/lu1[i];

        std::cout << "op: " << r << std::endl;

        if (lu1[i] < 0){
            if (r > u1) {
                u1 = r;
            }
        } else if (lu1[i] > 0){
            if (r < u2) {
                u2 = r;
            }
        }
    }
    std::cout<< "U's: " << u1 << " " <<  u2 << std::endl;
    if (u1 > u2) {
        std::cout << "Line Out of Bounds!" << std::endl;
        return;
    }
    if (u1 > 0) {
        std::cout << "Att1" << std::endl;
        (*it).x = x1 + u1*(x2-x1);
        (*it).y = y1 + u1*(y2-y1);
    }
    it++;
    if (u2 < 1) {
        std::cout << "Att2" << x2 << " " << u2  << " " << x1  << " " << x2  << " " << std::endl;
        (*it).x = x2 + u2*(x2-x1);
        (*it).y = y2 + u2*(y2-y1);
    }
    std::cout << "Point One: " << (*std::begin(vp)).x << " " << (*std::begin(vp)).y << std::endl;
    std::cout << "Point Two: " << (*it).x << " " << (*it).y << std::endl;

  }

  gint16 type() {
    if (orig.size() > 2) {
      return 2;
    }
    return orig.size();
  }

  void viewport(GtkWidget* area) {
    int wid, hei;
    int borderSize = 10;
    gtk_widget_get_size_request(area, &wid, &hei);
    auto it1 = normCoord.begin(), it2 = vp.begin();
    for (; it1 != normCoord.end(); ++it1, ++it2) {
      (*it2).x = ((((*it1).x +1) / (2)) * (wid - 0)) + borderSize;
      (*it2).y = ((1 - (((*it1).y +1) / (2))) * (hei - 0)) + borderSize;
    }
    line_clipping(wid, hei);

  }

  void transform(matrix<double> m) {
    for (auto& i : orig) {
      matrix<double> res = matrix<double>({{i.x, i.y, i.z}}) * m;
      i = coord(res[0][0], res[0][1], res[0][2]);
    }

  }

  void transform_normalize(matrix<double> m) {
    auto it = normCoord.begin();
    for (auto& i : orig) {
      matrix<double> res = matrix<double>({{i.x, i.y, i.z}}) * m;
      *it = coord(res[0][0], res[0][1], res[0][2]);
      it++;
    }
  }

  coord center() {
    coord sum = std::accumulate(orig.begin(), orig.end(), coord(0, 0));
    return sum / orig.size();
  }

  const std::string name;
  std::list<coord> orig, vp, normCoord;
  matrix<int> faces;
};

#endif // STRUCTURES_HPP
