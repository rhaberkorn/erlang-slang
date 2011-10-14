/* Advanced array manipulation routines for S-Lang */
/* Copyright (c) 1998, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

static int next_transposed_index (int *dims, int *max_dims, unsigned int num_dims)
{
   int i;

   for (i = 0; i < (int) num_dims; i++)
     {
	int dims_i;

	dims_i = dims [i] + 1;
	if (dims_i != (int) max_dims [i])
	  {
	     dims [i] = dims_i;
	     return 0;
	  }
	dims [i] = 0;
     }

   return -1;
}

static SLang_Array_Type *allocate_transposed_array (SLang_Array_Type *at)
{
   unsigned int num_elements;
   SLang_Array_Type *bt;
   VOID_STAR b_data;

   num_elements = at->num_elements;
   b_data = (VOID_STAR) SLmalloc (at->sizeof_type * num_elements);
   if (b_data == NULL)
     return NULL;

   bt = SLang_create_array (at->data_type, 0, b_data, at->dims, 2);
   if (bt == NULL)
     {
	SLfree ((char *)b_data);
	return NULL;
     }

   bt->dims[1] = at->dims[0];
   bt->dims[0] = at->dims[1];

   return bt;
}

static int check_for_empty_array (char *fun, unsigned int num)
{
   if (num)
     return 0;
   
   SLang_verror (SL_INVALID_PARM, "%s: array is empty", fun);
   return -1;
}

/* -------------- FLOAT --------------------- */
#if SLANG_HAS_FLOAT
#define GENERIC_TYPE float
#define TRANSPOSE_2D_ARRAY transpose_floats
#define GENERIC_TYPE_A float
#define GENERIC_TYPE_B float
#define GENERIC_TYPE_C float
#define INNERPROD_FUNCTION innerprod_float_float
#if SLANG_HAS_COMPLEX
# define INNERPROD_COMPLEX_A innerprod_complex_float
# define INNERPROD_A_COMPLEX innerprod_float_complex
#endif
#define SUM_FUNCTION sum_floats
#define SUM_RESULT_TYPE float
#define CUMSUM_FUNCTION cumsum_floats
#define CUMSUM_RESULT_TYPE float
#define MIN_FUNCTION min_floats
#define MAX_FUNCTION max_floats
#include "slarrfun.inc"

/* -------------- DOUBLE --------------------- */
#define GENERIC_TYPE double
#define TRANSPOSE_2D_ARRAY transpose_doubles
#define GENERIC_TYPE_A double
#define GENERIC_TYPE_B double
#define GENERIC_TYPE_C double
#define INNERPROD_FUNCTION innerprod_double_double
#if SLANG_HAS_COMPLEX
# define INNERPROD_COMPLEX_A innerprod_complex_double
# define INNERPROD_A_COMPLEX innerprod_double_complex
#endif
#define SUM_FUNCTION sum_doubles
#define SUM_RESULT_TYPE double
#define CUMSUM_FUNCTION cumsum_doubles
#define CUMSUM_RESULT_TYPE double
#define MIN_FUNCTION min_doubles
#define MAX_FUNCTION max_doubles
#include "slarrfun.inc"

#define GENERIC_TYPE_A double
#define GENERIC_TYPE_B float
#define GENERIC_TYPE_C double
#define INNERPROD_FUNCTION innerprod_double_float
#include "slarrfun.inc"

#define GENERIC_TYPE_A float
#define GENERIC_TYPE_B double
#define GENERIC_TYPE_C double
#define INNERPROD_FUNCTION innerprod_float_double
#include "slarrfun.inc"

/* Finally pick up the complex_complex multiplication
 * and do the integers
 */
#if SLANG_HAS_COMPLEX
# define INNERPROD_COMPLEX_COMPLEX innerprod_complex_complex
#endif
#endif				       /* SLANG_HAS_FLOAT */

/* -------------- INT --------------------- */
#define GENERIC_TYPE int
#define TRANSPOSE_2D_ARRAY transpose_ints
#define SUM_FUNCTION sum_ints
#define SUM_RESULT_TYPE double
#define CUMSUM_FUNCTION cumsum_ints
#define CUMSUM_RESULT_TYPE double
#define MIN_FUNCTION min_ints
#define MAX_FUNCTION max_ints
#include "slarrfun.inc"

