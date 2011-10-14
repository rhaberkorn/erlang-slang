/* time related system calls */
/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include <sys/types.h>
#include <time.h>

#if defined(__BORLANDC__)
# include <dos.h>
#endif
#if defined(__GO32__) || (defined(__WATCOMC__) && !defined(__QNX__))
# include <dos.h>
# include <bios.h>
#endif

#include <errno.h>

#include "slang.h"
#include "_slang.h"

#ifdef __WIN32__
#include <windows.h>
/* Sleep is defined badly in MSVC... */
# ifdef _MSC_VER
#  define sleep(n) _sleep((n)*1000)
# else
#  ifdef sleep
#   undef sleep
#  endif
#  define sleep(x) if(x)Sleep((x)*1000)
# endif
#endif


#if defined(IBMPC_SYSTEM)
/* For other system (Unix and VMS), _SLusleep is in sldisply.c */
int _SLusleep (unsigned long s)
{
   sleep (s/1000000L);
   s = s % 1000000L;

# if defined(__WIN32__)
   Sleep (s/1000);
#else
# if defined(__IBMC__)
   DosSleep(s/1000);
# else
#  if defined(_MSC_VER)
   _sleep (s/1000);
#  endif
# endif
#endif
   return 0;
}
#endif

#if defined(__IBMC__) && !defined(_AIX)
/* sleep is not a standard function in VA3. */
unsigned int sleep (unsigned int seconds)
{
   DosSleep(1000L * ((long)seconds));
   return 0;
}
#endif

static char *ctime_cmd (unsigned long *tt)
{
   char *t;

   t = ctime ((time_t *) tt);
   t[24] = 0;  /* knock off \n */
   return (t);
}

static void sleep_cmd (void)
{
   unsigned int secs;
#if SLANG_HAS_FLOAT
   unsigned long usecs;
   double x;

   if (-1 == SLang_pop_double (&x, NULL, NULL))
     return;

   if (x < 0.0) 
     x = 0.0;
   secs = (unsigned int) x;
   sleep (secs);
   x -= (double) secs;
   usecs = (unsigned long) (1e6 * x);
   if (usecs > 0) _SLusleep (usecs);
#else
   if (-1 == SLang_pop_uinteger (&secs))
     return;
   if (secs != 0) sleep (secs);
#endif
}

static unsigned long _time_cmd (void)
{
   return (unsigned long) time (NULL);
}

#if defined(__GO32__)
static char *djgpp_current_time (void) /*{{{*/
{
   union REGS rg;
   unsigned int year;
   unsigned char month, day, weekday, hour, minute, sec;
   char days[] = "SunMonTueWedThuFriSat";
   char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
   static char the_date[26];

   rg.h.ah = 0x2A;
#ifndef __WATCOMC__
   int86(0x21, &rg, &rg);
   year = rg.x.cx & 0xFFFF;
#else
   int386(0x21, &rg, &rg);
   year = rg.x.ecx & 0xFFFF;
#endif

   month = 3 * (rg.h.dh - 1);
   day = rg.h.dl;
   weekday = 3 * rg.h.al;

   rg.h.ah = 0x2C;

#ifndef __WATCOMC__
   int86(0x21, &rg, &rg);
#else
   int386(0x21, &rg, &rg);
#endif

   hour = rg.h.ch;
   minute = rg.h.cl;
   sec = rg.h.dh;

   /* we want this form: Thu Apr 14 15:43:39 1994\n  */
   sprintf(the_date, "%.3s %.3s%3d %02d:%02d:%02d %d\n",
	   days + weekday, months + month,
	   day, hour, minute, sec, year);
   return the_date;
}

/*}}}*/

#endif

char *SLcurrent_time_string (void) /*{{{*/
{
   char *the_time;
#ifndef __GO32__
   time_t myclock;

   myclock = time((time_t *) 0);
   the_time = (char *) ctime(&myclock);
#else
   the_time = djgpp_current_time ();
#endif
   /* returns the form Sun Sep 16 01:03:52 1985\n\0 */
   the_time[24] = '\0';
   return(the_time);
}

/*}}}*/

static SLang_CStruct_Field_Type TM_Struct [] =
{
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_sec, "tm_sec", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_min, "tm_min", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_hour, "tm_hour", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_mday, "tm_mday", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_mon, "tm_mon", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_year, "tm_year", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_wday, "tm_wday", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_yday, "tm_yday", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_isdst, "tm_isdst", 0),
   SLANG_END_CSTRUCT_TABLE
};

