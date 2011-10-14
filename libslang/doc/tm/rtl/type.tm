\function{_slang_guess_type}
\synopsis{Guess the data type that a string represents.}
\usage{DataType_Type _slang_guess_type (String_Type s)}
\description
  This function tries to determine whether its argument \var{s}
  represents an integer (short, int, long), floating point (float,
  double), or a complex number.  If it appears to be none of these,
  then a string is assumed.  It returns one of the following values
  depending on the format of the string \var{s}:
#v+
    Short_Type     :  short integer           (e.g., "2h")
    UShort_Type    :  unsigned short integer  (e.g., "2hu")
    Integer_Type   :  integer                 (e.g., "2")
    UInteger_Type  :  unsigned integer        (e.g., "2")
    Long_Type      :  long integer            (e.g., "2l")
    ULong_Type     :  unsigned long integer   (e.g., "2l")
    Float_Type     :  float                   (e.g., "2.0f")
    Double_Type    :  double                  (e.g., "2.0")
    Complex_Type   :  imaginary               (e.g., "2i")
    String_Type    :  Anything else.          (e.g., "2foo")
#v-
  For example, \exmp{_slang_guess_type("1e2")} returns
  \var{Double_Type} but \exmp{_slang_guess_type("e12")} returns
  \var{String_Type}.
\seealso{integer, string, double, atof}
\done

\function{_typeof}
\synopsis{Get the data type of an object}
\usage{DataType_Type _typeof (x)}
\description
  This function is similar to the \var{typeof} function except in the
  case of arrays.  If the object \exmp{x} is an array, then the data
  type of the array will be returned. otherwise \var{_typeof} returns
  the data type of \var{x}. 
\example
#v+
  if (Integer_Type == _typeof (x)) 
    message ("x is an integer or an integer array");
#v-
\seealso{typeof, array_info, _slang_guess_type, typecast}
\done

\function{atof}
\synopsis{Convert a string to a double precision number}
\usage{Double_Type atof (String_Type s)}
\description
  This function converts a string \var{s} to a double precision value
  and returns the result.  It performs no error checking on the format
  of the string.  The function \var{_slang_guess_type} may be used to
  check the syntax of the string.
\example
#v+
     define error_checked_atof (s)
     {
        switch (_slang_guess_type (s))
        {
           case Double_Type:
             return atof (s);
        }
        {
           case Integer_Type:
             return double (integer (s));
        }

        verror ("%s is not a double", s);
    }
#v-
\seealso{typecast, double, _slang_guess_type}
\done

\function{char}
\synopsis{Convert an ascii value into a string}
\usage{String_Type char (Integer_Type c)}
\description
  The \var{char} function converts an integer ascii value \var{c} to a string
  of unit length such that the first character of the string is \var{c}.
  For example, \exmp{char('a')} returns the string \exmp{"a"}.
\seealso{integer, string, typedef}
\done

\function{define_case}
\synopsis{Define upper-lower case conversion.}
\usage{define_case (Integer_Type ch_up, Integer_Type ch_low);}
\description
  This function defines an upper and lowercase relationship between two
  characters specified by the arguments.  This relationship is used by
  routines which perform uppercase and lowercase conversions.
  The first integer \var{ch_up} is the ascii value of the uppercase character
  and the second parameter \var{ch_low} is the ascii value of its
  lowercase counterpart.
\seealso{strlow, strup}
\done

\function{double}
\synopsis{Convert an object to double precision}
\usage{result = double (x)}
\description
  The \var{double} function typecasts an object \var{x} to double
  precision.  For example, if \var{x} is an array of integers, an
  array of double types will be returned.  If an object cannot be
  converted to \var{Double_Type}, a type-mismatch error will result.
\notes
  The \var{double} function is equivalent to the typecast operation
#v+
     typecast (x, Double_Type)
#v-
  To convert a string to a double precision number, use the \var{atof}
  function.
\seealso{typecast, atof, int}
\done

