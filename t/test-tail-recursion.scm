(define g (lambda (x) (if (> x 0) (f (- x 1)) 'g)))
(define f (lambda (x) (if (> x 0) (g (- x 1)) 'f)))
(test eq? (g 1000000) 'g)
(test eq? (g 1000001) 'f)

'ok

