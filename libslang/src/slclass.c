/* User defined objects */
/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */
#include "slinclud.h"

/* #define SL_APP_WANTS_FOREACH */
#include "slang.h"
#include "_slang.h"

static SLang_Class_Type *Registered_Types[256];
SLang_Class_Type *_SLclass_get_class (unsigned char type)
{
   SLang_Class_Type *cl;

   cl = Registered_Types [type];
   if (cl == NULL)
     SLang_exit_error ("Application error: Type %d not registered", (int) type);

   return cl;
}

int SLclass_is_class_defined (unsigned char type)
{
   return (NULL != Registered_Types[type]);
}

VOID_STAR _SLclass_get_ptr_to_value (SLang_Class_Type *cl,
				     SLang_Object_Type *obj)
{
   VOID_STAR p;

   switch (cl->cl_class_type)
     {
      case SLANG_CLASS_TYPE_MMT:
      case SLANG_CLASS_TYPE_PTR:
      case SLANG_CLASS_TYPE_SCALAR:
	p = (VOID_STAR) &obj->v;
	break;

      case SLANG_CLASS_TYPE_VECTOR:
	p = obj->v.ptr_val;
	break;

      default:
	p = NULL;
     }
   return p;
}

char *SLclass_get_datatype_name (unsigned char stype)
{
   SLang_Class_Type *cl;

   cl = _SLclass_get_class (stype);
   return cl->cl_name;
}

static int method_undefined_error (unsigned char type, char *method, char *name)
{
   if (name == NULL) name = SLclass_get_datatype_name (type);

   SLang_verror (SL_TYPE_MISMATCH, "%s method not defined for %s",
		 method, name);
   return -1;
}

static int
scalar_vector_bin_op_result (int op, unsigned char a, unsigned char b,
			     unsigned char *c)
{
   (void) a; (void) b;
   switch (op)
     {
      case SLANG_NE:
      case SLANG_EQ:
	*c = SLANG_INT_TYPE;
	return 1;
     }
   return 0;
}

static int
scalar_vector_bin_op (int op,
		      unsigned char a_type, VOID_STAR ap, unsigned int na,
		      unsigned char b_type, VOID_STAR bp, unsigned int nb,
		      VOID_STAR cp)
{
   int *c;
   char *a, *b;
   unsigned int da, db;
   unsigned int n, n_max;
   unsigned int data_type_len;
   SLang_Class_Type *cl;

   (void) b_type;
   cl = _SLclass_get_class (a_type);

   data_type_len = cl->cl_sizeof_type;

   a = (char *) ap;
   b = (char *) bp;
   c = (int *) cp;

   if (na == 1) da = 0; else da = data_type_len;
   if (nb == 1) db = 0; else db = data_type_len;
   if (na > nb) n_max = na; else n_max = nb;

   switch (op)
     {
      default:
	return 0;

      case SLANG_NE:
	for (n = 0; n < n_max; n++)
	  {
	     c[n] = (0 != SLMEMCMP(a, b, data_type_len));
	     a += da; b += db;
	  }
	break;

      case SLANG_EQ:
	for (n = 0; n < n_max; n++)
	  {
	     c[n] = (0 == SLMEMCMP(a, b, data_type_len));
	     a += da; b += db;
	  }
	break;
     }
   return 1;
}

static int scalar_fread (unsigned char type, FILE *fp, VOID_STAR ptr,
			 unsigned int desired, unsigned int *actual)
{
   unsigned int n;

   n = fread ((char *) ptr, _SLclass_get_class (type)->cl_sizeof_type,
	      desired, fp);
   *actual = n;
   return 0;
}

static int scalar_fwrite (unsigned char type, FILE *fp, VOID_STAR ptr,
			  unsigned int desired, unsigned int *actual)
{
   unsigned int n;

   n = fwrite ((char *) ptr, _SLclass_get_class (type)->cl_sizeof_type,
	       desired, fp);
   *actual = n;
   return 0;
}

static int vector_apush (unsigned char type, VOID_STAR ptr)
{
   SLang_Class_Type *cl;

   cl = _SLclass_get_class (type);
   return (*cl->cl_push)(type, (VOID_STAR) &ptr);
}

static int vector_apop (unsigned char type, VOID_STAR ptr)
{
   SLang_Class_Type *cl;

   cl = _SLclass_get_class (type);
   return (*cl->cl_pop)(type, (VOID_STAR) &ptr);
}

static int default_push_mmt (unsigned char type_unused, VOID_STAR ptr)
{
   SLang_MMT_Type *ref;

   (void) type_unused;
   ref = *(SLang_MMT_Type **) ptr;
   return SLang_push_mmt (ref);
}

static void default_destroy_simple (unsigned char type_unused, VOID_STAR ptr_unused)
{
   (void) type_unused;
   (void) ptr_unused;
}

static void default_destroy_user (unsigned char type, VOID_STAR ptr)
{
   (void) type;
   SLang_free_mmt (*(SLang_MMT_Type **) ptr);
}

