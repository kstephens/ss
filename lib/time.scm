(define (%gettimeofday)
  (let ((tv (C:%struct-timeval)))
    (C:%gettimeofday tv C:%NULL)
    tv))

(define (current-time)
  (let ((tv (%gettimeofday)))
    (+
      (C:ss_to_flonum (C:%struct-timeval.tv_sec tv))
      (/ (C:ss_to_flonum (C:%struct-timeval.tv_usec tv)) 1000000.0))))

