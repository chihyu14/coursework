/* $Id: main.cpp,v 1.4 2012-12-04 19:36:57-08 - - $
 * Derek Frank, dmfrank@ucsc.edu
 *
 * NAME
 *    ydc - an arbitrary precision calculator
 *
 * SYNOPSIS
 *    ydc  [-@ flags]
 *
 * DESCRIPTION
 *    Dc  is a reverse-polish desk calculator  which  supports unlimited
 *    precision  arithmetic.   It  also  allows you to define  and  call
 *    macros.    Normally  dc  reads  from the  standard  input; if  any
 *    command  arguments  are given to it,  they are  filenames,  and dc
 *    reads and executes  the contents of  the files before reading from
 *    standard  input.   All normal  output is to  standard output;  all
 *    error output is to standard error.
 *
 *    A reverse-polish calculator stores numbers on a stack.  Entering a
 *    number  pushes  it  on  the  stack.    Arithmetic  operations  pop
 *    arguments off the stack and push the results.
 *
 *    To enter a number in dc,  type the digits with an optional decimal
 *    point.  Exponential notation is not supported. To enter a negative
 *    number,  begin the  number with  ``_''.  ``-'' cannot  be used for
 *    this,  as it  is a binary  operator for  subtraction instead.   To
 *    enter  two  numbers  in succession,  separate them  with spaces or
 *    newlines.  These have no meaning as commands.
 *
 * OPTIONS
 *    -@  flags
 *           The -@ option is  followed by a sequence of flags to enable
 *           debugging output, which is written to cerr.
 *
 * Printing Commands
 *    p      Prints the value on the top of the stack, without  altering
 *           the stack.  A newline is printed after the value.
 *
 *    f      Prints  the  entire  contents of the stack without altering
 *           anything.  This is a good command to use if you are lost or
 *           want to  figure  out what  the effect  of some  command has
 *           been.
 *
 * Arithmetic
 *    +      Pops two values  off the stack,  adds them,  and pushes the
 *           result.   The precision of the result is determined only by
 *           the values of the arguments, and is enough to be exact.
 *
 *    -      Pops  two values,  subtracts the first  one popped from the
 *           second one popped, and pushes the result.
 *
 *    *      Pops two values,  multiplies them,  and pushes  the result.
 *           The number of fraction digits  in the result depends on the
 *           current precision  value and the  number of fraction digits
 *           in the two arguments.
 *
 *    /      Pops two values,   divides the second  one popped  from the
 *           first  one popped,  and pushes  the result.   The number of
 *           fraction digits is specified by the precision value.
 *
 *    %      Pops two  values,  computes the remainder  of the  division
 *           that the / command  would do,  and pushes that.   The value
 *           computed  is the  same  as that  computed  by the  sequence
 *           Sd dld/ Ld*- .
 *
 *    ^      Pops two values  and exponentiates,  using the  first value
 *           popped as the exponent  and the second  popped as the base.
 *           The fraction part of the exponent is ignored. The precision
 *           value  specifies  the  number  of  fraction  digits  in the
 *           result.
 *
 *    Most arithmetic operations are affected by the ``precision value",
 *    which you can set with the k command.  The default precision value
 *    is zero,  which means that all  arithmetic except for addition and
 *    subtraction produces integer results.
 *
 * Stack Control
 *    c      Clears the stack, rendering it empty.
 *
 *    d      Duplicates  the value  on the  top of  the  stack,  pushing
 *           another copy of it.   Thus, ``4d*p'' computes 4 squared and
 *           prints it.
 *
 * Strings
 *    Dc can operate on strings as well as on numbers.   The only things
 *    you can do with strings  are print them and execute them as macros
 *    (which means that the contents of the string are processed  as  dc
 *    commands).   All registers and the stack can hold strings,  and dc
 *    always knows  whether any  given object  is a string  or a number.
 *    Some  commands  such as  arithmetic  operations  demand numbers as
 *    arguments and print  errors if given strings.   Other commands can
 *    accept either a number or a string; for example, the p command can
 *    accept either and prints the object according to its type.
 *
 *    q      exits from  a macro and also  from the macro  which invoked
 *           it. If called from the top level, or from a macro which was
 *           called  directly  from the top  level,  the q command  will
 *           cause dc to exit.
 *
 * EXIT STATUS
 *
 *    0      No errors were detected.
 *
 *    1      Error messages were printed to cerr.
 */


