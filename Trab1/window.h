#ifndef WINDOW_H
#define WINDOW_H
#include "structures.hpp"
class Window {
    private:
        coord rsw;
        coord riw;
        coord lsw;
        coord liw;
        float ymin;
        float ymax;
        float xmin;
        float xmax;

    public:
        Window(float xmax, float xmin, float ymax, float ymin) {
            this->xmax = xmax;
            this->xmin = xmin;
            this->ymax = ymax;
            this->ymin = ymin;
            this->rsw = new coord(xmax,ymax);
            this->riw = new coord(xmax,ymin);
            this->lsw = new coord(xmin,ymax);
            this->liw = new coord(xmin,ymin);
        }
        // void viewportTransform(float *x, float *y);
}

#endif
