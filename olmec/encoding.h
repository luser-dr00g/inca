#ifndef ENCODING_H_
#define ENCODING_H_
#include "common.h"

extern int null;
extern int mark;
extern int nil;

void init_en();

int gettag(int d);
int getval(int d);
int newdata(int tag, int val);

int cache(int tag, void *ptr);
void *getptr(int d);
int getfill(int d);

#endif
