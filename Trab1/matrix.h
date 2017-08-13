#ifndef MATRIX_H
#define MATRIX_H
#include <iostream>
#include <cstring>
#include <stdlib.h>

using namespace std;

template<typename T>
class matrix{
    private:
        int row;
        int columns;
        T* elem;
    public:
        matrix(int r, int c);

        int size();

        void display();

        matrix& operator+ (const matrix& m1);

        matrix& operator* (const matrix& m1);

        // matrix& operator+= (const matrix& rhs);

        // matrix& operator*= (const matrix& A);

        matrix& operator= (const matrix& A);

        T& matrixGet(int h, int w) const;

        T& operator()(size_t h, size_t w) const;
};
#include "matrix.hpp"
#endif
