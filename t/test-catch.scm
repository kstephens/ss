
;; (C:ss_set_eval_verbose 10)
(let ((c #f) (body #f) (rescue #f) (ensure #f) (result #f))
  (set! result
    (%catch
      (lambda (_c)
        (set! c _c)
        (set! body #t)
        'body)
      (lambda (_c _v)
        (test eq? _c c)
        (set! rescue _v)
        'rescue)
      (lambda (_c)
        (test eq? _c c)
        (set! ensure #t)
        'ensure)))
  (test eq? body #t)
  (test eq? rescue #f)
  (test eq? ensure #t)
  (test eq? result 'body))

(let ((c #f) (body #f) (rescue #f) (ensure #f) (result #f))
  (set! result
    (%catch
      (lambda (_c)
        (set! c _c)
        ;; (display "body")(newline)
        (_c 'throw)
        (set! body #t)
        'body)
      (lambda (_c _v)
        ;; (display "rescue")(newline)
        (test eq? _c c)
        (set! rescue _v)
        'rescue)
      (lambda (_c)
        ;; (display "ensure")(newline)
        (test eq? _c c)
        (set! ensure #t)
        'ensure)))
  (test eq? body #f)
  (test eq? rescue 'throw)
  (test eq? ensure #t)
  (test eq? result 'rescue))

(let ((body #f) (rescue #f) (ensure #f) (result #f))
  (set! result
    (catch c
      (begin (set! body #t) 'body)
      (value (set! rescue value) 'rescue)
      (begin (set! ensure #t) 'ensure)))
  (test eq? body #t)
  (test eq? rescue #f)
  (test eq? ensure #t)
  (test eq? result 'body))

(let ((body #f) (rescue #f) (ensure #f) (result #f))
  (set! result
    (catch c
      (begin (set! body #t) (throw c 'body) 'body)
      (value (set! rescue value) 'rescue)
      (begin (set! ensure #t) 'ensure)))
  (test eq? body #t)
  (test eq? rescue 'body)
  (test eq? ensure #t)
  (test eq? result 'rescue))

(test-name "nested catch, no throw")
(let ( (body1 #f) (rescue1 #f) (ensure1 #f) (result1 #f)
       (body2 #f) (rescue2 #f) (ensure2 #f) (result2 #f)
       )
  (set! result1
    (catch c1
      (begin (set! body1 #t) 
        (set! result2
          (catch c2
            (begin (set! body2 #t) 'body2)
            (value (set! rescue2 #t) 'rescue2)
            (begin (set! ensure2 #t) 'ensure2)
            ))
        result2)
      (value (set! rescue1 value) 'rescue1)
      (begin (set! ensure1 #t) 'ensure1)))
  (test eq? body1 #t)
  (test eq? rescue1 #f)
  (test eq? ensure1 #t)
  (test eq? result1 'body2)
  (test eq? body2 #t)
  (test eq? rescue2 #f)
  (test eq? ensure2 #t)
  (test eq? result2 'body2))

(test-name "inner throw/rescue does not affect outer catch.")
(let ( (body1 #f) (rescue1 #f) (ensure1 #f) (result1 #f)
       (body2 #f) (rescue2 #f) (ensure2 #f) (result2 #f)
       )
  (set! result1
    (catch c1
      (begin (set! body1 #t) 
        (set! result2
          (catch c2
            (begin (set! body2 #t) (throw c2 'body2) 'body2)
            (value (set! rescue2 #t) 'rescue2)
            (begin (set! ensure2 #t) 'ensure2)
            ))
        result2)
      (value (set! rescue1 value) 'rescue1)
      (begin (set! ensure1 #t) 'ensure1)))
  (test eq? body1 #t)
  (test eq? body2 #t)
  (test eq? rescue2 #t)
  (test eq? ensure2 #t)
  (test eq? result2 'rescue2)
  (test eq? rescue1 #f)
  (test eq? ensure1 #t)
  (test eq? result1 'rescue2)
  )

(test-name "outer throw/rescue does not affect outer catch.")
(let ( (body1 #f) (rescue1 #f) (ensure1 #f) (result1 #f)
       (body2 #f) (rescue2 #f) (ensure2 #f) (result2 #f)
       )
  ;; (display "c1")(newline)
  (set! result1
    (catch c1
      (begin
        ;; (display "c1 body")(newline)
        (set! body1 #t) 
        (set! result2
          (catch c2
            (begin
              ;; (display "c2 body")(newline)
              (set! body2 #t)
              (throw c1 'body2)
              'body2)
            #f
            (begin
              ;; (display "c2 ensure")(newline)
              (set! ensure2 #t) 'ensure2)
            ))
        result2)
      (value
        ;; (display "c1 rescue")(newline)
        (set! rescue1 value) 'rescue1)
      (begin
        ;; (display "c1 ensure")(newline)
        (set! ensure1 #t) 'ensure1)))
  (test eq? body1 #t)
  (test eq? body2 #t)
  (test eq? rescue2 #f)
  (test eq? ensure2 #t)
  (test eq? result2 #f)
  (test eq? rescue1 'body2)
  (test eq? ensure1 #t)
  (test eq? result1 'rescue1)
  )

(test-name "inner rethrow/rescue does not affect outer catch.")
(let ( (body1 #f) (rescue1 #f) (ensure1 #f) (result1 #f)
       (body2 #f) (rescue2 #f) (ensure2 #f) (result2 #f)
       )
  (set! result1
    (catch c1
      (begin (set! body1 #t) 
        (set! result2
          (catch c2
            (begin (set! body2 #t) (throw c1 'body2) 'body2)
            (value (set! rescue2 #t) (rethrow) 'rescue2)
            (begin (set! ensure2 #t) 'ensure2)
            ))
        result2)
      (value (set! rescue1 value) 'rescue1)
      (begin (set! ensure1 #t) 'ensure1)))
  (test eq? body1 #t)
  (test eq? body2 #t)
  (test eq? rescue2 #t)
  (test eq? ensure2 #t)
  (test eq? result2 #f)
  (test eq? rescue1 'body2)
  (test eq? ensure1 #t)
  (test eq? result1 'rescue1)
  )

(test-name "deep rethrow/rescue.")
(let ( (body1 #f) (rescue1 #f) (ensure1 #f) (result1 #f)
       (body2 #f) (rescue2 #f) (ensure2 #f) (result2 #f)
       (body3 #f) (rescue3 #f) (ensure3 #f) (result3 #f)
       )
  (catch c1
    (begin
      (set! body1 #t) 
      (catch c2
        (begin
          (set! body2 #t)
          (catch c3
            (begin
              (set! body3 #t) 
              (throw c1 'body3)
              'body3)
            #f
            (begin (set! ensure3 #t) 'ensure3))
          'body2)
        (value (set! rescue2 value) (rethrow) 'rescue2)
        (begin (set! ensure2 #t) 'ensure2)
        )
      'body1)
    (value
      (set! rescue1 value)
      'rescue1)
    (begin
      (set! ensure1 #t)
      'ensure1))
  (test eq? body1 #t)
  (test eq? body2 #t)
  (test eq? body3 #t)
  (test eq? rescue3 #f)
  (test eq? ensure3 #t)
  (test eq? rescue2 'body3)
  (test eq? ensure2 #t)
  (test eq? rescue1 'body3)
  (test eq? ensure1 #t)
  )

'ok
