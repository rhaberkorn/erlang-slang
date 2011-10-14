\function{_push_struct_field_values}
\synopsis{Push the values of a structure's fields onto the stack}
\usage{Integer_Type num = _push_struct_field_values (Struct_Type s)}
\description
  The \var{_push_struct_field_values} function pushes the values of
  all the fields of a structure onto the stack, returning the
  number of items pushed.  The fields are pushed such that the last
  field of the structure is pushed first.
\seealso{get_struct_field_names, get_struct_field}
\done

\function{get_struct_field}
\synopsis{Get the value associated with a structure field}
\usage{x = get_struct_field (Struct_Type s, String field_name)}
\description
   The \var{get_struct_field} function gets the value of the field
   whose name is specified by \var{field_name} of the structure \var{s}.
\example
   The following example illustrates how this function may be used to 
   to print the value of a structure.
#v+
      define print_struct (s)
      {
         variable name;

         foreach (get_struct_field_names (s))
           {
             name = ();
             value = get_struct_field (s, name);
             vmessage ("s.%s = %s\n", name, string(value));
           }
      }
#v-
\seealso{set_struct_field, get_struct_field_names, array_info}

\done

\function{get_struct_field_names}
\synopsis{Retrieve the field names associated with a structure}
\usage{String_Type[] = get_struct_field_names (Struct_Type s)}
\description
   The \var{get_struct_field_names} function returns an array of
   strings whose elements specify the names of the fields of the
   struct \var{s}.
\example
   The following example illustrates how the
   \var{get_struct_field_names} function may be used to print the
   value of a structure.
#v+
      define print_struct (s)
      {
         variable name, value;

         foreach (get_struct_field_names (s))
           {
             name = ();
             value = get_struct_field (s, name);
             vmessage ("s.%s = %s\n", name, string (value));
           }
      }
#v-
\seealso{_push_struct_field_values, get_struct_field}
\done

\function{is_struct_type}
\synopsis{Determine whether or not an object is a structure}
\usage{Integer_Type is_struct_type (X)}
\description
  The \var{is_struct_type} function returns \1 if the parameter
  refers to a structure or a user-defined type.  If the object is
  neither, \0 will be returned.
\seealso{typeof, _typeof}
\done

\function{set_struct_field}
\synopsis{Set the value associated with a structure field}
\usage{set_struct_field (s, field_name, field_value)}
#v+
   Struct_Type s;
   String_Type field_name;
   Generic_Type field_value;
#v-
\description
   The \var{set_struct_field} function sets the value of the field
   whose name is specified by \var{field_name} of the structure
   \var{s} to \var{field_value}.
\seealso{get_struct_field, get_struct_field_names, set_struct_fields, array_info}
\done

\function{set_struct_fields}
\synopsis{Set the fields of a structure}
\usage{set_struct_fields (Struct_Type s, ...)}
\description
  The \var{set_struct_fields} function may be used to set zero or more
  fields of a structure.  The fields are set in the order in which
  they were created when the structure was defined.
\example
#v+
    variable s = struct { name, age, height };
    set_struct_fields (s, "Bill", 13, 64);
#v-
\seealso{set_struct_field, get_struct_field_names}
\done

