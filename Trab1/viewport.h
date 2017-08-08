#ifndef VIEWPORT_H
#define VIEWPORT_H
#include "structures.hpp"
class Viewport {
    private:
        coord rsv;
        coord riv;
        coord lsv;
        coord liv;
        float ymin;
        float ymax;
        float xmin;
        float xmax;

    public:
        Viewport(float xmax, float xmin, float ymax, float ymin) {
            this->xmax = xmax;
            this->xmin = xmin;
            this->ymax = ymax;
            this->ymin = ymin;
            this->rsv = new coord(xmax,ymax);
            this->riv = new coord(xmax,ymin);
            this->lsv = new coord(xmin,ymax);
            this->liv = new coord(xmin,ymin);
        }
        // void viewportTransform(float *x, float *y);
}

#endif