static int default_pop (unsigned char type, VOID_STAR ptr)
{
   return SLclass_pop_ptr_obj (type, (VOID_STAR *) ptr);
}

static int default_datatype_deref (unsigned char type)
{
   return method_undefined_error (type, "datatype_deref", NULL);
}

static int default_acopy (unsigned char type, VOID_STAR from, VOID_STAR to)
{
   SLang_Class_Type *cl;

   cl = _SLclass_get_class (type);
   if (-1 == (*cl->cl_apush) (type, from))
     return -1;
   return (*cl->cl_apop) (type, to);
}

static int scalar_acopy (SLtype type, VOID_STAR from, VOID_STAR to)
{
   memcpy ((char *)to, (char *)from, _SLclass_get_class (type)->cl_sizeof_type);
   return 0;
}

int SLclass_dup_object (SLtype type, VOID_STAR from, VOID_STAR to)
{
   SLang_Class_Type *cl = _SLclass_get_class (type);
   return cl->cl_acopy (type, from, to);
}

static int default_dereference_object (unsigned char type, VOID_STAR ptr)
{
   (void) ptr;
   return method_undefined_error (type, "dereference", NULL);
}

static char *default_string (unsigned char stype, VOID_STAR v)
{
   char buf [256];
   char *s;
#if SLANG_HAS_COMPLEX
   double *cplx;
#endif
   s = buf;

   switch (stype)
     {
      case SLANG_STRING_TYPE:
	s = *(char **) v;
	break;

      case SLANG_NULL_TYPE:
	s = "NULL";
	break;

      case SLANG_DATATYPE_TYPE:
	s = SLclass_get_datatype_name ((unsigned char) *(int *)v);
	break;

#if SLANG_HAS_COMPLEX
      case SLANG_COMPLEX_TYPE:
	cplx = *(double **) v;
	if (cplx[1] < 0)
	  sprintf (s, "(%g - %gi)", cplx [0], -cplx [1]);
	else
	  sprintf (s, "(%g + %gi)", cplx [0], cplx [1]);
	break;
#endif
      default:
	s = SLclass_get_datatype_name (stype);
     }

   return SLmake_string (s);
}

static int
use_cmp_bin_op_result (int op, unsigned char a, unsigned char b,
		       unsigned char *c)
{
   if (a != b)
     return 0;
   switch (op)
     {
      case SLANG_NE:
      case SLANG_EQ:
      case SLANG_LT:
      case SLANG_LE:
      case SLANG_GT:
      case SLANG_GE:
	*c = SLANG_INT_TYPE;
	return 1;
     }
   return 0;
}

static int
use_cmp_bin_op (int op,
		unsigned char a_type, VOID_STAR ap, unsigned int na,
		unsigned char b_type, VOID_STAR bp, unsigned int nb,
		VOID_STAR cp)
{
   int *c;
   char *a, *b;
   unsigned int da, db;
   unsigned int n, n_max;
   unsigned int data_type_len;
   SLang_Class_Type *cl;
   int (*cmp)(unsigned char, VOID_STAR, VOID_STAR, int *);

   (void) b_type;
   cl = _SLclass_get_class (a_type);
   cmp = cl->cl_cmp;
   data_type_len = cl->cl_sizeof_type;

   a = (char *) ap;
   b = (char *) bp;
   c = (int *) cp;

   if (na == 1) da = 0; else da = data_type_len;
   if (nb == 1) db = 0; else db = data_type_len;
   if (na > nb) n_max = na; else n_max = nb;

   switch (op)
     {
	int result;
	
      default:
	return 0;

      case SLANG_NE:
	for (n = 0; n < n_max; n++)
	  {
	     if (-1 == (*cmp) (a_type, (VOID_STAR)a, (VOID_STAR)b, &result))
	       return -1;
	     c[n] = (result != 0);
	     a += da; b += db;
	  }
	break;

      case SLANG_EQ:
	for (n = 0; n < n_max; n++)
	  {
	     if (-1 == (*cmp) (a_type, (VOID_STAR)a, (VOID_STAR)b, &result))
	       return -1;
	     c[n] = (result == 0);
	     a += da; b += db;
	  }
	break;

      case SLANG_GT:
	for (n = 0; n < n_max; n++)
	  {
	     if (-1 == (*cmp) (a_type, (VOID_STAR)a, (VOID_STAR)b, &result))
	       return -1;
	     c[n] = (result > 0);
	     a += da; b += db;
	  }
	break;
      case SLANG_GE:
	for (n = 0; n < n_max; n++)
	  {
	     if (-1 == (*cmp) (a_type, (VOID_STAR)a, (VOID_STAR)b, &result))
	       return -1;
	     c[n] = (result >= 0);
	     a += da; b += db;
	  }
	break;
      case SLANG_LT:
	for (n = 0; n < n_max; n++)
	  {
	     if (-1 == (*cmp) (a_type, (VOID_STAR)a, (VOID_STAR)b, &result))
	       return -1;
	     c[n] = (result < 0);
	     a += da; b += db;
	  }
	break;
      case SLANG_LE:
	for (n = 0; n < n_max; n++)
	  {
	     if (-1 == (*cmp) (a_type, (VOID_STAR)a, (VOID_STAR)b, &result))
	       return -1;
	     c[n] = (result <= 0);
	     a += da; b += db;
	  }
	break;
     }
   return 1;
}


