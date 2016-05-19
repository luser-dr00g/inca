#ifndef ENCODING_H_
#define ENCODING_H_
#include "common.h"

extern object null;
extern object mark;
extern object nil;
extern object blank;

void init_en();

int gettag(object d);
int getval(object d);
object newdata(int tag, int val);

object cache(int tag, void *ptr);
void *getptr(object d);
object getfill(object d);

#endif
