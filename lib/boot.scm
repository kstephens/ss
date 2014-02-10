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

(define %unspec (if #f #t))
(C_ss_make_constant '%unspec)

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

(define integer? fixnum?)
(C_ss_make_constant 'integer?)

(define (real? x)
  (if (fixnum? x) #t (flonum? x)))
(C_ss_make_constant 'number?)

(define number? real?)
(C_ss_make_constant 'real?)

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

(define (string-length a)
  (C_ss_i (C_ss_string_l a)))
(C_ss_make_constant 'string-length)
(define string-ref C_ss_string_ref)
(C_ss_make_constant 'string-ref)

(define (vector-length a)
  (C_ss_i (C_ss_vector_l a)))
(C_ss_make_constant 'vector-length)
(define vector-ref C_ss_vector_ref)
(C_ss_make_constant 'vector-ref)

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
      (close-port port)
      result)))

(define (%define-macro name . form)
  (if (pair? name)
    (list 'C_ss_make_syntax (list 'quote (car name)) (list 'lambda (cdr name) (cons 'begin form)))
    (list 'C_ss_make_syntax (list 'quote name) (car form))))
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
        (_DIV2 (C_ss_to_flonum x) y))
      (_DIV2 x y))
    (_DIV2 x y)))

(define (ss_call_macro_char c port)
  (error 'read "invalid read macro char" c))

(define (%gensym x)
  (C_ss_box_symbol (C_ss_I 0)))

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