int SLclass_get_class_id (SLang_Class_Type *cl)
{
   if (cl == NULL)
     return -1;
   return (int) cl->cl_data_type;
}

SLang_Class_Type *SLclass_allocate_class (char *name)
{
   SLang_Class_Type *cl;
   unsigned int i;

   for (i = 0; i < 256; i++)
     {
	cl = Registered_Types [i];
	if ((cl != NULL)
	    && (0 == strcmp (cl->cl_name, name)))
	  {
	     SLang_verror (SL_DUPLICATE_DEFINITION, "Type name %s already exists", name);
	     return NULL;
	  }
     }

   cl = (SLang_Class_Type *) SLmalloc (sizeof (SLang_Class_Type));
   if (cl == NULL) return NULL;

   SLMEMSET ((char *) cl, 0, sizeof (SLang_Class_Type));

   if (NULL == (cl->cl_name = SLang_create_slstring (name)))
     {
	SLfree ((char *) cl);
	return NULL;
     }

   return cl;
}

static int DataType_Ids [256];

int SLang_push_datatype (unsigned char data_type)
{
   /* This data type could be a copy of another type, e.g., short and
    * int if they are the same size (Int16 == Short).  So, make sure
    * we push the original and not the copy. 
    */
   data_type = _SLclass_get_class (data_type)->cl_data_type;
   return SLclass_push_int_obj (SLANG_DATATYPE_TYPE, (int) data_type);
}

static int datatype_deref (unsigned char type, VOID_STAR ptr)
{
   SLang_Class_Type *cl;
   int status;

   /* The parser generated code for this as if a function call were to be
    * made.  However, we are calling the deref object routine
    * instead of the function call.  So, I must simulate the function call.
    */
   if (-1 == _SL_increment_frame_pointer ())
     return -1;

   type = (unsigned char) *(int *) ptr;
   cl = _SLclass_get_class (type);
   status = (*cl->cl_datatype_deref) (type);

   (void) _SL_decrement_frame_pointer ();
   return status;
}

static int datatype_push (unsigned char type_unused, VOID_STAR ptr)
{
   (void) type_unused;
   return SLang_push_datatype (*(int *) ptr);
}

int SLang_pop_datatype (unsigned char *type)
{
   int i;

   if (-1 == SLclass_pop_int_obj (SLANG_DATATYPE_TYPE, &i))
     return -1;

   *type = (unsigned char) i;
   return 0;
}

static int datatype_pop (unsigned char type, VOID_STAR ptr)
{
   if (-1 == SLang_pop_datatype (&type))
     return -1;

   *(int *) ptr = type;
   return 0;
}

int _SLclass_init (void)
{
   SLang_Class_Type *cl;

   /* First initialize the container classes.  This is so binary operations
    * added later will work with them.
    */
   if (-1 == _SLarray_init_slarray ())
     return -1;

   /* DataType_Type */
   if (NULL == (cl = SLclass_allocate_class ("DataType_Type")))
     return -1;
   cl->cl_pop = datatype_pop;
   cl->cl_push = datatype_push;
   cl->cl_dereference = datatype_deref;
   if (-1 == SLclass_register_class (cl, SLANG_DATATYPE_TYPE, sizeof(int),
				     SLANG_CLASS_TYPE_SCALAR))
     return -1;

   return 0;
}

static int register_new_datatype (char *name, unsigned char type)
{
   DataType_Ids [type] = type;
   return SLadd_intrinsic_variable (name, (VOID_STAR) (DataType_Ids + type),
				    SLANG_DATATYPE_TYPE, 1);
}

int SLclass_create_synonym (char *name, unsigned char type)
{
   if (NULL == _SLclass_get_class (type))
     return -1;

   return register_new_datatype (name, type);
}

int _SLclass_copy_class (unsigned char to, unsigned char from)
{
   SLang_Class_Type *cl = _SLclass_get_class (from);

   if (Registered_Types[to] != NULL)
     SLang_exit_error ("Application error: Class already exists");

   Registered_Types[to] = cl;

#if _SLANG_OPTIMIZE_FOR_SPEED
   if (to != SLANG_UNDEFINED_TYPE)
     _SLang_set_class_type(to, cl->cl_class_type);
#endif
   return 0;
}

