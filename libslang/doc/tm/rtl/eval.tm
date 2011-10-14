\function{autoload}
\synopsis{Load a function from a file}
\usage{autoload (String_Type funct, String_Type file)}
\description
    The \var{autoload} function is used to declare \var{funct} to the
    interpreter and indicate that it should be loaded from \var{file} when
    it is actually used.
\example
    Suppose \var{bessel_j0} is a function defined in the file
    \var{bessel.sl}.  Then the statement
#v+
      autoload ("bessel_j0", "bessel.sl");
#v-
    will cause \var{bessel.sl} to be loaded prior to the execution of
    \var{bessel_j0}
\seealso{evalfile}
\done

\function{byte_compile_file}
\synopsis{Compile a file to byte-code for faster loading.}
\usage{byte_compile_file (String_Type file, Integer_Type method)}
\description
  The \var{byte_compile_file} function byte-compiles \var{file}
  producing a new file with the same name except a \var{'c'} is added
  to the output file name.  For example, \var{file} is
  \exmp{"site.sl"}, then the function produces a new file named
  \exmp{site.slc}.
\notes
  The \var{method} parameter is not used in the current
  implementation.  Its use is reserved for the future.  For now, set
  it to \exmp{0}.
\seealso{evalfile}
\done

\function{eval}
\synopsis{Interpret a string as \slang code}
\usage{eval (String_Type expression, [,String_Type namespace])}
\description
  The \var{eval} function parses a string as S-Lang code and executes the
  result.  If called with the optional namespace argument, then the
  string will be evaluated in the specified namespace.

  This is a useful function in many contexts such as dynamically
  generating function definitions where there is no way to generate
  them otherwise.
\example
#v+
    if (0 == is_defined ("my_function"))
      eval ("define my_function () { message (\"my_function\"); }");
#v-
\seealso{is_defined, autoload, evalfile}
\done

\function{evalfile}
\synopsis{Interpret a file containing \slang code.}
\usage{Integer_Type evalfile (String_Type file, [,String_Type namespace])}
\description
  The \var{evalfile} function loads \var{file} into the interpreter
  and executes it.  If called with the optional namespace argument,
  the file will be loaded into the specified namespace, which will be
  created if necessary.  If no errors were encountered, \exmp{1} will
  be returned; otherwise, a \slang error will be generated and the
  function will return zero.
\example
#v+
    define load_file (file)
    {
       ERROR_BLOCK { _clear_error (); }
       () = evalfile (file);
    }
#v-
\notes
  For historical reasons, the return value of this function is not
  really useful.
  
  The file is searched along an application-defined load-path.  The
  \ifun{get_slang_load_path} and \ifun{set_slang_load_path} functions
  may be used to set and query the path.
\seealso{eval, autoload, set_slang_load_path, get_slang_load_path}
\done

\function{get_slang_load_path}
\synopsis{Get the value of the interpreter's load-path}
\usage{String_Type get_slang_load_path ()}
\description
  This function retrieves the value of the delimiter-separated search
  path used for loading files.
\notes
  Some applications may not support the built-in load-path searching
  facility provided by the underlying library.
\seealso{}
\done

\function{set_slang_load_path}
\synopsis{Set the value of the interpreter's load-path}
\usage{set_slang_load_path (String_Type path)}
\description
  This function may be used to set the value of the
  delimiter-separated search path used by the \ifun{evalfile} and
  \ifun{autoload} functions for locating files.
\example
#v+
    public define prepend_to_slang_load_path (p)
    {
       variable s = stat_file (p);
       if (s == NULL) return;
       if (0 == stat_is ("dir", s.st_mode))
         return;

       variable d = path_get_delimiter ();
       set_slang_load_path (strcat (p, d, get_slang_load_path ()));
    }
#v-
\notes
  Some applications may not support the built-in load-path searching
  facility provided by the underlying library.
\seealso{get_slang_load_path, path_get_delimiter, evalfile, autoload}
\done

