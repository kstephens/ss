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
    (write OBJ . PORT)
    (display OBJ . PORT)
    (newline . PORT)
    (read . PORT)

## Values

* Integer (immediate fixnum)
* Real (boxed flonum)
* String
* Character (immediate)
* Vector
* Pair
* Boolean
* Port
* Catch

## Features

* Eval-by-reference: rewrites s-expressions during evaluation.
* Automatically generates C function callables from CPP output of ss.c.
* Simple macros.
* catch, throw, rescue, ensure.

## Build

    $ make
     
## Test

    $ ./sssh
     ss> (load "t/test.scm")

## Portability

* 64-bit Linux
* 64-bit OS X

## Dependencies

* perl
* rlwrap
* BDW GC

## Run

    $ ./sssh

