/* Basic type operations for S-Lang */
/* Copyright (c) 1992, 1996, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#if SLANG_HAS_FLOAT
# include <math.h>
#endif

/* #define SL_APP_WANTS_FOREACH	*/       /* for String_Type */
#include "slang.h"
#include "_slang.h"

int SLpop_string (char **s) /*{{{*/
{
   char *sls;

   *s = NULL;

   if (-1 == SLang_pop_slstring (&sls))
     return -1;

   if (NULL == (*s = SLmake_string (sls)))
     {
	SLang_free_slstring (sls);
	return -1;
     }

   SLang_free_slstring (sls);
   return 0;
}

/*}}}*/

int SLang_pop_slstring (char **s) /*{{{*/
{
   return SLclass_pop_ptr_obj (SLANG_STRING_TYPE, (VOID_STAR *)s);
}

/*}}}*/

/* if *data != 0, string should be freed upon use. */
int SLang_pop_string(char **s, int *data) /*{{{*/
{
   if (SLpop_string (s))
     return -1;

   *data = 1;
   return 0;
}

/*}}}*/

int _SLang_push_slstring (char *s)
{
   if (0 == SLclass_push_ptr_obj (SLANG_STRING_TYPE, (VOID_STAR)s))
     return 0;

   SLang_free_slstring (s);
   return -1;
}

int _SLpush_alloced_slstring (char *s, unsigned int len)
{
   if (NULL == (s = _SLcreate_via_alloced_slstring (s, len)))
     return -1;
   
   return _SLang_push_slstring (s);
}

int SLang_push_string (char *t) /*{{{*/
{
   if (t == NULL)
     return SLang_push_null ();

   if (NULL == (t = SLang_create_slstring (t)))
     return -1;

   return _SLang_push_slstring (t);
}

/*}}}*/

int _SLang_dup_and_push_slstring (char *s)
{
   if (NULL == (s = _SLstring_dup_slstring (s)))
     return SLang_push_null ();

   return _SLang_push_slstring (s);
}


/* This function _always_ frees the malloced string */
int SLang_push_malloced_string (char *c) /*{{{*/
{
   int ret;

   ret = SLang_push_string (c);
   SLfree (c);

   return ret;
}

/*}}}*/

#if 0
static int int_int_power (int a, int b)
{
   int r, s;

   if (a == 0) return 0;
   if (b < 0) return 0;
   if (b == 0) return 1;

   s = 1;
   if (a < 0)
     {
	if ((b % 2) == 1) s = -1;
	a = -a;
     }

   /* FIXME: Priority=low
    * This needs optimized
    */
   r = 1;
   while (b)
     {
	r = r * a;
	b--;
     }
   return r * s;
}
#endif

static int
string_string_bin_op_result (int op, unsigned char a, unsigned char b,
			     unsigned char *c)
{
   (void) a;
   (void) b;
   switch (op)
     {
      default:
	return 0;

      case SLANG_PLUS:
	*c = SLANG_STRING_TYPE;
	break;

      case SLANG_GT:
      case SLANG_GE:
      case SLANG_LT:
      case SLANG_LE:
      case SLANG_EQ:
      case SLANG_NE:
	*c = SLANG_CHAR_TYPE;
	break;
     }
   return 1;
}

