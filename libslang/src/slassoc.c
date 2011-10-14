/* Copyright (c) 1998, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */
#include "slinclud.h"

/* #define SL_APP_WANTS_FOREACH */
#include "slang.h"
#include "_slang.h"

#define USE_NEW_ANYTYPE_CODE 1

typedef struct _SLAssoc_Array_Element_Type
{
   char *key;		       /* slstring */
   struct _SLAssoc_Array_Element_Type *next;
   SLang_Object_Type value;
}
_SLAssoc_Array_Element_Type;

typedef struct
{
   _SLAssoc_Array_Element_Type *elements[SLASSOC_HASH_TABLE_SIZE];
   SLang_Object_Type default_value;
   unsigned int num_elements;
#define HAS_DEFAULT_VALUE	1
   unsigned int flags;
   unsigned char type;
#if _SLANG_OPTIMIZE_FOR_SPEED
   int is_scalar_type;
#endif
}
SLang_Assoc_Array_Type;

#define USE_CACHED_STRING	1

#if USE_CACHED_STRING 
static char *Cached_String;
static SLang_Object_Type *Cached_Obj;
static SLang_Assoc_Array_Type *Cached_Array;
#endif

static SLang_Assoc_Array_Type *alloc_assoc_array (unsigned char type, int has_default_value)
{
   SLang_Assoc_Array_Type *a;

   a = (SLang_Assoc_Array_Type *)SLmalloc (sizeof (SLang_Assoc_Array_Type));
   if (a == NULL)
     {
	if (has_default_value)
	  SLdo_pop_n (1);
	return NULL;
     }

   memset ((char *) a, 0, sizeof (SLang_Assoc_Array_Type));
   a->type = type;
#if _SLANG_OPTIMIZE_FOR_SPEED
   a->is_scalar_type = (SLANG_CLASS_TYPE_SCALAR == _SLang_get_class_type (type));
#endif

   if (has_default_value)
     {
	if (
#if USE_NEW_ANYTYPE_CODE
	    ((type != SLANG_ANY_TYPE) && (-1 == SLclass_typecast (type, 1, 1)))
#else
	    (-1 == SLclass_typecast (type, 1, 1))
#endif
	    || (-1 == SLang_pop (&a->default_value)))
	  {
	     SLfree ((char *) a);
	     return NULL;
	  }

	a->flags |= HAS_DEFAULT_VALUE;
     }
   return a;
}

static void free_element (_SLAssoc_Array_Element_Type *e)
{
   if (e == NULL)
     return;

   SLang_free_object (&e->value);
   SLang_free_slstring (e->key);
#if USE_CACHED_STRING
   if (e->key == Cached_String)
     Cached_String = NULL;
#endif
   SLfree ((char *)e);
}

static void delete_assoc_array (SLang_Assoc_Array_Type *a)
{
   unsigned int i;

   if (a == NULL) return;

   for (i = 0; i < SLASSOC_HASH_TABLE_SIZE; i++)
     {
	_SLAssoc_Array_Element_Type *e;

	e = a->elements[i];
	while (e != NULL)
	  {
	     _SLAssoc_Array_Element_Type *next_e;

	     next_e = e->next;
	     free_element (e);
	     e = next_e;
	  }
     }
   if (a->flags & HAS_DEFAULT_VALUE)
     SLang_free_object (&a->default_value);

   SLfree ((char *) a);
}

_INLINE_
static SLang_Object_Type *
find_element (SLang_Assoc_Array_Type *a, char *str, unsigned long hash)
{
   unsigned int h;
   _SLAssoc_Array_Element_Type *e;

   h = (unsigned int) (hash % SLASSOC_HASH_TABLE_SIZE);
   e = a->elements[h];

   while (e != NULL)
     {
	if (str == e->key)       /* slstrings can be compared this way */
	  {
#if USE_CACHED_STRING
	     Cached_String = str;
	     Cached_Obj = &e->value;
	     Cached_Array = a;
#endif
	     return &e->value;
	  }

	e = e->next;
     }

   return NULL;
}

