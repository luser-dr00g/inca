#ifndef AR_H_
#define AR_H_
#include "../ppnarg.h"

typedef struct ar {
    int type;
    enum {
        none = 0;
        temp = 1;
    } flag;
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

extern array nilarray;
void init_array(void);

int productdims(int rank, const int *dims);
array array_new_rank_pdims(int rank, const int *dims); // type=normal

void loaddimsv(int rank, int *dims, va_list ap);
array (array_new_rank_dims)(int rank, ...); // type=normal
#define array_new_dims(...) (array_new_rank_dims)(PP_NARG(__VA_ARGS__),__VA_ARGS__)

int *constant(array a,int idx);
int *j_vector(array a,int idx);
array array_new_function(int rank, const int *dims,
        const int *data, int datan, int *(*func)(array,int)); // type=function

array cast_rank_pdims(int *data, int rank, const int *dims); // type=indirect
array (cast_rank_dims)(int *data, int rank, ...); // type=indirect
#define cast_dims(data,...) (cast_rank_dims)(data,PP_NARG(__VA_ARGS__),__VA_ARGS__)

array clone(array a); // type=indirect
array copy(array a); // type=normal
int issolid(array a);
array makesolid(array a); // type=normal

int *vector_index(int ind, const int *dims, int n, int *vec);
int ravel_index(const int *vec, const int *dims, int n);
int *elemr(array a, int idx);
int *elema(array a, const int *ind);
int *elemv(array a, va_list ap);
int *elem(array a, ...);

void transpose2(array a);
void transpose(array a, int shift);
void transposea(array a, const int *spec);

array slice(array a, int i); // type=indirect
array slicea(array a, const int *spec); // type=indirect
array slices(array a, const int *s, const int *f); // type=indirect
array extend(array a, int extra); // type=indirect

array scalar(int n);
array (vector_n)(int n, ...);
#define vector(...) (vector_n)(PP_NARG(__VA_ARGS__),__VA_ARGS__)

array cat(array x, array y);
array iota(int n); // type=function

#endif
