(define (error code . other)
  (C:ss_error &env (symbol->string code) other C:%NULL))

(define (apply func args) ;; FIXME
  (C:ss_applyv &env func args))

;; ss_ADD has arity checks, while C:ss_ADD does not.
(define + ss_ADD)
(define - ss_SUB)
(define * ss_MUL)
(define (/ a b) (ss_DIV a b))
(define %neg ss_NEG)
(define < ss_LT)
(define > ss_GT)
(define <= ss_LE)
(define >= ss_GE)
(define == ss_EQ)
(define != ss_NE)
(define = ss_EQ)

(define eq? C:ss_eqQ)
(C:ss_make_constant 'eq?)

(define %unspec (if #f #t))
(C:ss_make_constant '%unspec)

(define (not x) (if x #f #t))

(define (%type x) (C:ss_type x))
(C:ss_make_constant '%type)
(define %<type> (%type (%type #t)))
(C:ss_make_constant '%<type>)

(define %<null> (%type '()))
(C:ss_make_constant '%<null>)
(define (null? x) (eq? x '()))
(C:ss_make_constant 'null?)

(define %<boolean> (%type #t))
(C:ss_make_constant '%<boolean>)
(define (boolean? x) (eq? (%type x) %<boolean>))
(C:ss_make_constant 'boolean?)

(define %<fixnum> (%type 1))
(C:ss_make_constant '%<fixnum>)
(define (fixnum? x) (eq? (%type x) %<fixnum>))
(C:ss_make_constant 'fixnum?)

(define %<flonum> (%type 1.23))
(C:ss_make_constant '%<flonum>)
(define (flonum? x) (eq? (%type x) %<flonum>))
(C:ss_make_constant 'flonum?)

(define integer? fixnum?)
(C:ss_make_constant 'integer?)

(define rational? integer?)
(C:ss_make_constant 'rational?)

(define (real? x)
  (if (fixnum? x) #t (flonum? x)))
(C:ss_make_constant 'real?)

(define number? real?)
(C:ss_make_constant 'number?)

(define %<pair> (%type '(a cons)))
(C:ss_make_constant '%<pair>)
(define (pair? x) (eq? (%type x) %<pair>))
(C:ss_make_constant 'pair?)
(define cons C:ss_cons)
(define car C:ss_car)
(define cdr C:ss_cdr)
(define set-car! C:ss_set_carE)
(define set-cdr! C:ss_set_cdrE)

(define %<string> (%type "string"))
(C:ss_make_constant '%<string>)
(define (string? x) (eq? (%type x) %<string>))
(C:ss_make_constant 'string?)
(define symbol->string C:%struct-ss_s_symbol.name)

(define %<symbol> (%type 'symbol))
(C:ss_make_constant '%<symbol>)
(define (symbol? x) (eq? (%type x) %<symbol>))
(C:ss_make_constant 'symbol?)

(define %<char> (%type #\a))
(C:ss_make_constant '%<char>)
(define (char? x) (eq? (%type x) %<char>))
(C:ss_make_constant 'char?)
(define (char->integer c)
  (C:ss_C c))
(define (integer->char i)
  (C:ss_c i))

(define %eos (integer->char -1))
(C:ss_make_constant '%eos)
(define %<eos> (%type %eos))
(C:ss_make_constant '%<eos>)
(define (eof-object? x) (eq? x %eos))
(C:ss_make_constant 'eof-object?)

(define %<vector> (%type '#(1 2)))
(C:ss_make_constant '%<vector>)
(define (vector? x) (eq? (%type x) %<vector>))
(C:ss_make_constant 'vector?)

(define (string-length a)
  (C:ss_i (C:ss_string_L a)))
(C:ss_make_constant 'string-length)
(define string-ref C:ss_string_R)
(C:ss_make_constant 'string-ref)
(define string-set! C:ss_string_S)
(C:ss_make_constant 'string-set!)

(define (vector-length a)
  (C:ss_vector_L a))
(C:ss_make_constant 'vector-length)
(define vector-ref C:ss_vector_R)
(C:ss_make_constant 'vector-ref)
(define vector-set! C:ss_vector_S)
(C:ss_make_constant 'vector-set!)

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
(define (vector-copy v)
  (C:ss_vecnv (C:ss_vector_L v) (C:ss_vector_V v)))

(define (list . l) l)
(C:ss_make_constant 'list)
(define (list-length l)
  (C:ss_i (C:ss_list_length l)))
(define length list-length)
(define (list-pair-ref l k)
  (if (null? l)
    (error 'list-pair-ref "range")
    (if (= k 0) l
      (list-pair-ref (cdr l) (- k 1)))))
(define (list-ref l k)
  (car (list-pair-ref l k)))
(define (list-set! l k v)
  (set-car! (list-pair-ref l k) v))
(define (reverse l) (%list-reverse-2 l '()))
(define (%list-reverse-2 l e)
  (if (null? l) e
    (%list-reverse-2 (cdr l) (cons (car l) e))))
(define (list->vector l)
  (C:ss_list_to_vector l))
(define (vector . l) (list->vector l))
(define (%vector-clear v i value)
  (if (< i (vector-length v))
    (begin
      (vector-set! v i value)
      (%vector-clear v (+ i 1) value))
    v))
(define (make-vector size . value)
  (let ((v (C:ss_vecn size)))
    (if (null? value)
      v
      (%vector-clear v 0 (car value)))))

(define (vector->list v)
  (let ( (v->l! #f)
         (l (list #f)))
    (set! v->l!
      (lambda (l i)
        (if (< i (vector-length v))
          (begin
            (set-cdr! l (cons (vector-ref v i) '()))
            (v->l! (cdr l) (+ i 1))))))
    (v->l! l 0)
    (cdr l)))

(define (vector->list! v l)
  (let ((v->l! #f))
    (set! v->l!
      (lambda (l i)
        (if (pair? l)
          (begin
            (set-car! l (vector-ref v i))
            (v->l! (cdr l) (+ i 1))))))
    (v->l! l 0))
  l)

(define (%map-1 f l)
  (if (null? l) l
    (cons (f (car l)) (%map-1 f (cdr l)))))
(define (%map f lists)
  (if (null? (car lists)) '()
    (cons
      (apply f (%map-1 car lists))
      (%map f (%map-1 cdr lists)))))
(define (map f . lists) (%map f lists))
(define (%for-each f lists)
  (if (null? (car lists)) %unspec
    (begin
      (apply f (%map-1 car lists))
      (%for-each f (%map-1 cdr lists)))))
(define (for-each f . lists) (%for-each f lists))

(define (%reduce f a l)
  (if (null? l) a
    (f (car l) (%reduce f a (cdr l)))))

(define (%append-2 a b)
  (if (null? a) b
    (cons (car a) (%append-2 (cdr a) b))))
(define (%append-3 l lists)
  (if (null? lists) l
    (%append-3 (%append-2 l (car lists)) (cdr lists))))
(define (append l . lists) (%append-3 l lists))

(define (assp f l)
  (if (null? l) #f
    (if (f (car (car l)))
      (car l)
      (assp f (cdr l)))))
(define (assq x l)  (assp (lambda (y) (eq? x y))    l))
(define (assv x l)  (assp (lambda (y) (eqv? x y))   l))
(define (assoc x l) (assp (lambda (y) (equal? x y)) l))
(define (memp f l)
  (if (null? l) #f
    (if (f (car l)) #t
      (memp f (cdr l)))))
(define (memq x l)   (memp (lambda (y) (eq? x y))    l))
(define (memv x l)   (memp (lambda (y) (eqv? x y))   l))
(define (member x l) (memp (lambda (y) (equal? x y)) l))

(define (%open-file func file mode)
  (let ((port (C:ss_m_port
                (C:%fopen (C:%ss_S file) (C:%ss_S mode))
                file mode)))
    (if port port
      (error func "cannot open" file (C:ss_errstr #f)))))

(define (open-input-file file)
  (%open-file 'open-input-file file "r"))
(define (close-port port)
  (C:ss_port_close port))

(define (%write-port port str)
  (C:fwrite str (string-length str) 1 port))

(define (write obj . port)
  (C:ss_write_3 obj (if (null? port) ss_stdout (car port)) 'write))
(define (display obj . port)
  (C:ss_write_3 obj (if (null? port) ss_stdout (car port)) 'display))

(define *load-verbose* #f)
(define (load-file file)
  (let ((port (open-input-file file)))
    (let ((result (C:ss_repl_run (C:ss_m_repl &env port (if *load-verbose* ss_stderr #f)))))
      (close-port port)
      result)))
(define load load-file)

(define (%define-macro name . form)
  (if (pair? name)
    (list 'C:ss_make_syntax (list 'quote (car name)) (list 'lambda (cdr name) (cons 'begin form)))
    (list 'C:ss_make_syntax (list 'quote name) (car form))))
(C:ss_make_syntax 'define-macro %define-macro)

(define (%string-truncate! str len)
  (C:%ss_set str 1 (C:%ss_I len)))
(define (%string-to-number str radix)
  (C:%ss_string_TO_number str (C:%ss_I radix)))
(load "lib/string.scm")

(load "lib/cxr.scm")
(load "lib/quasiquote.scm")
(define-macro quasiquote %quasiquote)
(load "lib/string.scm")
(load "lib/sort.scm")

;;(C:ss_set_eval_verbose 10)
(define-macro (define-constant name . body)
  (if (pair? name)
    `(define-constant ,(car name) (lambda ,(cdr name) ,@body))
    `(begin
      (define ,name ,@body)
      (C:ss_make_constant ',name))))

(define-constant ss_DIV2 ss_DIV)
(define-constant %fixnum-div ss_DIV2)
(define-constant %fixnum-mod ss_MOD)
(define-constant (ss_DIV x y)
  (if (fixnum? x)
    (if (fixnum? y)
      (if (ss_EQ (ss_MOD x y) 0)
        (ss_DIV2 x y)
        (ss_DIV2 (C:ss_to_flonum x) y))
      (ss_DIV2 x y))
    (ss_DIV2 x y)))
(load "lib/math.scm")

(define (ss_call_macro_char c port)
  (error 'read "invalid read macro char" c))

(define (%gensym x)
  (C:ss_box_symbol #f))

(define-macro (let* bindings . body)
  (if (null? bindings) `(begin ,@body)
    `(let ((,(caar bindings) ,@(cdar bindings)))
       (let* ,(cdr bindings) ,@body))))

(define-macro (letrec bindings . body)
  `(let ,(map (lambda (b) `(,(car b) ',%unspec)) bindings)
     ,@(map (lambda (b) `(set! ,(car b) ,@(cdr b))) bindings)
     ,@body))

(define-macro (or . terms)
  (if (null? terms) #f
    (if (null? (cdr terms)) (car terms)
      (letrec ( (tmp (%gensym 'or))
                (%or (lambda (terms)
                       (if (null? terms) #f
                         `(begin
                            (set! ,tmp ,(car terms))
                            (if ,tmp ,tmp ,(%or (cdr terms))))))) )
        `(let ((,tmp ,(car terms)))
           (if ,tmp ,tmp ,(%or (cdr terms))))))))

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

(define-constant (%catch body rescue ensure)
  (C:%ss_catch &env body rescue ensure))
(define-constant (%make-throwable value)
  (C:%ss_m_throwable value))
(define-constant (%throw catch value)
  (C:%ss_throw_ &env catch (%make-throwable value)))
(define-constant (%rethrow)
  (C:%ss_rethrow &env))
(define-macro (catch name body rescue . ensure)
  `(%catch
     (lambda (,name) ,body)
     ,(if (not rescue)   #f `(lambda (,name ,(car rescue)) ,@(cdr rescue)))
     ,(if (null? ensure) #f `(lambda (,name) ,(car ensure)))))
(define-constant throw %throw)
(define-constant rethrow %rethrow)

(define-constant C:%NULL (C:%ss_I 0))

(load "lib/each.scm")

(display "  ;; ss: boot.scm loaded." ss_stderr)(newline ss_stderr)

;; (load "t/test.scm")
