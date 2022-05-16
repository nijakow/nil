#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lisp.h"
#include "secd.h"

#define LISP_HEAP_SIZE (1024 * 1024)

struct lisp_heap
{
  unsigned long alloc;
  char data[LISP_HEAP_SIZE];
};

struct lisp_heap LISP_HEAPS[2];
unsigned int LISP_CURRENT_HEAP = 0;
any LISP_SYMBOL_TABLE = LISP_NIL;

struct secd THE_SECD;


static inline void* lisp_heap_here(struct lisp_heap* heap)
{
  return &heap->data[heap->alloc];
}

static inline void* lisp_heap_alloc_bytes(struct lisp_heap* heap, unsigned long size)
{
  void* ptr;

  ptr = lisp_heap_here(heap);
  if ((size & 0x07) != 0)
    size = (size & ~((unsigned long) 0x07)) + 0x08;
  heap->alloc += size;
  printf("Alloc is %lu bytes, before %p, now %p, %lu bytes in use\n",
         size,
         ptr,
         &heap->data[heap->alloc],
         heap->alloc);
  return ptr;
}



any lisp_alloc(unsigned int size)
{
  struct lisp_obj* obj;
  unsigned int bsize;

  obj = (struct lisp_obj*) lisp_heap_alloc_bytes(&LISP_HEAPS[LISP_CURRENT_HEAP], size);

  if (obj != NULL)
    {
      obj->header.mark = 0;
      obj->header.bytes = 0;
      bsize = size - sizeof(struct lisp_obj);
      obj->header.word_size = (bsize / sizeof(any)) + (((bsize % sizeof(any)) == 0) ? 0 : 1);
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
  printf("Marking %p\n", ptr);
  if (!ptr->header.mark)
    {
      ptr->header.mark = 0x01;
      if (!ptr->header.bytes)
        {
          for (i = 0; i < ptr->header.word_size; i++)
            {
              lisp_mark_object(*lisp_obj_at(ptr, i));
            }
        }
    }
  ptr->header.mark = 0x03;
}

void lisp_mark()
{
  lisp_mark_object(LISP_SYMBOL_TABLE);
  lisp_mark_object(THE_SECD.s);
  lisp_mark_object(THE_SECD.e);
  lisp_mark_object(THE_SECD.c);
  lisp_mark_object(THE_SECD.d);
}

void lisp_update_any(any* slot)
{
  if (is_ref(*slot) && *slot != LISP_NIL)
    *slot = *lisp_obj_at(deref(*slot), 0);
}

void lisp_update_roots()
{
  lisp_update_any(&LISP_SYMBOL_TABLE);
  lisp_update_any(&THE_SECD.s);
  lisp_update_any(&THE_SECD.e);
  lisp_update_any(&THE_SECD.c);
  lisp_update_any(&THE_SECD.d);
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
              lisp_update_any(lisp_obj_at(obj, i));
            }
        }
    }
  lisp_update_roots();
}

void lisp_gc()
{
  lisp_mark();
  lisp_sweep();
}

#include <unistd.h>
void lisp_opt_gc()
{
  if ((LISP_HEAP_SIZE - LISP_HEAPS[LISP_CURRENT_HEAP].alloc) < LISP_HEAP_SIZE / 4)
    {
      printf("Triggering GC!\n");
      sleep(1);
      lisp_gc();
    }
}

any lisp_cons(any car, any cdr)
{
  any cell;

  cell = lisp_alloc(sizeof(struct lisp_obj) + sizeof(any) * 2);
  lisp_rplaca(cell, car);
  lisp_rplacd(cell, cdr);

  return cell;
}


any lisp_getstr(const char* text)
{
  any name;
  size_t len;
  size_t x;

  len = strlen(text);
  name = lisp_alloc(sizeof(struct lisp_obj) + (len + 1) * sizeof(char));
  ((struct lisp_obj*) deref(name))->header.bytes = 1;
  for (x = 0; x <= len; x++)
    *lisp_obj_char_at(deref(name), x) = text[x];
  return name;
}

any lisp_getsym(const char* text)
{
  any sym;
  any name;
  unsigned int x;

  for (sym = LISP_SYMBOL_TABLE;
       sym != LISP_NIL;
       sym = *lisp_obj_at(deref(sym), 0))
    {
      name = *lisp_obj_at(deref(sym), 1);
      x = 0;
      while (1)
        {
          if (text[x] != *lisp_obj_char_at(deref(name), x))
            break;
          else if (text[x] == '\0')
            return sym;
          x = x + 1;
        }
    }
  name = lisp_getstr(text);
  sym = lisp_alloc(sizeof(struct lisp_obj) + 5 * sizeof(any));
  *lisp_obj_at(deref(sym), 0) = LISP_SYMBOL_TABLE;
  *lisp_obj_at(deref(sym), 1) = name;
  *lisp_obj_at(deref(sym), 2) = sym;
  *lisp_obj_at(deref(sym), 3) = LISP_NIL;
  *lisp_obj_at(deref(sym), 4) = LISP_NIL;
  LISP_SYMBOL_TABLE = sym;
  return sym;
}


void lisp_init()
{
  LISP_HEAPS[0].alloc = 0;
  LISP_HEAPS[1].alloc = 0;
  THE_SECD.s = LISP_NIL;
  THE_SECD.e = LISP_NIL;
  THE_SECD.c = LISP_NIL;
  THE_SECD.d = LISP_NIL;
}

void lisp_terminate()
{
}
