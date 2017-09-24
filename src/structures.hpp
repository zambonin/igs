#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <gtk/gtk.h>
#include <iomanip>
#include <iostream>
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
      : wid(width - b), hei(height - b) {}

  double wid, hei, angle{0};
  coord center{};
};

#endif // STRUCTURES_HPP
