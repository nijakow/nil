(progn

  (defun car (c)
    (asm (ld 0 0)
         (lds 0)))

  (defun cdr (c)
    (asm (ld 0 0)
         (lds 1)))

  (defun rplaca (c v)
    (asm (ld 0 0)
         (ld 0 1)
         (sts 0)))

  (defun rplacd (c v)
    (asm (ld 0 0)
         (ld 0 1)
         (sts 1)))

  (defun cons (a d)
    (asm (blt 0 0)
         (ld 0 0)
         (sts 0)
         (ld 0 1)
         (sts 1)))

  (defun eq (a b)
    (asm (ld 0 0)
         (ld 0 1)
         (blt 1 2)))

  (defun symbol-value (s)
    (asm (ld 0 0)
         (ldc 2)
         (lds)))

  (defun symbol-function (s)
    (asm (ld 0 0)
         (ldc 3)
         (lds)))
  )
