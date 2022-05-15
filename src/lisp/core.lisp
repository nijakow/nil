(progn

  (defun eq (a b)
    (asm (ld 0 0)
         (ld 0 1)
         (blt 0)))

  (defun car (c)
    (asm (ld 0 0)
         (ldc 0)
         (lds)))

  (defun cdr (c)
    (asm (ld 0 0)
         (ldc 1)
         (lds)))

  (defun symbol-value (s)
    (asm (ld 0 0)
         (ldc 2)
         (lds)))

  (defun symbol-function (s)
    (asm (ld 0 0)
         (ldc 3)
         (lds)))
  )
