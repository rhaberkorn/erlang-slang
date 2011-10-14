/* Copyright (c) 1998, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */
#include "slinclud.h"

static void print (char *symbol, int is_defined)
{
   if (is_defined)
     fprintf (stdout, "%s is defined\n", symbol);
   else
     fprintf (stdout, "%s is NOT defined\n", symbol);
}

int main (int argc, char **argv)
{
   print ("__MSDOS__",
#ifdef __MSDOS__
	  1
#else
	  0
#endif
	  );

   print ("IBMPC_SYSTEM",
#ifdef IBMPC_SYSTEM
	  1
#else
	  0
#endif
	  );

   print ("REAL_UNIX_SYSTEM",
#ifdef REAL_UNIX_SYSTEM
	  1
#else
	  0
#endif
	  );

   print ("__os2__",
#ifdef __os2__
	  1
#else
	  0
#endif
	  );

   print ("__WIN32__",
#ifdef __WIN32__
	  1
#else
	  0
#endif
	  );

   print ("__unix__",
#ifdef __unix__
	  1
#else
	  0
#endif
	  );

   print ("__GO32__",
#ifdef __GO32__
	  1
#else
	  0
#endif
	  );

   print ("__DJGPP__",
#ifdef __DJGPP__
	  1
#else
	  0
#endif
	  );

   print ("__MSDOS_16BIT__",
#ifdef __MSDOS_16BIT__
	  1
#else
	  0
#endif
	  );

   return 0;
}
