#ifndef AR_H_
#define AR_H_
#include "../ppnarg.h"

typedef struct ar {
    int type;
    int rank;    // number of dimensions
    int *dims;   // size of each dimension
    int cons;    // constant term of the indexing formula
    int *weight; // corresponding coefficient in the indexing formula
    int *data;   // address of first array element
    int *(*func)(struct ar *,int); // data function (if function type)
} *array;

enum type {
    normal,
    indirect,
    function
};

int productdims(int rank, int *dims);
array array_new_dims(int rank, int *dims);
array array_new_function(int rank, int *dims,
        int *data, int datan, int *(*func)(array,int)); // type=function
int *constant(array a,int idx);
int *j_vector(array a,int idx);
void loaddimsv(int rank, int *dims, va_list ap);
array (array_new)(int rank, ...);
#define array_new(...) (array_new)(PP_NARG(__VA_ARGS__),__VA_ARGS__)
array cast_dims(int *data, int rank, int *dims); // type=indirect
array (cast)(int *data, int rank, ...); // type=indirect
#define cast(data,...) (cast)(data,PP_NARG(__VA_ARGS__),__VA_ARGS__)
array clone(array a); // type=indirect
array copy(array a);

int *elema(array a, int *ind);
int *elemv(array a, va_list ap);
int *elem(array a, ...);

int *vector_index(int ind, int *dims, int n, int *vec);
int ravel_index(int *vec, int *dims, int n);

void transpose2(array a);
void transpose(array a, int shift);
void transposea(array a, int *spec);
array slice(array a, int i); // type=indirect
array slicea(array a, int *spec); // type=indirect
array slices(array a, int *s, int *f); // type=indirect
array extend(array a, int extra); // type=indirect

array cat(array x, array y);
array iota(int n); // type=function
array scalar(int n);
array (vector)(int n, ...);
#define vector(...) (vector)(PP_NARG(__VA_ARGS__),__VA_ARGS__)

int issolid(array a);
array makesolid(array a);

#endif