/* -------------- UNSIGNED INT --------------------- */
#define GENERIC_TYPE unsigned int
#define SUM_FUNCTION sum_uints
#define SUM_RESULT_TYPE double
#define MIN_FUNCTION min_uints
#define MAX_FUNCTION max_uints
#include "slarrfun.inc"

#if SIZEOF_LONG != SIZEOF_INT
/* -------------- LONG --------------------- */
# define GENERIC_TYPE long
# define TRANSPOSE_2D_ARRAY transpose_longs
# define SUM_FUNCTION sum_longs
# define SUM_RESULT_TYPE double
# define MIN_FUNCTION min_longs
# define MAX_FUNCTION max_longs
# include "slarrfun.inc"
/* -------------- UNSIGNED LONG --------------------- */
# define GENERIC_TYPE unsigned long
# define SUM_FUNCTION sum_ulongs
# define SUM_RESULT_TYPE double
# define MIN_FUNCTION min_ulongs
# define MAX_FUNCTION max_ulongs
# include "slarrfun.inc"
#else
# define transpose_longs transpose_ints
# define sum_longs sum_ints
# define sum_ulongs sum_uints
# define min_longs min_ints
# define min_ulongs min_uints
# define max_longs max_ints
# define max_ulongs max_uints
#endif

#if SIZEOF_SHORT != SIZEOF_INT
/* -------------- SHORT --------------------- */
# define GENERIC_TYPE short
# define TRANSPOSE_2D_ARRAY transpose_shorts
# define SUM_FUNCTION sum_shorts
# define SUM_RESULT_TYPE double
# define MIN_FUNCTION min_shorts
# define MAX_FUNCTION max_shorts
# include "slarrfun.inc"
/* -------------- UNSIGNED SHORT --------------------- */
# define GENERIC_TYPE unsigned short
# define SUM_FUNCTION sum_ushorts
# define SUM_RESULT_TYPE double
# define MIN_FUNCTION min_ushorts
# define MAX_FUNCTION max_ushorts
# include "slarrfun.inc"
#else
# define transpose_shorts transpose_ints
# define sum_shorts sum_ints
# define sum_ushorts sum_uints
# define min_shorts min_ints
# define min_ushorts min_uints
# define max_shorts max_ints
# define max_ushorts max_uints
#endif

/* -------------- CHAR --------------------- */
#define GENERIC_TYPE char
#define TRANSPOSE_2D_ARRAY transpose_chars
#define SUM_FUNCTION sum_chars
#define SUM_RESULT_TYPE double
#define MIN_FUNCTION min_chars
#define MAX_FUNCTION max_chars
#include "slarrfun.inc"
/* -------------- UNSIGNED CHAR --------------------- */
#define GENERIC_TYPE unsigned char
#define SUM_FUNCTION sum_uchars
#define SUM_RESULT_TYPE double
#define MIN_FUNCTION min_uchars
#define MAX_FUNCTION max_uchars
#include "slarrfun.inc"

