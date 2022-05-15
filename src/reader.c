#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#include "reader.h"

struct reader
{
  char c;
};

char here(struct reader* reader)
{
  return reader->c;
}

void forward(struct reader* reader)
{
  reader->c = getchar();
};

any lisp_reader_read_expr(struct reader*);

void skip_whitespace(struct reader* reader)
{
  while (isspace(here(reader)))
    forward(reader);
}

any lisp_reader_read_number(struct reader* reader)
{
  int v;

  v = 0;
  while (isdigit(here(reader)))
    {
      v = (v * 10) + (here(reader) - '0');
      forward(reader);
    }
  return intref(v);
}

any lisp_reader_read_list(struct reader* reader)
{
  any v;

  skip_whitespace(reader);
  switch (here(reader))
    {
    case ')':
      forward(reader);
      return LISP_NIL;
    case '.':
      forward(reader);
      v = lisp_reader_read_expr(reader);
      skip_whitespace(reader);
      assert(here(reader) == ')');
      forward(reader);
      return v;
    default:
      v = lisp_reader_read_expr(reader);
      return lisp_cons(v, lisp_reader_read_list(reader));
    }
}

int is_special_char(char c)
{
  return isspace(c) || (c == ')');
}

any lisp_reader_read_symbol(struct reader* reader)
{
  unsigned int fill;
  char buffer[1024];

  fill = 0;
  while (!is_special_char(here(reader))) /* TODO: Overflow check! */
    {
      buffer[fill++] = here(reader);
      forward(reader);
    }
  buffer[fill++] = '\0';
  return lisp_getsym(buffer);
}

any lisp_reader_read_expr(struct reader* reader)
{
  skip_whitespace(reader);
  if (isdigit(here(reader)))
    return lisp_reader_read_number(reader);
  switch (here(reader))
    {
    case '(':
      forward(reader);
      return lisp_reader_read_list(reader);
    default:
      return lisp_reader_read_symbol(reader);
    }
}

any lisp_reader_read(char* ptr)
{
  struct reader reader;

  forward(&reader);
  return lisp_reader_read_expr(&reader);
}
