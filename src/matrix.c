#include "lib/matrix.h"

#include <stdlib.h>

matrix_t * matrix_create(int rows, int cols)
{
    matrix_t * this = malloc(sizeof(matrix_t));
    
    this->rows = rows,
    this->cols = cols,
    this->data = malloc(sizeof(int) * this->rows * this->cols);
    
    return this;
}

void matrix_destroy(matrix_t * this)
{
    free(this->data);
    free(this);
}
