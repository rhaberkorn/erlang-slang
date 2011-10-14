#% -*- mode: tm; mode: fold -*-

#% text-macro definitions #%{{{
#i linuxdoc.tm

#d slang \bf{S-Lang}
#d slrn \bf{slrn}
#d jed \bf{jed}
#d kw#1 \tt{$1}
#d exmp#1 \tt{$1}
#d var#1 \tt{$1}
#d ldots ...
#d times *
#d math#1 $1
#d sc#1 \tt{$1}
#d verb#1 \tt{$1}
#d sldxe \bf{sldxe}
#d url#1 <htmlurl url="$1" name="$1">
#d slang-library-reference \bf{The \slang Library Reference}
#d chapter#1 <chapt>$1<p>
#d preface <preface>
#d tag#1 <tag>$1</tag>
#d appendix <appendix>

#d NULL <tt>NULL</tt>
#d kbd#1 <tt>$1</tt>

#d documentstyle book

#%}}}

\linuxdoc

\begin{\documentstyle}

\title A Guide to the S-Lang Language
\author John E. Davis, \tt{davis@space.mit.edu}
\date \__today__

\toc

#i preface.tm

\chapter{Introduction} #%{{{

   \slang is a powerful interpreted language that may be embedded into
   an application to make the application extensible.  This enables
   the application to be used in ways not envisioned by the programmer,
   thus providing the application with much more flexibility and
   power.  Examples of applications that take advantage of the
   interpreter in this way include the \jed editor and the \slrn
   newsreader.

\sect{Language Features}

   The language features both global and local variables, branching
   and looping constructs, user-defined functions, structures,
   datatypes, and arrays.  In addition, there is limited support for
   pointer types.  The concise array syntax rivals that of commercial
   array-based numerical computing environments.

\sect{Data Types and Operators} #%{{{

   The language provides built-in support for string, integer (signed
   and unsigned long and short), double precision floating point, and
   double precision complex numbers.  In addition, it supports user
   defined structure types, multi-dimensional array types, and
   associative arrays.  To facilitate the construction of
   sophisticated data structures such as linked lists and trees, a
   `reference' type was added to the language.  The reference type
   provides much of the same flexibility as pointers in other
   languages.  Finally, applications embedding the interpreter may
   also provide special application specific types, such as the
   \var{Mark_Type} that the \jed editor provides.

   The language provides standard arithmetic operations such as
   addition, subtraction, multiplication, and division.  It also
   provides support for modulo arithmetic as well as operations at
   the bit level, e.g., exclusive-or.  Any binary or unary operator
   may be extended to work with any data type.  For example, the
   addition operator (\var{+}) has been extended to work between
   string types to permit string concatenation.

   The binary and unary operators work transparently with array types.
   For example, if \var{a} and \var{b} are arrays, then \exmp{a + b}
   produces an array whose elements are the result of element by
   element addition of \var{a} and \var{b}.  This permits one to do
   vector operations without explicitly looping over the array
   indices.

#%}}}

\sect{Statements and Functions} #%{{{

   The \slang language supports several types of looping constructs and
   conditional statements.  The looping constructs include \kw{while},
   \kw{do...while}, \kw{for}, \kw{forever}, \kw{loop}, \kw{foreach},
   and \kw{_for}. The conditional statements include \kw{if},
   \kw{if-then-else}, and \kw{!if}.

   User defined functions may be defined to return zero, one, or more
   values.  Functions that return zero values are similar to
   `procedures' in languages such as PASCAL.  The local variables of a
   function are always created on a stack allowing one to create
   recursive functions.  Parameters to a function are always passed by
   value and never by reference. However, the language supports a
   \em{reference} data type that allows one to simulate pass by
   reference.

   Unlike many interpreted languages, \slang allows functions to be
   dynamically loaded (function autoloading).  It also provides
   constructs specifically designed for error handling and recovery as
   well as debugging aids (e.g., function tracebacks).

   Functions and variables may be declared as private belonging to a
   namespace associated with the compilation unit that defines the
   function or variable.  The ideas behind the namespace implementation
   stems from the C language and should be quite familiar to any one
   familiar with C.

#%}}}

\sect{Error Handling} #%{{{

   The \slang language defines a construct called an \em{error-block}
   that may be used for error handling and recovery.  When a non-fatal
   run-time error is encountered, any error blocks that have been
   defined are executed as the run-time stack unwinds.  An error block
   can optionally clear the error and the program will continue
   running after the statement that triggered the error.  This
   mechanism is somewhat similar to try-catch in C++.

#%}}}

\sect{Run-Time Library} #%{{{

   Functions that compose the \slang run-time library are called
   \em{intrinsics}.  Examples of \slang intrinsic functions available
   to every \slang application include string manipulation functions
   such as \var{strcat}, \var{strchop}, and \var{strcmp}.  The \slang
   library also provides mathematical functions such as \var{sin},
   \var{cos}, and \var{tan}; however, not all applications enable the
   use of these intrinsics.  For example, to conserve memory, the 16
   bit version of the \jed editor does not provide support for any
   mathematics other than simple integer arithmetic, whereas other
   versions of the editor do support these functions.

   Most applications embedding the languages will also provide a set of
   application specific intrinsic functions.  For example, the \jed
   editor adds over 100 application specific intrinsic functions to
   the language.  Consult your application specific documentation to
   see what additional intrinsics are supported.

#%}}}

\sect{Input/Output}

   The language supports C-like stdio input/output functions such as
   \var{fopen}, \var{fgets}, \var{fputs}, and \var{fclose}.  In
   addition it provides two functions, \var{message} and \var{error},
   for writing to the standard output device and standard error.
   Specific applications may provide other I/O mechanisms, e.g.,
   the \jed editor supports I/O to files via the editor's
   buffers.

