#i linuxdoc.tm

#d slang \bf{S-lang}
#d jed \bf{jed}
#d slang-c-programmers-guide \em{S-Lang Library C Programmer's Guide}
#d kw#1 \tt{$1}
#d exmp#1 \tt{$1}
#d var#1 \tt{$1}
#d ldots ...
#d chapter#1 <chapt>$1<p>
#d preface <preface>
#d tag#1 <tag>$1</tag>

#d function#1 \sect{<bf>$1</bf>\label{$1}}<descrip>
#d variable#1 \sect{<bf>$1</bf>\label{$1}}<descrip>
#cd function#1 <p><bf>$1</bf>\label{$1}<p><descrip>
#d synopsis#1 <tag> Synopsis </tag> $1
#d keywords#1 <tag> Keywords </tag> $1
#d usage#1 <tag> Usage </tag> <tt>$1</tt>
#d description <tag> Description </tag>
#d example <tag> Example </tag>
#d notes <tag> Notes </tag>
#d seealso#1 <tag> See Also </tag> <tt>$1</tt>
#d documentstyle article
#d r#1 \ref{$1}{$1}
#d done </descrip><p>
#d 0 \exmp{0}
#d -1 \exmp{-1}
#d 1 \exmp{1}

\linuxdoc
\begin{\documentstyle}

\title {The \slang C Library Reference}
\author John E. Davis, \tt{davis@space.mit.edu}
\date \__today__

\toc


\function{SLsmg_fill_region}
\synopsis{Fill a rectangular region with a character}
\usage{void SLsmg_fill_region (r, c, nr, nc, ch)}
#v+
    int r
    int c
    unsigned int nr
    unsigned int nc
    unsigned char ch
#v-
\description
  The \var{SLsmg_fill_region} function may be used to a 
  rectangular region with the character \var{ch} in the current color.
  The rectangle's upper left corner is at row \var{r} and column
  \var{c}, and spans \var{nr} rows and \var{nc} columns.  The position
  of the virtual cursor will be left at (\var{r}, \var{c}).
\seealso{SLsmg_write_char, SLsmg_set_color}
\done



\function{SLsmg_set_char_set}
\synopsis{Turn on or off line drawing characters}
\usage{void SLsmg_set_char_set (int a);}
\description
  \var{SLsmg_set_char_set} may be used to select or deselect the line drawing
  character set as the current character set.  If \var{a} is non-zero,
  the line drawing character set will be selected.  Otherwise, the
  standard character set will be selected.
\notes
  There is no guarantee that this function will actually enable the
  use of line drawing characters.  All it does is cause subsequent
  characters to be rendered using the terminal's alternate character
  set.  Such character sets usually contain line drawing characters.
\seealso{SLsmg_write_char, SLtt_get_terminfo}
\done


\variable{int SLsmg_Scroll_Hash_Border;}
\synopsis{Set the size of the border for the scroll hash}
\usage{int SLsmg_Scroll_Hash_Border = 0;}
\description
  This variable may be used to ignore the characters that occur at the
  beginning and the end of a row when performing the hash calculation
  to determine whether or not a line has scrolled.  The default value
  is zero which means that all the characters on a line will be used.
\seealso{SLsmg_refresh}
\done


\function{SLsmg_suspend_smg}
\synopsis{Suspend screen management}
\usage{int SLsmg_suspend_smg (void)}
\description
  \var{SLsmg_suspend_smg} can be used to suspend the state of the
  screen management facility during suspension of the program.  Use of
  this function will reset the display back to its default state.  The
  funtion \var{SLsmg_resume_smg} should be called after suspension.  

  It returns zero upon success, or \-1 upon error.

  This function is similar to \var{SLsmg_reset_smg} except that the
  state of the display prior to calling \var{SLsmg_suspend_smg} is saved.
\seealso{SLsmg_resume_smg, SLsmg_reset_smg}
\done


\function{SLsmg_resume_smg}
\synopsis{Resume screen management}
\usage{int SLsmg_resume_smg (void)}
\description
  \var{SLsmg_resume_smg} should be called after
  \var{SLsmg_suspend_smg} to redraw the display exactly like it was
  before \var{SLsmg_suspend_smg} was called.  It returns zero upon
  success, or \-1 upon error.
\seealso{SLsmg_suspend_smg}
\done


\function{SLsmg_erase_eol}
\synopsis{Erase to the end of the row}
\usage{void SLsmg_erase_eol (void);}
\description
  \var{SLsmg_erase_eol} erases all characters from the current
  position to the end of the line.  The newly created space is given
  the color of the current color.  This function has no effect on the
  position of the virtual cursor.
\seealso{SLsmg_gotorc, SLsmg_erase_eos, SLsmg_fill_region}
\done


\function{SLsmg_gotorc}
\synopsis{Move the virtual cursor}
\usage{void SLsmg_gotorc (int r, int c)}
\description
  The \var{SLsmg_gotorc} function moves the virtual cursor to the row
  \var{r} and column \var{c}.  The first row and first column is
  specified by \exmp{r = 0} and \exmp{c = 0}.
\seealso{SLsmg_refresh}
\done


\function{SLsmg_erase_eos}
\synopsis{Erase to the end of the screen}
\usage{void SLsmg_erase_eos (void);}
\description
  The \var{SLsmg_erase_eos} is like \var{SLsmg_erase_eol} except that
  it erases all text from the current position to the end of the
  display.  The current color will be used to set the background of
  the erased area.
\seealso{SLsmg_erase_eol}
\done


\function{SLsmg_reverse_video}
\synopsis{Set the current color to 1}
\usage{void SLsmg_reverse_video (void);}
\description
  This function is nothing more than \exmp{SLsmg_set_color(1)}.  
\seealso{SLsmg_set_color}
\done


\function{SLsmg_set_color (int)}
\synopsis{Set the current color}
\usage{void SLsmg_set_color (int c);}
\description
  \var{SLsmg_set_color} is used to set the current color.  The
  parameter \var{c} is really a color object descriptor.  Actual
  foreground and background colors as well as other visual attributes
  may be associated with a color descriptor via the
  \var{SLtt_set_color} function.
\example
  This example defines color \exmp{7} to be green foreground on black
  background and then displays some text in this color:
#v+
      SLtt_set_color (7, NULL, "green", "black");
      SLsmg_set_color (7);
      SLsmg_write_string ("Hello");
      SLsmg_refresh ();
#v-
\notes
  It is important to understand that the screen managment routines
  know nothing about the actual colors associated with a color
  descriptor.  Only the descriptor itself is used by the \var{SLsmg}
  routines.  The lower level \var{SLtt} interface converts the color
  descriptors to actual colors.  Thus
#v+
      SLtt_set_color (7, NULL, "green", "black");
      SLsmg_set_color (7);
      SLsmg_write_string ("Hello");
      SLtt_set_color (7, NULL, "red", "blue");
      SLsmg_write_string ("World");
      SLsmg_refresh ();
#v-
  will result in \exmp{"hello"} displayed in red on blue and \em{not}
  green on black.
\seealso{SLtt_set_color, SLtt_set_color_object}
\done


\function{SLsmg_normal_video}
\synopsis{Set the current color to 0}
\usage{void SLsmg_normal_video (void);}
\description
  \var{SLsmg_normal_video} sets the current color descriptor to \var{0}.
\seealso{SLsmg_set_color}
\done


\function{SLsmg_printf}
\synopsis{Format a string on the virtual display}
\usage{void SLsmg_printf (char *fmt, ...)}
\description
  \var{SLsmg_printf} format a \var{printf} style variable argument
  list and writes it on the virtual display.  The virtual cursor will
  be moved to the end of the string.
\seealso{SLsmg_write_string, SLsmg_vprintf}
\done


\function{SLsmg_vprintf}
\synopsis{Format a string on the virtual display}
\usage{void SLsmg_vprintf (char *fmt, va_list ap)}
\description
  \var{SLsmg_vprintf} formats a string in the manner of \em{vprintf}
  and writes the result to the display.  The virtual cursor is
  advanced to the end of the string.
\seealso{SLsmg_write_string, SLsmg_printf}
\done


\function{SLsmg_write_string}
\synopsis{Write a character string on the display }
\usage{void SLsmg_write_string (char *s)}
\description
  The function \var{SLsmg_write_string} displays the string \var{s} on
  the virtual display at the current position and moves the position
  to the end of the string.
\seealso{SLsmg_printf, SLsmg_write_nstring}
\done


\function{SLsmg_write_nstring}
\synopsis{Write the first n characters of a string on the display}
\usage{void SLsmg_write_nstring (char *s, unsigned int n);}
\description
  \var{SLsmg_write_nstring} writes the first \var{n} characters of
  \var{s} to this virtual display.  If the length of the string
  \var{s} is less than \var{n}, the spaces will used until
  \var{n} characters have been written.  \var{s} can be \var{NULL}, in
  which case \var{n} spaces will be written.
\seealso{SLsmg_write_string, SLsmg_write_nchars}
\done


\function{SLsmg_write_char}
\synopsis{Write a character to the virtual display}
\usage{void SLsmg_write_char (char ch);}
\description
  \var{SLsmg_write_char} writes the character \var{ch} to the virtual
  display.
\seealso{SLsmg_write_nchars, SLsmg_write_string}
\done


\function{SLsmg_write_nchars}
\synopsis{Write n characters to the virtual display}
\usage{void SLsmg_write_nchars (char *s, unsigned int n);}
\description
  \var{SLsmg_write_nchars} writes at most \var{n} characters from the
  string \var{s} to the display.  If the length of \var{s} is less
  than \var{n}, the whole length of the string will get written.
  
  This function differs from \var{SLsmg_write_nstring} in that
  \var{SLsmg_write_nstring} will pad the string to write exactly
  \var{n} characters.  \var{SLsmg_write_nchars} does not perform any
  padding.
\seealso{SLsmg_write_nchars, SLsmg_write_nstring}
\done


\function{SLsmg_write_wrapped_string}
\synopsis{Write a string to the display with wrapping}
\usage{void SLsmg_write_wrapped_string (s, r, c, nr, nc, fill)}
#v+
    char *s
    int r, c
    unsigned int nr, nc
    int fill
#v-
\description
  \var{SLsmg_write_wrapped_string} writes the string \var{s} to the
  virtual display.  The string will be confined to the rectangular
  region whose upper right corner is at row \var{r} and column \var{c},
  and consists of \var{nr} rows and \var{nc} columns.  The string will
  be wrapped at the boundaries of the box.  If \var{fill} is non-zero,
  the last line to which characters have been written will get padded
  with spaces.
\notes
  This function does not wrap on word boundaries.  However, it will
  wrap when a newline charater is encountered.
\seealso{SLsmg_write_string}
\done


\function{SLsmg_cls}
\synopsis{Clear the virtual display}
\usage{void SLsmg_cls (void)}
\description
  \var{SLsmg_cls} erases the virtual display using the current color.
  This will cause the physical display to get cleared the next time
  \var{SLsmg_refresh} is called.
\notes
  This function is not the same as
#v+
     SLsmg_gotorc (0,0); SLsmg_erase_eos ();
#v-
  since these statements do not guarantee that the physical screen
  will get cleared.
\seealso{SLsmg_refresh, SLsmg_erase_eos}
\done


\function{SLsmg_refresh}
\synopsis{Update physical screen}
\usage{void SLsmg_refresh (void)}
\description
  The \var{SLsmg_refresh} function updates the physical display to
  look like the virtual display.
\seealso{SLsmg_suspend_smg, SLsmg_init_smg, SLsmg_reset_smg}
\done


\function{SLsmg_touch_lines}
\synopsis{Mark lines on the virtual display for redisplay}
\usage{void SLsmg_touch_lines (int r, unsigned int nr)}
\description
  \var{SLsmg_touch_lines} marks the \var{nr} lines on the virtual
  display starting at row \var{r} for redisplay upon the next call to
  \var{SLsmg_refresh}.
\notes
  This function should rarely be called, if ever.  If you find that
  you need to call this function, then your application should be
  modified to properly use the \var{SLsmg} screen management routines.
  This function is provided only for curses compatibility.
\seealso{SLsmg_refresh}
\done

\function{SLsmg_init_smg}
\synopsis{Initialize the \var{SLsmg} routines}
\usage{int SLsmg_init_smg (void)}
\description
  The \var{SLsmg_init_smg} function initializes the \var{SLsmg} screen
  management routines.   Specifically, this function allocates space
  for the virtual display and calls \var{SLtt_init_video} to put the
  terminal's physical display in the proper state.  It is up to the
  caller to make sure that the \var{SLtt} routines are initialized via
  \var{SLtt_get_terminfo} before calling \var{SLsmg_init_smg}.
  
  This function should also be called any time the size of the
  physical display has changed so that it can reallocate a new virtual
  display to match the physical display.
  
  It returns zero upon success, or \-1 upon failure.
\seealso{SLsmg_reset_smg}
\done


\function{SLsmg_reset_smg}
\synopsis{Reset the \var{SLsmg} routines}
\usage{int SLsmg_reset_smg (void);}
\description
  \var{SLsmg_reset_smg} resets the \var{SLsmg} screen management
  routines by freeing all memory allocated while it was active.  It
  also calls \var{SLtt_reset_video} to put the terminal's display in
  it default state.
\seealso{SLsmg_init_smg}
\done


