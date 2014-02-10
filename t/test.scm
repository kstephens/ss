(define (test cmp a b)
  (if (cmp a b) a
    (error 'test " expected:" b " given:" a)))

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

'ok
