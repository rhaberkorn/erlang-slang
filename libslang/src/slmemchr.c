/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

/* These routines are fast memcpy, memset routines.  When available, I
   use system rouines.  For msdos, I use inline assembly. */

/* The current versions only work in the forward direction only!! */

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

char *SLmemchr(register char *p, register char c, register int n)
{
   int n2;
   register char *pmax;

   pmax = p + (n - 32);

   while (p <= pmax)
     {
	if ((*p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c)
	    || (*++p == c) || (*++p == c) || (*++p == c) || (*++p == c))
	  return p;
	p++;
     }

   n2 = n % 32;

   while (n2--)
     {
	if (*p == c) return p;
	p++;
     }
   return(NULL);
}