/* This routine works only with linear arrays */
static SLang_Array_Type *transpose (SLang_Array_Type *at)
{
   int dims [SLARRAY_MAX_DIMS];
   int *max_dims;
   unsigned int num_dims;
   SLang_Array_Type *bt;
   int i;
   unsigned int sizeof_type;
   int is_ptr;
   char *b_data;

   max_dims = at->dims;
   num_dims = at->num_dims;

   if ((at->num_elements == 0)
       || (num_dims == 1))
     {
	bt = SLang_duplicate_array (at);
	if (num_dims == 1) bt->num_dims = 2;
	goto transpose_dims;
     }

   /* For numeric arrays skip the overhead below */
   if (num_dims == 2)
     {
	bt = allocate_transposed_array (at);
	if (bt == NULL) return NULL;

	switch (at->data_type)
	  {
	   case SLANG_INT_TYPE:
	   case SLANG_UINT_TYPE:
	     return transpose_ints (at, bt);
#if SLANG_HAS_FLOAT
	   case SLANG_DOUBLE_TYPE:
	    return transpose_doubles (at, bt);
	   case SLANG_FLOAT_TYPE:
	     return transpose_floats (at, bt);
#endif
	   case SLANG_CHAR_TYPE:
	   case SLANG_UCHAR_TYPE:
	     return transpose_chars (at, bt);
	   case SLANG_LONG_TYPE:
	   case SLANG_ULONG_TYPE:
	     return transpose_longs (at, bt);
	   case SLANG_SHORT_TYPE:
	   case SLANG_USHORT_TYPE:
	     return transpose_shorts (at, bt);
	  }
     }
   else
     {
	bt = SLang_create_array (at->data_type, 0, NULL, max_dims, num_dims);
	if (bt == NULL) return NULL;
     }

   sizeof_type = at->sizeof_type;
   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);

   memset ((char *)dims, 0, sizeof(dims));

   b_data = (char *) bt->data;

   do
     {
	if (-1 == _SLarray_aget_transfer_elem (at, dims, (VOID_STAR) b_data,
					       sizeof_type, is_ptr))
	  {
	     SLang_free_array (bt);
	     return NULL;
	  }
	b_data += sizeof_type;
     }
   while (0 == next_transposed_index (dims, max_dims, num_dims));

   transpose_dims:

   num_dims = bt->num_dims;
   for (i = 0; i < (int) num_dims; i++)
     bt->dims[i] = max_dims [num_dims - i - 1];

   return bt;
}

static void array_transpose (SLang_Array_Type *at)
{
   if (NULL != (at = transpose (at)))
     (void) SLang_push_array (at, 1);
}

#if SLANG_HAS_FLOAT
static int get_inner_product_parms (SLang_Array_Type *a, int *dp,
				    unsigned int *loops, unsigned int *other)
{
   int num_dims;
   int d;
   
   d = *dp;
   
   num_dims = (int)a->num_dims;
   if (num_dims == 0) 
     {
	SLang_verror (SL_INVALID_PARM, "Inner-product operation requires an array of at least 1 dimension.");
	return -1;
     }

   /* An index of -1 refers to last dimension */
   if (d == -1)
     d += num_dims;
   *dp = d;

   if (a->num_elements == 0)
     {				       /* [] # [] ==> [] */
	*loops = *other = 0;
	return 0;
     }

   *loops = a->num_elements / a->dims[d];

   if (d == 0)
     {
	*other = *loops;  /* a->num_elements / a->dims[0]; */
	return 0;
     }
   
   *other = a->dims[d];
   return 0;
}

/* This routines takes two arrays A_i..j and B_j..k and produces a third
 * via C_i..k = A_i..j B_j..k.
 * 
 * If A is a vector, and B is a 2-d matrix, then regard A as a 2-d matrix
 * with 1-column.
 */
