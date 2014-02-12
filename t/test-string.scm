
(let ((s1 "0123456789"))
  (test = (string-length s1) 10)
  (test eq? (string-ref s1 0) #\0)
  (test eq? (string-ref s1 4) #\4)
  (string-set! s1 2 #\C)
  (test eq? (equal? s1 "01C3456789") #t)
)

'ok
