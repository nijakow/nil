#include "secd.h"

enum instruction_type
  {
    IT_NIL,
    IT_LDC,
    IT_LD,
    IT_SEL,
    IT_JOIN,
    IT_LDF,
    IT_AP,
    IT_RET,
    IT_DUM,
    IT_RAP
  };

static inline any secd_c(struct secd* secd)
{
  any v;
  v = secd->c;
  secd->c = lisp_cdr(v);
  return v;
}

static inline void secd_push(struct secd* secd, any v)
{
  secd->s = lisp_cons(v, secd->s);
}

static inline any secd_pop(struct secd* secd)
{
  any v;
  v = lisp_car(secd->s);
  secd->s = lisp_cdr(secd->s);
  return v;
}

static inline void secd_pushd(struct secd* secd, any v)
{
  secd->d = lisp_cons(v, secd->d);
}

static inline any secd_popd(struct secd* secd)
{
  any v;
  v = lisp_car(secd->d);
  secd->d = lisp_cdr(secd->d);
  return v;
}

static inline void secd_preserve(struct secd* secd)
{
  secd_pushd(secd, secd->s);
  secd_pushd(secd, secd->e);
  secd_pushd(secd, secd->c);
}

static inline void secd_restore(struct secd* secd)
{
  secd->c = secd_popd(secd);
  secd->e = secd_popd(secd);
  secd->s = secd_popd(secd);
}

any secd_lookup(struct secd* secd, any pos)
{
  int x;
  int y;
  int i;
  any p;

  x = intval(lisp_car(pos));
  y = intval(lisp_cdr(pos));
  p = secd->e;
  for (i = 0; i < x; i++)
    p = lisp_cdr(p);
  p = lisp_car(p);
  for (i = 0; i < y; i++)
    p = lisp_cdr(p);
  return lisp_car(p);
}

void secd_ap(struct secd* secd)
{
  any func;
  any arglist;
  int args;
  int i;

  args = intval(secd_c(secd));
  arglist = LISP_NIL;
  for (i = 0; i < args; i++)
    arglist = lisp_cons(secd_pop(secd), arglist);
  func = secd_pop(secd);
  secd_preserve(secd);
  secd->s = LISP_NIL;
  secd->c = lisp_car(func);
  secd->e = lisp_cons(arglist, lisp_cdr(func));
}

void secd_ret(struct secd* secd)
{
  any v;

  v = secd_pop(secd);
  secd_restore(secd);
  secd_push(secd, v);
}

void secd_run(struct secd* secd)
{
  any instruction;
  any x;
  any y;
  any z;

  while (1)
    {
      instruction = secd_c(secd);
      switch (intval(instruction))
        {
        case IT_NIL:
          secd_push(secd, LISP_NIL);
          break;
        case IT_LDC:
          secd_push(secd, secd_c(secd));
          break;
        case IT_LD:
          secd_push(secd, secd_lookup(secd, secd_c(secd)));
          break;
        case IT_SEL:
          x = secd_c(secd);
          y = secd_c(secd);
          z = secd_pop(secd);
          secd_pushd(secd, secd->c);
          if (z == LISP_NIL)
            secd->c = y;
          else
            secd->c = x;
          break;
        case IT_JOIN:
          secd->c = secd_popd(secd);
          break;
        case IT_LDF:
          secd_push(secd, lisp_cons(secd_c(secd), secd->e));
          break;
        case IT_AP:
          secd_ap(secd);
          break;
        case IT_RET:
          secd_ret(secd);
          break;
        case IT_DUM:
          secd_pushd(secd, LISP_NIL);
          break;
        case IT_RAP:
          /* TODO */
          break;
        }
    }
}

