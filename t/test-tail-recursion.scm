(define g (lambda (x) (if (> x 0) (f (- x 1)) 'g)))
(define f (lambda (x) (if (> x 0) (g (- x 1)) 'f)))
(g 100000)