int SLclass_register_class (SLang_Class_Type *cl, unsigned char type, unsigned int type_size, unsigned char class_type)
{
   char *name;
   unsigned int i;
   int can_binop = 1;		       /* scalar_vector_bin_op should work
					* for all data types.
					*/

   if (type == SLANG_VOID_TYPE) for (i = _SLANG_MIN_UNUSED_TYPE; i < 256; i++)
     {
	if (Registered_Types[i] == NULL)
	  {
	     type = (unsigned char) i;
	     break;
	  }
     }

   if ((NULL != Registered_Types [type])
       || (type == SLANG_VOID_TYPE))
     {
	SLang_verror (SL_APPLICATION_ERROR, "Class type %d already in use", (int) type);
	return -1;
     }

   cl->cl_data_type = type;
   cl->cl_class_type = class_type;
   name = cl->cl_name;

   switch (class_type)
     {
      case SLANG_CLASS_TYPE_MMT:
	if (cl->cl_push == NULL) cl->cl_push = default_push_mmt;
	if (cl->cl_destroy == NULL)
	  return method_undefined_error (type, "destroy", name);
	cl->cl_user_destroy_fun = cl->cl_destroy;
	cl->cl_destroy = default_destroy_user;
	type_size = sizeof (VOID_STAR);
	break;

      case SLANG_CLASS_TYPE_SCALAR:
	if (cl->cl_destroy == NULL) cl->cl_destroy = default_destroy_simple;
	if ((type_size == 0)
	    || (type_size > sizeof (_SL_Object_Union_Type)))
	  {
	     SLang_verror (SL_INVALID_PARM,
			   "Type size for %s not appropriate for SCALAR type",
			   name);
	     return -1;
	  }
	if (cl->cl_pop == NULL)
	  return method_undefined_error (type, "pop", name);
	if (cl->cl_fread == NULL) cl->cl_fread = scalar_fread;
	if (cl->cl_fwrite == NULL) cl->cl_fwrite = scalar_fwrite;
	if (cl->cl_acopy == NULL) cl->cl_acopy = scalar_acopy;

	can_binop = 1;
	break;

      case SLANG_CLASS_TYPE_PTR:
	if (cl->cl_destroy == NULL)
	  return method_undefined_error (type, "destroy", name);
	type_size = sizeof (VOID_STAR);
	break;

      case SLANG_CLASS_TYPE_VECTOR:
	if (cl->cl_destroy == NULL)
	  return method_undefined_error (type, "destroy", name);
	if (cl->cl_pop == NULL)
	  return method_undefined_error (type, "pop", name);
	cl->cl_apop = vector_apop;
	cl->cl_apush = vector_apush;
	cl->cl_adestroy = default_destroy_simple;
	if (cl->cl_fread == NULL) cl->cl_fread = scalar_fread;
	if (cl->cl_fwrite == NULL) cl->cl_fwrite = scalar_fwrite;
	if (cl->cl_acopy == NULL) cl->cl_acopy = scalar_acopy;
	can_binop = 1;
	break;

      default:
	SLang_verror (SL_INVALID_PARM, "%s: unknown class type (%d)", name, class_type);
	return -1;
     }

#if _SLANG_OPTIMIZE_FOR_SPEED
   if (type != SLANG_UNDEFINED_TYPE)
     _SLang_set_class_type (type, class_type);
#endif

   if (type_size == 0)
     {
	SLang_verror (SL_INVALID_PARM, "type size must be non-zero for %s", name);
	return -1;
     }

   if (cl->cl_string == NULL) cl->cl_string = default_string;
   if (cl->cl_acopy == NULL) cl->cl_acopy = default_acopy;
   if (cl->cl_datatype_deref == NULL) cl->cl_datatype_deref = default_datatype_deref;

   if (cl->cl_pop == NULL) cl->cl_pop = default_pop;

   if (cl->cl_push == NULL)
     return method_undefined_error (type, "push", name);

   if (cl->cl_byte_code_destroy == NULL)
     cl->cl_byte_code_destroy = cl->cl_destroy;
   if (cl->cl_push_literal == NULL)
     cl->cl_push_literal = cl->cl_push;

   if (cl->cl_dereference == NULL)
     cl->cl_dereference = default_dereference_object;

   if (cl->cl_apop == NULL) cl->cl_apop = cl->cl_pop;
   if (cl->cl_apush == NULL) cl->cl_apush = cl->cl_push;
   if (cl->cl_adestroy == NULL) cl->cl_adestroy = cl->cl_destroy;
   if (cl->cl_push_intrinsic == NULL) cl->cl_push_intrinsic = cl->cl_push;

   if ((cl->cl_foreach == NULL)
       || (cl->cl_foreach_open == NULL)
       || (cl->cl_foreach_close == NULL))
     {
	cl->cl_foreach = _SLarray_cl_foreach;
	cl->cl_foreach_open = _SLarray_cl_foreach_open;
	cl->cl_foreach_close = _SLarray_cl_foreach_close;
     }

   cl->cl_sizeof_type = type_size;

   if (NULL == (cl->cl_transfer_buf = (VOID_STAR) SLmalloc (type_size)))
     return -1;

   Registered_Types[type] = cl;

   if (-1 == register_new_datatype (name, type))
     return -1;

   if (cl->cl_cmp != NULL)
     {
	if (-1 == SLclass_add_binary_op (type, type, use_cmp_bin_op, use_cmp_bin_op_result))
	  return -1;
     }
   else if (can_binop
	    && (-1 == SLclass_add_binary_op (type, type, scalar_vector_bin_op, scalar_vector_bin_op_result)))
     return -1;

   cl->cl_anytype_typecast = _SLanytype_typecast;

   return 0;
}

