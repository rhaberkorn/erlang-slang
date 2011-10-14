\variable{errno}
\synopsis{Error code set by system functions.}
\usage{Integer_Type errno}
\description
  A system function can fail for a variety of reasons.  For example, a
  file operation may fail because lack of disk space, or the process
  does not have permission to perform the operation.  Such functions
  will return \var{-1} and set the variable \var{errno} to an error
  code describing the reason for failure.

  Particular values of \var{errno} may be specified by the following
  symbolic constants (read-only variables) and the corresponding
  \var{errno_string} value:
#v+
     EPERM            "Not owner"
     ENOENT           "No such file or directory"
     ESRCH            "No such process"
     ENXIO            "No such device or address"
     ENOEXEC          "Exec format error"
     EBADF            "Bad file number"
     ECHILD           "No children"
     ENOMEM           "Not enough core"
     EACCES           "Permission denied"
     EFAULT           "Bad address"
     ENOTBLK          "Block device required"
     EBUSY            "Mount device busy"
     EEXIST           "File exists"
     EXDEV            "Cross-device link"
     ENODEV           "No such device"
     ENOTDIR          "Not a directory"
     EISDIR           "Is a directory"
     EINVAL           "Invalid argument"
     ENFILE           "File table overflow"
     EMFILE           "Too many open files"
     ENOTTY           "Not a typewriter"
     ETXTBSY          "Text file busy"
     EFBIG            "File too large"
     ENOSPC           "No space left on device"
     ESPIPE           "Illegal seek"
     EROFS            "Read-only file system"
     EMLINK           "Too many links"
     EPIPE            "Broken pipe"
     ELOOP            "Too many levels of symbolic links"
     ENAMETOOLONG     "File name too long"
#v-
\example
  The \var{mkdir} function will attempt to create a directory.  If
  that directory already exists, the function will fail and set
  \var{errno} to \var{EEXIST}.
#v+
    define create_dir (dir)
    {
       if (0 == mkdir (dir)) return;
       if (errno != EEXIST)
         error ("mkdir %s failied: %s", dir, errno_string);
    }
#v-
\seealso{errno_string, error, mkdir}
\done

\function{errno_string}
\synopsis{Return a string describing an errno.}
\usage{String_Type errno_string (Integer_Type err)}
\description
  The \var{errno_string} function returns a string describing the
  integer error code \var{err}.  The variable \var{err} usually
  corresponds to the \var{errno} intrinsic function.  See the
  description for \var{errno} for more information.
\example
  The \var{errno_string} function may be used as follows:
