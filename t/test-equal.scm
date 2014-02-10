
(test eq? (eq? 0 0) #t)
(test eq? (eq? 0 1) #f)
(test eq? (eq? 1 1.0) #f)

(test eq? (equal? 0 0) #t)
(test eq? (equal? 0 1) #f)
(test eq? (equal? 1 1.0) #f)

(test eq? (equal? 'a 'a) #t)
(test eq? (equal? 'a 'b) #f)

(test eq? (equal? "a" "a") #t)
(test eq? (equal? "a" "ab") #f)
(test eq? (equal? "abc" "abc") #t)

(test eq? (equal? (cons 1 1) (cons 1 1)) #t)
(test eq? (equal? (cons 2 1) (cons 1 1)) #f)
(test eq? (equal? (cons 1 2) (cons 1 1)) #f)

(test eq? (equal? '#() '#()) #t)
(test eq? (equal? '#(1) '#(1)) #t)
(test eq? (equal? '#(1) '#(1 2)) #f)
(test eq? (equal? '#(1 2) '#(1)) #f)

'ok