static int
string_string_bin_op (int op,
		      unsigned char a_type, VOID_STAR ap, unsigned int na,
		      unsigned char b_type, VOID_STAR bp, unsigned int nb,
		      VOID_STAR cp)
{
   char *ic;
   char **a, **b, **c;
   unsigned int n, n_max;
   unsigned int da, db;

   (void) a_type;
   (void) b_type;

   if (na == 1) da = 0; else da = 1;
   if (nb == 1) db = 0; else db = 1;

   if (na > nb) n_max = na; else n_max = nb;

   a = (char **) ap;
   b = (char **) bp;
   
   if ((op != SLANG_NE) && (op != SLANG_EQ))
     for (n = 0; n < n_max; n++)
       {
	  if ((*a == NULL) || (*b == NULL))
	    {
	       SLang_verror (SL_VARIABLE_UNINITIALIZED, "String element[%u] not initialized for binary operation", n);
	       return -1;
	    }
	  a += da; b += db;
       }

   a = (char **) ap;
   b = (char **) bp;
   ic = (char *) cp;
   c = NULL;

   switch (op)
     {
      case SLANG_DIVIDE:
      case SLANG_MINUS:
      default:
	return 0;

       case SLANG_PLUS:
	/* Concat */
	c = (char **) cp;
	for (n = 0; n < n_max; n++)
	  {
	     if (NULL == (c[n] = SLang_concat_slstrings (*a, *b)))
	       goto return_error;

	     a += da; b += db;
	  }
	break;

      case SLANG_NE:
	for (n = 0; n < n_max; n++)
	  {
	     if ((*a == NULL) || (*b == NULL))
	       ic [n] = (*a != *b);
	     else
	       ic [n] = (*a != *b) && (0 != strcmp (*a, *b));

	     a += da;
	     b += db;
	  }
	break;
      case SLANG_GT:
	for (n = 0; n < n_max; n++)
	  {
	     ic [n] = (strcmp (*a, *b) > 0);
	     a += da;
	     b += db;
	  }
	break;
      case SLANG_GE:
	for (n = 0; n < n_max; n++)
	  {
	     ic [n] = (strcmp (*a, *b) >= 0);
	     a += da;
	     b += db;
	  }
	break;
      case SLANG_LT:
	for (n = 0; n < n_max; n++)
	  {
	     ic [n] = (strcmp (*a, *b) < 0);
	     a += da;
	     b += db;
	  }
	break;
      case SLANG_LE:
	for (n = 0; n < n_max; n++)
	  {
	     ic [n] = (strcmp (*a, *b) <= 0);
	     a += da;
	     b += db;
	  }
	break;
      case SLANG_EQ:
	for (n = 0; n < n_max; n++)
	  {
	     if ((*a == NULL) || (*b == NULL))
	       ic[n] = (*a == *b);
	     else
	       ic [n] = (*a == *b) || (strcmp (*a, *b) == 0);
	     a += da;
	     b += db;
	  }
	break;
     }
   return 1;

   return_error:
   if (c != NULL)
     {
	unsigned int nn;
	for (nn = 0; nn < n; nn++)
	  {
	     SLang_free_slstring (c[nn]);
	     c[nn] = NULL;
	  }
	for (nn = n; nn < n_max; nn++)
	  c[nn] = NULL;
     }
   return -1;
}

static void string_destroy (unsigned char unused, VOID_STAR s)
{
   (void) unused;
   SLang_free_slstring (*(char **) s);
}

static int string_push (unsigned char unused, VOID_STAR sptr)
{
   (void) unused;
   return SLang_push_string (*(char **) sptr);
}

static int string_cmp (unsigned char unused, VOID_STAR ap, VOID_STAR bp, int *c)
{
   char *a, *b;
   (void) unused;
   
   a = *(char **) ap;
   b = *(char **) bp;
   if (a != b)
     {
	if (a == NULL) *c = -1;
	else if (b == NULL) *c = 1;
	else *c = strcmp (a, b);
	return 0;
     }
   *c = 0;
   return 0;
}

static int string_to_int (unsigned char a_type, VOID_STAR ap, unsigned int na,
			  unsigned char b_type, VOID_STAR bp)
{
   char **s;
   unsigned int i;
   int *b;

   (void) a_type;
   (void) b_type;

   s = (char **) ap;
   b = (int *) bp;
   for (i = 0; i < na; i++)
     {
	if (s[i] == NULL) b[i] = 0;
	else b[i] = s[i][0];
     }
   return 1;
}

