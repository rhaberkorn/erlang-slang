/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

/* These routines are simple and inefficient.  They were designed to work on
 * SunOS when using Electric Fence.
 */

#include "slang.h"
#include "_slang.h"
char *SLstrcpy(register char *aa, register char *b)
{
   char *a = aa;
   while ((*a++ = *b++) != 0);
   return aa;
}

int SLstrcmp(register char *a, register char *b)
{
   while (*a && (*a == *b))
     {
	a++;
	b++;
     }
   if (*a) return((unsigned char) *a - (unsigned char) *b);
   else if (*b) return ((unsigned char) *a - (unsigned char) *b);
   else return 0;
}

char *SLstrncpy(char *a, register char *b,register  int n)
{
   register char *aa = a;
   while ((n > 0) && *b)
     {
	*aa++ = *b++;
	n--;
     }
   while (n-- > 0) *aa++ = 0;
   return (a);
}
