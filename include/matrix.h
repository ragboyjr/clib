#ifndef _LIB_MATRIX_H
#define _LIB_MATRIX_H

union matrix_datum {
    int i;
    double d;
};

typedef struct _matrix_t {
    /*union matrix_datum * data;*/
    int * data;
    int rows,
        cols;
} matrix_t;

matrix_t * matrix_create(int rows, int cols);
void matrix_destroy(matrix_t *);

#define matrix_idx(m, r, c) (m)->data[(r) * (m)->cols + (c)]

#endif