static int string_acopy (SLtype unused, VOID_STAR src_sptr, VOID_STAR dest_sptr)
{
   char *s;
   (void) unused;
   if (NULL == (s = SLang_create_slstring (*(char **)src_sptr)))
     return -1;
   *(char **)dest_sptr = s;
   return 0;
}

struct _SLang_Foreach_Context_Type
{
   char *string;
   unsigned int n;
};

static SLang_Foreach_Context_Type *
string_foreach_open (unsigned char type, unsigned int num)
{
   char *s;
   SLang_Foreach_Context_Type *c;

   (void) type;
   if (num != 0)
     {
	SLang_verror (SL_NOT_IMPLEMENTED,
		      "'foreach using' form not supported by String_Type");
	SLdo_pop_n (num + 1);
	return NULL;
     }
   if (-1 == SLang_pop_slstring (&s))
     return NULL;

   c = (SLang_Foreach_Context_Type *)SLmalloc (sizeof (SLang_Foreach_Context_Type));
   if (c == NULL)
     {
	SLang_free_slstring (s);
	return NULL;
     }

   memset ((char *) c, 0, sizeof (SLang_Foreach_Context_Type));
   c->string = s;

   return c;
}

static void string_foreach_close (unsigned char type, SLang_Foreach_Context_Type *c)
{
   (void) type;
   if (c == NULL) return;
   SLang_free_slstring (c->string);
   SLfree ((char *) c);
}

static int string_foreach (unsigned char type, SLang_Foreach_Context_Type *c)
{
   char ch;

   (void) type;
   ch = c->string[c->n];
   if (ch == 0)
     return 0;			       /* done */

   c->n += 1;

   if (-1 == SLclass_push_int_obj (SLANG_INT_TYPE, ch))
     return -1;

   return 1;
}

int _SLstring_list_push (_SLString_List_Type *p)
{
   unsigned int num;
   int inum;
   SLang_Array_Type *at;
   char **buf;

   if ((buf = p->buf) == NULL)
     return SLang_push_null ();
   
   num = p->num;
   inum = (int) num;
   
   if (num == 0) num++;
   if (num != p->max_num)
     {
	if (NULL == (buf = (char **)SLrealloc ((char *) buf, sizeof (char *) * num)))
	  {
	     _SLstring_list_delete (p);
	     return -1;
	  }
	p->max_num = num;
	p->buf = buf;
     }
     
   if (NULL == (at = SLang_create_array (SLANG_STRING_TYPE, 0, (VOID_STAR) buf, &inum, 1)))
     {
	_SLstring_list_delete (p);
	return -1;
     }
   p->buf = NULL;
   _SLstring_list_delete (p);
   return SLang_push_array (at, 1);
}

int _SLstring_list_init (_SLString_List_Type *p, unsigned int max_num, unsigned int delta_num)
{
   if (NULL == (p->buf = (char **) SLmalloc (max_num * sizeof (char *))))
     return -1;
   
   p->max_num = max_num;
   p->num = 0;
   p->delta_num = delta_num;
   return 0;
}

int _SLstring_list_append (_SLString_List_Type *p, char *s)
{
   if (s == NULL)
     {
	_SLstring_list_delete (p);
	return -1;
     }

   if (p->max_num == p->num)
     {
	char **b;
	unsigned int max_num = p->num + p->delta_num;
	b = (char **)SLrealloc ((char *)p->buf, max_num * sizeof (char *));
	if (b == NULL)
	  {
	     _SLstring_list_delete (p);
	     SLang_free_slstring (s);
	     return -1;
	  }
	p->buf = b;
	p->max_num = max_num;
     }
   
   p->buf[p->num] = s;
   p->num++;
   return 0;
}

void _SLstring_list_delete (_SLString_List_Type *p)
{
   if (p->buf != NULL)
     {
	unsigned int i, imax;
	char **buf = p->buf;
	imax = p->num;
	for (i = 0; i < imax; i++)
	  SLang_free_slstring (buf[i]);
	SLfree ((char *)buf);
	p->buf = NULL;
     }
}

