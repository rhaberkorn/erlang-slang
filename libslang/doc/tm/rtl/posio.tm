\function{close}
\synopsis{Close an open file descriptor}
\usage{Int_Type close (FD_Type fd)}
\description
  The \var{close} function is used to open file descriptor of type
  \var{FD_Type}.  Upon success \0 is returned, otherwise the function
  returns \-1 and sets \var{errno} accordingly.
\seealso{open, fclose, read, write}
\done

\function{dup_fd}
\synopsis{Duplicate a file descriptor}
\usage{FD_Type dup_fd (FD_Type fd)}
\description
  The \var{dup_fd} function duplicates and file descriptor and returns
  its duplicate.  If the function fails, \NULL will be returned and
  \var{errno} set accordingly.
\notes
  This function is essentually a wrapper around the POSIX \var{dup}
  function.
\seealso{open, close}
\done

\function{fileno}
\synopsis{Convert a stdio File_Type object to a FD_Type descriptor}
\usage{FD_Type fileno (File_Type fp)}
\description
  The \var{fileno} function returns the \var{FD_Type} descriptor
  associated with the \var{File_Type} file pointer.  Upon failure,
  \NULL is returned.
\seealso{fopen, open, fclose, close, dup_fd}
\done

\function{isatty}
\synopsis{Determine if an open file descriptor refers to a terminal}
\usage{Int_Type isatty (FD_Type or File_Type fd)}
\description
  This function returns \1 if the file descriptor \var{fd} refers to a
  terminal; otherwise it returns \0.  The object \var{fd} may either
  be a \var{File_Type} stdio descriptor or an \var{FD_Type} object.
\seealso{fopen, fclose, fileno}
\done

\function{lseek}
\synopsis{Reposition a file descriptor's file pointer}
\usage{Long_Type lseek (FD_Type fd, Long_Type ofs, int mode)}
   The \var{lseek} function repositions the file pointer associated
   with the open file descriptor \var{fp} to offset \var{ofs}
   according to the mode parameter.  Specifically, \var{mode} must be
   one of the values:
#v+
     SEEK_SET   Set the offset to ofs
     SEEK_CUR   Add ofs to the current offset
     SEEK_END   Add ofs to the current file size
#v-
   Upon error, \var{lseek} returns \-1 and sets \var{errno}.  If
   successful, it returns the new filepointer offset.
\notes
   Not all file descriptors are capable of supporting the seek
   operation, e.g., a descriptor associated with a pipe.
   
   By using \var{SEEK_END} with a positive value of the \var{ofs}
   parameter, it is possible to position the file pointer beyond the
   current size of the file.
\seealso{fseek, ftell, open, close}
\done

\function{open}
\synopsis{Open a file}
\usage{FD_Type open (String_Type filename, Int_Type flags [,Int_Type mode])}
\description
  The \var{open} function attempts to open a file specified by the
  \var{filename} parameter according to the \var{flags} parameter,
  which must be one of the following values:
#v+
     O_RDONLY   (read-only)
     O_WRONLY   (write-only)
     O_RDWR     (read/write)
#v-
  In addition, \var{flags} may also be bitwise-or'd with any of the
  following:
#v+
     O_BINARY   (open the file in binary mode)
     O_TEXT     (open the file in text mode)
     O_CREAT    (create file if it does not exist)
     O_EXCL     (fail if the file already exists)
     O_NOCTTY   (do not make the device the controlling terminal)
     O_TRUNC    (truncate the file if it exists)
     O_APPEND   (open the file in append mode)
     O_NONBLOCK (open the file in non-blocking mode)
#v-
   Some of these flags only make sense when combined with other flags.
   For example, if O_EXCL is used, then O_CREAT must also be
   specified, otherwise unpredictable behavior may result.

   If \var{O_CREAT} is used for the \var{flags} parameter then the
   \var{mode} parameter must be present. \var{mode} specifies the
   permissions to use if a new file is created. The actual file
   permissions will be affected by the process's \var{umask} via
   \exmp{mode&~umask}.  The \var{mode} parameter's value is
   constructed via bitwise-or of the following values:
#v+
     S_IRWXU    (Owner has read/write/execute permission)
     S_IRUSR    (Owner has read permission)
     S_IWUSR    (Owner has write permission)
     S_IXUSR    (Owner has execute permission)
     S_IRWXG    (Group has read/write/execute permission)
     S_IRGRP    (Group has read permission)
     S_IWGRP    (Group has write permission)
     S_IXGRP    (Group has execute permission)
     S_IRWXO    (Others have read/write/execute permission)
     S_IROTH    (Others have read permission)
     S_IWOTH    (Others have write permission)
     S_IXOTH    (Others have execute permission)
#v-
   Upon success \var{open} returns a file descriptor object
   (\var{FD_Type}), otherwise \var{NULL} is returned and \var{errno}
   is set.
\notes
   If you are not familiar with the \var{open} system call, then it
   is recommended that you use \var{fopen} instead.
\seealso{fopen, close, read, write, stat_file}
\done

\function{read}
\synopsis{Read from an open file descriptor}
\usage{UInt_Type read (FD_Type fd, Ref_Type buf, UInt_Type num)}
\description
  The \var{read} function attempts to read at most \var{num} bytes
  into the variable indicated by \var{buf} from the open file
  descriptor \var{fd}.  It returns the number of bytes read, or \-1
  and sets \var{errno} upon failure.  The number of bytes read may be
  less than \var{num}, and will be zero if an attempt is made to read
  past the end of the file.
\notes
  \var{read} is a low-level function and may return \-1 for a variety
  of reasons.  For example, if non-blocking I/O has been specified for
  the open file descriptor and no data is available for reading then
  the function will return \-1 and set \var{errno} to \var{EAGAIN}.
\seealso{fread, open, close, write}
\done  

\function{write}
\synopsis{Write to an open file descriptor}
\usage{UInt_Type write (FD_Type fd, BString_Type buf)}
\description
   The \var{write} function attempts to write the bytes specified by
   the \var{buf} parameter to the open file descriptor \var{fd}.  It
   returns the number of bytes successfully written, or \-1 and sets
   \var{errno} upon failure.  The number of bytes written may be less
   than \exmp{length(buf)}.
\seealso{read, fwrite, open, close}
\done

