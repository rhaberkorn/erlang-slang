\function{__class_id}
\synopsis{Return the class-id of a specified type}
\usage{Int_Type __class_id (DataType_Type type))}
\description
  This function returns the internal class-id of a specified data type.
\seealso{typeof, _typeof, __class_type}
\done

\function{__class_type}
\synopsis{Return the class-type of a specified type}
\usage{Int_Type __class_type (DataType_Type type))}
\description
  Internally \slang objects are classified according to four types:
  scalar, vector, pointer, and memory managed types.  For example, an
  integer is implemented as a scalar, a complex number as a vector,
  and a string is represented as a pointer.  The \var{__class_type}
  function returns an integer representing the class-type associated
  with the specified data type. Specifically, it returns:
#v+
       0    memory-managed
       1    scalar
       2    vector
       3    pointer
#v-
\seealso{typeof, _typeof, __class_id}
\done

\function{__eqs}
\synopsis{Test for equality between two objects}
\usage{Int_Type __eqs (a, b)}
\description
  This function tests its two arguments for equalit and returns \1
  if they are equal, and \0 otherwise.  To be equal, the data type of
  the arguments must match and the values of the objects must
  reference the same underlying object.
\example
   __eqs (1, 1)         ===> 1
   __eqs (1, 1.0)       ===> 0
   __eqs ("a", 1)       ===> 0
   __eqs ([1,2], [1,2]) ===> 0
\seealso{typeof, __get_reference}
\notes
   This function should be thought of as a test for "sameness".
\done

\function{__get_reference}
\synopsis{Get a reference to a global object}
\usage{Ref_Type __get_reference (String_Type nm)}
\description
  This function returns a reference to a global variable or function
  whose name is specified by \var{nm}.  If no such object exists, it
  returns \var{NULL}, otherwise it returns a reference.
\example
   For example, consider the function:
#v+
    define runhooks (hook)
    {
       variable f;
       f = __get_reference (hook);
       if (f != NULL)
         @f ();
    }
#v-
   This function could be called from another \slang function to
   allow customization of that function, e.g., if the function
   represents a mode, the hook could be called to setup keybindings
   for the mode.
\seealso{is_defined, typeof, eval, autoload, __is_initialized, __uninitialize}
\done

\function{__uninitialize}
\synopsis{Uninitialize a variable}
\usage{__uninitialize (Ref_Type x)}
\description
  The \var{__uninitialize} function may be used to uninitialize the
  variable referenced by the parameter \var{x}.
\example
  The following two lines are equivalent:
#v+
     () = __tmp(z);
     __uninitialize (&z);
#v-
\seealso{__tmp, __is_initialized}
\done

\variable{_auto_declare}
\synopsis{Set automatic variable declaration mode}
\usage{Integer_Type _auto_declare}
\description
  The \var{_auto_declare} may be used to have all undefined variables
  implicitely declared as \var{static}.  If set to zero, any variable
  must be declared witha \var{variable} declaration before it can be
  used.  If set to one, then any undeclared variabled will be declared
  as a \var{static} global variable.

  The \var{_auto_declare} variable is local to each compilation unit and
  setting its value in one unit has no effect upon its value in other
  units.   The value of this variable has no effect upon the variables
  in a function.
\example
  The following code will not compile if \var{X} not been
  declared:
#v+
    X = 1;
#v-
  However, 
#v+
    _auto_declare = 1;   % declare variables as static.
    X = 1;
#v-
  is equivalent to 
#v+
    static variable X = 1;
#v-
\notes
  This variable should be used sparingly and is intended primarily for
  interactive applications where one types \slang commands at a prompt.
\done

\function{current_namespace}
\synopsis{Get the name of the current namespace}
\usage{String_Type current_namespace ()}
\description
   The \var{current_namespace} function returns the name of the
   current namespace.  If the current namespace is anonymous, that is,
   has not been given a name via the \var{implements} function, the
   empty string \exmp{""} will be returned.
\seealso{implements, use_namespace, import}
\done

\function{getenv}
\synopsis{Get the value of an environment variable}
\usage{String_Type getenv(String_Type var)}
\description
   The \var{getenv} function returns a string that represents the
   value of an environment variable \var{var}.  It will return
   \var{NULL} if there is no environment variable whose name is given
   by \var{var}.
\example
#v+
    if (NULL != getenv ("USE_COLOR"))
      {
        set_color ("normal", "white", "blue");
        set_color ("status", "black", "gray");
        USE_ANSI_COLORS = 1;
      }
#v-
\seealso{putenv, strlen, is_defined}
\done

\function{implements}
\synopsis{Name a private namespace}
\usage{implements (String_Type name);}
\description
  The \var{implements} function may be used to name the private
  namespace associated with the current compilation unit.  Doing so
  will enable access to the members of the namespace from outside the
  unit.  The name of the global namespace is \exmp{Global}.
\example
  Suppose that some file \exmp{t.sl} contains:
#v+
     implements ("Ts_Private");
     static define message (x)
     {
        Global->vmessage ("Ts_Private message: %s", x);
     }
     message ("hello");
#v-
  will produce \exmp{"Ts_Private message: hello"}.  This \var{message}
  function may be accessed from outside via:
#v+
    Ts_Private->message ("hi");
#v-
\notes
  Since \var{message} is an intrinsic function, it is global and may
  not be redefined in the global namespace.
\seealso{use_namespace, current_namespace, import}
\done

\function{putenv}
\synopsis{Add or change an environment variable}
\usage{putenv (String_Type s)}
\description
    This functions adds string \var{s} to the environment.  Typically,
    \var{s} should of the form \var{"name=value"}.  The function
    signals a \slang error upon failure.
\notes
    This function is not available on all systems.
\seealso{getenv, sprintf}
\done

\function{use_namespace}
\synopsis{Change to another namespace}
\usage{use_namespace (String_Type name)}
\description
   The \var{use_namespace} function changes the current namespace to
   the one specified by the parameter.  If the specified namespace
   does not exist, an error will be generated.
\seealso{implements, current_namespace, import}
\done

