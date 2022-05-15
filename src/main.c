#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "lisp.h"
#include "reader.h"

int main(int argc, char *argv[])
{
  lisp_init();
  lisp_reader_read(NULL);
  lisp_terminate();
  return 0;
}