\sect{Obtaining \slang} #%{{{

  Comprehensive information about the library may be obtained via the
  World Wide Web from \tt{http://www.s-lang.org}.

  \slang as well as some programs that embed it are freely available
  via anonymous ftp in the United States from
\begin{itemize}
  \item \url{ftp://space.mit.edu/pub/davis}.
\end{itemize}
  It is also available outside the United States from the following
  mirror sites:
\begin{itemize}
    \item \url{ftp://ftp.uni-stuttgart.de/pub/unix/misc/slang/}
    \item \url{ftp://ftp.fu-berlin.de/pub/unix/news/slrn/}
    \item \url{ftp://ftp.ntua.gr/pub/lang/slang/}
\end{itemize}

  The Usenet newsgroup \var{alt.lang.s-lang} was created for \slang
  programmers to exchange information and share macros for the various
  programs the embed the language.  The newsgroup \var{comp.editors}
  can be a useful resource for \slang macros for the \jed editor.
  Similarly, \slrn users will find \var{news.software.readers} to be a
  valuable source of information.
  
  Finally, two mailing lists dealing with the \slang library have been
  created: 
\begin{itemize}
     \item \tt{slang-announce@babayaga.math.fu-berlin.de}
     \item \tt{slang-workers@babayaga.math.fu-berlin.de}
\end{itemize}
  The first list is for announcements of new releases of the library, while the
  second list is intended for those who use the library for their own code
  development.  To subscribe to the announcement list, send an email to
  \tt{slang-announce-subscribe@babayaga.math.fu-berlin.de} and include
  the word \tt{subscribe} in the body of the message.  To subscribe to
  the developers list, use the address
  \tt{slang-workers-subscribe@babayaga.math.fu-berlin.de}.

#%}}}

#%}}}

\chapter{Overview of the Language} #%{{{

   This purpose of this section is to give the reader a feel for the
   \slang language, its syntax, and its capabilities.  The information
   and examples presented in this section should be sufficient to
   provide the reader with the necessary background to understand the
   rest of the document.

\sect{Variables and Functions} #%{{{

   \slang is different from many other interpreted languages in the
   sense that all variables and functions must be declared before they
   can be used.

   Variables are declared using the \kw{variable} keyword, e.g.,
#v+
     variable x, y, z;
#v-
   declares three variables, \var{x}, \var{y}, and \var{z}.  Note the
   semicolon at the end of the statement.  \em{All \slang statements must
   end in a semi-colon.}

   Unlike compiled languages such as C, it is not necessary to specify
   the data type of a \slang variable.  The data type of a \slang
   variable is determined upon assignment.  For example, after
   execution of the statements
#v+
     x = 3;
     y = sin (5.6);
     z = "I think, therefore I am.";
#v-
   \var{x} will be an integer, \var{y} will be a
   double, and \var{z} will be a string.  In fact, it is even possible
   to re-assign \var{x} to a string:
#v+
     x = "x was an integer, but now is a string";
#v-
   Finally, one can combine variable declarations and assignments in
   the same statement:
#v+
     variable x = 3, y = sin(5.6), z = "I think, therefore I am.";
#v-

   Most functions are declared using the \kw{define} keyword.  A
   simple example is
#v+
      define compute_average (x, y)
      {
         variable s = x + y;
         return s / 2.0;
      }
#v-
   which defines a function that simply computes the average of two
   numbers and returns the result.  This example shows that a function
   consists of three parts: the function name, a parameter list, and
   the function body.

   The parameter list consists of a comma separated list of variable
   names.  It is not necessary to declare variables within a parameter
   list; they are implicitly declared.  However, all other \em{local}
   variables used in the function must be declared.  If the function
   takes no parameters, then the parameter list must still be present,
   but empty:
#v+
      define go_left_5 ()
      {
         go_left (5);
      }
#v-
   The last example is a function that takes no arguments and returns
   no value.  Some languages such as PASCAL distinguish such objects
   from functions that return values by calling these objects
   \em{procedures}.  However, \slang, like C, does not make such a
   distinction.
   
   The language permits \em{recursive} functions, i.e., functions that
   call themselves.  The way to do this in \slang is to first declare
   the function using the form:
\begin{tscreen}
     define \em{function-name} ();
\end{tscreen}
   It is not necessary to declare a parameter list when declaring a
   function in this way.

   The most famous example of a recursive function is the factorial
   function.  Here is how to implement it using \slang:
#v+
     define factorial ();   % declare it for recursion
     
     define factorial (n)
     {
        if (n < 2) return 1;
        return n * factorial (n - 1);
     }
#v-
   This example also shows how to mix comments with code.  \slang uses
   the `\var{%}' character to start a comment and all characters from
   the comment character to the end of the line are ignored.

#%}}}

\sect{Strings} #%{{{

   Perhaps the most appealing feature of any interpreted language is
   that it frees the user from the responsibility of memory management.
   This is particularly evident when contrasting how
   \slang handles string variables with a lower level language such as
   C.  Consider a function that concatenates three strings.  An
   example in \slang is:
#v+
     define concat_3_strings (a, b, c)
     {
        return strcat (a, strcat (b, c));
     }
#v-
   This function uses the built-in
   \var{strcat} function for concatenating two strings.  In C, the
   simplest such function would look like:
#v+
     char *concat_3_strings (char *a, char *b, char *c)
     {
        unsigned int len;
        char *result;
        len = strlen (a) + strlen (b) + strlen (c);
        if (NULL == (result = (char *) malloc (len + 1)))
          exit (1);
        strcpy (result, a);
        strcat (result, b);
        strcat (result, c);
        return result;
     }
#v-
   Even this C example is misleading since none of the issues of memory
   management of the strings has been dealt with.  The \slang language
   hides all these issues from the user.

   Binary operators have been defined to work with the string data
   type.  In particular the \var{+} operator may be used to perform
   string concatenation.  That is, one can use the
   \var{+} operator as an alternative to \var{strcat}:
#v+
      define concat_3_strings (a, b, c)
      {
         return a + b + c;
      }
#v-
   See section ??? for more information about string variables.

#%}}}

\sect{Referencing and Dereferencing} #%{{{
   The unary prefix operator, \var{&}, may be used to create a
   \em{reference} to an object, which is similar to a pointer
   in other languages.  References are commonly used as a mechanism to
   pass a function as an argument to another function as the following
   example illustrates:
#v+
       define compute_functional_sum (funct)
       {
          variable i, s;

          s = 0;
          for (i = 0; i < 10; i++)
           {
              s += (@funct)(i);
           }
          return s;
       }
       
       variable sin_sum = compute_functional_sum (&sin);
       variable cos_sum = compute_functional_sum (&cos);
#v-
   Here, the function \var{compute_functional_sum} applies the
   function specified by the parameter \var{funct} to the first
   \exmp{10} integers and returns the sum.  The two statements
   following the function definition show how the \var{sin} and
   \var{cos} functions may be used.
   
   Note the \var{@} operator in the definition of
   \var{compute_functional_sum}.  It is known as the \em{dereference}
   operator and is the inverse of the reference operator.
   
   Another use of the reference operator is in the context of the
   \var{fgets} function.  For example,
#v+
      define read_nth_line (file, n)
      {
         variable fp, line;
         fp = fopen (file, "r");
           
         while (n > 0)
           {
              if (-1 == fgets (&line, fp))
                return NULL;
              n--;
           }
         return line;
      }
#v-
   uses the \var{fgets} function to read the nth line of a file.
   In particular, a reference to the local variable \var{line} is
   passed to \var{fgets}, and upon return \var{line} will be set to
   the character string read by \var{fgets}.
   
   Finally, references may be used as an alternative to multiple
   return values by passing information back via the parameter list.
   The example involving \var{fgets} presented above provided an
   illustration of this.  Another example is
#v+
       define set_xyz (x, y, z)
       {
          @x = 1;
          @y = 2;
          @z = 3;
       }
       variable X, Y, Z;
       set_xyz (&X, &Y, &Z);
#v-
   which, after execution, results in \var{X} set to \exmp{1}, \var{Y}
   set to \exmp{2}, and \var{Z} set to \exmp{3}.  A C programmer will
   note the similarity of \var{set_xyz} to the following C
   implementation:
#v+
      void set_xyz (int *x, int *y, int *z)
      {
         *x = 1;
         *y = 2;
         *z = 3;
      }
#v-
#%}}}

\sect{Arrays} #%{{{
   The \slang language supports multi-dimensional arrays of all
   datatypes.  For example, one can define arrays of references to
   functions as well as arrays of arrays.  Here are a few examples of
   creating arrays:
#v+
       variable A = Integer_Type [10];  
       variable B = Integer_Type [10, 3];
       variable C = [1, 3, 5, 7, 9];
#v-
   The first example creates an array of \var{10} integers and assigns
   it to the variable \var{A}.  The second example creates a 2-d array
   of \var{30} integers arranged in \var{10} rows and \var{3} columns
   and assigns the result to \var{B}.  In the last example, an array
   of \var{5} integers is assigned to the variable \var{C}.  However,
   in this case the elements of the array are initialized to the
   values specified.  This is known as an \em{inline-array}.
   
   \slang also supports something called an
   \em{range-array}.  An example of such an array is
#v+
      variable C = [1:9:2];
#v-
   This will produce an array of 5 integers running from \exmp{1}
   through \exmp{9} in increments of \exmp{2}.
   
   Arrays are passed by reference to functions and never by value.
   This permits one to write functions which can initialize arrays.
   For example,
#v+
      define init_array (a)
      {
         variable i, imax;
         
         imax = length (a);
         for (i = 0; i < imax; i++)
           { 
              a[i] = 7;
           }
      }
      
      variable A = Integer_Type [10];
      init_array (A);
#v-
   creates an array of \var{10} integers and initializes all its
   elements to \var{7}.  

   There are more concise ways of accomplishing the result of the
   previous example.  These include:
#v+
      variable A = [7, 7, 7, 7, 7, 7, 7, 7, 7, 7];
      variable A = Integer_Type [10];  A[[0:9]] = 7;
      variable A = Integer_Type [10];  A[*] = 7;
#v-      
   The second and third methods use an array of indices to index the array
   \var{A}.  In the second, the range of indices has been explicitly
   specified, whereas the third example uses a wildcard form.  See
   section ??? for more information about array indexing.

   Although the examples have pertained to integer arrays, the fact is
   that \slang arrays can be of any type, e.g.,
#v+
       variable A = Double_Type [10];
       variable B = Complex_Type [10];
       variable C = String_Type [10];
       variable D = Ref_Type [10];
#v-
   create \var{10} element arrays of double, complex, string, and
   reference types, respectively.  The last example may be used to
   create an array of functions, e.g.,
#v+
      D[0] = &sin;
      D[1] = &cos;
#v-

   The language also defines unary, binary, and mathematical
   operations on arrays.  For example, if \var{A} and \var{B} are
   integer arrays, then \exmp{A + B} is an array whose elements are
   the sum of the elements of \var{A} and \var{B}.  A trivial example
   that illustrates the power of this capability is
#v+
        variable X, Y;
        X = [0:2*PI:0.01];
        Y = 20 * sin (X);
#v-
   which is equivalent to the highly simplified C code:
#v+
        double *X, *Y;
        unsigned int i, n;
        
        n = (2 * PI) / 0.01 + 1;
        X = (double *) malloc (n * sizeof (double));
        Y = (double *) malloc (n * sizeof (double));
        for (i = 0; i < n; i++)
          {
            X[i] = i * 0.01;
            Y[i] = 20 * sin (X[i]);
          }
#v-


#%}}}

\sect{Structures and User-Defined Types} #%{{{

   A \em{structure} is similar to an array in the sense that it is a
   container object.  However, the elements of an array must all be of
   the same type (or of \var{Any_Type}), whereas a structure is
   heterogeneous.  As an example, consider
#v+
      variable person = struct 
      {
         first_name, last_name, age
      };
      variable bill = @person;
      bill.first_name = "Bill";
      bill.last_name = "Clinton";
      bill.age = 51;
#v-
   In this example a structure consisting of the three fields has been
   created and assigned to the variable \var{person}.  Then an
   \em{instance} of this structure has been created using the
   dereference operator and assigned to \var{bill}.  Finally, the
   individual fields of \var{bill} were initialized.  This is an
   example of an \em{anonymous} structure.
   
   A \em{named} structure is really a new data type and may be created
   using the \kw{typedef} keyword:
#v+
      typedef struct
      {
         first_name, last_name, age
      }
      Person_Type;
      
      variable bill = @Person_Type;
      bill.first_name = "Bill";
      bill.last_name = "Clinton";
      bill.age = 51;
#v-     
   The big advantage of creating a new type is that one can go on to
   create arrays of the data type
#v+
      variable People = Person_Type [100];
      People[0].first_name = "Bill";
      People[1].first_name = "Hillary";
#v-

   The creation and initialization of a structure may be facilitated
   by a function such as
#v+
      define create_person (first, last, age)
      {
          variable person = @Person_Type;
          person.first_name = first;
          person.last_name = last;
          person.age = age;
          return person;
      }
      variable Bill = create_person ("Bill", "Clinton", 51);
#v-
   
   Other common uses of structures is the creation of linked lists,
   binary trees, etc.  For more information about these and other
   features of structures, see section ???.


#%}}}

\sect{Namespaces}

   In addition to the global namespace, each compilation unit (e.g., a
   file) is given a private namespace.  A variable or function name
   that is declared using the \var{static} keyword will be placed in
   the private namespace associated with compilation unit.  For
   example, 
#v+
       variable i;
       static variable i;
#v-
   defines two variables called \var{i}.  The first declaration
   defines \var{i} in the global namespace, but the second declaration
   defines \var{i} in the private namespace.
   
   The \exmp{->} operator may be used in conjunction with the name of
   the namespace to access objects in the name space.  In the above
   example, to access the variable \var{i} in the global namespace,
   one would use \exmp{Global->i}.  Unless otherwise specified, a
   private namespace has no name and its objects may not be accessed
   from outside the compilation unit.  However, the \var{implements}
   function may be used give the private namespace a name, allowing
   access to its objects.  For example, if the file \exmp{t.sl} contains
#v+
      implements ("A");
      static variable i;
#v-
   then another file may access the variable \var{i} via \exmp{A->i}.
   
#%}}}

\chapter{Data Types and Literal Constants} #%{{{

   The current implementation of the \slang language permits up to 256
   distinct data types, including predefined data types such as integer and
   floating point, as well as specialized applications specific data
   types.  It is also possible to create new data types in the
   language using the \kw{typedef} mechanism.

   Literal constants are objects such as the integer \exmp{3} or the
   string \exmp{"hello"}.  The actual data type given to a literal
   constant depends upon the syntax of the constant.  The following
   sections describe the syntax of literals of specific data types.

\sect{Predefined Data Types} #%{{{

   The current version of \slang defines integer, floating point,
   complex, and string types. It also defines special purpose data
   types such as \var{Null_Type}, \var{DataType_Type}, and
   \var{Ref_Type}.  These types are discussed below.

\sect1{Integers} #%{{{

   The \slang language supports both signed and unsigned characters,
   short integer, long integer, and plain integer types. On most 32
   bit systems, there is no difference between an integer and a long
   integer; however, they may differ on 16 and 64 bit systems.
   Generally speaking, on a 16 bit system, plain integers are 16 bit
   quantities with a range of -32767 to 32767.  On a 32 bit system,
   plain integers range from -2147483648 to 2147483647.

   An plain integer \em{literal} can be specified in one of several ways:
\begin{itemize}
\item As a decimal (base 10) integer consisting of the characters
      \var{0} through \var{9}, e.g., \var{127}.  An integer specified
      this way cannot begin with a leading \var{0}.  That is,
      \var{0127} is \em{not} the same as \var{127}.

\item Using hexadecimal (base 16) notation consisting of the characters
      \var{0} to \var{9} and \var{A} through \var{F}.  The hexadecimal
      number must be preceded by the characters \var{0x}.  For example,
      \var{0x7F} specifies an integer using hexadecimal notation and has
      the same value as decimal \var{127}.

\item In Octal notation using characters \var{0} through \var{7}.  The Octal
      number must begin with a leading \var{0}.  For example,
      \var{0177} and \var{127} represent the same integer.

   Short, long, and unsigned types may be specified by using the
   proper suffixes: \var{L} indicates that the integer is a long
   integer, \var{h} indicates that the integer is a short integer, and
   \var{U} indicates that it is unsigned.  For example, \exmp{1UL}
   specifies an unsigned long integer.

   Finally, a character literal may be specified using a notation
   containing a character enclosed in single quotes as \exmp{'a'}.
   The value of the character specified this way will lie in the
   range 0 to 256 and will be determined by the ASCII value of the
   character in quotes.  For example,
#v+
              i = '0';
#v-
      assigns to \var{i} the character 48 since the \exmp{'0'} character 
      has an ASCII value of 48.
\end{itemize}

    Any integer may be preceded by a minus sign to indicate that it is a
    negative integer.

#%}}}

\sect1{Floating Point Numbers} #%{{{

    Single and double precision floating point literals must contain either a
    decimal point or an exponent (or both). Here are examples of
    specifying the same double precision point number:
#v+
         12.    12.0    12e0   1.2e1   120e-1   .12e2   0.12e2
#v-
    Note that \var{12} is \em{not} a floating point number since it
    contains neither a decimal point nor an exponent.  In fact,
    \var{12} is an integer.

    One may append the \var{f} character to the end of the number to
    indicate that the number is a single precision literal.

#%}}}

\sect1{Complex Numbers} #%{{{

    The language implements complex numbers as a pair of double
    precision floating point numbers.  The first number in the pair
    forms the \em{real} part, while the second number forms the
    \em{imaginary} part.  That is, a complex number may be regarded as the
    sum of a real number and an imaginary number.
    
    Strictly speaking, the current implementation of the \slang does
    not support generic complex literals.  However, it does support
    imaginary literals and a more generic complex number with a non-zero
    real part may be constructed from the imaginary literal via
    addition of a real number.

    An imaginary literal is specified in the same way as a floating
    point literal except that \var{i} or \var{j} is appended.  For
    example,
#v+
         12i    12.0i   12e0j
#v-
    all represent the same imaginary number.  Actually, \var{12i} is
    really an imaginary integer except that \slang automatically
    promotes it to a double precision imaginary number.

    A more generic complex number may be constructed from an imaginary
    literal via addition, e.g.,
#v+
        3.0 + 4.0i
#v-
    produces a complex number whose real part is \exmp{3.0} and whose
    imaginary part is \exmp{4.0}.

    The intrinsic functions \var{Real} and \var{Imag} may be used to
    retrieve the real and imaginary parts of a complex number,
    respectively.

#%}}}

\sect1{Strings} #%{{{

    A string literal must be enclosed in double quotes as in:
#v+
      "This is a string".
#v-
    Although there is no imposed limit on the length of a string,
    string literals must be less than 256 characters in length.  It is
    possible to go beyond this limit by string concatenation, e.g.,
#v+
     "This is the first part of a long string"
       + "and this is the second half"
#v-
    Any character except a newline (ASCII 10) or the null character
    (ASCII 0) may appear explicitly in a string literal.  However,
    these characters may be used implicitly using the mechanism
    described below. 

    The backslash character is a special character and is used to
    include other special characters (such as a newline character) in
    the string. The special characters recognized are:
#v+
       \"    --  double quote
       \'    --  single quote
       \\    --  backslash
       \a    --  bell character (ASCII 7)
       \t    --  tab character (ASCII 9)
       \n    --  newline character (ASCII 10)
       \e    --  escape character (ASCII 27)
       \xhhh --  character expressed in HEXADECIMAL notation
       \ooo  --  character expressed in OCTAL notation
       \dnnn --  character expressed in DECIMAL
#v-
    For example, to include the double quote character as part of the
    string, it must be preceded by a backslash character, e.g.,
#v+
       "This is a \"quote\""
#v-
    Similarly, the next illustrates how a newline character may be
    included:
#v+
       "This is the first line\nand this is the second"
#v-
#%}}}


\sect1{Null_Type}

   Objects of type \var{Null_Type} can have only one value:
   \var{NULL}.  About the only thing that you can do with this data
   type is to assign it to variables and test for equality with
   other objects.  Nevertheless, \var{Null_Type} is an important and
   extremely useful data type.  Its main use stems from the fact that
   since it can be compared for equality with any other data type, it
   is ideal to represent the value of an object which does not yet
   have a value, or has an illegal value.
   
   As a trivial example of its use, consider
#v+
      define add_numbers (a, b)
      {
         if (a == NULL) a = 0;
         if (b == NULL) b = 0;
         return a + b;
      }
      variable c = add_numbers (1, 2);
      variable d = add_numbers (1, NULL);
      variable e = add_numbers (1,);
      variable f = add_numbers (,);
#v-
   It should be clear that after these statements have been executed,
   \var{c} will have a value of \exmp{3}.  It should also be clear
   that \var{d} will have a value of \exmp{1} because \var{NULL} has
   been passed as the second parameter.  One feature of the language
   is that if a parameter has been omitted from a function call, the
   variable associated with that parameter will be set to \var{NULL}.
   Hence, \var{e} and \var{f} will be set to \exmp{1} and \exmp{0},
   respectively.
   
   The \var{Null_Type} data type also plays an important role in the
   context of \em{structures}.

\sect1{Ref_Type}
   Objects of \var{Ref_Type} are created using the unary
   \em{reference} operator \var{&}.  Such objects may be
   \em{dereferenced} using the dereference operator \var{@}.  For
   example, 
#v+
      variable sin_ref = &sin;
      variable y = (@sin_ref) (1.0);
#v-
   creates a reference to the \var{sin} function and assigns it to
   \var{sin_ref}.  The second statement uses the dereference operator
   to call the function that \var{sin_ref} references.
   
   The \var{Ref_Type} is useful for passing functions as arguments to
   other functions, or for returning information from a function via
   its parameter list.  The dereference operator is also used to create
   an instance of a structure.  For these reasons, further discussion
   of this important type can be found in section ??? and section ???.

\sect1{Array_Type and Struct_Type}
   
   Variables of type \var{Array_Type} and \var{Struct_Type} are known
   as \em{container objects}.  They are much more complicated than the
   simple data types discussed so far and each obeys a special syntax.
   For these reasons they are discussed in a separate chapters.
   See ???.

\sect1{DataType_Type Type} #%{{{

   \slang defines a type called \var{DataType_Type}.  Objects of
   this type have values that are type names.  For example, an integer
   is an object of type \var{Integer_Type}.  The literals of
   \var{DataType_Type} include:
#v+
     Char_Type            (signed character)
     UChar_Type           (unsigned character)
     Short_Type           (short integer)
     UShort_Type          (unsigned short integer)
     Integer_Type         (plain integer)
     UInteger_Type        (plain unsigned integer)
     Long_Type            (long integer)
     ULong_Type           (unsigned long integer)
     Float_Type           (single precision real)
     Double_Type          (double precision real)
     Complex_Type         (complex numbers)
     String_Type          (strings, C strings)
     BString_Type         (binary strings)
     Struct_Type          (structures)
     Ref_Type             (references)
     Null_Type            (NULL)
     Array_Type           (arrays)
     DataType_Type        (data types)
#v-
   as well as the names of any other types that an application
   defines.
   
   The built-in function \var{typeof} returns the data type of
   its argument, i.e., a \var{DataType_Type}.  For instance
   \exmp{typeof(7)} returns \var{Integer_Type} and
   \var{typeof(Integer_Type)} returns \var{DataType_Type}.  One can use this
   function as in the following example:
#v+
      if (Integer_Type == typeof (x)) message ("x is an integer");
#v-
   The literals of \var{DataType_Type} have other uses as well.  One
   of the most common uses of these literals is to create arrays, e.g.,
#v+
        x = Complex_Type [100];
#v-
   creates an array of \exmp{100} complex numbers and assigns it to
   \var{x}.
#%}}}

#%}}}

\sect{Typecasting: Converting from one Type to Another}

   Occasionally, it is necessary to convert from one data type to
   another.  For example, if you need to print an object as a string,
   it may be necessary to convert it to a \var{String_Type}.  The
   \var{typecast} function may be used to perform such conversions.
   For example, consider
#v+
      variable x = 10, y;
      y = typecast (x, Double_Type);
#v-
   After execution of these statements, \var{x} will have the integer
   value \exmp{10} and \var{y} will have the double precision floating
   point value \exmp{10.0}.  If the object to be converted is an
   array, the \var{typecast} function will act upon all elements of
   the array.  For example, 
#v+
      variable x = [1:10];       % Array of integers
      variable y = typecast (x, Double_Type);
#v-
   will create an array of \exmp{10} double precision values and
   assign it to \var{y}.  One should also realize that it is not
   always possible to perform a typecast.  For example, any attempt to
   convert an \var{Integer_Type} to a \var{Null_Type} will result in a
   run-time error.
   
   Often the interpreter will perform implicit type conversions as necessary
   to complete calculations.  For example, when multiplying an
   \var{Integer_Type} with a \var{Double_Type}, it will convert the
   \var{Integer_Type} to a \var{Double_Type} for the purpose of the
   calculation.  Thus, the example involving the conversion of an
   array of integers to an array of doubles could have been performed
   by multiplication by \exmp{1.0}, i.e.,
#v+
      variable x = [1:10];       % Array of integers
      variable y = 1.0 * x;
#v-

   The \var{string} intrinsic function is similar to the typecast
   function except that it converts an object to a string
   representation.  It is important to understand that a typecast from
   some type to \var{String_Type} is \em{not} the same as converting
   an object to its string operation.   That is,
   \exmp{typecast(x,String_Type)} is not equivalent to
   \exmp{string(x)}.  The reason for this is that when given an array,
   the \var{typecast} function acts on each element of the array to
   produce another array, whereas the \var{string} function produces a
   a string.
   
   The \var{string} function is useful for printing the value of an
   object.  This use is illustrated in the following simple example:
#v+
      define print_object (x)
      {
         message (string (x));
      }
#v-
   Here, the \var{message} function has been used because it writes a
   string to the display.  If the \var{string} function was not used
   and the \var{message} function was passed an integer, a
   type-mismatch error would have resulted.

#%}}}

\chapter{Identifiers} #%{{{

   The names given to variables, functions, and data types are called
   \em{identifiers}.  There are some restrictions upon the actual
   characters that make up an identifier.  An identifier name must
   start with a letter (\var{[A-Za-z]}), an underscore character, or a
   dollar sign.  The rest of the characters in the name can be any
   combination of letters, digits, dollar signs, or underscore
   characters.  However, all identifiers whose name begins with two
   underscore characters are reserved for internal use by the
   interpreter and declarations of objects with such names should be
   avoided.

   Examples of valid identifiers include:
#v+
      mary    _3    _this_is_ok
      a7e1    $44   _44$_Three
#v-
   However, the following are not legal:
#v+
      7abc   2e0    #xx
#v-
   In fact, \exmp{2e0} actually specifies the real number
   \exmp{2.0}.

   Although the maximum length of identifiers is unspecified by the
   language, the length should be kept below \exmp{64} characters.

   The following identifiers are reserved by the language for use as
   keywords:
#v+
        !if            _for        do         mod       sign       xor
        ERROR_BLOCK    abs         do_while   mul2      sqr        public
        EXIT_BLOCK     and         else       not       static     private
        USER_BLOCK0    andelse     exch       or        struct
        USER_BLOCK1    break       for        orelse    switch
        USER_BLOCK2    case        foreach    pop       typedef
        USER_BLOCK3    chs         forever    return    using
        USER_BLOCK4    continue    if         shl       variable
        __tmp          define      loop       shr       while
#v-
   In addition, the next major \slang release (v2.0) will reserve
   \exmp{try} and \exmp{catch}, so it is probably a good idea to avoid
   those words until then.

#%}}}

\chapter{Variables} #%{{{

   A variable must be declared before it can be used, otherwise an
   undefined name error will be generated.  A variable is declared
   using the \kw{variable} keyword, e.g,
#v+
      variable x, y, z;
#v-
   declares three variables, \exmp{x}, \exmp{y}, and \exmp{z}.  This
   is an example of a variable declaration statement, and like all
   statements, it must end in a semi-colon.

   Variables declared this way are untyped and inherit a type upon
   assignment.  The actual type checking is performed at run-time.  For
   example,
#v+
        x = "This is a string";
        x = 1.2;
        x = 3;
        x = 2i;
#v-
   results in x being set successively to a string, a float, an
   integer, and to a complex number (\exmp{0+2i}).  Any attempt to use
   a variable before it has acquired a type will result in an
   uninitialized variable error.

   It is legal to put executable code in a variable declaration list.
   That is,
#v+
         variable x = 1, y = sin (x);
#v-
   are legal variable declarations.  This also provides a convenient way
   of initializing a variable.

   Variables are classified as either \em{global} or \em{local}. A
   variable declared inside a function is said to be local and has no
   meaning outside the function.  A variable is said to be global if
   it was declared outside a function.  Global variables are further
   classified as being \var{public}, \var{static}, or \var{private},
   according to the name space where they were defined.
   See chapter ??? for more information about name spaces.

   The following global variables are predefined by the language and
   are mainly used as convenience variables:
#v+
      $0 $1 $2 $3 $4 $5 $6 $7 $8 $9
#v-

   An \em{intrinsic} variable is another type of global variable.
   Such variables have a definite type which cannot be altered.
   Variables of this type may also be defined to be read-only, or
   constant variables.  An example of an intrinsic variable is
   \var{PI} which is a read-only double precision variable with a value
   of approximately \exmp{3.14159265358979323846}.
   
#%}}}

\chapter{Operators} #%{{{

   \slang supports a variety of operators that are grouped into three
   classes: assignment operators, binary operators, and unary operators.

   An assignment operator is used to assign a value to a variable.
   They will be discussed more fully in the context of the assignment
   statement in section ???.

   An unary operator acts only upon a single quantity while a binary
   operation is an operation between two quantities.  The boolean
   operator \var{not} is an example of an unary operator.  Examples of
   binary operators include the usual arithmetic operators
   \var{+}, \var{-}, \var{*}, and \var{/}.  The operator given by
   \var{-} can be either an unary operator (negation) or a binary operator
   (subtraction); the actual operation is determined from the context
   in which it is used.

   Binary operators are used in algebraic forms, e.g., \exmp{a + b}.
   Unary operators fall in one of two classes: postfix-unary or
   prefix-unary.  For example, in the expression \exmp{-x}, the minus
   sign is a prefix-unary operator.
   
   Not all data types have binary or unary operations defined.  For
   example, while \var{String_Type} objects support the \var{+}
   operator, they do not admit the \var{*} operator.

\sect{Unary Operators}

   The \bf{unary} operators operate only upon a single operand.  They
   include: \var{not}, \var{~}, \var{-}, \var{@}, \var{&}, as well as the
   increment and decrement operators \var{++} and \var{--},
   respectively.

   The boolean operator \var{not} acts only upon integers and produces
   \var{0} if its operand is non-zero, otherwise it produces \var{1}.

   The bit-level not operator \var{~} performs a similar function,
   except that it operates on the individual bits of its integer
   operand.

   The arithmetic negation operator \var{-} is the most well-known
   unary operator.  It simply reverses the sign of its operand.

   The reference (\var{&}) and dereference (\var{@}) operators will be
   discussed in greater detail in section ???.  Similarly, the
   increment (\var{++}) and decrement (\var{--}) operators will be
   discussed in the context of the assignment operator.

\sect{Binary Operators} #%{{{

   The binary operators may be grouped according to several classes:
   arithmetic operators, relational operators, boolean operators, and
   bitwise operators.
   
   All binary and unary operators may be overloaded.  For example, the
   arithmetic plus operator has been overloaded by the
   \var{String_Type} data type to permit concatenation between strings.

\sect1{Arithmetic Operators} #%{{{

   The arithmetic operators include \var{+}, \var{-}, \var{*}, \var{/},
   which perform addition, subtraction, multiplication, and division,
   respectively.  In addition to these, \slang supports the \var{mod}
   operator as well as the power operator \var{^}.

   The data type of the result produced by the use of one of these
   operators depends upon the data types of the binary participants.
   If they are both integers, the result will be an integer.  However,
   if the operands are not of the same type, they will be converted to
   a common type before the operation is performed.  For example, if
   one is a floating point value and the other is an integer, the
   integer will be converted to a float. In general, the promotion
   from one type to another is such that no information is lost, if
   possible.  As an example, consider the expression \exmp{8/5} which
   indicates division of the integer \var{8} by the integer \var{5}.
   The result will be the integer \var{1} and \em{not} the floating
   point value \var{1.6}.  However, \exmp{8/5.0} will produce
   \var{1.6} because \exmp{5.0} is a floating point number.

#%}}}

\sect1{Relational Operators} #%{{{

   The relational operators are \var{>}, \var{>=}, \var{<}, \var{<=},
   \var{==}, and \var{!=}.  These perform the comparisons greater
   than, greater than or equal, less than, less than or equal, equal,
   and not equal, respectively.  The result of one of these
   comparisons is the integer \var{1} if the comparison is true, or
   \var{0} if the comparison is false.  For example, \exmp{6 >= 5}
   returns \var{1}, but \var{6 == 5} produces
   \var{0}.

#%}}}

\sect1{Boolean Operators} #%{{{
   There are only two boolean binary operators: \var{or} and
   \var{and}.  These operators are defined only for integers and
   produce an integer result.  The \var{or} operator returns \var{1}
   if either of its operands are non-zero, otherwise it produces
   \var{0}.  The \var{and} operator produces \var{1} if and only if
   both its operands are non-zero, otherwise it produces \var{0}.

   Neither of these operators perform the so-called boolean
   short-circuit evaluation.  For example, consider the expression:
#v+
      (x != 0) and (1/x > 10)
#v-
   Here, if \var{x} were to have a value of zero, a division by zero error
   would occur because even though \var{x!=0} evaluates to zero, the
   \var{and} operator is not short-circuited and the \var{1/x} expression
   would still be evaluated.  Although these operators are not
   short-circuited, \slang does have another mechanism of performing
   short-circuit boolean evaluation via the \kw{orelse} and
   \kw{andelse} expressions.  See below for information about these
   constructs.

#%}}}

\sect1{Bitwise Operators} #%{{{

   The bitwise binary operators are defined only with integer operands
   and are used for bit-level operations.  Operators that fall in this
   class include \var{&}, \var{|}, \var{shl}, \var{shr}, and
   \var{xor}.  The \var{&} operator performs a boolean AND operation
   between the corresponding bits of the operands.  Similarly, the
   \var{|} operator performs the boolean OR operation on the bits.
   The bit-shifting operators \var{shl} and \var{shr} shift the bits
   of the first operand by the number given by the second operand to
   the left or right, respectively.  Finally, the \var{xor} performs
   an EXCLUSIVE-OR operation.

   These operators are commonly used to manipulate variables whose
   individual bits have distinct meanings.  In particular, \var{&} is
   usually used to test bits, \var{|} can be used to set bits, and
   \var{xor} may be used to flip a bit.

   As an example of using \var{&} to perform tests on bits, consider
   the following: The \jed text editor stores some of the information
   about a buffer in a bitmapped integer variable.  The value of this
   variable may be retrieved using the \jed intrinsic function
   \var{getbuf_info}, which actually returns four quantities: the
   buffer flags, the name of the buffer, directory name, and file
   name.  For the purposes of this section, only the buffer flags are
   of interest and can be retrieved via a function such as
#v+
      define get_buffer_flags ()
      {
         variable flags;
         (,,,flags) = getbuf_info ();
         return flags;
      }
#v-
   The buffer flags is a bitmapped quantity where the 0th bit
   indicates whether or not the buffer has been modified, the first
   bit indicates whether or not autosave has been enabled for the
   buffer, and so on.  Consider for the moment the task of determining
   if the buffer has been modified.  This can be
   determined by looking at the zeroth bit, if it is \var{0} the
   buffer has not been modified, otherwise it has.  Thus we can create
   the function,
#v+
     define is_buffer_modified ()
     {
        variable flags = get_buffer_flags ();
        return (flags & 1);
     }
#v-
   where the integer \exmp{1} has been used since it has all of its
   bits set to \var{0}, except for the zeroth one, which is set to
   \var{1}.  (At this point, it should also be apparent that bits are
   numbered from zero, thus an \var{8} bit integer consists of bits
   \var{0} to \var{7}, where \var{0} is the least significant bit and
   \var{7} is the most significant one.)  Similarly, we can create another
   function
#v+
     define is_autosave_on ()
     {
        variable flags = get_buffer_flags ();
        return (flags & 2);
     }
#v-
   to determine whether or not autosave has been turned on for the
   buffer.

   The \var{shl} operator may be used to form the integer with only
   the \em{nth} bit set.  For example, \exmp{1 shl 6} produces an
   integer with all bits set to zero except the sixth bit, which is
   set to one.  The following example exploits this fact:
#v+
     define test_nth_bit (flags, nth)
     {
        return flags & (1 shl nth);
     }
#v-
  
#%}}}

\sect1{Namespace operator}
   The operator \var{->} is used to in conjunction with the name of a
   namespace to access an object within the namespace.  For example,
   if \exmp{A} is the name of a namespace containing the variable
   \var{v}, then \exmp{A->v} refers to that variable.

\sect1{Operator Precedence}

\sect1{Binary Operators and Functions Returning Multiple Values} #%{{{
   Care must be exercised when using binary operators with an operand
   the returns multiple values.  In fact, the current implementation
   of the \slang language will produce incorrect results if both
   operands of a binary expression return multiple values.  \em{At
   most, only one of operands of a binary expression can return
   multiple values, and that operand must be the first one, not the
   second.}  For example,
#v+
    define read_line (fp)
    {
       variable line, status;
       
       status = fgets (&line, fp);
       if (status == -1)
         return -1;
       return (line, status);
    }
#v-
   defines a function, \var{read_line} that takes a single argument, a
   handle to an open file, and returns one or two values, depending
   upon the return value of \var{fgets}.  Now consider
#v+
        while (read_line (fp) > 0)
          {
             text = ();
             % Do something with text
             .
             .
          }
#v-
   Here the relational binary operator \var{>} forms a comparison
   between one of the return values (the one at the top of the stack)
   and \var{0}.  In accordance with the above rule, since \var{read_line}
   returns multiple values, it occurs as the left binary operand.
   Putting it on the right as in
#v+
        while (0 < read_line (fp))    % Incorrect
          {
             text = ();
             % Do something with text
             .
             .
          }
#v-
   violates the rule and will result in the wrong answer.

#%}}}

#%}}}

\sect{Mixing Integer and Floating Point Arithmetic}

   If a binary operation (\var{+}, \var{-}, \var{*} , \var{/}) is
   performed on two integers, the result is an integer.  If at least
   one of the operands is a float, the other is converted to float and
   the result is float.  For example:
#v+
      11 / 2           --> 5   (integer)
      11 / 2.0         --> 5.5 (float)
      11.0 / 2         --> 5.5 (float)
      11.0 / 2.0       --> 5.5 (float)
#v-
   Finally note that only integers may be used as array indices, 
   loop control variables, and bit operations.  The conversion
   functions, \var{int} and \var{float}, may be used convert between
   floats and ints where appropriate, e.g.,
#v+
      int (1.5)         --> 1 (integer)
      float(1.5)        --> 1.5 (float)
      float (1)         --> 1.0 (float)
#v-

\sect{Short Circuit Boolean Evaluation}

   The boolean operators \var{or} and \var{and} \em{are not short
   circuited} as they are in some languages.  \slang uses 
   \var{orelse} and \var{andelse} expressions for short circuit boolean
   evaluation.  However, these are not binary operators. Expressions
   of the form:
\begin{tscreen}
        \em{expr-1} and \em{expr-2} and ... \em{expr-n}
\end{tscreen}
   can be replaced by the short circuited version using \var{andelse}:
\begin{tscreen}
        andelse {\em{expr-1}} {\em{expr-2}} ... {\em{expr-n}}
\end{tscreen}
   A similar syntax holds for the \var{orelse} operator.  For example, consider
   the statement:
#v+
      if ((x != 0) and (1/x > 10)) do_something ();
#v-
   Here, if \var{x} were to have a value of zero, a division by zero error
   would occur because even though \var{x!=0} evaluates to zero, the
   \var{and} operator is not short circuited and the \var{1/x} expression
   would be evaluated causing division by zero. For this case, the
   \var{andelse} expression could be used to avoid the problem:
#v+
      if (andelse
          {x != 0}
          {1 / x > 10})  do_something ();
#v-

#%}}}

\chapter{Statements} #%{{{

   Loosely speaking, a \em{statement} is composed of \em{expressions}
   that are grouped according to the syntax or grammar of the language
   to express a complete computation.  Statements are analogous to
   sentences in a human language and expressions are like phrases.
   All statements in the \slang language must end in a semi-colon.

   A statement that occurs within a function is executed only during
   execution of the function.  However, statements that occur outside
   the context of a function are evaluated immediately.

   The language supports several different types of statements such as
   assignment statements, conditional statements, and so forth.  These
   are described in detail in the following sections.

\sect{Variable Declaration Statements}
   Variable declarations were already discussed in chapter ???.  For
   the sake of completeness, a variable declaration is a statement of
   the form
\begin{tscreen}
     variable \em{variable-declaration-list} ;
\end{tscreen}
   where the \em{variable-declaration-list} is a comma separated list
   of one or more variable names with optional initializations, e.g.,
#v+
     variable x, y = 2, z;
#v-
\sect{Assignment Statements} #%{{{

   Perhaps the most well known form of statement is the \em{assignment
   statement}.  Statements of this type consist of a left-hand side,
   an assignment operator, and a right-hand side.  The left-hand side
   must be something to which an assignment can be performed.  Such
   an object is called an \em{lvalue}.
   
   The most common assignment operator is the simple assignment
   operator \var{=}.  Simple of its use include
#v+
      x = 3;
      x = some_function (10);
      x = 34 + 27/y + some_function (z);
      x = x + 3;
#v-
   In addition to the simple assignment operator, \slang
   also supports the assignment operators \var{+=} and \var{-=}.
   Internally, \slang transforms
#v+
       a += b;
#v-
   to
#v+
       a = a + b;
#v-
   Similarly, \exmp{a -= b} is transformed to \exmp{a = a - b}.  It is
   extremely important to realize that, in general, \exmp{a+b} is not
   equal to \exmp{b+a}.  This means that \exmp{a+=b} is not the same
   as \exmp{a=b+a}.  As an example consider
#v+
      a = "hello"; a += "world";
#v-
   After execution of these two statements, \var{a} will have the
   value \exmp{"helloworld"} and not \exmp{"worldhello"}.

   Since adding or subtracting \exmp{1} from a variable is quite
   common, \slang also supports the unary increment and decrement
   operators \exmp{++}, and \exmp{--}, respectively.  That is, for
   numeric data types, 
#v+
       x = x + 1;
       x += 1;
       x++;
#v-
   are all equivalent.  Similarly,
#v+
       x = x - 1;
       x -= 1;
       x--;
#v-
   are also equivalent.

   Strictly speaking, \var{++} and \var{--} are unary operators.  When
   used as \var{x++}, the \var{++} operator is said to be a
   \em{postfix-unary} operator.  However, when used as \var{++x} it is
   said to be a \em{prefix-unary} operator.  The current
   implementation does not distinguish between the two forms, thus
   \var{x++} and \var{++x} are equivalent.  The reason for this
   equivalence is \em{that assignment expressions do not return a value in
   the \slang language} as they do in C.  Thus one should exercise care
   and not try to write C-like code such as
#v+
      x = 10;
      while (--x) do_something (x);     % Ok in C, but not in S-Lang
#v-
   The closest valid \slang form involves a \em{comma-expression}:
#v+
      x = 10;
      while (x--, x) do_something (x);  % Ok in S-Lang and in C
#v-

   \slang also supports a \em{multiple-assignment} statement.  It is
   discussed in detail in section ???.

#%}}}

\sect{Conditional and Looping Statements} #%{{{

  \slang supports a wide variety of conditional and looping
  statements.  These constructs operate on statements grouped together
  in \em{blocks}.  A block is a sequence of \slang statements enclosed
  in braces and may contain other blocks. However, a block cannot
  include function declarations.  In the following,
  \em{statement-or-block} refers to either a single
  \slang statement or to a block of statements, and
  \em{integer-expression} is an integer-valued expression.
  \em{next-statement} represents the statement following the form
   under discussion.

\sect1{Conditional Forms} #%{{{
\sect2{if}
   The simplest condition statement is the \kw{if} statement.  It
   follows the syntax
\begin{tscreen}
        if (\em{integer-expression}) \em{statement-or-block}
        \em{next-statement}
\end{tscreen}
   If \em{integer-expression} evaluates to a non-zero result, then the
   statement or group of statements implied \em{statement-or-block}
   will get executed.  Otherwise, control will proceed to
   \em{next-statement}.

   An example of the use of this type of conditional statement is
#v+
       if (x != 0) 
         {
            y = 1.0 / x;
            if (x > 0) z = log (x);
         }
#v-
   This example illustrates two \var{if} statements where the second
   \var{if} statement is part of the block of statements that belong to
   the first.

\sect2{if-else}
   Another form of \kw{if} statement is the \em{if-else} statement.
   It follows the syntax:
\begin{tscreen}
      if (\em{integer-expression}) \em{statement-or-block-1}
      else \em{statement-or-block-2}
      \em{next-statement}
\end{tscreen}
   Here, if \em{expression} returns non-zero,
   \em{statement-or-block-1} will get executed and control will pass
   on to \em{next-statement}. However, if \em{expression} returns zero,
   \em{statement-or-block-2} will get executed before continuing with
   \em{next-statement}.  A simple example of this form is
#v+
     if (x > 0) z = log (x); else error ("x must be positive");
#v-
   Consider the more complex example:
#v+
     if (city == "Boston")
       if (street == "Beacon") found = 1;
     else if (city == "Madrid") 
       if (street == "Calle Mayor") found = 1;
     else found = 0;
#v-
   This example illustrates a problem that beginners have with
   \em{if-else} statements.  The grammar presented above shows that
   the this example is equivalent to
#v+
     if (city == "Boston")
       {
         if (street == "Beacon") found = 1;
         else if (city == "Madrid")
           {
             if (street == "Calle Mayor") found = 1;
             else found = 0;
           }
       }
#v-
   It is important to understand the grammar and not be seduced by the
   indentation!

\sect2{!if}

   One often encounters \kw{if} statements similar to
\begin{tscreen}
     if (\em{integer-expression} == 0) \em{statement-or-block}
\end{tscreen}
   or equivalently,
\begin{tscreen}
     if (not(\em{integer-expression})) \em{statement-or-block}
\end{tscreen}
   The \kw{!if} statement was added to the language to simplify the
   handling of such statements.  It obeys the syntax
\begin{tscreen}
     !if (\em{integer-expression}) \em{statement-or-block}
\end{tscreen}
   and is functionally equivalent to
\begin{tscreen}
     if (not (\em{expression})) \em{statement-or-block}
\end{tscreen}

\sect2{orelse, andelse}

  These constructs were discussed earlier.  The syntax for the
  \var{orelse} statement is:
\begin{tscreen}
     orelse {\em{integer-expression-1}} ... {\em{integer-expression-n}}
\end{tscreen}
  This causes each of the blocks to be executed in turn until one of
  them returns a non-zero integer value.  The result of this statement
  is the integer value returned by the last block executed.  For
  example,
#v+
     orelse { 0 } { 6 } { 2 } { 3 }
#v-
  returns \var{6} since the second block is the first to return a
  non-zero result.  The last two block will not get executed.

  The syntax for the \var{andelse} statement is:
\begin{tscreen}
     andelse {\em{integer-expression-1}} ... {\em{integer-expression-n}}
\end{tscreen}
  Each of the blocks will be executed in turn until one of
  them returns a zero value.  The result of this statement is the
  integer value returned by the last block executed.  For example,
#v+
     andelse { 6 } { 2 } { 0 } { 4 }
#v-
  returns \var{0} since the third block will be the last to execute.

\sect2{switch}
  The switch statement deviates the most from its C counterpart.  The
  syntax is:
#v+
          switch (x)
            { ...  :  ...}
              .
              .
            { ...  :  ...}
#v-
   The `\var{:}' operator is a special symbol which means to test
   the top item on the stack, and if it is non-zero, the rest of the block
   will get executed and control will pass out of the switch statement.
   Otherwise, the execution of the block will be terminated and the process
   will be repeated for the next block.  If a block contains no
   \var{:} operator, the entire block is executed and control will
   pass onto the next statement following the \kw{switch} statement.
   Such a block is known as the \em{default} case.
   
   As a simple example, consider the following:
#v+
      switch (x)
        { x == 1 : message("Number is one.");}
        { x == 2 : message("Number is two.");}
        { x == 3 : message("Number is three.");}
        { x == 4 : message("Number is four.");}
        { x == 5 : message("Number is five.");}
        { message ("Number is greater than five.");}
#v-
   Suppose \var{x} has an integer value of \exmp{3}.  The first two
   blocks will terminate at the `\var{:}' character because each of the
   comparisons with \var{x} will produce zero.  However, the third
   block will execute to completion.  Similarly, if \var{x} is
   \exmp{7}, only the last block will execute in full.

   A more familiar way to write the previous example used the
   \kw{case} keyword:
#v+
      switch (x)
        { case 1 : print("Number is one.");}
        { case 2 : print("Number is two.");}
        { case 3 : print("Number is three.");}
        { case 4 : print("Number is four.");}
        { case 5 : print("Number is five.");}
        { print ("Number is greater than five.");}
#v-
   The \var{case} keyword is a more useful comparison operator because
   it can perform a comparison between different data types while
   using \var{==} may result in a type-mismatch error.  For example,
#v+
      switch (x)
        { (x == 1) or (x == "one") : print("Number is one.");}
        { (x == 2) or (x == "two") : print("Number is two.");}
        { (x == 3) or (x == "three") : print("Number is three.");}
        { (x == 4) or (x == "four") : print("Number is four.");}
        { (x == 5) or (x == "five") : print("Number is five.");}
        { print ("Number is greater than five.");}
#v-
  will fail because the \var{==} operation is not defined between
  strings and integers.  The correct way to write this to use the
  \var{case} keyword:
#v+
      switch (x)
        { case 1 or case "one" : print("Number is one.");}
        { case 2 or case "two" : print("Number is two.");}
        { case 3 or case "three" : print("Number is three.");}
        { case 4 or case "four" : print("Number is four.");}
        { case 5 or case "five" : print("Number is five.");}
        { print ("Number is greater than five.");}
#v-

#%}}}

\sect1{Looping Forms} #%{{{

\sect2{while}
   The \kw{while} statement follows the syntax
\begin{tscreen}
      while (\em{integer-expression}) \em{statement-or-block}
      \em{next-statement}
\end{tscreen}
   It simply causes \em{statement-or-block} to get executed as long as
   \em{integer-expression} evaluates to a non-zero result.  For
   example, 
#v+
      i = 10; 
      while (i) 
        {
          i--;
          newline ();
        }
#v-
   will cause the \var{newline} function to get called 10 times.
   However, 
#v+
      i = -10;
      while (i) 
        {
          i--;
          newline ();
        }
#v-
   would loop forever (or until \var{i} wraps from the most negative
   integer value to the most positive and then decrements to zero).

   
   If you are a C programmer, do not let the syntax of the language
   seduce you into writing this example as you would in C:
#v+
      i = 10;
      while (i--) newline ();
#v-
   The fact is that expressions such as \var{i--} do not return a
   value in \slang as they do in C.  If you must write this way, use
   the comma operator as in
#v+
      i = 10;
      while (i, i--) newline ();
#v-

\sect2{do...while}
   The \kw{do...while} statement follows the syntax
\begin{tscreen}
      do 
         \em{statement-or-block}
      while (\em{integer-expression});
\end{tscreen}
   The main difference between this statement and the \var{while}
   statement is that the \kw{do...while} form performs the test
   involving \em{integer-expression} after each execution
   of \em{statement-or-block} rather than before.  This guarantees that
   \em{statement-or-block} will get executed at least once.
   
   A simple example from the \jed editor follows:
#v+
     bob ();      % Move to beginning of buffer
     do
       {
          indent_line ();
       }
     while (down (1));
#v-
   This will cause all lines in the buffer to get indented via the
   \jed intrinsic function \var{indent_line}.

\sect2{for}
   Perhaps the most complex looping statement is the \kw{for}
   statement; nevertheless, it is a favorite of many programmers.
   This statement obeys the syntax
\begin{tscreen}
    for (\em{init-expression}; \em{integer-expression}; \em{end-expression}) 
      \em{statement-or-block}
    \em{next-statement}
\end{tscreen}
   In addition to \em{statement-or-block}, its specification requires
   three other expressions.  When executed, the \kw{for} statement
   evaluates \em{init-expression}, then it tests
   \em{integer-expression}.  If \em{integer-expression} returns zero,
   control passes to \em{next-statement}.  Otherwise, it executes
   \em{statement-or-block} as long as \em{integer-expression}
   evaluates to a non-zero result.  After every execution of
   \em{statement-or-block}, \em{end-expression} will get evaluated.
   
   This statement is \em{almost} equivalent to 
\begin{tscreen}
    \em{init-expression};
    while (\em{integer-expression})
      {
         \em{statement-or-block}
         \em{end-expression};
      }
\end{tscreen}
   The reason that they are not fully equivalent involves what happens
   when \em{statement-or-block} contains a \kw{continue} statement.
   
   Despite the apparent complexity of the \kw{for} statement, it is
   very easy to use.  As an example, consider
#v+
     s = 0;
     for (i = 1; i <= 10; i++) s += i;
#v-
   which computes the sum of the first 10 integers.

\sect2{loop}
   The \kw{loop} statement simply executes a block of code a fixed
   number of times.  It follows the syntax
\begin{tscreen}
      loop (\em{integer-expression}) \em{statement-or-block}
      \em{next-statement}
\end{tscreen}
   If the \em{integer-expression} evaluates to a positive integer,
   \em{statement-or-block} will get executed that many times.
   Otherwise, control will pass to \em{next-statement}.
   
   For example,
#v+
      loop (10) newline ();
#v-
   will cause the function \var{newline} to get called 10 times.

\sect2{_for}
   Like \kw{loop}, the \kw{_for} statement simply executes a block of
   code a fixed number times.  Unlike the \kw{loop} statement, the
   \kw{_for} loop is useful in situations where the loop index is
   needed.  It obeys the syntax
\begin{tscreen}
      _for (\em{first-value}, \em{last-value}, \em{increment})
         \em{block}
      \em{next-statement}
\end{tscreen}
   Each time through the loop, the current value of the loop index is
   pushed onto the stack.  The first time through, the loop index
   will have the value of \em{first-value}.  The second time its value
   will be \em{first-value} + \em{increment}, and so on.  The loop
   will terminate when the value of the loop index exceeds
   \em{last-value}.  The current implementation requires the control
   parameters \em{first-value}, \em{last-value}, and \em{increment} to
   be integered valued expressions.

   For example, it may be used to compute the sum of the first ten
   integers:
#v+
     s = 0;
     _for (1, 10, 1)
       {
         i = ();
         s += i;
       }
#v-

   The execution speed of the \kw{_for} loop is more than twice as fast as
   the more powerful \kw{for} loop making it a better choice for many
   situations.

\sect2{forever}
   The \kw{forever} statement is similar to the \kw{loop} statement
   except that it loops forever, or until a \kw{break} or a
   \kw{return} statement is executed.  It obeys the syntax
\begin{tscreen}
     forever \em{statement-or-block}
\end{tscreen}
   A trivial example of this statement is
#v+
     n = 10;
     forever
       {
          if (n == 0) break;
          newline ();
          n--;
       }
#v-

\sect2{foreach}
   The \kw{foreach} statement is used to loop over one or more
   statements for every element in a container object.  A container
   object is a data type that consists of other types.  Examples
   include both ordinary and associative arrays, structures, and
   strings.  Every time through the loop the current member of the
   object is pushed onto the stack.

   The simple type of \kw{foreach} statement obeys the syntax
\begin{tscreen}
     foreach (\em{container-object}) \em{statement-or-block}
\end{tscreen}
   Here \em{container-object} can be an expression that returns a
   container object.  A simple example is
#v+
     foreach (["apple", "peach", "pear"])
      {
         fruit = ();
         process_fruit (fruit);
      } 
#v-
   This example shows that if the container object is an array, then
   successive elements of the array are pushed onto the stack prior to
   each execution cycle.  If the container object is a string, then
   successive characters of the string are pushed onto the stack.

   What actually gets pushed onto the stack may be controlled via the
   \kw{using} form of the \kw{foreach} statement.  This more complex
   type of \kw{foreach} statement follows the syntax
\begin{tscreen}
     foreach ( \em{container-object} ) using ( \em{control-list} ) 
       \em{statement-or-block}
\end{tscreen}
   The allowed values of \em{control-list} will depend upon the type
   of container object.  For associative arrays (\var{Assoc_Type}),
   \em{control-list} specified whether \em{keys}, \em{values}, or both
   are pushed onto the stack.  For example,
#v+
     foreach (a) using ("keys") 
       {
          k = ();
           .
           .
       }
#v-
   results in the keys of the associative array \var{a} being pushed
   on the list.  However, 
#v+
     foreach (a) using ("values")
       {
          v = ();
           .
           .
       }
#v-
   will cause the values to be used, and
#v+
     foreach (a) using ("keys", "values")
       {
          (k,v) = ();
           .
           .
       }
#v-
  will use both the keys and values of the array.
  
  Similarly, for linked-lists of structures, one may walk the list via
  code like
#v+
     foreach (linked_list) using ("next")
       {
          s = ();
            .
            .
       }
#v-
  This \kw{foreach} statement is equivalent
#v+
     s = linked_list;
     while (s != NULL)
       {
          .
          .
         s = s.next;
       }
#v-
  Consult the type-specific documentation for a discussion of the
  \kw{using} control words, if any, appropriate for a given type.

\sect{break, return, continue}

   \slang also includes the non-local transfer functions \var{return}, \var{break},
   and \var{continue}.  The \var{return} statement causes control to return to the
   calling function while the \var{break} and \var{continue} statements are used in
   the context of loop structures.  Consider:
#v+
       define fun ()
       {
          forever
            {
               s1;
               s2;
               ..
               if (condition_1) break;
               if (condition_2) return;
               if (condition_3) continue;
               ..
               s3;
            }
          s4;
          ..
       }
#v-
   Here, a function \var{fun} has been defined that contains a \var{forever}
   loop consisting of statements \var{s1}, \var{s2},\ldots,\var{s3}, and
   three \var{if} statements.  As long as the expressions \var{condition_1},
   \var{condition_2}, and \var{condition_3} evaluate to zero, the statements
   \var{s1}, \var{s2},\ldots,\var{s3} will be repeatedly executed.  However,
   if \var{condition_1} returns a non-zero value, the \var{break} statement
   will get executed, and control will pass out of the \var{forever} loop to
   the statement immediately following the loop which in this case is
   \var{s4}. Similarly, if \var{condition_2} returns a non-zero number,
   the \var{return} statement will cause control to pass back to the
   caller of \var{fun}.  Finally, the \var{continue} statement will
   cause control to pass back to the start of the loop, skipping the
   statement \var{s3} altogether.


#%}}}

#%}}}

#%}}}

\chapter{Functions} #%{{{

   A function may be thought of as a group of statements that work
   together to perform a computation.  While there are no imposed
   limits upon the number statements that may occur within a function,
   it is considered poor programming practice if a function contains
   many statements. This notion stems from the belief that a function
   should have a simple, well defined purpose.

\sect{Declaring Functions} #%{{{

   Like variables, functions must be declared before they can be used. The
   \kw{define} keyword is used for this purpose.  For example,
#v+
      define factorial ();
#v-
   is sufficient to declare a function named \var{factorial}.  Unlike
   the \var{variable} keyword used for declaring variables, the
   \var{define} keyword does not accept a list of names.  

   Usually, the above form is used only for recursive functions.  In
   most cases, the function name is almost always followed by a
   parameter list and the body of the function:
\begin{tscreen}
      define \em{function-name} (\em{parameter-list})
      {
         \em{statement-list}
      }
\end{tscreen}
   The \em{function-name} is an identifier and must conform to the
   naming scheme for identifiers discussed in chapter ???.
   The \em{parameter-list} is a comma-separated list of variable names
   that represent parameters passed to the function, and
   may be empty if no parameters are to be passed.
   The body of the function is enclosed in braces and consists of zero
   or more statements (\em{statement-list}).
   
   The variables in the \em{parameter-list} are implicitly declared,
   thus, there is no need to declare them via a variable declaration
   statement.  In fact any attempt to do so will result in a syntax
   error.
   
#%}}}

\sect{Parameter Passing Mechanism} #%{{{

   Parameters to a function are always passed by value and never by
   reference.  To see what this means, consider
#v+
     define add_10 (a) 
     {
        a = a + 10;
     }
     variable b = 0;
     add_10 (b);
#v-
   Here a function \var{add_10} has been defined, which when executed,
   adds \exmp{10} to its parameter.  A variable \var{b} has also been
   declared and initialized to zero before it is passed to
   \var{add_10}.  What will be the value of \var{b} after the call to
   \var{add_10}?  If \slang were a language that passed parameters by
   reference, the value of \var{b} would be changed to
   \var{10}.  However, \slang always passes by value, which means that
   \var{b} would retain its value of zero after the function call.
   
   \slang does provide a mechanism for simulating pass by reference
   via the reference operator.  See the next section for more details.
   
   If a function is called with a parameter in the parameter list
   omitted, the corresponding variable in the function will be set to
   \var{NULL}.  To make this clear, consider the function
#v+
     define add_two_numbers (a, b)
     {
        if (a == NULL) a = 0;
        if (b == NULL) b = 0;
        return a + b;
     }
#v-
   This function must be called with two parameters.  However, we can
   omit one or both of the parameters by calling it in one of the
   following ways:
#v+
     variable s = add_two_numbers (2,3);
     variable s = add_two_numbers (2,);
     variable s = add_two_numbers (,3);
     variable s = add_two_numbers (,);
#v-
   The first example calls the function using both parameters;
   however, at least one of the parameters was omitted in the other
   examples.  The interpreter will implicitly convert the last three
   examples to
#v+
     variable s = add_two_numbers (2, NULL);
     variable s = add_two_numbers (NULL, 3);
     variable s = add_two_numbers (NULL, NULL);
#v-
   It is important to note that this mechanism is available only for
   function calls that specify more than one parameter.  That is,
#v+
     variable s = add_10 ();
#v-
  is \em{not} equivalent to \exmp{add_10(NULL)}.  The reason for this
  is simple: the parser can only tell whether or not \var{NULL} should
  be substituted by looking at the position of the comma character in
  the parameter list, and only function calls that indicate more than
  one parameter will use a comma.  A mechanism for handling single
  parameter function calls is described in the next section.

#%}}}

\sect{Referencing Variables} #%{{{

   One can achieve the effect of passing by reference by using the
   reference (\var{&}) and dereference (\var{@}) operators. Consider
   again the \var{add_10} function presented in the previous section.
   This time we write it as
#v+
     define add_10 (a)
     {  
        @a = @a + 10;
     }
     variable b = 0;
     add_10 (&b);
#v-
   The expression \var{&b} creates a \em{reference} to the variable
   \var{b} and it is the reference that gets passed to \var{add_10}.
   When the function \var{add_10} is called, the value of \var{a} will
   be a reference to \var{b}.  It is only by \em{dereferencing} this
   value that \var{b} can be accessed and changed.  So, the statement
   \exmp{@a=@a+10;} should be read `add \exmp{10}' to the value of the
   object that \var{a} references and assign the result to the object
   that \var{a} references.
   
   The reader familiar with C will note the similarity between
   \em{references} in \slang and \em{pointers} in C.  

   One of the main purposes for references is that this mechanism
   allows reference to functions to be passed to other functions.  As
   a simple example from elementary calculus, consider the following
   function which returns an approximation to the derivative of another
   function at a specified point:
#v+
     define derivative (f, x)
     {
        variable h = 1e-6;
        return ((@f)(x+h) - (@f)(x)) / h;
     }
#v-
   It can be used to differentiate the function
#v+
     define x_squared (x)
     {
        return x^2;
     }
#v-
   at the point \exmp{x = 3} via the expression
   \exmp{derivative(&x_squared,3)}.


#%}}}

\sect{Functions with a Variable Number of Arguments} #%{{{

  \slang functions may be defined to take a variable number of
  arguments.  The reason for this is that the calling routine pushes
  the arguments onto the stack before making a function call, and it
  is up to the called function to pop the values off the stack and
  make assignments to the variables in the parameter list.  These
  details are, for the most part, hidden from the programmer.
  However, they are important when a variable number of arguments are
  passed.

  Consider the \var{add_10} example presented earlier.  This time it
  is written
#v+
     define add_10 ()
     {
        variable x;
        x = ();
        return x + 10;
     }
     variable s = add_10 (12);  % ==> s = 22;
#v-
  For the uninitiated, this example looks as if it
  is destined for disaster.  The \var{add_10} function looks like it
  accepts zero arguments, yet it was called with a single argument.
  On top of that, the assignment to \var{x} looks strange.  The truth
  is, the code presented in this example makes perfect sense, once you
  realize what is happening.
  
  First, consider what happened when \var{add_10} is called with the
  the parameter \exmp{12}.  Internally, \exmp{12} is
  pushed onto the stack and then the function called.  Now,
  consider the function itself.  \var{x} is a variable local to the
  function.  The strange looking assignment `\exmp{x=()}' simply
  takes whatever is on the stack and assigns it to \var{x}.  In
  other words, after this statement, the value of \var{x} will be
  \exmp{12}, since \exmp{12} will be at the top of the stack.

  A generic function of the form
#v+
    define function_name (x, y, ..., z)
    {
       .
       .
    }
#v-
  is internally transformed by the interpreter to
#v+
    define function_name ()
    {
       variable x, y, ..., z;
       z = ();
       .
       .
       y = ();
       x = ();
       .
       .
    }
#v- 
  before further parsing.  (The \var{add_10} function, as defined above, is
  already in this form.)  With this knowledge in hand, one can write a
  function that accepts a variable number of arguments.  Consider the
  function:
#v+
    define average_n (n)
    {
       variable x, y;
       variable s;
       
       if (n == 1) 
         {
            x = ();
            s = x;
         }
       else if (n == 2)
         {
            y = ();
            x = ();
            s = x + y;
         }
       else error ("average_n: only one or two values supported");
       
       return s / n;
   }
   variable ave1 = average_n (3.0, 1);        % ==> 3.0
   variable ave2 = average_n (3.0, 5.0, 2);   % ==> 4.0
#v-
  Here, the last argument passed to \var{average_n} is an integer
  reflecting the number of quantities to be averaged.  Although this
  example works fine, its principal limitation is obvious: it only
  supports one or two values.  Extending it to three or more values
  by adding more \exmp{else if} constructs is rather straightforward but
  hardly worth the effort.  There must be a better way, and there is:
#v+
   define average_n (n)
   {
      variable s, x;
      s = 0;
      loop (n) 
        {
           x = ();    % get next value from stack
           s += x;
        }
      return s / n;
   }
#v-
  The principal limitation of this approach is that one must still
  pass an integer that specifies how many values are to be averaged.

  Fortunately, a special variable exists that is local to every function
  and contains the number of values that were passed to the function.
  That variable has the name \var{_NARGS} and may be used as follows:
#v+
   define average_n ()
   {
      variable x, s = 0;
      
      if (_NARGS == 0) error ("Usage: ave = average_n (x, ...);");

      loop (_NARGS)
        {
           x = ();
           s += x;
        }
      return s / _NARGS;
   }
#v-
  Here, if no arguments are passed to the function, a simple message
  that indicates how it is to be used is printed out.


#%}}}


\sect{Returning Values}

   As stated earlier, the usual way to return values from a function
   is via the \kw{return} statement.  This statement has the
   simple syntax
\begin{tscreen}
      return \em{expression-list} ;
\end{tscreen}
   where \em{expression-list} is a comma separated list of expressions.
   If the function does not return any values, the expression list
   will be empty.  As an example of a function that can return
   multiple values, consider
#v+
        define sum_and_diff (x, y)
        {
            variable sum, diff;

            sum = x + y;  diff = x - y;
            return sum, diff;
        }
#v-
   which is a function returning two values.

   It is extremely important to note that \em{the calling routine must
   explicitly handle all values returned by a function}.  Although
   some languages such as C do not have this restriction, \slang does
   and it is a direct result of a \slang function's ability to return
   many values and accept a variable number of parameters.  Examples
   of properly handling the above function include
#v+
       variable s, d;
       (s, d) = sum_and_diff (5, 4);  % ignore neither
       (s,) = sum_and_diff (5, 4);    % ignore diff
       (,) = sum_and_diff (5, 4);     % ignore both sum and diff
#v-
   See the section below on assignment statements for more information
   about this important point.

\sect{Multiple Assignment Statement} #%{{{

   \slang functions can return more than one value, e.g.,
#v+
       define sum_and_diff (x, y)
       {
          return x + y, x - y;
       }
#v-
   returns two values.  It accomplishes this by placing both values on
   the stack before returning.  If you understand how \slang functions
   handle a variable number of parameters (section ???), then it
   should be rather obvious that one assigns such values to variables.
   One way is to use, e.g.,
#v+ 
      sum_and_diff (9, 4);
      d = ();
      s = ();
#v-

   However, the most convenient way to accomplish this is to use a
   \em{multiple assignment statement} such as 
#v+
       (s, d) = sum_and_diff (9, 4);
#v-
   The most general form of the multiple assignment statement is
#v+
     ( var_1, var_2, ..., var_n ) = expression;
#v-
   In fact, internally the interpreter transforms this statement into
   the form
#v+
     expression; var_n = (); ... var_2 = (); var_1 = ();
#v-
   for further processing.

   If you do not care about one of return values, simply omit the
   variable name from the list.  For example,
#v+
        (s, ) = sum_and_diff (9, 4);
#v-
   assigns the sum of \exmp{9} and \exmp{4} to \var{s} and the
   difference (\exmp{9-4}) will be removed from the stack.

   As another example, the \jed editor provides a function called
   \var{down} that takes an integer argument and returns an integer.
   It is used to move the current editing position down the number of
   lines specified by the argument passed to it.  It returns the number
   of lines it successfully moved the editing position.  Often one does
   not care about the return value from this function.  Although it is
   always possible to handle the return value via
#v+
       variable dummy = down (10);
#v-
   it is more convenient to use a multiple assignment expression and
   omit the variable name, e.g.,
#v+
       () = down (10);
#v-

   Some functions return a \em{variable number} of values instead of a
   \em{fixed number}.  Usually, the value at the top of the stack will
   indicate the actual number of return values.  For such functions,
   the multiple assignment statement cannot directly be used.  To see
   how such functions can be dealt with, consider the following
   function:
#v+
     define read_line (fp)
     {
        variable line;
        if (-1 == fgets (&line, fp))
          return -1;
        return (line, 0);
     }
#v-
   This function returns either one or two values, depending upon the
   return value of \var{fgets}.  Such a function may be handled as in
   the following example:
#v+
      status = read_line (fp);
      if (status != -1)
        {
           s = ();
           .
           .
        }
#v-
   In this example, the \em{last} value returned by \var{read_line} is
   assigned to \var{status} and then tested.  If it is non-zero, the
   second return value is assigned to \var{s}.  In particular note the
   empty set of parenthesis in the assignment to \var{s}.  This simply
   indicates that whatever is on the top of the stack when the
   statement is executed will be assigned to \var{s}.

   Before leaving this section it is important to reiterate the fact
   that if a function returns a value, the caller must deal with that
   return value.  Otherwise, the value will continue to live onto the
   stack and may eventually lead to a stack overflow error.
   Failing to handle the return value of a function is the
   most common mistake that inexperienced \slang programmers make.
   For example, the \var{fflush} function returns a value that many C
   programmer's never check.  Instead of writing
#v+
      fflush (fp);
#v-
   as one could in C, a \slang programmer should write
#v+
      () = fflush (fp);
#v-
   in \slang.  (Many good C programmer's write \exmp{(void)fflush(fp)}
   to indicate that the return value is being ignored).

#%}}}

\sect{Exit-Blocks}

   An \em{exit-block} is a set of statements that get executed when a
   functions returns.  They are very useful for cleaning up when a
   function returns via an explicit call to \var{return} from deep
   within a function.
   
   An exit-block is created by using the \kw{EXIT_BLOCK} keyword
   according to the syntax 
\begin{tscreen}
      EXIT_BLOCK { \em{statement-list} }
\end{tscreen}
   where \em{statement-list} represents the list of statements that
   comprise the exit-block.  The following example illustrates the use
   of an exit-block:
#v+
      define simple_demo ()
      {
         variable n = 0;

         EXIT_BLOCK { message ("Exit block called."); }

         forever
          {
            if (n == 10) return;
            n++;
          }
      }
#v-
   Here, the function contains an exit-block and a \var{forever} loop.
   The loop will terminate via the \kw{return} statement when \var{n}
   is 10.  Before it returns, the exit-block will get executed.

   A function can contain multiple exit-blocks, but only the last
   one encountered during execution will actually get executed.  For
   example,
#v+
      define simple_demo (n)
      {
         EXIT_BLOCK { return 1; }
         
         if (n != 1)
           {
              EXIT_BLOCK { return 2; }
           }
         return;
      }
#v-
   If \var{1} is passed to this function, the first exit-block will
   get executed because the second one would not have been encountered
   during the execution.  However, if some other value is passed, the
   second exit-block would get executed.  This example also
   illustrates that it is possible to explicitly return from an
   exit-block, although nested exit-blocks are illegal.

#%}}}

\chapter{Name Spaces} #%{{{

  By default, all global variables and functions are defined in the
  global namespace.  In addition to the global namespace, every
  compilation unit (e.g., a file containing \slang code) has an
  anonymous namespace.  Objects may be defined in the anonymous
  namespace via the \var{static} declaration keyword.  For example,
#v+
     static variable x;
     static define hello () { message ("hello"); }
#v-
  defines a variable \var{x} and a function \var{hello} in the
  anonymous namespace.  This is useful when one wants to define
  functions and variables that are only to be used within the file, or
  more precisely the compilation unit, that defines them.

  The \var{implements} function may be used to give the anonymous
  namespace a name to allow access to its objects from outside the
  compilation unit that defines them.  For example,
#v+
     implements ("foo");
     static variable x;
#v-
  allows the variable \var{x} to be accessed via \var{foo->x}, e.g.,
#v+
     if (foo->x == 1) foo->x = 2;
#v-     

  The \var{implements} function does more than simply giving the
  anonymous namespace a name.  It also changes the default variable
  and function declaration mode from \var{public} to \var{static}.
  That is, 
#v+
     implements ("foo");
     variable x;
#v-
  and
#v+
     implements ("foo");
     static variable x;
#v-
  are equivalent.  Then to create a public object within the
  namespace, one must explicitly use the \var{public} keyword.

  Finally, the \var{private} keyword may be used to create an object
  that is truly private within the compilation unit.  For example,
#v+
    implements ("foo");
    variable x;
    private variable y;
#v-
  allows \var{x} to be accessed from outside the namespace via
  \var{foo->x}, however \var{y} cannot be accessed.

#%}}}

\chapter{Arrays} #%{{{

   An array is a container object that can contain many values of one
   data type.  Arrays are very useful objects and are indispensable
   for certain types of programming.  The purpose of this chapter is
   to describe how arrays are defined and used in the \slang language.
   
\sect{Creating Arrays} #%{{{

   The \slang language supports multi-dimensional arrays of all data
   types.  Since the \var{Array_Type} is a data type, one can even
   have arrays of arrays.  To create a multi-dimensional array of
   \em{SomeType} use the syntax
#v+
      SomeType [dim0, dim1, ..., dimN]
#v-
   Here \em{dim0}, \em{dim1}, ... \em{dimN} specify the size of
   the individual dimensions of the array.  The current implementation
   permits arrays consist of up to \var{7} dimensions.  When a
   numeric array is created, all its elements are initialized to zero.
   The initialization of other array types depend upon the data type,
   e.g., \var{String_Type} and \var{Struct_Type} arrays are
   initialized to \var{NULL}.

   As a concrete example, consider
#v+
     a = Integer_Type [10];
#v-
   which creates a one-dimensional array of \exmp{10} integers and
   assigns it to \var{a}.
   Similarly, 
#v+
     b = Double_Type [10, 3];
#v-
   creates a \var{30} element array of double precision numbers
   arranged in \var{10} rows and \var{3} columns, and assigns it to
   \var{b}.

\sect1{Range Arrays}

   There is a more convenient syntax for creating and initializing a
   1-d arrays.  For example, to create an array of ten
   integers whose elements run from \exmp{1} through \exmp{10}, one
   may simply use:
#v+
     a = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
#v-
   Similarly, 
#v+
     b = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0];
#v-
   specifies an array of ten doubles.  
   
   An even more compact way of specifying a numeric array is to use a
   \em{range-array}.  For example,
#v+
     a = [0:9];
#v-
   specifies an array of 10 integers whose elements range from \var{0}
   through \var{9}.  The most general form of a range array is
#v+
     [first-value : last-value : increment]
#v-
   where the \em{increment} is optional and defaults to \exmp{1}. This
   creates an array whose first element is \em{first-value} and whose
   successive values differ by \em{increment}.  \em{last-value} sets
   an upper limit upon the last value of the array as described below.

   If the range array \var{[a:b:c]} is integer valued, then the
   interval specified by \var{a} and \var{b} is closed.  That is, the
   kth element of the array \math{x_k} is given by \math{x_k=a+ck} and
   must satisfy \math{a<=x_k<=b}.  Hence, the number of elements in an
   integer range array is given by the expression \math{1 + (b-a)/c}.

   The situation is somewhat more complicated for floating point range
   arrays.  The interval specified by a floating point range array
   \var{[a:b:c]} is semi-open such that \var{b} is not contained in
   the interval.  In particular, the kth element of \var{[a:b:c]} is
   given by \math{x_k=a+kc} such that \math{a<=x_k<b} when
   \math{c>=0}, and \math{b<x_k<=a} otherwise.  The number of elements
   in the array is one greater than the largest \math{k} that
   satisfies the open interval constraint.

   Here are a few examples that illustrate the above comments:
#v+
       [1:5:1]         ==> [1,2,3,4,5]
       [1.0:5.0:1.0]   ==> [1.0, 2.0, 3.0, 4.0]
       [5:1:-1]        ==> [5,4,3,2,1]
       [5.0:1.0:-1.0]  ==> [5.0, 4.0, 3.0, 2.0];
       [1:1]           ==> [1]
       [1.0:1.0]       ==> []
       [1:-3]          ==> []
#v-

\sect1{Creating arrays via the dereference operator}

   Another way to create an array is apply the dereference operator
   \var{@} to the \var{DataType_Type} literal \var{Array_Type}.  The
   actual syntax for this operation resembles a function call
\begin{tscreen}
     variable a = @Array_Type (\em{data-type}, \em{integer-array});
\end{tscreen}
  where \em{data-type} is of type \var{DataType_Type} and
  \em{integer-array} is a 1-d array of integers that specify the size
  of each dimension.  For example,
#v+
     variable a = @Array_Type (Double_Type, [10, 20]);
#v-
  will create a \exmp{10} by \var{20} array of doubles and assign it
  to \var{a}.  This method of creating arrays derives its power from
  the fact that it is more flexible than the methods discussed in this
  section.  We shall encounter it again in section ??? in the context
  of the \var{array_info} function.

#%}}}

\sect{Reshaping Arrays} #%{{{
   It is sometimes possible to change the `shape' of an array using
   the \var{reshape} function.  For example, a 1-d 10 element array
   may be reshaped into a 2-d array consisting of 5 rows and 2
   columns.  The only restriction on the operation is that the arrays
   must be commensurate.  The \var{reshape} function follows the
   syntax
\begin{tscreen}
       reshape (\em{array-name}, \em{integer-array});
\end{tscreen}
   where \em{array-name} specifies the array to be reshaped to have
   the dimensions given by \var{integer-array}, a 1-dimensional array of
   integers.  It is important to note that this does \em{not} create a
   new array, it simply reshapes the existing array.  Thus,
#v+
       variable a = Double_Type [100];
       reshape (a, [10, 10]);
#v-
   turns \var{a} into a \exmp{10} by \exmp{10} array.

#%}}}

\sect{Indexing Arrays} #%{{{
   An individual element of an array may be referred to by its
   \em{index}.  For example, \exmp{a[0]} specifies the zeroth element
   of the one dimensional array \var{a}, and \exmp{b[3,2]} specifies
   the element in the third row and second column of the two
   dimensional array \var{b}.  As in C array indices are numbered from
   \var{0}.  Thus if \var{a} is a one-dimensional array of ten
   integers, the last element of the array is given by \var{a[9]}.
   Using \var{a[10]} would result in a range error.
  
   A negative index may be used to index from the end of the array,
   with \exmp{a[-1]} referring to the last element of \var{a},
   \exmp{a[-2]} referring to the next to the last element, and so on.

   One may use the indexed value like any other variable.  For
   example, to set the third element of an integer array to \var{6}, use
#v+
     a[2] = 6;
#v-
   Similarly, that element may be used in an expression, such as
#v+
     y = a[2] + 7;
#v-
   Unlike other \slang variables which inherit a type upon assignment,
   array elements already have a type.  For example, an attempt to
   assign a string value to an element of an integer array will result
   in a type-mismatch error.

   One may use any integer expression to index an array.  A simple
   example that computes the sum of the elements of 10 element 1-d
   array is
#v+
      variable i, s;
      s = 0;
      for (i = 0; i < 10; i++) s += a[i];
#v-
   However, if the built-in \var{sum} function is available (not all programs
   using \slang support this), then it should be used to compute the
   sum of an array, e.g.,
#v+
     s = sum(a);
#v-

   Unlike many other languages, \slang permits arrays to be indexed by
   other integer arrays.   Suppose that \var{a} is a 1-d array of 10
   doubles.  Now consider:
#v+
      i = [6:8];
      b = a[i];
#v-
   Here, \var{i} is a 1-dimensional range array of three integers with
   \exmp{i[0]} equal to \exmp{6}, \exmp{i[1]} equal to \exmp{7},
   and \exmp{i[2]} equal to \exmp{8}.  The statement \var{b = a[i];}
   will create a 1-d array of three doubles and assign it to \var{b}.
   The zeroth element of \var{b}, \exmp{b[0]} will be set to the sixth
   element of \var{a}, or \exmp{a[6]}, and so on.  In fact, these two simple
   statements are equivalent to
#v+
     b = Double_Type [3];
     b[0] = a[6];
     b[1] = a[7];
     b[2] = a[8];
#v-
   except that using an array of indices is not only much more
   convenient, but executes much faster.

   More generally, one may use an index array to specify which
   elements are to participate in a calculation.  For example, consider
#v+
     a = Double_Type [1000];
     i = [0:499];
     j = [500:999];
     a[i] = -1.0;
     a[j] = 1.0;
#v-
   This creates an array of \exmp{1000} doubles and sets the first
   \exmp{500} elements to \exmp{-1.0} and the last \exmp{500} to
   \var{1.0}.  Actually, one may do away with the \var{i} and \var{j}
   variables altogether and use
#v+
     a = Double_Type [1000];
     a [[0:499]] = -1.0;
     a [[500:999]] = 1.0;
#v-
   It is important to understand the syntax used and, in particular,
   to note that \exmp{a[[0:499]]} is \em{not} the same as
   \exmp{a[0:499]}.  In fact, the latter will generate a syntax error.
   
   Often, it is convenient to use a \em{rubber} range to specify
   indices.  For example, \exmp{a[[500:]]} specifies all elements of
   \var{a} whose index is greater than or equal to \var{500}.  Similarly,
   \exmp{a[[:499]]} specifies the first 500 elements of \var{a}.
   Finally, \exmp{a[[:]]} specifies all the elements of \var{a};
   however, using \exmp{a[*]} is more convenient.

   One should be careful when using index arrays with negative
   elements.  As pointed out above, a negative index is used to index
   from the end of the array.  That is, \exmp{a[-1]} refers to the
   last element of \exmp{a}.  How should \exmp{a[[[0:-1]]} be
   interpreted?  By itself, \var{[0:-1]} is an empty array; hence, one
   might expect \exmp{a[[0:-1]]} to refer to no elements.  However,
   when used in an array indexing context, \exmp{[0:-1]} is
   interpreted as an array indexing the first through the last
   elements of the array.  While this is a very convenient mechanism
   to specifiy the last 3 elements of an array using
   \exmp{a[[-3:-1]]}, it is very easy to forget these semantics.

   Now consider a multi-dimensional array.  For simplicity, suppose
   that \var{a} is a \exmp{100} by \exmp{100} array of doubles.  Then
   the expression \var{a[0, *]} specifies all elements in the zeroth
   row.  Similarly, \var{a[*, 7]} specifies all elements in the
   seventh column.  Finally, \var{a[[3:5][6:12]]} specifies the
   \exmp{3} by \exmp{7} region consisting of rows \exmp{3}, \exmp{4},
   and \exmp{5}, and columns \exmp{6} through \exmp{12} of \var{a}.

   We conclude this section with a few examples.

   Here is a function that computes the trace (sum of the diagonal
   elements) of a square 2 dimensional \var{n} by \var{n} array:
#v+
      define array_trace (a, n)
      {
         variable s = 0, i;
         for (i = 0; i < n; i++) s += a[i, i];
         return s;
      }
#v-
   This fragment creates a \exmp{10} by \exmp{10} integer array, sets
   its diagonal elements to \exmp{5}, and then computes the trace of
   the array: 
#v+
      a = Integer_Type [10, 10];
      for (j = 0; j < 10; j++) a[j, j] = 5;
      the_trace = array_trace(a, 10);
#v-
   We can get rid of the \kw{for} loop as follows:
#v+
      j = Integer_Type [10, 2];
      j[*,0] = [0:9];
      j[*,1] = [0:9];
      a[j] = 5;
#v-
   Here, the goal was to construct a 2-d array of indices that
   correspond to the diagonal elements of \var{a}, and then use that
   array to index \var{a}.  To understand how
   this works, consider the middle statements.  They are equivalent
   to the following \var{for} loops:
#v+
      variable i;
      for (i = 0; i < 10; i++) j[i, 0] = i;
      for (i = 0; i < 10; i++) j[i, 1] = i;
#v-
   Thus, row \var{n} of \var{j} will have the value \exmp{(n,n)},
   which is precisely what was sought.
   
   Another example of this technique is the function:
#v+
      define unit_matrix (n)
      {
         variable a = Integer_Type [n, n];
         variable j = Integer_Type [n, 2];
         j[*,0] = [0:n - 1];
         j[*,1] = [0:n - 1];
         
         a[j] = 1;
         return a;
      }
#v-
   This function creates an \var{n} by \var{n} unit matrix,
   that is a 2-d \var{n} by \var{n} array whose elements are all zero
   except on the diagonal where they have a value of \exmp{1}.


#%}}}

\sect{Arrays and Variables}

   When an array is created and assigned to a variable, the
   interpreter allocates the proper amount of space for the array,
   initializes it, and then assigns to the variable a \em{reference}
   to the array.   So, a variable that represents an array has a value
   that is really a reference to the array.  This has several
   consequences, some good and some bad.  It is believed that the
   advantages of this representation outweigh the disadvantages.
   First, we shall look at the positive aspects.

   When a variable is passed to a function, it is always the value of
   the variable that gets passed.  Since the value of a variable
   representing an array is a reference, a reference to the array gets
   passed.  One major advantage of this is rather obvious: it is a
   fast and efficient way to pass the array.  This also has another
   consequence that is illustrated by the function
#v+
      define init_array (a, n)
      {
         variable i;
         
         for (i = 0; i < n; i++) a[i] = some_function (i);
      }
#v-
   where \var{some_function} is a function that generates a scalar
   value to initialize the \em{ith} element.  This function can be
   used in the following way:
#v+
      variable X = Double_Type [100000];
      init_array (X, 100000);
#v-
   Since the array is passed to the function by reference, there is no
   need to make a separate copy of the \var{100000} element array. As
   pointed out above, this saves both execution time and memory. The
   other salient feature to note is that any changes made to the
   elements of the array within the function will be manifested in the
   array outside the function.  Of course, in this case, this is a
   desirable side-effect.

   To see the downside of this representation, consider:
#v+
      variable a, b;
      a = Double_Type [10];
      b = a;
      a[0] = 7;
#v-
   What will be the value of \exmp{b[0]}?  Since the value of \var{a}
   is really a reference to the array of ten doubles, and that
   reference was assigned to \var{b}, \var{b} also refers to the same
   array.  Thus any changes made to the elements of \var{a}, will also
   be made implicitly to \var{b}.

   This begs the question: If the assignment of one variable which
   represents an array, to another variable results in the assignment
   of a reference to the array, then how does one make separate copies
   of the array?  There are several answers including using an index
   array, e.g., \exmp{b = a[*]}; however, the most natural method is
   to use the dereference operator:
#v+
      variable a, b;
      a = Double_Type [10];
      b = @a;
      a[0] = 7;
#v-
   In this example, a separate copy of \var{a} will be created and
   assigned to \var{b}.  It is very important to note that \slang
   never implicitly dereferences an object.  So, one must explicitly use
   the dereference operator.  This means that the elements of a
   dereferenced array are not themselves dereferenced.  For example,
   consider dereferencing an array of arrays, e.g.,
#v+
      variable a, b;
      a = Array_Type [2];  
      a[0] = Double_Type [10];
      a[1] = Double_Type [10];
      b = @a;
#v-
   In this example, \exmp{b[0]} will be a reference to the array that
   \exmp{a[0]} references because \exmp{a[0]} was not explicitly
   dereferenced.

\sect{Using Arrays in Computations} #%{{{

   Many functions and operations work transparently with arrays.
   For example, if \var{a} and \var{b} are arrays, then the sum
   \exmp{a + b} is an array whose elements are formed from the sum of
   the corresponding elements of \var{a} and \var{b}.  A similar
   statement holds for all other binary and unary operations.

   Let's consider a simple example.  Suppose, that we wish to solve a
   set of \var{n} quadratic equations whose coefficients are given by
   the 1-d arrays \var{a}, \var{b}, and \var{c}.  In general, the
   solution of a quadratic equation will be two complex numbers.  For
   simplicity, suppose that all we really want is to know what subset of
   the coefficients, \var{a}, \var{b}, \var{c}, correspond to
   real-valued solutions.  In terms of \var{for} loops, we can write:
#v+
     variable i, d, index_array;
     index_array = Integer_Type [n];
     for (i = 0; i < n; i++)
       {
          d = b[i]^2 - 4 * a[i] * c[i];
          index_array [i] = (d >= 0.0);
       }
#v-
   In this example, the array \var{index_array} will contain a
   non-zero value if the corresponding set of coefficients has a
   real-valued solution.  This code may be written much more compactly
   and with more clarity as follows:
#v+
     variable index_array = ((b^2 - 4 * a * c) >= 0.0);
#v-

   \slang has a powerful built-in function called \var{where}.  This
   function takes an array of integers and returns a 2-d array of
   indices that correspond to where the elements of the input array
   are non-zero.  This simple operation is extremely useful. For
   example, suppose \var{a} is a 1-d array of \var{n} doubles, and it
   is desired to set to zero all elements of the array whose value is
   less than zero. One way is to use a \var{for} loop:
#v+
     for (i = 0; i < n; i++) 
       if (a[i] < 0.0) a[i] = 0.0;
#v-
   If \var{n} is a large number, this statement can take some time to
   execute.  The optimal way to achieve the same result is to use the
   \var{where} function:
#v+
     a[where (a < 0.0)] = 0;
#v-
   Here, the expression \exmp{(a < 0.0)} returns an array whose
   dimensions are the same size as \var{a} but whose elements are
   either \exmp{1} or \exmp{0}, according to whether or not the
   corresponding element of \var{a} is less than zero.  This array of
   zeros and ones is then passed to \var{where} which returns a 2-d
   integer array of indices that indicate where the elements of
   \var{a} are less than zero.  Finally, those elements of \var{a} are
   set to zero.

   As a final example, consider once more the example involving the set of
   \var{n} quadratic equations presented above.  Suppose that we wish
   to get rid of the coefficients of the previous example that
   generated non-real solutions.  Using an explicit \var{for} loop requires
   code such as:
#v+
     variable i, j, nn, tmp_a, tmp_b, tmp_c;
     
     nn = 0;
     for (i = 0; i < n; i++) 
       if (index_array [i]) nn++;
     
     tmp_a = Double_Type [nn];
     tmp_b = Double_Type [nn];
     tmp_c = Double_Type [nn];
     
     j = 0;
     for (i = 0; i < n; i++)
       {
          if (index_array [i]) 
            {
               tmp_a [j] = a[i];
               tmp_b [j] = b[i];
               tmp_c [j] = c[i];
               j++;
            }
       }
     a = tmp_a;
     b = tmp_b;
     c = tmp_c;
#v-
   Not only is this a lot of code, it is also clumsy and error-prone.
   Using the \var{where} function, this task is trivial:
#v+
     variable i;
     i = where (index_array != 0);
     a = a[i];
     b = b[i];
     c = c[i];
#v-

   All the examples up to now assumed that the dimensions of the array
   were known.  Although the intrinsic function \var{length} may be
   used to get the total number of elements of an array, it cannot be
   used to get the individual dimensions of a multi-dimensional array.
   However, the function \var{array_info} may be used to
   get information about an array, such as its data type and size.
   The function returns three values: the data type, the number of
   dimensions, and an integer array containing the size
   of each dimension.  It may be used to determine the number of rows
   of an array as follows:
#v+
     define num_rows (a)
     {
        variable dims, type, num_dims;
        
        (dims, num_dims, type) = array_info (a);
        return dims[0];
     }  
#v-
   The number of columns may be obtained in a similar manner:
#v+
     define num_cols (a)
     {
        variable dims, type, num_dims;
        
        (dims, num_dims, type) = array_info (a);
        if (num_dims > 1) return dims[1];
        return 1;
     }     
#v-
   
   Another use of \var{array_info} is to create an array that has the
   same number of dimensions as another array:
#v+
     define make_int_array (a)
     {
        variable dims, num_dims, type;
        
        (dims, num_dims, type) = array_info (a);
        return @Array_Type (Integer_Type, dims);
     }
#v-

#%}}}

#%}}}

\chapter{Associative Arrays} #%{{{

   An associative array differs from an ordinary array in the sense
   that its size is not fixed and that is indexed by a string, called
   the \em{key}. For example, consider:
#v+
       variable A = Assoc_Type [Integer_Type];
       A["alpha"] = 1;
       A["beta"] = 2;
       A["gamma"] = 3;
#v-
   Here, \var{A} represents an associative array of integers
   (\var{Integer_Type}) and three keys have been added to the array.
   
   As the example suggests, an associative array may be created using
   one of the following forms:
\begin{tscreen}
      Assoc_Type [\em{type}]
      Assoc_Type [\em{type}, \em{default-value}]
      Assoc_Type []
\end{tscreen}
   The last form returns an associative array of \var{Any_Type}
   objects allowing any type of object to may be stored in
   the array.

   The form involving a \em{default-value} is useful for associating a
   default value for non-existent array members.  This feature is
   explained in more detail below.

   There are several functions that are specially designed to work
   with associative arrays.  These include:
\begin{itemize}
\item \var{assoc_get_keys}, which returns an ordinary array of strings
      containing the keys in the array.
   
\item \var{assoc_get_values}, which returns an ordinary array of the
      values of the associative array.
   
\item \var{assoc_key_exists}, which can be used to determine whether
      or not a key exists in the array.

\item \var{assoc_delete_key}, which may be used to remove a key (and
      its value) from the array.
\end{itemize}

   To illustrate the use of an associative array, consider the problem
   of counting the number of repeated occurrences of words in a list.
   Let the word list be represented as an array of strings given by
   \var{word_list}.  The number of occurrences of each word may be
   stored in an associative array as follows:
#v+
     variable a, word;
     a = Assoc_Type [Integer_Type];
     foreach (word_list)
       {
          word = ();
          if (0 == assoc_key_exists (a, word))
            a[word] = 0;
          a[word]++;  % same as a[word] = a[word] + 1;
       }
#v-
   Note that \var{assoc_key_exists} was necessary to determine whether
   or not a word was already added to the array in order to properly
   initialize it.  However, by creating the associative array with a
   default value of \exmp{0}, the above code may be simplified to
#v+
     variable a, word;
     a = Assoc_Type [Integer_Type, 0];
     foreach (word_list)
       {
          word = ();
          a[word]++;
       }
#v-


#%}}}

\chapter{Structures and User-Defined Types} #%{{{

   A \em{structure} is a heterogeneous container object, i.e., it is
   an object with elements whose values do not have to be of the same
   data type.  The elements or fields of a structure are named, and
   one accesses a particular field of the structure via the field
   name. This should be contrasted with an array whose values are of
   the same type, and whose elements are accessed via array indices.

   A \em{user-defined} data type is a structure with a fixed set of
   fields defined by the user.
   
\sect{Defining a Structure}

   The \kw{struct} keyword is used to define a structure.  The syntax
   for this operation is:
\begin{tscreen}
     struct {\em{field-name-1}, \em{field-name-2}, ... \em{field-name-N}};
\end{tscreen}
   This creates and returns a structure with \em{N} fields whose names
   are specified by \em{field-name-1}, \em{field-name-2}, ...,
   \em{field-name-N}.  When a structure is created, all its fields are
   initialized to \var{NULL}.

   For example,
#v+
     variable t = struct { city_name, population, next };
#v-
   creates a structure with three fields and assigns it to the
   variable \var{t}.

   Alternatively, a structure may be created by dereferencing
   \var{Struct_Type}.  For example, the above structure may also be
   created using one of the two forms:
#v+
      t = @Struct_Type ("city_name", "population", "next");
      t = @Struct_Type (["city_name", "population", "next"]);
#v-
   These are useful when creating structures dynamically where one does
   not know the name of the fields until run-time.

   Like arrays, structures are passed around via a references.  Thus,
   in the above example, the value of \var{t} is a reference to the
   structure.  This means that after execution of
#v+
     variable u = t;
#v-
   \em{both} \var{t} and \var{u} refer to the \em{same} structure,
   since only the reference was used in the assignment.  To actually
   create a new copy of the structure, use the \em{dereference}
   operator, e.g.,
#v+
     variable u = @t;
#v-

\sect{Accessing the Fields of a Structure}

   The dot (\var{.}) operator is used to specify the particular
   field of structure.  If \var{s} is a structure and \var{field_name}
   is a field of the structure, then \exmp{s.field_name} specifies
   that field of \var{s}.  This specification can be used in
   expressions just like ordinary variables.  Again, consider
#v+
     variable t = struct { city_name, population, next };
#v-
   described in the last section.  Then,
#v+
     t.city_name = "New York";
     t.population = 13000000;
     if (t.population > 200) t = t.next;
#v-
   are all valid statements involving the fields of \var{t}.

\sect{Linked Lists}

  One of the most important uses of structures is to create a
  \em{dynamic} data structure such as a \em{linked-list}.  A
  linked-list is simply a chain of structures that are linked together
  such that one structure in the chain is the value of a field of the
  previous structure in the chain.  To be concrete, consider the
  structure discussed earlier:
#v+
     variable t = struct { city_name, population, next };
#v-
  and suppose that we desire to create a list of such structures.
  The purpose of the \var{next} field is to provide the link to the
  next structure in the chain.  Suppose that there exists a function,
  \var{read_next_city}, that reads city names and populations from a
  file.  Then we can create the list via:
#v+
     define create_population_list ()
     {
        variable city_name, population, list_root, list_tail;
        variable next;
        
        list_root = NULL;
        while (read_next_city (&city_name, &population))
          {
             next = struct {city_name, population, next };

             next.city_name = city_name;
             next.population = population;
             next.next = NULL;

             if (list_root == NULL)
               list_root = next;
             else
               list_tail.next = next;
               
             list_tail = next;
          }
        return list_root;
     }
#v-
  In this function, the variables \var{list_root} and \var{list_tail}
  represent the beginning and end of the list, respectively. As long
  as \var{read_next_city} returns a non-zero value, a new structure is
  created, initialized, and then appended to the list via the
  \var{next} field of the \var{list_tail} structure.  On the first
  time through the loop, the list is created via the assignment to the
  \var{list_root} variable.  
  
  This function may be used as follows:
#v+
    variable Population_List = create_population_list ();
    if (Population_List == NULL) error ("List is empty");
#v-
  We can create other functions that manipulate the list.  An example is
  a function that finds the city with the largest population:
#v+
    define get_largest_city (list)
    {
       variable largest;

       largest = list;
       while (list != NULL)
         {
            if (list.population > largest.population)
              largest = list;
            list = list.next;
         }
       return largest.city_name;
    }
    
    vmessage ("%s is the largest city in the list", 
               get_largest_city (Population_List)));
#v-
  The \var{get_largest_city} is a typical example of how one traverses
  a linear linked-list by starting at the head of the list and
  successively moves to the next element of the list via the
  \var{next} field.
  
  In the previous example, a \kw{while} loop was used to traverse the
  linked list.  It is faster to use a \kw{foreach} loop for this:
#v+
    define get_largest_city (list)
    {
       variable largest, elem;

       largest = list;
       foreach (list)
         {
            elem = ();
            if (item.population > largest.population)
              largest = item;
         }
       return largest.city_name;
    }
#v-  
  Here a \kw{foreach} loop has been used to walk the list via its
  \exmp{next} field.  If the field name was not \exmp{next}, then it
  would have been necessary to use the \kw{using} form of the
  \kw{foreach} statement.  For example, if the field name implementing the
  linked list was \exmp{next_item}, then 
#v+
     foreach (list) using ("next_item")
     {
        elem = ();
        .
        .
     }
#v-
  would have been used.  In other words, unless otherwise indicated
  via the \kw{using} clause, \kw{foreach} walks the list using a field
  named \exmp{next}.
  
  Now consider a function that sorts the list according to population.
  To illustrate the technique, a \em{bubble-sort} will be used, not
  because it is efficient, it is not, but because it is simple and
  intuitive.
#v+
    define sort_population_list (list)
    {
       variable changed;
       variable node, next_node, last_node;
       do
         {
            changed = 0;
            node = list;
            next_node = node.next;
            last_node = NULL;
            while (next_node != NULL)
              {
                 if (node.population < next_node.population)
                   {
                      % swap node and next_node
                      node.next = next_node.next;
                      next_node.next = node;
                      if (last_node != NULL)
                        last_node.next = next_node;
                      
                      if (list == node) list = next_node;
                      node = next_node;
                      next_node = node.next;
                      changed++;
                   }
                 last_node = node;
                 node = next_node;
                 next_node = next_node.next;
              }
         }
       while (changed);
       
       return list;
    }
#v-
   Note the test for equality between \var{list} and \var{node}, i.e.,
#v+
                      if (list == node) list = next_node;
#v-
   It is important to appreciate the fact that the values of these
   variables are references to structures, and that the 
   comparison only compares the references and \em{not} the actual
   structures they reference.  If it were not for this, the algorithm
   would fail.
   
\sect{Defining New Types}

   A user-defined data type may be defined using the \kw{typedef}
   keyword.  In the current implementation, a user-defined data type
   is essentially a structure with a user-defined set of fields. For
   example, in the previous section a structure was used to represent
   a city/population pair.  We can define a data type called
   \var{Population_Type} to represent the same information:
#v+
      typedef struct 
      {
         city_name, 
         population
      } Population_Type;
#v-
   This data type can be used like all other data types.  For example,
   an array of Population_Type types can be created,
#v+
      variable a = Population_Type[10];
#v-
   and `populated' via expressions such as
#v+
      a[0].city_name = "Boston";
      a[0].population = 2500000;
#v-
   The new type \var{Population_Type} may also be used with the
   \var{typeof} function:
#v+
      if (Population_Type = typeof (a)) city = a.city_name;
#v-
   The dereference \var{@} may be used to create an instance of the
   new type:
#v+
     a = @Population_Type;
     a.city_name = "Calcutta";
     a.population = 13000000;
#v-


#%}}}

\chapter{Error Handling} #%{{{

   Many intrinsic functions signal errors in the event of failure.
   User defined functions may also generate an error condition via the
   \var{error} function.  Depending upon the severity of the error, it
   can be caught and cleared using a construct called an
   \em{error-block}.

\sect{Error-Blocks}

   When the interpreter encounters a recoverable run-time error, it
   will return to top-level by \em{unwinding} its function call
   stack.  Any error-blocks that it encounters as part of this
   unwinding process will get executed.  Errors such as syntax errors
   and memory allocation errors are not recoverable, and error-blocks
   will not get executed when such errors are encountered.

   An error-block is defined using the syntax
#v+
       ERROR_BLOCK { statement-list }
#v-
   where \em{statement-list} represents a list of statements that
   comprise the error-block.  A simple example of an error-block is
#v+
       define simple (a)
       {
          ERROR_BLOCK { message ("error-block executed"); }
          if (a) error ("Triggering Error");
          message ("hello");
       }
#v-
   Executing this function via \exmp{simple(0)} will result in the
   message \exmp{"hello"}.  However, calling it using \exmp{simple(1)}
   will generate an error that will be caught, but not cleared, by
   the error-block and the \exmp{"error-block executed"} message will
   result.
   
   Error-blocks are never executed unless triggered by an error.  The
   only exception to this is when the user explicitly indicates that
   the error-block in scope should execute.  This is indicated by the
   special keyword \var{EXECUTE_ERROR_BLOCK}.  For example,
   \var{simple} could be recoded as
#v+
       define simple (a)
       {
          variable err_string = "error-block executed";
          ERROR_BLOCK { message (err_string); }
          if (a) error ("Triggering Error");
          err_string = "hello";
          EXECUTE_ERROR_BLOCK;
       }
#v-
   Please note that \var{EXECUTE_ERROR_BLOCK} does not initiate an
   error condition; it simply causes the error-block to be executed
   and control will pass onto the next statement following the
   \var{EXECUTE_ERROR_BLOCK} statement.

\sect{Clearing Errors}

   Once an error has been caught by an error-block, the error can be cleared
   by the \var{_clear_error} function.  After the error has been cleared,
   execution will resume at the next statement at the level of the error block
   following the statement that generated the error.  For example, consider:
#v+
       define make_error ()
       {
           error ("Error condition created.");
           message ("This statement is not executed.");
       }

       define test ()
       {
           ERROR_BLOCK
             {
                _clear_error ();
             }
           make_error ();
           message ("error cleared.");
       }
#v-
   Calling \var{test} will trigger an error in the \var{make_error}
   function, but will get cleared in the \var{test} function.  The
   call-stack will unwind from \var{make_error} back into \var{test}
   where the error-block will get executed.  As a result, execution
   resumes after the statement that makes the call to \var{make_error}
   since this statement is at the same level as the error-block that
   cleared the error.

   Here is another example that illustrates how multiple error-blocks
   work:
#v+
       define example ()
       {
          variable n = 0, s = "";
          variable str;

          ERROR_BLOCK {
              str = sprintf ("s=%s,n=%d", s, n);
              _clear_error ();
          }

          forever
            {
              ERROR_BLOCK {
               s += "0";
               _clear_error ();
              }

              if (n == 0) error ("");

              ERROR_BLOCK {
               s += "1";
              }

              if (n == 1) error ("");
              n++;
            }
          return str;
       }
#v-
   Here, three error-blocks have been declared.  One has been declared
   outside the \var{forever} loop and the other two have been declared
   inside the \var{forever} loop.  Each time through the loop, the variable
   \var{n} is incremented and a different error-block is triggered.  The
   error-block that gets triggered is the last one encountered, since
   that will be the one in scope.  On the first time through the loop,
   \var{n} will be zero and the first error-block in the loop will get
   executed.  This error block clears the error and execution resumes
   following the \var{if} statement that triggered the error. The
   variable \var{n} will get incremented to \exmp{1} and, on the
   second cycle through the loop the second \var{if} statement
   will trigger an error causing the second error-block to execute.
   This time, the error is not cleared and the call-stack unwinds out
   of the \var{forever} loop, at which point the error-block outside
   the loop is in scope, causing it to execute. This error-block
   prints out the values of the variables \var{s} and \var{n}.  It
   will clear the error and execution resumes on the statement
   \em{following} the \var{forever} loop.  The result of this
   complicated series of events is that the function will return the
   string \exmp{"s=01,n=1"}. 

#%}}}

\chapter{Loading Files: evalfile and autoload} 

\chapter{File Input/Output} #%{{{

 \slang provides built-in supports for two different I/O facilities.
 The simplest interface is modeled upon the C language \var{stdio}
 streams interface and consists of functions such as \var{fopen},
 \var{fgets}, etc.  The other interface is modeled on a lower level
 POSIX interface consisting of functions such as \var{open},
 \var{read}, etc.  In addition to permitting more control, the lower
 level interface permits one to access network objects as well as disk
 files.

\sect{Input/Output via stdio}
\sect1{Stdio Overview}
 The \var{stdio} interface consists of the following functions:
\begin{itemize}
\item \var{fopen}, which opens a file for read or writing.

\item \var{fclose}, which closes a file opened by \var{fopen}.

\item \var{fgets}, used to read a line from the file.

\item \var{fputs}, which writes text to the file.

\item \var{fprintf}, used to write formatted text to the file.

\item \var{fwrite}, which may be used to write objects to the
       file.

\item \var{fread}, which reads a specified number of objects from
       the file.

\item \var{feof}, which is used to test whether the file pointer is at the
       end of the file.

\item \var{ferror}, which is used to see whether or not the stream
       associated with the file has an error.
  
\item \var{clearerr}, which clears the end-of-file and error
       indicators for the stream.

\item \var{fflush}, used to force all buffered data associated with
       the stream to be written out.

\item \var{ftell}, which is used to query the file position indicator
       of the stream.
       
\item \var{fseek}, which is used to set the position of the file
      position indicator of the stream.

\item \var{fgetslines}, which reads all the lines in a text file and
      returns them as an array of strings.

\end{itemize}

 In addition, the interface supports the \var{popen} and \var{pclose}
 functions on systems where the corresponding C functions are available.

 Before reading or writing to a file, it must first be opened using
 the \var{fopen} function.  The only exceptions to this rule involves
 use of the pre-opened streams: \var{stdin}, \var{stdout}, and
 \var{stderr}.  \var{fopen} accepts two arguments: a file name and a
 string argument that indicates how the file is to be opened, e.g.,
 for reading, writing, update, etc.  It returns a \var{File_Type}
 stream object that is used as an argument to all other functions of
 the \var{stdio} interface.  Upon failure, it returns \NULL.  See the
 reference manual for more information about \var{fopen}.

\sect1{Stdio Examples}

 In this section, some simple examples of the use of the \var{stdio}
 interface is presented.  It is important to realize that all the
 functions of the interface return something, and that return value
 must be dealt with.
 
 The first example involves writing a function to count the number of
 lines in a text file.  To do this, we shall read in the lines, one by
 one, and count them:
#v+
    define count_lines_in_file (file)
    {
       variable fp, line, count;
       
       fp = fopen (file, "r");    % Open the file for reading
       if (fp == NULL)
         verror ("%s failed to open", file);
         
       count = 0;
       while (-1 != fgets (&line, fp))
         count++;
         
       () = fclose (fp);
       return count;
    }
#v-
 Note that \exmp{&line} was passed to the \var{fgets} function.  When
 \var{fgets} returns, \var{line} will contain the line of text read in
 from the file.  Also note how the return value from \var{fclose} was
 handled.  
 
 Although the preceding example closed the file via \var{fclose},
 there is no need to explicitly close a file because \slang will
 automatically close the file when it is no longer referenced.  Since
 the only variable to reference the file is \var{fp}, it would have
 automatically been closed when the function returned.

 Suppose that it is desired to count the number of characters in the
 file instead of the number of lines.  To do this, the \var{while}
 loop could be modified to count the characters as follows:
#v+
      while (-1 != fgets (&line, fp))
        count += strlen (line);
#v-
 The main difficulty with this approach is that it will not work for
 binary files, i.e., files that contain null characters.  For such
 files, the file should be opened in \em{binary} mode via
#v+
      fp = fopen (file, "rb");
#v-
 and then the data read in using the \var{fread} function:
#v+
      while (-1 != fread (&line, Char_Type, 1024, fp))
           count += bstrlen (line);
#v-
 The \var{fread} function requires two additional arguments: the type
 of object to read (\var{Char_Type} in the case), and the number of
 such objects to read.  The function returns the number of objects
 actually read, or -1 upon failure.  The \var{bstrlen} function was
 used to compute the length of \var{line} because for \var{Char_Type}
 or \var{UChar_Type} objects, the \var{fread} function assigns a
 \em{binary} string (\var{BString_Type}) to \var{line}.

 The \kw{foreach} construct also works with \var{File_Type} objects.
 For example, the number of characters in a file may be counted via
#v+
     foreach (fp) using ("char")
     {
        ch = ();
        count++;
     }
#v-
 To count the number of lines, one can use:
#v+
     foreach (fp) using ("line")
     {
        line = ();
        num_lines++;
        count += strlen (line);
     }
#v-
 Often one is not interested in trailing whitespace in the lines of a
 file.   To have trailing whitespace automatically stripped from the
 lines as they are read in, use the \exmp{"wsline"} form, e.g.,
#v+
     foreach (fp) using ("wsline")
     {  
        line = ();
          .
          .
     }
#v-

 Finally, it should be mentioned that none of these examples should
 be used to count the number of bytes in a file when that
 information is more readily accessible by another means.  For
 example, it is preferable to get this information via the
 \var{stat_file} function:
#v+
     define count_chars_in_file (file)
     {
        variable st;
        
        st = stat_file (file);
        if (st == NULL)
          error ("stat_file failed.");
        return st.st_size;
     }
#v-

\sect{POSIX I/O}

\sect{Advanced I/O techniques}

  The previous examples illustrate how to read and write objects of a
  single data-type from a file, e.g.,
#v+
      num = fread (&a, Double_Type, 20, fp);
#v-
  would result in a \exmp{Double_Type[num]} array being assigned to
  \var{a} if successful.  However, suppose that the binary data file
  consists of numbers in a specified byte-order.  How can one read
  such objects with the proper byte swapping?  The answer is to use
  the \var{fread} function to read the objects as \var{Char_Type} and
  then \em{unpack} the resulting string into the specified data type,
  or types.  This process is facilitated using the \var{pack} and
  \var{unpack} functions.

  The \var{pack} function follows the syntax
\begin{tscreen}
    BString_Type pack (\em{format-string}, \em{item-list});
\end{tscreen}
  and combines the objects in the \em{item-list} according to
  \em{format-string} into a binary string and returns the result.
  Likewise, the \var{unpack} function may be used to convert a binary
  string into separate data objects:
\begin{tscreen}
   (\em{variable-list}) = unpack (\em{format-string}, \em{binary-string});
\end{tscreen}

  The format string consists of one or more data-type specification
  characters, and each may be followed by an optional decimal length
  specifier. Specifically, the data-types are specified according to
  the following table:
#v+
     c     char
     C     unsigned char
     h     short
     H     unsigned short
     i     int
     I     unsigned int
     l     long
     L     unsigned long
     j     16 bit int
     J     16 unsigned int
     k     32 bit int
     K     32 bit unsigned int
     f     float
     d     double
     F     32 bit float
     D     64 bit float
     s     character string, null padded
     S     character string, space padded
     x     a null pad character
#v-
  A decimal length specifier may follow the data-type specifier. With
  the exception of the \var{s} and \var{S} specifiers, the length
  specifier indicates how many objects of that data type are to be
  packed or unpacked from the string.  When used with the \var{s} or
  \var{S} specifiers, it indicates the field width to be used.  If the
  length specifier is not present, the length defaults to one.

  With the exception of \var{c}, \var{C}, \var{s}, \var{S}, and
  \var{x}, each of these may be prefixed by a character that indicates
  the byte-order of the object:
#v+
     >    big-endian order (network order)
     <    little-endian order
     =    native byte-order
#v-
  The default is native byte order.

  Here are a few examples that should make this more clear:
#v+
     a = pack ("cc", 'A', 'B');         % ==> a = "AB";
     a = pack ("c2", 'A', 'B');         % ==> a = "AB";
     a = pack ("xxcxxc", 'A', 'B');     % ==> a = "\0\0A\0\0B";
     a = pack ("h2", 'A', 'B');         % ==> a = "\0A\0B" or "\0B\0A"
     a = pack (">h2", 'A', 'B');        % ==> a = "\0\xA\0\xB"
     a = pack ("<h2", 'A', 'B');        % ==> a = "\0B\0A"
     a = pack ("s4", "AB", "CD");       % ==> a = "AB\0\0"
     a = pack ("s4s2", "AB", "CD");     % ==> a = "AB\0\0CD"
     a = pack ("S4", "AB", "CD");       % ==> a = "AB  "
     a = pack ("S4S2", "AB", "CD");     % ==> a = "AB  CD"
#v-
  
  When unpacking, if the length specifier is greater than one, then an
  array of that length will be returned.  In addition, trailing
  whitespace and null character are stripped when unpacking an object
  given by the \var{S} specifier.  Here are a few examples:
#v+
    (x,y) = unpack ("cc", "AB");         % ==> x = 'A', y = 'B'
    x = unpack ("c2", "AB");             % ==> x = ['A', 'B']
    x = unpack ("x<H", "\0\xAB\xCD");    % ==> x = 0xCDABuh
    x = unpack ("xxs4", "a b c\0d e f");  % ==> x = "b c\0"
    x = unpack ("xxS4", "a b c\0d e f");  % ==> x = "b c"
#v-

\sect1{Example: Reading /var/log/wtmp}

  Consider the task of reading the Unix system file
  \var{/var/log/utmp}, which contains login records about who logged
  onto the system.  This file format is documented in section 5 of the
  online Unix man pages, and consists of a sequence of entries
  formatted according to the C structure \var{utmp} defined in the
  \var{utmp.h} C header file.  The actual details of the structure
  may vary from one version of Unix to the other.  For the purposes of
  this example, consider its definition under the Linux operating
  system running on an Intel processor:
#v+
    struct utmp {
       short ut_type;              /* type of login */
       pid_t ut_pid;               /* pid of process */
       char ut_line[12];           /* device name of tty - "/dev/" */
       char ut_id[2];              /* init id or abbrev. ttyname */
       time_t ut_time;             /* login time */
       char ut_user[8];            /* user name */
       char ut_host[16];           /* host name for remote login */
       long ut_addr;               /* IP addr of remote host */
    };
#v-
  On this system, \var{pid_t} is defined to be an \var{int} and
  \var{time_t} is a \var{long}.  Hence, a format specifier for the
  \var{pack} and \var{unpack} functions is easily constructed to be:
#v+
     "h i S12 S2 l S8 S16 l"
#v-
  However, this particular definition is naive because it does not
  allow for structure padding performed by the C compiler in order to
  align the data types on suitable word boundaries.  Fortunately, the
  intrinsic function \var{pad_pack_format} may be used to modify a
  format by adding the correct amount of padding in the right places.
  In fact, \var{pad_pack_format} applied to the above format on an
  Intel-based Linux system produces the result:
#v+
     "h x2 i S12 S2 x2 l S8 S16 l"
#v-
  Here we see that 4 bytes of padding were added.

  The other missing piece of information is the size of the structure.
  This is useful because we would like to read in one structure at a
  time using the \var{fread} function.  Knowing the size of the
  various data types makes this easy; however it is even easier to use
  the \var{sizeof_pack} intrinsic function, which returns the size (in
  bytes) of the structure described by the pack format.
  
  So, with all the pieces in place, it is rather straightforward to
  write the code:
#v+
    variable format, size, fp, buf;
    
    typedef struct
    {
       ut_type, ut_pid, ut_line, ut_id,
       ut_time, ut_user, ut_host, ut_addr
    } UTMP_Type;

    format = pad_pack_format ("h i S12 S2 l S8 S16 l");
    size = sizeof_pack (format);

    define print_utmp (u)
    {
       
      () = fprintf (stdout, "%-16s %-12s %-16s %s\n",
		    u.ut_user, u.ut_line, u.ut_host, ctime (u.ut_time));
    }

       
   fp = fopen ("/var/log/utmp", "rb");
   if (fp == NULL)
     error ("Unable to open utmp file");

   () = fprintf (stdout, "%-16s %-12s %-16s %s\n",
                          "USER", "TTY", "FROM", "LOGIN@");

   variable U = @UTMP_Type;

   while (-1 != fread (&buf, Char_Type, size, fp))
     {
       set_struct_fields (U, unpack (format, buf));
       print_utmp (U);
     }

   () = fclose (fp);
#v-
  A few comments about this example are in order.  First of all, note
  that a new data type called \var{UTMP_Type} was created, although
  this was not really necessary.  We also opened the file in binary
  mode, but this too is optional under a Unix system where there is no
  distinction between binary and text modes. The \var{print_utmp}
  function does not print all of the structure fields.  Finally, last
  but not least, the return values from \var{fprintf} and \var{fclose}
  were dealt with.

#%}}}

\chapter{Debugging} #%{{{

 The current implementation provides no support for an interactive
 debugger, although a future version will.  Nevertheless, \slang has
 several features that aid the programmer in tracking down problems,
 including function call tracebacks and the tracing of function calls.
 However, the biggest debugging aid stems from the fact that the
 language is interpreted permitting one to easily add debugging
 statements to the code.
  
 To enable debugging information, add the lines
#v+
    _debug_info = 1;
    _traceback = 1;
#v-
 to the top of the source file of the code containing the bug and the
 reload the file.  Setting the \var{_debug_info} variable to
 \exmp{1} causes line number information to be compiled into the
 functions when the file is loaded.  The \var{_traceback} variable
 controls whether or not traceback information should be generated.
 If it is set to \exmp{1}, the values of local variables will be
 dumped when the traceback is generated.  Setting this variable
 to \exmp{-1} will cause only function names to be reported in the
 traceback.

 Here is an example of a traceback report:
#v+
    S-Lang Traceback: error
    S-Lang Traceback: verror
    S-Lang Traceback: (Error occurred on line 65)
    S-Lang Traceback: search_generic_search
      Local Variables:
        $0: Type: String_Type,  Value:  "Search forward:"
        $1: Type: Integer_Type, Value:  1
        $2: Type: Ref_Type,     Value:  _function_return_1
        $3: Type: String_Type,  Value:  "abcdefg"
        $4: Type: Integer_Type, Value:  1
    S-Lang Traceback: (Error occurred on line 72)
    S-Lang Traceback: search_forward
#v-
 There are several ways to read this report; perhaps the simplest is
 to read it from the bottom.  This report says that on line \exmp{72},
 the \var{search_forward} function called the
 \var{search_generic_search} function.  On line \var{65} it called the
 \verb{verror} function, which called \var{error}.  The
 \var{search_generic_search} function contains \var{5} local variables
 and are represented symbolically as \exmp{$0} through \exmp{$4}.


#%}}}

#i regexp.tm

\chapter{Future Directions} #%{{{

 Several new features or enhancements to the \slang language are
 planned for the next major release.  In no particular order, these
 include:
\begin{itemize}
  \item An interactive debugging facility.
  \item Function qualifiers.  These entities should already be
  familiar to VMS users or to those who are familiar with the IDL
  language.  Basically, a qualifier is an optional argument that is
  passed to a function, e.g., \exmp{plot(X,Y,/logx)}.  Here
  \exmp{/logx} is a qualifier that specifies that the plot function
  should use a log scale for \exmp{x}.
  \item File local variables and functions.  A file local variable or
  function is an object that is global to the file that defines it.
  \item Multi-threading.  Currently the language does not support
  multiple threads.
\end{itemize}


#%}}}

\appendix

#i copyright.tm

\end{\documentstyle}
