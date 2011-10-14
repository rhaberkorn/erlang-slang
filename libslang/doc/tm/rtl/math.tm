\function{Conj}
\synopsis{Compute the complex conjugate of a number}
\usage{z1 = Conj (z)}
\description
  The \var{Conj} function returns the complex conjugate of a number.
  If its argument is an array, the \var{Conj} function will be applied to each
  element and the result returned as an array.
\seealso{Real, Imag, abs}
\done

\function{Imag}
\synopsis{Compute the imaginary part of a number}
\usage{i = Imag (z)}
\description
  The \var{Imag} function returns the imaginary part of a number.
  If its argument is an array, the \var{Imag} function will be applied to each
  element and the result returned as an array.
\seealso{Real, Conj, abs}
\done

\function{Real}
\synopsis{Compute the real part of a number}
\usage{r = Real (z)}
\description
  The \var{Real} function returns the real part of a number. If its
  argument is an array, the \var{Real} function will be applied to
  each element and the result returned as an array.
\seealso{Imag, Conj, abs}
\done

\function{abs}
\synopsis{Compute the absolute value of a number}
\usage{y = abs(x)}
\description
  The \var{abs} function returns the absolute value of an arithmetic
  type.  If its argument is a complex number (\var{Complex_Type}),
  then it returns the modulus.  If the argument is an array, a new
  array will be created whose elements are obtained from the original
  array by using the \var{abs} function.
\seealso{sign, sqr}
\done

\function{acos}
\synopsis{Compute the arc-cosine of an number}
\usage{y = acos (x)}
\description
  The \var{acos} function computes the arc-cosine of a number and
  returns the result as an array.  If its argument is an array, the
  \var{acos} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{acosh}
\synopsis{Compute the inverse cosh of an number}
\usage{y = acosh (x)}
\description
  The \var{acosh} function computes the inverse cosh of a number and
  returns the result as an array.  If its argument is an array, the
  \var{acosh} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{asin}
\synopsis{Compute the arc-sine of an number}
\usage{y = asin (x)}
\description
  The \var{asin} function computes the arc-sine of a number and
  returns the result as an array.  If its argument is an array, the
  \var{asin} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{asinh}
\synopsis{Compute the inverse-sinh of an number}
\usage{y = asinh (x)}
\description
  The \var{asinh} function computes the inverse-sinh of a number and
  returns the result as an array.  If its argument is an array, the
  \var{asinh} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{atan}
\synopsis{Compute the arc-tangent of an number}
\usage{y = atan (x)}
\description
  The \var{atan} function computes the arc-tangent of a number and
  returns the result as an array.  If its argument is an array, the
  \var{atan} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{atanh}
\synopsis{Compute the inverse-tanh of an number}
\usage{y = atanh (x)}
\description
  The \var{atanh} function computes the inverse-tanh of a number and
  returns the result as an array.  If its argument is an array, the
  \var{atanh} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{cos}
\synopsis{Compute the cosine of an number}
\usage{y = cos (x)}
\description
  The \var{cos} function computes the cosine of a number and
  returns the result as an array.  If its argument is an array, the
  \var{cos} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{cosh}
\synopsis{Compute the hyperbolic cosine of an number}
\usage{y = cosh (x)}
\description
  The \var{cosh} function computes the hyperbolic cosine of a number and
  returns the result as an array.  If its argument is an array, the
  \var{cosh} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{exp}
\synopsis{Compute the exponential of an number}
\usage{y = exp (x)}
\description
  The \var{exp} function computes the exponential of a number and
  returns the result as an array.  If its argument is an array, the
  \var{exp} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{log}
\synopsis{Compute the logarithm of an number}
\usage{y = log (x)}
\description
  The \var{log} function computes the logarithm of a number and
  returns the result as an array.  If its argument is an array, the
  \var{log} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{log10}
\synopsis{Compute the base-10 logarithm of an number}
\usage{y = log10 (x)}
\description
  The \var{log10} function computes the base-10 logarithm of a number and
  returns the result as an array.  If its argument is an array, the
  \var{log10} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{mul2}