int SLclass_add_math_op (unsigned char type,
			 int (*handler)(int,
					unsigned char, VOID_STAR, unsigned int,
					VOID_STAR),
			 int (*result) (int, unsigned char, unsigned char *))
{
   SLang_Class_Type *cl = _SLclass_get_class (type);

   cl->cl_math_op = handler;
   cl->cl_math_op_result_type = result;
   return 0;
}

int SLclass_add_binary_op (unsigned char a, unsigned char b,
			   int (*f) (int,
				     unsigned char, VOID_STAR, unsigned int,
				     unsigned char, VOID_STAR, unsigned int,
				     VOID_STAR),
			   int (*r) (int, unsigned char, unsigned char, unsigned char *))
{
   SLang_Class_Type *cl;
   SL_OOBinary_Type *ab;

   if ((f == NULL) || (r == NULL))
     {
	SLang_verror (SL_INVALID_PARM, "SLclass_add_binary_op");
	return -1;
     }

   cl = _SLclass_get_class (a);
   (void) _SLclass_get_class (b);

   if (NULL == (ab = (SL_OOBinary_Type *) SLmalloc (sizeof(SL_OOBinary_Type))))
     return -1;

   ab->data_type = b;
   ab->binary_function = f;
   ab->binary_result = r;
   ab->next = cl->cl_binary_ops;
   cl->cl_binary_ops = ab;

   if ((a != SLANG_ARRAY_TYPE)
       && (b != SLANG_ARRAY_TYPE))
     {
	if ((-1 == _SLarray_add_bin_op (a))
	    || (-1 == _SLarray_add_bin_op (b)))
	  return -1;
     }

   return 0;
}

int SLclass_add_unary_op (unsigned char type,
			  int (*f)(int,
				   unsigned char, VOID_STAR, unsigned int,
				   VOID_STAR),
			  int (*r)(int, unsigned char, unsigned char *))
{
   SLang_Class_Type *cl;

   cl = _SLclass_get_class (type);
   if ((f == NULL) || (r == NULL))
     {
	SLang_verror (SL_INVALID_PARM, "SLclass_add_unary_op");
	return -1;
     }

   cl->cl_unary_op = f;
   cl->cl_unary_op_result_type = r;

   return 0;
}

int SLclass_add_app_unary_op (unsigned char type,
			      int (*f)(int,
				       unsigned char, VOID_STAR, unsigned int,
				       VOID_STAR),
			      int (*r)(int, unsigned char, unsigned char *))
{
   SLang_Class_Type *cl;

   cl = _SLclass_get_class (type);
   if ((f == NULL) || (r == NULL))
     {
	SLang_verror (SL_INVALID_PARM, "SLclass_add_app_unary_op");
	return -1;
     }

   cl->cl_app_unary_op = f;
   cl->cl_app_unary_op_result_type = r;

   return 0;
}

int SLclass_set_pop_function (SLang_Class_Type *cl, int (*f)(unsigned char, VOID_STAR))
{
   if (cl == NULL) return -1;
   cl->cl_pop = f;

   return 0;
}

int SLclass_set_push_function (SLang_Class_Type *cl, int (*f)(SLtype, VOID_STAR))
{
   if (cl == NULL) return -1;
   cl->cl_push = f;

   return 0;
}

int SLclass_set_apush_function (SLang_Class_Type *cl, int (*f)(SLtype, VOID_STAR))
{
   if (cl == NULL) return -1;
   cl->cl_apush = f;

   return 0;
}

int SLclass_set_acopy_function (SLang_Class_Type *cl, int (*f)(SLtype, VOID_STAR, VOID_STAR))
{
   if (cl == NULL) return -1;
   cl->cl_acopy = f;

   return 0;
}

int SLclass_set_string_function (SLang_Class_Type *cl, char *(*f)(unsigned char, VOID_STAR))
{
   if (cl == NULL) return -1;

   cl->cl_string = f;
   return 0;
}

int SLclass_set_destroy_function (SLang_Class_Type *cl, void (*f)(unsigned char, VOID_STAR))
{
   if (cl == NULL) return -1;

   cl->cl_destroy = f;
   return 0;
}

int SLclass_set_sget_function (SLang_Class_Type *cl, int (*f)(unsigned char, char *))
{
   if (cl == NULL) return -1;
   cl->cl_sget = f;
   return 0;
}

int SLclass_set_sput_function (SLang_Class_Type *cl, int (*f)(unsigned char, char *))
{
   if (cl == NULL) return -1;
   cl->cl_sput = f;
   return 0;
}

int SLclass_set_aget_function (SLang_Class_Type *cl, int (*f)(unsigned char, unsigned int))
{
   if (cl == NULL) return -1;
   cl->cl_aget = f;
   return 0;
}

int SLclass_set_aput_function (SLang_Class_Type *cl, int (*f)(unsigned char, unsigned int))
{
   if (cl == NULL) return -1;
   cl->cl_aput = f;
   return 0;
}

