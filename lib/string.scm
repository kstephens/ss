(define (%string-escape-1 s i t j)
  (define (ac c)
    (string-set! t j c)
    (set! j (+ j 1)))
  (define (ao i)
    (ac (integer->char (+ (char->integer #\0) (truncate-remainder i 8)))))
  (if (>= i (string-length s))
    (%string-truncate! t j)
    (let ((k (char->integer (string-ref s i))))
      (case k
        ((10) (ac #\\) (ac #\n))
        ((13) (ac #\\) (ac #\r))
        ((9)  (ac #\\) (ac #\t))
        ((7)  (ac #\\) (ac #\a))
        ((8)  (ac #\\) (ac #\b))
        ((27) (ac #\\) (ac #\e)) ;; alt
        ((92 34) (ac #\\) (ac (integer->char k)))
        (else
          (if (and (>= k 32) (< k 127))
            (ac (integer->char k))
            (begin
              (ac #\\)
              (ao (truncate-quotient k 64))
              (ao (truncate-quotient k 8))
              (ao k)
              ))))
      (%string-escape-1 s (+ i 1) t j))))
(define (%string-escape s)
  (%string-escape-1 s 0 (make-string (* 4 (string-length s))) 0))

(define (%char->digit c)
  (cond
    ((and (char<=? #\0 c) (char<=? c #\9))    (- (char->integer c) (char->integer #\0)))
    ((and (char<=? #\a c) (char<=? c #\z)) (+ (- (char->integer c) (char->integer #\a)) 10))
    ((and (char<=? #\A c) (char<=? c #\Z)) (+ (- (char->integer c) (char->integer #\A)) 10))
    (else #f)))

(define (string->integer s . radix)
  (%string-to-number s (if radix (car radix) 10)))

(define (%string-unescape-1 s i j)
  (if (>= i (string-length s))
    (%string-truncate! s j)
    (begin
      (if (and (char=? #\\ (string-ref s i)) (< i (- (string-length s) 1)))
        (begin
          (set! i (+ i 1))
          (let ((c (string-ref s i)))
            (set! c (case c
                      ((#\n) (integer->char 10))
                      ((#\r) (integer->char 13))
                      ((#\t) (integer->char 9))
                      ((#\b) (integer->char 8))
                      ((#\a) (integer->char 7))
                      ((#\e) (integer->char 27)) ;; alt
                      ((#\x #\X) c) ;; FIXME
                      ((#\0)     c) ;; FIXME
                      (else      c)
                      ))
            (string-set! s j c)))
        (string-set! s j (string-ref s i)))
      (%string-unescape-1 s (+ i 1) (+ j 1)))))
(define (%string-unescape s) (%string-unescape-1 s 0 0))

(define (string-copy! to at from start end)
  (if (and (< at (string-length to)) (< start end))
    (begin
      (string-set! to at (string-ref from start))
      (string-copy! to (+ at 1) from (+ start 1) end)
      )))

(define (%string-fill! string fill start end)
  (if (and (< start (string-length to)) (< start end))
    (begin
      (string-set! string start fill)
      (%string-fill! string (+ start 1) end)
      )))
(define (string-fill string fill . opts)
  (let ((start 0) (end (string-length string)))
    (if (not (null? opts))
      (begin
        (set! start (car opts))
        (if (not (null? (cdr opts)))
          (set! end (cadr opts)))))
    (%string-fill! string fill start end)))
