
(test eq? (C:ss_c (C:toupper (C:ss_C #\a))) #\A)
(test eq? (C:ss_c (C:toupper (C:ss_C #\A))) #\A)
(test eq? (C:ss_c (C:tolower (C:ss_C #\A))) #\a)
(test eq? (C:ss_c (C:tolower (C:ss_C #\a))) #\a)

'ok