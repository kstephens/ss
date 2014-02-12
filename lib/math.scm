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

