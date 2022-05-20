
(defparameter *xc/symbol/val* 2)
(defparameter *xc/symbol/fval* 3)

(defun xc/write-nil (tail) (cons 0 tail))
(defun xc/write-ldc (e tail) (cons 1 (cons e tail)))
(defun xc/write-ld (x tail) (cons 2 (cons x tail)))
(defun xc/write-st (x tail) (cons 3 (cons x tail)))
(defun xc/write-sel (tail1 tail2) (cons 4 (cons tail1 (cons tail2 nil))))
(defun xc/write-join (tail) (cons 5 (cons tail nil)))
(defun xc/write-ldf (f tail) (cons 6 (cons f tail)))
(defun xc/write-ap (a tail) (cons 7 (cons a tail)))
(defun xc/write-ret () (cons 8 nil))
(defun xc/write-pop (tail) (cons 9 tail))
(defun xc/write-lds (i tail) (cons 10 (cons i tail)))
(defun xc/write-sts (i tail) (cons 11 (cons i tail)))
(defun xc/write-ldg (sym tail) (xc/write-ldc sym
                                             (xc/write-lds *xc/symbol/val* tail)))
(defun xc/write-stg (tail) (xc/write-sts *xc/symbol/val* tail))
(defun xc/write-ldgf (sym tail) (xc/write-ldc sym
                                              (xc/write-lds *xc/symbol/fval* tail)))
(defun xc/write-stgf (tail) (xc/write-sts *xc/symbol/fval* tail))
(defun xc/write-blt (num args tail) (cons 12 (cons num (cons args tail))))

(defun xc/find (e lst i)
  (cond ((null lst) nil)
        ((eq (car lst) e) i)
        (t (xc/find e (cdr lst) (+ i 1)))))

(defun xc/lookup (expr env depth)
  (if (null env)
      nil
    (let ((v (xc/find expr (car env) 0)))
      (if v (cons depth v)
        (xc/lookup expr (cdr env) (+ depth 1))))))

(defun xc/asm-add-label (name code label-list)
  (cons (cons name code) label-list))

(defun xc/make-jump-db ()
  (cons nil nil))

(defun xc/get-entry (db key func)
  (let ((handle (assoc key (car db))))
    (if handle (funcall func (cdr handle))
      (let ((handle2 (assoc key (cdr db))))
        (if handle2
            (rplacd handle2 (cons func (cdr handle2)))
          (rplacd db (cons (cons key (cons func nil)) (cdr db))))))))

(defun xc/set-entry (db key val)
  (rplaca db (cons (cons key val) (cdr db)))
  (let ((handle (assoc key (cdr db))))
    (mapcar (lambda (e) (funcall e val)) (cdr handle))))

