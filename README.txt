This is release version 1.5 of FC++, a library for functional and
declarative programming in C++.

The release comes in two parts:

  1. FC++ library code, available from either FC++.1.5.tar.gz (for
     unix-like operating systems) or FC++.1.5.zip (for Microsoft Windows
     operating systems).  This contains that actual library code header
     files themselves which you will need to "include" in your programs.

  2. FC++ example/test files, available from either FC++-clients.1.5.tar.gz
     or FC++-clients.1.5.zip.  These are examples of how FC++ may actually
     be used and are one of the best ways to learn how to use the library.

For more information about FC++, please see the project web page

  http://sourceforge.net/projects/fcpp

or see the original web page

  http://www-static.cc.gatech.edu/~yannis/fc++

(This project is in the early stages of being moved to sourceforge.net
so the latter web page is currently (8 February 2007) the more useful
of the two.)

We conclude with a summary of what each of the library header files are for.

config.h     Auto-detects certain compilers/versions to deal with compiler bugs
curry.h      Has the bindMofN() functoids, the curryN() operators, and Const()
full.h       Defines FullN functoid wrappers and makeFullN()
function.h   The indirect functoid classes (FunN) and supporting implementation
lambda.h     The guts of lambda(), its special syntax, LEType
list.h       The List class and its support functoids
monad.h      Defines operations like unit(),bind(); instances like List,Maybe
operator.h   Operators like Plus, many conversion functions, misc
pre_lambda.h A number of forward decls and meta-programming helpers
prelude.h    Functions found in the Haskell Standard Prelude
ref_count.h  Reference-counting pointer classes
reuse.h      The ReuserN classes (which make recursive functoids more efficient)
signature.h  Classes like FunType (used for nested typedefs)
smart.h      Smartness infrastructure and FunctoidTraits class
