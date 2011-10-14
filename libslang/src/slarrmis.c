/* Misc Array Functions */
/* Copyright (c) 1997, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

int SLang_get_array_element (SLang_Array_Type *at, int *indices, VOID_STAR data)
{
   int is_ptr;

   if ((at == NULL)
       || (indices == NULL)
       || (data == NULL))
     return -1;

   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);
   if (is_ptr) *(VOID_STAR *) data = NULL;
   return _SLarray_aget_transfer_elem (at, indices, data, at->sizeof_type, is_ptr);
}

int SLang_set_array_element (SLang_Array_Type *at, int *indices, VOID_STAR data)
{
   if ((at == NULL)
       || (indices == NULL)
       || (data == NULL))
     return -1;
   
   return _SLarray_aput_transfer_elem (at, indices, data, at->sizeof_type,
				       at->flags & SLARR_DATA_VALUE_IS_POINTER);
}