static int push_tm_struct (struct tm *tms)
{
   return SLang_push_cstruct ((VOID_STAR) tms, TM_Struct);
}

static void localtime_cmd (long *t)
{
   time_t tt = (time_t) *t;
   (void) push_tm_struct (localtime (&tt));
}
   
static void gmtime_cmd (long *t)
{
#ifdef HAVE_GMTIME
   time_t tt = (time_t) *t;
   (void) push_tm_struct (gmtime (&tt));
#else
   localtime_cmd (t);
#endif
}

#ifdef HAVE_TIMES

# ifdef HAVE_SYS_TIMES_H
#  include <sys/times.h>
# endif

#include <limits.h>

#ifdef CLK_TCK
# define SECS_PER_TICK (1.0/(double)CLK_TCK)
#else 
# ifdef CLOCKS_PER_SEC
#  define SECS_PER_TICK (1.0/(double)CLOCKS_PER_SEC)
# else
#  define SECS_PER_TICK (1.0/60.0)
# endif
#endif

typedef struct
{ 
   double tms_utime;
   double tms_stime;
   double tms_cutime;
   double tms_cstime;
}
TMS_Type;

static SLang_CStruct_Field_Type TMS_Struct [] =
{
   MAKE_CSTRUCT_FIELD(TMS_Type, tms_utime, "tms_utime", SLANG_DOUBLE_TYPE, 0),
   MAKE_CSTRUCT_FIELD(TMS_Type, tms_utime, "tms_stime", SLANG_DOUBLE_TYPE, 0),
   MAKE_CSTRUCT_FIELD(TMS_Type, tms_utime, "tms_cutime", SLANG_DOUBLE_TYPE, 0),
   MAKE_CSTRUCT_FIELD(TMS_Type, tms_utime, "tms_cstime", SLANG_DOUBLE_TYPE, 0),
   SLANG_END_CSTRUCT_TABLE
};

static void times_cmd (void)
{
   TMS_Type d;
   struct tms t;

   (void) times (&t);

   d.tms_utime = SECS_PER_TICK * (double)t.tms_utime; 
   d.tms_stime = SECS_PER_TICK * (double)t.tms_stime;
   d.tms_cutime = SECS_PER_TICK * (double)t.tms_cutime;
   d.tms_cstime = SECS_PER_TICK * (double)t.tms_cstime;
   (void) SLang_push_cstruct ((VOID_STAR)&d, TMS_Struct);
}

static struct tms Tic_TMS;

static void tic_cmd (void)
{
   (void) times (&Tic_TMS);
}

static double toc_cmd (void)
{
   struct tms t;
   double d;

   (void) times (&t);
   d = ((t.tms_utime - Tic_TMS.tms_utime)
	+ (t.tms_stime - Tic_TMS.tms_stime)) * SECS_PER_TICK;
   Tic_TMS = t;
   return d;
}

#endif				       /* HAVE_TIMES */


static SLang_Intrin_Fun_Type Time_Funs_Table [] =
{
   MAKE_INTRINSIC_1("ctime", ctime_cmd, SLANG_STRING_TYPE, SLANG_ULONG_TYPE),
   MAKE_INTRINSIC_0("sleep", sleep_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_time", _time_cmd, SLANG_ULONG_TYPE),
   MAKE_INTRINSIC_0("time", SLcurrent_time_string, SLANG_STRING_TYPE),
   MAKE_INTRINSIC_1("localtime", localtime_cmd, SLANG_VOID_TYPE, SLANG_LONG_TYPE),
   MAKE_INTRINSIC_1("gmtime", gmtime_cmd, SLANG_VOID_TYPE, SLANG_LONG_TYPE),

#ifdef HAVE_TIMES
   MAKE_INTRINSIC_0("times", times_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("tic", tic_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("toc", toc_cmd, SLANG_DOUBLE_TYPE),
#endif
   SLANG_END_INTRIN_FUN_TABLE
};

int _SLang_init_sltime (void)
{
#ifdef HAVE_TIMES
   (void) tic_cmd ();
#endif
   return SLadd_intrin_fun_table (Time_Funs_Table, NULL);
}