static _SLAssoc_Array_Element_Type *
create_element (SLang_Assoc_Array_Type *a, char *str, unsigned long hash)
{
   unsigned int h;
   _SLAssoc_Array_Element_Type *e;

   e = (_SLAssoc_Array_Element_Type *) SLmalloc (sizeof (_SLAssoc_Array_Element_Type));
   if (e == NULL)
     return NULL;

   memset ((char *) e, 0, sizeof (_SLAssoc_Array_Element_Type));
   h = (unsigned int) (hash % SLASSOC_HASH_TABLE_SIZE);

   if (NULL == (str = _SLstring_dup_hashed_string (str, hash)))
     {
	SLfree ((char *) e);
	return NULL;
     }

   e->key = str;
   e->next = a->elements[h];
   a->elements[h] = e;

   a->num_elements += 1;
#if USE_CACHED_STRING
   Cached_String = str;
   Cached_Obj = &e->value;
   Cached_Array = a;
#endif
   return e;
}

static int store_object (SLang_Assoc_Array_Type *a, char *s, SLang_Object_Type *obj)
{
   unsigned long hash;
   SLang_Object_Type *v;

#if USE_CACHED_STRING
   if ((s == Cached_String) && (a == Cached_Array))
     {
	v = Cached_Obj;
	SLang_free_object (v);
     }
   else
     {
#endif
	hash = _SLcompute_string_hash (s);
	if (NULL != (v = find_element (a, s, hash)))
	  SLang_free_object (v);
	else
	  {
	     _SLAssoc_Array_Element_Type *e;
	     
	     e = create_element (a, s, hash);
	     if (e == NULL)
	       return -1;
	     
	     v = &e->value;
	  }
#if USE_CACHED_STRING
     }
#endif

   *v = *obj;

   return 0;
}

static void assoc_destroy (unsigned char type, VOID_STAR ptr)
{
   (void) type;
   delete_assoc_array ((SLang_Assoc_Array_Type *) ptr);
}

static int pop_index (unsigned int num_indices,
		      SLang_MMT_Type **mmt,
		      SLang_Assoc_Array_Type **a,
		      char **str)
{
   if (NULL == (*mmt = SLang_pop_mmt (SLANG_ASSOC_TYPE)))
     {
	*a = NULL;
	*str = NULL;
	return -1;
     }

   if ((num_indices != 1)
       || (-1 == SLang_pop_slstring (str)))
     {
	SLang_verror (SL_NOT_IMPLEMENTED,
		      "Assoc_Type arrays require a single string index");
	SLang_free_mmt (*mmt);
	*mmt = NULL;
	*a = NULL;
	*str = NULL;
	return -1;
     }

   *a = (SLang_Assoc_Array_Type *) SLang_object_from_mmt (*mmt);
   return 0;
}

int _SLassoc_aget (SLtype type, unsigned int num_indices)
{
   SLang_MMT_Type *mmt;
   char *str;
   SLang_Assoc_Array_Type *a;
   SLang_Object_Type *obj;
   int ret;

   (void) type;

   if (-1 == pop_index (num_indices, &mmt, &a, &str))
     return -1;

#if USE_CACHED_STRING
   if ((str == Cached_String) && (a == Cached_Array))
     obj = Cached_Obj;
   else
#endif
     obj = find_element (a, str, _SLcompute_string_hash (str));

   if ((obj == NULL)
       && (a->flags & HAS_DEFAULT_VALUE))
     obj = &a->default_value;

   if (obj == NULL)
     {
	SLang_verror (SL_INTRINSIC_ERROR,
		      "No such element in Assoc Array: %s", str);
	ret = -1;
     }
   else
     {
#if _SLANG_OPTIMIZE_FOR_SPEED
	if (a->is_scalar_type)
	  ret = SLang_push (obj);
	else
#endif
	  ret = _SLpush_slang_obj (obj);
     }

   SLang_free_slstring (str);
   SLang_free_mmt (mmt);
   return ret;
}

