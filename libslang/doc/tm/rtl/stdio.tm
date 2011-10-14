\function{clearerr}
\synopsis{Clear the error of a file stream}
\usage{clearerr (File_Type fp}
\description
  The \var{clearerr} function clears the error and end-of-file flags
  associated with the open file stream \var{fp}.
\seealso{ferror, feof, fopen}
\done

\function{fclose}
\synopsis{Close a file}
\usage{Integer_Type fclose (File_Type fp)}
\description
  The \var{fclose} function may be used to close an open file pointer
  \var{fp}.  Upon success it returns zero, and upon failure it sets
  \var{errno} and returns \exmp{-1}.  Failure usually indicates a that
  the file system is full or that \var{fp} does not refer to an open file.
\notes
  Many C programmers call \var{fclose} without checking the return
  value.  The \slang language requires the programmer to explicitly
  handle any value returned by a \slang function.  The simplest way to
  handle the return value from \var{fclose} is to use it as:
#v+
     () = fclose (fp);
#v-
\seealso{fopen, fgets, fflush, pclose, errno}
\done

\function{fdopen}
\synopsis{Convert a FD_Type file descriptor to a stdio File_Type object}
\usage{File_Type fdopen (FD_Type, String_Type mode)}
\description
   The \var{fdopen} function creates and returns a stdio
   \var{File_Type} object from the open \var{FD_Type}
   descriptor \var{fd}.  The \var{mode} parameter corresponds to the
   \var{mode} parameter of the \var{fopen} function and must be
   consistent with the mode of the descriptor \var{fd}.  The function
   returns \NULL upon failure and sets \var{errno}.
\notes
   The \var{fclose} function does not close the \var{File_Type} object
   returned from this function.  The underlying file object must be
   closed by the \var{close} function.
\seealso{fileno, fopen, open, close, fclose}
\done

\function{feof}
\synopsis{Get the end-of-file status}
\usage{Integer_Type feof (File_Type fp)}
\description
  This function may be used to determine the state of the end-of-file
  indicator of the open file descriptor \var{fp}.  It returns \var{0}
  if the indicator is not set, or non-zero if it is.  The end-of-file
  indicator may be cleared by the \var{clearerr} function.
\seealso{ferror, clearerr, fopen}
\done

\function{ferror}
\synopsis{Determine the error status of an open file descriptor}
\usage{Integer_Type ferror (File_Type fp)}
\description
  This function may be used to determine the state of the error
  indicator of the open file descriptor \var{fp}.  It returns \var{0}
  if the indicator is not set, or non-zero if it is.  The error
  indicator may be cleared by the \var{clearerr} function.
\seealso{feof, clearerr, fopen}
\done

\function{fflush}
\synopsis{Flush an output stream}
\usage{Integer_Type fflush (File_Type fp)}
\description
  The \var{fflush} function may be used to update the \em{output}
  stream specified by \var{fp}.  It returns \var{0} upon success, or
  \var{-1} upon failure and sets \var{errno} accordingly.  In
  particular, this function will fail if \var{fp} does not represent
  an output stream, or if \var{fp} is associated with a disk file and
  there is insufficient disk space.
\example
  This example illustrates how to use the \var{fflush} function
  without regard to the return value:
#v+
    () = fputs ("Enter value> ", stdout);
    () = fflush (stdout);
#v-
\notes
  Many C programmers disregard the return value from the \var{fflush}
  function.  The above example illustrates how to properly do this in
  the \slang langauge.
\seealso{fopen, fclose}
\done

\function{fgets}
\synopsis{Read a line from a file.}
\usage{Integer_Type fgets (SLang_Ref_Type ref, File_Type fp)}
\description
  \var{fgets} reads a line from the open file specified by \var{fp}
  and places the characters in the variable whose reference is
  specified by \var{ref}.
  It returns \exmp{-1} if \var{fp} is not associated with an open file
  or an attempt was made to read at the end the file; otherwise, it
  returns the number of characters read.
\example
  The following example returns the lines of a file via a linked list:
#v+
    define read_file (file)
    {
       variable buf, fp, root, tail;
       variable list_type = struct { text, next };

       root = NULL;

       fp = fopen(file, "r");
       if (fp == NULL)
         error("fopen %s failed." file);
       while (-1 != fgets (&buf, fp))
         {
            if (root == NULL)
              {
                 root = @list_type;
                 tail = root;
              }
            else
              {
                 tail.next = @list_type;
                 tail = tail.next;
              }
            tail.text = buf;
            tail.next = NULL;
         }
       () = fclose (fp);
       return root;
    }
#v-
\seealso{fopen, fclose, fputs, fread, error}
\done

\function{fgetslines}
\synopsis{Read all the lines from an open file}
\usage{String_Type[] fgetslines (File_Type fp)}
\description
  The \var{fgetslines} function returns all the remaining lines as an
  array of strings in the file specified by the open file pointer
  \var{fp}.  If the file is empty, an empty string array will be
  returned.  The function returns \var{NULL} upon error.
\example
  The following function returns the number of lines in a file:
#v+
    define count_lines_in_file (file)
    {
       variable fp, lines;

       fp = fopen (file, "r");
       if (fp == NULL)
         return -1;
       
       lines = fgetslines (fp);
       if (lines == NULL)
         return -1;
       
       return length (lines);
    }
#v-
  Note that the file was implicitly closed by the function.
\notes
  This function should not be used if the file contains many lines
  since that would require that all the lines be read into memory.
\seealso{fgets, fread, fopen}
\done

\function{fopen}
\synopsis{Open a file}
\usage{File_Type fopen (String_Type f, String_Type m)}
\description
  The \var{fopen} function opens a file \var{f} according to the mode
  string \var{m}.  Allowed values for \var{m} are:
#v+
     "r"    Read only
     "w"    Write only
     "a"    Append
     "r+"   Reading and writing at the beginning of the file.
     "w+"   Reading and writing.  The file is created if it does not
              exist; otherwise, it is truncated.
     "a+"   Reading and writing at the end of the file.  The file is created
              if it does not already exist.
#v-
  In addition, the mode string can also include the letter \var{'b'}
  as the last character to indicate that the file is to be opened in
  binary mode.

  Upon success, \var{fopen} a \var{File_Type} object which is meant to
  be used in other operations that require an open file.  Upon
  failure, the function returns \var{NULL}.
\example
  The following function opens a file in append mode and writes a
  string to it:
#v+
    define append_string_to_file (file, str)
    {
       variable fp = fopen (file, "a");
       if (fp == NULL) verror ("%s could not be opened", file);
       () = fputs (string, fp);
       () = fclose (fp);
    }
#v-
  Note that the return values from \var{fputs} and \var{fclose} are
  ignored.
\notes
  There is no need to explicitly close a file opened with \var{fopen}.
  If the returned \var{File_Type} object goes out of scope, \slang
  will automatically close the file.  However, explicitly closing a
  file after use is recommended.
\seealso{fclose, fgets, fputs, popen}
\done

\function{fprintf}
\synopsis{Create and write a formatted string to a file}
\usage{Int_Type fprintf (File_Type fp, String_Type fmt, ...)}
\description
  \var{fprintf} formats the objects specified by the variable argument
  list according to the format \var{fmt} and write the result to the
  open file pointer \var{fp}.  
  
  The format string obeys the same syntax and semantics as the
  \var{sprintf} format string.  See the description of the
  \var{sprintf} function for more information.
  
  \var{fprintf} returns the number of characters written to the file,
  or \-1 upon error.
\seealso{fputs, printf, fwrite, message}
\done

\function{fputs}
\synopsis{Write a string to an open stream}
\usage{Integer_Type fputs (String_Type s, File_Type fp);}
\description
  The \var{fputs} function writes the string \var{s} to the open file
  pointer \var{fp}. It returns -1 upon failure and sets \var{errno},
  otherwise it returns the length of the string.
\example
  The following function opens a file in append mode and uses the
  \var{fputs} function to write to it.
#v+
    define append_string_to_file (str, file)
    {
       variable fp;
       fp = fopen (file, "a");
       if (fp == NULL) verror ("Unable to open %s", file);
       if ((-1 == fputs (s, fp))
           or (-1 == fclose (fp)))
         verror ("Error writing to %s", file);
    }
#v-
\notes
  One must not disregard the return value from the \var{fputs}
  function, as many C programmers do.  Doing so may lead to a stack
  overflow error.
  
  To write an object that contains embedded null characters, use the
  \var{fwrite} function.
\seealso{fclose, fopen, fgets, fwrite}
\done

\function{fread}
\synopsis{Read binary data from a file}
\usage{UInt_Type fread (Ref_Type b, DataType_Type t, UInt_Type n, File_Type fp)}
\description
  The \var{fread} function may be used to read \var{n} objects of type
  \var{t} from an open file pointer \var{fp}.  Upon success, it
  returns the number of objects read from the file and places the
  objects in the variable specified by \var{b}.  Upon error or end of
  file, it returns \var{-1}.  If more than one object is read from the
  file, those objects will be placed in an array of the appropriate
  size.  The exception to this is when reading \var{Char_Type} or
  \var{UChar_Type} objects from a file, in which case the data will be
  returned as an \var{n} character BString_Type binary string, but
  only if \var{n}>1.
\example
  The following example illustrates how to read 50 bytes from a file:
#v+
     define read_50_bytes_from_file (file)
     {
        variable fp, n, buf;
        
        fp = fopen (file, "rb");
        if (fp == NULL) error ("Open failed");
        n = fread (&buf, Char_Type, 50, fp);
        if (n == -1)
        error ("fread failed");
        () = fclose (fp);
        return buf;
     }
#v-
\notes
  Use the \var{pack} and \var{unpack} functions to read data with a
  specific byte-ordering.
\seealso{fwrite, fgets, fopen, pack, unpack}
\done

\function{fseek}
\synopsis{Reposition a stream}
\usage{Integer_Type fseek (File_Type fp, Integer_Type ofs, Integer_Type whence}
\description
  The \var{fseek} function may be used to reposition the file position
  pointer associated with the open file stream \var{fp}. Specifically,
  it moves the pointer \var{ofs} bytes relative to the position
  indicated by \var{whence}.  If whence is set to one of the symbolic
  constants \exmp{SEEK_SET}, \exmp{SEEK_CUR}, or \exmp{SEEK_END}, the
  offset is relative to the start of the file, the current position
  indicator, or end-of-file, respectively.
  
  The function return zero upon success, or \-1 upon failure and sets
  \var{errno} accordingly.
\example
    define rewind (fp)
    {
       if (0 == fseek (fp, 0, SEEK_SET)) return;
       vmessage ("rewind failed, reason: %s", errno_string (errno));
    }
\notes
  The current implementation uses an integer to specify the offset.
  One some systems, a long integer may be required making this
  function fail for very large files, i.e., files that are longer than
  the maximum value of an integer.
\seealso{ftell, fopen}
\done

\function{ftell}
\synopsis{Obtain the current position in an open stream}
\usage{Integer_Type ftell (File_Type fp)}
\description
  The ftell function may be used to obtain the current position in the
  stream associated with the open file pointer \var{fp}.  It returns
  the position of the pointer measured in bytes from the beginning of
  the file.  Upon error, it returns \exmp{-1} and sets \var{errno}.
\seealso{fseek, fopen}
\done

\function{fwrite}
\synopsis{Write binary data to a file}
\usage{UInt_Type fwrite (b, File_Type fp)}
\description
  The \var{fwrite} may be used to write the object represented by
  \var{b} to an open file.  If \var{b} is a string or an array, the
  function will attempt to write all elements of the object to the
  file.  It returns the number of objects successfully written,
  otherwise it returns \-1 upon error and sets \var{errno}
  accordingly.
\example
  The following example illustrates how to write an integer array to a
  file.  In this example, \var{fp} is an open file descriptor:
#v+
     variable a = [1:50];     % 50 element integer array
     if (50 != fwrite (a, fp))
       error ("fwrite failed");
#v-
  Here is how to write the array one element at a time:
#v+
     variable a = [1:50];
     foreach (a)
       {
          variable ai = ();
          if (1 != fwrite(ai, fp))
          error ("fwrite failed");
       }
#v-
\notes
  Not all data types may support the \var{fwrite} operation.  However,
  it is supported by all vector, scalar, and string objects.
\seealso{fread, fputs, fopen, pack, unpack}
\done

\function{pclose}
\synopsis{Close an object opened with popen}
\usage{Integer_Type pclose (File_Type fp)}
\description
  The \var{pclose} function waits for the process associated with
  \var{fp} to exit and the returns the exit status of the command.
\seealso{pclose, fclose}
\done

\function{popen}
\synopsis{Open a process}
\usage{File_Type popen (String_Type cmd, String_Type mode)}
\description
  The \var{popen} function executes a process specified by \var{cmd}
  and opens a unidirectional pipe to the newly created process.  The
  \var{mode} indicates whether or not the pipe is open for reading
  or writing.  Specifically, if \var{mode} is \exmp{"r"}, then the
  pipe is opened for reading, or if \var{mode} is \exmp{"w"}, then the
  pipe will be open for writing.

  Upon success, a \var{File_Type} pointer will be returned, otherwise
  the function failed and \var{NULL} will be returned.
\notes
  This function is not available on all systems.
\seealso{pclose, fopen}
\done

\function{printf}
\synopsis{Create and write a formatted string to stdout}
\usage{Int_Type printf (String_Type fmt, ...)}
\description
  \var{fprintf} formats the objects specified by the variable argument
  list according to the format \var{fmt} and write the result to
  \var{stdout}.  This function is equivalent to \var{fprintf} used
  with the \var{stdout} file pointer.  See \var{fprintf} for more
  information.
  
  \var{printf} returns the number of characters written to the file,
  or \-1 upon error.
\notes
  Many C programmers do not check the return status of the
  \var{printf} C library function.  Make sure that if you do not care
  about whether or not the function succeeds, then code it as in the
  following example:
#v+
     () = printf ("%s laid %d eggs\n", chicken_name, num_egg);
#v-
\seealso{fputs, printf, fwrite, message}
\done