(defun xc/asm-main (code db env tail)
  (cond ((null code) tail)
        ((symbolp (car code))
         (let ((r (xc/asm-main (cdr code) db env tail)))
           (xc/set-entry db (car code) r)
           r))
        (t (let ((opcode (caar code))
                 (args (cdar code))
                 (r (xc/asm-main (cdr code) db env tail)))
             (setq tail r)
             (cond ((eq opcode 'nil) (xc/write-nil tail))
                   ((eq opcode 'ldc) (xc/write-ldc (car args) tail))
                   ((eq opcode 'ld) (xc/write-ld (cons (car args) (cadr args)) tail))
                   ((eq opcode 'sel)
                    (let ((r (xc/write-sel '? '?)))
                      (xc/get-entry db (car args) (lambda (v) (rplaca (cdr r) v)))
                      (xc/get-entry db (cadr args) (lambda (v) (rplaca (cddr r) v)))
                      r))
                   ((eq opcode 'join)
                    (let ((r (xc/write-join '?)))
                      (xc/get-entry db (car args) (lambda (v) (rplaca (cdr r) v)))
                      r))
                   ((eq opcode 'ldf) (xc/write-ldf '? tail))
                   ((eq opcode 'ap) (xc/write-ap (car args) tail))
                   ((eq opcode 'ret) (xc/write-ret))
                   ((eq opcode 'pop) (xc/write-pop tail))
                   ((eq opcode 'st) (xc/write-st (car args) tail))
                   ((eq opcode 'lds) (xc/write-lds (car args) tail))
                   ((eq opcode 'sts) (xc/write-sts (car args) tail))
                   ((eq opcode 'blt) (xc/write-blt (car args) (cadr args) tail))
                   ((eq opcode 'lisp) (xc/compile (cons 'progn args) env db tail))
                   (t (error "Unknown opcode!")))))))

(defun xc/asm (code env db tail)
  (xc/asm-main code (xc/make-jump-db) env tail))

(defun xc/compile-tagbody-main (body env db tail)
  (cond ((null body) (cons tail t))
        ((symbolp (car body))
         (let ((r (xc/compile-tagbody-main (cdr body) env db tail)))
           (xc/set-entry db (car body) (car r))
           r))
        (t (let ((r (xc/compile-tagbody-main (cdr body) env db tail)))
             (cons (xc/compile (car body)
                               env
                               db
                               (if (cdr r)
                                   (car r)
                                 (xc/write-pop (car r))))
                   nil)))))

(defun xc/compile-tagbody (body env db tail)
  (car (xc/compile-tagbody-main body env db tail)))

(defun xc/compile (expr env db tail)
  (cond ((eq expr nil)
         (xc/write-nil tail))
        ((symbolp expr)
         (let ((v (xc/lookup expr env 0)))
           (if v
               (xc/write-ld v tail)
             (xc/write-ldg expr tail))))
        ((consp expr)
         (cond ((eq (car expr) 'quote)
                (xc/write-ldc (cadr expr) tail))
               ((eq (car expr) 'function)
                (if (symbolp (cadr expr))
                    (xc/write-ldgf (cadr expr) tail)
                  (xc/compile (cadr expr) env db tail)))
               ((eq (car expr) 'setq)
                (let ((v (xc/lookup (cadr expr) env 0)))
                  (if v
                      (xc/compile (caddr expr)
                                  env
                                  db
                                  (xc/write-st v tail))
                    (xc/write-ldc (cadr expr)
                                  (xc/compile (caddr expr)
                                              env
                                              db
                                              (xc/write-stg tail))))))
               ((eq (car expr) 'progn)
                (let ((rev (reverse (cdr expr))))
                  (setq tail (xc/compile (car rev) env db tail))
                  (loop for e in (cdr rev) do
                        (setq tail (xc/compile e env db (xc/write-pop tail))))
                  tail))
               ((eq (car expr) 'if)
                (xc/compile (cadr expr)
                            env
                            db
                            (xc/write-sel (xc/compile (caddr expr) env db (xc/write-join tail))
                                          (if (cdddr expr)
                                              (xc/compile (cadddr expr) env db (xc/write-join tail))
                                            (xc/write-nil (xc/write-join tail))))))
               ((eq (car expr) 'cond)
                (if (null (cdr expr))
                    (xc/write-nil tail)
                  (xc/compile (list 'if
                                    (caadr expr)
                                    (cons 'progn (cdadr expr))
                                    (cons 'cond (cddr expr)))
                              env
                              db
                              tail)))
               ((eq (car expr) 'go)
                (let ((r (xc/write-join '?)))
                  (xc/get-entry db (cadr expr) (lambda (v) (rplaca (cdr r) v)))
                  r))
               ((eq (car expr) 'tagbody)
                (xc/compile-tagbody (cdr expr) env db tail))
               ((eq (car expr) 'lambda)
                (xc/write-ldf (xc/compile (cons 'progn (cddr expr)) (cons (cadr expr) env) (xc/make-jump-db) (xc/write-ret)) tail))
               ((eq (car expr) 'defun)
                (xc/write-ldc (cadr expr)
                              (xc/compile (cons 'lambda (cddr expr))
                                          env
                                          db
                                          (xc/write-stgf tail))))
               ((eq (car expr) 'let)
                (xc/compile (cons (cons 'lambda (cons (mapcar #'car (cadr expr)) (cddr expr)))
                                  (mapcar (lambda (e) (cons 'progn (cdr e))) (cadr expr)))
                            env
                            db
                            tail))
               ((eq (car expr) 'asm)
                (xc/asm (cdr expr) env db tail))
               (t (setq tail (xc/write-ap (length (cdr expr)) tail))
                  (loop for e in (reverse (cdr expr)) do
                        (setq tail (xc/compile e env db tail)))
                  (xc/compile (list 'function (car expr)) env db tail))))
        (t (xc/write-ldc expr tail))))

(setq *print-circle* t)

(defun comp (expr)
  (xc/compile expr '() (xc/make-jump-db) (xc/write-ret)))

(pprint (comp (read)))
(terpri)
(exit)