int _SLassoc_aput (SLtype type, unsigned int num_indices)
{
   SLang_MMT_Type *mmt;
   char *str;
   SLang_Assoc_Array_Type *a;
   SLang_Object_Type obj;
   int ret;

   (void) type;

   if (-1 == pop_index (num_indices, &mmt, &a, &str))
     return -1;

   ret = -1;

   if (0 == SLang_pop (&obj))
     {
	if ((obj.data_type != a->type)
#if USE_NEW_ANYTYPE_CODE
	    && (a->type != SLANG_ANY_TYPE)
#endif
	    )
	  {
	     (void) SLang_push (&obj);
	     if ((-1 == SLclass_typecast (a->type, 1, 1))
		 || (-1 == SLang_pop (&obj)))
	       goto the_return;
	  }
	
	if (-1 == store_object (a, str, &obj))
	  SLang_free_object (&obj);
	else
	  ret = 0;
     }

   the_return:
   SLang_free_slstring (str);
   SLang_free_mmt (mmt);
   return ret;
}

static int assoc_anew (unsigned char type, unsigned int num_dims)
{
   SLang_MMT_Type *mmt;
   SLang_Assoc_Array_Type *a;
   int has_default_value;

   has_default_value = 0;
   switch (num_dims)
     {
      case 0:
	type = SLANG_ANY_TYPE;
	break;
      case 2:
	(void) SLreverse_stack (2);
	has_default_value = 1;
	/* drop */
      case 1:
	if (0 == SLang_pop_datatype (&type))
	  break;
	num_dims--;
	/* drop */
      default:
	SLdo_pop_n (num_dims);
	SLang_verror (SL_SYNTAX_ERROR, "Usage: Assoc_Type [DataType_Type]");
	return -1;
     }

   a = alloc_assoc_array (type, has_default_value);
   if (a == NULL)
     return -1;

   if (NULL == (mmt = SLang_create_mmt (SLANG_ASSOC_TYPE, (VOID_STAR) a)))
     {
	delete_assoc_array (a);
	return -1;
     }

   if (-1 == SLang_push_mmt (mmt))
     {
	SLang_free_mmt (mmt);
	return -1;
     }

   return 0;
}

static void assoc_get_keys (SLang_Assoc_Array_Type *a)
{
   SLang_Array_Type *at;
   int num;
   unsigned int i, j;
   char **data;

   /* Note: If support for threads is added, then we need to modify this
    * algorithm to prevent another thread from modifying the array.
    * However, that should be handled in inner_interp.
    */
   num = a->num_elements;

   if (NULL == (at = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &num, 1)))
     return;

   data = (char **)at->data;

   i = 0;
   for (j = 0; j < SLASSOC_HASH_TABLE_SIZE; j++)
     {
	_SLAssoc_Array_Element_Type *e;

	e = a->elements[j];
	while (e != NULL)
	  {
	     /* Next cannot fail because it is an slstring */
	     data [i] = SLang_create_slstring (e->key);
	     e = e->next;
	     i++;
	  }
     }
   (void) SLang_push_array (at, 1);
}

static int
transfer_element (SLang_Class_Type *cl, VOID_STAR dest_data,
		  SLang_Object_Type *obj)
{
   unsigned int sizeof_type;
   VOID_STAR src_data;

#if USE_NEW_ANYTYPE_CODE
   if (cl->cl_data_type == SLANG_ANY_TYPE)
     {
	SLang_Any_Type *any;

	if ((-1 == _SLpush_slang_obj (obj))
	    || (-1 == SLang_pop_anytype (&any)))
	  return -1;
	
	*(SLang_Any_Type **)dest_data = any;
	return 0;
     }
#endif
   /* Optimize for scalar */
   if (cl->cl_class_type == SLANG_CLASS_TYPE_SCALAR)
     {
	sizeof_type = cl->cl_sizeof_type;
	memcpy ((char *) dest_data, (char *)&obj->v, sizeof_type);
	return 0;
     }

   src_data = _SLclass_get_ptr_to_value (cl, obj);

   if (-1 == (*cl->cl_acopy) (cl->cl_data_type, src_data, dest_data))
     return -1;

   return 0;
}

