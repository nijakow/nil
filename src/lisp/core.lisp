(progn
  (setq t 't)

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
    (asm (ld 0 0)
         (ld 0 1)
         (blt 0 2)))

  (defun eq (a b)
    (asm (ld 0 0)
         (ld 0 1)
         (blt 1 2)))

  (defun null (e)
    (eq e '()))

  (defun not (e)
    (if e nil t))

  (defun numberp (e)
    (asm (ld 0 0)
         (blt 7 1)))

  (defun consp (e)
    (asm (ld 0 0)
         (blt 8 1)))

  (defun symbolp (e)
    (asm (ld 0 0)
         (blt 9 1)))

  (defun symbol-name (s)
    (asm (ld 0 0)
         (lds 1)))

  (defun symbol-value (s)
    (asm (ld 0 0)
         (lds 2)))

  (defun symbol-function (s)
    (asm (ld 0 0)
         (lds 3)))

  (defun = (a b)
    (eq a b))

  (defun < (a b)
    (asm (ld 0 0)
         (ld 0 1)
         (blt 3 2)))

  (defun + (a b)
    (asm (ld 0 0)
         (ld 0 1)
         (blt 2 2)))

  (defun - (a b)
    (asm (ld 0 0)
         (ld 0 1)
         (blt 5 2)))

  (defun * (a b)
    (asm (ld 0 0)
         (ld 0 1)
         (blt 12 2)))

  (defun / (a b)
    (asm (ld 0 0)
         (ld 0 1)
         (blt 13 2)))

  (defun mod (a b)
    (asm (ld 0 0)
         (ld 0 1)
         (blt 14 2)))

  (defun $load-slot (obj off)
    (asm (ld 0 0)
         (ld 0 1)
         (blt 10 2)))

  (defun $store-slot (obj off value)
    (asm (ld 0 0)
         (ld 0 1)
         (ld 0 2)
         (blt 11 3)))

  (defun $load-byte (sym off)
    (asm (ld 0 0)
         (ld 0 1)
         (blt 6 2)))

  (defun putchar (c)
    (asm (ld 0 0)
         (blt 4 1)))

  (defun $print-string (s i)
    (let ((c ($load-byte s i)))
      (if (= c 0) 'nil
        (progn (putchar c)
               ($print-string s (+ i 1))))))

  (defun $print-number-loop (n)
    (cond ((= n 0) nil)
          (t ($print-number-loop (/ n 10))
             (putchar (+ 48 (mod n 10))))))

  (defun $print-number (n)
    (cond ((< n 0) (putchar 45) ($print-number (- 0 n)))
          ((= n 0) (putchar 48))
          (t ($print-number-loop n))))

  (defun $print-list (e)
    (prin1 (car e))
    (cond ((consp (cdr e))
           (putchar 32)
           ($print-list (cdr e)))
          ((not (null (cdr e)))
           (putchar 32)
           (putchar 46)
           (putchar 32)
           (prin1 (cdr e)))))

  (defun prin1 (e)
    (cond ((symbolp e) ($print-string (symbol-name e) 0))
          ((null e)
           (putchar 40)
           (putchar 41))
          ((consp e)
           (putchar 40)
           ($print-list e)
           (putchar 41))
          ((numberp e)
           ($print-number e))
          (t (putchar 63))))

  (prin1 '(a (42 c) d e . f)))
