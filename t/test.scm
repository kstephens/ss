(define (test-error . args)
  (display "test-error: ")
  (write args)
  (newline)
  (C_abort))

(define *test-verbose* #f)
(define (%test cmp a b expr)
  (if *test-verbose*
    (begin
      (write expr) (newline)))
  (if (cmp a b) a
    (test-error expr 'given: a 'expected: b)))
(define-macro (test cmp a b)
  `(%test ,cmp ,a ,b '(test ,cmp ,a ,b)))

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
(test-file "t/test-tail-recursion.scm")

;; (test eq? 'a 'b)

'ok
