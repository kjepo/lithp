# lithp

`lithp` is a simple LISP interpreter I wrote for CMPS203 Programming
Languages when I was a grad student at UCSC.  The assignment was to
write a class for S-expressions in C++ and some methods to do simple
arithmetic on S-expressions.  I went a bit overboard and wrote a
LISP-interpreter instead.  While `lithp` can do `ADD` and `MUL` I also
handed in solutions with Church numerals, where numbers are represented
by lambda expressions.

`lithp` is 368 lines long and handles lambda expressions, `LET` and `LETREC`
but leaks memory like a sieve.

Also provided are some test files.

```
$ make lithp
$ ./lithp < test.lithp
```