static void do_inner_product (void)
{
   SLang_Array_Type *a, *b, *c;
   void (*fun)(SLang_Array_Type *, SLang_Array_Type *, SLang_Array_Type *,
	       unsigned int, unsigned int, unsigned int, unsigned int, 
	       unsigned int);
   unsigned char c_type;
   int dims[SLARRAY_MAX_DIMS];
   int status;
   unsigned int a_loops, b_loops, b_inc, a_stride;
   int ai_dims, i, j;
   unsigned int num_dims, a_num_dims, b_num_dims;
   int ai, bi;

   /* The result of a inner_product will be either a float, double, or
    * a complex number.
    * 
    * If an integer array is used, it will be promoted to a float.
    */
   
   switch (SLang_peek_at_stack1 ())
     {
      case SLANG_DOUBLE_TYPE:
	if (-1 == SLang_pop_array_of_type (&b, SLANG_DOUBLE_TYPE))
	  return;
	break;

#if SLANG_HAS_COMPLEX
      case SLANG_COMPLEX_TYPE:
	if (-1 == SLang_pop_array_of_type (&b, SLANG_COMPLEX_TYPE))
	  return;
	break;
#endif
      case SLANG_FLOAT_TYPE:
      default:
	if (-1 == SLang_pop_array_of_type (&b, SLANG_FLOAT_TYPE))
	  return;
	break;
     }

   switch (SLang_peek_at_stack1 ())
     {
      case SLANG_DOUBLE_TYPE:
	status = SLang_pop_array_of_type (&a, SLANG_DOUBLE_TYPE);
	break;

#if SLANG_HAS_COMPLEX
      case SLANG_COMPLEX_TYPE:
	status = SLang_pop_array_of_type (&a, SLANG_COMPLEX_TYPE);
	break;
#endif
      case SLANG_FLOAT_TYPE:
      default:
	status = SLang_pop_array_of_type (&a, SLANG_FLOAT_TYPE);
	break;
     }
   
   if (status == -1)
     {
	SLang_free_array (b);
	return;
     }
   
   ai = -1;			       /* last index of a */
   bi = 0;			       /* first index of b */
   if ((-1 == get_inner_product_parms (a, &ai, &a_loops, &a_stride))
       || (-1 == get_inner_product_parms (b, &bi, &b_loops, &b_inc)))
     {
	SLang_verror (SL_TYPE_MISMATCH, "Array dimensions are not compatible for inner-product");
	goto free_and_return;
     }
       
   a_num_dims = a->num_dims;
   b_num_dims = b->num_dims;

   /* Coerse a 1-d vector to 2-d */
   if ((a_num_dims == 1) 
       && (b_num_dims == 2)
       && (a->num_elements))
     {
	a_num_dims = 2;
	ai = 1;
	a_loops = a->num_elements;
	a_stride = 1;
     }

   if ((ai_dims = a->dims[ai]) != b->dims[bi])
     {
	SLang_verror (SL_TYPE_MISMATCH, "Array dimensions are not compatible for inner-product");
	goto free_and_return;
     }

   num_dims = a_num_dims + b_num_dims - 2;
   if (num_dims > SLARRAY_MAX_DIMS)
     {
	SLang_verror (SL_NOT_IMPLEMENTED,
		      "Inner-product result exceeds maximum allowed dimensions");
	goto free_and_return;
     }

   if (num_dims)
     {
	j = 0;
	for (i = 0; i < (int)a_num_dims; i++)
	  if (i != ai) dims [j++] = a->dims[i];
	for (i = 0; i < (int)b_num_dims; i++)
	  if (i != bi) dims [j++] = b->dims[i];
     }
   else
     {
	/* a scalar */
	num_dims = 1;
	dims[0] = 1;
     }

   c_type = 0; fun = NULL;
   switch (a->data_type)
     {
      case SLANG_FLOAT_TYPE:
	switch (b->data_type)
	  {
	   case SLANG_FLOAT_TYPE:
	     c_type = SLANG_FLOAT_TYPE;
	     fun = innerprod_float_float;
	     break;
	   case SLANG_DOUBLE_TYPE:
	     c_type = SLANG_DOUBLE_TYPE;
	     fun = innerprod_float_double;
	     break;
#if SLANG_HAS_COMPLEX
	   case SLANG_COMPLEX_TYPE:
	     c_type = SLANG_COMPLEX_TYPE;
	     fun = innerprod_float_complex;
	     break;
#endif
	  }
	break;
      case SLANG_DOUBLE_TYPE:
	switch (b->data_type)
	  {
	   case SLANG_FLOAT_TYPE:
	     c_type = SLANG_DOUBLE_TYPE;
	     fun = innerprod_double_float;
	     break;
	   case SLANG_DOUBLE_TYPE:
	     c_type = SLANG_DOUBLE_TYPE;
	     fun = innerprod_double_double;
	     break;
#if SLANG_HAS_COMPLEX
	   case SLANG_COMPLEX_TYPE:
	     c_type = SLANG_COMPLEX_TYPE;
	     fun = innerprod_double_complex;
	     break;
#endif
	  }
	break;
#if SLANG_HAS_COMPLEX
      case SLANG_COMPLEX_TYPE:
	c_type = SLANG_COMPLEX_TYPE;
	switch (b->data_type)
	  {
	   case SLANG_FLOAT_TYPE:
	     fun = innerprod_complex_float;
	     break;
	   case SLANG_DOUBLE_TYPE:
	     fun = innerprod_complex_double;
	     break;
	   case SLANG_COMPLEX_TYPE:
	     fun = innerprod_complex_complex;
	     break;
	  }
	break;
#endif
      default:
	break;
     }

   if (NULL == (c = SLang_create_array (c_type, 0, NULL, dims, num_dims)))
     goto free_and_return;

   (*fun)(a, b, c, a_loops, a_stride, b_loops, b_inc, ai_dims);

   (void) SLang_push_array (c, 1);
   /* drop */

   free_and_return:
   SLang_free_array (a);
   SLang_free_array (b);
}
#endif

