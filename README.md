# ss

A Small Scheme

Evaluator uses lazy, eval-by-reference expression rewriting with constant-folding and
other transformations.

## Core Syntax

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

* Eval-by-reference: optimizes and rewrites s-expressions during evaluation.
* Automatically wraps C structs, functions, defines from CPP output of ss.c.
* Simple macros.
* catch, throw, rescue, ensure.

## Eval-By-Reference

The ss_eval() function evaluates expressions by reference --
the evaluator is given the address of a expression value location.
The evaluator can choose to rewrite the expression value using multiple
rewrite mechanisms depending on the type of the expression and the current
context.  Rewriting is lazy -- expressions are rewritten only when they
are evaluated.
Rewriting improves evaluation performance and reduces memory by compressing long pair chains
into efficent semantic objects.  The tagged representations are enumerated in the evaluator in
a C switch statement.

## Rewrite Mechanisms

### Primitive Syntax

Syntax s-expression lists are rewritten as internally tagged expressions, denoted
below as #<TAG ...>.

#### Conditionals

    (if a b)        =>  #<if a b #<unspec>>
    (if a b c)      =>  #<if a b c>

A conditional expression with constant test can be rewritten as either branch.

#### Literals

    (quote x)        =>  #<quote x>

#### Basic Blocks

    (begin a)        =>  a
    (begin a b ...)  =>  #<begin #(a b ...)>

The begin form transform prepares its body for proper tail recursion and space optimization.

#### Application

    (proc args ...)  =>  #<app proc args ...>

The application vector form has a length that can be computed in O(1) time for efficient arity checking.
The vector form is time and space efficent when allocating new parameter bindings.

#### Closures

    (lambda formals . body)  =>  #<lambda formals (begin . body)>

The lambda vector form is aware of its lexical enviroment, parameter positions and rest-args.
The body is rewritten as above to aid proper tail-recursion.

#### Variable References

     sym                 =>  #<var sym up over>
     #<var sym up over>  =>  (top-level? sym) #<global sym #<cell>>
     #<var sym up over>  =>  (constant? sym)  #<quote val>
     (set! sym val)      =>  #<var! #<var sym up over> val>

Initial variable reference expressions are symbols.
Symbols are rewritten as internal variable expressions with "up-and-over" coordinates given the lexical environment:
"up" denotes how many parent closures the variable is bound, "over" denotes its position in the closure's argument vector.
Variable expressions that are bound to top-level environments are rewritten as global variable expressions pointing to new cells containing the original variable value.
Variables that are declared as constants are rewritten as quoted expressions of their value.
Variable assignments are rewritten as #<var!> forms with rewritten variables references.

#### Constant Expression Folding

The evaluator tracks the constant-ness of each evaluated expression.  The source expression
may be rewritten as a constant depending on the context, given these rules:

* A self-evaluating value: number, boolean, string, character, is a constant expression.
* A quoted form is a constant expression.
* A constant variable reference is a constant expression and is rewritten as a quoted value. 
* A side-effect-free function application on constants is a constant expression and is written as a quoted value.
* A constant conditional expression can be rewritten as one of its branch expressions.

#### Macros

Transform expressions using symbol => expression-transformer mapping.

#### Variable-arity Numeric Functions

Common numeric expressions are expanded into inline binary and unary polymorphic primitive subexpressions.
These numeric subexpressions are then subject to constant expression folding.

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