int SLclass_set_anew_function (SLang_Class_Type *cl, int (*f)(unsigned char, unsigned int))
{
   if (cl == NULL) return -1;
   cl->cl_anew = f;
   return 0;
}

/* Misc */
void _SLclass_type_mismatch_error (unsigned char a, unsigned char b)
{
   SLang_verror (SL_TYPE_MISMATCH, "Expecting %s, found %s",
		 SLclass_get_datatype_name (a),
		 SLclass_get_datatype_name (b));
}

/* */

static int null_binary_fun (int op,
			    unsigned char a, VOID_STAR ap, unsigned int na,
			    unsigned char b, VOID_STAR bp, unsigned int nb,
			    VOID_STAR cp)
{
   int *ic;
   unsigned int i;
   int c;

   (void) ap; (void) bp;

   switch (op)
     {
      case SLANG_EQ:
	c = (a == b);
	break;

      case SLANG_NE:
	c = (a != b);
	break;

      default:
	return 0;
     }

   if (na > nb) nb = na;
   ic = (int *) cp;
   for (i = 0; i < nb; i++)
     ic[i] = c;

   return 1;
}

static char *get_binary_op_string (int op)
{
   static char *ops[SLANG_MOD] =
     {
	"+", "=", "*", "/", "==", "!=", ">", ">=", "<", "<=", "^",
	"or", "and", "&", "|", "xor", "shl", "shr", "mod"
     };

   if ((op > SLANG_MOD) || (op <= 0))
     return "- ?? -";		       /* Note: -??- is a trigraph (sigh) */
   return ops[op - 1];
}

int (*_SLclass_get_binary_fun (int op,
			       SLang_Class_Type *a_cl, SLang_Class_Type *b_cl,
			       SLang_Class_Type **c_cl, int do_error))
(int,
 unsigned char, VOID_STAR, unsigned int,
 unsigned char, VOID_STAR, unsigned int,
 VOID_STAR)
{
   SL_OOBinary_Type *bt;
   unsigned char a, b, c;

   a = a_cl->cl_data_type;
   b = b_cl->cl_data_type;

   if ((a == SLANG_NULL_TYPE) || (b == SLANG_NULL_TYPE))
     {
	*c_cl = _SLclass_get_class (SLANG_INT_TYPE);
	return null_binary_fun;
     }

   bt = a_cl->cl_binary_ops;

   while (bt != NULL)
     {
	if (bt->data_type == b)
	  {
	     if (1 != (*bt->binary_result)(op, a, b, &c))
	       break;

	     if (c == a) *c_cl = a_cl;
	     else if (c == b) *c_cl = b_cl;
	     else *c_cl = _SLclass_get_class (c);

	     return bt->binary_function;
	  }

	bt = bt->next;
     }

   if (do_error)
     SLang_verror (SL_TYPE_MISMATCH, "%s %s %s is not possible",
		   a_cl->cl_name, get_binary_op_string (op), b_cl->cl_name);

   *c_cl = NULL;
   return NULL;
}

int (*_SLclass_get_unary_fun (int op,
			      SLang_Class_Type *a_cl,
			      SLang_Class_Type **b_cl,
			      int utype))
(int, unsigned char, VOID_STAR, unsigned int, VOID_STAR)
{
   int (*f)(int, unsigned char, VOID_STAR, unsigned int, VOID_STAR);
   int (*r)(int, unsigned char, unsigned char *);
   unsigned char a;
   unsigned char b;

   switch (utype)
     {
      case _SLANG_BC_UNARY:
	f = a_cl->cl_unary_op;
	r = a_cl->cl_unary_op_result_type;
	break;

      case _SLANG_BC_MATH_UNARY:
	f = a_cl->cl_math_op;
	r = a_cl->cl_math_op_result_type;
	break;

      case _SLANG_BC_APP_UNARY:
	f = a_cl->cl_app_unary_op;
	r = a_cl->cl_app_unary_op_result_type;
	break;

      default:
	f = NULL;
	r = NULL;
     }

   a = a_cl->cl_data_type;
   if ((f != NULL) && (r != NULL) && (1 == (*r) (op, a, &b)))
     {
	if (a == b)
	  *b_cl = a_cl;
	else
	  *b_cl = _SLclass_get_class (b);
	return f;
     }

   SLang_verror (SL_TYPE_MISMATCH, "undefined unary operation/function on %s",
		 a_cl->cl_name);

   *b_cl = NULL;

   return NULL;
}

