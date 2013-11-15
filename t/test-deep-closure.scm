(define ((constant value) . rest) value)

(define (((sequence op) start value) . rest)
  (let ((result start))
    (set! start (op start (value)))
    result))
(define arithmetic (sequence +))
(define geometric (sequence *))
(map (arithmetic 1 (constant 3)) '(1 2 3 4))
  ;;  => (1 4 7 10)
(map (geometric 1 (constant 2)) '(1 2 3 4))
  ;; => (1 2 4 8)

(define g (sequence _MUL))
(define a (sequence _ADD))
(define as (a 1 3))
(define gs (g 1 5))
(gs)
(gs)
(gs)
gs
(as)
(as)
(as)
as
(define my_op as)
(my_op)