/* Ref type */
int SLang_pop_ref (SLang_Ref_Type **ref)
{
   return SLclass_pop_ptr_obj (SLANG_REF_TYPE, (VOID_STAR *)ref);
}

/* Note: This is ok if ptr is NULL.  Some routines rely on this behavior */
int _SLang_push_ref (int is_global, VOID_STAR ptr)
{
   SLang_Ref_Type *r;

   if (ptr == NULL)
     return SLang_push_null ();

   r = (SLang_Ref_Type *) SLmalloc (sizeof (SLang_Ref_Type));
   if (r == NULL) return -1;

   r->is_global = is_global;
   r->v.nt = (SLang_Name_Type *) ptr;

   if (-1 == SLclass_push_ptr_obj (SLANG_REF_TYPE, (VOID_STAR) r))
     {
	SLfree ((char *) r);
	return -1;
     }
   return 0;
}

static void ref_destroy (unsigned char type, VOID_STAR ptr)
{
   (void) type;
   SLfree ((char *) *(SLang_Ref_Type **)ptr);
}

void SLang_free_ref (SLang_Ref_Type *ref)
{
   SLfree ((char *) ref);
}

static int ref_push (unsigned char type, VOID_STAR ptr)
{
   SLang_Ref_Type *ref;

   (void) type;

   ref = *(SLang_Ref_Type **) ptr;

   if (ref == NULL)
     return SLang_push_null ();

   return _SLang_push_ref (ref->is_global, (VOID_STAR) ref->v.nt);
}

int SLang_assign_to_ref (SLang_Ref_Type *ref, SLtype type, VOID_STAR v)
{
   SLang_Object_Type *stkptr;
   SLang_Class_Type *cl;
   
   cl = _SLclass_get_class (type);

   /* Use apush since this function is passing ``array'' bytes rather than the
    * address of the data.  I need to somehow make this more consistent.  To
    * see what I mean, consider:
    * 
    *    double z[2];
    *    char *s = "silly";
    *    char bytes[10];  BAD--- Don't do this
    *    int i;
    * 
    *    SLang_assign_to_ref (ref, SLANG_INT_TYPE,    &i);
    *    SLang_assign_to_ref (ref, SLANG_STRING_TYPE, &s);
    *    SLang_assign_to_ref (ref, SLANG_COMPLEX_TYPE, z);
    * 
    * That is, all external routines that take a VOID_STAR argument need to
    * be documented such that how the function should be called with the
    * various class_types.
    */
   if (-1 == (*cl->cl_apush) (type, v))
     return -1;

   stkptr = _SLang_get_run_stack_pointer ();
   if (0 == _SLang_deref_assign (ref))
     return 0;

   if (stkptr != _SLang_get_run_stack_pointer ())
     SLdo_pop ();

   return -1;
}

static char *ref_string (unsigned char type, VOID_STAR ptr)
{
   SLang_Ref_Type *ref;

   (void) type;
   ref = *(SLang_Ref_Type **) ptr;
   if (ref->is_global)
     {
	char *name, *s;

	name = ref->v.nt->name;
	if ((name != NULL)
	    && (NULL != (s = SLmalloc (strlen(name) + 2))))
	  {
	     *s = '&';
	     strcpy (s + 1, name);
	     return s;
	  }
	
	return NULL;
     }
   return SLmake_string ("Local Variable Reference");
}

static int ref_dereference (unsigned char unused, VOID_STAR ptr)
{
   (void) unused;
   return _SLang_dereference_ref (*(SLang_Ref_Type **) ptr);
}

