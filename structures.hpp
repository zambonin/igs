#pragma once

#include <list>
#include <string>

class coord {
 public:
  coord(double _x, double _y) : x(_x), y(_y) {}

  double x, y;
};

class drawable {
 public:
  explicit drawable(std::string n) : name(n) {}

  // TODO [arch] assuming a global surface `cr` already created somewhere else
  virtual void draw() = 0;
  virtual void list_coord() = 0;

  short type() {
    return clist.size() ? (clist.size() < 3) : 2;
  }

  const std::string name;
  std::list<coord> clist;
};

class dot : drawable {
 public:
  dot(std::string n, coord _c) : drawable(n), c(_c) {}

  void list_coord() override {
    clist.push_back(c);
  }

  void draw() {
    cairo_translate(cr, c.x, c.y);
    cairo_arc(cr, 0, 0, 1, 0, 2 * M_PI);
    cairo_fill(cr);
  }

  coord c;
};

class line : drawable {
 public:
  line(std::string n, dot _s, dot _e) : drawable(n), start(_s), end(_e) {}

  void list_coord() override {
    start.list_coord();
    end.list_coord();
  }

  void draw() {
    cairo_move_to(cr, start.c.x, start.c.y);
    cairo_line_to(cr, end.c.x, end.c.y);
    cairo_stroke(cr);
  }

  dot start, end;
};

class polygon : drawable {
 public:
  polygon(std::string n, std::list<line> _l) : drawable(n), lines(_l) {}

  void list_coord() override {
    for (auto& i : lines) {
      i.list_coord();
    }
  }

  void draw() {
    for (auto& i : lines) {
      i.draw();
    }
  }

  std::list<line> lines;
};
