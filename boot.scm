(define + _ADD) ;; FIXME
(define * _MUL) ;; FIXME
(define < _LT)
(define > _GT)
(define <= _LE)
(define >= _GE)
(define == _EQ)
(define != _NE)
(define = _EQ)

(define eq? C_ss_eqQ)
(C_ss_make_constant 'eq?)

(define (not x) (if x #f #t))

(define (%type x) (C_ss_i (C_ss_type x)))
(C_ss_make_constant '%type)

(define <null> (%type '()))
(C_ss_make_constant '<null>)
(define (null? x) (eq? x '()))
(C_ss_make_constant 'null?)

(define <fixnum> (%type 1))
(C_ss_make_constant '<fixnum>)
(define (fixnum? x) (eq? (%type x) <fixnum>))
(C_ss_make_constant 'fixnum?)

(define <flonum> (%type 1.23))
(C_ss_make_constant '<flonum>)
(define (flonum? x) (eq? (%type x) <flonum>))
(C_ss_make_constant 'flonum?)

(define (number? x)
  (if (fixnum? x) #t (flonum? x)))

(define <pair> (%type '(a cons)))
(C_ss_make_constant '<pair>)
(define (pair? x) (eq? (%type x) <pair>))
(C_ss_make_constant 'pair?)

(define <string> (%type "string"))
(C_ss_make_constant '<string>)
(define (string? x) (eq? (%type x) <string>))
(C_ss_make_constant 'string?)

(define <char> (%type #\a))
(C_ss_make_constant '<char>)
(define (char? x) (eq? (%type x) <char>))
(C_ss_make_constant 'char?)

(define <vector> (%type '#(1 2)))
(C_ss_make_constant '<vector>)
(define (vector? x) (eq? (%type x) <vector>))
(C_ss_make_constant 'vector?)

(define (list . l) l)
(C_ss_make_constant 'list)
(define (caar x) (car (car x)))
(define (cadr x) (car (cdr x)))
(define (cadar x) (car (cdr (car x))))
(define (%append-2 a b)
  (if (null? a) b
    (cons (car a) (%append-2 (cdr a) b))))
(define (%append-3 l lists)
  (if (null? lists) l
    (%append-3 (%append-2 l (car lists)) (cdr lists))))
(define (append l . lists) (%append-3 l lists))

(C_ss_make_syntax 'define-macro
  (lambda (name . body)
    (if (pair? name)
      (cons 'define-macro (cons (car name) (cons (cons 'lambda (cons (cdr name) body)) '())))
      (list 'C_ss_make_syntax (list 'quote name) (car body)))))

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
(define-macro quasiquote %quasiquote)

(define-macro (define-constant name . body)
  (if (pair? name)
    `(define-constant ,(car name) (lambda ,(cdr name) ,@body))
    `(begin
      (define ,name ,@body)
      (C_ss_make_constant ',name))))

(define-constant _DIV2 _DIV)
(define-constant (_DIV x y)
  (if (fixnum? x)
    (if (fixnum? y)
      (if (_EQ (_MOD x y) 0)
        (_DIV2 x y)
        (_DIV2 (C_ss_to_real x) y))
      (_DIV2 x y))
    (_DIV2 x y)))

(define (map proc args)
  (if (null? args)
    '()
    (cons (proc (car args))
       (map proc (cdr args)))))

(define (error code . other)
  (C_ss_error &env "" (cons code other)))

(define (%open-file func file mode)
  (let ((port (C_ss_m_port
                (C_fopen (C_ss_string_v file) (C_ss_string_v mode))
                (C_ss_string_v file) (C_ss_string_v mode))))
    (if port port
      (error func "cannot open" file (C_ss_errstr #f)))))

(define (open-read-file file)
  (%open-file 'open-read-file file "r"))

(define (%write-port port str)
  (C_fwrite (C_ss_string_v str) (C_ss_string_l str) (C_ss_unbox_integer 1) (C_ss_car port)))

(define (load file)
  (let ((port (open-read-file file)))
    (let ((result (C_ss_repl &env port ss_stderr #f)))
      (C_ss_port_close port)
      result)))

(write ";; ss - boot.scm loaded")(newline)
