\function{chdir}
\synopsis{Change the current working directory.}
\usage{Integer_Type chdir (String_Type dir)}
\description
  The \var{chdir} function may be used to changed the current working
  directory to the directory specified by \var{dir}.  Upon success it
  returns zero; however, upon failure it returns \exmp{-1} and sets
  \var{errno} accordingly.
\seealso{mkdir, stat_file}
\done

\function{chmod}
\synopsis{Change the mode of a file}
\usage{Integer_Type chmod (String_Type file, Integer_Type mode)}
\description
  The \var{chmod} function changes the permissions of \var{file} to those
  specified by \var{mode}.  It returns \exmp{0} upon success, or
  \exmp{-1} upon failure setting \var{errno} accordingly.

  See the system specific documentation for the C library
  function \var{chmod} for a discussion of the \var{mode} parameter.
\seealso{chown, stat_file}
\done

\function{chown}
\synopsis{Change the owner of a file}
\usage{Integer_Type chown (String_Type file, Integer_Type uid, Integer_Type gid)}
\description
  The \var{chown} function is used to change the user-id and group-id of
  \var{file} to \var{uid} and \var{gid}, respectively.  It returns
  \var{zero} upon success and \exmp{-1} upon failure, with \var{errno}
  set accordingly.
\notes
  On most systems, only the super user can change the ownership of a
  file.

  Some systems do not support this function.
\seealso{chmod, stat_file}
\done

\function{getcwd}
\synopsis{Get the current working directory}
\usage{String_Type getcwd ()}
\description
  The \var{getcwd} function returns the absolute pathname of the
  current working directory.  If an error occurs or it cannot
  determine the working directory, it returns \var{NULL} and sets
  \var{errno} accordingly.
\notes
  Under Unix, OS/2, and MSDOS, the pathname returned by this function
  includes the trailing slash character.  Some versions also include
  the drive specifier.
\seealso{mkdir, chdir, errno}
\done

\function{listdir}
\synopsis{Get a list of the files in a directory}
\usage{String_Type[] listdir (String_Type dir)}
\description
  The \var{listdir} function returns the directory listing of all the
  files in the specified directory \var{dir} as an array of strings.
  It does not return the special files \exmp{".."} and \exmp{"."} as
  part of the list.
\seealso{stat_file, stat_is, length}
\done

\function{lstat_file}
\synopsis{Get information about a symbolic link}
\usage{Struct_Type lstat_file (String_Type file)}
\description
  The \var{lstat_file} function behaves identically to \var{stat_file}
  but if \var{file} is a symbolic link, \var{lstat_file} returns
  information about the link itself, and not the file that it
  references.

  See the documentation for \var{stat_file} for more information.
\notes
  On systems that do not support symbolic links, there is no
  difference between this function and the \var{stat_file} function.
\seealso{stat_file, readlink}
\done

\function{mkdir}
\synopsis{Create a new directory}
\usage{Integer_Type mkdir (String_Type dir, Integer_Type mode)}
\description
  The \var{mkdir} function creates a directory whose name is specified
  by the \var{dir} parameter with permissions specified by \var{mode}.
  Upon success \var{mkdir} returns zero, or it returns \exmp{-1} and
  sets \var{errno} accordingly.  In particular, if the directory
  already exists, the function will fail and set errno to
  \var{EEXIST}.
\example
#v+
     define my_mkdir (dir)
     {
        if (0 == mkdir (dir, 0777)) return;
        if (errno == EEXIST) return;
        verror ("mkdir %s failed: %s", dir, errno_string (errno));
     }
#v-
\notes
  The \var{mode} parameter may not be meaningful on all systems.  On
  systems where it is meaningful, the actual permissions on the newly
  created directory are modified by the process's umask.
\seealso{rmdir, getcwd, chdir, fopen, errno}
\done

\function{readlink}
\synopsis{String_Type readlink (String_Type path)}
\usage{Get the value of a symbolic link}
\description
  The \var{readlink} function returns the value of a symbolic link and
  returns it as a string.  Upon failure, \NULL is returned and
  \var{errno} set accordingly.
\notes
  Not all systems support this function.
\seealso{lstat_file, stat_file, stat_is}
\done

\function{remove}
\synopsis{Delete a file}
\usage{Integer_Type remove (String_Type file)}
\description
  The \var{remove} function deletes a file.  It returns \0 upon
  success, or \-1 upon error and sets \var{errno} accordingly.
\seealso{rename, rmdir}
\done

\function{rename}
\synopsis{Rename a file}
\usage{Integer_Type rename (String_Type old, String_Type new)}
\description
  The \var{rename} function renames a file from \var{old} to \var{new}
  moving it between directories if necessary.  This function may fail
  if the directories do not refer to the same file system.  It returns
  \0 upon success, or \-1 upon error and sets \var{errno} accordingly.
\seealso{remove, errno}
\done

\function{rmdir}
\synopsis{Remove a directory}
\usage{Integer_Type rmdir (String_Type dir)}
\description
  The \var{rmdir} function deletes a specified directory.  It returns
  \0 upon success or \-1 upon error and sets \var{errno} accordingly.
\notes
  The directory must be empty before it can be removed.
\seealso{rename, remove, mkdir}
\done

\function{stat_file}
\synopsis{Get information about a file}
\usage{Struct_Type stat_file (String_Type file)}
\description
  The \var{stat_file} function returns information about \var{file}
  through the use of the system \var{stat} call.  If the stat call
  fails, the function returns \var{NULL} and sets errno accordingly.
  If it is successful, it returns a stat structure with the following
  integer fields:
#v+
    st_dev
    st_ino
    st_mode
    st_nlink
    st_uid
    st_gid
    st_rdev
    st_size
    st_atime
    st_mtime
    st_ctime
#v-
  See the man page for \var{stat} for a discussion of these fields.
\example
  The following example shows how the \var{stat_file} function may be
  used to get the size of a file:
#v+
     define file_size (file)
     {
        variable st;
        st = stat_file(file);
        if (st == NULL) verror ("Unable to stat %s", file);
        return st.st_size;
     }
#v-
\seealso{lstat_file, stat_is}
\done

\function{stat_is}
\synopsis{Parse the \var{st_mode} field of a stat structure}
\usage{Char_Type stat_is (String_Type type, Integer_Type st_mode)}
\description
  The \var{stat_is} function returns a signed character value about
  the type of file specified by \var{st_mode}.  Specifically,
  \var{type} must be one of the strings:
#v+
     "sock"     (socket)
     "fifo"     (fifo)
     "blk"      (block device)
     "chr"      (character device)
     "reg"      (regular file)
     "lnk"      (link)
     "dir"      (dir)
#v-
  It returns a non-zero value if \var{st_mode} corresponds to
  \var{type}.
\example
  The following example illustrates how to use the \var{stat_is}
  function to determine whether or not a file is a directory:
#v+
     define is_directory (file)
     {
        variable st;

        st = stat_file (file);
        if (st == NULL) return 0;
        return stat_is ("dir", st.st_mode);
     }
#v-
\seealso{stat_file, lstat_file}
\done