\function{SLsmg_char_at}
\synopsis{Get the character at the current position on the virtual display}
\usage{unsigned short SLsmg_char_at(void)}
\description
  The \var{SLsmg_char_at} function returns the character and its color
  at the current position on the virtual display.
\seealso{SLsmg_read_raw, SLsmg_write_char}
\done


\function{SLsmg_set_screen_start}
\synopsis{Set the origin of the virtual display}
\usage{void SLsmg_set_screen_start (int *r, int *c)}
\description
  \var{SLsmg_set_screen_start} sets the origin of the virtual display
  to the row \var{*r} and the column \var{*c}.  If either \var{r} or \var{c}
  is \var{NULL}, then the corresponding value will be set to \var{0}.
  Otherwise, the location specified by the pointers will be updated to
  reflect the old origin.
  
  See \tt{slang/demo/pager.c} for how this function may be used to
  scroll horizontally.
\seealso{SLsmg_init_smg}
\done


\function{SLsmg_draw_hline}
\synopsis{Draw a horizontal line}
\usage{void SLsmg_draw_hline (unsigned int len)}
\description
  The \var{SLsmg_draw_hline} function draws a horizontal line of
  length \var{len} on the virtual display.  The position of the
  virtual cursor is left at the end of the line.
\seealso{SLsmg_draw_vline}
\done


\function{SLsmg_draw_vline}
\synopsis{Draw a vertical line}
\usage{void SLsmg_draw_vline (unsigned int len);}
\description
  The \var{SLsmg_draw_vline} function draws a vertical line of
  length \var{len} on the virtual display.  The position of the
  virtual cursor is left at the end of the line.
\seealso{??}
\done


\function{SLsmg_draw_object}
\synopsis{Draw an object from the alternate character set}
\usage{void SLsmg_draw_object (int r, int c, unsigned char obj)}
\description
  The \var{SLsmg_draw_object} function may be used to place the object
  specified by \var{obj} at row \var{r} and column \var{c}.  The
  object is really a character from the alternate character set and
  may be specified using one of the following constants:
#v+
    SLSMG_HLINE_CHAR         Horizontal line 
    SLSMG_VLINE_CHAR         Vertical line 
    SLSMG_ULCORN_CHAR        Upper left corner 
    SLSMG_URCORN_CHAR        Upper right corner
    SLSMG_LLCORN_CHAR        Lower left corner
    SLSMG_LRCORN_CHAR        Lower right corner
    SLSMG_CKBRD_CHAR         Checkboard character
    SLSMG_RTEE_CHAR          Right Tee
    SLSMG_LTEE_CHAR          Left Tee
    SLSMG_UTEE_CHAR          Up Tee
    SLSMG_DTEE_CHAR          Down Tee
    SLSMG_PLUS_CHAR          Plus or Cross character
#v-
\seealso{SLsmg_draw_vline, SLsmg_draw_hline, SLsmg_draw_box}
\done


\function{SLsmg_draw_box}
\synopsis{Draw a box on the virtual display}
\usage{void SLsmg_draw_box (int r, int c, unsigned int dr, unsigned int dc)}
\description
  \var{SLsmg_draw_box} uses the \var{SLsmg_draw_hline} and
  \var{SLsmg_draw_vline} functions to draw a rectangular box on the
  virtual display.  The box's upper left corner is placed at row
  \var{r} and column \var{c}.  The width and length of the box is
  specified by \var{dc} and \var{dr}, respectively.
\seealso{SLsmg_draw_vline, SLsmg_draw_hline, SLsmg_draw_object}
\done

\function{SLsmg_set_color_in_region}
\synopsis{Change the color of a specifed region}
\usage{void SLsmg_set_color_in_region (color, r, c, dr, dc)}
#v+
  int color;
  int r, c;
  unsigned int dr, dc;
#v-
\description
  \var{SLsmg_set_color_in_region} may be used to change the color of a
  rectangular region whose upper left corner is given by
  (\var{r},\var{c}), and whose width and height is given by \var{dc}
  and \var{dr}, respectively.  The color of the region is given by the
  \var{color} parameter.
\seealso{SLsmg_draw_box, SLsmg_set_color}
\done


\function{SLsmg_get_column}
\synopsis{Get the column of the virtual cursor}
\usage{int SLsmg_get_column(void);}
\description
  The \var{SLsmg_get_column} function returns the current column of
  the virtual cursor on the virtual display.
\seealso{SLsmg_get_row, SLsmg_gotorc}
\done


\function{SLsmg_get_row}
\synopsis{Get the row of the virtual cursor}
\usage{int SLsmg_get_row(void);}
\description
  The \var{SLsmg_get_row} function returns the current row of the
  virtual cursor on the virtual display.
\seealso{SLsmg_get_column, SLsmg_gotorc}
\done


\function{SLsmg_forward}
\synopsis{Move the virtual cursor forward n columns}
\usage{void SLsmg_forward (int n);}
\description
  The \var{SLsmg_forward} function moves the virtual cursor forward
  \var{n} columns.
\seealso{SLsmg_gotorc}
\done


\function{SLsmg_write_color_chars}
\synopsis{Write characters with color descriptors to virtual display}
\usage{void SLsmg_write_color_chars (unsigned short *s, unsigned int len)}
\description
  The \var{SLsmg_write_color_chars} function may be used to write
  \var{len} characters, each with a different color descriptor to the
  virtual display.  Each character and its associated color are
  encoded as an \exmp{unsigned short} such that the lower eight bits
  form the character and the next eight bits form the color.
\seealso{SLsmg_char_at, SLsmg_write_raw}
\done


\function{SLsmg_read_raw}
\synopsis{Read characters from the virtual display}
\usage{unsigned int SLsmg_read_raw (unsigned short *buf, unsigned int len)}
\description
  \var{SLsmg_read_raw} attempts to read \var{len} characters from the
  current position on the virtual display into the buffer specified by
  \var{buf}.  It returns the number of characters actually read.  This
  number will be less than \var{len} if an attempt is made to read
  past the right margin of the display.
\notes
  The purpose of the pair of functions, \var{SLsmg_read_raw} and
  \var{SLsmg_write_raw}, is to permit one to copy the contents of one
  region of the virtual display to another region.
\seealso{SLsmg_char_at, SLsmg_write_raw}
\done


\function{SLsmg_write_raw}
\synopsis{Write characters directly to the virtual display}
\usage{unsigned int SLsmg_write_raw (unsigned short *buf, unsigned int len)}
\description
  The \var{SLsmg_write_raw} function attempts to write \var{len}
  characters specified by \var{buf} to the display at the current
  position.  It returns the number of characters successfully written,
  which will be less than \var{len} if an attempt is made to write
  past the right margin.
\notes
  The purpose of the pair of functions, \var{SLsmg_read_raw} and
  \var{SLsmg_write_raw}, is to permit one to copy the contents of one
  region of the virtual display to another region.
\seealso{SLsmg_read_raw}
\done


\function{SLallocate_load_type}
\synopsis{Allocate a SLang_Load_Type object}
\usage{SLang_Load_Type *SLallocate_load_type (char *name)}
\description
  The \var{SLallocate_load_type} function allocates and initializes
  space for a \var{SLang_Load_Type} object and returns it.  Upon
  failure, the function returns \var{NULL}.  The parameter \var{name}
  must uniquely identify the object.  For example, if the object
  represents a file, then \var{name} could be the absolute path name
  of the file.
\seealso{SLdeallocate_load_type, SLang_load_object}
\done

\function{SLdeallocate_load_type}
\synopsis{Free a SLang_Load_Type object}
\usage{void SLdeallocate_load_type (SLang_Load_Type *slt)}
\description
  This function frees the memory associated with a
  \var{SLang_Load_Type} object that was acquired from a call to the
  \var{SLallocate_load_type} function.
\seealso{SLallocate_load_type, SLang_load_object}
\done


\function{SLang_load_object}
\synopsis{Load an object into the interpreter}
\usage{int SLang_load_object (SLang_Load_Type *obj)}
\description
  The function \var{SLang_load_object} is a generic function that may
  be used to loaded an object of type \var{SLang_Load_Type} into the
  interpreter.  For example, the functions \var{SLang_load_file} and
  \var{SLang_load_string} are wrappers around this function to load a
  file and a string, respectively.
\seealso{SLang_load_file, SLang_load_string, SLallocate_load_type}
\done


\function{SLclass_allocate_class}
\synopsis{Allocate a class for a new data type}
\usage{SLang_Class_Type *SLclass_allocate_class (char *name)}
\description
  The purpose of this function is to allocate and initialize space
  that defines a new data type or class called \var{name}.  If
  successful, a pointer to the class is returned, or upon failure the
  function returns \var{NULL}.
  
  This function does not automatically create the new data type.
  Callback functions must first be associated with the data type via
  functions such as \var{SLclass_set_push_function}, and the data
  type must be registered with the interpreter via
  \var{SLclass_register_class}.  See the \slang library programmer's
  guide for more information.
\seealso{SLclass_register_class, SLclass_set_push_function}
\done


\function{SLclass_register_class}
\synopsis{Register a new data type with the interpreter}
\usage{int SLclass_register_class (cl, type, sizeof_type, class_type)}
#v+
    SLang_Class_Type *cl
    unsigned char type
    unsigned int sizeof_type
    unsigned char class_type
#v-
\description
  The \var{SLclass_register_class} function is used to register a new
  class or data type with the interpreter.  If successful, the
  function returns \exmp{0}, or upon failure, it returns \var{-1}.

  The first parameter, \var{cl}, must have been previously obtained
  via the \var{SLclass_allocate_class} function.

  The second parameter, \var{type} specifies the data type of the new
  class.  It must be an unsigned character with value greater that
  \exmp{127}.  The values in the range \exmp{0-127} are reserved for
  internal use by the library.

  The size that the data type represents in bytes is specified by the
  third parameter, \var{sizeof_type}.   This value should not be
  confused with the sizeof the structure that represents the data
  type, unless the data type is of class \var{SLANG_CLASS_TYPE_VECTOR}
  or \var{SLANG_CLASS_TYPE_SCALAR}.  For pointer objects, the value
  of this parameter is just \var{sizeof(void *)}.

  The final parameter specifies the class type of the data type.  It must
  be one of the values:
#v+
     SLANG_CLASS_TYPE_SCALAR
     SLANG_CLASS_TYPE_VECTOR
     SLANG_CLASS_TYPE_PTR
     SLANG_CLASS_TYPE_MMT
#v-
  The \var{SLANG_CLASS_TYPE_SCALAR} indicates that the new data type
  is a scalar.  Examples of scalars in \var{SLANG_INT_TYPE} and
  \var{SLANG_DOUBLE_TYPE}.
  
  Setting \var{class_type} to SLANG_CLASS_TYPE_VECTOR implies that the
  new data type is a vector, or a 1-d array of scalar types.  An
  example of a data type of this class is the
  \var{SLANG_COMPLEX_TYPE}, which represents complex numbers.
  
  \var{SLANG_CLASS_TYPE_PTR} specifies the data type is of a pointer
  type.  Examples of data types of this class include
  \var{SLANG_STRING_TYPE} and \var{SLANG_ARRAY_TYPE}.  Such types must
  provide for their own memory management.
  
  Data types of class \var{SLANG_CLASS_TYPE_MMT} are pointer types
  except that the memory management, i.e., creation and destruction of
  the type, is handled by the interpreter.  Such a type is called a
  \em{memory managed type}.  An example of this data type is the
  \var{SLANG_FILEPTR_TYPE}.
\notes
   See the \slang-c-programmers-guide for more information.
\seealso{SLclass_allocate_class}
\done


