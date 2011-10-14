/* These functions are provided for backward compatibility and are obsolete.
 * Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

/* Compatibility */
int SLang_init_slunix (void)
{
   if ((-1 == SLang_init_posix_dir ())
       || (-1 == SLang_init_posix_process ())
       || (-1 == SLdefine_for_ifdef ("__SLUNIX__")))
     return -1;

   return 0;
}

int SLang_init_slfile (void)
{
   if ((-1 == SLang_init_stdio ())
       || (-1 == SLang_init_posix_dir ())
       || (-1 == SLdefine_for_ifdef("__SLFILE__")))
     return -1;

   return 0;
}

