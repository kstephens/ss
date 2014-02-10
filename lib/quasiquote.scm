(define *quasiquote-debug* #f)
(define (%qq o l)
  (if (pair? o)
    (let ((l1 (if (eq? (car o) 'quasiquote)           (+ l 1)
                (if (eq? (car o) 'unquote)            (- l 1)
                  (if (eq? (car o) 'unquote-splicing) (- l 1)
                    l)))))
      (if (= l 0)
        (if (eq? (car o) 'unquote)
          (cadr o)
          (if (pair? (car o))
            (if (eq? (caar o) 'unquote-splicing)
              (list 'append (cadar o)        (%qq (cdr o) l ))
              (list 'cons   (%qq (car o) l1) (%qq (cdr o) l1)))
            (list   'cons   (%qq (car o) l1) (%qq (cdr o) l1))))
        (list       'cons   (%qq (car o) l1) (%qq (cdr o) l1))))
    (if (vector? o)
      (list 'list->vector (%qq (vector->list o) l))
      (if (number? o)    o
        (if (string? o)  o
          (if (char? o)  o
            (list 'quote o)))))))
(define (%quasiquote expr)
  (if *quasiquote-debug*
    (begin (display "  (quasiquote ")(write expr)(display ")")(newline)))
  (let ((result (%qq expr 0)))
    (if *quasiquote-debug*
      (begin (display "    => ")(write result)(newline)))
    result))