static int ref_cmp (unsigned char type, VOID_STAR a, VOID_STAR b, int *c)
{
   SLang_Ref_Type *ra, *rb;

   (void) type;
   
   ra = *(SLang_Ref_Type **)a;
   rb = *(SLang_Ref_Type **)b;
   
   if (ra == NULL)
     {
	if (rb == NULL) *c = 0;
	else *c = -1;
	return 0;
     }
   if (rb == NULL)
     {
	*c = 1;
	return 0;
     }
	
   if (ra->v.nt == rb->v.nt)
     *c = 0;
   else *c = strcmp (ra->v.nt->name, rb->v.nt->name);
   return 0;
}

   
SLang_Name_Type *SLang_pop_function (void)
{
   SLang_Ref_Type *ref;
   SLang_Name_Type *f;

   if (SLang_peek_at_stack () == SLANG_STRING_TYPE)
     {
	char *name;
	
	if (-1 == SLang_pop_slstring (&name))
	  return NULL;
	
	if (NULL == (f = SLang_get_function (name)))
	  {
	     SLang_verror (SL_UNDEFINED_NAME, "Function %s does not exist", name);
	     SLang_free_slstring (name);
	     return NULL;
	  }
	SLang_free_slstring (name);
	return f;
     }

   if (-1 == SLang_pop_ref (&ref))
     return NULL;

   f = SLang_get_fun_from_ref (ref);
   SLang_free_ref (ref);
   return f;
}

/* This is a placeholder for version 2 */
void SLang_free_function (SLang_Name_Type *f)
{
   (void) f;
}

/* NULL type */
int SLang_push_null (void)
{
   return SLclass_push_ptr_obj (SLANG_NULL_TYPE, NULL);
}

int SLang_pop_null (void)
{
   SLang_Object_Type obj;
   return _SLang_pop_object_of_type (SLANG_NULL_TYPE, &obj, 0);
}

static int null_push (unsigned char unused, VOID_STAR ptr_unused)
{
   (void) unused; (void) ptr_unused;
   return SLang_push_null ();
}

static int null_pop (unsigned char type, VOID_STAR ptr)
{
   (void) type;

   if (-1 == SLang_pop_null ())
     return -1;

   *(char **) ptr = NULL;
   return 0;
}

/* Implement foreach (NULL) using (whatever) to do nothing.  This is useful
 * because suppose that X is a list but is NULL in some situations.  Then
 * when it is NULL, we want foreach(X) to do nothing.
 */
static SLang_Foreach_Context_Type *
null_foreach_open (unsigned char type, unsigned int num)
{
   (void) type;
   SLdo_pop_n (num + 1);
   return (SLang_Foreach_Context_Type *)1;
}

static void null_foreach_close (unsigned char type, SLang_Foreach_Context_Type *c)
{
   (void) type;
   (void) c;
}

static int null_foreach (unsigned char type, SLang_Foreach_Context_Type *c)
{
   (void) type;
   (void) c;
   return 0;
}

static int null_to_bool (unsigned char type, int *t)
{
   (void) type;
   *t = 0;
   return SLang_pop_null ();
}

/* AnyType */
int _SLanytype_typecast (unsigned char a_type, VOID_STAR ap, unsigned int na,
			 unsigned char b_type, VOID_STAR bp)
{
   SLang_Class_Type *cl;
   SLang_Any_Type **any;
   unsigned int i;
   unsigned int sizeof_type;

   (void) b_type;

   any = (SLang_Any_Type **) bp;
   
   cl = _SLclass_get_class (a_type);
   sizeof_type = cl->cl_sizeof_type;

   for (i = 0; i < na; i++)
     {
	if ((-1 == (*cl->cl_apush) (a_type, ap))
	    || (-1 == SLang_pop_anytype (&any[i])))
	  {
	     while (i != 0)
	       {
		  i--;
		  SLang_free_anytype (any[i]);
		  any[i] = NULL;
	       }
	     return -1;
	  }
	ap = (VOID_STAR)((char *)ap + sizeof_type);
     }

   return 1;
}

int SLang_pop_anytype (SLang_Any_Type **any)
{
   SLang_Object_Type *obj;

   *any = NULL;

   if (NULL == (obj = (SLang_Object_Type *) SLmalloc (sizeof (SLang_Object_Type))))
     return -1;

   if (-1 == SLang_pop (obj))
     {
	SLfree ((char *) obj);
	return -1;
     }
   *any = (SLang_Any_Type *)obj;
   return 0;
}

