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
        coord(double _x=0, double _y=0, double _z = 1)
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

        bool clipping(int wid, int hei) {
            if (this->type() == 1) {
                return point_clipping(wid,hei);
            }
            if (this->type() == 2) {
                return line_clipping(wid, hei);
            }
            return poligon_clipping(wid, hei);
        }

        // coord* intersection(coord p1, coord p2, coord p3, coord p4) {
        //     // Store the values for fast access and easy
        //     // equations-to-code conversion
        //     double x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
        //     double y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;
        //
        //     double d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        //     // If d is zero, there is no intersection
        //     if (!(d != 0)) return NULL;
        //
        //     // Get the x and y
        //     double pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
        //     double x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
        //     double y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
        //
        //     // Check if the x and y coordinates are within both lines
        //     if ( x < std::min(x1, x2) || x > std::max(x1, x2) ||
        //             x < std::min(x3, x4) || x > std::max(x3, x4) ) return NULL;
        //     if ( y < std::min(y1, y2) || y > std::max(y1, y2) ||
        //             y < std::min(y3, y4) || y > std::max(y3, y4) ) return NULL;
        //
        //     // Return the point of intersection
        //     //
        //     coord* ret = new coord(x,y);
        //     return ret;
        // }
        bool intersection(std::vector<coord> segment1, std::vector<coord> segment2, coord& result){
            double p0_x=segment1[0].x,  p0_y=segment1[0].y,
                   p1_x=segment1[1].x, p1_y=segment1[1].y,
                   p2_x=segment2[0].x,  p2_y=segment2[0].y,
                   p3_x=segment2[1].x, p3_y=segment2[1].y;

            double delta_x1 = p0_x - p1_x;
            double delta_y1 = p0_y - p1_y;

            double delta_x2 = p2_x - p3_x;
            double delta_y2 = p2_y - p3_y;

            double m1;
            double m2;

            if(delta_x1 == 0 && delta_x2 == 0)//colinear
                return false;

            if(delta_x1==0){//find the point on line 2
                m2 = (delta_y2/delta_x2);
                double y = m2 * (p0_x - p2_x) + p2_y;
                coord r(p0_x, y);
                result = r;
                return true;
            }
            if(delta_x2==0){//find the point on line 1
                m1 = (delta_y1/delta_x1);
                double y = m1 * (p2_x - p0_x) + p0_y;
                coord r(p2_x,y);
                result = r;
                return true;
            }

            m1 = (delta_y1/delta_x1);
            m2 = (delta_y2/delta_x2);

            if(m1==m2)//No Collision parallels
                return false;

            double x = (- m1 * p0_x + m2 * p2_x + p0_y - p2_y) / (m2 - m1);
            double y = m2 * (x - p2_x) + p2_y;
            coord r(x,y);
            result = r;
            return true;
        }
        bool poligon_clipping(int wid, int hei) {
            std::vector<coord> coords, oldCoords;
            std::vector<coord> vpCoords = {coord(10,10), coord(10,hei-10), coord(wid-10,hei-10), coord(wid-10,10)};
            oldCoords.assign(vp.begin(), vp.end());
            std::cout << "NEW CLIPPING!!!" << std::endl;

            for (int j = 0; j < oldCoords.size(); j++) {

                std::vector<coord> polygon_line = {coord(oldCoords[j].x, oldCoords[j].y),
                    coord(oldCoords[(j+1)%oldCoords.size()].x,
                            oldCoords[(j+1)%oldCoords.size()].y)};
                int count = 0;
                std::cout << "-------------------------------------------------" << std::endl;
                for (int i = 0; i < vpCoords.size(); i++) {
                    std::vector<coord> window_line = {coord(vpCoords[i].x, vpCoords[i].y),
                    coord(vpCoords[(i+1)%vpCoords.size()].x,
                            vpCoords[(i+1)%vpCoords.size()].y)};

                    for (auto& i: window_line) {
                        std::cout << "Wline: " << i << std::endl;
                    }

                    for (auto& i: polygon_line) {
                        std::cout << "Pline: " << i << std::endl;
                    }
                    bool sourceOut = ((window_line[1].x - window_line[0].x)*(polygon_line[0].y - window_line[0].y) > (window_line[1].y - window_line[0].y)*(polygon_line[0].x - window_line[0].x));
                    bool targetOut = ((window_line[1].x - window_line[0].x)*(polygon_line[1].y - window_line[0].y) > (window_line[1].y - window_line[0].y)*(polygon_line[1].x - window_line[0].x));


                    if (sourceOut && !targetOut) {
                        std::cout << "Entered!" << std::endl;
                        coord out;
                        intersection(polygon_line, window_line, out);
                        std::cout << "Emplaced!! " << out << std::endl;
                        coords.push_back(out);
                        coords.push_back(polygon_line[1]);

                    }
                    if (targetOut && !sourceOut) {
                        std::cout << "Entered1!" << std::endl;
                        coord out;
                        intersection(polygon_line, window_line, out);
                        coords.push_back(polygon_line[0]);
                        std::cout << "Emplaced!! " << out << std::endl;
                        coords.push_back(out);

                    }
                    if (!targetOut && !sourceOut) {
                        count++;
                    }

                }
                if (count == vpCoords.size()) {
                    coords.push_back(polygon_line[0]);
                    coords.push_back(polygon_line[1]);
                }
            }
            if (coords.size() == 0) {
                return false;
            }
            vp.assign(coords.begin(), coords.end());

            return true;
        }

        bool point_clipping(int wid, int hei) {
            double x1 = (*std::begin(vp)).x;
            double y1 = (*std::begin(vp)).y;
            if (x1 >= wid-10 || x1 <= 10) {
                return false;
            } else if (y1 >=hei-10 || y1 <= 10) {
                return false;
            }
            return true;
        }

        bool line_clipping(int wid, int hei) {
            int xmin= 10, xmax = wid-10, ymin = 10 ,ymax = hei-10;
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
            p1 = -1*(x2 - x1);
            lu1.emplace_back(p1);
            p2 = x2 - x1;
            lu1.emplace_back(p2);
            p3 = -1*(y2 - y1);
            lu1.emplace_back(p3);
            p4 = y2 - y1;
            lu1.emplace_back(p4);

            q1 = x1 - xmin;
            lu2.emplace_back(q1);
            q2 = xmax - x1;
            lu2.emplace_back(q2);
            q3 = y1 - ymin;
            lu2.emplace_back(q3);
            q4 = ymax - y1;
            lu2.emplace_back(q4);
            // if ((p1 == 0 || p2 == 0 || p3 == 0 || p4 == 0) && (q1 < 0 || q2 < 0 || q3 < 0 || q4 < 0)) {
            // return false;
            // }
            std::vector<double> r;
            double u1 = 0.0, u2 = 1.0;
            for (unsigned int i = 0; i < lu1.size(); ++i) {
                if (lu1[i] == 0 && lu2[i] < 0) {
                    return false;
                }
                r.emplace_back(lu2[i] / lu1[i]);
                if (lu1[i] < 0)
                    u1 = std::max({u1, r[i]});
                if (lu1[i] > 0)
                    u2 = std::min({u2, r[i]});
            }

            if (u1 > u2) {
                return false;
            }
            if (u1 > 0) {
                (*it).x = x1 + u1*(x2-x1);
                (*it).y = y1 + u1*(y2-y1);
            }
            it++;
            if (u2 < 1) {
                (*it).x = x1 + u2*(x2-x1);
                (*it).y = y1 + u2*(y2-y1);
            }
            return true;
        }

        gint16 type() {
            if (orig.size() > 2) {
                return 3;
            }
            return orig.size();
        }

        void viewport(GtkWidget* area) {
            vp.assign(orig.begin(), orig.end());
            auto it1 = normCoord.begin(), it2 = vp.begin();
            for (; it1 != normCoord.end(); ++it1, ++it2) {
                (*it2).x = ((((*it1).x +1) / (2)) * (384 - 0)) + 10;
                (*it2).y = ((1 - (((*it1).y +1) / (2))) * (384 - 0)) + 10;
            }
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
