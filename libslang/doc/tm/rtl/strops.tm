\function{Sprintf}
\synopsis{Format objects into a string}
\usage{String_Type Sprintf (String_Type format, ..., Integer_Type n)}
\description
  \var{Sprintf} formats a string from \var{n} objects according to
  \var{format}.  Unlike \var{sprintf}, the \var{Sprintf} function
  requires the number of items to format.
  
  The format string is a C library \var{sprintf} style format
  descriptor.  Briefly, the format string may consist of ordinary
  characters (not including the \exmp{%} character), which are copied
  into the output string as-is, and a conversion specification
  introduced by the \exmp{%} character.  The \var{%} character must be
  followed by at least one other character to specify the conversion:
#v+
     s    value is a string
     f    value is a floating point number
     e    print float in exponential form, e.g., 2.345e08
     g    print float as e or g, depending upon its value
     c    value is an ascii character
     %    print the percent character
     d    print a signed decimal integer
     u    print an unsigned decimal integer
     o    print an integer as octal
     X    print an integer as hexadecimal
     S    convert value to a string and format as string
#v-
  Note that \var{%S} is a \slang extension which will cause the value
  to be formatted as string.  In fact, \exmp{sprintf("%S",x)} is
  equivalent to \exmp{sprintf("%s",string(x))}.
#v+
     s = Sprintf("%f is greater than %f but %s is better than %s\n",
                 PI, E, "Cake" "Pie", 4);
#v-
  The final argument to \var{Sprintf} is the number of items to format; in
  this case, there are 4 items.
\seealso{sprintf, string, sscanf}
\done

\function{create_delimited_string}
\synopsis{Concatenate strings using a delimiter}
\usage{String_Type create_delimited_string (delim, s_1, s_2, ..., s_n, n)}
#v+
    String_Type delim, s_1, ..., s_n
    Integer_Type n
#v-
\description
  \var{create_delimited_string} performs a concatenation operation on
  the \var{n} strings \var{s_1}, ...,\var{s_n}, using the string
  \var{delim} as a delimiter.  The resulting string is equivalent to
  one obtained via
#v+
      s_1 + delim + s_2 + delim + ... + s_n
#v-
\example
  One use for this function is to construct path names, e.g.,
#v+
    create_delimited_string ("/", "user", "local", "bin", 3);
#v-
  will produce \exmp{"usr/local/bin"}.
\notes
  The expression \exmp{strcat(a,b)} is equivalent to
  \exmp{create_delimited_string("", a, b, 2)}.
\seealso{strjoin, is_list_element, extract_element, strchop, strcat}
\done

\function{extract_element}
\synopsis{Extract the nth element of a string with delimiters}
\usage{String_Type extract_element (String_Type list, Integer_Type nth, Integer_Type delim);}
\description
  The \var{extract_element} function may be used to extract the
  \var{nth} element of the \var{delim} delimited list of strings
  \var{list}.  The function will return the \var{nth} element of the
  list, unless \var{nth} specifies more elements than the list
  contains, in which case \var{NULL} will be returned.
  Elements in the list are numbered from \var{0}.
\example
  The expression
#v+
     extract_element ("element 0, element 1, element 2", 1, ',')
#v-
  returns the string \exmp{" element 1"}, whereas
#v+
     extract_element ("element 0, element 1, element 2", 1, ' ')
#v-
  returns \exmp{"0,"}.

  The following function may be used to compute the number of elements
  in the list:
#v+
     define num_elements (list, delim)
     {
        variable nth = 0;
        while (NULL != extract_element (list, nth, delim))
          nth++;
        return nth;
     }
#v-

  Alternatively, the \var{strchop} function may be more useful.  In
  fact, \var{extract_element} may be expressed in terms of the
  function \var{strchop} as
#v+
    define extract_element (list, nth, delim)
    {
       list = strchop(list, delim, 0);
       if (nth >= length (list))
         return NULL;
       else
         return list[nth];
    }
#v-
   and the \var{num_elements} function used above may be recoded more
   simply as:
#v+
    define num_elements (list, delim)
    {
       return length (strchop (length, delim, 0));
    }
#v-
\seealso{is_list_element, is_substr, strtok, strchop, create_delimited_string}
\done