static void assoc_get_values (SLang_Assoc_Array_Type *a)
{
   SLang_Array_Type *at;
   int num;
   unsigned int j;
   char *dest_data;
   unsigned char type;
   SLang_Class_Type *cl;
   unsigned int sizeof_type;

   /* Note: If support for threads is added, then we need to modify this
    * algorithm to prevent another thread from modifying the array.
    * However, that should be handled in inner_interp.
    */
   num = a->num_elements;
   type = a->type;

   cl = _SLclass_get_class (type);
   sizeof_type = cl->cl_sizeof_type;

   if (NULL == (at = SLang_create_array (type, 0, NULL, &num, 1)))
     return;

   dest_data = (char *)at->data;

   for (j = 0; j < SLASSOC_HASH_TABLE_SIZE; j++)
     {
	_SLAssoc_Array_Element_Type *e;

	e = a->elements[j];
	while (e != NULL)
	  {
	     if (-1 == transfer_element (cl, (VOID_STAR) dest_data, &e->value))
	       {
		  SLang_free_array (at);
		  return;
	       }

	     dest_data += sizeof_type;
	     e = e->next;
	  }
     }
   (void) SLang_push_array (at, 1);
}

static int assoc_key_exists (SLang_Assoc_Array_Type *a, char *key)
{
   return (NULL != find_element (a, key, _SLcompute_string_hash (key)));
}

static void assoc_delete_key (SLang_Assoc_Array_Type *a, char *key)
{
   unsigned int h;
   _SLAssoc_Array_Element_Type *v, *v0;

   h = (unsigned int) (_SLcompute_string_hash (key) % SLASSOC_HASH_TABLE_SIZE);

   v0 = NULL;
   v = a->elements[h];
   while (v != NULL)
     {
	if (v->key == key)
	  {
	     if (v0 != NULL)
	       v0->next = v->next;
	     else
	       a->elements[h] = v->next;

	     free_element (v);
	     a->num_elements -= 1;
	     return;
	  }
	v0 = v;
	v = v->next;
     }

   /* No such element.  Let it pass with no error. */
}

#define A SLANG_ASSOC_TYPE
#define S SLANG_STRING_TYPE
static SLang_Intrin_Fun_Type Assoc_Table [] =
{
   MAKE_INTRINSIC_1("assoc_get_keys", assoc_get_keys, SLANG_VOID_TYPE, A),
   MAKE_INTRINSIC_1("assoc_get_values", assoc_get_values, SLANG_VOID_TYPE, A),
   MAKE_INTRINSIC_2("assoc_key_exists", assoc_key_exists, SLANG_INT_TYPE, A, S),
   MAKE_INTRINSIC_2("assoc_delete_key", assoc_delete_key, SLANG_VOID_TYPE, A, S),

   SLANG_END_INTRIN_FUN_TABLE
};
#undef A
#undef S

static int assoc_length (unsigned char type, VOID_STAR v, unsigned int *len)
{
   SLang_Assoc_Array_Type *a;

   (void) type;
   a = (SLang_Assoc_Array_Type *) SLang_object_from_mmt (*(SLang_MMT_Type **)v);
   *len = a->num_elements;
   return 0;
}

struct _SLang_Foreach_Context_Type
{
   SLang_MMT_Type *mmt;
   SLang_Assoc_Array_Type *a;
   unsigned int this_hash_index;
   unsigned int next_same_hash_index;
#define CTX_WRITE_KEYS		1
#define CTX_WRITE_VALUES	2
   unsigned char flags;
#if _SLANG_OPTIMIZE_FOR_SPEED
   int is_scalar;
#endif
};

