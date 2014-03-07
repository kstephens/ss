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

S-expression syntax is lists rewritten as internally tagged expressions, denoted
below with "#".

#### Conditionals

```scheme
(if a b)        =>  #<if a b #<unspec> >
(if a b c)      =>  #<if a b c >
```

A conditional expression with constant test can be rewritten as either branch.

#### Literals

```scheme
(quote x)        =>  #<quote x>
```

#### Basic Blocks

```scheme
(begin a)        =>  a
(begin a b ...)  =>  #<begin a b ...>
```

The begin form transform prepares its body for proper tail recursion and space optimization.

#### Application

```scheme
(proc args ...)  =>  #<(proc args ...)>
```

The application vector form is time and space efficient for mapping parameters to arguments and arity checking.

#### Closures

```scheme
(lambda formals . body)  =>  #<l formals (begin . body)>
```

The lambda form contains parameter, rest-arg positions and its lexical enviroment.
The body is transformed to a basic block which is rewritten to aid proper tail-recursion.

#### Variable References

```scheme
sym                 =>  #<v sym up over>
#<v sym up over>    =>  #<g sym #<cell>>   iff (top-level? sym)
#<v sym up over>    =>  #<quote val>       iff (constant? sym)
(set! sym val)      =>  #<v! #<v sym up over> val>
```

Initial variable reference expressions are symbols.
A symbol is rewritten as a variable expressions with "up-and-over" coordinates:
"up": how many parent closures links to follow, "over" index into its closure argument vector.
A variable bound in the top-level environment is rewritten as a global variable pointing to a new cell containing the original variable value.
A variable declared as a constants is rewritten as a quoted value.
A variable assignment is rewritten as an assignement form containing a variable and a value expression.

#### Constant Expression Folding

The evaluator tracks the constant-ness of each evaluated expression.  The source expression
may be rewritten as a constant depending on the context, given these rules:

* A self-evaluating value: number, boolean, string, character, is a constant expression.
* A quoted form is a constant expression.
* A constant variable reference is a constant expression and is rewritten as a quoted value.
* A side-effect-free function application on constants is a constant expression and is written as a quoted value.
* A constant conditional expression can be rewritten as one of its branch expressions.

#### Macros

A macro is an expression transformer bound to a symbol.  An application form with a macro symbol in the car position is macro application.  A macro transformer is applied to macro application arguments and the process is repeated.

#### Variable-arity Numeric Functions

Common numeric expressions are expanded into inline binary and unary polymorphic primitive subexpressions.
These numeric subexpressions are then subject to constant expression folding.

### Rewrite Examples

#### Conditional Folding

```scheme
  #> (define (f) (if #t 1 0))
  #> (%type f)
#<type closure >

      ;; The 3rd slot of a closure is its lambda.
  #> (%type (C:ss_get f 2))
#<type lambda >

  #> (C:%ss_get f 2)
#<l () (begin (if #t 1 0)) >

  #> (f)
1
  #> (C:%ss_get f 2)
#<l () 1 >
```

#### Global Variable

```scheme
  #> (define x '(a b c d e))
  #> (define (f) (cons (car (cdr (cdr x))) 5))

    ;; Note: lambda body is in consed s-expression form.
  #> (C:ss_get f 2)
#<l () (begin (cons (car (cdr (cdr x))) 5)) >

    ;; After evaluation, function applications and variable references in the lambda body are rewritten.
  #> (f)
(c . 5)
  #> (C:ss_get f 2)
#<l () #<(#<g cons > #<(#<g car > #<(#<g cdr > #<(#<g cdr > #<g x >)> )> )>  5)>  >
```

#### Numeric Operator Expansion and Global Constant Substitution

```scheme
  #> (define (f x) (+ x 2 3 5))
  #> (C:ss_get f 2)
#<l (x) (begin (+ x 2 3 5)) >

;; Note: (ss_ADD 2 (ss_ADD 3 (ss_ADD 5))) => 10
  #> (f 1)
11
  #> (C:ss_get f 2)
#<l (x) #<(#<g ss_ADD > #<v x 0 0> 10)>  >

    ;; Note: the constant global var is replaced with its constant value.
  #> (C:ss_make_constant 'ss_ADD)
ss_ADD
  #> (f 1)
11
  #> (C:ss_get f 2)
#<l (x) #<(#<p ss_ADD #@0x0 (+ z ...) :safe > #<v x 0 0> 10)>  >
```

## C FFI

The Makefile creates *.def files under gen/ which are generated from C preprocessor output of ss.c, ss.i.  This information is used to create introspective data about ss.c:

* Types from ss_t_*.
* Symbols from ss_sym(*).
* Primitives from ss_prim(...)
* Syntax from ss_syntax(..)
* C types parsed from ss.i.
* C structs parsed from ss.i.
* C functions parsed from ss.i.
* C #defines parsed from ss.i for string and numeric constants.

The cwrap.c creates wrapping primitives around C data types and functions.
The C primitives are prefixed with "C:".

### FFI Examples

```scheme
  ;; Create a float[10] array.
  #> (define fa (C:make-float*: 10 0.5))
  #> fa
#<C:float* >
  ;; Get fa[0].
  #> (C:float*-ref fa 0)
0.5
  ;; Set fa[0].
  #> (C:float*-set! fa 0 1.23)
#<C:float* >
  #> (C:float*-ref fa 0)
1.2300000190734863

  ;; char* are boxed as strings.
  #> (C:getenv "PATH")
"/opt/local/bin:/opt/local/sbin:..."

  ;; %-prefix functions are unsafe,
  ;;   no boxing/unboxing of arguments or return values.
  ;; C:%ss_s creates strings from C char*,
  ;; C:%ss_S creates C char* from strings.
  #> (C:%ss_s (C:%getenv (C:%ss_S "PATH")))
"/opt/local/bin:/opt/local/sbin:..."
```
### Low-level Access

```scheme
  #> (C:strtol "1234" C:%NULL 10)
1234
  #> (C:%ss_i (C:%strtol (C:%ss_S "1234") C:%NULL (C:%ss_I 10)))
1234
  #> (C:strtod "1234.56" C:%NULL)
1234.56
  #> (C:%ss_box_flonum (C:%strtod (C:%ss_S "1234.56") C:%NULL (C:%ss_I 10)))
1234.56 ;; ???
```

### Interesting Hackery

```scheme
    ;; The offset of struct ss_s_type instance_size element.
    ;; In C: (size_t) &((struct ss_s_type*) 0)->instance_size)
  #> (C:%ss_i (C:%ss_get (C:%struct-ss_s_type.instance_size& C:%NULL) 0))
32
    ;; The first element is C_ssP is the ss* to instance_size.
  #> (C:%struct-ss_s_type.instance_size& C:%NULL)
#<C_ssP #@0x105408828 >

```

## Surprises

Most of the primitives, especially the C primitives are not typesafe, only a handful of primitives
check type or arity.  Incorrect values or types can cause an ungraceful fatal error.
The goal was to simplify C callouts and minimize the bootstrapping and build the
typechecking after boot.scm and in the JIT or compiler.

## Build

    $ make
     
## Test

    $ ./sssh
     ss> (load "t/test.scm")

## Portability

* 64-bit OS X
* 64-bit Linux (currently broken)

## Dependencies

* perl
* rlwrap
* BDW GC
* colorgcc (on Linux)

## Run

    $ ./sssh

