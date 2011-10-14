/* Structure type implementation */
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

void _SLstruct_delete_struct (_SLang_Struct_Type *s)
{
   _SLstruct_Field_Type *field, *field_max;

   if (s == NULL) return;

   if (s->num_refs > 1)
     {
	s->num_refs -= 1;
	return;
     }

   field = s->fields;
   if (field != NULL)
     {
	field_max = field + s->nfields;

	while (field < field_max)
	  {
	     SLang_free_object (&field->obj);
	     SLang_free_slstring (field->name);   /* could be NULL */
	     field++;
	  }
	SLfree ((char *) s->fields);
     }
   SLfree ((char *) s);
}

static _SLang_Struct_Type *allocate_struct (unsigned int nfields)
{
   _SLang_Struct_Type *s;
   _SLstruct_Field_Type *f;
   unsigned int i, size;

   s = (_SLang_Struct_Type *) SLmalloc (sizeof (_SLang_Struct_Type));
   if (s == NULL) return NULL;

   SLMEMSET((char *) s, 0, sizeof (_SLang_Struct_Type));

   size = nfields * sizeof(_SLstruct_Field_Type);
   if (NULL == (f = (_SLstruct_Field_Type *) SLmalloc (size)))
     {
	SLfree ((char *) s);
	return NULL;
     }
   SLMEMSET ((char *) f, 0, size);
   s->nfields = nfields;
   s->fields = f;

   /* By default, all structs will be created with elements set to NULL.  I
    * do not know whether or not it is better to use SLANG_UNDEFINED_TYPE.
    */
   for (i = 0; i < nfields; i++)
     f[i].obj.data_type = SLANG_NULL_TYPE;

   return s;
}

static int push_struct_of_type (unsigned char type, _SLang_Struct_Type *s)
{
   SLang_Object_Type obj;

   obj.data_type = type;
   obj.v.struct_val = s;
   s->num_refs += 1;

   if (0 == SLang_push (&obj))
     return 0;

   s->num_refs -= 1;
   return -1;
}

int _SLang_push_struct (_SLang_Struct_Type *s)
{
   return push_struct_of_type (SLANG_STRUCT_TYPE, s);
}

int _SLang_pop_struct (_SLang_Struct_Type **sp)
{
   SLang_Object_Type obj;
   SLang_Class_Type *cl;
   unsigned char type;

   if (0 != SLang_pop (&obj))
     return -1;

   type = obj.data_type;
   if (type != SLANG_STRUCT_TYPE)
     {
	cl = _SLclass_get_class (type);
	if (cl->cl_struct_def == NULL)
	  {
	     *sp = NULL;
	     SLang_free_object (&obj);
	     SLang_verror (SL_TYPE_MISMATCH,
			   "Expecting struct type object.  Found %s",
			   cl->cl_name);
	     return -1;
	  }
     }

   *sp = obj.v.struct_val;
   return 0;
}

static void struct_destroy (unsigned char type, VOID_STAR vs)
{
   (void) type;
   _SLstruct_delete_struct (*(_SLang_Struct_Type **) vs);
}

static int struct_push (unsigned char type, VOID_STAR ptr)
{
   return push_struct_of_type (type, *(_SLang_Struct_Type **) ptr);
}

static _SLstruct_Field_Type *find_field (_SLang_Struct_Type *s, char *name)
{
   _SLstruct_Field_Type *f, *fmax;

   f = s->fields;
   fmax = f + s->nfields;

   while (f < fmax)
     {
	/* Since both these are slstrings, only compare pointer */
	if (name == f->name)
	  return f;

	f++;
     }

   return NULL;
}

static _SLstruct_Field_Type *pop_field (_SLang_Struct_Type *s, char *name,
					_SLstruct_Field_Type *(*find)(_SLang_Struct_Type *, char *))
{
   _SLstruct_Field_Type *f;

   f = (*find) (s, name);
   if (f == NULL)
     SLang_verror (SL_INVALID_PARM, "struct has no field named %s", name);
   return f;
}

