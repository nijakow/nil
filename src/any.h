#ifndef NIL_ANY_H
#define NIL_ANY_H

#include <stdint.h>
#include <stdbool.h>

enum lisp_type
  {
    TYPE_CONS,
    TYPE_SYMBOL,
    TYPE_VECTOR,
    TYPE_INT,
    TYPE_CFUNC
  };

struct lisp_obj;

typedef uintptr_t any;

#define LISP_NIL ((any) 0)

static inline bool is_ref(any v)
{
  return (v & 0x07) == 0;
}

static inline struct lisp_obj* deref(any v)
{
  return (struct lisp_obj*) (v & ~((any) 0x07));
}

static inline any ref(void* v)
{
  return ((any) v);
}

static inline int intval(any v)
{
  return (int) (v >> 1);
}

static inline int intref(int v)
{
  return (((any) v) << 1) | 0x01;
}

#endif