\function{int}
\synopsis{Typecast an object to an integer}
\usage{int (s)}
\description
  This function performs a typecast of \var{s} from its data type to
  an object of \var{Integer_Type}.  If \var{s} is a string, it returns
  returns the ascii value of the first character of the string
  \var{s}.  If \var{s} is \var{Double_Type}, \var{int} truncates the
  number to an integer and returns it.
\example
  \var{int} can be used to convert single character strings to
  integers.  As an example, the intrinsic function \var{isdigit} may
  be defined as
#v+
    define isdigit (s)
    {
      if ((int (s) >= '0') and (int (s) <= '9')) return 1;
      return 0;
    }
#v-
\notes
  This function is equalent to \exmp{typecast (s, Integer_Type)};
\seealso{typecast, double, integer, char, isdigit}
\done

\function{integer}
\synopsis{Convert a string to an integer}
\usage{Integer_Type integer (String_Type s)}
\description
  The \var{integer} function converts a string representation of an
  integer back to an integer.  If the string does not form a valid
  integer, a type-mismatch error will be generated.
\example
  \exmp{integer ("1234")} returns the integer value \exmp{1234}.
\notes
  This function operates only on strings and is not the same as the
  more general \var{typecast} operator.
\seealso{typecast, _slang_guess_type, string, sprintf, char}
\done

\function{isdigit}
\synopsis{Tests for a decimal digit character}
\usage{Integer_Type isdigit (String_Type s)}
\description
  This function returns a non-zero value if the first character in the
  string \var{s} is a digit; otherwise, it returns zero.
\example
  A simple, user defined implementation of \var{isdigit} is
#v+
    define isdigit (s)
    {
       return ((s[0] <= '9') and (s[0]  >= '0'));
    }
#v-
  However, the intrinsic function \var{isdigit} executes many times faster
  than the equivalent representation defined above.
\notes
  Unlike the C function with the same name, the \slang function takes
  a string argument.
\seealso{int, integer}
\done

\function{string}
\synopsis{Convert an object to a string representation.}
\usage{Integer_Type string (obj)}
\description
   The \var{string} function may be used to convert an object
   \var{obj} of any type to a string representation.
   For example, \exmp{string(12.34)} returns \exmp{"12.34"}.
\example
#v+
     define print_anything (anything)
     {
        message (string (anything));
     }
#v-
\notes
   This function is \em{not} the same as typecasting to a \var{String_Type}
   using the \var{typecast} function.
\seealso{typecast, sprintf, integer, char}
\done

\function{tolower}
\synopsis{Convert a character to lowercase.}
\usage{Integer_Type lower (Integer_Type ch)}
\description
  This function takes an integer \var{ch} and returns its lowercase
  equivalent.
\seealso{toupper, strup, strlow, int, char, define_case}
\done

\function{toupper}
\synopsis{Convert a character to uppercase.}
\usage{Integer_Type toupper (Integer_Type ch)}
\description
  This function takes an integer \var{ch} and returns its uppercase
  equivalent.
\seealso{tolower, strup, strlow, int, char, define_case}
\done

\function{typecast}
\synopsis{Convert an object from one data type to another.}
\usage{typecast (x, new_type)}
\description
  The \var{typecast} function performs a generic typecast operation on
  \var{x} to convert it to \var{new_type}.  If \var{x} represents an
  array, the function will attempt to convert all elements of \var{x}
  to \var{new_type}.  Not all objects can be converted and a
  type-mismatch error will result upon failure.
\example
#v+
    define to_complex (x)
    {
       return typecast (x, Complex_Type);
    }
#v-
  defines a function that converts its argument, \var{x} to a complex
  number.
\seealso{int, double, typeof}
\done

\function{typeof}
\synopsis{Get the data type of an object.}
\usage{DataType_Type typeof (x)}
\description
  This function returns the data type of \var{x}.
\example
#v+
  if (Integer_Type == typeof (x)) message ("x is an integer");
#v-
\seealso{_typeof, is_struct_type, array_info, _slang_guess_type, typecast}
\done