\function{SLclass_set_string_function}
\synopsis{Set a data type's string representation callback}
\usage{int SLclass_set_string_function (cl, sfun)}
#v+
   SLang_Class_Type *cl
   char *(*sfun) (unsigned char, VOID_STAR);
#v-
\description
  The \var{SLclass_set_string_function} routine is used to define a
  callback function, \var{sfun}, that will be used when a string
  representation of an object of the data type represented by \var{cl}
  is needed.  \var{cl} must have already been obtained via a call to
  \var{SLclass_allocate_class}.  When called, \var{sfun} will be
  passed two arguments: a unsigned char which represents the data
  type, and the address of the object for which a string represetation
  is required.  The callback function must return a \em{malloced}
  string.
  
  Upon success, \var{SLclass_set_string_function} returns zero, or
  upon error it returns \-1.
\example
  A callback function that handles both \var{SLANG_STRING_TYPE} and
  \var{SLANG_INT_TYPE} variables looks like:
#v+
     char *string_and_int_callback (unsigned char type, VOID_STAR addr)
     {
        char buf[64];

	switch (type)
	  {
	     case SLANG_STRING_TYPE:
	       return SLmake_string (*(char **)addr);
	     
	     case SLANG_INTEGER_TYPE:
	       sprintf (buf, "%d", *(int *)addr);
	       return SLmake_string (buf);
          }
        return NULL;
     }
#v-
\notes
  The default string callback simply returns the name of the data type.
\seealso{SLclass_allocate_class, SLclass_register_class}
\done


\function{SLclass_set_destroy_function}
\synopsis{Set the destroy method callback for a data type}
\usage{int SLclass_set_destroy_function (cl, destroy_fun)}
#v+
    SLang_Class_Type *cl
    void (*destroy_fun) (unsigned char, VOID_STAR);
#v-
\description
  \var{SLclass_set_destroy_function} is used to set the destroy
  callback for a data type.  The data type's class \var{cl} must have
  been previously obtained via a call to \var{SLclass_allocate_class}.
  When called, \var{destroy_fun} will be passed two arguments: a
  unsigned char which represents the data type, and the address of the
  object to be destroyed.
  
  \var{SLclass_set_destroy_function} returns zero upon success, and
  \-1 upon failure.
\example
  The destroy method for \var{SLANG_STRING_TYPE} looks like:
#v+
    static void string_destroy (unsigned char type, VOID_STAR ptr)
    {
       char *s = *(char **) ptr;
       if (s != NULL) SLang_free_slstring (*(char **) s);
    }
#v-
\notes
  Data types of class SLANG_CLASS_TYPE_SCALAR do not require a destroy
  callback.  However, other classes do.
\seealso{SLclass_allocate_class, SLclass_register_class}
\done


\function{SLclass_set_push_function}
\synopsis{Set the push callback for a new data type}
\usage{int SLclass_set_push_function (cl, push_fun)}
#v+
    SLang_Class_Type *cl
    int (*push_fun) (unsigned char, VOID_STAR);
#v-
\description
   \var{SLclass_set_push_function} is used to set the push callback
   for a new data type specified by \var{cl}, which must have been
   previously obtained via \var{SLclass_allocate_class}.
   
   The parameter \var{push_fun} is a pointer to the push callback.  It
   is required to take two arguments: an unsigned character
   representing the data type, and the address of the object to be
   pushed.  It must return zero upon success, or \-1 upon failure.
   
   \var{SLclass_set_push_function} returns zero upon success, or \-1
   upon failure.
\example
   The push callback for \var{SLANG_COMPLEX_TYPE} looks like:
#v+
      static int complex_push (unsigned char type, VOID_STAR ptr)
      {
         double *z = *(double **) ptr;
         return SLang_push_complex (z[0], z[1]);
      }
#v-
\seealso{SLclass_allocate_class, SLclass_register_class}
\done


\function{SLclass_set_pop_function}
\synopsis{Set the pop callback for a new data type}
\usage{int SLclass_set_pop_function (cl, pop_fun)}
#v+
    SLang_Class_Type *cl
    int (*pop_fun) (unsigned char, VOID_STAR);
#v-
\description
   \var{SLclass_set_pop_function} is used to set the callback for
   popping an object from the stack for a new data type specified by
   \var{cl}, which must have been previously obtained via
   \var{SLclass_allocate_class}. 

   The parameter \var{pop_fun} is a pointer to the pop callback
   function, which is required to take two arguments: an unsigned
   character representing the data type, and the address of the object
   to be popped.  It must return zero upon success, or \-1 upon
   failure.

   \var{SLclass_set_pop_function} returns zero upon success, or \-1
   upon failure.
\example
   The pop callback for \var{SLANG_COMPLEX_TYPE} looks like:
#v+
      static int complex_push (unsigned char type, VOID_STAR ptr)
      {
         double *z = *(double **) ptr;
         return SLang_pop_complex (&z[0], &z[1]);
      }
#v-
\seealso{SLclass_allocate_class, SLclass_register_class}
\done


\function{SLclass_get_datatype_name}
\synopsis{Get the name of a data type}
\usage{char *SLclass_get_datatype_name (unsigned char type)}
\description
  The \var{SLclass_get_datatype_name} function returns the name of the
  data type specified by \var{type}.  For example, if \var{type} is
  \var{SLANG_INT_TYPE}, the string \exmp{"Integer_Type"} will be
  returned.
  
  This function returns a pointer that should not be modified or freed.
\seealso{SLclass_allocate_class, SLclass_register_class}
\done

\function{SLang_free_mmt}
\synopsis{Free a memory managed type}
\usage{void SLang_free_mmt (SLang_MMT_Type *mmt)}
\description
  The \var{SLang_MMT_Type} function is used to free a memory managed
  data type.
\seealso{SLang_object_from_mmt, SLang_create_mmt}
\done


\function{SLang_object_from_mmt}
\synopsis{Get a pointer to the value of a memory managed type}
\usage{VOID_STAR SLang_object_from_mmt (SLang_MMT_Type *mmt)}
\description
  The \var{SLang_object_from_mmt} function returns a pointer to the
  actual object whose memory is being managed by the interpreter.
\seealso{SLang_free_mmt, SLang_create_mmt}
\done


\function{SLang_create_mmt}
\synopsis{Create a memory managed data type}
\usage{SLang_MMT_Type *SLang_create_mmt (unsigned char t, VOID_STAR ptr)}
\description
  The \var{SLang_create_mmt} function returns a pointer to a new
  memory managed object.  This object contains information necessary
  to manage the memory associated with the pointer \var{ptr} which
  represents the application defined data type of type \var{t}.
\seealso{SLang_object_from_mmt, SLang_push_mmt, SLang_free_mmt}
\done


\function{SLang_push_mmt}
\synopsis{Push a memory managed type}
\usage{int SLang_push_mmt (SLang_MMT_Type *mmt)}
\description
   This function is used to push a memory managed type onto the
   interpreter stack.  It returns zero upon success, or \exmp{-1} upon
   failure.
\seealso{SLang_create_mmt, SLang_pop_mmt}
\done


\function{SLang_pop_mmt}
\synopsis{Pop a memory managed data type}
\usage{SLang_MMT_Type *SLang_pop_mmt (unsigned char t)}
\description
  The \var{SLang_pop_mmt} function may be used to pop a memory managed
  type of type \var{t} from the stack.  It returns a pointer to the
  memory managed object upon success, or \var{NULL} upon failure.  The
  function \var{SLang_object_from_mmt} should be used to access the
  actual pointer to the data type.
\seealso{SLang_object_from_mmt, SLang_push_mmt}
\done


\function{SLang_inc_mmt}
\synopsis{Increment a memory managed type reference count}
\usage{void SLang_inc_mmt (SLang_MMT_Type *mmt);}
\description
  The \var{SLang_inc_mmt} function may be used to increment the
  reference count associated with the memory managed data type given
  by \var{mmt}.
\seealso{SLang_free_mmt, SLang_create_mmt, SLang_pop_mmt, SLang_pop_mmt}
\done


\function{SLang_vmessage}
\synopsis{Display a message to the message device}
\usage{void SLang_vmessage (char *fmt, ...)}
\description
  This function prints a \var{printf} style formatted variable
  argument list to the message device.  The default message device is
  \var{stdout}.
\seealso{SLang_verror}
\done


\function{SLang_exit_error}
\synopsis{Exit the program and display an error message}
\usage{void SLang_exit_error (char *fmt, ...)}
\description
   The \var{SLang_exit_error} function terminates the program and
   displays an error message using a \var{printf} type variable
   argument list.  The default behavior to this function is to write
   the message to \var{stderr} and exit with the \var{exit} system
   call. 
   
   If the function pointer \var{SLang_Exit_Error_Hook} is
   non-NULL, the function to which it points will be called.  This
   permits an application to perform whatever cleanup is necessary.
   This hook has the prototype:
#v+
     void (*SLang_Exit_Error_Hook)(char *, va_list);
#v-
\seealso{SLang_verror, exit}
\done


\function{SLang_init_slang}
\synopsis{Initialize the interpreter}
\usage{int SLang_init_slang (void)}
\description
  The \var{SLang_init_slang} function must be called by all
  applications that use the \slang interpreter.  It initializes the
  interpreter, defines the built-in data types, and adds a set of core
  intrinsic functions.
  
  The function returns \var{0} upon success, or \var{-1} upon failure.
\seealso{SLang_init_slfile, SLang_init_slmath, SLang_init_slunix}
\done

\function{SLang_init_slfile}
\synopsis{Initialize the interpreter file I/O intrinsics}
\usage{int SLang_init_slfile (void)}
\description
  This function initializes the interpreters file I/O intrinsic
  functions.  This function adds intrinsic functions such as
  \var{fopen}, \var{fclose}, and \var{fputs} to the interpreter.
  It returns \exmp{0} if successful, or \exmp{-1} upon error.
\notes
  Before this function can be called, it is first necessary to call
  \var{SLang_init_slang}.  It also adds
  the preprocessor symbol \var{__SLFILE__} to the interpreter.
\seealso{SLang_init_slang, SLang_init_slunix, SLang_init_slmath}
\done


\function{SLang_init_slmath}
\synopsis{Initialize the interpreter math intrinsics}
\usage{int SLang_init_slmath (void)}
\description
  The \var{SLang_init_slmath} function initializes the interpreter's
  mathematical intrinsic functions and makes them available to the
  language.  The intrinsic functions include \var{sin}, \var{cos},
  \var{tan}, etc...  It returns \exmp{0} if successful, or \exmp{-1}
  upon failure.
\notes
  This function must be called after \var{SLang_init_slang}.  It adds
  the preprocessor symbol \var{__SLMATH__} to the interpreter.
\seealso{SLang_init_slang, SLang_init_slfile, SLang_init_slunix}
\done


\function{SLang_init_slunix}
\synopsis{Make available some unix system calls to the interpreter}
\usage{int SLang_init_slunix (void)}
\description
  The \var{SLang_init_slunix} function initializes the interpreter's
  unix system call intrinsic functions and makes them available to the
  language.  Examples of functions made available by
  \var{SLang_init_slunix} include \var{chmod}, \var{chown}, and
  \var{stat_file}.  It returns \exmp{0} if successful, or \exmp{-1}
  upon failure.
\notes
  This function must be called after \var{SLang_init_slang}.  It adds
  the preprocessor symbol \var{__SLUNIX__} to the interpreter.
\seealso{SLang_init_slang, SLang_init_slfile, SLang_init_slmath}
\done


\function{SLadd_intrin_fun_table}
\synopsis{Add a table of intrinsic functions to the interpreter}
\usage{int SLadd_intrin_fun_table(SLang_Intrin_Fun_Type *tbl, char *pp_name);}
\description
  The \var{SLadd_intrin_fun_table} function adds an array, or table, of
  \var{SLang_Intrin_Fun_Type} objects to the interpreter.  The first
  parameter, \var{tbl} specifies the table to be added.  The second
  parameter \var{pp_name}, if non-NULL will be added to the list of
  preprocessor symbols.
  
  This function returns \-1 upon failure or zero upon success.
\notes
  A table should only be loaded one time and it is considered to be an
  error on the part of the application if it loads a table more than
  once.
\seealso{SLadd_intrin_var_table, SLadd_intrinsic_function, SLdefine_for_ifdef}
\done

\function{SLadd_intrin_var_table}
\synopsis{Add a table of intrinsic variables to the interpreter}
\usage{int SLadd_intrin_var_table (SLang_Intrin_Var_Type *tbl, char *pp_name);}
\description
  The \var{SLadd_intrin_var_table} function adds an array, or table, of
  \var{SLang_Intrin_Var_Type} objects to the interpreter.  The first
  parameter, \var{tbl} specifies the table to be added.  The second
  parameter \var{pp_name}, if non-NULL will be added to the list of
  preprocessor symbols.
  
  This function returns \-1 upon failure or zero upon success.
\notes
  A table should only be loaded one time and it is considered to be an
  error on the part of the application if it loads a table more than
  once.
\seealso{SLadd_intrin_var_table, SLadd_intrinsic_function, SLdefine_for_ifdef}
\done


\function{SLang_load_file}
\synopsis{Load a file into the interpreter}
\usage{int SLang_load_file (char *fn)}
\description
  The \var{SLang_load_file} function opens the file whose name is
  specified by \var{fn} and feeds it to the interpreter, line by line,
  for execution.  If \var{fn} is \var{NULL}, the function will take
  input from \var{stdin}.

  If no error occurs, it returns \exmp{0}; otherwise,
  it returns \exmp{-1}, and sets \var{SLang_Error} accordingly.  For
  example, if it fails to open the file, it will return \exmp{-1} with
  \var{SLang_Error} set to \var{SL_OBJ_NOPEN}.
\notes
   If the hook \var{SLang_Load_File_Hook} declared as
#v+
      int (*SLang_Load_File_Hook)(char *);
#v-
   is non-NULL, the function point to by it will be used to load the
   file.  For example, the \jed editor uses this hook to load files
   via its own routines.
\seealso{SLang_load_object, SLang_load_string}
\done


\function{SLang_restart}
\synopsis{Reset the interpreter after an error}
\usage{void SLang_restart (int full)}
\description
   The \var{SLang_restart} function should be called by the
   application at top level if an error occurs.  If the parameter
   \var{full} is non-zero, any objects on the \slang run time stack
   will be removed from the stack; otherwise, the stack will be left
   intact.  Any time the stack is believed to be trashed, this routine
   should be called with a non-zero argument (e.g., if
   \var{setjmp}/\var{longjmp} is called).
   
   Calling \var{SLang_restart} does not reset the global variable
   \var{SLang_Error} to zero.  It is up to the application to reset
   that variable to zero after calling \var{SLang_restart}.
\example
#v+
      while (1)
        {
	   if (SLang_Error) 
	     {
	        SLang_restart (1);
		SLang_Error = 0;
	     }
	   (void) SLang_load_file (NULL);
	}
#v-
\seealso{SLang_init_slang, SLang_load_file}
\done


\function{SLang_byte_compile_file}
\synopsis{Byte-compile a file for faster loading}
\usage{int SLang_byte_compile_file(char *fn, int reserved)}
\description
  The \var{SLang_byte_compile_file} function ``byte-compiles'' the
  file \var{fn} for faster loading by the interpreter.  This produces
  a new file whose filename is equivalent to the one specified by
  \var{fn}, except that a \var{'c'} is appended to the name.  For
  example, if \var{fn} is set to \exmp{init.sl}, then the new file
  will have the name exmp{init.slc}.  The meaning of the second
  parameter, \var{reserved}, is reserved for future use.  For now, set
  it to \var{0}.
  
  The function returns zero upon success, or \exmp{-1} upon error and
  sets SLang_Error accordingly.
\seealso{SLang_load_file, SLang_init_slang}
\done


\function{SLang_autoload}
\synopsis{Autoload a function from a file}
\usage{int SLang_autoload(char *funct, char *filename)}
\description
  The \var{SLang_autoload} function may be used to associate a
  \var{slang} function name \var{funct} with the file \var{filename}
  such that if \var{funct} has not already been defined when needed,
  it will be loaded from \var{filename}.
  
  \var{SLang_autoload} has no effect if \var{funct} has already been
  defined.  Otherwise it declares \var{funct} as a user-defined \slang
  function.  It returns \exmp{0} upon success, or \exmp{-1} upon error.
\seealso{SLang_load_file, SLang_is_defined}
\done


\function{SLang_load_string}
\synopsis{Interpret a string}
\usage{int SLang_load_string(char *str)}
\description
  The \var{SLang_load_string} function feeds the string specified by
  \var{str} to the interpreter for execution.  It returns zero upon
  success, or \exmp{-1} upon failure.
\seealso{SLang_load_file, SLang_load_object}
\done


\function{SLdo_pop}
\synopsis{Delete an object from the stack}
\usage{int SLdo_pop(void)}
\description
   This function removes an object from the top of the interpeter's
   run-time stack and frees any memory associated with it.  It returns
   zero upon success, or \var{-1} upon error (most likely due to a
   stack-underflow).
\seealso{SLdo_pop_n, SLang_pop_integer, SLang_pop_string}
\done

\function{SLdo_pop_n}
\synopsis{Delete n objects from the stack}
\usage{int SLdo_pop_n (unsigned int n)}
\description
   The \var{SLdo_pop_n} function removes the top \var{n} objects from
   the interpreter's run-time stack and frees all memory associated
   with the objects.  It returns zero upon success, or \var{-1} upon
   error (most likely due to a stack-underflow).
\seealso{SLdo_pop, SLang_pop_integer, SLang_pop_string}
\done


\function{SLang_pop_integer}
\synopsis{Pop an integer off the stack}
\usage{int SLang_pop_integer (int *i)}
\description
   The \var{SLang_pop_integer} function removes an integer from the
   top of the interpreter's run-time stack and returns its value via
   the pointer \var{i}.  If successful, it returns zero.  However, if
   the top stack item is not of type \var{SLANG_INT_TYPE}, or the
   stack is empty, the function will return \exmp{-1} and set
   \var{SLang_Error} accordingly.
\seealso{SLang_push_integer, SLang_pop_double}
\done


\function{SLpop_string}
\synopsis{Pop a string from the stack}
\usage{int SLpop_string (char **strptr);}
\description
   The \var{SLpop_string} function pops a string from the stack and
   returns it as a malloced pointer.  It is up to the calling routine
   to free this string via a call to \var{free} or \var{SLfree}.  If
   successful, \var{SLpop_string} returns zero.  However, if the top
   stack item is not of type \var{SLANG_STRING_TYPE}, or the stack is
   empty, the function will return \exmp{-1} and set
   \var{SLang_Error} accordingly.
\example
#v+
      define print_string (void)
      {
         char *s;
	 if (-1 == SLpop_string (&s))
	   return;
	 fputs (s, stdout);
	 SLfree (s);
      }
#v-
\notes
   This function should not be confused with \var{SLang_pop_slstring},
   which pops a \em{hashed} string from the stack.
\seealso{SLang_pop_slstring. SLfree}
\done


\function{SLang_pop_string}
\synopsis{Pop a string from the stack}
\usage{int SLang_pop_string(char **strptr, int *do_free)}
\description
   The \var{SLpop_string} function pops a string from the stack and
   returns it as a malloced pointer via \var{strptr}.  After the
   function returns, the integer pointed to by the second parameter
   will be set to a non-zero value if \var{*strptr} should be freed via 
   \var{free} or \var{SLfree}.  If successful, \var{SLpop_string}
   returns zero.  However, if the top stack item is not of type
   \var{SLANG_STRING_TYPE}, or the stack is empty, the function will
   return \exmp{-1} and set \var{SLang_Error} accordingly.
\notes
   This function is considered obsolete and should not be used by
   applications.  If one requires a malloced string for modification,
   \var{SLpop_string} should be used.  If one requires a constant
   string that will not be modifed by the application,
   \var{SLang_pop_slstring} should be used.
\seealso{SLang_pop_slstring, SLpop_string}
\done


\function{SLang_pop_slstring}
\synopsis{Pop a hashed string from the stack}
\usage{int SLang_pop_slstring (char **s_ptr)}
\description
   The \var{SLang_pop_slstring} function pops a hashed string from the
   \slang run-time stack and returns it via \var{s_ptr}.  It returns
   zero if successful, or \-1 upon failure.  The resulting string
   should be freed via a call to \var{SLang_free_slstring} after use.
\example
#v+
   void print_string (void)
   {
      char *s;
      if (-1 == SLang_pop_slstring (&s))
        return;
      fprintf (stdout, "%s\n", s);
      SLang_free_slstring (s);
   }
#v-
\notes
   \var{SLang_free_slstring} is the preferred function for popping
   strings.  This is a result of the fact that the interpreter uses
   hashed strings as the native representation for string data.
   
   One must \em{never} free a hashed string using \var{free} or
   \var{SLfree}.  In addition, one must never make any attempt to
   modify a hashed string and doing so will result in memory
   corruption.
\seealso{SLang_free_slstring, SLpop_string}
\done

\function{SLang_pop_double}
\synopsis{Pop a double from the stack}
\usage{int SLang_pop_double (double *dptr, int *iptr, int *conv)}
\description
   The \var{SLang_pop_double} function pops a double precision number
   from the stack and returns it via \var{dptr}.  If the number was
   derived from an integer, \var{*conv} will be set to \exmp{1} upon
   return, otherwise, \var{*conv} will be set to \exmp{0}.  This
   function returns \0 upon success, otherwise it returns \-1 and sets
   \var{SLang_Error} accordingly.
\notes
   If one does not care whether or not \exmp{*dptr} was derived from
   an integer, \var{iptr} and \var{conv} may be passed as \var{NULL}
   pointers.
\seealso{SLang_pop_integer, SLang_push_double}
\done


\function{SLang_pop_complex}
\synopsis{Pop a complex number from the stack}
\usage{int SLang_pop_complex (double *re, double *im)}
\description
   \var{SLang_pop_complex} pops a complex number from the stack and
   returns it via the parameters \var{re} and \var{im} as the real and
   imaginary parts of the complex number, respectively.  This function
   automatically converts objects of type \var{SLANG_DOUBLE_TYPE} and
   \var{SLANG_INT_TYPE} to \var{SLANG_COMPLEX_TYPE}, if necessary.
   It returns zero upon success, or \-1 upon error setting
   \var{SLang_Error} accordingly.
\seealso{SLang_pop_integer, SLang_pop_double, SLang_push_complex}
\done


\function{SLang_push_complex}
\synopsis{Push a complex number onto the stack}
\usage{int SLang_push_complex (double re, double im)}
\description
   \var{SLang_push_complex} may be used to push the complex number
   whose real and imaginary parts are given by \var{re} and \var{im},
   respectively.  It returns zero upon success, or \-1 upon error
   setting \var{SLang_Error} accordingly.
\seealso{SLang_pop_complex, SLang_push_double}
\done


\function{SLang_push_double}
\synopsis{Push a double onto the stack}
\usage{int SLang_push_double(double d)}
\description
   \var{SLang_push_double} may be used to push the double precision
   floating point number \var{d} onto the interpreter's run-time
   stack.  It returns zero upon success, or \-1 upon error setting
   \var{SLang_Error} accordingly.
\seealso{SLang_pop_double, SLang_push_integer}
\done


\function{SLang_push_string}
\synopsis{Push a string onto the stack}
\usage{int SLang_push_string (char *s)}
\description
   \var{SLang_push_string} pushes a copy of the string specified by
   \var{s} onto the interpreter's run-time stack.  It returns zero
   upon success, or \-1 upon error setting \var{SLang_Error}
   accordingly.
\notes
   If \var{s} is \var{NULL}, this function pushes \var{NULL}
   (\var{SLANG_NULL_TYPE}) onto the stack.
\seealso{SLang_push_malloced_string}
\done

\function{SLang_push_integer}
\synopsis{Push an integer onto the stack}
\usage{int SLang_push_integer (int i)}
\description
   \var{SLang_push_integer} the integer \var{i} onto the interpreter's
   run-time stack.  It returns zero upon success, or \-1 upon error
   setting \var{SLang_Error} accordingly.
\seealso{SLang_pop_integer, SLang_push_double, SLang_push_string}
\done

\function{SLang_push_malloced_string}
\synopsis{Push a malloced string onto the stack}
\usage{int SLang_push_malloced_string (char *s);}
\description
   \var{SLang_push_malloced_string} may be used to push a malloced
   string onto the interpreter's run-time stack.  It returns zero upon
   success, or \-1 upon error setting \var{SLang_Error} accordingly.
\example
   The following example illustrates that it is up to the calling
   routine to free the string if \var{SLang_push_malloced_string} fails:
#v+
      int push_hello (void)
      {
         char *s = malloc (6);
	 if (s == NULL) return -1;
	 strcpy (s, "hello");
	 if (-1 == SLang_push_malloced_string (s))
	   {
	      free (s);
	      return -1;
	   }
	 return 0;
      }
#v-
\example
   The function \var{SLang_create_slstring} returns a hashed string.
   Such a string may not be malloced and should not be passed to
   \var{SLang_push_malloced_string}.
\notes
   If \var{s} is \var{NULL}, this function pushes \var{NULL}
   (\var{SLANG_NULL_TYPE}) onto the stack.
\seealso{SLang_push_string, SLmake_string}
\done


\function{SLang_is_defined}
\synopsis{Check to see if the interpreter defines an object}
\usage{int SLang_is_defined (char *nm)}
\description
   The \var{SLang_is_defined} function may be used to determine
   whether or not a variable or function whose name is given by
   \var{em} has been defined.  It returns zero if no such object has
   been defined.  Othewise it returns a non-zero value whose meaning
   is given by the following table:
#v+
      1    intrinsic function  (SLANG_INTRINSIC)
      2    user-defined slang function (SLANG_FUNCTION)
     -1    intrinsic variable (SLANG_IVARIABLE)
     -2    user-defined global variable (SLANG_GVARIABLE)
#v-
\seealso{SLadd_intrinsic_function, SLang_run_hooks, SLang_execute_function}
\done


\function{SLang_run_hooks}
\synopsis{Run a user-defined hook with arguments}
\usage{int SLang_run_hooks (char *fname, unsigned int n, ...)}
\description
   The \var{SLang_run_hooks} function may be used to execute a
   user-defined function named \var{fname}.  Before execution of the
   function, the \var{n} string arguments specified by the variable
   parameter list are pushed onto the stack.  If the function
   \var{fname} does not exist, \var{SLang_run_hooks} returns zero;
   otherwise, it returns \exmp{1} upon successful execution of the
   function, or \-1 if an error occurred.
\example
   The \jed editor uses \var{SLang_run_hooks} to setup the mode of a
   buffer based on the filename extension of the file associated with
   the buffer:
#v+
      char *ext = get_filename_extension (filename);
      if (ext == NULL) return -1;
      if (-1 == SLang_run_hooks ("mode_hook", 1, ext))
        return -1;
      return 0;
#v-
\seealso{SLang_is_defined, SLang_execute_function}
\done


\function{SLang_execute_function}
\synopsis{Execute a user or intrinsic function}
\usage{int SLang_execute_function (char *fname)}
\description
   This function may be used to execute either a user-defined function
   or an intrinisic function.  The name of the function is specified
   by \var{fname}.  It returns zero if \var{fname} is not defined, or
   \exmp{1} if the function was successfully executed, or \-1 upon
   error.
\notes
   The function \var{SLexecute_function} may be a better alternative
   for some uses.
\seealso{SLang_run_hooks, SLexecute_function, SLang_is_defined}
\done

\function{SLang_verror}
\synopsis{Signal an error with a message}
\usage{void SLang_verror (int code, char *fmt, ...);}
\description
   The \var{SLang_verror} function sets \var{SLang_Error} to
   \var{code} if \var{SLang_Error} is 0.  It also displays the error
   message implied by the \var{printf} variable argument list using
   \var{fmt} as the format.
\example
#v+
      FILE *open_file (char *file)
      {
         char *file = "my_file.dat";
	 if (NULL == (fp = fopen (file, "w")))
	   SLang_verror (SL_INTRINSIC_ERROR, "Unable to open %s", file);
	 return fp;
      }
#v-
\seealso{SLang_vmessage, SLang_exit_error}
\done


\function{SLang_doerror}
\synopsis{Signal an error}
\usage{void SLang_doerror (char *err_str)}
\description
  The \var{SLang_doerror} function displays the string \var{err_str}
  to the error device and signals a \slang error.
\notes
  \var{SLang_doerror} is considered to obsolete.  Applications should
  use the \var{SLang_verror} function instead.
\seealso{SLang_verror, SLang_exit_error}
\done


\function{SLang_get_function}
\synopsis{Get a pointer to a \slang function}
\usage{SLang_Name_Type *SLang_get_function (char *fname)}
\description
  This function returns a pointer to the internal \slang table entry
  of a function whose name is given by \var{fname}.  It returns
  \var{NULL} upon failure.  The value returned by this function can be
  used \var{SLexecute_function} to call the function directly
  from C.
\seealso{SLexecute_function}
\done



\function{SLexecute_function}
\synopsis{Execute a \slang or intrinsic function}
\usage{int SLexecute_function (SLang_Name_Type *nt)}
\description
  The \var{SLexecute_function} allows an application to call the
  \slang function specified by the \var{SLang_Name_Type} pointer
  \var{nt}.  This parameter must be non \var{NULL} and must have been
   previously obtained by a call to \var{SLang_get_function}.
\example
   Consider the \slang function:
#v+
     define my_fun (x)
     {
        return x^2 - 2;
     }
#v-
   Suppose that it is desired to call this function many times with
   different values of x.  There are at least two ways to do this.
   The easiest way is to use \var{SLang_execute_function} by passing
   the string \exmp{"my_fun"}.  A better way that is much faster is to
   use \var{SLexecute_function}:
#v+
      int sum_a_function (char *fname, double *result)
      {
         double sum, x, y;
	 SLang_Name_Type *nt;

	 if (NULL == (nt = SLang_get_function (fname)))
	   return -1;
	 
	 sum = 0;
	 for (x = 0; x < 10.0; x += 0.1)
	   {
	      SLang_start_arg_list ();
	      if (-1 == SLang_push_double (x))
	        return -1;
	      SLang_end_arg_list ();
	      if (-1 == SLexecute_function (nt))
	        return -1;
	      if (-1 == SLang_pop_double (&y, NULL, NULL))
	        return -1;
	      
	      sum += y;
	   }
	 return sum;
      }
#v-
   Although not necessary in this case, \var{SLang_start_arg_list} and
   \var{SLang_end_arg_list} were used to provide the function with
   information about the number of parameters passed to it.
\seealso{SLang_get_function, SLang_start_arg_list, SLang_end_arg_list}
\done


\function{SLang_peek_at_stack}
\synopsis{Find the type of object on the top of the stack}
\usage{int SLang_peek_at_stack (void)}
\description
  The \var{SLang_peek_at_stack} function is useful for determining the
  data type of the object at the top of the stack.  It returns the
  data type, or -1 upon a stack-underflow error.  It does not remove
  anything from the stack.
\seealso{SLang_pop_string, SLang_pop_integer}
\done


\function{SLmake_string}
\synopsis{Duplicate a string}
\usage{char *SLmake_string (char *s)}
\description
  The \var{SLmake_string} function creates a new copy of the string
  \var{s}, via \var{malloc}, and returns it.  Upon failure it returns
  \var{NULL}.  Since the resulting string is malloced, it should be
  freed when nolonger needed via a call to either \var{free} or
  \var{SLfree}.
\notes
  \var{SLmake_string} should not be confused with the function
  \var{SLang_create_slstring}, which performs a similar function.
\seealso{SLmake_nstring, SLfree, SLmalloc, SLang_create_slstring}
\done


\function{SLmake_nstring}
\synopsis{Duplicate a substring}
\usage{char *SLmake_nstring (char *s, unsigned int n)}
\description
  This function is like \var{SLmake_nstring} except that it creates a
  null terminated string formed from the first \var{n} characters of
  \var{s}.  Upon failure, it returns \var{NULL}, otherwise it returns
  the new string.  When nolonger needed, the returned string should be
  freed with either \var{free} or \var{SLfree}.
\seealso{SLmake_nstring, SLfree, SLang_create_nslstring}
\done


\function{SLang_create_nslstring}
\synopsis{Created a hashed substring}
\usage{char *SLang_create_nslstring (char *s, unsigned int n)}
\description
  \var{SLang_create_nslstring} is like \var{SLang_create_slstring}
  except that only the first \var{n} characters of \var{s} are used to
  perform the string.  Upon error, it returns \var{NULL}, otherwise it
  returns the hashed substring.  Such a string must be freed by the
  function \var{SLang_free_slstring}.
\notes
  Do not use \var{free} or \var{SLfree} to free the string returned by
  \var{SLang_create_slstring} or \var{SLang_create_nslstring}.  Also
  it is important that no attempt is made to modify the hashed string
  returned by either of these functions.  If one needs to modify a
  string, the functions \var{SLmake_string} or \var{SLmake_nstring}
  should be used instead.
\seealso{SLang_free_slstring, SLang_create_slstring, SLmake_nstring}
\done

\function{SLang_create_slstring}
\synopsis{Create a hashed string}
\usage{char *SLang_create_slstring (char *s)}
\description
  The \var{SLang_create_slstring} creates a copy of \var{s} and
  returns it as a hashed string.  Upon error, the function returns
  \var{NULL}, otherwise it returns the hashed string.  Such a string
  must only be freed via the \var{SLang_free_slstring} function.
\notes
  Do not use \var{free} or \var{SLfree} to free the string returned by
  \var{SLang_create_slstring} or \var{SLang_create_nslstring}.  Also
  it is important that no attempt is made to modify the hashed string
  returned by either of these functions.  If one needs to modify a
  string, the functions \var{SLmake_string} or \var{SLmake_nstring}
  should be used instead.
\seealso{SLang_free_slstring, SLang_create_nslstring, SLmake_string}
\done


\function{SLang_free_slstring}
\synopsis{Free a hashed string}
\usage{void SLang_free_slstring (char *s)}
\description
  The \var{SLang_free_slstring} function is used to free a hashed
  string such as one returned by \var{SLang_create_slstring},
  \var{SLang_create_nslstring}, or \var{SLang_create_static_slstring}.
  If \var{s} is \var{NULL}, the routine does nothing.
\seealso{SLang_create_slstring, SLang_create_nslstring, SLang_create_static_slstring}
\done


\function{SLang_concat_slstrings}
\synopsis{Concatenate two strings to produce a hashed string}
\usage{char *SLang_concat_slstrings (char *a, char *b)}
\description
  The \var{SLang_concat_slstrings} function concatenates two strings,
  \var{a} and \var{b}, and returns the result as a hashed string.
  Upon failure, \var{NULL} is returned.
\notes
  A hashed string can only be freed using \var{SLang_free_slstring}.
  Never use either \var{free} or \var{SLfree} to free a hashed string,
  otherwise memory corruption will result.
\seealso{SLang_free_slstring, SLang_create_slstring}
\done

\function{SLang_create_static_slstring}
\synopsis{Create a hashed string}
\usage{char *SLang_create_static_slstring (char *s_literal)}
\description
  The \var{SLang_create_static_slstring} creates a hashed string from
  the string literal \var{s_literal} and returns the result.  Upon
  failure it returns \var{NULL}.
\example
#v+
     char *create_hello (void)
     {
        return SLang_create_static_slstring ("hello");
     }
#v-
\notes
  This function should only be used with string literals.
\seealso{SLang_create_slstring, SLang_create_nslstring}
\done


\function{SLmalloc}
\synopsis{Allocate some memory}
\usage{char *SLmalloc (unsigned int nbytes)}
\description
  This function uses \var{malloc} to allocate \var{nbytes} of memory.
  Upon error it returns \var{NULL}; otherwise it returns a pointer to
  the allocated memory.  One should use \var{SLfree} to free the
  memory after used.
\seealso{SLfree, SLrealloc, SLcalloc}
\done


\function{SLcalloc}
\synopsis{Allocate some memory}
\usage{char *SLcalloc (unsigned int num_elem, unsigned int elem_size)}
\description
  This function uses \var{calloc} to allocate memory for
  \var{num_elem} objects with each of size \var{elem_size} and returns
  the result.  In addition, the newly allocated memory is zeroed. 
  Upon error it returns \var{NULL}; otherwise it returns a pointer to
  the allocated memory.  One should use \var{SLfree} to free the
  memory after used.
\seealso{SLmalloc, SLrealloc, SLfree}
\done


\function{SLfree}
\synopsis{Free some allocated memory}
\usage{void SLfree (char *ptr)}
\description
  The \var{SLfree} function uses \var{free} to deallocate the memory
  specified by \var{ptr}, which may be \var{NULL} in which case the
  function does nothing.
\notes
  Never use this function to free a hashed string returned by one of
  the family of \var{slstring} functions, e.g.,
  \var{SLang_pop_slstring}.
\seealso{SLmalloc, SLcalloc, SLrealloc, SLmake_string}
\done


\function{SLrealloc}
\synopsis{Resize a dynamic memory block}
\usage{char *SLrealloc (char *ptr, unsigned int new_size)}
\description
  The \var{SLrealloc} uses the \var{realloc} function to resize the
  memory block specified by \var{ptr} to the new size \var{new_size}.
  If \var{ptr} is \var{NULL}, the function call is equivalent to
  \exmp{SLmalloc(new_size)}.  Similarly, if \var{new_size} is zero,
  the function call is equivalent to \var{SLfree(ptr)}.  

  If the function fails, or if \var{new_size} is zero, \var{NULL} is
  returned.  Otherwise a pointer is returned to the (possibly moved)
  new block of memory.
\seealso{SLfree, SLmalloc, SLcalloc}
\done


\function{SLcurrent_time_string}
\synopsis{Get the current time as a string}
\usage{char *SLcurrent_time_string (void)}
\description
  The \var{SLcurrent_time_string} function uses the C library function
  \var{ctime} to obtain a string representation of the
  current date and time in the form
#v+
     "Wed Dec 10 12:50:28 1997"
#v-
  However, unlike the \var{ctime} function, a newline character is not
  present in the string.  
  
  The returned value points to a statically allocated memory block
  which may get overwritten on subsequent function calls.
\seealso{SLmake_string}
\done



\function{SLatoi}
\synopsis{Convert a text string to an integer}
\usage{int SLatoi(unsigned char *str}
\description
  \var{SLatoi} parses the string \var{str} to interpret it as an
  integer value.  Unlike \var{atoi}, \var{SLatoi} can also parse
  strings containing integers expressed in
  hexidecimal (e.g., \exmp{"0x7F"}) and octal (e.g., \exmp{"012"}.)
  notation.
\seealso{SLang_guess_type}
\done


\function{SLang_pop_fileptr}
\synopsis{Pop a file pointer}
\usage{int SLang_pop_fileptr (SLang_MMT_Type **mmt, FILE **fp)}
\description
  \var{SLang_pop_fileptr} pops a file pointer from the \slang
  run-time stack.  It returns zero upon success, or \-1 upon failure.
  
  A \slang file pointer (SLANG_FILEPTR_TYPE) is actually a memory
  managed object.  For this reason, \var{SLang_pop_fileptr} also
  returns the memory managed object via the argument list.  It is up
  to the calling routine to call \var{SLang_free_mmt} to free the
  object.
\example
  The following example illustrates an application defined intrinsic
  function that writes a user defined double precision number to a
  file.  Note the use of \var{SLang_free_mmt}:
#v+
     int write_double (void)
     {
        double t;
	SLang_MMT_Type *mmt;
	FILE *fp;
	int status;

	if (-1 == SLang_pop_double (&d, NULL, NULL))
	  return -1;
	if (-1 == SLang_pop_fileptr (&mmt, &fp))
	  return -1;
	
	status = fwrite (&d, sizeof (double), 1, fp);
	SLang_free_mmt (mmt);
        return status;
     }
#v-
  This function can be used by a \slang function as follows:
#v+
     define write_some_values ()
     {
        variable fp, d;

	fp = fopen ("myfile.dat", "wb");
	if (fp == NULL)
	  error ("file failed to open");
	for (d = 0; d < 10.0; d += 0.1)
	  {
	     if (-1 == write_double (fp, d))
	       error ("write failed");
	  }
	if (-1 == fclose (fp))
	  error ("fclose failed");
     }
#v-
\seealso{SLang_free_mmt, SLang_pop_double}
\done


\function{SLadd_intrinsic_function}
\synopsis{Add a new intrinsic function to the interpreter}
\usage{int SLadd_intrinsic_function (name, f, type, nargs, ...)}
#v+
    char *name
    FVOID_STAR f
    unsigned char type
    unsigned int nargs
#v-
\description
  The \var{SLadd_intrinsic_function} function may be used to add a new
  intrinsic function.  The \slang name of the function is specified by
  \var{name} and the actual function pointer is given by \var{f}, cast
  to \var{FVOID_STAR}.  The third parameter, \var{type} specifies the
  return type of the function and must be one of the following values:
#v+
    SLANG_VOID_TYPE   (returns nothing)
    SLANG_INT_TYPE    (returns int)
    SLANG_DOUBLE_TYPE (returns double)
    SLANG_STRING_TYPE (returns char *)
#v-
  The \var{nargs} parameter specifies the number of parameters to pass
  to the function.  The variable argument list following \var{nargs}
  must consists of \var{nargs} integers which specify the data type of
  each argument.

  The function returns zero upon success or \-1 upon failure.
\example
  The \jed editor uses this function to change the \var{system}
  intrinsic function to the following:
#v+
     static int jed_system (char *cmd)
     {
        if (Jed_Secure_Mode)
	  {
	    msg_error ("Access denied.");
	    return -1;
	  }	  
	return SLsystem (cmd);
     }
#v-
  After initializing the interpreter with \var{SLang_init_slang},
  \jed calls \var{SLadd_intrinsic_function} to substitute the above
  definition for the default \slang definition:
#v+
     if (-1 == SLadd_intrinsic_function ("system", (FVOID_STAR)jed_system,
                                          SLANG_INT_TYPE, 1,
					  SLANG_STRING_TYPE))
       return -1;
#v-
\seealso{SLadd_intrinsic_variable, SLadd_intrinsic_array}
\done

\function{SLadd_intrinsic_variable}
\synopsis{Add an intrinsic variable to the interpreter}
\usage{int SLadd_intrinsic_variable (name, addr, type, rdonly)}
#v+
    char *name
    VOID_STAR type
    unsigned char type
    int rdonly
#v-
\description
  The \var{SLadd_intrinsic_variable} function adds an intrinsic
  variable called \var{name} to the interpeter.  The second parameter
  \var{addr} specifies the address of the variable (cast to
  \var{VOID_STAR}).  The third parameter, \var{type}, specifies the
  data type of the variable.  If the fourth parameter, \var{rdonly},
  is non-zero, the variable will interpreted by the interpreter as
  read-only.
  
  If successful, \var{SLadd_intrinsic_variable} returns zero,
  otherwise it returns \-1.
\example
  Suppose that \var{My_Global_Int} is a global variable (at least not
  a local one):
#v+
    int My_Global_Int;
#v-
  It can be added to the interpreter via the function call
#v+
    if (-1 == SLadd_intrinsic_variable ("MyGlobalInt",
                                        (VOID_STAR)&My_Global_Int,
					SLANG_INT_TYPE, 0))
      exit (1);
#v-
\notes
  The current implementation requires all pointer type intrinsic
  variables to be read-only.  For example,
#v+
    char *My_Global_String;
#v-
  is of type \var{SLANG_STRING_TYPE}, and must be declared as
  read-only.  Finally, not that
#v+
   char My_Global_Char_Buf[256];
#v-
  is \em{not} a \var{SLANG_STRING_TYPE} object.  This difference is
  very important because internally the interpreter dereferences the
  address passed to it to get to the value of the variable.
\seealso{SLadd_intrinsic_function, SLadd_intrinsic_array}
\done
}


\function{SLclass_add_unary_op}
\synopsis{??}
\usage{int SLclass_add_unary_op (unsigned char,int (*) (int, unsigned char, VOID_STAR, unsigned int, VOID_STAR), int (*) (int, unsigned char, unsigned char *));}
\description
??
\seealso{??}
\done


\function{SLclass_add_app_unary_op}
\synopsis{??}
\usage{int SLclass_add_app_unary_op (unsigned char, int (*) (int,unsigned char, VOID_STAR, unsigned int,VOID_STAR),int (*) (int, unsigned char, unsigned char *));}
\description
??
\seealso{??}
\done

\function{SLclass_add_binary_op}
\synopsis{??}
\usage{int SLclass_add_binary_op (unsigned char, unsigned char,int (*)(int, unsigned char, VOID_STAR, unsigned int,unsigned char, VOID_STAR, unsigned int,VOID_STAR),int (*) (int, unsigned char, unsigned char, unsigned char *));}
\description
??
\seealso{??}
\done

\function{SLclass_add_math_op}
\synopsis{??}
\usage{int SLclass_add_math_op (unsigned char,int (*)(int,unsigned char, VOID_STAR, unsigned int,VOID_STAR),int (*)(int, unsigned char, unsigned char *));}
\description
??
\seealso{??}
\done

\function{SLclass_add_typecast}
\synopsis{??}
\usage{int SLclass_add_typecast (unsigned char, unsigned char int (*)_PROTO((unsigned char, VOID_STAR, unsigned int,unsigned char, VOID_STAR)),int);}
\description
??
\seealso{??}
\done

\function{SLang_init_tty}
\synopsis{Initialize the terminal keyboard interface}
\usage{int SLang_init_tty (int intr_ch, int no_flow_ctrl, int opost)}
\description
  \var{SLang_init_tty} initializes the terminal for single character
  input.  If the first parameter \var{intr_ch} is in the range 0-255,
  it will be used as the interrupt character, e.g., under Unix this
  character will generate a \var{SIGINT} signal.  Otherwise, if it is
  \exmp{-1}, the interrupt character will be left unchanged.  

  If the second parameter \var{no_flow_ctrl} is non-zero, flow control
  (\var{XON}/\var{XOFF}) processing will be
  enabled.  
  
  If the last parmeter \var{opost} is non-zero, output processing by the
  terminal will be enabled.  If one intends to use this function in
  conjunction with the \slang screen management routines
  (\var{SLsmg}), this paramete shold be set to zero.
  
  \var{SLang_init_tty} returns zero upon success, or \-1 upon error.
\notes
  Terminal I/O is a complex subject.  The \slang interface presents a
  simplification that the author has found useful in practice.  For
  example, the only special character processing that
  \var{SLang_init_tty} enables is that of the \var{SIGINT} character,
  and the generation of other signals via the keyboard is disabled.
  However, generation of the job control signal \var{SIGTSTP} is possible
  via the \var{SLtty_set_suspend_state} function.
  
  Under Unix, the integer variable \var{SLang_TT_Read_FD} is used to
  specify the input descriptor for the terminal.  If
  \var{SLang_TT_Read_FD} represents a terminal device as determined
  via the \var{isatty} system call, then it will be used as the
  terminal file descriptor.  Otherwise, the terminal device
  \exmp{/dev/tty} will used as the input device.  The default value of
  \var{SLang_TT_Read_FD} is \-1 which causes \exmp{/dev/tty} to be
  used.  So, if you prefer to use \var{stdin} for input, then set
  \var{SLang_TT_Read_FD} to \exmp{fileno(stdin)} \em{before} calling
  \var{SLang_init_tty}.
  
  If the variable \var{SLang_TT_Baud_Rate} is zero when this function
  is called, the function will attempt to determine the baud rate by
  querying the terminal driver and set \var{SLang_TT_Baud_Rate} to
  that value.  
\seealso{SLang_reset_tty, SLang_getkey, SLtty_set_suspend_state}
\done


\function{SLang_reset_tty}
\synopsis{Reset the terminal}
\usage{void SLang_reset_tty (void)}
\description
  \var{SLang_reset_tty} resets the terminal interface back to the
  state it was in before \var{SLang_init_tty} was called.
\seealso{SLang_init_tty}
\done


\function{SLtty_set_suspend_state}
\synopsis{Enable or disable keyboard suspension}
\usage{void SLtty_set_suspend_state (int s)}
\description
  The \var{SLtty_set_suspend_state} function may be used to enable or
  disable keyboard generation of the \var{SIGTSTP} job control signal.
  If \var{s} is non-zero, generation of this signal via the terminal
  interface will be enabled, otherwise it will be disabled.
  
  This function should only be called after the terminal driver has be
  initialized via \var{SLang_init_tty}.  The \var{SLang_init_tty}
  always disables the generation of \var{SIGTSTP} via the keyboard.
\seealso{SLang_init_tty}
\done

\function{SLang_getkey}
\synopsis{Read a character from the keyboard}
\usage{unsigned int SLang_getkey (void);}
\description
  The \var{SLang_getkey} reads a single character from the terminal
  and returns it.  The terminal must first be initialized via a call
  to \var{SLang_init_tty} before this function can be called.  Upon
  success, \var{SLang_getkey} returns the character read from the
  terminal, otherwise it returns \var{SLANG_GETKEY_ERROR}.
\seealso{SLang_init_tty, SLang_input_pending, SLang_ungetkey}
\done

\function{SLang_ungetkey_string}
\synopsis{Unget a key string}
\usage{int SLang_ungetkey_string (unsigned char *buf, unsigned int n)}
\description
  The \var{SLang_ungetkey_string} function may be used to push the
  \var{n} characters pointed to by \var{buf} onto the buffered input
  stream that \var{SLgetkey} uses.  If there is not enough room for
  the characters, \-1 is returned and none are buffered.  Otherwise,
  it returns zero.
\notes
  The difference between \var{SLang_buffer_keystring} and
  \var{SLang_ungetkey_string} is that the \var{SLang_buffer_keystring}
  appends the characters to the end of the getkey buffer, whereas
  \var{SLang_ungetkey_string} inserts the characters at the beginning
  of the input buffer.
\seealso{SLang_ungetkey, SLang_getkey}
\done


\function{SLang_buffer_keystring}
\synopsis{Append a keystring to the input buffer}
\usage{int SLang_buffer_keystring (unsigned char *b, unsigned int len)}
\description
  \var{SLang_buffer_keystring} places the \var{len} characters
  specified by \var{b} at the \em{end} of the buffer that
  \var{SLang_getkey} uses.  Upon success it returns 0; otherwise, no
  characters are buffered and it returns \-1.
\notes
  The difference between \var{SLang_buffer_keystring} and
  \var{SLang_ungetkey_string} is that the \var{SLang_buffer_keystring}
  appends the characters to the end of the getkey buffer, whereas
  \var{SLang_ungetkey_string} inserts the characters at the beginning
  of the input buffer.
\seealso{SLang_getkey, SLang_ungetkey, SLang_ungetkey_string}
\done


\function{SLang_ungetkey}
\synopsis{Push a character back onto the input buffer}
\usage{int SLang_ungetkey (unsigned char ch)}
\description
  \var{SLang_ungetkey} pushes the character \var{ch} back onto the
  \var{SLgetkey} input stream.  Upon success, it returns zero,
  otherwise it returns \1.
\example
  This function is implemented as:
#v+
    int SLang_ungetkey (unsigned char ch)
    {
       return SLang_ungetkey_string(&ch, 1);
    }
#v-
\seealso{SLang_getkey, SLang_ungetkey_string}
\done


\function{SLang_flush_input}
\synopsis{Discard all keyboard input waiting to be read}
\usage{void SLang_flush_input (void)}
\description
  \var{SLang_flush_input} discards all input characters waiting to be
  read by the \var{SLang_getkey} function.
\seealso{SLang_getkey}
\done


\function{SLang_input_pending}
\synopsis{Check to see if input is pending}
\usage{int SLang_input_pending (int tsecs)}
\description
  \var{SLang_input_pending} may be used to see if an input character
  is available to be read without causing \var{SLang_getkey} to block.
  It will wait up to \var{tsecs} tenths of a second if no characters
  are immediately available for reading.  If \var{tsecs} is less than
  zero, then \var{SLang_input_pending} will wait \exmp{-tsecs}
  milliseconds for input, otherwise \var{tsecs} represents \var{1/10}
  of a second intervals.
\notes
  Not all systems support millisecond resolution.
\seealso{SLang_getkey}
\done


\function{SLang_set_abort_signal}
\synopsis{Set the signal to trap SIGINT}
\usage{void SLang_set_abort_signal (void (*f)(int));}
\description
  \var{SLang_set_abort_signal} sets the function that gets
  triggered when the user presses the interrupt key (\var{SIGINT}) to
  the function \var{f}.  If \var{f} is \var{NULL} the default handler
  will get installed.
\example
  The default interrupt handler on a Unix system is:
#v+
     static void default_sigint (int sig)
     {
        SLKeyBoard_Quit = 1;
	if (SLang_Ignore_User_Abort == 0) SLang_Error = SL_USER_BREAK;
	SLsignal_intr (SIGINT, default_sigint);
   }
#v-
\notes
  For Unix programmers, the name of this function may appear
  misleading since it is associated with \var{SIGINT} and not
  \var{SIGABRT}.  The origin of the name stems from the original intent
  of the function: to allow the user to abort the running of a \slang
  interpreter function.
\seealso{SLang_init_tty, SLsignal_intr}
\done


\function{SLkm_define_key}
\synopsis{Define a key in a keymap}
\usage{int SLkm_define_key (char *seq, FVOID_STAR f, SLKeyMap_List_Type *km)}
\description
  \var{SLkm_define_key} associates the key sequence \var{seq} with the
  function pointer \var{f} in the keymap specified by \var{km}.  Upon
  success, it returns zero, otherwise it returns a negative integer
  upon error.
\seealso{SLkm_define_keysym, SLang_define_key}
\done



\function{SLang_define_key}
\synopsis{Define a key in a keymap}
\usage{int SLang_define_key(char *seq, char *fun, SLKeyMap_List_Type *km)}
\description
  \var{SLang_define_key} associates the key sequence \var{seq} with
  the function whose name is \var{fun} in the keymap specified by
  \var{km}.
\seealso{SLkm_define_keysym, SLkm_define_key}
\done


\function{SLkm_define_keysym}
\synopsis{Define a keysym in a keymap}
\usage{int SLkm_define_keysym (seq, ks, km)}
#v+
      char *seq;
      unsigned int ks;
      SLKeyMap_List_Type *km;
#v-
\description
  \var{SLkm_define_keysym} associates the key sequence \var{seq} with
  the keysym \var{ks} in the keymap \var{km}.  Keysyms whose value is
  less than or equal to \exmp{0x1000} is reserved by the library and
  should not be used.
\seealso{SLkm_define_key, SLang_define_key}
\done

\function{SLang_undefine_key}
\synopsis{Undefined a key from a keymap}
\usage{void SLang_undefine_key(char *seq, SLKeyMap_List_Type *km);}
\description
  \var{SLang_undefine_key} removes the key sequence \var{seq} from the
  keymap \var{km}.
\seealso{SLang_define_key}
\done

\function{SLang_create_keymap}
\synopsis{Create a new keymap}
\usage{SLKeyMap_List_Type *SLang_create_keymap (name, km)}
#v+
     char *name;
     SLKeyMap_List_Type *km;
#v-
\description
  \var{SLang_create_keymap} creates a new keymap called \var{name} by
  copying the key definitions from the keymap \var{km}.  If \var{km}
  is \var{NULL}, the newly created keymap will be empty and it is up
  to the calling routine to initialize it via the
  \var{SLang_define_key} and \var{SLkm_define_keysym} functions.
  \var{SLang_create_keymap} returns a pointer to the new keymap, or
  \var{NULL} upon failure.
\seealso{SLang_define_key, SLkm_define_keysym}
\done


\function{SLang_do_key}
\synopsis{Read a keysequence and return its keymap entry}
\usage{SLang_Key_Type *SLang_do_key (kml, getkey)}
#v+
     SLKeyMap_List_Type *kml;
     int (*getkey)(void);
#v-
\description
  The \var{SLang_do_key} function reads characters using the function
  specified by the \var{getkey} function pointer and uses the
  key sequence to return the appropriate entry in the keymap specified
  by \var{kml}.
  
  \var{SLang_do_key} returns \var{NULL} if the key sequence is not
  defined by the keymap, otherwise it returns a pointer to an object
  of type \var{SLang_Key_Type}, which is defined in \exmp{slang.h} as
#v+
     #define SLANG_MAX_KEYMAP_KEY_SEQ 14
     typedef struct SLang_Key_Type
     {
       struct SLang_Key_Type *next;
       union
       {
          char *s;
	  FVOID_STAR f;
	  unsigned int keysym;
       }
       f;
       unsigned char type;	       /* type of function */
     #define SLKEY_F_INTERPRET  0x01
     #define SLKEY_F_INTRINSIC  0x02
     #define SLKEY_F_KEYSYM     0x03
       unsigned char str[SLANG_MAX_KEYMAP_KEY_SEQ + 1];/* key sequence */
     }
SLang_Key_Type;

#v-
  The \var{type} field specifies which field of the union \var{f}
  should be used.  If \var{type} is \var{SLKEY_F_INTERPRET}, then
  \var{f.s} is a string that should be passed to the interpreter for
  evaluation.  If \var{type} is \var{SLKEY_F_INTRINSIC}, then
  \var{f.f} refers to function that should be called.  Otherwise,
  \var{type} is \var{SLKEY_F_KEYSYM} and \var{f.keysym} represents the
  value of the keysym that is associated with the key sequence.
\seealso{SLkm_define_keysym, SLkm_define_key}
\done


\function{SLang_find_key_function}
\synopsis{Obtain a function pointer associated with a keymap}
\usage{FVOID_STAR SLang_find_key_function (fname, km);}
#v+
    char *fname;
    SLKeyMap_List_Type *km;
#v-
\description
  The \var{SLang_find_key_function} routine searches through the
  \var{SLKeymap_Function_Type} list of functions associated with the
  keymap \var{km} for the function with name \var{fname}.
  If a matching function is found, a pointer to the function will
  be returned, otherwise \var{SLang_find_key_function} will return
  \var{NULL}.
\seealso{SLang_create_keymap, SLang_find_keymap}
\done


\function{SLang_find_keymap}
\synopsis{Find a keymap}
\usage{SLKeyMap_List_Type *SLang_find_keymap (char *keymap_name);}
\description
  The \var{SLang_find_keymap} function searches through the list of
  keymaps looking for one whose name is \var{keymap_name}.  If a
  matching keymap is found, the function returns a pointer to the
  keymap.  It returns \var{NULL} if no such keymap exists.
\seealso{SLang_create_keymap, SLang_find_key_function}
\done

\function{SLang_process_keystring}
\synopsis{Un-escape a key-sequence}
\usage{char *SLang_process_keystring (char *kseq);}
\description
  The \var{SLang_process_keystring} function converts an escaped key
  sequence to its raw form by converting two-character combinations
  such as \var{^A} to the \em{single} character \exmp{Ctrl-A} (ASCII
  1).  In addition, if the key sequence contains constructs such as
  \exmp{^(XX)}, where \exmp{XX} represents a two-character termcap
  specifier, the termcap escape sequence will be looked up and
  substituted.   
  
  Upon success, \var{SLang_process_keystring} returns a raw
  key-sequence whose first character represents the total length of
  the key-sequence, including the length specifier itself.  It returns
  \var{NULL} upon failure.
\example
  Consider the following examples:
#v+
     SLang_process_keystring ("^X^C");
     SLang_process_keystring ("^[[A");
#v-
  The first example will return a pointer to a buffer of three characters
  whose ASCII values are given by \exmp{\{3,24,3\}}.  Similarly, the
  second example will return a pointer to the four characters
  \exmp{\{4,27,91,65\}}.  Finally, the result of
#v+
     SLang_process_keystring ("^[^(ku)");
#v-
  will depend upon the termcap/terminfo capability \exmp{"ku"}, which
  represents the escape sequence associated with the terminal's UP
  arrow key.  For an ANSI terminal whose UP arrow produces
  \exmp{"ESC [ A"}, the result will be \exmp{5,27,27,91,65}.
\notes
  \var{SLang_process_keystring} returns a pointer to a static area
  that will be overwritten on subsequent calls.
\seealso{SLang_define_key, SLang_make_keystring}
\done

\function{SLang_make_keystring}
\synopsis{Make a printable key sequence}
\usage{char *SLang_make_keystring (unsigned char *ks);}
\description
  The \var{SLang_make_keystring} function takes a raw key sequence
  \var{ks} and converts it to a printable form by converting
  characters such as ASCII 1 (ctrl-A) to \exmp{^A}.  That is, it
  performs the opposite function of \var{SLang_process_keystring}.
\notes
  This function returns a pointer to a static area that will be
  overwritten on the next call to \var{SLang_make_keystring}.
\seealso{SLang_process_keystring}
\done


\function{SLextract_list_element}
\synopsis{Extract a substring of a delimited string}
\usage{int SLextract_list_element (dlist, nth, delim, buf, buflen)}
#v+
    char *dlist;
    unsigned int nth;
    char delim;
    char *buf;
    unsigned int buflen;
#v-
\description
  \var{SLextract_list_element} may be used to obtain the \var{nth}
  element of a list of strings, \var{dlist}, that are delimited by the
  character \var{delim}.  The routine copies the \var{nth} element of
  \var{dlist} to the buffer \var{buf} whose size is \var{buflen}
  characters.  It returns zero upon success, or \-1 if \var{dlist}
  does not contain an \var{nth} element.
\example
  A delimited list of strings may be turned into an array of strings
  as follows.  For conciseness, all malloc error checking has been
  omitted.
#v+
    int list_to_array (char *list, char delim, char ***ap)
    {
       unsigned int nth;
       char **a;
       char buf[1024];

       /* Determine the size of the array */
       nth = 0;
       while (0 == SLextract_list_element (list, nth, delim, buf, sizeof(buf)))
         nth++;
       
       ap = (char **) SLmalloc ((nth + 1) * sizeof (char **));
       nth = 0;
       while (0 == SLextract_list_element (list, nth, delim, buf, sizeof(buf)))
         {
	     a[nth] = SLmake_string (buf);
	     nth++;
	 }
       a[nth] = NULL;
       *ap = a;
       return 0;
    }
#v-
\seealso{SLmalloc, SLmake_string}
\done


#%+
\function{SLprep_open_prep}
\synopsis{??}
\usage{int SLprep_open_prep (SLPreprocess_Type *);}
\description
??
\seealso{??}
\done


\function{SLprep_close_prep}
\synopsis{??}
\usage{void SLprep_close_prep (SLPreprocess_Type *);}
\description
??
\seealso{??}
\done


\function{SLprep_line_ok}
\synopsis{??}
\usage{int SLprep_line_ok (char *, SLPreprocess_Type *);}
\description
??
\seealso{??}
\done


\function{SLdefine_for_ifdef}
\synopsis{??}
\usage{int SLdefine_for_ifdef (char *);}
\description
??
\seealso{??}
\done

\function{SLang_Read_Line_Type * SLang_rline_save_line (SLang_RLine_Info_Type *);}
\synopsis{??}
\usage{SLang_Read_Line_Type * SLang_rline_save_line (SLang_RLine_Info_Type *);}
\description
??
\seealso{??}
\done


\function{int SLang_init_readline (SLang_RLine_Info_Type *);}
\synopsis{??}
\usage{int SLang_init_readline (SLang_RLine_Info_Type *);}
\description
??
\seealso{??}
\done


\function{int SLang_read_line (SLang_RLine_Info_Type *);}
\synopsis{??}
\usage{int SLang_read_line (SLang_RLine_Info_Type *);}
\description
??
\seealso{??}
\done


\function{int SLang_rline_insert (char *);}
\synopsis{??}
\usage{int SLang_rline_insert (char *);}
\description
??
\seealso{??}
\done


\function{void SLrline_redraw (SLang_RLine_Info_Type *);}
\synopsis{??}
\usage{void SLrline_redraw (SLang_RLine_Info_Type *);}
\description
??
\seealso{??}
\done


\function{int SLtt_flush_output (void);}
\synopsis{??}
\usage{int SLtt_flush_output (void);}
\description
??
\seealso{??}
\done


\function{void SLtt_set_scroll_region(int, int);}
\synopsis{??}
\usage{void SLtt_set_scroll_region(int, int);}
\description
??
\seealso{??}
\done


\function{void SLtt_reset_scroll_region(void);}
\synopsis{??}
\usage{void SLtt_reset_scroll_region(void);}
\description
??
\seealso{??}
\done


\function{void SLtt_reverse_video (int);}
\synopsis{??}
\usage{void SLtt_reverse_video (int);}
\description
??
\seealso{??}
\done


\function{void SLtt_bold_video (void);}
\synopsis{??}
\usage{void SLtt_bold_video (void);}
\description
??
\seealso{??}
\done


\function{void SLtt_begin_insert(void);}
\synopsis{??}
\usage{void SLtt_begin_insert(void);}
\description
??
\seealso{??}
\done


\function{void SLtt_end_insert(void);}
\synopsis{??}
\usage{void SLtt_end_insert(void);}
\description
??
\seealso{??}
\done


\function{void SLtt_del_eol(void);}
\synopsis{??}
\usage{void SLtt_del_eol(void);}
\description
??
\seealso{??}
\done


\function{void SLtt_goto_rc (int, int);}
\synopsis{??}
\usage{void SLtt_goto_rc (int, int);}
\description
??
\seealso{??}
\done


\function{void SLtt_delete_nlines(int);}
\synopsis{??}
\usage{void SLtt_delete_nlines(int);}
\description
??
\seealso{??}
\done


\function{void SLtt_delete_char(void);}
\synopsis{??}
\usage{void SLtt_delete_char(void);}
\description
??
\seealso{??}
\done


\function{void SLtt_erase_line(void);}
\synopsis{??}
\usage{void SLtt_erase_line(void);}
\description
??
\seealso{??}
\done


\function{void SLtt_normal_video(void);}
\synopsis{??}
\usage{void SLtt_normal_video(void);}
\description
??
\seealso{??}
\done


\function{void SLtt_cls(void);}
\synopsis{??}
\usage{void SLtt_cls(void);}
\description
??
\seealso{??}
\done


\function{void SLtt_beep(void);}
\synopsis{??}
\usage{void SLtt_beep(void);}
\description
??
\seealso{??}
\done


\function{void SLtt_reverse_index(int);}
\synopsis{??}
\usage{void SLtt_reverse_index(int);}
\description
??
\seealso{??}
\done


\function{void SLtt_smart_puts(unsigned short *, unsigned short *, int, int);}
\synopsis{??}
\usage{void SLtt_smart_puts(unsigned short *, unsigned short *, int, int);}
\description
??
\seealso{??}
\done


\function{void SLtt_write_string (char *);}
\synopsis{??}
\usage{void SLtt_write_string (char *);}
\description
??
\seealso{??}
\done


\function{void SLtt_putchar(char);}
\synopsis{??}
\usage{void SLtt_putchar(char);}
\description
??
\seealso{??}
\done


\function{int SLtt_init_video (void);}
\synopsis{??}
\usage{int SLtt_init_video (void);}
\description
??
\seealso{??}
\done


\function{int SLtt_reset_video (void);}
\synopsis{??}
\usage{int SLtt_reset_video (void);}
\description
??
\seealso{??}
\done


\function{void SLtt_get_terminfo(void);}
\synopsis{??}
\usage{void SLtt_get_terminfo(void);}
\description
??
\seealso{??}
\done


\function{void SLtt_get_screen_size (void);}
\synopsis{??}
\usage{void SLtt_get_screen_size (void);}
\description
??
\seealso{??}
\done


\function{int SLtt_set_cursor_visibility (int);}
\synopsis{??}
\usage{int SLtt_set_cursor_visibility (int);}
\description
??
\seealso{??}
\done



\function{int SLtt_initialize (char *);}
\synopsis{??}
\usage{int SLtt_initialize (char *);}
\description
??
\seealso{??}
\done


\function{void SLtt_enable_cursor_keys(void);}
\synopsis{??}
\usage{void SLtt_enable_cursor_keys(void);}
\description
??
\seealso{??}
\done


\function{void SLtt_set_term_vtxxx(int *);}
\synopsis{??}
\usage{void SLtt_set_term_vtxxx(int *);}
\description
??
\seealso{??}
\done


\function{void SLtt_set_color_esc (int, char *);}
\synopsis{??}
\usage{void SLtt_set_color_esc (int, char *);}
\description
??
\seealso{??}
\done


\function{void SLtt_wide_width(void);}
\synopsis{??}
\usage{void SLtt_wide_width(void);}
\description
??
\seealso{??}
\done


\function{void SLtt_narrow_width(void);}
\synopsis{??}
\usage{void SLtt_narrow_width(void);}
\description
??
\seealso{??}
\done


\function{int SLtt_set_mouse_mode (int, int);}
\synopsis{??}
\usage{int SLtt_set_mouse_mode (int, int);}
\description
??
\seealso{??}
\done


\function{void SLtt_set_alt_char_set (int);}
\synopsis{??}
\usage{void SLtt_set_alt_char_set (int);}
\description
??
\seealso{??}
\done


\function{int SLtt_write_to_status_line (char *, int);}
\synopsis{??}
\usage{int SLtt_write_to_status_line (char *, int);}
\description
??
\seealso{??}
\done


\function{void SLtt_disable_status_line (void);}
\synopsis{??}
\usage{void SLtt_disable_status_line (void);}
\description
??
\seealso{??}
\done


\function{char *SLtt_tgetstr (char *);}
\synopsis{??}
\usage{char *SLtt_tgetstr (char *);}
\description
??
\seealso{??}
\done


\function{int SLtt_tgetnum (char *);}
\synopsis{??}
\usage{int SLtt_tgetnum (char *);}
\description
??
\seealso{??}
\done


\function{int SLtt_tgetflag (char *);}
\synopsis{??}
\usage{int SLtt_tgetflag (char *);}
\description
??
\seealso{??}
\done


\function{char *SLtt_tigetent (char *);}
\synopsis{??}
\usage{char *SLtt_tigetent (char *);}
\description
??
\seealso{??}
\done


\function{char *SLtt_tigetstr (char *, char **);}
\synopsis{??}
\usage{char *SLtt_tigetstr (char *, char **);}
\description
??
\seealso{??}
\done


\function{int SLtt_tigetnum (char *, char **);}
\synopsis{??}
\usage{int SLtt_tigetnum (char *, char **);}
\description
??
\seealso{??}
\done


\function{SLtt_Char_Type SLtt_get_color_object (int);}
\synopsis{??}
\usage{SLtt_Char_Type SLtt_get_color_object (int);}
\description
??
\seealso{??}
\done


\function{void SLtt_set_color_object (int, SLtt_Char_Type);}
\synopsis{??}
\usage{void SLtt_set_color_object (int, SLtt_Char_Type);}
\description
??
\seealso{??}
\done


\function{void SLtt_set_color (int, char *, char *, char *);}
\synopsis{??}
\usage{void SLtt_set_color (int, char *, char *, char *);}
\description
??
\seealso{??}
\done


\function{void SLtt_set_mono (int, char *, SLtt_Char_Type);}
\synopsis{??}
\usage{void SLtt_set_mono (int, char *, SLtt_Char_Type);}
\description
??
\seealso{??}
\done


\function{void SLtt_add_color_attribute (int, SLtt_Char_Type);}
\synopsis{??}
\usage{void SLtt_add_color_attribute (int, SLtt_Char_Type);}
\description
??
\seealso{??}
\done


\function{void SLtt_set_color_fgbg (int, SLtt_Char_Type, SLtt_Char_Type);}
\synopsis{??}
\usage{void SLtt_set_color_fgbg (int, SLtt_Char_Type, SLtt_Char_Type);}
\description
??
\seealso{??}
\done



\function{int SLkp_define_keysym (char *, unsigned int);}
\synopsis{??}
\usage{int SLkp_define_keysym (char *, unsigned int);}
\description
??
\seealso{??}
\done



\function{int SLkp_init (void);}
\synopsis{??}
\usage{int SLkp_init (void);}
\description
??
\seealso{??}
\done


\function{int SLkp_getkey (void);}
\synopsis{??}
\usage{int SLkp_getkey (void);}
\description
??
\seealso{??}
\done



\function{int SLscroll_find_top (SLscroll_Window_Type *);}
\synopsis{??}
\usage{int SLscroll_find_top (SLscroll_Window_Type *);}
\description
??
\seealso{??}
\done


\function{int SLscroll_find_line_num (SLscroll_Window_Type *);}
\synopsis{??}
\usage{int SLscroll_find_line_num (SLscroll_Window_Type *);}
\description
??
\seealso{??}
\done


\function{unsigned int SLscroll_next_n (SLscroll_Window_Type *, unsigned int);}
\synopsis{??}
\usage{unsigned int SLscroll_next_n (SLscroll_Window_Type *, unsigned int);}
\description
??
\seealso{??}
\done


\function{unsigned int SLscroll_prev_n (SLscroll_Window_Type *, unsigned int);}
\synopsis{??}
\usage{unsigned int SLscroll_prev_n (SLscroll_Window_Type *, unsigned int);}
\description
??
\seealso{??}
\done


\function{int SLscroll_pageup (SLscroll_Window_Type *);}
\synopsis{??}
\usage{int SLscroll_pageup (SLscroll_Window_Type *);}
\description
??
\seealso{??}
\done


\function{int SLscroll_pagedown (SLscroll_Window_Type *);}
\synopsis{??}
\usage{int SLscroll_pagedown (SLscroll_Window_Type *);}
\description
??
\seealso{??}
\done



\function{SLSig_Fun_Type *SLsignal (int, SLSig_Fun_Type *);}
\synopsis{??}
\usage{SLSig_Fun_Type *SLsignal (int, SLSig_Fun_Type *);}
\description
??
\seealso{??}
\done


\function{SLSig_Fun_Type *SLsignal_intr (int, SLSig_Fun_Type *);}
\synopsis{??}
\usage{SLSig_Fun_Type *SLsignal_intr (int, SLSig_Fun_Type *);}
\description
??
\seealso{??}
\done


\function{int SLsig_block_signals (void);}
\synopsis{??}
\usage{int SLsig_block_signals (void);}
\description
??
\seealso{??}
\done


\function{int SLsig_unblock_signals (void);}
\synopsis{??}
\usage{int SLsig_unblock_signals (void);}
\description
??
\seealso{??}
\done


\function{int SLsystem (char *);}
\synopsis{??}
\usage{int SLsystem (char *);}
\description
??
\seealso{??}
\done


\function{void SLadd_at_handler (long *, char *);}
\synopsis{??}
\usage{void SLadd_at_handler (long *, char *);}
\description
??
\seealso{??}
\done



\function{void SLang_define_case(int *, int *);}
\synopsis{??}
\usage{void SLang_define_case(int *, int *);}
\description
??
\seealso{??}
\done


\function{void SLang_init_case_tables (void);}
\synopsis{??}
\usage{void SLang_init_case_tables (void);}
\description
??
\seealso{??}
\done



\function{unsigned char *SLang_regexp_match(unsigned char *, unsigned int, SLRegexp_Type *);}
\synopsis{??}
\usage{unsigned char *SLang_regexp_match(unsigned char *, unsigned int, SLRegexp_Type *);}
\description
??
\seealso{??}
\done



\function{int SLang_regexp_compile (SLRegexp_Type *);}
\synopsis{??}
\usage{int SLang_regexp_compile (SLRegexp_Type *);}
\description
??
\seealso{??}
\done


\function{char *SLregexp_quote_string (char *, char *, unsigned int);}
\synopsis{??}
\usage{char *SLregexp_quote_string (char *, char *, unsigned int);}
\description
??
\seealso{??}
\done


\function{int SLcmd_execute_string (char *, SLcmd_Cmd_Table_Type *);}
\synopsis{??}
\usage{int SLcmd_execute_string (char *, SLcmd_Cmd_Table_Type *);}
\description
??
\seealso{??}
\done



\function{int SLsearch_init (char *, int, int, SLsearch_Type *);}
\synopsis{??}
\usage{int SLsearch_init (char *, int, int, SLsearch_Type *);}
\description
??
/* This routine must first be called before any search can take place.
 * The second parameter specifies the direction of the search: greater than
 * zero for a forwrd search and less than zero for a backward search.  The
 * third parameter specifies whether the search is case sensitive or not.
 * The last parameter is a pointer to a structure that is filled by this
 * function and it is this structure that must be passed to SLsearch.
 */
\seealso{??}
\done


\function{unsigned char *SLsearch (unsigned char *, unsigned char *, SLsearch_Type *);}
\synopsis{??}
\usage{unsigned char *SLsearch (unsigned char *, unsigned char *, SLsearch_Type *);}
\description
??
/* To use this routine, you must first call 'SLsearch_init'.  Then the first
 * two parameters p1 and p2 serve to define the region over which the search
 * is to take place.  The third parameter is the structure that was previously
 * initialized by SLsearch_init.
 *
 * The routine returns a pointer to the match if found otherwise it returns
 * NULL.
 */
\seealso{??}
\done


\function{SLcomplex_abs}
\synopsis{Returns the norm of a complex number}
\usage{double SLcomplex_abs (double *z)}}
\description
  The \var{SLcomplex_abs} function returns the absolute value or the
  norm of the complex number given by \var{z}.
