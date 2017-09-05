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
                return line_clipping(wid, hei, 1);
            }
            return poligon_clipping(wid, hei);
        }

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
            // std::cout << "NEW CLIPPING!!!" << std::endl;

            for (int j = 0; j < oldCoords.size(); j++) {

                std::vector<coord> polygon_line = {coord(oldCoords[j].x, oldCoords[j].y),
                    coord(oldCoords[(j+1)%oldCoords.size()].x,
                            oldCoords[(j+1)%oldCoords.size()].y)};
                int count = 0;
                // std::cout << "-------------------------------------------------" << std::endl;
                for (int i = 0; i < vpCoords.size(); i++) {
                    std::vector<coord> window_line = {coord(vpCoords[i].x, vpCoords[i].y),
                    coord(vpCoords[(i+1)%vpCoords.size()].x,
                            vpCoords[(i+1)%vpCoords.size()].y)};

                    // for (auto& i: window_line) {
                        // std::cout << "Wline: " << i << std::endl;
                    // }

                    // for (auto& i: polygon_line) {
                        // std::cout << "Pline: " << i << std::endl;
                    // }
                    bool sourceOut = ((window_line[1].x - window_line[0].x)*(polygon_line[0].y - window_line[0].y) > (window_line[1].y - window_line[0].y)*(polygon_line[0].x - window_line[0].x));
                    bool targetOut = ((window_line[1].x - window_line[0].x)*(polygon_line[1].y - window_line[0].y) > (window_line[1].y - window_line[0].y)*(polygon_line[1].x - window_line[0].x));


                    if (sourceOut && !targetOut) {
                        // std::cout << "Entered!" << std::endl;
                        coord out;
                        intersection(polygon_line, window_line, out);
                        // std::cout << "Emplaced!! " << out << std::endl;
                        coords.push_back(out);
                        coords.push_back(polygon_line[1]);

                    }
                    if (targetOut && !sourceOut) {
                        // std::cout << "Entered1!" << std::endl;
                        coord out;
                        intersection(polygon_line, window_line, out);
                        coords.push_back(polygon_line[0]);
                        // std::cout << "Emplaced!! " << out << std::endl;
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

        bool line_clipping(int wid, int hei, int type) {
            if (type) {
                return cohen_suterland(wid, hei);
            }
            return liang_barsky(wid, hei);
        }

        bool cohen_suterland(int wid, int hei) {
            int u = 8, l = 1, r = 2,  b = 4, ul = 9, ur = 10, bl = 5, br = 6, i = 0;
            int xmin = 10, xmax = wid-10, ymin = 10, ymax = hei-10;
            int RC1 = 0, RC2 = 0;
            auto source = std::begin(vp), target = --std::end(vp);
            //RC1
            if ((*source).x < xmin) {
                if ((*source).y < ymin) {
                    RC1 = bl;
                } else if ((*source).y > ymax) {
                    RC1 = ul;

                } else {
                    RC1 = l;
                }
            } else if ((*source).x > xmax) {
                if ((*source).y < ymin) {
                    RC1 = br;

                } else if ((*source).y > ymax) {
                    RC1 = ur;
                } else {
                    RC1 = r;
                }
            } else {
                if ((*source).y < ymin) {
                    RC1 = b;

                } else if ((*source).y > ymax) {
                    RC1 = u;
                } else {
                    RC1 = i;
                }
            }
            // RC2
            if ((*target).x < xmin) {
                if ((*target).y < ymin) {
                    RC2 = bl;
                } else if ((*target).y > ymax) {
                    RC2 = ul;

                } else {
                    RC2 = l;
                }
            } else if ((*target).x > xmax) {
                if ((*target).y < ymin) {
                    RC2 = br;

                } else if ((*target).y > ymax) {
                    RC2 = ur;
                } else {
                    RC2 = r;
                }
            } else {
                if ((*target).y < ymin) {
                    RC2 = b;

                } else if ((*target).y > ymax) {
                    RC2 = u;
                } else {
                    RC2 = i;
                }
            }


            if ((RC1 == RC2) && RC1 == 0) {
                return true;
            }
            int RCr = RC1 & RC2;
            std::cout << "RCr: " << RCr << std::endl;
            if (RCr != 0) {
                return false;
            }
            if (RC1 != RC2) {
                if(!(RC1 & RC2)) {
                    double m = ((*target).y - (*source).y) / ((*target).x - (*source).x);
                    //source
                    if (RC1 != 0) {
                        if (RC1 == u) {
                            (*source).x = (*source).x + (1/m)*(ymax - (*source).y);
                            (*source).y = ymax;
                        }
                        if (RC1 == b) {
                            (*source).x = (*source).x + (1/m)*(ymin - (*source).y);
                            (*source).y = ymin;
                        }
                        if (RC1 == l) {
                            (*source).x = xmin;
                            (*source).y = m*(xmin - (*source).x) + (*source).y;
                        }
                        if (RC1 == r) {
                            (*source).x = xmax;
                            (*source).y = m*(xmax - (*source).x) + (*source).y;
                        }
                        if (RC1 == ul) {
                            (*source).x = xmin;
                            (*source).y = ymax;
                        }
                        if (RC1 == ur) {
                            (*source).x = xmax;
                            (*source).y = ymax;
                        }
                        if (RC1 == bl) {
                            (*source).x = xmin;
                            (*source).y = ymin;
                        }
                        if (RC1 == br) {
                            (*source).x = xmax;
                            (*source).y = ymin;
                        }
                    }
                    //target
                    if (RC2 != 0) {
                        std::cout << "Entered RC2!" << RC2 << std::endl;
                        if (RC2 == u) {
                            (*target).x = (*source).x + (1/m)*(ymax - (*source).y);
                            (*target).y = ymax;
                        }
                        if (RC2 == b) {
                            (*target).x = (*source).x + (1/m)*(ymin - (*source).y);
                            (*target).y = ymin;
                        }
                        if (RC2 == l) {
                            (*target).x = xmin;
                            (*target).y = m*(xmin - (*source).x) + (*source).y;
                        }
                        if (RC2 == r) {
                            (*target).x = xmax;
                            (*target).y = m*(xmax - (*source).x) + (*source).y;
                        }
                        if (RC2 == ul) {
                            (*target).x = xmin;
                            (*target).y = ymax;
                        }
                        if (RC2 == ur) {
                            (*target).x = xmax;
                            (*target).y = ymax;
                        }
                        if (RC2 == bl) {
                            (*target).x = xmin;
                            (*target).y = ymin;
                        }
                        if (RC2 == br) {
                            (*target).x = xmax;
                            (*target).y = ymin;
                        }
                    }
                    return true;
                }
            }
            return false;
        }

        bool liang_barsky(int wid, int hei) {
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
