#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "lisp.h"
#include "reader.h"
#include "secd.h"

int main(int argc, char *argv[])
{
  any v;

  lisp_init();
  v = lisp_reader_read(NULL);
  run_expr(v);
  lisp_terminate();
  return 0;
}
