(define (test-error . args)
  (display "test-error: ")
  (write args)
  (newline)
  (C_abort))

(define (test cmp a b)
  (if (cmp a b) a
    (test-error 'expected: b 'given: a)))

(define (epsilon? e)
  (lambda (a b)
    (if (_LT (- b e) a)
      (_LT a (+ b e))
      #f)))

(define (test-file name)
  (display ";; testing ")(display name)(display "...")(newline)
  (test eq? (load name) 'ok)
  (display ";; testing ")(display name)(display "... ok.")(newline)
  )

(test-file "t/test-math.scm")
(test-file "t/test-deep-closure.scm")

;; (test eq? 'a 'b)

'ok
