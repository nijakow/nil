#ifndef NIL_SECD_H
#define NIL_SECD_H

#include "lisp.h"

struct secd
{
  any s;
  any e;
  any c;
  any d;
};

any run_expr(any);
void secd_run(struct secd*);

#endif

