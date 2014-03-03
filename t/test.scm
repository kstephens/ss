(define (test-error . args)
  (display "test-error: ")
  (write args)
  (newline)
  (error 'test-error "FAILED" args))

(define *test-verbose* #f)
(define (%test cmp a b expr)
  (if *test-verbose*
    (begin
      (write expr) (newline)))
  (if (cmp a b) a
    (test-error expr 'given: a 'expected: b)))
(define-macro (test cmp a b)
  `(%test ,cmp ,a ,b '(test ,cmp ,a ,b)))

(define (not-eq? a b) (not (eq? a b)))

(define (epsilon? e)
  (lambda (a b)
    (if (< (- b e) a)
      (< a (+ b e))
      #f)))

(define (test-name name)
  (if *test-verbose*
    (begin
      (display "  ;; test: ")(display name)(display " ...")(newline))))

(define (test-file name)
  (display ";; test ")(display name)(display " ...")(newline)
  (test eq? (load name) 'ok)
  (display ";; test ")(display name)(display "  OK")(newline)
  )

;;(set! *test-verbose* #t)
;;(set! *test-verbose* #f)
(test-file "t/test-catch.scm")
(test-file "t/test-equal.scm")
(test-file "t/test-char.scm")
(test-file "t/test-string.scm")
(test-file "t/test-math.scm")
(test-file "t/test-deep-closure.scm")
(test-file "t/test-tail-recursion.scm")
(test-file "t/test-sort.scm")

;; (test eq? 'a 'b)

'ok
