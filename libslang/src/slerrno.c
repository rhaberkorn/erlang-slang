/* The point of this file is to handle errno values in a system independent
 * way so that they may be used in slang scripts.
 */
/* Copyright (c) 1998, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */
#include "slinclud.h"

#include <errno.h>
#include "slang.h"
#include "_slang.h"

typedef SLCONST struct
{
   char *msg;
   int sys_errno;
   char *symbolic_name;
}
Errno_Map_Type;

static Errno_Map_Type Errno_Map [] =
{
#ifndef EPERM
# define EPERM	-1
#endif
     {"Not owner",			EPERM,	"EPERM"},
#ifndef ENOENT
# define ENOENT	-1
#endif
     {"No such file or directory",	ENOENT,	"ENOENT"},
#ifndef ESRCH
# define ESRCH	-1
#endif
     {"No such process",		ESRCH,	"ESRCH"},
#ifndef EINTR
# define EINTR	-1
#endif
     {"Interrupted system call",	EINTR,	"EINTR"},
#ifndef EIO
# define EIO	-1
#endif
     {"I/O error",			EIO,	"EIO"},
#ifndef ENXIO
# define ENXIO	-1
#endif
     {"No such device or address",	ENXIO,	"ENXIO"},
#ifndef E2BIG
# define E2BIG	-1
#endif
     {"Arg list too long",		E2BIG,	"E2BIG"},
#ifndef ENOEXEC
# define ENOEXEC	-1
#endif
     {"Exec format error",		ENOEXEC,"ENOEXEC"},
#ifndef EBADF
# define EBADF	-1
#endif
     {"Bad file number",		EBADF,	"EBADF"},
#ifndef ECHILD
# define ECHILD	-1
#endif
     {"No children",			ECHILD,	"ECHILD"},
#ifndef EAGAIN
# define EAGAIN	-1
#endif
     {"Try again",			EAGAIN,	"EAGAIN"},
#ifndef ENOMEM
# define ENOMEM	-1
#endif
     {"Not enough core",		ENOMEM,	"ENOMEM"},
#ifndef EACCES
# define EACCES	-1
#endif
     {"Permission denied",		EACCES,	"EACCES"},
#ifndef EFAULT
# define EFAULT	-1
#endif
     {"Bad address",			EFAULT,	"EFAULT"},
#ifndef ENOTBLK
# define ENOTBLK	-1
#endif
     {"Block device required",		ENOTBLK,	"ENOTBLK"},
#ifndef EBUSY
# define EBUSY	-1
#endif
     {"Mount device busy",		EBUSY,	"EBUSY"},
#ifndef EEXIST
# define EEXIST	-1
#endif
     {"File exists",			EEXIST,	"EEXIST"},
#ifndef EXDEV
# define EXDEV	-1
#endif
     {"Cross-device link",		EXDEV,	"EXDEV"},
#ifndef ENODEV
# define ENODEV	-1
#endif
     {"No such device",			ENODEV,	"ENODEV"},
#ifndef ENOTDIR
# define ENOTDIR	-1
#endif
     {"Not a directory",		ENOTDIR,	"ENOTDIR"},
#ifndef EISDIR
# define EISDIR	-1
#endif
     {"Is a directory",			EISDIR,	"EISDIR"},
#ifndef EINVAL
# define EINVAL	-1
#endif
     {"Invalid argument",		EINVAL,	"EINVAL"},
#ifndef ENFILE
# define ENFILE	-1
#endif
     {"File table overflow",		ENFILE,	"ENFILE"},
#ifndef EMFILE
# define EMFILE	-1
#endif
     {"Too many open files",		EMFILE,	"EMFILE"},
#ifndef ENOTTY
# define ENOTTY	-1
#endif
     {"Not a typewriter",		ENOTTY,	"ENOTTY"},
#ifndef ETXTBSY
# define ETXTBSY	-1
#endif
     {"Text file busy",			ETXTBSY,	"ETXTBSY"},
#ifndef EFBIG
# define EFBIG	-1
#endif
     {"File too large",			EFBIG,	"EFBIG"},
#ifndef ENOSPC
# define ENOSPC	-1
#endif
     {"No space left on device",	ENOSPC,	"ENOSPC"},
#ifndef ESPIPE
# define ESPIPE	-1
#endif
     {"Illegal seek",			ESPIPE,	"ESPIPE"},
#ifndef EROFS
# define EROFS	-1
#endif
     {"Read-only file system",		EROFS,	"EROFS"},
#ifndef EMLINK
# define EMLINK	-1
#endif
     {"Too many links",			EMLINK,	"EMLINK"},
#ifndef EPIPE
# define EPIPE	-1
#endif
     {"Broken pipe",			EPIPE,	"EPIPE"},
#ifndef ELOOP
# define ELOOP	-1
#endif
     {"Too many levels of symbolic links",ELOOP,	"ELOOP"},
#ifndef ENAMETOOLONG
# define ENAMETOOLONG	-1
#endif
     {"File name too long",		ENAMETOOLONG,	"ENAMETOOLONG"},

     {NULL, 0, NULL}
};

int _SLerrno_errno;

int SLerrno_set_errno (int sys_errno)
{
   _SLerrno_errno = sys_errno;
   return 0;
}

char *SLerrno_strerror (int sys_errno)
{
   Errno_Map_Type *e;

   e = Errno_Map;
   while (e->msg != NULL)
     {
	if (e->sys_errno == sys_errno)
	  return e->msg;

	e++;
     }

   if (sys_errno == SL_ERRNO_NOT_IMPLEMENTED)
     return "System call not available for this platform";

   return "Unknown error";
}

static char *intrin_errno_string (int *sys_errno)
{
   return SLerrno_strerror (*sys_errno);
}

int _SLerrno_init (void)
{
   static Errno_Map_Type *e;

   if (e != NULL)		       /* already initialized */
     return 0;

   if ((-1 == SLadd_intrinsic_function ("errno_string", (FVOID_STAR) intrin_errno_string,
				       SLANG_STRING_TYPE, 1, SLANG_INT_TYPE))
       || (-1 == SLadd_intrinsic_variable ("errno", (VOID_STAR)&_SLerrno_errno, SLANG_INT_TYPE, 1)))
     return -1;

   e = Errno_Map;
   while (e->msg != NULL)
     {
	if (-1 == SLadd_intrinsic_variable (e->symbolic_name, (VOID_STAR) &e->sys_errno, SLANG_INT_TYPE, 1))
	  return -1;
	e++;
     }

   return 0;
}
