(test eq? (read_char (open-input-file "/dev/null")) %eos)
(test eq? (eof-object? (read_char (open-input-file "/dev/null"))) #t)
(test eq? (read_char (open-input-file "sssh")) #\#)

'ok
