(load "ss/each")

(define eq?-hash C:ss_eqQ_hash)
(define eqv?-hash C:ss_eqvQ_hash)

(define (equal?-hash x)
  (cond
    ((string? x)
      (let ((h 0))
        (string-each (lambda (e) (set! h (C:ss_hash_mix h (equal?-hash e)))) x)
        h))
    ((vector? x)
      (let ((h 0))
        (vector-each (lambda (e) (set! h (C:ss_hash_mix h (equal?-hash e)))) x)
        h))
    ((pair? x)
      (C:ss_hash_mix (equal?-hash (car x)) (equal?-hash (cdr x))))
    ((char? x)
      (C:ss_hash_mix 123 (eq?-hash (char->integer x))))
    (else (eqv?-hash x))))


