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
(define as (a 1 (constant 3)))
(define gs (g 1 (constant 3)))
;; (C_set_exec_verbose 10)
(test eq? (gs) 1)
(test eq? (gs) 3)
(test eq? (gs) 9)

(test eq? (as) 1)
(test eq? (as) 4)
(test eq? (as) 7)

(define my_op as)
(test eq? (my_op) 10)

'ok