#v+
    define sizeof_file (file)
    {
       variable st = stat (file);
       if (st == NULL)
         verror ("%s: %s", file, errno_string (errno);
       return st.st_size;
    }
#v-
\seealso{errno, stat, verror}
\done

\function{getegid}
\synopsis{Get the effective group id}
\usage{Int_Type getegid ()}
\description
  The \var{getegid} function returns the effective group ID of the
  current process.
\notes
  This function is not supported by all systems.
\seealso{getgid, geteuid, setgid}
\done

\function{geteuid}
\synopsis{Get the effective user-id of the current process}
\usage{Int_Type geteuid ()}
\description
  The \var{geteuid} function returns the effective user-id of the
  current process.
\notes
  This function is not supported by all systems.
\seealso{getuid, setuid, setgid}
\done

\function{getgid}
\synopsis{Get the group id}
\usage{Integer_Type getgid ()}
\description
  The \var{getgid} function returns the real group id of the current
  process.
\notes
  This function is not supported by all systems.
\seealso{getpid, getppid}
\done

\function{getpid}
\synopsis{Get the current process id}
\usage{Integer_Type getpid ()}
\description
  The \var{getpid} function returns the current process identification
  number.
\seealso{getppid, getgid}
\done

\function{getppid}
\synopsis{Get the parent process id}
\usage{Integer_Type getppid ()}
\description
  The \var{getpid} function returns the process identification
  number of the parent process.
\notes
  This function is not supported by all systems.
\seealso{getpid, getgid}
\done

\function{getuid}
\synopsis{Get the user-id of the current process}
\usage{Int_Type getuid ()}
\description
  The \var{getuid} function returns the user-id of the current
  process.
\notes
  This function is not supported by all systems.
\seealso{getuid, getegid}
\done

\function{kill}
\synopsis{Send a signal to a process}
\usage{Integer_Type kill (Integer_Type pid, Integer_Type sig)}
\description
  This function may be used to send a signal given by the integer \var{sig}
  to the process specified by \var{pid}.  The function returns zero upon
  success and \exmp{-1} upon failure setting errno accordingly.
\example
  The \var{kill} function may be used to determine whether or not
  a specific process exists:
#v+
    define process_exists (pid)
    {
       if (-1 == kill (pid, 0))
         return 0;     % Process does not exist
       return 1;
    }
#v-
\notes
  This function is not supported by all systems.
\seealso{getpid}
\done

\function{mkfifo}
\synopsis{Create a named pipe}
\usage{Int_Type mkfifo (String_Type name, Int_Type mode)}
\description
  The \var{mkfifo} attempts to create a named pipe with the specified
  name and mode (modified by the process's umask).  The function
  returns \0 upon success, or \-1 and sets \var{errno} upon failure.
\notes
  Not all systems support the \var{mkfifo} function and even on
  systems that do implement the \var{mkfifo} system call, the
  underlying file system may not support the concept of a named pipe,
  e.g, an NFS filesystem.
\seealso{stat_file}
\done

\function{setgid}
\synopsis{Set the group-id of the current process}
\usage{Int_Type setgid (Int_Type gid)}
\description
  The \var{setgid} function sets the effective group-id of the current
  process.  It returns zero upon success, or \-1 upon error and sets
  \var{errno} appropriately.
\notes
  This function is not supported by all systems.
\seealso{getgid, setuid}
\done

\function{setpgid}
\synopsis{Set the process group-id}
\usage{Int_Type setpgid (Int_Type pid, Int_Type gid)}
\description
  The \var{setpgid} function sets the group-id \var{gid} of the
  process whose process-id is \var{pid}.  If \var{pid} is \0, then the
  current process-id will be used.  If \var{pgid} is \0, then the pid
  of the affected process will be used.
  
  If successful zero will be returned, otherwise the function will
  return \-1 and set \var{errno} accordingly.
\notes
  This function is not supported by all systems.
\seealso{setgid, setuid}
\done

\function{setuid}
\synopsis{Set the user-id of the current process}
\usage{Int_Type setuid (Int_Type id)}
\description
  The \var{setuid} function sets the effective user-id of the current
  process.  It returns zero upon success, or \-1 upon error and sets
  \var{errno} appropriately.
\notes
  This function is not supported by all systems.
\seealso{setgid, setpgid, getuid, geteuid}
\done

\function{sleep}
\synopsis{Pause for a specified number of seconds}
\usage{sleep (Double_Type n)}
\description
  The \var{sleep} function delays the current process for the
  specified number of seconds.  If it is interrupted by a signal, it
  will return prematurely.
\notes
  Not all system support sleeping for a fractional part of a second.
\done

\function{system}
\synopsis{Execute a shell command}
\usage{Integer_Type system (String_Type cmd)}
\description
  The \var{system} function may be used to execute the string
  expression \var{cmd} in an inferior shell.  This function is an
  interface to the C \var{system} function which returns an
  implementation-defined result.   On Linux, it returns 127 if the
  inferior shell could not be invoked, -1 if there was some other
  error, otherwise it returns the return code for \var{cmd}.
\example
#v+
    define dir ()
    {
       () = system ("DIR");
    }
#v-
  displays a directory listing of the current directory under MSDOS or
  VMS.
\seealso{popen, listdir}
\done

\function{umask}
\synopsis{Set the file creation mask}
\usage{Int_Type umask (Int_Type m)}
\description
  The \var{umask} function sets the file creation mask to \var{m} and
  returns the previous mask.
\seealso{stat_file}
\done

\function{uname}
\synopsis{Get the system name}
\usage{Struct_Tye uname ()}
\description
  The \var{uname} function returns a structure containing information
  about the operating system.  The structure contains the following
  fields:
#v+
       sysname  (Name of the operating system)
       nodename (Name of the node within the network)
       release  (Release level of the OS)
       version  (Current version of the release)
       machine  (Name of the hardware)
#v-
\notes
  Not all systems support this function.
\seealso{getenv, pack, unpack}
\done