static _SLang_Struct_Type *
  create_struct (unsigned int nfields,
		 char **field_names,
		 unsigned char *field_types,
		 VOID_STAR *field_values)
{
   _SLang_Struct_Type *s;
   _SLstruct_Field_Type *f;
   unsigned int i;

   if (NULL == (s = allocate_struct (nfields)))
     return NULL;

   f = s->fields;
   for (i = 0; i < nfields; i++)
     {
	unsigned char type;
	SLang_Class_Type *cl;
	VOID_STAR value;
	char *name = field_names [i];

	if (name == NULL)
	  {
	     SLang_verror (SL_APPLICATION_ERROR, "A struct field name cannot be NULL");
	     goto return_error;
	  }

	if (-1 == _SLcheck_identifier_syntax (name))
	  goto return_error;

	if (NULL == (f->name = SLang_create_slstring (name)))
	  goto return_error;

	if ((field_values == NULL)
	    || (NULL == (value = field_values [i])))
	  {
	     f++;
	     continue;
	  }

	type = field_types[i];
	cl = _SLclass_get_class (type);

	if ((-1 == (cl->cl_apush (type, value)))
	    || (-1 == SLang_pop (&f->obj)))
	  goto return_error;

	f++;
     }

   return s;

   return_error:
   _SLstruct_delete_struct (s);
   return NULL;
}

int SLstruct_create_struct (unsigned int nfields,
			    char **field_names,
			    unsigned char *field_types,
			    VOID_STAR *field_values)
{
   _SLang_Struct_Type *s;
   
   if (NULL == (s = create_struct (nfields, field_names, field_types, field_values)))
     return -1;

   if (0 == _SLang_push_struct (s))
     return 0;

   _SLstruct_delete_struct (s);
   return -1;
}

/* Interpreter interface */

int _SLstruct_define_struct (void)
{
   int nfields;
   _SLang_Struct_Type *s;
   _SLstruct_Field_Type *f;

   if (-1 == SLang_pop_integer (&nfields))
     return -1;

   if (nfields <= 0)
     {
	SLang_verror (SL_INVALID_PARM, "Number of struct fields must be > 0");
	return -1;
     }

   if (NULL == (s = allocate_struct (nfields)))
     return -1;

   f = s->fields;
   while (nfields)
     {
	char *name;

	nfields--;
	if (-1 == SLang_pop_slstring (&name))
	  {
	     _SLstruct_delete_struct (s);
	     return -1;
	  }
	f[nfields].name = name;
     }

   if (-1 == _SLang_push_struct (s))
     {
	_SLstruct_delete_struct (s);
	return -1;
     }
   return 0;
}

/* Simply make a struct that contains the same fields as struct s.  Do not
 * duplicate the field values.
 */
static _SLang_Struct_Type *make_struct_shell (_SLang_Struct_Type *s)
{
   _SLang_Struct_Type *new_s;
   _SLstruct_Field_Type *new_f, *old_f;
   unsigned int i, nfields;

   nfields = s->nfields;
   if (NULL == (new_s = allocate_struct (nfields)))
     return NULL;

   new_f = new_s->fields;
   old_f = s->fields;

   for (i = 0; i < nfields; i++)
     {
	if (NULL == (new_f[i].name = SLang_create_slstring (old_f[i].name)))
	  {
	     _SLstruct_delete_struct (new_s);
	     return NULL;
	  }
     }
   return new_s;
}

static int struct_init_array_object (unsigned char type, VOID_STAR addr)
{
   SLang_Class_Type *cl;
   _SLang_Struct_Type *s;

   cl = _SLclass_get_class (type);
   if (NULL == (s = make_struct_shell (cl->cl_struct_def)))
     return -1;

   s->num_refs = 1;
   *(_SLang_Struct_Type **) addr = s;
   return 0;
}

static int
typedefed_struct_datatype_deref (unsigned char type)
{
   SLang_Class_Type *cl;
   _SLang_Struct_Type *s;

   cl = _SLclass_get_class (type);
   if (NULL == (s = make_struct_shell (cl->cl_struct_def)))
     return -1;

   if (-1 == push_struct_of_type (type, s))
     {
	_SLstruct_delete_struct (s);
	return -1;
     }

   return 0;
}

