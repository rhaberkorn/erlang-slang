\function{error}
\synopsis{Generate an error condition}
\usage{error (String_Type msg}
\description
  The \var{error} function generates a \slang error condition causing
  the interpreter to start unwinding to top-level.  It takes a single
  string parameter which is displayed on the stderr output device.
  The error condition may be cleared via an \var{ERROR_BLOCK} with the
  \var{_clear_error} function.  Consult \user-manual for more
  information.
\example
#v+
    define add_txt_extension (file)
    {
       if (typeof (file) != String_Type)
         error ("add_extension: parameter must be a string");
       file += ".txt";
       return file;
    }
#v-
\seealso{verror, _clear_error, message}
\done

\function{message}
\synopsis{Print a string onto the message device}
\usage{message (String_Type s}
\description
  The \var{message} function will print the string specified by
  \var{s} onto the message device.
\example
#v+
     define print_current_time ()
     {
       message (time ());
     }
#v-
\notes
  The message device will depend upon the application.  For example,
  the output message device for the \var{jed} editor correspond to the
  line at the bottom of the display window.  The default message
  device is the standard output device.
\seealso{vmessage, sprintf, error}
\done

\function{usage}
\synopsis{Generate a usage error}
\usage{usage (String_Type msg)}
\description
  The \var{usage} function generates a usage exception and displays
  \var{msg} to the message device.
\example
  Suppose that some function \var{plot} plots an array of \var{x} and
  \var{y} values.  The such a function could be written to issue a
  usage message if the wrong number of arguments were passed:
#v+
    define plot ()
    { 
       variable x, y;

       if (_NARGS != 2)
         usage ("plot (x, y)");
       
       (x, y) = ();
       % Now do the hard part
          .
	  .
    }
#v-
\seealso{error, message}
\done

\function{verror}
\synopsis{Generate an error condition}
\usage{verror (String_Type fmt, ...)}
\description
  The \var{verror} function performs the same role as the \var{error}
  function.  The only difference is that instead of a single string
  argument, \var{verror} takes a sprintf style argument list.
\example
#v+
    define open_file (file)
    {
       variable fp;

       fp = fopen (file, "r");
       if (fp == NULL) verror ("Unable to open %s", file);
       return fp;
    }
#v-
\notes
  In the current implementation, strictly speaking, the \var{verror}
  function is not an intrinsic function.  Rather it is a predefined
  \slang function using a combination of \var{Sprintf} and
  \var{error}.
\seealso{error, Sprintf, vmessage}
\done

\function{vmessage}
\synopsis{Print a formatted string onto the message device}
\usage{vmessage (String_Type fmt, ...)}
\description
  The \var{vmessage} function formats a sprintf style argument list
  and displays the resulting string onto the message device.
\notes
  In the current implementation, strictly speaking, the \var{vmessage}
  function is not an intrinsic function.  Rather it is a predefined
  \slang function using a combination of \var{Sprintf} and
  \var{message}.
\seealso{message, Sprintf, verror}
\done

