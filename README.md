# Nil

Nil is a tiny SECD-style Lisp executor.

I wrote it to demonstrate that a minimal bootstrappable Lisp implementation could be written in a few hundred lines of code. In fact, the entire engine fits in less than 1000 lines of C.

## Building and running

In order to build and run Nil, you need the following programs:

 - GNU Make
 - GCC
 - SBCL (any Common Lisp implementation should do the trick, but SBCL is recommended)

Compile the executable by running `cd src/ && make`. This should produce `nil.app`, which you can run by typing `./nil.app` within the `src/` directory. Once you start Nil, the program expects input. No prompt is printed.

In another window, type `cd src/lisp/`, then `./compile.sh`, and copy-paste the resulting SECD program into the running Nil instance. Nil will then automatically start execution of the compiled program.

