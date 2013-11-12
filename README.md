# ss

A Small Scheme

Implements lazy, threaded expression rewriting with constant-folding.

## Syntax

   (define VAR VALUE)
   (set! VAR VALUE)
   (lambda FORMALS . BODY)
   (let BINDINGS . BODY)
   (quote DATA)

## Primitives

   (cons A B)
   (car P)
   (cdr P)
   (+ x...)
   (- x)
   (- x y...)
   (* x...)
   (/ x)
   (/ x...)
   (write OBJ)
   (newline)
   (read)

## Values

   integers
   reals
   strings
   characters
   vectors
   pairs
   booleans

## Build

   make

Requires perl and BDWGC installed.

## Run

   ./ss

