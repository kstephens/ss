
(define (vector-sort cmp coll)
  (vector-sort! cmp (vector-copy coll)))

(define (vector-sort! cmp coll)
  (C:ss_sort &env coll cmp))

(define (list-sort cmp coll)
  (vector->list
    (vector-sort! cmp (list->vector coll))))

(define (list-sort! cmp coll)
  (vector->list!
    (vector-sort! cmp (list->vector coll))
    coll))

(define (sort cmp coll)
  (if (vector? coll)
    (vector-sort cmp coll)
    (list-sort cmp coll)))

(define (sort! cmp coll)
  (if (vector? coll)
    (vector-sort! cmp coll)
    (list-sort! cmp coll)))