static _SLang_Struct_Type *duplicate_struct (_SLang_Struct_Type *s)
{
   _SLang_Struct_Type *new_s;
   _SLstruct_Field_Type *new_f, *f, *fmax;

   new_s = make_struct_shell (s);

   if (new_s == NULL)
     return NULL;

   f = s->fields;
   fmax = f + s->nfields;
   new_f = new_s->fields;

   while (f < fmax)
     {
	SLang_Object_Type *obj;

	obj = &f->obj;
	if (obj->data_type != SLANG_UNDEFINED_TYPE)
	  {
	     if ((-1 == _SLpush_slang_obj (obj))
		 || (-1 == SLang_pop (&new_f->obj)))
	       {
		  _SLstruct_delete_struct (new_s);
		  return NULL;
	       }
	  }
	new_f++;
	f++;
     }

   return new_s;
}

static int struct_dereference (unsigned char type, VOID_STAR addr)
{
   _SLang_Struct_Type *s;

   if (NULL == (s = duplicate_struct (*(_SLang_Struct_Type **) addr)))
     return -1;

   if (-1 == push_struct_of_type (type, s))
     {
	_SLstruct_delete_struct (s);
	return -1;
     }

   return 0;
}

/*{{{ foreach */

struct _SLang_Foreach_Context_Type
{
   _SLang_Struct_Type *s;
   char *next_field_name;
};

static SLang_Foreach_Context_Type *
struct_foreach_open (unsigned char type, unsigned int num)
{
   SLang_Foreach_Context_Type *c;
   _SLang_Struct_Type *s;
   char *next_name;

   (void) type;

   if (-1 == _SLang_pop_struct (&s))
     return NULL;

   switch (num)
     {
      case 0:
	next_name = SLang_create_slstring ("next");
	break;

      case 1:
	if (-1 == SLang_pop_slstring (&next_name))
	  next_name = NULL;
	break;

      default:
	next_name = NULL;
	SLang_verror (SL_NOT_IMPLEMENTED,
		      "'foreach (Struct_Type) using' requires single control value");
	SLdo_pop_n (num);
	break;
     }

   if (next_name == NULL)
     {
	_SLstruct_delete_struct (s);
	return NULL;
     }

   c = (SLang_Foreach_Context_Type *)SLmalloc (sizeof (SLang_Foreach_Context_Type));
   if (c == NULL)
     {
	_SLstruct_delete_struct (s);
	SLang_free_slstring (next_name);
	return NULL;
     }
   memset ((char *) c, 0, sizeof (SLang_Foreach_Context_Type));

   c->next_field_name = next_name;
   c->s = s;

   return c;
}

static void struct_foreach_close (unsigned char type, SLang_Foreach_Context_Type *c)
{
   (void) type;
   if (c == NULL) return;

   SLang_free_slstring (c->next_field_name);
   if (c->s != NULL) _SLstruct_delete_struct (c->s);
   SLfree ((char *) c);
}

static int struct_foreach (unsigned char type, SLang_Foreach_Context_Type *c)
{
   _SLstruct_Field_Type *f;
   _SLang_Struct_Type *next_s;

   (void) type;

   if (c == NULL)
     return -1;

   if (c->s == NULL)
     return 0;			       /* done */

   if (-1 == _SLang_push_struct (c->s))
     return -1;

   /* Now get the next one ready for the next foreach loop */

   next_s = NULL;
   if (NULL != (f = find_field (c->s, c->next_field_name)))
     {
	SLang_Class_Type *cl;

	cl = _SLclass_get_class (f->obj.data_type);
	/* Note that I cannot simply look for SLANG_STRUCT_TYPE since the
	 * user may have typedefed another struct type.  So, look at the
	 * class methods.
	 */
	if (cl->cl_foreach_open == struct_foreach_open)
	  {
	     next_s = f->obj.v.struct_val;
	     next_s->num_refs += 1;
	  }
     }

   _SLstruct_delete_struct (c->s);
   c->s = next_s;

   /* keep going */
   return 1;
}

/*}}}*/

