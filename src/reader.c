#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#include "reader.h"

#define READER_MAX_REFS 1024

struct reader
{
  char c;
  any refs[READER_MAX_REFS];
};

char here(struct reader* reader)
{
  return reader->c;
}

void forward(struct reader* reader)
{
  reader->c = getchar();
};

any lisp_reader_read_expr(struct reader*, any*);

void skip_whitespace(struct reader* reader)
{
  while (isspace(here(reader)))
    forward(reader);
}

int lisp_reader_parse_number(struct reader* reader, int* n)
{
  int v;

  if (!isdigit(here(reader)))
    return 0;

  v = 0;
  while (isdigit(here(reader)))
    {
      v = (v * 10) + (here(reader) - '0');
      forward(reader);
    }
  *n = v;
  return 1;
}

any lisp_reader_read_number(struct reader* reader)
{
  int v;

  assert(lisp_reader_parse_number(reader, &v));
  return intref(v);
}

any lisp_reader_read_reference(struct reader* reader)
{
  int n;

  assert(lisp_reader_parse_number(reader, &n));
  if (here(reader) == '#')
    {
      forward(reader);
      return reader->refs[n];
    }
  else
    {
      assert(here(reader) == '=');
      forward(reader);
      return lisp_reader_read_expr(reader, &reader->refs[n]);
    }
}

any lisp_reader_read_list(struct reader* reader, any* loc)
{
  any p;
  any v;

  skip_whitespace(reader);
  switch (here(reader))
    {
    case ')':
      forward(reader);
      return LISP_NIL;
    case '.':
      forward(reader);
      v = lisp_reader_read_expr(reader, NULL);
      skip_whitespace(reader);
      assert(here(reader) == ')');
      forward(reader);
      return v;
    default:
      p = lisp_cons(LISP_NIL, LISP_NIL);
      if (loc != NULL)
        *loc = p;
      lisp_rplaca(p, lisp_reader_read_expr(reader, NULL));
      lisp_rplacd(p, lisp_reader_read_list(reader, NULL));
      return p;
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

any lisp_reader_read_expr(struct reader* reader, any* loc)
{
  skip_whitespace(reader);
  if (isdigit(here(reader)))
    return lisp_reader_read_number(reader);
  switch (here(reader))
    {
    case '(':
      forward(reader);
      return lisp_reader_read_list(reader, loc);
    case '#':
      forward(reader);
      return lisp_reader_read_reference(reader);
    default:
      return lisp_reader_read_symbol(reader);
    }
}

any lisp_reader_read(char* ptr)
{
  struct reader reader;

  forward(&reader);
  return lisp_reader_read_expr(&reader, NULL);
}