static int map_or_contract_array (SLCONST SLarray_Map_Type *c, int use_contraction,
				  int dim_specified, int *use_this_dim, 
				  VOID_STAR clientdata)
{
   int k, use_all_dims;
   SLang_Array_Type *at, *new_at;
   int *old_dims;
   int old_dims_buf[SLARRAY_MAX_DIMS];
   int sub_dims[SLARRAY_MAX_DIMS];
   int tmp_dims[SLARRAY_MAX_DIMS];
   unsigned int i, j, old_num_dims, sub_num_dims;
   SLtype new_data_type, old_data_type;
   char *old_data, *new_data;
   int w[SLARRAY_MAX_DIMS], wk;
   unsigned int old_sizeof_type, new_sizeof_type;
   unsigned int dims_k;
   int from_type;
   SLCONST SLarray_Map_Type *csave;
   SLarray_Map_Fun_Type *fmap;
   SLarray_Contract_Fun_Type *fcon;

   use_all_dims = 1;
   k = 0;
   if (dim_specified)
     {
	if (use_this_dim != NULL)
	  {
	     k = *use_this_dim;
	     use_all_dims = 0;
	  }
     }
   else if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_integer (&k))
	  return -1;

	use_all_dims = 0;
     }

   if (-1 == (from_type = SLang_peek_at_stack1 ()))
     return -1;

   csave = c;
   while (c->f != NULL)
     {
	if (c->from_type == (SLtype) from_type)
	  break;
	c++;
     }

   /* Look for a more generic version */
   if (c->f != NULL)
     {
	if (-1 == SLang_pop_array_of_type (&at, c->typecast_to_type))
	  return -1;
     }
   else
     {
	/* Look for a wildcard match */
	c = csave;
	while (c->f != NULL)
	  {
	     if (c->from_type == SLANG_VOID_TYPE)
	       break;
	     c++;
	  }
	if (c->f == NULL)
	  {
	     SLang_verror (SL_TYPE_MISMATCH, "%s is not supported by this function", SLclass_get_datatype_name (from_type));
	     return -1;
	  }
	
	/* Found it. So, typecast it to appropriate type */
	if (c->typecast_to_type == SLANG_VOID_TYPE)
	  {
	     if (-1 == SLang_pop_array (&at, 1))
	       return -1;
	  }
	else if (-1 == SLang_pop_array_of_type (&at, c->typecast_to_type))
	  return -1;
     }

   old_data_type = at->data_type;
   if (SLANG_VOID_TYPE == (new_data_type = c->result_type))
     new_data_type = old_data_type;

   old_num_dims = at->num_dims;

   if (use_all_dims == 0)
     {
	if (k < 0)
	  k += old_num_dims;

	if ((k < 0) || (k >= (int)old_num_dims))
	  {
	     SLang_verror (SL_INVALID_PARM, "Dimension %d is invalid for a %d-d array",
			   k, old_num_dims);
	     SLang_free_array (at);
	     return -1;
	  }
	old_dims = at->dims;
     }
   else
     {
	old_dims = old_dims_buf;
	old_dims[0] = at->num_elements;
	old_num_dims = 1;
     }
   
   fcon = (SLarray_Contract_Fun_Type *) c->f;
   fmap = c->f;

   if (use_contraction 
       && (use_all_dims || (old_num_dims == 1)))
     {
	SLang_Class_Type *cl;
	VOID_STAR buf;
	int status = 0;
	
	cl = _SLclass_get_class (new_data_type);
	buf = cl->cl_transfer_buf;

	if ((-1 == (*fcon) (at->data, 1, at->num_elements, buf))
	    || (-1 == SLang_push_value (new_data_type, buf)))
	  status = -1;
	
	SLang_free_array (at);
	return status;
     }

   /* The offset for the index i_0,i_1,...i_{N-1} is
    * i_0*W_0 + i_1*W_1 + ... i_{N-1}*W{N-1}
    * where W_j = d_{j+1}d_{j+2}...d_{N-1}
    * and d_k is the number of elements of the kth dimension.
    * 
    * For a specified value of k, we
    * So, summing over all elements in the kth dimension of the array
    * means using the set of offsets given by 
    *  
    *   i_k*W_k + sum(j!=k) i_j*W_j.
    *
    * So, we want to loop of all dimensions except for the kth using an 
    * offset given by sum(j!=k)i_jW_j, and an increment W_k between elements.
    */

   wk = 1;
   i = old_num_dims;
   while (i != 0)
     {
	i--;
	w[i] = wk;
	wk *= old_dims[i];
     }
   wk = w[k];
   
   /* Now set up the sub array */
   j = 0;
   for (i = 0; i < old_num_dims; i++)
     {
	if (i == (unsigned int) k)
	  continue;
	
	sub_dims[j] = old_dims[i];
	w[j] = w[i];
	tmp_dims[j] = 0;
	j++;
     }
   sub_num_dims = old_num_dims - 1;

   if (use_contraction)
     new_at = SLang_create_array1 (new_data_type, 0, NULL, sub_dims, sub_num_dims, 1);
   else
     new_at = SLang_create_array1 (new_data_type, 0, NULL, old_dims, old_num_dims, 1);

   if (new_at == NULL)
     {
	SLang_free_array (at);
	return -1;
     }

   new_data = (char *)new_at->data;
   old_data = (char *)at->data;
   old_sizeof_type = at->sizeof_type;
   new_sizeof_type = new_at->sizeof_type;
   dims_k = old_dims[k] * wk;

   do
     {
	unsigned int offset = 0;
	int status;

	for (i = 0; i < sub_num_dims; i++)
	  offset += w[i] * tmp_dims[i];
	
	if (use_contraction)
	  {
	     status = (*fcon) ((VOID_STAR)(old_data + offset*old_sizeof_type), wk,
			       dims_k, (VOID_STAR) new_data);
	     new_data += new_sizeof_type;
	  }
	else 
	  {
	     status = (*fmap) (old_data_type, (VOID_STAR) (old_data + offset*old_sizeof_type),
			       wk, dims_k,
			       new_data_type, (VOID_STAR) (new_data + offset*new_sizeof_type),
			       clientdata);
	  }
	
	if (status == -1)
	  {
	     SLang_free_array (new_at);
	     SLang_free_array (at);
	     return -1;
	  }
     }
   while (-1 != _SLarray_next_index (tmp_dims, sub_dims, sub_num_dims));

   SLang_free_array (at);
   return SLang_push_array (new_at, 1);
}

   
int SLarray_map_array (SLCONST SLarray_Map_Type *m)
{
   return map_or_contract_array (m, 0, 0, NULL, NULL);
}