#include <deque>
#include <exception>
#include <map>
#include <iostream>
#include <utility>
#include <cstdio>
#include <unistd.h>

using namespace std;

#include "bigint.h"
#include "iterstack.h"
#include "util.h"
#include "scanner.h"
#include "trace.h"

typedef iterstack<bigint> bigint_stack;

#define DO_BINOP(FN_NAME,TFLAG,OPER) \
   void FN_NAME (bigint_stack &stack) { \
      bigint right = stack.front(); \
      stack.pop_front(); \
      TRACE (TFLAG, "right = " << right); \
      bigint left = stack.front(); \
      stack.pop_front(); \
      TRACE (TFLAG, "left = " << left); \
      bigint result = left OPER (right); \
      TRACE (TFLAG, "result = " << result); \
      stack.push_front (result); \
   }
DO_BINOP(do_add, '+', +   )
DO_BINOP(do_sub, '-', -   )
DO_BINOP(do_mul, '*', *   )
DO_BINOP(do_div, '/', /   )
DO_BINOP(do_rem, '%', %   )
DO_BINOP(do_pow, '^', .pow)

void do_clear (bigint_stack &stack) {
   TRACE ('c', "");
   stack.clear();
}

void do_dup (bigint_stack &stack) {
   bigint top = stack.front();
   TRACE ('d', top);
   stack.push_front (top);
}

void do_printall (bigint_stack &stack) {
   bigint_stack::const_iterator itor = stack.begin();
   bigint_stack::const_iterator end = stack.end();
   for (; itor != end; ++itor) cout << *itor << endl;
}

void do_print (bigint_stack &stack) {
   cout << stack.front() << endl;
}

void do_debug (bigint_stack &stack) {
   (void) stack; // SUPPRESS: warning: unused parameter 'stack'
   cout << "Y not implemented" << endl;
}

class ydc_quit: public exception {};
void do_quit (bigint_stack &stack) {
   (void) stack; // SUPPRESS: warning: unused parameter 'stack'
   throw ydc_quit ();
}

typedef void (*function) (bigint_stack&);
typedef map <string, function> fnmap;
fnmap load_fn () {
   fnmap functions;
   functions["+"] = do_add;
   functions["-"] = do_sub;
   functions["*"] = do_mul;
   functions["/"] = do_div;
   functions["%"] = do_rem;
   functions["^"] = do_pow;
   functions["Y"] = do_debug;
   functions["c"] = do_clear;
   functions["d"] = do_dup;
   functions["f"] = do_printall;
   functions["p"] = do_print;
   functions["q"] = do_quit;;
   return functions;
}

//
// scan_options
//    Options analysis:  The only option is -Dflags. 
//

void scan_options (int argc, char **argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            traceflags::setflags (optarg);
            break;
         default:
            complain() << "-" << (char) optopt << ": invalid option"
                       << endl;
            break;
      }
   }
   if (optind < argc) {
      complain() << "operand not permitted" << endl;
   }
}

int main (int argc, char **argv) {
   sys_info::set_execname (argv[0]);
   scan_options (argc, argv);
   fnmap functions = load_fn();
   bigint_stack operand_stack;
   scanner input;
   try {
      for (;;) {
         try {
            token_t token = input.scan();
            if (token.symbol == SCANEOF) break;
            switch (token.symbol) {
               case NUMBER:
                  operand_stack.push_front (token.lexinfo);
                  break;
               case OPERATOR: {
                  function fn = functions[token.lexinfo];
                  if (fn == NULL) {
                     throw ydc_exn (octal (token.lexinfo[0])
                                    + " is unimplemented");
                  }
                  fn (operand_stack);
                  break;
                  }
               default:
                  break;
            }
         }catch (ydc_exn exn) {
            cout << exn.what() << endl;
         }
      }
   }catch (ydc_quit) {
   }
   return sys_info::get_status ();
}