static int struct_sput (unsigned char type, char *name)
{
   _SLang_Struct_Type *s;
   _SLstruct_Field_Type *f;
   SLang_Object_Type obj;

   (void) type;

   if (-1 == _SLang_pop_struct (&s))
     return -1;

   if ((NULL == (f = pop_field (s, name, find_field)))
       || (-1 == SLang_pop (&obj)))
     {
	_SLstruct_delete_struct (s);
	return -1;
     }

   SLang_free_object (&f->obj);
   f->obj = obj;
   _SLstruct_delete_struct (s);
   return 0;
}

static int struct_sget (unsigned char type, char *name)
{
   _SLang_Struct_Type *s;
   _SLstruct_Field_Type *f;
   int ret;

   (void) type;

   if (-1 == _SLang_pop_struct (&s))
     return -1;

   if (NULL == (f = pop_field (s, name, find_field)))
     {
	_SLstruct_delete_struct (s);
	return -1;
     }

   ret = _SLpush_slang_obj (&f->obj);
   _SLstruct_delete_struct (s);
   return ret;
}

static int struct_typecast
  (unsigned char a_type, VOID_STAR ap, unsigned int na,
   unsigned char b_type, VOID_STAR bp)
{
   _SLang_Struct_Type **a, **b;
   unsigned int i;

   (void) a_type;
   (void) b_type;

   a = (_SLang_Struct_Type **) ap;
   b = (_SLang_Struct_Type **) bp;
   for (i = 0; i < na; i++)
     {
	b[i] = a[i];
	if (a[i] != NULL)
	  a[i]->num_refs += 1;
     }

   return 1;
}

int _SLstruct_define_typedef (void)
{
   char *type_name;
   _SLang_Struct_Type *s, *s1;
   SLang_Class_Type *cl;

   if (-1 == SLang_pop_slstring (&type_name))
     return -1;

   if (-1 == _SLang_pop_struct (&s))
     {
	SLang_free_slstring (type_name);
	return -1;
     }

   if (NULL == (s1 = make_struct_shell (s)))
     {
	SLang_free_slstring (type_name);
	_SLstruct_delete_struct (s);
	return -1;
     }

   _SLstruct_delete_struct (s);

   if (NULL == (cl = SLclass_allocate_class (type_name)))
     {
	SLang_free_slstring (type_name);
	_SLstruct_delete_struct (s1);
	return -1;
     }
   SLang_free_slstring (type_name);

   cl->cl_struct_def = s1;
   cl->cl_init_array_object = struct_init_array_object;
   cl->cl_datatype_deref = typedefed_struct_datatype_deref;
   cl->cl_destroy = struct_destroy;
   cl->cl_push = struct_push;
   cl->cl_dereference = struct_dereference;
   cl->cl_foreach_open = struct_foreach_open;
   cl->cl_foreach_close = struct_foreach_close;
   cl->cl_foreach = struct_foreach;

   cl->cl_sget = struct_sget;
   cl->cl_sput = struct_sput;

   if (-1 == SLclass_register_class (cl,
				     SLANG_VOID_TYPE,   /* any open slot */
				     sizeof (_SLang_Struct_Type),
				     SLANG_CLASS_TYPE_PTR))
     {
	/* FIXME: Priority=low */
	/* There is a memory leak here if this fails... */
	return -1;
     }
   /* Note: typecast from a user type to a struct type allowed but not the other
    * way.
    */
   if (-1 == SLclass_add_typecast (cl->cl_data_type, SLANG_STRUCT_TYPE, struct_typecast, 1))
     return -1;

   return 0;
}

static int
struct_datatype_deref (unsigned char stype)
{
   (void) stype;

   if (SLang_peek_at_stack () == SLANG_ARRAY_TYPE)
     {
	SLang_Array_Type *at;
	int status;

	if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
	  return -1;

	status = SLstruct_create_struct (at->num_elements,
					 (char **) at->data, NULL, NULL);

	SLang_free_array (at);
	return status;
     }

   SLang_push_integer (SLang_Num_Function_Args);
   return _SLstruct_define_struct ();
}