int SLarray_map_array_1 (SLCONST SLarray_Map_Type *m, int *use_this_dim, 
			 VOID_STAR clientdata)
{
   return map_or_contract_array (m, 0, 1, use_this_dim, clientdata);
}

int SLarray_contract_array (SLCONST SLarray_Contract_Type *c)
{
   return map_or_contract_array ((SLarray_Map_Type *)c, 1, 0, NULL, NULL);
}

#if SLANG_HAS_COMPLEX
static int sum_complex (VOID_STAR zp, unsigned int inc, unsigned int num, VOID_STAR sp)
{
   double *z, *zmax;
   double sr, si;
   double *s;

   z = (double *)zp;
   zmax = z + 2*num;
   inc *= 2;
   sr = si = 0.0;
   while (z < zmax)
     {
	sr += z[0];
	si += z[1];
	z += inc;
     }
   s = (double *)sp;
   s[0] = sr;
   s[1] = si;
   return 0;
}

static int cumsum_complex (SLtype xtype, VOID_STAR xp, unsigned int inc, 
			   unsigned int num,
			   SLtype ytype, VOID_STAR yp, VOID_STAR clientdata)
{
   double *z, *zmax;
   double cr, ci;
   double *s;

   (void) xtype; (void) ytype; (void) clientdata;
   z = (double *)xp;
   zmax = z + 2*num;
   s = (double *)yp;
   inc *= 2;
   cr = ci = 0.0;
   while (z < zmax)
     {
	cr += z[0];
	ci += z[1];
	s[0] = cr;
	s[1] = ci;
	z += inc;
	s += inc;
     }
   return 0;
}
#endif
#if SLANG_HAS_FLOAT
static SLCONST SLarray_Contract_Type Sum_Functions [] =
{
     {SLANG_CHAR_TYPE, SLANG_CHAR_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Contract_Fun_Type *) sum_chars},
     {SLANG_UCHAR_TYPE, SLANG_UCHAR_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Contract_Fun_Type *) sum_uchars},
     {SLANG_SHORT_TYPE, SLANG_SHORT_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Contract_Fun_Type *) sum_shorts},
     {SLANG_USHORT_TYPE, SLANG_USHORT_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Contract_Fun_Type *) sum_ushorts},
     {SLANG_UINT_TYPE, SLANG_UINT_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Contract_Fun_Type *) sum_uints},
     {SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Contract_Fun_Type *) sum_ints},
     {SLANG_LONG_TYPE, SLANG_LONG_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Contract_Fun_Type *) sum_longs},
     {SLANG_ULONG_TYPE, SLANG_ULONG_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Contract_Fun_Type *) sum_ulongs},
     {SLANG_FLOAT_TYPE, SLANG_FLOAT_TYPE, SLANG_FLOAT_TYPE, (SLarray_Contract_Fun_Type *) sum_floats},
     {SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Contract_Fun_Type *) sum_doubles},
