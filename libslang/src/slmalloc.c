/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */
#include "slinclud.h"

#ifdef SL_MALLOC_DEBUG
# undef SL_MALLOC_DEBUG
#endif

#include "slang.h"
#include "_slang.h"

#ifdef __alpha
# define Chunk 8
#else
# define Chunk 4
#endif

static long Total_Allocated;
static long Max_Single_Allocation;
static long Max_Allocated;
/* #define SLDEBUG_DOUT */

#ifdef SLDEBUG_DOUT
static FILE *dout;
#endif

void SLmalloc_dump_statistics (void)
{
#ifdef SLDEBUG_DOUT
   fflush (dout);
#endif
   fprintf (stderr, "Total Allocated: %ld\nHighest single allocation: %ld\nHighest Total Allocated:%ld\n",
	    Total_Allocated, Max_Single_Allocation, Max_Allocated);
}

static void register_at_exit_fun (void)
{
   static int is_registered = 0;
   if (is_registered)
     return;
   is_registered = 1;

#ifdef SLDEBUG_DOUT
   if (dout == NULL) dout = fopen ("malloc.out", "w");
#endif
   SLang_add_cleanup_function (SLmalloc_dump_statistics);
}

static void fixup (unsigned char *p, unsigned long n, char *what)
{
   register_at_exit_fun ();

   p += Chunk;
   *(p - 4)= (unsigned char) ((n >> 24) & 0xFF);
   *(p - 3) = (unsigned char) ((n >> 16) & 0xFF);
   *(p - 2) = (unsigned char) ((n >> 8) & 0xFF);
   *(p - 1) = (unsigned char) (n & 0xFF);
   *(p + (int) n) = 27;
   *(p + (int) (n + 1)) = 182;
   *(p + (int) (n + 2)) = 81;
   *(p + (int) (n + 3)) = 86;
   Total_Allocated += (long) n;
   if (Total_Allocated > Max_Allocated) Max_Allocated = Total_Allocated;
   if ((long) n > Max_Single_Allocation)
     Max_Single_Allocation = (long) n;

#ifdef SLDEBUG_DOUT
   fprintf (dout, "ALLOC: %s\t%p %ld\n", what, p, (long) n);
#else
   (void) what;
#endif
}

static void SLmalloc_doerror (char *buf)
{
   SLang_doerror (buf);
}

static int check_memory (unsigned char *p, char *what)
{
   char buf[128];
   unsigned long n;

   register_at_exit_fun ();

   n = ((unsigned long) *(p - 4)) << 24;
   n |= ((unsigned long) *(p - 3)) << 16;
   n |= ((unsigned long) *(p - 2)) << 8;
   n |= (unsigned long) *(p - 1);

   if (n == 0xFFFFFFFFUL)
     {
	sprintf (buf, "%s: %p: Already FREE! Abort NOW.", what, p - Chunk);
	SLmalloc_doerror (buf);
	return -1;
     }

   if ((*(p + (int) n) != 27)
       || (*(p + (int) (n + 1)) != 182)
       || (*(p + (int) (n + 2)) != 81)
       || (*(p + (int) (n + 3)) != 86))
     {
	sprintf (buf, "\007%s: %p: Memory corrupt! Abort NOW.", what, p);
	SLmalloc_doerror (buf);
	return -1;
     }

   *(p - 4) = *(p - 3) = *(p - 2) = *(p - 1) = 0xFF;

   Total_Allocated -= (long) n;
   if (Total_Allocated < 0)
     {
	sprintf (buf, "\007%s: %p\nFreed %ld, Allocated is: %ld!\n",
		 what, p, (long) n, Total_Allocated);
	SLang_doerror (buf);
     }
#ifdef SLDEBUG_DOUT
   fprintf (dout, "FREE: %s:\t%p %ld\n", what, p, (long) n);
#endif
   return 0;
}

void SLdebug_free (char *p)
{
   if (p == NULL) return;
   if (-1 == check_memory ((unsigned char *) p, "FREE")) return;

   SLFREE (p - Chunk);
}

char *SLdebug_malloc (unsigned long n)
{
   char *p;

   if ((p = (char *) SLMALLOC (n + 2 * Chunk)) == NULL) return NULL;

   fixup ((unsigned char *) p, n, "MALLOC");
   return p + Chunk;
}

char *SLdebug_realloc (char *p, unsigned long n)
{
   if (-1 == check_memory ((unsigned char *) p, "REALLOC")) return NULL;
   if ((p = (char *) SLREALLOC (p - Chunk, n + 2 * Chunk)) == NULL) return NULL;
   fixup ((unsigned char *) p, n, "REALLOC");
   return p + Chunk;
}

char *SLdebug_calloc (unsigned long n, unsigned long size)
{
   char *p;
   int m;

   /* This is tough -- hope this is a good assumption!! */
   if (size >= Chunk) m = 1; else m = Chunk;

   if ((p = (char *) SLCALLOC (n + m + m, size)) == NULL) return NULL;
   fixup ((unsigned char *) p, size * n, "CALLOC");
   return p + Chunk;
}