static int register_struct (void)
{
   SLang_Class_Type *cl;

   if (NULL == (cl = SLclass_allocate_class ("Struct_Type")))
     return -1;

   (void) SLclass_set_destroy_function (cl, struct_destroy);
   (void) SLclass_set_push_function (cl, struct_push);
   cl->cl_dereference = struct_dereference;
   cl->cl_datatype_deref = struct_datatype_deref;

   cl->cl_foreach_open = struct_foreach_open;
   cl->cl_foreach_close = struct_foreach_close;
   cl->cl_foreach = struct_foreach;

   cl->cl_sget = struct_sget;
   cl->cl_sput = struct_sput;

   if (-1 == SLclass_register_class (cl, SLANG_STRUCT_TYPE, sizeof (_SLang_Struct_Type),
				     SLANG_CLASS_TYPE_PTR))
     return -1;

   return 0;
}

static void get_struct_field_names (_SLang_Struct_Type *s)
{
   SLang_Array_Type *a;
   char **data;
   int i, nfields;
   _SLstruct_Field_Type *f;

   nfields = (int) s->nfields;

   if (NULL == (a = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &nfields, 1)))
     return;

   f = s->fields;
   data = (char **) a->data;
   for (i = 0; i < nfields; i++)
     {
	/* Since we are dealing with hashed strings, the next call should not
	 * fail.  If it does, the interpreter will handle it at some other
	 * level.
	 */
	data [i] = SLang_create_slstring (f[i].name);
     }

   SLang_push_array (a, 1);
}

static int push_struct_fields (_SLang_Struct_Type *s)
{
   _SLstruct_Field_Type *f, *fmax;
   int num;

   f = s->fields;
   fmax = f + s->nfields;

   num = 0;
   while (fmax > f)
     {
	fmax--;
	if (-1 == _SLpush_slang_obj (&fmax->obj))
	  break;

	num++;
     }

   return num;
}

/* Syntax: set_struct_field (s, name, value); */
static void struct_set_field (void)
{
   _SLang_Struct_Type *s;
   _SLstruct_Field_Type *f;
   SLang_Object_Type obj;
   char *name;

   if (-1 == SLang_pop (&obj))
     return;

   if (-1 == SLang_pop_slstring (&name))
     {
	SLang_free_object (&obj);
	return;
     }

   if (-1 == _SLang_pop_struct (&s))
     {
	SLang_free_slstring (name);
	SLang_free_object (&obj);
	return;
     }

   if (NULL == (f = pop_field (s, name, find_field)))
     {
	_SLstruct_delete_struct (s);
	SLang_free_slstring (name);
	SLang_free_object (&obj);
	return;
     }

   SLang_free_object (&f->obj);
   f->obj = obj;

   _SLstruct_delete_struct (s);
   SLang_free_slstring (name);
}

/* Syntax: set_struct_fields (s, values....); */
static void set_struct_fields (void)
{
   unsigned int n;
   _SLang_Struct_Type *s;
   _SLstruct_Field_Type *f;

   n = (unsigned int) SLang_Num_Function_Args;

   if (-1 == SLreverse_stack (n))
     return;

   n--;
   if (-1 == _SLang_pop_struct (&s))
     {
	SLdo_pop_n (n);
	return;
     }

   if (n > s->nfields)
     {
	SLdo_pop_n (n);
	SLang_verror (SL_INVALID_PARM, "Too many values for structure");
	_SLstruct_delete_struct (s);
	return;
     }

   f = s->fields;
   while (n > 0)
     {
	SLang_Object_Type obj;

	if (-1 == SLang_pop (&obj))
	  break;

	SLang_free_object (&f->obj);
	f->obj = obj;

	f++;
	n--;
     }

   _SLstruct_delete_struct (s);
}

static void get_struct_field (char *name)
{
   (void) struct_sget (0, name);
}

static int is_struct_type (void)
{
   SLang_Object_Type obj;
   unsigned char type;
   int status;

   if (-1 == SLang_pop (&obj))
     return -1;

   type = obj.data_type;
   if (type == SLANG_STRUCT_TYPE)
     status = 1;
   else
     status = (NULL != _SLclass_get_class (type)->cl_struct_def);
   SLang_free_object (&obj);
   return status;
}

