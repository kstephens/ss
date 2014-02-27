(test eq? (char->integer #\A) 65)
(test eq? (integer->char 48) #\0)

(test eq? (char->integer (integer->char 256)) 0)
(test eq? (char->integer (integer->char 255)) 255)
;; eos <=> -1
(test eq? (integer->char -1) %eos)
(test eq? (char->integer %eos) -1)
(test eq? (char->integer (integer->char -1)) -1)
(test eq? (char->integer (integer->char -2)) -2)
(test not-eq? (%type (integer->char -2)) %<char>)

(test eq? (C:ss_c (C:toupper (C:ss_C #\a))) #\A)
(test eq? (C:ss_c (C:toupper (C:ss_C #\A))) #\A)
(test eq? (C:ss_c (C:tolower (C:ss_C #\A))) #\a)
(test eq? (C:ss_c (C:tolower (C:ss_C #\a))) #\a)

'ok
