(define eq? C_ss_eqQ)
(C_ss_make_constant 'eq?)

(define (%type x) (C_ss_i (C_ss_type x)))
(C_ss_make_constant '%type)

(define <null> (%type '()))
(C_ss_make_constant '<null>)

(define (null? x) (eq? x '()))
(C_ss_make_constant 'null?)

(define <pair> (%type '(a cons)))
(C_ss_make_constant '<pair>)

(define (pair? x) (eq? %type(x) <pair>))
(C_ss_make_constant 'pair?)

(define (list . l) l)

(define (map proc args)
  (if (null? args)
    '()
    (cons (proc (car args))
       (map proc (cdr args)))))

(define + _ADD) ;; FIXME
(define * _MUL) ;; FIXME


(define g (lambda (x) (if (> x 0) (f (- x 1)) 'g)))
(define f (lambda (x) (if (> x 0) (g (- x 1)) 'f)))
(g 10000)

;;;;

;; #|

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

#|
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
|#

;;|#
