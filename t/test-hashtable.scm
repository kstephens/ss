(load-file "lib/hashtable.scm")

(define eq?-hash C:ss_eqQ_hash)

#;
(let ((l '((a 1) (b 2) (c 3))))
  (test equal? l '((a 1) (b 2) (c 3)))
  (%assrm eq? 'a l)
  (test equal? l '((a 1) (b 2) (c 3)))
  (%assrm eq? 'b l)
  (test equal? l '((a 1) (c 3)))
  (%assrm eq? 'x l)
  (test equal? l '((a 1) (c 3)))
  )

(let ((ht (hashtable-new 10 eq? eq?-hash)))
  (test eq? (hashtable-ref ht 'a 'unknown) 'unknown)
  (hashtable-set! ht 'a 1)
  (hashtable-set! ht 'b 2)
  (test eq? (hashtable-ref ht 'a 'unknown) 1)
  (test eq? (hashtable-ref ht 'b 'unknown) 2)
  (hashtable-delete! ht 'a)
  (test eq? (hashtable-ref ht 'a 'unknown) 'unknown)
  (test eq? (hashtable-ref ht 'b 'unknown) 2)
  (hashtable-delete! ht 'b)
  (test eq? (hashtable-ref ht 'a 'unknown) 'unknown)
  (test eq? (hashtable-ref ht 'b 'unknown) 'unknown)
  )

(let ((ht (hashtable-new 10 eq? (lambda (x) 5))))
  (hashtable-set! ht 'a 1)
  (hashtable-set! ht 'b 2)
  (test eq? (hashtable-ref ht 'a 'unknown) 1)
  (test eq? (hashtable-ref ht 'b 'unknown) 2)
  (hashtable-delete! ht 'a)
  (test eq? (hashtable-ref ht 'a 'unknown) 'unknown)
  (test eq? (hashtable-ref ht 'b 'unknown) 2)
  (hashtable-delete! ht 'b)
  (test eq? (hashtable-ref ht 'a 'unknown) 'unknown)
  (test eq? (hashtable-ref ht 'b 'unknown) 'unknown)
  )

'ok
