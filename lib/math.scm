(define (truncate-quotient n1 n2)  (%fixnum-div n1 n2))
(define (truncate-remainder n1 n2) (%fixnum-mod n1 n2))
(define (abs x) (if (< x 0) (- x) x))

(define-constant %+ +)
(define-constant (+ . vals)
  (%reduce %+ 0 vals))

(define-constant %- -)
(define-constant (- x . vals)
  (if (null? vals)
    (%neg x)
    (%- x (%reduce %+ 0 vals))))

(define-constant %* *)
(define-constant (* . vals)
  (%reduce %* 1 vals))

(define-constant %/ /)
(define-constant (/ x . vals)
  (if (null? vals)
    (%/ 1.0 x) ;; rational
    (%/ x (%reduce %* 1 vals))))

(define-constant (%all-pairs? f x l)
  (if (null? l) #t
    (if (f x (car l))
      (%all-pairs? f (car l) (cdr l))
      #f)))

(define %= =)
(define-constant (= x . vals)
  (%all-pairs? %= x vals))

(define %< <)
(define-constant (< x . vals)
  (%all-pairs? %< x vals))

(define %> >)
(define-constant (> x . vals)
  (%all-pairs? %> x vals))

(define %<= <=)
(define-constant (<= x . vals)
  (%all-pairs? %<= x vals))

(define %>= >=)
(define-constant (>= x . vals)
  (%all-pairs? %>= x vals))

(define (trunc x)
  (if (integer? x)
    x
    (C:ss_i (C:ss_fixnum_ x))))

(define (floor x)
  (if (integer? x)
    x
    (C:ss_i (C:ss_fixnum_ (C:floor x)))))

(define (expr x y)
  ;; if y is a non-negative integer result is integer.
  (C:pow x y))