#ifndef AR_H_
#define AR_H_
#include "../ppnarg.h"

typedef struct ar {
    int rank;    // number of dimensions
    int *dims;   // size of each dimension
    int *weight; // corresponding coefficient in the indexing formula
    int *data;   // address of first array element
} *array;

int productdims(int rank, int dims[]);
array array_new_dims(int rank, int dims[]);
void loaddimsv(int rank, int dims[], va_list ap);
array (array_new)(int rank, ...);
#define array_new(...) (array_new)(PP_NARG(__VA_ARGS__),__VA_ARGS__)
array cast_dims(int data[], int rank, int dims[]);
array (cast)(int data[], int rank, ...);
#define cast(data,...) (cast)(data,PP_NARG(__VA_ARGS__),__VA_ARGS__)
array clone(array a);
array copy(array a);

int *elema(array a, int ind[]);
int *elemv(array a, va_list ap);
int *elem(array a, ...);

int *vector_index(int ind, int dims[], int n, int vec[]);
int ravel_index(int vec[], int dims[], int n);

void transpose2(array a);
void transpose(array a, int shift);
void transposea(array a, int spec[]);
array slice(array a, int i);
array slicea(array a, int spec[]);
array slices(array a, int s[], int f[]);
array extend(array a, int extra);

array cat(array x, array y);
array iota(int n);
array scalar(int n);

#endif
