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
(C_ss_make_constant 'number?)

(define <pair> (%type '(a cons)))
(C_ss_make_constant '<pair>)
(define (pair? x) (eq? (%type x) <pair>))
(C_ss_make_constant 'pair?)

(define <string> (%type "string"))
(C_ss_make_constant '<string>)
(define (string? x) (eq? (%type x) <string>))
(C_ss_make_constant 'string?)

(define <symbol> (%type 'symbol))
(C_ss_make_constant '<symbol>)
(define (symbol? x) (eq? (%type x) <symbol>))
(C_ss_make_constant 'symbol?)

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
(define (%append-2 a b)
  (if (null? a) b
    (cons (car a) (%append-2 (cdr a) b))))
(define (%append-3 l lists)
  (if (null? lists) l
    (%append-3 (%append-2 l (car lists)) (cdr lists))))
(define (append l . lists) (%append-3 l lists))

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
(define (close-port port)
  (C_ss_port_close port))

(define (%write-port port str)
  (C_fwrite (C_ss_string_v str) (C_ss_string_l str) (C_ss_unbox_integer 1) (C_ss_car port)))

(define *load-verbose* #f)
(define (load file)
  (let ((port (open-read-file file)))
    (let ((result (C_ss_repl &env port (if *load-verbose* ss_stderr #f) #f #f)))
      (C_ss_port_close port)
      result)))

(define (%define-macro name . form)
  (if (symbol? name)
    (list 'C_ss_make_syntax (list 'quote name) (car form))
    (list 'C_ss_make_syntax (list 'quote (car name)) (list 'lambda (cdr name) (cons 'begin form)))))
(C_ss_make_syntax 'define-macro %define-macro)

(load "lib/cxr.scm")
(load "lib/quasiquote.scm")
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

(write ";; ss - boot.scm loaded")(newline)
