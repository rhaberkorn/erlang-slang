/* -*- mode: C; mode: fold; -*- */
/* slnspace.c  --- Name Space implementation */
/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

static SLang_NameSpace_Type *Namespace_Tables;

static SLang_NameSpace_Type *find_name_table (char *name)
{
   SLang_NameSpace_Type *table_list;

   table_list = Namespace_Tables;
   while (table_list != NULL)
     {
	if (0 == strcmp (table_list->name, name))
	  break;
	table_list = table_list->next;
     }
   return table_list;
}

SLang_NameSpace_Type *_SLns_find_namespace (char *name)
{
   SLang_NameSpace_Type *table_list;

   table_list = Namespace_Tables;
   while (table_list != NULL)
     {
	if ((table_list->namespace_name != NULL)
	    && (0 == strcmp (table_list->namespace_name, name)))
	  break;
	table_list = table_list->next;
     }
   return table_list;
}

SLang_NameSpace_Type *_SLns_allocate_namespace (char *name, unsigned int size)
{
   SLang_NameSpace_Type *table_list;
   SLang_Name_Type **nt;
   static int num;
   char namebuf[64];

   if (name == NULL)
     {
	sprintf (namebuf, " *** internal ns <%d> *** ", num);
	name = namebuf;
	num++;
     }

   if (NULL != (table_list = find_name_table (name)))
     return table_list;

   if (NULL == (name = SLang_create_slstring (name)))
     return NULL;

   if (NULL == (table_list = (SLang_NameSpace_Type *)
		SLmalloc (sizeof (SLang_NameSpace_Type))))
     {
	SLang_free_slstring (name);
	return NULL;
     }
   
   if (NULL == (nt = (SLang_Name_Type **) SLmalloc (sizeof (SLang_Name_Type *) * size)))
     {
	SLang_free_slstring (name);
	SLfree ((char *)table_list);
	return NULL;
     }

   memset ((char *)nt, 0, size * sizeof (SLang_Name_Type *));
   memset ((char *) table_list, 0, sizeof (SLang_NameSpace_Type));

   table_list->name = name;
   table_list->table = nt;
   table_list->table_size = size;

   table_list->next = Namespace_Tables;
   Namespace_Tables = table_list;

   return table_list;
}

int _SLns_set_namespace_name (SLang_NameSpace_Type *t, char *name)
{
   SLang_NameSpace_Type *t1;
   
   t1 = _SLns_find_namespace (name);
   if (t == t1)
     return 0;			       /* already has this name */

   if (t1 == NULL)
     t1 = t;
   
   if ((t != t1) || (*name == 0))
     {
	SLang_verror (SL_INTRINSIC_ERROR, "Namespace \"%s\" already exists",
		      name);
	return -1;
     }

   if (t->namespace_name != NULL)
     {
	SLang_verror (SL_INTRINSIC_ERROR, "An attempt was made to redefine namespace from \"%s\" to \"%s\"\n",
		      t->namespace_name, name);
	return -1;
     }

   if (NULL == (name = SLang_create_slstring (name)))
     return -1;

   SLang_free_slstring (t->namespace_name);   /* NULL ok */
   t->namespace_name = name;
   
   return 0;
}

SLang_Array_Type *_SLnspace_apropos (SLang_NameSpace_Type *ns, char *pat, unsigned int what)
{
   SLang_Array_Type *at;
   unsigned int table_size;
   SLang_Name_Type *t, **table;
   int num_matches;
   unsigned int i;
   SLRegexp_Type rexp;
   unsigned char rbuf[512];
   unsigned int two;
   
   at = NULL;

   if ((ns == NULL)
       || ((table = ns->table) == NULL))
     return NULL;

   memset ((char *) &rexp, 0, sizeof (SLRegexp_Type));
   rexp.case_sensitive = 1;
   rexp.buf = rbuf;
   rexp.buf_len = sizeof (rbuf);
   rexp.pat = (unsigned char *)pat;

   if (0 != SLang_regexp_compile (&rexp))
     {
	SLang_verror (SL_INVALID_PARM, "Invalid regular expression: %s", pat);
	return NULL;
     }
   
   table_size = ns->table_size;

   two = 2;
   while (two != 0)
     {
	two--;
	
	num_matches = 0;
	for (i = 0; i < table_size; i++)
	  {
	     t = table[i];
	     while (t != NULL)
	       {
		  unsigned int flags;
		  char *name = t->name;

		  switch (t->name_type)
		    {
		     case SLANG_GVARIABLE:
		       flags = 8;
		       break;
		       
		     case SLANG_ICONSTANT:
		     case SLANG_DCONSTANT:
		     case SLANG_RVARIABLE:
		     case SLANG_IVARIABLE:
		       flags = 4;
		       break;

		     case SLANG_INTRINSIC:
		     case SLANG_MATH_UNARY:
		     case SLANG_APP_UNARY:
		       flags = 1;
		       break;
		       
		     case SLANG_FUNCTION:
		       flags = 2;
		       break;
		       
		     default:
		       flags = 0;
		       break;
		    }
		  
		  if ((flags & what)
		      && (NULL != SLang_regexp_match ((unsigned char *)name, strlen (name), &rexp)))
		    {
		       if (at != NULL)
			 {
			    if (-1 == SLang_set_array_element (at, &num_matches, (VOID_STAR)&name))
			      goto return_error;
			 }
		       num_matches++;
		    }
		  t = t->next;
	       }
	  }
	
	if (at == NULL)
	  {
	     at = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &num_matches, 1);
	     if (at == NULL)
	       goto return_error;
	  }
     }
   
   return at;
   
   return_error:
   SLang_free_array (at);
   return NULL;
}

SLang_NameSpace_Type *SLns_create_namespace (char *namespace_name)
{   
   SLang_NameSpace_Type *ns;

   if (namespace_name == NULL)
     namespace_name = "Global";

   ns = _SLns_find_namespace (namespace_name);
   if (ns != NULL)
     return ns;
   
   if (NULL == (ns = _SLns_allocate_namespace (NULL, SLSTATIC_HASH_TABLE_SIZE)))
     return NULL;

   if (-1 == _SLns_set_namespace_name (ns, namespace_name))
     {
	SLns_delete_namespace (ns);
	return NULL;
     }
   
   return ns;
}

void SLns_delete_namespace (SLang_NameSpace_Type *ns)
{
   (void) ns;
   /* V2.0 */
}

SLang_Array_Type *_SLns_list_namespaces (void)
{
   SLang_NameSpace_Type *table_list;
   SLang_Array_Type *at;
   int num, i;
   
   num = 0;
   table_list = Namespace_Tables;
   while (table_list != NULL)
     {
	if (table_list->namespace_name != NULL)
	  num++;
	table_list = table_list->next;
     }
   at = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &num, 1);
   if (at == NULL)
     return NULL;

   table_list = Namespace_Tables;
   i = 0;
   while ((table_list != NULL) 
	  && (i < num))
     {
	if (table_list->namespace_name != NULL)
	  {
	     char *name = table_list->namespace_name;
	     if (-1 == SLang_set_array_element (at, &i, (VOID_STAR)&name))
	       {
		  SLang_free_array (at);
		  return NULL;
	       }
	     i++;
	  }
	table_list = table_list->next;
     }
   return at;
}
