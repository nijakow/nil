#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lisp.h"

#define LISP_HEAP_SIZE (1024 * 1024)

struct lisp_heap
{
  unsigned long alloc;
  char data[LISP_HEAP_SIZE];
};

struct lisp_heap LISP_HEAPS[2];
unsigned int LISP_CURRENT_HEAP = 0;
any LISP_SYMBOL_TABLE = LISP_NIL;


static inline void* lisp_heap_here(struct lisp_heap* heap)
{
  return &heap->data[heap->alloc];
}

static inline void* lisp_heap_alloc_bytes(struct lisp_heap* heap, unsigned long size)
{
  void* ptr;

  ptr = lisp_heap_here(heap);
  heap->alloc += size;
  return ptr;
}



any lisp_alloc(enum lisp_type type, unsigned int word_size)
{
  struct lisp_obj* obj;

  obj = (struct lisp_obj*) lisp_heap_alloc_bytes(&LISP_HEAPS[LISP_CURRENT_HEAP],
                                                 sizeof(struct lisp_obj*) + word_size * sizeof(any));

  if (obj != NULL)
    {
      obj->header.mark = 0;
      obj->header.bytes = 0;
      obj->header.word_size = word_size;
      // TODO: Initialize type
      // TODO: Initialize body
    }

  return ref(obj);
}

void lisp_mark_object(any object)
{
  unsigned int i;
  struct lisp_obj* ptr;

  if (!is_ref(object) || object == LISP_NIL)
    return;
  ptr = deref(object);
  if (!ptr->header.mark)
    {
      ptr->header.mark = 0x01;
      if (!ptr->header.bytes)
        {
          for (i = 0; i < ptr->header.word_size; i++)
            lisp_mark_object(*lisp_obj_at(ptr, i));
        }
    }
  ptr->header.mark = 0x03;
}

void lisp_mark()
{
  lisp_mark_object(LISP_SYMBOL_TABLE);
}

void lisp_sweep()
{
  char* ptr;
  struct lisp_obj* obj;
  struct lisp_obj* obj2;
  unsigned int i;

  LISP_HEAPS[!LISP_CURRENT_HEAP].alloc = 0;
  ptr = LISP_HEAPS[LISP_CURRENT_HEAP].data;
  while (ptr < (char*) lisp_heap_here(&LISP_HEAPS[LISP_CURRENT_HEAP]))
    {
      obj = (struct lisp_obj*) ptr;
      ptr += sizeof(struct lisp_obj) + obj->header.word_size * sizeof(any);
      if (obj->header.mark)
        {
          obj2 = lisp_heap_alloc_bytes(&LISP_HEAPS[!LISP_CURRENT_HEAP],
                                       sizeof(struct lisp_obj) + obj->header.word_size * sizeof(any));
          memcpy(obj2, obj, sizeof(struct lisp_obj) + obj->header.word_size * sizeof(any));
          *lisp_obj_at(obj, 0) = ref(obj2);
        }
    }
  LISP_CURRENT_HEAP = !LISP_CURRENT_HEAP;
  ptr = LISP_HEAPS[LISP_CURRENT_HEAP].data;
  while (ptr < (char*) lisp_heap_here(&LISP_HEAPS[LISP_CURRENT_HEAP]))
    {
      obj = (struct lisp_obj*) ptr;
      ptr += sizeof(struct lisp_obj) + obj->header.word_size * sizeof(any);
      obj->header.mark = 0x00;
      if (!obj->header.bytes)
        {
          for (i = 0; i < obj->header.word_size; i++)
            {
              any* slot = lisp_obj_at(obj, i);
              if (is_ref(*slot) && *slot != LISP_NIL)
                  *slot = *lisp_obj_at(deref(*slot), 0);
            }
        }
    }
}

void lisp_gc()
{
  lisp_mark();
  lisp_sweep();
}

any lisp_cons(any car, any cdr)
{
  any cell;

  cell = lisp_alloc(TYPE_CONS, 2);
  lisp_rplaca(cell, car);
  lisp_rplacd(cell, cdr);

  return cell;
}

any lisp_getsym(const char* text)
{
  return LISP_NIL;
}


void lisp_init()
{
  LISP_HEAPS[0].alloc = 0;
  LISP_HEAPS[1].alloc = 0;
}

void lisp_terminate()
{
}
