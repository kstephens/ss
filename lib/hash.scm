(load "ss/each")

(define eq?-hash C:ss_eqQ_hash)
(define eqv?-hash C:ss_eqvQ_hash)

(define (string-hash x)
  (let ((h 0))
    (string-each (lambda (e) (set! h (C:ss_hash_mix h (equal?-hash e)))) x)
    h))
(define (vector-hash x)
  (let ((h 0))
    (vector-each (lambda (e) (set! h (C:ss_hash_mix h (equal?-hash e)))) x)
    h))
(define (pair-hash x)
  (C:ss_hash_mix (equal?-hash (car x)) (equal?-hash (cdr x))))

(define (equal?-hash x)
  (cond
    ((string? x) (string-hash x))
    ((vector? x) (vector-hash x))
    ((pair? x)   (pair-hash x))
    ((char? x)
      (C:ss_hash_mix 123 (eq?-hash (char->integer x))))
    (else (eqv?-hash x))))