\synopsis{Multiply a number by 2}
\usage{y = mul2(x)}
\description
  The \var{mul2} function multiplies an arithmetic type by two and
  returns the result.  If its argument is an array, a new array will
  be created whose elements are obtained from the original array by
  using the \var{mul2} function.
\seealso{sqr, abs}
\done

\function{polynom}
\synopsis{Evaluate a polynomial}
\usage{Double_Type polynom(Double_Type a, b, ...c, Integer_Type n, Double_Type x)}
\description
  The \var{polynom} function returns the value of the polynomial expression:
#v+
     ax^n + bx^(n - 1) + ... c
#v-
\notes
  The \var{polynom} function should be extended to work with complex
  and array data types.  The current implementation is limited to
  \var{Double_Type} quantities.
\seealso{exp}
\done

\function{set_float_format}
\synopsis{Set the format for printing floating point values.}
\usage{set_float_format (String_Type fmt)}
\description
  The \var{set_float_format} function is used to set the floating
  point format to be used when floating point numbers are printed.
  The routines that use this are the traceback routines and the
  \var{string} function. The default value is \exmp{"%f"}
\example
#v+
     s = string (PI);                %  --> s = "3.14159"
     set_float_format ("%16.10f");
     s = string (PI);                %  --> s = "3.1415926536"
     set_float_format ("%10.6e");
     s = string (PI);                %  --> s = "3.141593e+00"
#v-
\seealso{string, sprintf, double}
\done

\function{sign}
\synopsis{Compute the sign of a number}
\usage{y = sign(x)}
\description
  The \var{sign} function returns the sign of an arithmetic type.  If
  its argument is a complex number (\var{Complex_Type}), it returns
  the sign of the imaginary part of the number. If the argument is an
  array, a new array will be created whose elements are obtained from
  the original array by using the \var{sign} function.
  
  When applied to a real number or an integer, the \var{sign} function
  returns \-1, \0, or \exmp{+1} according to whether the number is
  less than zero, equal to zero, or greater than zero, respectively.
\seealso{abs}
\done

\function{sin}
\synopsis{Compute the sine of an number}
\usage{y = sin (x)}
\description
  The \var{sin} function computes the sine of a number and
  returns the result as an array.  If its argument is an array, the
  \var{sin} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{sinh}
\synopsis{Compute the hyperbolic sine of an number}
\usage{y = sinh (x)}
\description
  The \var{sinh} function computes the hyperbolic sine of a number and
  returns the result as an array.  If its argument is an array, the
  \var{sinh} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{sqr}
\synopsis{Compute the square of a number}
\usage{y = sqr(x)}
\description
  The \var{sqr} function returns the square of an arithmetic type.  If its
  argument is a complex number (\var{Complex_Type}), then it returns
  the square of the modulus.  If the argument is an array, a new array
  will be created whose elements are obtained from the original array
  by using the \var{sqr} function.
\seealso{abs, mul2}
\done

\function{sqrt}
\synopsis{Compute the square root of an number}
\usage{y = sqrt (x)}
\description
  The \var{sqrt} function computes the square root of a number and
  returns the result as an array.  If its argument is an array, the
  \var{sqrt} function will be applied to each element and the result returned
  as an array.
\seealso{sqr, cos, atan, acosh, cosh}
\done

\function{tan}
\synopsis{Compute the tangent of an number}
\usage{y = tan (x)}
\description
  The \var{tan} function computes the tangent of a number and
  returns the result as an array.  If its argument is an array, the
  \var{tan} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

\function{tanh}
\synopsis{Compute the hyperbolic tangent of an number}
\usage{y = tanh (x)}
\description
  The \var{tanh} function computes the hyperbolic tangent of a number and
  returns the result as an array.  If its argument is an array, the
  \var{tanh} function will be applied to each element and the result returned
  as an array.
\seealso{cos, atan, acosh, cosh}
\done