\seealso{SLcomplex_times}
\done


\function{double *SLcomplex_times (double *, double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_times (double *, double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_divide (double *, double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_divide (double *, double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_sin (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_sin (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_cos (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_cos (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_tan (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_tan (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_asin (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_asin (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_acos (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_acos (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_atan (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_atan (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_exp (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_exp (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_log (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_log (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_log10 (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_log10 (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_sqrt (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_sqrt (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_sinh (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_sinh (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_cosh (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_cosh (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_tanh (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_tanh (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_pow (double *, double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_pow (double *, double *, double *);}
\description
??
\seealso{??}
\done


\function{double SLmath_hypot (double x, double y);}
\synopsis{??}
\usage{double SLmath_hypot (double x, double y);}
\description
??
\seealso{??}
\done



extern double *SLcomplex_asinh (double *, double *);
\function{double *SLcomplex_acosh (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_acosh (double *, double *);}
\description
??
\seealso{??}
\done


\function{double *SLcomplex_atanh (double *, double *);}
\synopsis{??}
\usage{double *SLcomplex_atanh (double *, double *);}
\description
??
\seealso{??}
\done




\function{char *SLdebug_malloc (unsigned long);}
\synopsis{??}
\usage{char *SLdebug_malloc (unsigned long);}
\description
??
\seealso{??}
\done


\function{char *SLdebug_calloc (unsigned long, unsigned long);}
\synopsis{??}
\usage{char *SLdebug_calloc (unsigned long, unsigned long);}
\description
??
\seealso{??}
\done


\function{char *SLdebug_realloc (char *, unsigned long);}
\synopsis{??}
\usage{char *SLdebug_realloc (char *, unsigned long);}
\description
??
\seealso{??}
\done


\function{void SLdebug_free (char *);}
\synopsis{??}
\usage{void SLdebug_free (char *);}
\description
??
\seealso{??}
\done


\function{void SLmalloc_dump_statistics (void);}
\synopsis{??}
\usage{void SLmalloc_dump_statistics (void);}
\description
??
\seealso{??}
\done


\function{char *SLstrcpy(register char *, register char *);}
\synopsis{??}
\usage{char *SLstrcpy(register char *, register char *);}
\description
??
\seealso{??}
\done


\function{int SLstrcmp(register char *, register char *);}
\synopsis{??}
\usage{int SLstrcmp(register char *, register char *);}
\description
??
\seealso{??}
\done


\function{char *SLstrncpy(char *, register char *, register  int);}
\synopsis{??}
\usage{char *SLstrncpy(char *, register char *, register  int);}
\description
??
\seealso{??}
\done



\function{void SLmemset (char *, char, int);}
\synopsis{??}
\usage{void SLmemset (char *, char, int);}
\description
??
\seealso{??}
\done


extern int SLang_add_intrinsic_array (char *,   /* name */
				      unsigned char,   /* type */
				      int,   /* readonly */
				      VOID_STAR,   /* data */
				      unsigned int, ...);   /* num dims */



\function{void SLexpand_escaped_string (register char *, register char *, register char *);}
\synopsis{??}
\usage{void SLexpand_escaped_string (register char *, register char *, register char *);}
\description
??
\seealso{??}
\done



\function{void SLmake_lut (unsigned char *, unsigned char *, unsigned char);}
\synopsis{??}
\usage{void SLmake_lut (unsigned char *, unsigned char *, unsigned char);}
\description
??
\seealso{??}
\done



\function{int SLang_guess_type (char *);}
\synopsis{??}
\usage{int SLang_guess_type (char *);}
\description
??
\seealso{??}
\done

#%-

\end{\documentstyle}
