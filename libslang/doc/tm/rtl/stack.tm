\function{__pop_args}
\synopsis{Remove n function arguments from the stack}
\usage{variable args = __pop_args(Integer_Type n);}
\description
  This function together with the companion function \var{__push_args}
  is useful for passing the arguments of a function to another function.
  \var{__pop_args} returns an array of \var{n} structures with a
  single structure field called \var{value}, which represents the value
  of the argument.
\example
  Consider the following \var{print} function.  It prints all its
  arguments to \var{stdout} separated by spaces:
#v+
    define print ()
    {
       variable i;
       variable args = __pop_args (_NARGS);
   
       for (i = 0; i < _NARGS; i++)
         {
            () = fputs (string (args[i].value), stdout);
            () = fputs (" ", stdout);
         }
       () = fputs ("\n", stdout);
       () = fflush (stdout);
    }
#v-
  Now consider the problem of defining a function called \var{ones}
  that returns a multi-dimensional array with all the elements set to
  1.  For example, \exmp{ones(10)} should return a 1-d array of ones,
  whereas \exmp{ones(10,20)} should return a 10x20 array.
#v+
    define ones ()
    {
      !if (_NARGS) return 1;
      variable a;
   
      a = __pop_args (_NARGS);
      return @Array_Type (Integer_Type, [__push_args (a)]) + 1;
    }
#v-
  Here, \var{__push_args} was used to push on the arguments passed to
  the \var{ones} function onto the stack to be used when dereferencing
  \var{Array_Type}.
\seealso{__push_args, typeof, _pop_n}
\done

\function{__push_args}
\synopsis{Remove n function arguments onto the stack}
\usage{__push_args (Struct_Type args);}
\description
  This function together with the companion function \var{__pop_args}
  is useful for passing the arguments of one function to another.
  See the desription of \var{__pop_args} for more information.
\seealso{__pop_args, typeof, _pop_n}
\done

\function{_pop_n}
\synopsis{Remove objects from the stack}
\usage{_pop_n (Integer_Type n);}
\description
  The \var{_pop_n} function pops \var{n} objects from the top of the
  stack.
\example
#v+
    define add3 ()
    {
       variable x, y, z;
       if (_NARGS != 3)
         {
            _pop_n (_NARGS);
            error ("add3: Expecting 3 arguments");
         }
       (x, y, z) = ();
       return x + y + z;
    }
#v-
\seealso{_stkdepth, pop}
\done

\function{_print_stack}
\synopsis{print the values on the stack.}
\usage{_print_stack ()}
\description
  This function dumps out what is currently on the \slang.  It does not
  alter the stack and it is usually used for debugging purposes.
\seealso{_stkdepth, string}
\done

\function{_stk_reverse}
\synopsis{Reverse the order of the objects on the stack.}
\usage{_stk_reverse (Integer_Type n)}
\description
   The \var{_stk_reverse} function reverses the order of the top
   \var{n} items on the stack.
\seealso{_stkdepth, _stk_roll}
\done

\function{_stk_roll}
\synopsis{Roll items on the stack}
\usage{_stk_roll (Integer_Type n);}
\description
  This function may be used to alter the arrangement of objects on the
  stack.  Specifically, if the integer \var{n} is positive, the top
  \var{n} items on the stack are rotated up.  If
  \var{n} is negative, the top \var{abs(n)} items on the stack are
  rotated down.
\example
  If the stack looks like:
#v+
    item-0
    item-1
    item-2
    item-3
#v-
  where \exmp{item-0} is at the top of the stack, then
  \exmp{_stk_roll(-3)} will change the stack to:
#v+
    item-2
    item-0
    item-1
    item-3
#v-
\notes
  This function only has an effect for \exmp{abs(n) > 1}.
\seealso{_stkdepth, _stk_reverse, _pop_n, _print_stack}
\done

\function{_stkdepth}
\usage{Get the number of objects currently on the stack.}
\synopsis{Integer_Type _stkdepth ()}
\description
  The \var{_stkdepth} function returns number of items on stack prior
  to the call of \var{_stkdepth}.
\seealso{_print_stack, _stk_reverse, _stk_roll}
\done

\function{dup}
\synopsis{Duplicate the value at the top of the stack}
\usage{dup ()}
\description
  This function returns an exact duplicate of the object on top of the
  stack.  For some objects such as arrays or structures, it creates a
  new reference to the array.  However, for simple scalar S-Lang types such
  as strings, integers, and doubles, it creates a new copy of the
  object.
\seealso{pop, typeof}
\done

\function{exch}
\synopsis{Exchange two items on the stack}
\usage{exch ()}
\description
  The \var{exch} swaps the two top items on the stack.
\seealso{pop, _stk_reverse, _stk_roll}
\done

\function{pop}
\synopsis{Discard an item from the stack}
\usage{pop ()}
\description
  The \var{pop} function removes the top item from the stack.
\seealso{_pop_n}
\done

