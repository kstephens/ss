(define (truncate-quotient n1 n2)  (%fixnum-div n1 n2))
(define (truncate-remainder n1 n2) (%fixnum-mod n1 n2))
(define (abs x) (if (< x 0) (- x) x))

(define %+ +)
(define (+ . vals)
  (%reduce %+ 0 vals))

(define %- -)
(define (- x . vals)
  (if (null? vals)
    (_NEG x)
    (%- x (%reduce %+ 0 vals))))

(define %* *)
(define (* . vals)
  (%reduce %* 1 vals))

(define %/ /)
(define (/ x . vals)
  (if (null? vals)
    (_DIV 1.0 x)
    (%/ x (%reduce %* 1 vals))))

