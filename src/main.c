#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "lisp.h"

int main(int argc, char *argv[])
{
  lisp_init();
  lisp_terminate();
  return 0;
}
