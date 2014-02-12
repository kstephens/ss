
(let ( (v1 '#(a b c d))
       (l1 (list 0 1 2 3)))
  (vector->list! v1 l1)
  (test equal? l1 '(a b c d)))

(let ( (v1 '#(5 2 6 7 3 8 1 0 4))
       (l1 '(5 2 6 7 3 8 1 0 4)))
  (test equal? (sort < v1) '#(0 1 2 3 4 5 6 7 8))

  (sort! < v1)
  (test equal? v1 '#(0 1 2 3 4 5 6 7 8))
  (sort! > v1)
  (test equal? v1 '#(8 7 6 5 4 3 2 1 0))

  (test equal? (sort < l1) '(0 1 2 3 4 5 6 7 8))

  (sort! < l1)
  (test equal? l1 '(0 1 2 3 4 5 6 7 8))
  (sort! > l1)
  (test equal? l1 '(8 7 6 5 4 3 2 1 0))
)

'ok