int
SLclass_typecast (unsigned char to_type, int is_implicit, int allow_array)
{
   unsigned char from_type;
   SLang_Class_Type *cl_to, *cl_from;
   SLang_Object_Type obj;
   VOID_STAR ap;
   VOID_STAR bp;
   int status;

   if (-1 == SLang_pop (&obj))
     return -1;

   from_type = obj.data_type;
   if (from_type == to_type)
     {
	SLang_push (&obj);
	return 0;
     }

   cl_from = _SLclass_get_class (from_type);
   
   /* Since the typecast functions are designed to work on arrays, 
    * get the pointer to the value instead of just &obj.v.
    */
   ap = _SLclass_get_ptr_to_value (cl_from, &obj);

   if ((from_type == SLANG_ARRAY_TYPE)
       && (allow_array || (to_type != SLANG_ANY_TYPE)))
     {
	if (allow_array == 0)
	  goto return_error;

	cl_to = _SLclass_get_class (SLANG_ARRAY_TYPE);
	bp = cl_to->cl_transfer_buf;
	status = _SLarray_typecast (from_type, ap, 1, to_type, bp, is_implicit);
     }
   else
     {
	int (*t) (unsigned char, VOID_STAR, unsigned int, unsigned char, VOID_STAR);

	if (NULL == (t = _SLclass_get_typecast (from_type, to_type, is_implicit)))
	  {
	     SLang_free_object (&obj);
	     return -1;
	  }

	cl_to = _SLclass_get_class (to_type);
	bp = cl_to->cl_transfer_buf;
	status = (*t) (from_type, ap, 1, to_type, bp);
     }

   if (1 == status)
     {
	/* AnyType apush will do a reference, which is undesirable here.
	 * So, to avoid that, perform push instead of apush.  Yes, this is
	 * an ugly hack.
	 */
	if (to_type == SLANG_ANY_TYPE)
	  status = (*cl_to->cl_push)(to_type, bp);
	else 
	  status = (*cl_to->cl_apush)(to_type, bp);
	
	if (status == -1)
	  {
	     (*cl_to->cl_adestroy) (to_type, bp);
	     SLang_free_object (&obj);
	     return -1;
	  }

	/* cl_apush will push a copy, so destry this one */
	(*cl_to->cl_adestroy) (to_type, bp);
	SLang_free_object (&obj);
	return 0;
     }

   return_error:

   SLang_verror (SL_TYPE_MISMATCH, "Unable to typecast %s to %s",
		 cl_from->cl_name,
		 SLclass_get_datatype_name (to_type));
   SLang_free_object (&obj);
   return -1;
}

int (*_SLclass_get_typecast (unsigned char from, unsigned char to, int is_implicit))
(unsigned char, VOID_STAR, unsigned int,
 unsigned char, VOID_STAR)
{
   SL_Typecast_Type *t;
   SLang_Class_Type *cl_from;

   cl_from = _SLclass_get_class (from);

   t = cl_from->cl_typecast_funs;
   while (t != NULL)
     {
	if (t->data_type != to)
	  {
	     t = t->next;
	     continue;
	  }

	if (is_implicit && (t->allow_implicit == 0))
	  break;

	return t->typecast;
     }

   if (to == SLANG_ANY_TYPE)
     return _SLanytype_typecast;

   if ((is_implicit == 0)
       && (cl_from->cl_void_typecast != NULL))
     return cl_from->cl_void_typecast;

   SLang_verror (SL_TYPE_MISMATCH, "Unable to typecast %s to %s",
		 cl_from->cl_name,
		 SLclass_get_datatype_name (to));

   return NULL;
}

int
SLclass_add_typecast (unsigned char from, unsigned char to,
		      int (*f)_PROTO((unsigned char, VOID_STAR, unsigned int,
				      unsigned char, VOID_STAR)),
		      int allow_implicit)
{
   SL_Typecast_Type *t;
   SLang_Class_Type *cl;

   cl = _SLclass_get_class (from);
   if (to == SLANG_VOID_TYPE)
     {
	cl->cl_void_typecast = f;
	return 0;
     }

   (void) _SLclass_get_class (to);

   if (NULL == (t = (SL_Typecast_Type *) SLmalloc (sizeof (SL_Typecast_Type))))
     return -1;

   SLMEMSET((char *) t, 0, sizeof(SL_Typecast_Type));
   t->data_type = to;
   t->next = cl->cl_typecast_funs;
   t->typecast = f;
   t->allow_implicit = allow_implicit;

   cl->cl_typecast_funs = t;

   return 0;
}

SLang_MMT_Type *SLang_pop_mmt (unsigned char type) /*{{{*/
{
   SLang_MMT_Type *mmt;

   if (-1 == SLclass_pop_ptr_obj (type, (VOID_STAR *) &mmt))
     mmt = NULL;
   return mmt;

#if 0
   SLang_Object_Type obj;
   SLang_Class_Type *cl;

   if (_SLang_pop_object_of_type (type, &obj))
     return NULL;

   cl = _SLclass_get_class (type);
   if ((cl->cl_class_type == SLANG_CLASS_TYPE_MMT)
       && (obj.data_type == type))
     {
	return obj.v.ref;
     }

   _SLclass_type_mismatch_error (type, obj.data_type);
   SLang_free_object (&obj);
   return NULL;
#endif
}

/*}}}*/

int SLang_push_mmt (SLang_MMT_Type *ref) /*{{{*/
{
   if (ref == NULL)
     return SLang_push_null ();

   ref->count += 1;

   if (0 == SLclass_push_ptr_obj (ref->data_type, (VOID_STAR) ref))
     return 0;

   ref->count -= 1;
   return -1;
}

/*}}}*/

void SLang_inc_mmt (SLang_MMT_Type *ref)
{
   if (ref != NULL)
     ref->count += 1;
}

