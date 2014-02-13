
(let ((c #f) (rescued #f) (ensured #f) (result #f))
  (set! result
    (%catch
      (lambda (_c)
        ;; (display "body")(newline)
        (set! c _c)
        ;; (display "%throw")(newline)
        (%throw _c 'throw)
        'body)
      (lambda (_c _v)
        ;; (display "rescue")(newline)
        (test eq? _c c)
        (set! rescued _v)
        'rescue)
      (lambda (_c)
        ;; (display "ensure")(newline)
        (test eq? _c c)
        (set! ensured #t)
        'ensure)))
  (test eq? rescued 'throw)
  (test eq? ensured #t)
  (test eq? result 'rescue))

'ok
