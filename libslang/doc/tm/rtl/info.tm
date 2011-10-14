\variable{_NARGS}
\synopsis{The number of parameters passed to a function}
\usage{Integer_Type _NARGS}
   The value of the \var{_NARGS} variable represents the number of
   arguments passed to the function.  This variable is local to each
   function.
\example
   This example uses the \var{_NARGS} variable to print the list of
   values passed to the function:
#v+
     define print_values ()
     {
        variable arg;
        
        if (_NARGS == 0)
          {
             message ("Nothing to print");
             return;
          }
        foreach (__pop_args (_NARGS))
          {
             arg = ();
             vmessage ("Argument value is: %S", arg.value);
          }
     }
#v-
\seealso{__pop_args, __push_args, typeof}
\done

\function{__get_defined_symbols}
\synopsis{Get the symbols defined by the preprocessor}
\usage{Integer_Type __get_defined_symbols ()}
\description
  The \var{__get_defined_symbols} functions is used to get the list of
  all the symbols defined by the \slang preprocessor.  It pushes each
  of the symbols on the stack followed by the number of items pushed.
\seealso{is_defined, _apropos, _get_namespaces}
\done

\function{__is_initialized}
\synopsis{Determine whether or not a variable has a value}
\usage{Integer_Type __is_initialized (Ref_Type r)}
\description
   This function returns non-zero of the object referenced by \var{r}
   is initialized, i.e., whether it has a value.  It returns \0 if the
   referenced object has not been initialized.
\example
   For example, the function:
#v+
    define zero ()
    {
       variable f;
       return __is_initialized (&f);
    }
#v-
  will always return zero, but
#v+
    define one ()
    {
       variable f = 0;
       return __is_initialized (&f);
    }
#v-
  will return one.
\notes
  It is easy to see why a reference to the variable must be passed to
  \var{__is_initialized} and not the variable itself; otherwise, the
  value of the variable would be passed and the variable may have no
  value if it was not initialized.
\seealso{__get_reference, __uninitialize, is_defined, typeof, eval}
\done

\function{_apropos}
\synopsis{Generate a list of functions and variables}
\usage{Array_Type _apropos (String_Type ns, String_Type s, Integer_Type flags)}
\description
  The \var{_apropos} function may be used to get a list of all defined
  objects in the namespace \var{ns} whose name matches the regular
  expression \var{s} and whose type matches those specified by
  \var{flags}.  It returns an array of strings representing the
  matches. 

  The second parameter \var{flags} is a bit mapped value whose bits
  are defined according to the following table
#v+
     1          Intrinsic Function
     2          User-defined Function
     4          Intrinsic Variable
     8          User-defined Variable
#v-
\example
#v+
    define apropos (s)
    {
      variable n, name, a;
      a = _apropos ("Global", s, 0xF);
      
      vmessage ("Found %d matches:", length (a));
      foreach (a)
        {
           name = ();
           message (name);
        }
    }
#v-
  prints a list of all matches.
\notes
  If the namespace specifier \var{ns} is the empty string \exmp{""},
  then the namespace will default to the static namespace of the
  current compilation unit.
\seealso{is_defined, sprintf, _get_namespaces}
\done

\function{_function_name}
\synopsis{Returns the name of the currently executing function}
\usage{String_Type _function_name ()}
\description
  This function returns the name of the currently executing function.
  If called from top-level, it returns the empty string.
\seealso{_trace_function, is_defined}
\done

\function{_get_namespaces}
\synopsis{Returns a list of namespace names}
\usage{String_Type[] _get_namespaces ()}
\description
  This function returns a string array containing the names of the
  currently defined namespaces.
\seealso{_apropos, use_namespace, implements, __get_defined_symbols}
\done

\variable{_slang_doc_dir}
\synopsis{Installed documentation directory}
\usage{String_Type _slang_doc_dir;}
\description
   The \var{_slang_doc_dir} variable is a read-only whose value
   specifies the installation location of the \slang documentation.
\seealso{get_doc_string_from_file}
\done

\variable{_slang_version}
\synopsis{The S-Lang library version number}
\usage{Integer_Type _slang_version}
\description
   The \var{_slang_version} variable is read-only and whose
   value represents the number of the \slang library.
\seealso{_slang_version_string}
\done

\variable{_slang_version_string}
\synopsis{The S-Lang library version number as a string}
\usage{String_Type _slang_version_string}
\description
   The \var{_slang_version_string} variable is read-only and whose
   value represents the version number of the \slang library.
\seealso{_slang_version}
\done

\function{get_doc_string_from_file}
\synopsis{Read documentation from a file}
\usage{String_Type get_doc_string_from_file (String_Type f, String_Type t)}
\description
  \var{get_doc_string_from_file} opens the documentation file \var{f}
  and searches it for topic \var{t}.  It returns the documentation for
  \var{t} upon success, otherwise it returns \var{NULL} upon error.
  It will fail if \var{f} could not be opened or does not contain
  documentation for the topic.
\seealso{stat_file}
\seealso{_slang_doc_dir}
\done

\function{is_defined}
\synopsis{Indicate whether a variable or function defined.}
\usage{Integer_Type is_defined (String_Type obj)}
\description
   This function is used to determine whether or not a function or
   variable whose name is \var{obj} has been defined.  If \var{obj} is not
   defined, the function returns 0.  Otherwise, it returns a non-zero
   value that defpends on the type of object \var{obj} represents.
   Specifically, it returns one of the following values:
#v+
     +1 if an intrinsic function
     +2 if user defined function
     -1 if intrinsic variable
     -2 if user defined variable
      0 if undefined
#v-
\example
    For example, consider the function:
#v+
    define runhooks (hook)
    {
       if (2 == is_defined(hook)) eval(hook);
    }
#v-
    This function could be called from another \slang function to
    allow customization of that function, e.g., if the function
    represents a mode, the hook could be called to setup keybindings
    for the mode.
\seealso{typeof, eval, autoload, __get_reference, __is_initialized}
\done

