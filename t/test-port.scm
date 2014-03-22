(test eq? (read_char (open-read-file "/dev/null")) %eos)
(test eq? (eof-object? (read_char (open-read-file "/dev/null"))) #t)
(test eq? (read_char (open-read-file "sssh")) #\#)

'ok
