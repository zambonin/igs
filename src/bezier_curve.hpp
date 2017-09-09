#include "structures.hpp"

class bezier_curve : public drawable {
public:
  bezier_curve(std::string _name, const std::list<coord> &_orig, bool _f)
      : drawable(_name, _orig, _f) {
    produce_curve();
    this->orig = orig;
    this->scn = orig;
  }

  void produce_curve() {

    int n = ((this->orig.size() - 4) / 3) + 1;

    std::vector<coord> _orig;
    _orig.assign(std::begin(this->orig), std::end(this->orig));
    this->orig.clear();
    for (int i = 0; i < n; i++) {

      for (double j = 0; j < 1; j += step) {
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
    // this->orig.assign(std::begin(_orig), std::end(_orig));
  }

  double step = 0.0001;
};