VOID_STAR SLang_object_from_mmt (SLang_MMT_Type *ref)
{
   if (ref == NULL)
     return NULL;

   return ref->user_data;
}

SLang_MMT_Type *SLang_create_mmt (unsigned char t, VOID_STAR p)
{
   SLang_MMT_Type *ref;

   (void) _SLclass_get_class (t);      /* check to see if it is registered */

   if (NULL == (ref = (SLang_MMT_Type *) SLmalloc (sizeof (SLang_MMT_Type))))
     return NULL;

   SLMEMSET ((char *) ref, 0, sizeof (SLang_MMT_Type));

   ref->data_type = t;
   ref->user_data = p;
   /* FIXME!!  To be consistent with other types, the reference count should 
    * be set to 1 here.  However, doing so will require other code changes
    * involving the use of MMTs.  For instance, SLang_free_mmt would have
    * to be called after every push of the MMT.
    */
   return ref;
}

void SLang_free_mmt (SLang_MMT_Type *ref)
{
   unsigned char type;
   SLang_Class_Type *cl;

   if (ref == NULL)
     return;

   /* This can be zero if SLang_create_mmt is called followed
    * by this routine before anything gets a chance to attach itself
    * to it.
    */
   if (ref->count > 1)
     {
	ref->count -= 1;
	return;
     }

   type = ref->data_type;
   cl = _SLclass_get_class (type);
   (*cl->cl_user_destroy_fun) (type, ref->user_data);
   SLfree ((char *)ref);
}

int SLang_push_value (unsigned char type, VOID_STAR v)
{
   SLang_Class_Type *cl;

   cl = _SLclass_get_class (type);
   return (*cl->cl_apush)(type, v);
}

int SLang_pop_value (unsigned char type, VOID_STAR v)
{
   SLang_Class_Type *cl;

   cl = _SLclass_get_class (type);
   return (*cl->cl_apop)(type, v);
}

void SLang_free_value (unsigned char type, VOID_STAR v)
{
   SLang_Class_Type *cl;

   cl = _SLclass_get_class (type);
   (*cl->cl_adestroy) (type, v);
}

/* These routines are very low-level and are designed for application data
 * types to access the stack from their push/pop methods.  The int and
 * pointer versions are in slang.c
 */
#if SLANG_HAS_FLOAT
int SLclass_push_float_obj (unsigned char type, float x)
{
   SLang_Object_Type obj;
   obj.data_type = type;
   obj.v.float_val = x;
   return SLang_push (&obj);
}
#endif

int SLclass_push_long_obj (unsigned char type, long x)
{
   SLang_Object_Type obj;
   obj.data_type = type;
   obj.v.long_val = x;
   return SLang_push (&obj);
}

int SLclass_push_short_obj (unsigned char type, short x)
{
   SLang_Object_Type obj;
   obj.data_type = type;
   obj.v.short_val = x;
   return SLang_push (&obj);
}

int SLclass_push_char_obj (unsigned char type, char x)
{
   SLang_Object_Type obj;
   obj.data_type = type;
   obj.v.char_val = x;
   return SLang_push (&obj);
}

#if SLANG_HAS_FLOAT
int SLclass_pop_double_obj (unsigned char type, double *x)
{
   SLang_Object_Type obj;

   if (-1 == _SLang_pop_object_of_type (type, &obj, 0))
     return -1;

   *x = obj.v.double_val;
   return 0;
}

int SLclass_pop_float_obj (unsigned char type, float *x)
{
   SLang_Object_Type obj;

   if (-1 == _SLang_pop_object_of_type (type, &obj, 0))
     return -1;

   *x = obj.v.float_val;
   return 0;
}
#endif

int SLclass_pop_long_obj (unsigned char type, long *x)
{
   SLang_Object_Type obj;

   if (-1 == _SLang_pop_object_of_type (type, &obj, 0))
     return -1;

   *x = obj.v.long_val;
   return 0;
}

int SLclass_pop_int_obj (unsigned char type, int *x)
{
   SLang_Object_Type obj;

   if (-1 == _SLang_pop_object_of_type (type, &obj, 0))
     return -1;

   *x = obj.v.int_val;
   return 0;
}

int SLclass_pop_short_obj (unsigned char type, short *x)
{
   SLang_Object_Type obj;

   if (-1 == _SLang_pop_object_of_type (type, &obj, 0))
     return -1;

   *x = obj.v.short_val;
   return 0;
}

int SLclass_pop_char_obj (unsigned char type, char *x)
{
   SLang_Object_Type obj;

   if (-1 == _SLang_pop_object_of_type (type, &obj, 0))
     return -1;

   *x = obj.v.char_val;
   return 0;
}

int SLclass_pop_ptr_obj (unsigned char type, VOID_STAR *s)
{
   SLang_Object_Type obj;

   if (-1 == _SLang_pop_object_of_type (type, &obj, 0))
     {
	*s = (VOID_STAR) NULL;
	return -1;
     }
   *s = obj.v.ptr_val;
   return 0;
}

