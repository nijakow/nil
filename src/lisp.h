#ifndef NIL_LISP_H
#define NIL_LISP_H

#include "any.h"

struct lisp_obj
{
  struct
  {
    unsigned int mark : 2;
    unsigned int bytes : 1;
    unsigned int word_size : 29;
  } header;
};

static inline void* lisp_obj_payload(struct lisp_obj* obj)
{
  return (((char*) obj) + sizeof(struct lisp_obj));
}

static inline any* lisp_obj_at(struct lisp_obj* obj, unsigned int index)
{
  return &((any*) lisp_obj_payload(obj))[index];
}

static inline char* lisp_obj_char_at(struct lisp_obj* obj, unsigned int index)
{
  return &((char*) lisp_obj_payload(obj))[index];
}


any lisp_alloc(enum lisp_type, unsigned int);
void lisp_mark_object(any);
void lisp_gc();

any lisp_cons(any, any);

static inline any lisp_car(any c)
{
  return *lisp_obj_at(deref(c), 0);
}

static inline any lisp_cdr(any c)
{
  return *lisp_obj_at(deref(c), 1);
}

static inline void lisp_rplaca(any c, any v)
{
  *lisp_obj_at(deref(c), 0) = v;
}

static inline void lisp_rplacd(any c, any v)
{
  *lisp_obj_at(deref(c), 1) = v;
}

any lisp_getsym(const char*);

void lisp_init();
void lisp_terminate();

#endif