/* This function will result in an object that is represented by the
 * anytype object.
 */
int SLang_push_anytype (SLang_Any_Type *any)
{
   return _SLpush_slang_obj ((SLang_Object_Type *)any);
}

/* After this call, the stack will contain an Any_Type object */
static int anytype_push (unsigned char type, VOID_STAR ptr)
{
   SLang_Any_Type *obj;

   /* Push the object onto the stack, then pop it back off into our anytype
    * container.  That way, any memory managing associated with the type
    * will be performed automatically.  Another way to think of it is that
    * pushing an Any_Type onto the stack will create another copy of the
    * object represented by it.
    */
   if (-1 == _SLpush_slang_obj (*(SLang_Object_Type **)ptr))
     return -1;

   if (-1 == SLang_pop_anytype (&obj))
     return -1;

   /* There is no need to reference count the anytype objects since every
    * push results in a new anytype container.
    */
   if (-1 == SLclass_push_ptr_obj (type, (VOID_STAR) obj))
     {
	SLang_free_anytype (obj);
	return -1;
     }

   return 0;
}

static void anytype_destroy (unsigned char type, VOID_STAR ptr)
{
   SLang_Object_Type *obj;

   (void) type;
   obj = *(SLang_Object_Type **)ptr;
   SLang_free_object (obj);
   SLfree ((char *) obj);
}

void SLang_free_anytype (SLang_Any_Type *any)
{
   if (any != NULL)
     anytype_destroy (SLANG_ANY_TYPE, (VOID_STAR) &any);
}

static int anytype_dereference (unsigned char unused, VOID_STAR ptr)
{
   (void) unused;
   return _SLpush_slang_obj (*(SLang_Object_Type **) ptr);
}

#if 0
/* This function performs a deref since we may want the symmetry
 *  a = Any_Type[1];  a[x] = "foo"; bar = a[x]; ==> bar == "foo"
 * That is, we do not want bar to be an Any_Type.
 * 
 * Unfortunately, this does not work because of the use of the transfer
 * buffer by both slarray.c and sltypecast.c.  I can work around that 
 * but I am not sure that I like typeof(Any_Type[0]) != Any_Type.
 */
static int anytype_apush (SLtype type, VOID_STAR ptr)
{
   (void) type;
   return _SLpush_slang_obj (*(SLang_Object_Type **)ptr);
}
#endif

/* SLANG_INTP_TYPE */
static int intp_push (unsigned char unused, VOID_STAR ptr)
{
   (void) unused;
   return SLclass_push_int_obj (SLANG_INT_TYPE, **(int **)ptr);
}

static int intp_pop (unsigned char unused, VOID_STAR ptr)
{
   (void) unused;
   return SLang_pop_integer (*(int **) ptr);
}

static int undefined_push (unsigned char t, VOID_STAR p)
{
   (void) t; (void) p;
   if (SLang_Error == 0)
     SLang_Error = SL_VARIABLE_UNINITIALIZED;
   return -1;
}

