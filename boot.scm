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

(define (map proc args)
  (if (null? args)
    '()
    (cons (proc (car args))
       (map proc (cdr args)))))

(define + _ADD) ;; FIXME
(define * _MUL) ;; FIXME

;;;;

;; #|

(define (((sequence op) start constant) . rest)
  (let ((result start))
    (set! start (op start constant))
    result))
(define arithmetic (sequence +))
(define geometric (sequence *))
(map (arithmetic 1 3) '(1 2 3 4))
  ;;  => (1 4 7 10)
(map (geometric 1 2) '(1 2 3 4))
  ;; => (1 2 4 8)
