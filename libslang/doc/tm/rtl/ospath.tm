\function{path_basename}
\synopsis{Get the basename part of a pathname}
\usage{String_Type path_basename (String_Type path)}
\description
   The \var{path_basename} function returns the basename associated
   with the \var{path} parameter.  The basename is the non-directory
   part of the filename, e.g., on unix \exmp{c} is the basename of
   \exmp{/a/b/c}.
\seealso{path_dirname, path_extname, path_concat, path_is_absolute}
\done

\function{path_concat}
\synopsis{Combine elements of a pathname}
\usage{String_Type path_concat (String_Type dir, String_Type basename)}
\description
   The \var{path_concat} function combines the arguments \var{dir} and
   \var{basename} to produce a pathname.  For example, on unix is
   \var{dir} is \exmp{x/y} and \var{basename} is \exmp{z}, then the
   function will return \exmp{x/y/z}.
\seealso{path_dirname, path_basename, path_extname, path_is_absolute}
\done

\function{path_dirname}
\synopsis{Get the directory name part of a pathname}
\usage{String_Type path_dirname (String_Type path)}
\description
   The \var{path_dirname} function returns the directory name
   associated with a specified pathname.
\notes
   On systems that include a drive specifier as part of the pathname,
   the value returned by this function will include the driver
   specifier.
\seealso{path_basename, path_extname, path_concat, path_is_absolute}
\done

\function{path_extname}
\synopsis{Return the extension part of a pathname}
\usage{String_Type path_extname (String_Type path)}
\description
   The \var{path_extname} function returns the extension portion of a
   specified pathname.  If an extension is present, this function will
   also include the dot as part of the extension, i.e., if \var{path}
   is \exmp{file.c}, then this function returns \exmp{".c"}.  If no
   extension is present, the function returns an empty string \exmp{""}.
\notes
   Under VMS, the file version number is not returned as part of the
   extension.
\seealso{path_sans_extname, path_dirname, path_basename, path_concat, path_is_absolute}
\done

\function{path_get_delimiter}
\synopsis{Get the value of a search-path delimiter}
\usage{Char_Type path_get_delimiter ()}
\description
  This function returns the value of the character used to delimit
  fields of a search-path.
\seealso{set_slang_load_path, get_slang_load_path}
\done

\function{path_is_absolute}
\synopsis{Determine whether or not a pathname is absolute}
\usage{Int_Type path_is_absolute (String_Type path)}
\description
   The \var{path_is_absolute} function will return non-zero is
   \var{path} refers to an absolute pathname, otherwise it returns zero.
\seealso{path_dirname, path_basename, path_extname, path_concat}
\done

\function{path_sans_extname}
\synopsis{Strip the extension from a pathname}
\usage{String_Type path_sans_extname (String_Type path)}
\description
  The \var{path_sans_extname} function removes the file name extension
  (including the dot) from the path and returns the result.
\seealso{path_extname, path_basename, path_dirname, path_concat}
\done

