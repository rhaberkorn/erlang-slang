\function{array_to_bstring}
\synopsis{Convert an array to a binary string}
\usage{BString_Type array_to_bstring (Array_Type a)}
\description
   The \var{array_to_bstring} function returns the elements of an
   array \var{a} as a binary string.
\seealso{bstring_to_array, init_char_array}
\done

\function{bstring_to_array}
\synopsis{Convert a binary string to an array of characters}
\usage{UChar_Type[] bstring_to_array (BString_Type b)}
\description
   The \var{bstring_to_array} function returns an array of unsigned
   characters whose elements correspond to the characters in the
   binary string.
\seealso{array_to_bstring, init_char_array}
\done

\function{bstrlen}
\synopsis{Get the length of a binary string}
\usage{UInt_Type bstrlen (BString_Type s)}
\description
  The \var{bstrlen} function may be used to obtain the length of a
  binary string.  A binary string differs from an ordinary string (a C
  string) in that a binary string may include null chracters.
\example
#v+
    variable s = "hello\0";
    len = bstrlen (s);      % ==> len = 6
    len = strlen (s);       % ==> len = 5
#v-
\seealso{strlen, length}
\done

\function{pack}
\synopsis{Pack objects into a binary string}
\usage{BString_Type pack (String_Type fmt, ...)}
\description
  The \var{pack} function combines zero or more the objects (represented
  by the ellipses above) into a binary string acording to the format
  string \var{fmt}.
  
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
  A decimal length specifier may follow the data-type specifier.  With
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
  The default is to use native byte order.

  When unpacking via the \var{unpack} function, if the length
  specifier is greater than one, then an array of that length will be
  returned.  In addition, trailing whitespace and null character are
  stripped when unpacking an object given by the \var{S} specifier.
\example
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
\seealso{unpack, sizeof_pack, pad_pack_format, sprintf}
\done

\function{pad_pack_format}
\synopsis{Add padding to a pack format}
\usage{BString_Type pad_pack_format (String_Type fmt)}
\description
  The \var{pad_pack_format} function may be used to add the
  appropriate padding to the format \var{fmt} such that the data types
  specified by the format will be properly aligned for the system.
  This is especially important when reading or writing files that
  assume the native alignment.
  
  See the S-Lang User's Guide for more information about the use of
  this function.
\seealso{pack, unpack, sizeof_pack}
\done

\function{sizeof_pack}
\synopsis{Compute the size implied by a pack format string}
\usage{UInt_Type sizeof_pack (String_Type fmt)}
\description
  The \var{sizeof_pack} function returns the size of the binary string
  represented by the format string \var{fmt}.  This information may be
  needed when reading a structure from a file.
\notes
\seealso{pack, unpack, pad_pack_format}
\done

\function{unpack}
\synopsis{Unpack Objects from a Binary String}
\usage{(...) = unpack (String_Type fmt, BString_Type s)}
\description
  The \var{unpack} function unpacks objects from a binary string
  \var{s} according to the format \var{fmt} and returns the objects to
  the stack in the order in which they were unpacked.  See the
  documentation of the \var{pack} function for details about the
  format string.
\example
#v+
    (x,y) = unpack ("cc", "AB");         % ==> x = 'A', y = 'B'
    x = unpack ("c2", "AB");             % ==> x = ['A', 'B']
    x = unpack ("x<H", "\0\xAB\xCD");    % ==> x = 0xCDABuh
    x = unpack ("xxs4", "a b c\0d e f");  % ==> x = "b c\0"
    x = unpack ("xxS4", "a b c\0d e f");  % ==> x = "b c"
#v-
\seealso{pack, sizeof_pack, pad_pack_format}
\done