#if SLANG_HAS_COMPLEX
     {SLANG_COMPLEX_TYPE, SLANG_COMPLEX_TYPE, SLANG_COMPLEX_TYPE, (SLarray_Contract_Fun_Type *) sum_complex},
#endif
     {0, 0, 0, NULL}
};

static void array_sum (void)
{
   (void) SLarray_contract_array (Sum_Functions);
}
#endif

static SLCONST SLarray_Contract_Type Array_Min_Funs [] = 
{
     {SLANG_CHAR_TYPE, SLANG_CHAR_TYPE, SLANG_CHAR_TYPE, (SLarray_Contract_Fun_Type *) min_chars},
     {SLANG_UCHAR_TYPE, SLANG_UCHAR_TYPE, SLANG_UCHAR_TYPE, (SLarray_Contract_Fun_Type *) min_uchars},
     {SLANG_SHORT_TYPE, SLANG_SHORT_TYPE, SLANG_SHORT_TYPE, (SLarray_Contract_Fun_Type *) min_shorts},
     {SLANG_USHORT_TYPE, SLANG_USHORT_TYPE, SLANG_USHORT_TYPE, (SLarray_Contract_Fun_Type *) min_ushorts},
     {SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE, (SLarray_Contract_Fun_Type *) min_ints},
     {SLANG_UINT_TYPE, SLANG_UINT_TYPE, SLANG_UINT_TYPE, (SLarray_Contract_Fun_Type *) min_uints},
     {SLANG_LONG_TYPE, SLANG_LONG_TYPE, SLANG_LONG_TYPE, (SLarray_Contract_Fun_Type *) min_longs},
     {SLANG_ULONG_TYPE, SLANG_ULONG_TYPE, SLANG_ULONG_TYPE, (SLarray_Contract_Fun_Type *) min_ulongs},
#if SLANG_HAS_FLOAT
     {SLANG_FLOAT_TYPE, SLANG_FLOAT_TYPE, SLANG_FLOAT_TYPE, (SLarray_Contract_Fun_Type *) min_floats},
     {SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Contract_Fun_Type *) min_doubles},
#endif
     {0, 0, 0, NULL}
};
   
static void 
array_min (void)
{
   (void) SLarray_contract_array (Array_Min_Funs);
}

