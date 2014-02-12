(define + _ADD) ;; FIXME
(define * _MUL) ;; FIXME
(define < _LT)
(define > _GT)
(define <= _LE)
(define >= _GE)
(define == _EQ)
(define != _NE)
(define = _EQ)

(define eq? C:ss_eqQ)
(C:ss_make_constant 'eq?)

(define %unspec (if #f #t))
(C:ss_make_constant '%unspec)

(define (not x) (if x #f #t))

(define (%type x) (C:ss_i (C:ss_type x)))
(C:ss_make_constant '%type)

(define <null> (%type '()))
(C:ss_make_constant '<null>)
(define (null? x) (eq? x '()))
(C:ss_make_constant 'null?)

(define <boolean> (%type #t))
(C:ss_make_constant '<boolean>)
(define (boolean? x) (eq? (%type x) <boolean>))
(C:ss_make_constant 'boolean?)

(define <fixnum> (%type 1))
(C:ss_make_constant '<fixnum>)
(define (fixnum? x) (eq? (%type x) <fixnum>))
(C:ss_make_constant 'fixnum?)

(define <flonum> (%type 1.23))
(C:ss_make_constant '<flonum>)
(define (flonum? x) (eq? (%type x) <flonum>))
(C:ss_make_constant 'flonum?)

(define integer? fixnum?)
(C:ss_make_constant 'integer?)

(define (real? x)
  (if (fixnum? x) #t (flonum? x)))
(C:ss_make_constant 'number?)

(define number? real?)
(C:ss_make_constant 'real?)

(define <pair> (%type '(a cons)))
(C:ss_make_constant '<pair>)
(define (pair? x) (eq? (%type x) <pair>))
(C:ss_make_constant 'pair?)
(define cons C:ss_cons)
(define car C:ss_car)
(define cdr C:ss_cdr)
(define set-car! C:ss_set_carE)
(define set-cdr! C:ss_set_cdrE)

(define <string> (%type "string"))
(C:ss_make_constant '<string>)
(define (string? x) (eq? (%type x) <string>))
(C:ss_make_constant 'string?)

(define <symbol> (%type 'symbol))
(C:ss_make_constant '<symbol>)
(define (symbol? x) (eq? (%type x) <symbol>))
(C:ss_make_constant 'symbol?)

(define <char> (%type #\a))
(C:ss_make_constant '<char>)
(define (char? x) (eq? (%type x) <char>))
(C:ss_make_constant 'char?)
(define (char->integer c)
  (C:ss_i (C:ss_C c)))
(define (integer->char i)
  (C:ss_c (C:ss_I i)))

(define <vector> (%type '#(1 2)))
(C:ss_make_constant '<vector>)
(define (vector? x) (eq? (%type x) <vector>))
(C:ss_make_constant 'vector?)

(define (string-length a)
  (C:ss_i (C:ss_string_L a)))
(C:ss_make_constant 'string-length)
(define string-ref C:ss_string_R)
(C:ss_make_constant 'string-ref)

(define (vector-length a)
  (C:ss_i (C:ss_vector_L a)))
(C:ss_make_constant 'vector-length)
(define vector-ref C:ss_vector_R)
(C:ss_make_constant 'vector-ref)

(define (%string-equal? a b)
  (let ((i (- (string-length a) 1)) (e? #f))
    (set! e?
      (lambda (i)
        (if (= i 0)
          (equal? (string-ref a 0) (string-ref b 0))
          (if (equal? (string-ref a i) (string-ref b i))
            (e? (- i 1))
            #f))))
    (if (>= i 0) (e? i) #t)))

(define (%vector-equal? a b)
  (let ((i (- (vector-length a) 1)) (e? #f))
    (set! e?
      (lambda (i)
        (if (= i 0)
          (equal? (vector-ref a 0) (vector-ref b 0))
          (if (equal? (vector-ref a i) (vector-ref b i))
            (e? (- i 1))
            #f))))
    (if (>= i 0) (e? i) #t)))

(define (list . l) l)
(C:ss_make_constant 'list)
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
  (C:ss_error &env "" (cons code other)))

(define (%open-file func file mode)
  (let ((port (C:ss_m_port
                (C:fopen (C:ss_string_V file) (C:ss_string_V mode))
                (C:ss_string_V file) (C:ss_string_V mode))))
    (if port port
      (error func "cannot open" file (C:ss_errstr #f)))))

(define (open-read-file file)
  (%open-file 'open-read-file file "r"))
(define (close-port port)
  (C:ss_port_close port))

(define (%write-port port str)
  (C:fwrite (C:ss_string_V str) (C:ss_string_L str) (C:ss_unbox_integer 1) (C:ss_car port)))

(define (read . port)
  (C:ss_read &env (if (null? port) ss_stdout (car port))))
(define (write obj . port)
  (C:ss_write_3 obj (if (null? port) ss_stdout (car port)) 'write))
(define (display obj . port)
  (C:ss_write_3 obj (if (null? port) ss_stdout (car port)) 'display))

(define *load-verbose* #f)
(define (load-file file)
  (let ((port (open-read-file file)))
    (let ((result (C:ss_repl &env port (if *load-verbose* ss_stderr #f) #f #f)))
      (close-port port)
      result)))
(define load load-file)

(define (%define-macro name . form)
  (if (pair? name)
    (list 'C:ss_make_syntax (list 'quote (car name)) (list 'lambda (cdr name) (cons 'begin form)))
    (list 'C:ss_make_syntax (list 'quote name) (car form))))
(C:ss_make_syntax 'define-macro %define-macro)

(load "lib/cxr.scm")
(load "lib/quasiquote.scm")
(define-macro quasiquote %quasiquote)

(define-macro (define-constant name . body)
  (if (pair? name)
    `(define-constant ,(car name) (lambda ,(cdr name) ,@body))
    `(begin
      (define ,name ,@body)
      (C:ss_make_constant ',name))))

(define-constant _DIV2 _DIV)
(define-constant (_DIV x y)
  (if (fixnum? x)
    (if (fixnum? y)
      (if (_EQ (_MOD x y) 0)
        (_DIV2 x y)
        (_DIV2 (C:ss_to_flonum x) y))
      (_DIV2 x y))
    (_DIV2 x y)))

(define (ss_call_macro_char c port)
  (error 'read "invalid read macro char" c))

(define (%gensym x)
  (C:ss_box_symbol (C:ss_I 0)))

(define (%or tmp terms)
  (if (null? terms) #f
    `(begin
       (set! ,tmp ,(car terms))
       (if ,tmp ,tmp ,(%or tmp (cdr terms))))))

(define-macro (letrec bindings . body)
  `(let ,(map (lambda (b) `(,(car b) ',%unspec)) bindings)
     ,@(map (lambda (b) `(set! ,(car b) ,@(cdr b))) bindings)
     ,@body))

(define-macro (or . terms)
  (if (null? terms) #f
    (if (null? (cdr terms)) (car terms)
      (let ((tmp (%gensym 'or)))
        `(let ((,tmp ,(car terms)))
           (if ,tmp ,tmp ,(%or tmp (cdr terms))))))))

(define-macro (and . terms)
  (if (null? terms) #t
    (if (null? (cdr terms)) (car terms)
      `(if ,(car terms) (and ,@(cdr terms)) #f))))

(define-macro (cond . cases)
  (letrec ((%cond 
             (lambda (cases)
               (if (null? cases) `',%unspec
                 (let ((case (car cases)))
                   (if (eq? 'else (car case))
                     `(begin ,@(cdr case))
                     (if (eq? '=> (cadr case))
                       (let ((tmp (%gensym 'cond)))
                         `(let ((,tmp ,(car case)))
                            (if ,tmp (,(caddr case) ,tmp)
                              ,(%cond (cdr cases)))))
                       `(if ,(car case)
                          (begin ,@(cdr case))
                          ,(%cond (cdr cases))))))))))
    (%cond cases)))

(define (equal? a b)
  (cond
    ((eq? a b)
      #t)
    ((and (pair? a) (pair? b))
      (and (equal? (car a) (car b)) (equal? (cdr a) (cdr b))))
    ((and (vector? a) (vector? b) (= (vector-length a) (vector-length b)))
      (%vector-equal? a b))
    ((and (string? a) (string? b) (= (string-length a) (string-length b)))
      (%string-equal? a b))
    (else #f)))

(display ";; ss - boot.scm loaded.")(newline)
