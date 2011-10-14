\function{_clear_error}
\synopsis{Clear an error condition}
\usage{_clear_error ()}
\description
  This function may be used in error-blocks to clear the error that
  triggered execution of the error block.  Execution resumes following
  the statement, in the scope of the error-block, that triggered the
  error.
\example
  Consider the following wrapper around the \var{putenv} function:
#v+
    define try_putenv (name, value)
    {
       variable status;
       ERROR_BLOCK
        {
          _clear_error ();
          status = -1;
        }
       status = 0;
       putenv (sprintf ("%s=%s", name, value);
       return status;
    }
#v-
  If \var{putenv} fails, it generates an error condition, which the
  \var{try_putenv} function catches and clears.  Thus \var{try_putenv}
  is a function that returns \exmp{-1} upon failure and \var{0} upon
  success.
\seealso{_trace_function, _slangtrace, _traceback}
\done

\variable{_debug_info}
\synopsis{Configure debugging information}
\usage{Integer_Type _debug_info}
\description
  The \var{_debug_info} variable controls whether or not extra code
  should be generated for additional debugging and traceback
  information.  Currently, if \var{_debug_info} is zero, no extra code
  will be generated; otherwise extra code will be inserted into the
  compiled bytecode for additional debugging data.

  The value of this variable is local to each compilation unit and
  setting its value in one unit has no effect upon its value in other
  units.
\example
#v+
    _debug_info = 1;   % Enable debugging information
#v-
\notes
  Setting this variable to a non-zero value may slow down the
  interpreter somewhat.
\seealso{_traceback, _slangtrace}
\done

\variable{_slangtrace}
\synopsis{Turn function tracing on or off.}
\usage{Integer_Type _slangtrace}
\description
  The \var{_slangtrace} variable is a debugging aid that when set to a
  non-zero value enables tracing when function declared by
  \var{_trace_function} is entered.  If the value is greater than
  zero, both intrinsic and user defined functions will get traced.
  However, if set to a value less than zero, intrinsic functions will
  not get traced.
\seealso{_trace_function, _traceback, _print_stack}
\done

\function{_trace_function}
\synopsis{Set the function to trace}
\usage{_trace_function (String_Type f)}
\description
  \var{_trace_function} declares that the \slang function with name
  \var{f} is to be traced when it is called.  Calling
  \var{_trace_function} does not in itself turn tracing on.  Tracing
  is turned on only when the variable \var{_slangtrace} is non-zero.
\seealso{_slangtrace, _traceback}
\done

\variable{_traceback}
\synopsis{Generate a traceback upon error}
\usage{Integer_Type _traceback}
\description
  \var{_traceback} is an intrinsic integer variable whose value
  controls whether or not a traceback of the call stack is to be
  generated upon error.  If \var{_traceback} is greater than zero, a
  full traceback will be generated, which includes the values of local
  variables.  If the value is less than zero, a traceback will be
  generated without local variable information, and if
  \var{_traceback} is zero the traceback will not be generated.

  Local variables are represented in the form \var{$n} where \var{n} is an
  integer numbered from zero.  More explicitly, \var{$0} represents the
  first local variable, \var{$1} represents the second, and so on.
  Please note that function parameters are local variables and that the
  first parameter corresponds to \var{$0}.
\seealso{_slangtrace, error}
\done

