(define + _ADD) ;; FIXME
(define * _MUL) ;; FIXME
(define < _LT)
(define > _GT)
(define <= _LE)
(define >= _GE)
(define == _EQ)
(define != _NE)

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

(define (pair? x) (eq? (%type x) <pair>))
(C_ss_make_constant 'pair?)

(define (list . l) l)

(define <fixnum> (%type 1))
(C_ss_make_constant '<fixnum>)
(define (fixnum? x) (eq? (%type x) <fixnum>))
(C_ss_make_constant 'fixnum?)

(define <flonum> (%type 1.23))
(C_ss_make_constant '<flonum>)
(define (flonum? x) (eq? (%type x) <flonum>))
(C_ss_make_constant 'flonum?)

(define _DIV2 _DIV)
(C_ss_make_constant '_DIV2)

(define (_DIV x y)
  (if (fixnum? x)
    (if (fixnum? y)
      (if (_EQ (_MOD x y) 0)
        (_DIV2 x y)
        (_DIV2 (C_ss_to_real x) y))
      (_DIV2 x y))
    (_DIV2 x y)))

(define (map proc args)
  (if (null? args)
    '()
    (cons (proc (car args))
       (map proc (cdr args)))))

(write ";; ss - boot.scm loaded")(newline)
