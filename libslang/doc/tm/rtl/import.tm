\function{get_import_module_path}
\synopsis{Get the search path for dynamically loadable objects}
\usage{String_Type get_import_module_path ()}
\description
  The \var{get_import_module_path} may be used to get the search path
  for dynamically shared objects.  Such objects may be made accessable
  to the application via the \var{import} function.
\seealso{import, set_import_module_path}
\done

\function{import}
\synopsis{Dynamically link to a specified module}
\usage{import (String_Type module [, String_Type namespace])}
\description
  The \var{import} function causes the run-time linker to dynamically
  link to the shared object specified by the \var{module} parameter.
  It seaches for the shared object as follows: First a search is
  performed along all module paths specified by the application.  Then
  a search is made along the paths defined via the
  \var{set_import_module_path} function.  If not found, a search is
  performed along the paths given by the \var{SLANG_MODULE_PATH}
  environment variable.  Finally, a system dependent search is
  performed (e.g., using the \var{LD_LIBRARY_PATH} environment
  variable).

  The optional second parameter may be used to specify a namespace
  for the intrinsic functions and variables of the module.  If this
  parameter is not present, the intrinsic objects will be placed into
  the global namespace.

  This function signals an error if the specified module is not found.
\notes
  The \var{import} function is not available on all systems.
\seealso{set_import_module_path, use_namespace, current_namespace, getenv, evalfile}
\done

\function{set_import_module_path}
\synopsis{Set the search path for dynamically loadable objects}
\usage{set_import_module_path (String_Type path_list)}
\description
  The \var{set_import_module_path} may be used to set the search path
  for dynamically shared objects.  Such objects may be made accessable
  to the application via the \var{import} function.
  
  The actual syntax for the specification of the set of paths will
  vary according to the operating system.  Under Unix, a colon
  character is used to separate paths in \var{path_list}.  For win32
  systems a semi-colon is used.
\seealso{import, get_import_module_path}
\done

