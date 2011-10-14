/* Process specific system calls */
/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#ifdef HAVE_IO_H
# include <io.h>		       /* for chmod */
#endif

#ifdef HAVE_PROCESS_H
# include <process.h>			/* for getpid */
#endif

#if defined(__BORLANDC__)
# include <dos.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

#include <errno.h>

#include "slang.h"
#include "_slang.h"

#ifdef HAVE_KILL
static int kill_cmd (int *pid, int *sig)
{
   int ret;

   if (-1 == (ret = kill ((pid_t) *pid, *sig)))
     _SLerrno_errno = errno;
   return ret;
}
#endif

static int getpid_cmd (void)
{
   return getpid ();
}

#ifdef HAVE_GETPPID
static int getppid_cmd (void)
{
   return getppid ();
}
#endif

#ifdef HAVE_GETGID
static int getgid_cmd (void)
{
   return getgid ();
}
#endif

#ifdef HAVE_GETEGID
static int getegid_cmd (void)
{
   return getegid ();
}
#endif

#ifdef HAVE_GETEUID
static int geteuid_cmd (void)
{
   return geteuid ();
}
#endif

#ifdef HAVE_GETUID
static int getuid_cmd (void)
{
   return getuid ();
}
#endif

#ifdef HAVE_SETGID
static int setgid_cmd (int *gid)
{
   if (0 == setgid (*gid))
     return 0;
   _SLerrno_errno = errno;
   return -1;
}
#endif

#ifdef HAVE_SETPGID
static int setpgid_cmd (int *pid, int *pgid)
{
   if (0 == setpgid (*pid, *pgid))
     return 0;
   _SLerrno_errno = errno;
   return -1;
}
#endif

#ifdef HAVE_SETUID
static int setuid_cmd (int *uid)
{
   if (0 == setuid (*uid))
     return 0;
   _SLerrno_errno = errno;
   return -1;
}
#endif

static SLang_Intrin_Fun_Type Process_Name_Table[] =
{
   MAKE_INTRINSIC_0("getpid", getpid_cmd, SLANG_INT_TYPE),

#ifdef HAVE_GETPPID
   MAKE_INTRINSIC_0("getppid", getppid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETGID
   MAKE_INTRINSIC_0("getgid", getgid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETEGID
   MAKE_INTRINSIC_0("getegid", getegid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETEUID
   MAKE_INTRINSIC_0("geteuid", geteuid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_GETUID
   MAKE_INTRINSIC_0("getuid", getuid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_SETGID
   MAKE_INTRINSIC_I("setgid", setgid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_SETPGID
   MAKE_INTRINSIC_II("setpgid", setpgid_cmd, SLANG_INT_TYPE),
#endif
#ifdef HAVE_SETUID
   MAKE_INTRINSIC_I("setuid", setuid_cmd, SLANG_INT_TYPE),
#endif

#ifdef HAVE_KILL
   MAKE_INTRINSIC_II("kill", kill_cmd, SLANG_INT_TYPE),
#endif
   SLANG_END_INTRIN_FUN_TABLE
};

int SLang_init_posix_process (void)
{
   if ((-1 == SLadd_intrin_fun_table (Process_Name_Table, "__POSIX_PROCESS__"))
       || (-1 == _SLerrno_init ()))
     return -1;
   return 0;
}
