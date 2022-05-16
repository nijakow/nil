#include <assert.h>
#include <stdio.h>

#include "secd.h"


enum instruction_type
  {
    IT_NIL,
    IT_LDC,
    IT_LD,
    IT_ST,
    IT_SEL,
    IT_JOIN,
    IT_LDF,
    IT_AP,
    IT_RET,
    IT_POP,
    IT_LDS,
    IT_STS,
    IT_BLT
  };

static inline any secd_c(struct secd* secd)
{
  any v;
  v = secd->c;
  secd->c = lisp_cdr(v);
  return lisp_car(v);
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

void secd_setq(struct secd* secd, any pos, any val)
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
  lisp_rplaca(p, val);
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

void secd_blt(struct secd* secd)
{
  int builtin;
  int args;
  any x;
  any y;
  any z;

  builtin = intval(secd_c(secd));
  args = intval(secd_c(secd));

  switch (builtin)
    {
    case 0: /* CONS */
      assert(args == 2);
      y = secd_pop(secd);
      x = secd_pop(secd);
      z = lisp_cons(x, y);
      secd_push(secd, z);
      break;
    case 1: /* EQ */
      assert(args == 2);
      y = secd_pop(secd);
      x = secd_pop(secd);
      secd_push(secd, (x == y) ? intref(1) : LISP_NIL); /* TODO: Return T */
      break;
    default:
      /* TODO: Error */
      break;
    }
}

void secd_run(struct secd* secd)
{
  any instruction;
  any x;
  any y;
  any z;
  int i;

  while (1)
    {
      lisp_opt_gc(); /* TODO: Only call this periodically */
      instruction = secd_c(secd);
      switch (intval(instruction))
        {
        case IT_NIL:
          printf("NIL\n");
          secd_push(secd, LISP_NIL);
          break;
        case IT_LDC:
          printf("LDC\n");
          secd_push(secd, secd_c(secd));
          break;
        case IT_LD:
          printf("LD\n");
          secd_push(secd, secd_lookup(secd, secd_c(secd)));
          break;
        case IT_ST:
          printf("ST\n");
          secd_setq(secd, secd_c(secd), secd_pop(secd));
          break;
        case IT_SEL:
          printf("SEL\n");
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
          printf("JOIN\n");
          secd->c = secd_c(secd);
          break;
        case IT_LDF:
          printf("LDF\n");
          secd_push(secd, lisp_cons(secd_c(secd), secd->e));
          break;
        case IT_AP:
          printf("AP\n");
          secd_ap(secd);
          break;
        case IT_RET:
          printf("RET\n");
          if (secd->d == LISP_NIL)
            return;
          secd_ret(secd);
          break;
        case IT_POP:
          printf("POP\n");
          secd_pop(secd);
          break;
        case IT_LDS:
          printf("LDS\n");
          i = intval(secd_c(secd));
          x = secd_pop(secd);
          if (is_ref(x))
            x = *lisp_obj_at(deref(x), i);
          secd_push(secd, x);
          break;
        case IT_STS:
          printf("STS\n");
          i = intval(secd_c(secd));
          y = secd_pop(secd);
          x = secd_pop(secd);
          if (is_ref(x))
            *lisp_obj_at(deref(x), i) = y;
          secd_push(secd, x);
          break;
        case IT_BLT:
          printf("BLT\n");
          secd_blt(secd);
          break;
        default:
          printf("???\n");
          break;
        }
    }
}


extern struct secd THE_SECD;

any run_expr(any expr)
{
  THE_SECD.s = LISP_NIL;
  THE_SECD.e = LISP_NIL;
  THE_SECD.c = expr;
  THE_SECD.d = LISP_NIL;
  secd_run(&THE_SECD);
  return secd_pop(&THE_SECD);
}
