#ifndef MATRIX_HPP
#define MATRIX_HPP

using namespace std;

template<typename T>
matrix<T>::matrix(int r, int c) {
    row = r;
    columns = c;
    elem = (T*) calloc(this->size(), sizeof(T));
}

template<typename T>
int matrix<T>::size() {
    return this->row*this->columns;
}

template<typename T>
void matrix<T>::display() {
    cout <<"The matrix is:\n";
    for(int i = 0; i < row; i++) {
        for(int j = 0; j < columns; j++) {
            cout << elem[i*row+j] <<" ";
        }
        cout <<endl;
    }
}

template<typename T>
matrix<T>& matrix<T>::operator+ (const matrix& m1) {
    matrix<T> *mcopy = new matrix<T>(row, columns);
    for(int i = 0; i < row; i++) {
        for(int j = 0; j < columns; j++) {
            mcopy->matrixGet(i,j) = this->elem[i*row+j] + m1.elem[i*row+j];
        }
    }
    return *mcopy;

}

template<typename T>
matrix<T>& matrix<T>::operator* (const matrix& m1) {
    matrix<T> *mcopy = new matrix<T>(row, m1.columns);
    if(columns == m1.row) {
        for(int i = 0; i < row; ++i) {
            for(int k = 0; k < m1.columns; ++k) {
                for(int inner = 0; inner < columns; ++inner) {
                   mcopy->matrixGet(i,k) += this->elem[i*columns+inner] * m1.elem[inner*columns+k];
                }
            }
        }
    }

    return *mcopy;
}

// template<typename T>
// matrix<T>& matrix<T>::operator+= (const matrix& rhs) {
//     *this = this +
//     return *this;
//
// }
//
// template<typename T>
// matrix<T>& matrix<T>::operator*= (const matrix& A) {
// }

template<typename T>
matrix<T>& matrix<T>::operator= (const matrix& A) {
    elem = A.elem;
    columns = A.columns;
    row = A.row;

    return *this;
}

template<typename T>
T& matrix<T>::matrixGet(int h, int w) const{
    return this->elem[h*row+w];
}

template<typename T>
T& matrix<T>::operator()(size_t h, size_t w) const {
    return this->matrixGet(h,w);
}
#endif
