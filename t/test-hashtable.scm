(load-file "lib/hashtable.scm")

(define eq?-hash C:ss_eqQ_hash)

(let ((ht (hashtable-new 10 eq? eq?-hash)))
  (test eq? (hashtable-ref ht 'a 'unknown) 'unknown)
  (hashtable-set! ht 'a 1)
  (hashtable-set! ht 'b 2)
  (test eq? (hashtable-ref ht 'a 'unknown) 1)
  (test eq? (hashtable-ref ht 'b 'unknown) 2)
  )

'ok