\function{is_list_element}
\synopsis{Test whether a delimited string contains a specific element}
\usage{Integer_Type is_list_element (String_Type list, String_Type elem, Integer_Type delim)}
\description
  The \var{is_list_element} function may be used to determine whether
  or not a delimited list of strings, \var{list}, contains the element
  \var{elem}.  If \var{elem} is not an element of \var{list}, the function
  will return zero, otherwise, it returns 1 plus the matching element
  number.
\example
  The expression
#v+
     is_list_element ("element 0, element 1, element 2", "0,", ' ');
#v-
  returns \exmp{2} since \exmp{"0,"} is element number one of the list
  (numbered from zero).
\seealso{extract_element, is_substr, create_delimited_string}
\done

\function{is_substr}
\synopsis{Test for a specified substring within a string.}
\usage{Integer_Type is_substr (String_Type a, String_Type b)}
\description
  This function may be used to determine if \var{a} contains the
  string \var{b}.  If it does not, the function returns 0; otherwise it
  returns the position of the first occurance of \var{b} in \var{a}.
\notes
  It is important to remember that the first character of a string
  corresponds to a position value of \exmp{1}.
\seealso{substr, string_match, strreplace}
\done

\function{make_printable_string}
\synopsis{Format a string suitable for parsing}
\usage{String_Type make_printable_string(String_Type str)}
\description
  This function formats a string in such a way that it may be used as
  an argument to the \var{eval} function.  The resulting string is
  identical to \var{str} except that it is enclosed in double quotes and the
  backslash, newline, and double quote characters are expanded.
\seealso{eval, str_quote_string}
\done

\function{sprintf}
\synopsis{Format objects into a string}
\usage{String sprintf (String format, ...);}
\description
  This function performs a similar task as the C function with the same
  name.  It differs from the \slang function \var{Sprintf} in that it
  does not require the number of items to format.
  See the documentation for \var{Sprintf} for more information.
\seealso{Sprintf, string, sscanf, vmessage}
\done

\function{sscanf}
\synopsis{Parse a formatted string}
\usage{Int_Type sscanf (s, fmt, r1, ... rN)}
#v+
    String_Type s, fmt;
    Ref_Type r1, ..., rN
#v-
\description
 The \var{sscanf} function parses the string \var{s} according to the
 format \var{fmt} and sets the variables whose references are given by
 \var{r1}, ..., \var{rN}.  The function returns the number of
 references assigned, or \var{-1} upon error.
 
 The format string \var{fmt} consists of ordinary characters and
 conversion specifiers.  A conversion specifier begins with the
 special character \var{%} and is described more fully below.  A white
 space character in the format string matches any amount of whitespace
 in the input string.  Parsing of the format string stops whenever a
 match fails.

 The \var{%} is used to denote a conversion specifier whose general
 form is given by \exmp{%[*][width][type]format} where the brackets
 indicate optional items.  If \var{*} is present, then the conversion
 will be performed by no assignment to a reference will be made.  The
 \var{width} specifier specifies the maximum field width to use for
 the conversion.  The \var{type} modifier is used to indicate size of
 the object, e.g., a short integer, as follows. 
 
 If \em{type} is given as the character \var{h}, then if the format
 conversion is for an integer (\var{dioux}), the object assigned will
 be a short integer.  If \em{type} is \var{l}, then the conversion
 will be to a long integer for integer conversions, or to a double
 precession floating point number for floating point conversions.

 The format specifier is a character that specifies the conversion:
#v+
       %     Matches a literal percent character.  No assigment is
             performed.
       d     Matches a signed decimal integer.
       D     Matches a long decimal integer (equiv to `ld')
       u     Matches an unsigned decimal integer
       U     Matches an unsigned long decimal integer (equiv to `lu')
       i     Matches either a hexidecimal integer, decimal integer, or 
             octal integer.
       I     Equivalent to `li'.
       x     Matches a hexidecimal integer.
       X     Matches a long hexidecimal integer (same as `lx').
       e,f,g Matches a decimal floating point number (Float_Type).
       E,F,G Matches a double precision floating point number, same as `lf'.
       s     Matches a string of non-whitespace characters (String_Type).
       c     Matches one character.  If width is given, width
             characters are matched.
       n     Assigns the number of characters scanned so far.
       [...] Matches zero or more characters from the set of characters
             enclosed by the square brackets.  If '^' is given as the
             first character, then the complement set is matched. 
#v-
\example
 Suppose that \var{s} is \exmp{"Coffee: (3,4,12.4)"}.  Then
#v+
    n = sscanf (s, "%[a-zA-Z]: (%d,%d,%lf)", &item, &x, &y, &z);
#v-
 will set \var{n} to \4, \var{item} to \exmp{"Coffee"}, \var{x} to \3,
 \var{y} to \4, and \var{z} to the double precision number
 \exmp{12.4}.  However,
#v+
    n = sscanf (s, "%s: (%d,%d,%lf)", &item, &x, &y, &z);
#v-
 will set \var{n} to \1, \var{item} to \exmp{"Coffee:"} and the
 remaining variables will not be assigned.
\seealso{sprintf, unpack, string, atof, int, integer, string_match}
\done

\function{str_delete_chars}
\synopsis{Delete characters from a string}
\usage{String_Type str_delete_chars (String_Type str, String_Type del_set}
\description
  This function may be used to delete the set of characters specified
  by \var{del_set} from the string \var{str}.  The result is returned.
\example
#v+
    str = str_delete_chars (str, "^A-Za-z");
#v-
  will remove all characters except \exmp{A-Z} and \exmp{a-z} from
  \var{str}.
\done

\function{str_quote_string}
\synopsis{Escape characters in a string.}
\usage{String_Type str_quote_string(String_Type str, String_Type qlis, Integer_Type quote)}
\description
  The \var{str_quote_string} returns a string identical to \var{str}
  except that all characters in the set specified by the string
  \var{qlis} are escaped with the \var{quote} character, including the
  quote character itself.   This function is useful for making a
  string that can be used in a regular expression.
\example
  Execution of the statements
#v+
   node = "Is it [the coat] really worth $100?";
   tag = str_quote_string (node, "\\^$[]*.+?", '\\');
#v-
  will result in \var{tag} having the value:
#v+
    Is it \[the coat\] really worth \$100\?
#v-
\seealso{str_uncomment_string, make_printable_string}
\done

\function{str_replace}
\synopsis{Replace a substring of a string}
\usage{Integer_Type str_replace (String_Type a, String_Type b, String_Type c)}
\description
  The \var{str_replace} function replaces the first occurance of \var{b} in
  \var{a} with \var{c} and returns an integer that indicates whether a
  replacement was made or not. If \var{b} does not occur in \var{a}, zero is
  returned.  However, if \var{b} occurs in \var{a}, a non-zero integer is
  returned as well as the new string resulting from the replacement.
\notes
  This function has been superceded by \var{strreplace}.
\seealso{strreplace}
\done

\function{str_uncomment_string}
\synopsis{Remove comments from a string}
\usage{String_Type str_uncomment_string(String_Type s, String_Type beg, String_Type end)}
\description
  This function may be used to remove comments from a string \var{s}.
  The parameters, \var{beg} and \var{end}, are strings of equal length
  whose corresponding characters specify the begin and end comment
  characters, respectively.  It returns the uncommented string.
\example
  The expression
#v+
     str_uncomment_string ("Hello (testing) 'example' World", "'(", "')")
#v-
  returns the string \exmp{"Hello   World"}.
\notes
  This routine does not handle multicharacter comment delimiters and it
  assumes that comments are not nested.
\seealso{str_quote_string}
\done

\function{strcat}
\synopsis{Concatenate strings}
\usage{String_Type strcat (String_Type a_1, ...,  String_Type a_N)}
\description
   The \var{strcat} function concatenates its N \var{String_Type}
   arguments \var{a_1}, ... \var{a_N} together and returns the result.
\example
#v+
    strcat ("Hello", " ", "World");
#v-
   produces the string \exmp{"Hello World"}.
\notes
   This function is equivalent to the binary operation \exmp{a_1+...+a_N}.
   However, \var{strcat} is much faster making it the preferred method
   to concatenate string.
\seealso{sprintf, create_delimited_string}
\done

\function{strchop}
\synopsis{Chop or split a string into substrings.}
\usage{String_Type[] strchop (String_Type str, Integer_Type delim, Integer_Type quote)}
\description
   The \var{strchop} function may be used to split-up a string
   \var{str} that consists of substrings delimited by the character
   specified by \var{delim}.  If the integer \var{quote} is non-zero,
   it will be taken as a quote character for the delimiter.  The
   function returns the substrings as an array.
\example
   The following function illustrates how to sort a comma separated
   list of strings:
#v+
     define sort_string_list (a)
     { 
        variable i, b, c;
        b = strchop (a, ',', 0);
        
        i = array_sort (b, &strcmp);
        b = b[i];   % rearrange
        
        % Convert array back into comma separated form
        return strjoin (b, ",");
     }
#v-
\notes
   The semantics of this \var{strchop} and \var{strchopr} have been
   changed since version 1.2.x of the interpreter.  Old versions of
   these functions returned the values on the stack, which meant that
   one could not chop up arbitrarily long strings that consist of
   many substrings.
   
   The function \var{strchopr} should be used if it is desired to have
   the string chopped-up in the reverse order.
\seealso{strchopr, extract_element, strjoin, strtok}
\done

\function{strchopr}
\synopsis{Chop or split a string into substrings.}
\usage{String_Type[] strchopr (String_Type str, String_Type delim, String_Type quote)}
\description
  This routine performs exactly the same function as \var{strchop} except
  that it returns the substrings in the reverse order.  See the
  documentation for \var{strchop} for more information.
\seealso{strchop, extract_element, strtok, strjoin}
\done

\function{strcmp}
\synopsis{Compare two strings}
\usage{Interpret strcmp (String_Type a, String_Type b)}
\description
   The \var{strcmp} function may be used to perform a case-sensitive
   string comparison, in the lexicongraphic sense, on strings \var{a} and
   \var{b}.  It returns 0 if the strings are identical, a negative integer
   if \var{a} is less than \var{b}, or a positive integer if \var{a} is greater
   than \var{b}.
\example
   The \var{strup} function may be used to perform a case-insensitive
   string comparison:
#v+
    define case_insensitive_strcmp (a, b)
    {
      return strcmp (strup(a), strup(b));
    }
#v-
\notes
   One may also use one of the binary comparison operators, e.g.,
   \exmp{a > b}.
\seealso{strup, strncmp}
\done

\function{strcompress}
\synopsis{Remove excess whitespace characters from a string}
\usage{String_Type strcompress (String_Type s, String_Type white)}
\description
  The \var{strcompress} function compresses the string \var{s} by
  replacing a sequence of one or more characters from the set
  \var{white} by the first character of \var{white}. In addition, it
  also removes all leading and trailing characters from \var{s} that
  are part of \var{white}.
\example
  The expression
#v+
    strcompress (",;apple,,cherry;,banana", ",;");
#v-
  returns the string \exmp{"apple,cherry,banana"}.
\seealso{strtrim, strtrans}
\done

\function{string_match}
\synopsis{Match a string against a regular expression}
\usage{Integer_Type string_match(String_Type str, String_Type pat, Integer_Type pos)}
\description
  The \var{string_match} function returns zero if \var{str} does not
  match regular expression specified by \var{pat}.  This function
  performs the match starting at position \var{pos} (numbered from 1) in
  \var{str}.  This function returns the position of the start of the
  match.  To find the exact substring actually matched, use
  \var{string_match_nth}.
\seealso{string_match_nth, strcmp, strncmp}
\done

\function{string_match_nth}
\synopsis{Get the result of the last call to string_match}
\usage{(Integer_Type, Integer_Type) = string_match_nth(Integer_Type nth)}
\description
  The \var{string_match_nth} function returns two integers describing
  the result of the last call to \var{string_match}.  It returns both
  the offset into the string and the length of characters matches by
  the \var{nth} submatch.

  By convention, \var{nth} equal to zero means the entire match.
  Otherwise, \var{nth} must be an integer with a value 1 through 9,
  and refers to the set of characters matched by the \var{nth} regular
  expression enclosed by the pairs \exmp{\\(, \\)}.
\example
  Consider:
#v+
     variable matched, pos, len;
     matched = string_match("hello world", "\\([a-z]+\\) \\([a-z]+\\)", 1);
     if (matched) (pos, len) = string_match_nth(2);
#v-
  This will set \var{matched} to 1 since a match will be found at the
  first position, \var{pos} to 6 since \var{w} is offset 6 characters
  from the beginning of the string, and \var{len} to 5 since
  \exmp{"world"} is 5 characters long.
\notes
  The position offset is \em{not} affected by the value of the offset
  parameter to the \var{string_match} function. For example, if the
  value of the last parameter to the \var{string_match} function had
  been 3, \var{pos} would still have been set to 6.

  Note also that \var{string_match_nth} returns the \em{offset} from
  the beginning of the string and not the position of the match.
\seealso{string_match}
\done

\function{strjoin}
\synopsis{Concatenate elements of a string array}
\usage{String_Type strjoin (Array_Type a, String_Type delim)}
\description
   The \var{strjoin} function operates on an array of strings by joining
   successive elements together separated with a delimiter \var{delim}.
   If \var{delim} is the empty string \exmp{""}, then the result will
   simply be the concatenation of the elements.
\example
   Suppose that
#v+
      days = ["Sun","Mon","Tue","Wed","Thu","Fri","Sat","Sun"];
#v-
   Then \exmp{strjoin (days,"+")} will produce
   \exmp{"Sun+Mon+Tue+Wed+Thu+Fri+Sat+Sun"}.  Similarly,
   \exmp{strjoin (["","",""], "X")} will produce \exmp{"XX"}.
\seealso{create_delimited_string, strchop, strcat}
\done

\function{strlen}
\synopsis{Compute the length of a string}
\usage{Integer_Type strlen (String_Type a)}
\description
   The \var{strlen} function may be used to compute the length of a string.
\example
   After execution of
#v+
   variable len = strlen ("hello");
#v-
   \var{len} will have a value of \exmp{5}.
\seealso{bstrlen, length, substr}
\done

\function{strlow}
\synopsis{Convert a string to lowercase}
\usage{String_Type strlow (String_Type s)}
\description
  The \var{strlow} function takes a string \var{s} and returns another
  string identical to \var{s} except that all upper case characters
  that comprise \var{s} will be converted to lower case.
\example
  The function
#v+
    define Strcmp (a, b)
    {
      return strcmp (strlow (a), strlow (b));
    }
#v-
  performs a case-insensitive comparison operation of two strings by
  converting them to lower case first.
\seealso{strup, tolower, strcmp, strtrim, define_case}
\done

\function{strncmp}
\synopsis{Compare the first few characters of two strings}
\usage{Integer_Type strncmp (String_Type a, String_Type b, Integer_Type n)}
\description
  This function behaves like \var{strcmp} except that it compares only the
  first \var{n} characters in the strings \var{a} and \var{b}.  See
  the documentation for \var{strcmp} for information about the return
  value.
\example
  The expression
#v+
     strcmp ("apple", "appliance", 3);
#v-
  will return zero since the first three characters match.
\seealso{strcmp, strlen}
\done

\function{strreplace}
\synopsis{Replace one or more substrings}
\usage{(new, n) = strreplace (a, b, c, max_n)}
#v+
   String_Type a, b, c, rep;
   Int_Type n, max_n;
#v-
\description
  The \var{strreplace} function may be used to replace one or more
  occurances of \var{b} in \var{a} with \var{c}.  If the integer
  \var{max_n} is positive, then the first \var{max_n} occurances of
  \var{b} in \var{a} will be replaced.  Otherwise, if \var{max_n} is
  negative, then the last \exmp{abs(max_n)} occurances will be replaced.
  
  The function returns the resulting string and an integer indicating
  how many replacements were made.
\example
  The following function illustrates how \var{strreplace} may be used
  to remove all occurances of a specified substring
#v+
  define delete_substrings (a, b)
  {
     (a, ) = strreplace (a, b, "", strlen (a));
     return a;
  }
#v-
\seealso{is_substr, strsub, strtrim, strtrans, str_delete_chars}
\done

\function{strsub}
\synopsis{Replace a character with another in a string.}
\usage{String_Type strsub (String_Type s, Integer_Type pos, Integer_Type ch)}
\description
  The \var{strsub} character may be used to substitute the character
  \var{ch} for the character at position \var{pos} of the string
  \var{s}.  The resulting string is returned.
\example
#v+
    define replace_spaces_with_comma (s)
    {
      variable n;
      while (n = is_substr (s, " "), n) s = strsub (s, n, ',');
      return s;
    }
#v-
  For uses such as this, the \var{strtrans} function is a better choice.
\notes
  The first character in the string \var{s} is specified by \var{pos}
  equal to 1.
\seealso{is_substr, strreplace, strlen}
\done

\function{strtok}
\synopsis{Extract tokens from a string}
\usage{String_Type[] strtok (String_Type str [,String_Type white])}
\description
  \var{strtok} breaks the string \var{str} into a series of tokens and
  returns them as an array of strings.  If the second parameter
  \var{white} is present, then it specifies the set of characters that
  are to be regarded as whitespace when extracting the tokens, and may
  consist of the whitespace characters or a range of such characters.
  If the first character of \var{white} is \exmp{'^'}, then the
  whitespace characters consist of all characters except those in
  \var{white}. For example, if \var{white} is \exmp{" \\t\\n,;."},
  then those characters specifiy the whitespace characters.  However,
  if \var{white} is given by \exmp{"^a-zA-Z0-9_"}, then any character
  is a whitespace character except those in the ranges \exmp{a-z},
  \exmp{A-Z}, \exmp{0-9}, and the underscore character.
  
  If the second parameter is not present, then it defaults to 
  \exmp{" \\t\\r\\n\\f"}.
\example
  The following example may be used to count the words in a text file:
#v+
    define count_words (file)
    {
       variable fp, line, count;
       
       fp = fopen (file, "r");
       if (fp == NULL) return -1;
       
       count = 0;
       while (-1 != fgets (&line, fp))
         {
           line = strtok (line, "^a-zA-Z");
           count += length (line);
         }
       () = fclose (fp);
       return count;
    }
#v-
\seealso{strchop, strcompress, extract_element, strjoin}
\done

\function{strtrans}
\synopsis{Replace characters in a string}
\usage{String_Type strtrans (str, old_set, new_set)}
#v+
   String_Type str, old_set, new_set;
#v-
\description
  The \var{strtrans} function may be used to replace all the characters
  from the set \var{old_set} with the corresponding characters from
  \var{new_set} in the string \var{str}.  If \var{new_set} is empty,
  then the characters in \var{old_set} will be removed from \var{str}.
  This function returns the result.
\example
#v+
    str = strtrans (str, "A-Z", "a-z");   % lower-case str
    str = strtrans (str, "^0-9", " ");    % Replace anything but 0-9 by space
#v-
\seealso{strreplace, strtrim, strup, strlow}
\done

\function{strtrim}
\synopsis{Remove whitespace from the ends of a string}
\usage{String_Type strtrim (String_Type s [,String_Type w])}
\description
  The \var{strtrim} function removes all leading and trailing whitespace
  characters from the string \var{s} and returns the result.  The
  optional second parameter specifies the set of whitespace
  characters.  If the argument is not present, then the set defaults
  to \exmp{" \\t\\r\\n"}.
\seealso{strtrim_beg, strtrim_end, strcompress}
\done

\function{strtrim_beg}
\synopsis{Remove leading whitespace from a string}
\usage{String_Type strtrim_beg (String_Type s [,String_Type w])}
\description
  The \var{strtrim_beg} function removes all leading whitespace
  characters from the string \var{s} and returns the result.  The
  optional second parameter specifies the set of whitespace
  characters.  If the argument is not present, then the set defaults
  to \exmp{" \\t\\r\\n"}.
\seealso{strtrim, strtrim_end, strcompress}
\done

\function{strtrim_end}
\synopsis{Remove trailing whitespace from a string}
\usage{String_Type strtrim_end (String_Type s [,String_Type w])}
\description
  The \var{strtrim_end} function removes all trailing whitespace
  characters from the string \var{s} and returns the result.  The
  optional second parameter specifies the set of whitespace
  characters.  If the argument is not present, then the set defaults
  to \exmp{" \\t\\r\\n"}.
\seealso{strtrim, strtrim_beg, strcompress}
\done

\function{strup}
\synopsis{Convert a string to uppercase}
\usage{String_Type strup (String_Type s)}
\description
  The \var{strup} function takes a string \var{s} and returns another
  string identical to \var{s} except that all lower case characters
  that comprise \var{s} will be converted to upper case.
\example
  The function
#v+
    define Strcmp (a, b)
    {
      return strcmp (strup (a), strup (b));
    }
#v-
  performs a case-insensitive comparison operation of two strings by
  converting them to upper case first.
\seealso{strlow, toupper, strcmp, strtrim, define_case, strtrans}
\done

\function{substr}
\synopsis{Extract a substring from a string}
\usage{String_Type substr (String_Type s, Integer_Type n, Integer_Type len)}
\description
  The \var{substr} function returns a substring with length \var{len}
  of the string \var{s} beginning at position \var{n}.  If \var{len} is
  \exmp{-1}, the entire length of the string \var{s} will be used for
  \var{len}.  The first character of \var{s} is given by \var{n} equal
  to 1.
\example
#v+
     substr ("To be or not to be", 7, 5);
#v-
  returns \exmp{"or no"}
\notes
  In many cases it is more convenient to use array indexing rather
  than the \var{substr} function.  In fact, \exmp{substr(s,i+1,strlen(s))} is
  equivalent to \exmp{s[[i:]]}.
\seealso{is_substr, strlen}
\done

