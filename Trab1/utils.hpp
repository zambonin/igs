#include "structures.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

matrix<double> m_transfer(const coord &c) {
  return matrix<double>({{1, 0, 0}, {0, 1, 0}, {c.x, c.y, 1}});
}

matrix<double> m_rotate(double a) {
  return matrix<double>({{cos(a), -sin(a), 0}, {sin(a), cos(a), 0}, {0, 0, 1}});
}

matrix<double> m_scale(const coord &c) {
  return matrix<double>({{c.x, 0, 0}, {0, c.y, 0}, {0, 0, 1}});
}

std::list<coord> split(const char *input) {

  std::list<coord> c;
  std::list<double> tmp;

  std::istringstream iss1(input);
  std::string s1, s2;

  while (getline(iss1, s1, ' ')) {
    std::istringstream iss2(s1);
    while (getline(iss2, s2, ';')) {
      try {
        tmp.push_back(std::stod(s2));
      } catch (const std::invalid_argument &ia) {
      }
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

template <typename Out>
void split(const std::string &s, char delim, Out result) {
  std::stringstream ss;
  ss.str(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

drawable read_obj(const std::string &path) {
  std::ifstream file(path);
  std::string l;
  std::vector<std::string> line;

  const std::string name = split(path, '/').back();
  std::vector<std::vector<int>> faces;
  std::list<coord> points;

  try {
    while (std::getline(file, l)) {
      line = split(l, ' ');
      if (line.size() == 0) {
        continue;
      } else if (line[0] == "f") {
        std::vector<int> face;
        std::transform(line.begin() + 1, line.end(), std::back_inserter(face),
                       [](const std::string &str) { return std::stoi(str); });
        faces.emplace_back(face);
      } else if (line[0] == "v") {
        std::vector<double> point;
        std::transform(line.begin() + 1, line.end(), std::back_inserter(point),
                       [](const std::string &str) { return std::stod(str); });
        points.emplace_back(point);
      }
    }
  } catch (const std::invalid_argument &ia) {
    return drawable(name, {}, {});
  }
  return drawable(name, points, matrix<int>(faces));
}

void write_obj(const std::string &path, const drawable &d) {
  std::ofstream(path) << d;
}
