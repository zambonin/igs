#include "structures.hpp"
#include <cmath>
#include <sstream>

matrix m_transfer(const coord& c) {
  return matrix({{1, 0, 0}, {0, 1, 0}, {c.x, c.y, 1}});
}

matrix m_rotate(double a) {
  return matrix({{cos(a), -sin(a), 0}, {sin(a), cos(a), 0}, {0, 0, 1}});
}

matrix m_scale(const coord& c) {
  return matrix({{c.x, 0, 0}, {0, c.y, 0}, {0, 0, 1}});
}

std::list<coord> split(const char* input) {

  std::list<coord> c;
  std::list<double> tmp;

  std::istringstream iss1(input);
  std::string s1, s2;

  while(getline(iss1, s1, ' ')) {
    std::istringstream iss2(s1);
    while(getline(iss2, s2, ';')) {
      try {
        tmp.push_back(std::stod(s2));
      } catch (const std::invalid_argument& ia) {}
    }
    c.emplace_back(coord(tmp.front(), tmp.back()));

    if (tmp.size() != 2) {
      c.clear();
      return c;
    }
    tmp.clear();
  }

  return c;

}