static SLang_Foreach_Context_Type *
cl_foreach_open (unsigned char type, unsigned int num)
{
   SLang_Foreach_Context_Type *c;
   unsigned char flags;
   SLang_MMT_Type *mmt;

   (void) type;

   if (NULL == (mmt = SLang_pop_mmt (SLANG_ASSOC_TYPE)))
     return NULL;

   flags = 0;

   while (num--)
     {
	char *s;

	if (-1 == SLang_pop_slstring (&s))
	  {
	     SLang_free_mmt (mmt);
	     return NULL;
	  }

	if (0 == strcmp (s, "keys"))
	  flags |= CTX_WRITE_KEYS;
	else if (0 == strcmp (s, "values"))
	  flags |= CTX_WRITE_VALUES;
	else
	  {
	     SLang_verror (SL_NOT_IMPLEMENTED,
			   "using '%s' not supported by SLassoc_Type",
			   s);
	     SLang_free_slstring (s);
	     SLang_free_mmt (mmt);
	     return NULL;
	  }

	SLang_free_slstring (s);
     }

   if (NULL == (c = (SLang_Foreach_Context_Type *) SLmalloc (sizeof (SLang_Foreach_Context_Type))))
     {
	SLang_free_mmt (mmt);
	return NULL;
     }

   memset ((char *) c, 0, sizeof (SLang_Foreach_Context_Type));

   if (flags == 0) flags = CTX_WRITE_VALUES|CTX_WRITE_KEYS;

   c->flags = flags;
   c->mmt = mmt;
   c->a = (SLang_Assoc_Array_Type *) SLang_object_from_mmt (mmt);
#if _SLANG_OPTIMIZE_FOR_SPEED
   c->is_scalar = (SLANG_CLASS_TYPE_SCALAR == _SLang_get_class_type (c->a->type));
#endif
   return c;
}

static void cl_foreach_close (unsigned char type, SLang_Foreach_Context_Type *c)
{
   (void) type;
   if (c == NULL) return;
   SLang_free_mmt (c->mmt);
   SLfree ((char *) c);
}

static int cl_foreach (unsigned char type, SLang_Foreach_Context_Type *c)
{
   SLang_Assoc_Array_Type *a;
   _SLAssoc_Array_Element_Type *e;
   unsigned int i, j;

   (void) type;

   if (c == NULL)
     return -1;

   a = c->a;

   i = c->this_hash_index;
   if (i >= SLASSOC_HASH_TABLE_SIZE)
     return 0;

   e = a->elements[i];

   j = c->next_same_hash_index;
   c->next_same_hash_index = j + 1;

   while ((j > 0) && (e != NULL))
     {
	j--;
	e = e->next;
     }

   if (e == NULL)
     {
	do
	  {
	     i++;
	     if (i >= SLASSOC_HASH_TABLE_SIZE)
	       return 0;		       /* no more */
	  }
	while (a->elements [i] == NULL);

	e = a->elements[i];
	c->this_hash_index = i;
	c->next_same_hash_index = 1;
     }

   if ((c->flags & CTX_WRITE_KEYS)
       && (-1 == SLang_push_string (e->key)))
     return -1;

   if (c->flags & CTX_WRITE_VALUES)
     {
#if _SLANG_OPTIMIZE_FOR_SPEED
	if (c->is_scalar)
	  {
	     if (-1 == SLang_push (&e->value))
	       return -1;
	  }
	else
#endif
	  if (-1 == _SLpush_slang_obj (&e->value))
	    return -1;
     }
   
   /* keep going */
   return 1;
}

int SLang_init_slassoc (void)
{
   SLang_Class_Type *cl;

   if (SLclass_is_class_defined (SLANG_ASSOC_TYPE))
     return 0;

   if (NULL == (cl = SLclass_allocate_class ("Assoc_Type")))
     return -1;

   (void) SLclass_set_destroy_function (cl, assoc_destroy);
   (void) SLclass_set_aput_function (cl, _SLassoc_aput);
   (void) SLclass_set_aget_function (cl, _SLassoc_aget);
   (void) SLclass_set_anew_function (cl, assoc_anew);
   cl->cl_length = assoc_length;
   cl->cl_foreach_open = cl_foreach_open;
   cl->cl_foreach_close = cl_foreach_close;
   cl->cl_foreach = cl_foreach;

   if (-1 == SLclass_register_class (cl, SLANG_ASSOC_TYPE, sizeof (SLang_Assoc_Array_Type), SLANG_CLASS_TYPE_MMT))
     return -1;

   if (-1 == SLadd_intrin_fun_table (Assoc_Table, "__SLASSOC__"))
     return -1;

   return 0;
}

