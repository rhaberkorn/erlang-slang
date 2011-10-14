#% -*- mode: tm; mode: fold; eval: .0 =TAB -*-
#%{{{Macros 

#i linuxdoc.tm

#d slang \bf{S-lang}
#d kw#1 \tt{$1}
#d exmp#1 \tt{$1}
#d var#1 \tt{$1}

#d ivar#1 \tt{$1}
#d ifun#1 \tt{$1}
#d cvar#1 \tt{$1}
#d cfun#1 \tt{$1}
#d svar#1 \tt{$1}
#d sfun#1 \tt{$1}

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
#d r#1 \ref{$1}{$1}
#d done </descrip><p>
#d -1 <tt>-1</tt>
#d 0 <tt>0</tt>
#d 1 <tt>1</tt>
#d 2 <tt>2</tt>
#d 3 <tt>3</tt>
#d 4 <tt>4</tt>
#d 5 <tt>5</tt>
#d 6 <tt>6</tt>
#d 7 <tt>7</tt>
#d 8 <tt>8</tt>
#d 9 <tt>9</tt>
#d NULL <tt>NULL</tt>
#d documentstyle book


#d user-manual \bf{A Guide to the S-Lang Language}


#%}}}

\linuxdoc
\begin{\documentstyle}

\title S-Lang Run-Time Library Reference: Version 1.4.0
\author John E. Davis, \tt{davis@space.mit.edu}
\date \__today__

\toc

\chapter{Array Functions}
#i rtl/array.tm

\chapter{Associative Array Functions}
#i rtl/assoc.tm

\chapter{Functions that Operate on Strings}
#i rtl/strops.tm

\chapter{Functions that Manipulate Structures}
#i rtl/struct.tm

\chapter{Informational Functions}
#i rtl/info.tm

\chapter{Mathematical Functions}
#i rtl/math.tm

\chapter{Message and Error Functions}
#i rtl/message.tm

\chapter{Time and Date Functions}
#i rtl/time.tm

\chapter{Data-Type Conversion Functions}
#i rtl/type.tm

\chapter{Stdio File I/O Functions}
#i rtl/stdio.tm

\chapter{Low-level POSIX I/O functions}
#i rtl/posio.tm

\chapter{Directory Functions}
#i rtl/dir.tm

\chapter{Functions that parse pathnames}
#i rtl/ospath.tm

\chapter{System Call Functions}
#i rtl/posix.tm

\chapter{Eval Functions}
#i rtl/eval.tm

\chapter{Module Functions}
#i rtl/import.tm

\chapter{Debugging Functions}
#i rtl/debug.tm

\chapter{Stack Functions}
#i rtl/stack.tm

\chapter{Miscellaneous Functions}
#i rtl/misc.tm

\end{\documentstyle}
