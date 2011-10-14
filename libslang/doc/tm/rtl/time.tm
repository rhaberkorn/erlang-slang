\function{_time}
\synopsis{Get the current time in seconds}
\usage{ULong_Type _time ()}
\description
  The \var{_time} function returns the number of elapsed seconds since
  00:00:00 GMT, January 1, 1970.  The \var{ctime} function may be used
  to convert this into a string representation.
\seealso{ctime, time, localtime, gmtime}
\done

\function{ctime}
\synopsis{Convert a calendar time to a string}
\usage{String_Type ctime(ULong_Type secs)}
\description
  This function returns a string representation of the time as given
  by \var{secs} seconds since 1970.
\seealso{time, _time, localtime, gmtime}
\done

\function{gmtime}
\synopsis{Break down a time in seconds to GMT timezone}
\usage{Struct_Type gmtime (Long_Type secs)}
\description
   The \var{gmtime} function is exactly like \var{localtime} except
   that the values in the structure it returns are with respect to GMT
   instead of the local timezone.  See the documentation for
   \var{localtime} for more information.
\notes
   On systems that do not support the \var{gmtime} C library function,
   this function is the same as \var{localtime}.
\seealso{localtime, _time}
\done

\function{localtime}
\synopsis{Break down a time in seconds to local timezone}
\usage{Struct_Type localtime (Long_Type secs)}
\description
   The \var{localtime} function takes a parameter \var{secs}
   representing the number of seconds since 00:00:00, January 1 1970
   UTC and returns a structure containing information about \var{secs}
   in the local timezone.  The structure contains the following
   \var{Int_Type} fields:

   \var{tm_sec} The number of seconds after the minute, normally
      in the range 0 to 59, but can be up to 61 to allow for
      leap seconds.

   \var{tm_min} The number of minutes after the hour, in the
      range 0 to 59.

   \var{tm_hour} The number of hours past midnight, in the range
      0 to 23.

   \var{tm_mday} The day of the month, in the range 1 to 31.

   \var{tm_mon} The number of months since January, in the range
      0 to 11.

   \var{tm_year} The number of years since 1900.

   \var{tm_wday} The number of days since Sunday, in the range 0
      to 6.

   \var{tm_yday} The number of days since January 1, in the
      range 0 to 365.

   \var{tm_isdst} A flag that indicates whether daylight saving
      time is in effect at the time described.  The value is
      positive if daylight saving time is in effect, zero if it
      is not, and negative if the information is not available.
\seealso{gmtime, _time, ctime}
\done

\function{tic}
\synopsis{Start timing}
\usage{void tic ()}
\description
  The \var{tic} function restarts the internal clock used for timing
  the execution of commands.  To get the elapsed time of the clock,
  use the \var{toc} function.
\seealso{toc, times}
\done

\function{time}
\synopsis{Return the current data and time as a string}
\usage{String_Type time ()}
\description
  This function returns the current time as a string of the form:
#v+
    Sun Apr 21 13:34:17 1996
#v-
\seealso{ctime, message, substr}
\done

\function{times}
\synopsis{Get process times}
\usage{Struct_Type times ()}
\description
  The \var{times} function returns a structure containing the
  following fields:
#v+
    tms_utime     (user time)
    tms_stime     (system time)
    tms_cutime    (user time of child processes)
    tms_cstime    (system time of child processes)
#v-
\notes
  Not all systems support this function.
\seealso{tic, toc, _times}
\done

\function{toc}
\synopsis{Get elapsed CPU time}
\usage{Double_Type toc ()}
\description
  The \var{toc} function returns the elapsed CPU time in seconds since
  the last call to \var{tic}.  The CPU time is the amount of time the
  CPU spent running the code of the current process.
\example
  The \var{tic} and \var{toc} functions are ideal for timing the
  execution of the interpreter:
#v+
     variable a = "hello", b = "world", c, n = 100000, t;
     
     tic ();  loop (n) c = a + b; t = toc ();
     vmessage ("a+b took %f seconds\n", t);
     tic ();  loop (n) c = strcat(a,b); t = toc ();
     vmessage ("strcat took %f seconds\n", t);
#v-
\notes
  This function may not be available on all systems.

  The implementation of this function is based upon the \var{times}
  system call.  The precision of the clock is system dependent.
\seealso{tic, times, _time}
\done

