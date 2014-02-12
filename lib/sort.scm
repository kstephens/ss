(define (sort cmp coll)
  (if (vector? coll)
    (sort! cmp (vector-copy coll))
    (vector->list! (sort! cmp (list->vector coll)) coll)))

(define (sort! cmp coll)
  (if (vector? coll)
    (C:ss_sort &env coll cmp)
    (vector->list!
      (C:ss_sort &env (list->vector coll) cmp)
      coll)))