int _SLregister_types (void)
{
   SLang_Class_Type *cl;

#if 1
   /* A good compiler should optimize this code away. */
   if ((sizeof(short) != SIZEOF_SHORT)
       || (sizeof(int) != SIZEOF_INT)
       || (sizeof(long) != SIZEOF_LONG)
       || (sizeof(float) != SIZEOF_FLOAT)
       || (sizeof(double) != SIZEOF_DOUBLE))
     SLang_exit_error ("S-Lang Library not built properly.  Fix SIZEOF_* in config.h and recompile");
#endif
   if (-1 == _SLclass_init ())
     return -1;

   /* Undefined Type */
   if (NULL == (cl = SLclass_allocate_class ("Undefined_Type")))
     return -1;
   (void) SLclass_set_push_function (cl, undefined_push);
   (void) SLclass_set_pop_function (cl, undefined_push);
   if (-1 == SLclass_register_class (cl, SLANG_UNDEFINED_TYPE, sizeof (int),
				     SLANG_CLASS_TYPE_SCALAR))
     return -1;
   /* Make Void_Type a synonym for Undefined_Type.  Note that this does 
    * not mean that Void_Type represents SLANG_VOID_TYPE.  Void_Type is 
    * used by array_map to indicate no array is to be created.
    */
   if (-1 == SLclass_create_synonym ("Void_Type", SLANG_UNDEFINED_TYPE))
     return -1;

   if (-1 == _SLarith_register_types ())
     return -1;

   /* SLANG_INTP_TYPE */
   if (NULL == (cl = SLclass_allocate_class ("_IntegerP_Type")))
     return -1;
   (void) SLclass_set_push_function (cl, intp_push);
   (void) SLclass_set_pop_function (cl, intp_pop);
   if (-1 == SLclass_register_class (cl, SLANG_INTP_TYPE, sizeof (int),
				     SLANG_CLASS_TYPE_SCALAR))
     return -1;

   /* String Type */

   if (NULL == (cl = SLclass_allocate_class ("String_Type")))
     return -1;
   (void) SLclass_set_destroy_function (cl, string_destroy);
   (void) SLclass_set_push_function (cl, string_push);
   (void) SLclass_set_acopy_function (cl, string_acopy);
   cl->cl_foreach_open = string_foreach_open;
   cl->cl_foreach_close = string_foreach_close;
   cl->cl_foreach = string_foreach;
   cl->cl_cmp = string_cmp;
   if (-1 == SLclass_register_class (cl, SLANG_STRING_TYPE, sizeof (char *),
				     SLANG_CLASS_TYPE_PTR))
     return -1;

   /* ref Type */
   if (NULL == (cl = SLclass_allocate_class ("Ref_Type")))
     return -1;
   cl->cl_dereference = ref_dereference;
   cl->cl_push = ref_push;
   cl->cl_destroy = ref_destroy;
   cl->cl_string = ref_string;
   cl->cl_cmp = ref_cmp;
   if (-1 == SLclass_register_class (cl, SLANG_REF_TYPE,
				     sizeof (SLang_Ref_Type *),
				     SLANG_CLASS_TYPE_PTR))
     return -1;

   /* NULL Type */

   if (NULL == (cl = SLclass_allocate_class ("Null_Type")))
     return -1;
   cl->cl_push = null_push;
   cl->cl_pop = null_pop;
   cl->cl_foreach_open = null_foreach_open;
   cl->cl_foreach_close = null_foreach_close;
   cl->cl_foreach = null_foreach;
   cl->cl_to_bool = null_to_bool;
   if (-1 == SLclass_register_class (cl, SLANG_NULL_TYPE, sizeof (char *),
				     SLANG_CLASS_TYPE_SCALAR))
     return -1;

   /* AnyType */
   if (NULL == (cl = SLclass_allocate_class ("Any_Type")))
     return -1;
   (void) SLclass_set_push_function (cl, anytype_push);
   (void) SLclass_set_destroy_function (cl, anytype_destroy);
#if 0
   (void) SLclass_set_apush_function (cl, anytype_apush);
#endif
   cl->cl_dereference = anytype_dereference;
   if (-1 == SLclass_register_class (cl, SLANG_ANY_TYPE, sizeof (VOID_STAR),
				     SLANG_CLASS_TYPE_PTR))
     return -1;

   if (-1 == _SLang_init_bstring ())
     return -1;

   if ((-1 == SLclass_add_typecast (SLANG_STRING_TYPE, SLANG_INT_TYPE, string_to_int, 0))
       || (-1 == SLclass_add_binary_op (SLANG_STRING_TYPE, SLANG_STRING_TYPE, string_string_bin_op, string_string_bin_op_result)))
     return -1;

   return 0;
}