static SLCONST SLarray_Contract_Type Array_Max_Funs [] =
{
     {SLANG_CHAR_TYPE, SLANG_CHAR_TYPE, SLANG_CHAR_TYPE, (SLarray_Contract_Fun_Type *) max_chars},
     {SLANG_UCHAR_TYPE, SLANG_UCHAR_TYPE, SLANG_UCHAR_TYPE, (SLarray_Contract_Fun_Type *) max_uchars},
     {SLANG_SHORT_TYPE, SLANG_SHORT_TYPE, SLANG_SHORT_TYPE, (SLarray_Contract_Fun_Type *) max_shorts},
     {SLANG_USHORT_TYPE, SLANG_USHORT_TYPE, SLANG_USHORT_TYPE, (SLarray_Contract_Fun_Type *) max_ushorts},
     {SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_INT_TYPE, (SLarray_Contract_Fun_Type *) max_ints},
     {SLANG_UINT_TYPE, SLANG_UINT_TYPE, SLANG_UINT_TYPE, (SLarray_Contract_Fun_Type *) max_uints},
     {SLANG_LONG_TYPE, SLANG_LONG_TYPE, SLANG_LONG_TYPE, (SLarray_Contract_Fun_Type *) max_longs},
     {SLANG_ULONG_TYPE, SLANG_ULONG_TYPE, SLANG_ULONG_TYPE, (SLarray_Contract_Fun_Type *) max_ulongs},
#if SLANG_HAS_FLOAT
     {SLANG_FLOAT_TYPE, SLANG_FLOAT_TYPE, SLANG_FLOAT_TYPE, (SLarray_Contract_Fun_Type *) max_floats},
     {SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Contract_Fun_Type *) max_doubles},
#endif
     {0, 0, 0, NULL}
};

static void 
array_max (void)
{
   (void) SLarray_contract_array (Array_Max_Funs);
}


static SLCONST SLarray_Map_Type CumSum_Functions [] =
{
     {SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Map_Fun_Type *) cumsum_doubles},
     {SLANG_INT_TYPE, SLANG_INT_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Map_Fun_Type *) cumsum_ints},
     {SLANG_LONG_TYPE, SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Map_Fun_Type *) cumsum_doubles},
     {SLANG_FLOAT_TYPE, SLANG_FLOAT_TYPE, SLANG_FLOAT_TYPE, (SLarray_Map_Fun_Type *) cumsum_floats},
     {SLANG_UINT_TYPE, SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Map_Fun_Type *) cumsum_doubles},
     {SLANG_ULONG_TYPE, SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Map_Fun_Type *) cumsum_doubles},
     {SLANG_CHAR_TYPE, SLANG_FLOAT_TYPE, SLANG_FLOAT_TYPE, (SLarray_Map_Fun_Type *) cumsum_floats},
     {SLANG_UCHAR_TYPE, SLANG_FLOAT_TYPE, SLANG_FLOAT_TYPE, (SLarray_Map_Fun_Type *) cumsum_floats},
     {SLANG_SHORT_TYPE, SLANG_FLOAT_TYPE, SLANG_FLOAT_TYPE, (SLarray_Map_Fun_Type *) cumsum_floats},
     {SLANG_USHORT_TYPE, SLANG_FLOAT_TYPE, SLANG_FLOAT_TYPE, (SLarray_Map_Fun_Type *) cumsum_floats},
     {SLANG_VOID_TYPE, SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE, (SLarray_Map_Fun_Type *) cumsum_doubles},
#if SLANG_HAS_COMPLEX
     {SLANG_COMPLEX_TYPE, SLANG_COMPLEX_TYPE, SLANG_COMPLEX_TYPE, (SLarray_Map_Fun_Type *) cumsum_complex},
#endif
     {0, 0, 0, NULL}
};

static void array_cumsum (void)
{
   (void) SLarray_map_array (CumSum_Functions);
}

static SLang_Intrin_Fun_Type Array_Fun_Table [] =
{
   MAKE_INTRINSIC_1("transpose", array_transpose, SLANG_VOID_TYPE, SLANG_ARRAY_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

static SLang_Intrin_Fun_Type Array_Math_Fun_Table [] =
{
#if SLANG_HAS_FLOAT
   MAKE_INTRINSIC_0("sum", array_sum, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("cumsum", array_cumsum, SLANG_VOID_TYPE),
#endif
   MAKE_INTRINSIC_0("min", array_min, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("max", array_max, SLANG_VOID_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

int SLang_init_array (void)
{
   if (-1 == SLadd_intrin_fun_table (Array_Fun_Table, "__SLARRAY__"))
     return -1;
#if SLANG_HAS_FLOAT
   _SLang_Matrix_Multiply = do_inner_product;
#endif
   return 0;
}

int SLang_init_array_extra (void)
{
   if (-1 == SLadd_intrin_fun_table (Array_Math_Fun_Table, "__SLARRAY__"))
     return -1;
   return 0;
}

