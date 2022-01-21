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

Here's an example of the infamous Fibonacci function:
```
(LETREC (FIB (LAMBDA (N)
	(IF (EQ N 0) 1
	        (IF (EQ N 1) 1
		    	(ADD (FIB (SUB N 1)) (FIB (SUB N 2)))))))
			     (FIB 5))
```			     

Some more test files are included in the repo.  To compile and invoke:

```
$ make lithp
$ ./lithp < test.lithp
```