static SLang_Intrin_Fun_Type Struct_Table [] =
{
   MAKE_INTRINSIC_1("get_struct_field_names", get_struct_field_names, SLANG_VOID_TYPE, SLANG_STRUCT_TYPE),
   MAKE_INTRINSIC_1("get_struct_field", get_struct_field, SLANG_VOID_TYPE, SLANG_STRING_TYPE),
   MAKE_INTRINSIC_1("_push_struct_field_values", push_struct_fields, SLANG_INT_TYPE, SLANG_STRUCT_TYPE),
   MAKE_INTRINSIC_0("set_struct_field", struct_set_field, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("set_struct_fields", set_struct_fields, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("is_struct_type", is_struct_type, SLANG_INT_TYPE),
   /* MAKE_INTRINSIC_I("_create_struct", create_struct, SLANG_VOID_TYPE), */
   SLANG_END_INTRIN_FUN_TABLE
};

int _SLstruct_init (void)
{
   if ((-1 == SLadd_intrin_fun_table (Struct_Table, NULL))
       || (-1 == register_struct ()))
     return -1;

   return 0;
}

void _SLstruct_pop_args (int *np)
{
   SLang_Array_Type *at;
   int i, n;
   _SLang_Struct_Type **data;

   n = *np;

   if (n < 0)
     {
	SLang_Error = SL_INVALID_PARM;
	return;
     }

   data = (_SLang_Struct_Type **) SLmalloc ((n + 1) * sizeof (_SLang_Struct_Type *));
   if (data == NULL)
     {
	SLdo_pop_n (n);
	return;
     }

   memset ((char *)data, 0, n * sizeof (_SLang_Struct_Type *));

   i = n;
   while (i > 0)
     {
	_SLang_Struct_Type *s;
	_SLstruct_Field_Type *f;

	i--;

	if (NULL == (s = allocate_struct (1)))
	  goto return_error;

	data[i] = s;
	s->num_refs += 1;	       /* keeping a copy */

	f = s->fields;
	if (NULL == (f->name = SLang_create_slstring ("value")))
	  goto return_error;

	if (-1 == SLang_pop (&f->obj))
	  goto return_error;
     }

   if (NULL == (at = SLang_create_array (SLANG_STRUCT_TYPE, 0,
					 (VOID_STAR) data, &n, 1)))
     goto return_error;

   (void) SLang_push_array (at, 1);
   return;

   return_error:
   for (i = 0; i < n; i++)
     {
	_SLang_Struct_Type *s;

	s = data[i];
	if (s != NULL)
	  _SLstruct_delete_struct (s);
     }

   SLfree ((char *) data);
}

void _SLstruct_push_args (SLang_Array_Type *at)
{
   _SLang_Struct_Type **sp;
   unsigned int num;

   if (at->data_type != SLANG_STRUCT_TYPE)
     {
	SLang_Error = SL_TYPE_MISMATCH;
	return;
     }

   sp = (_SLang_Struct_Type **) at->data;
   num = at->num_elements;

   while ((SLang_Error == 0) && (num > 0))
     {
	_SLang_Struct_Type *s;

	num--;
	if (NULL == (s = *sp++))
	  {
	     SLang_push_null ();
	     continue;
	  }

	/* I should check to see if the value field is present, but... */
	(void) _SLpush_slang_obj (&s->fields->obj);
     }
}

/* C structures */
static _SLstruct_Field_Type *find_field_via_strcmp (_SLang_Struct_Type *s, char *name)
{
   _SLstruct_Field_Type *f, *fmax;

   f = s->fields;
   fmax = f + s->nfields;

   while (f < fmax)
     {
	if (0 == strcmp (name, f->name))
	  return f;

	f++;
     }
   return NULL;
}

static void free_cstruct_field (SLang_CStruct_Field_Type *cfield, VOID_STAR cs)
{
   SLang_Class_Type *cl;

   if ((cfield->read_only == 0) 
       && (NULL != (cl = _SLclass_get_class (cfield->type))))
     _SLarray_free_array_elements (cl, (VOID_STAR)((char*)cs + cfield->offset), 1);
}

void SLang_free_cstruct (VOID_STAR cs, SLang_CStruct_Field_Type *cfields)
{
   if ((cs == NULL) || (cfields == NULL))
     return;

   while (cfields->field_name != NULL)
     {
	free_cstruct_field (cfields, cs);
	cfields++;
     }
}

int SLang_pop_cstruct (VOID_STAR cs, SLang_CStruct_Field_Type *cfields)
{
   _SLang_Struct_Type *s;
   SLang_CStruct_Field_Type *cfield;
   char *field_name;
   char *cs_addr;

   if ((cfields == NULL) || (cs == NULL))
     return -1;

   if (-1 == _SLang_pop_struct (&s))
     return -1;

   cfield = cfields;
   cs_addr = (char *) cs;

   while (NULL != (field_name = cfield->field_name))
     {
	_SLstruct_Field_Type *f;
	SLang_Class_Type *cl;

	if ((cfield->read_only == 0)
	    && ((NULL == (f = pop_field (s, field_name, find_field_via_strcmp)))
		|| (-1 == _SLpush_slang_obj (&f->obj))
		|| (NULL == (cl = _SLclass_get_class (cfield->type)))
		|| (-1 == (*cl->cl_apop)(cfield->type, (VOID_STAR) (cs_addr + cfield->offset)))))
	  goto return_error;

	cfield++;
     }

   _SLstruct_delete_struct (s);
   return 0;

   return_error:
   while (cfield != cfields)
     {
	free_cstruct_field (cfield, cs);
	cfield--;
     }
   _SLstruct_delete_struct (s);
   return -1;
}

static _SLang_Struct_Type *create_cstruct (VOID_STAR cs, SLang_CStruct_Field_Type *cfields)
{
   unsigned int i, n;
   _SLang_Struct_Type *s;
   SLang_CStruct_Field_Type *cfield;
   char **field_names;
   VOID_STAR *field_values;
   SLtype *field_types;

   if ((cs == NULL) || (cfields == NULL))
     return NULL;

   cfield = cfields;
   while (cfield->field_name != NULL)
     cfield++;
   n = cfield - cfields;
   if (n == 0)
     {
	SLang_verror (SL_APPLICATION_ERROR, "C structure has no fields");
	return NULL;
     }
   
   s = NULL;
   field_types = NULL;
   field_values = NULL;
   if ((NULL == (field_names = (char **) SLmalloc (n*sizeof (char *))))
       || (NULL == (field_types = (SLtype *)SLmalloc (n*sizeof(SLtype))))
       || (NULL == (field_values = (VOID_STAR *)SLmalloc (n*sizeof(VOID_STAR)))))
     goto return_error;

   for (i = 0; i < n; i++)
     {
	cfield = cfields + i;
	field_names[i] = cfield->field_name;
	field_types[i] = cfield->type;
	field_values[i] = (VOID_STAR)((char *)cs + cfield->offset);
     }

   s = create_struct (n, field_names, field_types, field_values);
   /* drop */

   return_error:
   SLfree ((char *) field_values);
   SLfree ((char *) field_types);
   SLfree ((char *) field_names);

   return s;
}
   
int SLang_push_cstruct (VOID_STAR cs, SLang_CStruct_Field_Type *cfields)
{
   _SLang_Struct_Type *s;
   
   if (NULL == (s = create_cstruct (cs, cfields)))
     return -1;
   
   if (0 == _SLang_push_struct (s))
     return 0;
   
   _SLstruct_delete_struct (s);
   return -1;
}

int SLang_assign_cstruct_to_ref (SLang_Ref_Type *ref, VOID_STAR cs, SLang_CStruct_Field_Type *cfields)
{
   _SLang_Struct_Type *s;

   if (NULL == (s = create_cstruct (cs, cfields)))
     return -1;

   if (0 == SLang_assign_to_ref (ref, SLANG_STRUCT_TYPE, (VOID_STAR) &s))
     return 0;

   _SLstruct_delete_struct (s);
   return -1;
}

