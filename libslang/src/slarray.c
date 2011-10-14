/* Array manipulation routines for S-Lang */
/* Copyright (c) 1997, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

/* #define SL_APP_WANTS_FOREACH */
#include "slang.h"
#include "_slang.h"

typedef struct
{
   int first_index;
   int last_index;
   int delta;
}
SLarray_Range_Array_Type;

/* Use SLang_pop_array when a linear array is required. */
static int pop_array (SLang_Array_Type **at_ptr, int convert_scalar)
{
   SLang_Array_Type *at;
   int one = 1;
   int type;

   *at_ptr = NULL;
   type = SLang_peek_at_stack ();

   switch (type)
     {
      case -1:
	return -1;

      case SLANG_ARRAY_TYPE:
	return SLclass_pop_ptr_obj (SLANG_ARRAY_TYPE, (VOID_STAR *) at_ptr);

      case SLANG_NULL_TYPE:
	convert_scalar = 0;
	/* drop */
      default:
	if (convert_scalar == 0)
	  {
	     SLdo_pop ();
	     SLang_verror (SL_TYPE_MISMATCH, "Context requires an array.  Scalar not converted");
	     return -1;
	  }
	break;
     }

   if (NULL == (at = SLang_create_array ((unsigned char) type, 0, NULL, &one, 1)))
     return -1;

   if (-1 == at->cl->cl_apop ((unsigned char) type, at->data))
     {
	SLang_free_array (at);
	return -1;
     }

   *at_ptr = at;

   return 0;
}

static VOID_STAR linear_get_data_addr (SLang_Array_Type *at, int *dims)
{
   unsigned int num_dims;
   unsigned int ofs;
   unsigned int i;
   int *max_dims;

   ofs = 0;
   max_dims = at->dims;
   num_dims = at->num_dims;

   for (i = 0; i < num_dims; i++)
     {
	int d = dims[i];

	if (d < 0)
	  d = d + max_dims[i];

	ofs = ofs * (unsigned int)max_dims [i] + (unsigned int) d;
     }

   return (VOID_STAR) ((char *)at->data + (ofs * at->sizeof_type));
}

static VOID_STAR get_data_addr (SLang_Array_Type *at, int *dims)
{
   VOID_STAR data;

   data = at->data;
   if (data == NULL)
     {
	SLang_verror (SL_UNKNOWN_ERROR, "Array has no data");
	return NULL;
     }

   data = (*at->index_fun) (at, dims);

   if (data == NULL)
     {
	SLang_verror (SL_UNKNOWN_ERROR, "Unable to access array element");
	return NULL;
     }

   return data;
}

void _SLarray_free_array_elements (SLang_Class_Type *cl, VOID_STAR s, unsigned int num)
{
   unsigned int sizeof_type;
   void (*f) (unsigned char, VOID_STAR);
   char *p;
   unsigned char type;

   if ((cl->cl_class_type == SLANG_CLASS_TYPE_SCALAR)
       || (cl->cl_class_type == SLANG_CLASS_TYPE_VECTOR))
     return;

   f = cl->cl_destroy;
   sizeof_type = cl->cl_sizeof_type;
   type = cl->cl_data_type;

   p = (char *) s;
   while (num != 0)
     {
	if (NULL != *(VOID_STAR *)p)
	  {
	     (*f) (type, (VOID_STAR)p);
	     *(VOID_STAR *) p = NULL;
	  }
	p += sizeof_type;
	num--;
     }
}

static int destroy_element (SLang_Array_Type *at,
			    int *dims,
			    VOID_STAR data)
{
   data = get_data_addr (at, dims);
   if (data == NULL)
     return -1;

   /* This function should only get called for arrays that have
    * pointer elements.  Do not call the destroy method if the element
    * is NULL.
    */
   if (NULL != *(VOID_STAR *)data)
     {
	(*at->cl->cl_destroy) (at->data_type, data);
	*(VOID_STAR *) data = NULL;
     }
   return 0;
}

/* This function only gets called when a new array is created.  Thus there
 * is no need to destroy the object first.
 */
static int new_object_element (SLang_Array_Type *at,
			       int *dims,
			       VOID_STAR data)
{
   data = get_data_addr (at, dims);
   if (data == NULL)
     return -1;

   return (*at->cl->cl_init_array_object) (at->data_type, data);
}

int _SLarray_next_index (int *dims, int *max_dims, unsigned int num_dims)
{
   while (num_dims)
     {
	int dims_i;

	num_dims--;

	dims_i = dims [num_dims] + 1;
	if (dims_i != (int) max_dims [num_dims])
	  {
	     dims [num_dims] = dims_i;
	     return 0;
	  }
	dims [num_dims] = 0;
     }

   return -1;
}

static int do_method_for_all_elements (SLang_Array_Type *at,
				       int (*method)(SLang_Array_Type *,
						     int *,
						     VOID_STAR),
				       VOID_STAR client_data)
{
   int dims [SLARRAY_MAX_DIMS];
   int *max_dims;
   unsigned int num_dims;

   if (at->num_elements == 0)
     return 0;

   max_dims = at->dims;
   num_dims = at->num_dims;

   SLMEMSET((char *)dims, 0, sizeof(dims));

   do
     {
	if (-1 == (*method) (at, dims, client_data))
	  return -1;
     }
   while (0 == _SLarray_next_index (dims, max_dims, num_dims));

   return 0;
}

void SLang_free_array (SLang_Array_Type *at)
{
   unsigned int flags;

   if (at == NULL) return;

   if (at->num_refs > 1)
     {
	at->num_refs -= 1;
	return;
     }

   flags = at->flags;

   if (flags & SLARR_DATA_VALUE_IS_INTRINSIC)
     return;			       /* not to be freed */

   if (flags & SLARR_DATA_VALUE_IS_POINTER)
     (void) do_method_for_all_elements (at, destroy_element, NULL);

   if (at->free_fun != NULL)
     at->free_fun (at);
   else
     SLfree ((char *) at->data);

   SLfree ((char *) at);
}

SLang_Array_Type *
SLang_create_array1 (unsigned char type, int read_only, VOID_STAR data,
		     int *dims, unsigned int num_dims, int no_init)
{
   SLang_Class_Type *cl;
   unsigned int i;
   SLang_Array_Type *at;
   unsigned int num_elements;
   unsigned int sizeof_type;
   unsigned int size;

   if (num_dims > SLARRAY_MAX_DIMS)
     {
	SLang_verror (SL_NOT_IMPLEMENTED, "%u dimensional arrays are not supported", num_dims);
	return NULL;
     }

   for (i = 0; i < num_dims; i++)
     {
	if (dims[i] < 0)
	  {
	     SLang_verror (SL_INVALID_PARM, "Size of array dim %u is less than 0", i);
	     return NULL;
	  }
     }

   cl = _SLclass_get_class (type);

   at = (SLang_Array_Type *) SLmalloc (sizeof(SLang_Array_Type));
   if (at == NULL)
     return NULL;

   SLMEMSET ((char*) at, 0, sizeof(SLang_Array_Type));

   at->data_type = type;
   at->cl = cl;
   at->num_dims = num_dims;
   at->num_refs = 1;

   if (read_only) at->flags = SLARR_DATA_VALUE_IS_READ_ONLY;
   switch (cl->cl_class_type)
     {
      case SLANG_CLASS_TYPE_VECTOR:
      case SLANG_CLASS_TYPE_SCALAR:
	break;

      default:
	at->flags |= SLARR_DATA_VALUE_IS_POINTER;
     }

   num_elements = 1;
   for (i = 0; i < num_dims; i++)
     {
	at->dims [i] = dims[i];
	num_elements = dims [i] * num_elements;
     }

   /* Now set the rest of the unused dimensions to 1.  This makes it easier
    * when transposing arrays.
    */
   while (i < SLARRAY_MAX_DIMS)
     at->dims[i++] = 1;

   at->num_elements = num_elements;
   at->index_fun = linear_get_data_addr;
   at->sizeof_type = sizeof_type = cl->cl_sizeof_type;

   if (data != NULL)
     {
	at->data = data;
	return at;
     }

   size = num_elements * sizeof_type;

   if (size == 0) size = 1;

   if (NULL == (data = (VOID_STAR) SLmalloc (size)))
     {
	SLang_free_array (at);
	return NULL;
     }

   if (no_init == 0)
     SLMEMSET ((char *) data, 0, size);

   at->data = data;

   if ((cl->cl_init_array_object != NULL)
       && (-1 == do_method_for_all_elements (at, new_object_element, NULL)))
     {
	SLang_free_array (at);
	return NULL;
     }
   return at;
}

SLang_Array_Type *
SLang_create_array (unsigned char type, int read_only, VOID_STAR data,
		    int *dims, unsigned int num_dims)
{
   return SLang_create_array1 (type, read_only, data, dims, num_dims, 0);
}

int SLang_add_intrinsic_array (char *name,
			       unsigned char type,
			       int read_only,
			       VOID_STAR data,
			       unsigned int num_dims, ...)
{
   va_list ap;
   unsigned int i;
   int dims[SLARRAY_MAX_DIMS];
   SLang_Array_Type *at;

   if ((num_dims > SLARRAY_MAX_DIMS)
       || (name == NULL)
       || (data == NULL))
     {
	SLang_verror (SL_INVALID_PARM, "Unable to create intrinsic array");
	return -1;
     }

   va_start (ap, num_dims);
   for (i = 0; i < num_dims; i++)
     dims [i] = va_arg (ap, int);
   va_end (ap);

   at = SLang_create_array (type, read_only, data, dims, num_dims);
   if (at == NULL)
     return -1;
   at->flags |= SLARR_DATA_VALUE_IS_INTRINSIC;

   /* Note: The variable that refers to the intrinsic array is regarded as
    * read-only.  That way, Array_Name = another_array; will fail.
    */
   if (-1 == SLadd_intrinsic_variable (name, (VOID_STAR) at, SLANG_ARRAY_TYPE, 1))
     {
	SLang_free_array (at);
	return -1;
     }
   return 0;
}

static int pop_array_indices (int *dims, unsigned int num_dims)
{
   unsigned int n;
   int i;

   if (num_dims > SLARRAY_MAX_DIMS)
     {
	SLang_verror (SL_INVALID_PARM, "Array size not supported");
	return -1;
     }

   n = num_dims;
   while (n != 0)
     {
	n--;
	if (-1 == SLang_pop_integer (&i))
	  return -1;

	dims[n] = i;
     }

   return 0;
}

int SLang_push_array (SLang_Array_Type *at, int free_flag)
{
   if (at == NULL)
     return SLang_push_null ();

   at->num_refs += 1;

   if (0 == SLclass_push_ptr_obj (SLANG_ARRAY_TYPE, (VOID_STAR) at))
     {
	if (free_flag)
	  SLang_free_array (at);
	return 0;
     }

   at->num_refs -= 1;

   if (free_flag) SLang_free_array (at);
   return -1;
}

/* This function gets called via expressions such as Double_Type[10, 20];
 */
static int push_create_new_array (void)
{
   unsigned int num_dims;
   SLang_Array_Type *at;
   unsigned char type;
   int dims [SLARRAY_MAX_DIMS];
   int (*anew) (unsigned char, unsigned int);

   num_dims = (SLang_Num_Function_Args - 1);

   if (-1 == SLang_pop_datatype (&type))
     return -1;

   anew = (_SLclass_get_class (type))->cl_anew;
   if (anew != NULL)
     return (*anew) (type, num_dims);

   if (-1 == pop_array_indices (dims, num_dims))
     return -1;

   if (NULL == (at = SLang_create_array (type, 0, NULL, dims, num_dims)))
     return -1;

   return SLang_push_array (at, 1);
}

static int push_element_at_addr (SLang_Array_Type *at,
				 VOID_STAR data, int allow_null)
{
   SLang_Class_Type *cl;

   cl = at->cl;
   if ((at->flags & SLARR_DATA_VALUE_IS_POINTER)
       && (*(VOID_STAR *) data == NULL))
     {
	if (allow_null)
	  return SLang_push_null ();

	SLang_verror (SL_VARIABLE_UNINITIALIZED,
		      "%s array has unitialized element", cl->cl_name);
	return -1;
     }

   return (*cl->cl_apush)(at->data_type, data);
}

static int coerse_array_to_linear (SLang_Array_Type *at)
{
   SLarray_Range_Array_Type *range;
   int *data;
   int xmin, dx;
   unsigned int i, imax;

   /* FIXME: Priority = low.  This assumes that if an array is not linear, then
    * it is a range.
    */
   if (0 == (at->flags & SLARR_DATA_VALUE_IS_RANGE))
     return 0;

   range = (SLarray_Range_Array_Type *) at->data;
   xmin = range->first_index;
   dx = range->delta;

   imax = at->num_elements;
   data = (int *) SLmalloc ((imax + 1) * sizeof (int));
   if (data == NULL)
     return -1;

   for (i = 0; i < imax; i++)
     {
	data [i] = xmin;
	xmin += dx;
     }

   SLfree ((char *) range);
   at->data = (VOID_STAR) data;
   at->flags &= ~SLARR_DATA_VALUE_IS_RANGE;
   at->index_fun = linear_get_data_addr;
   return 0;
}

static void
free_index_objects (SLang_Object_Type *index_objs, unsigned int num_indices)
{
   unsigned int i;
   SLang_Object_Type *obj;

   for (i = 0; i < num_indices; i++)
     {
	obj = index_objs + i;
	if (obj->data_type != 0)
	  SLang_free_object (obj);
     }
}

static int
pop_indices (SLang_Array_Type *at_to_index,
	     SLang_Object_Type *index_objs, unsigned int num_indices,
	     int *is_index_array)
{
   unsigned int i;

   SLMEMSET((char *) index_objs, 0, num_indices * sizeof (SLang_Object_Type));

   *is_index_array = 0;

   if (num_indices >= SLARRAY_MAX_DIMS)
     {
	SLang_verror (SL_INVALID_PARM, "too many indices for array");
	return -1;
     }

   i = num_indices;
   while (i != 0)
     {
	SLang_Object_Type *obj;

	i--;
	obj = index_objs + i;
	if (-1 == _SLang_pop_object_of_type (SLANG_INT_TYPE, obj, 1))
	  goto return_error;

	if (obj->data_type == SLANG_ARRAY_TYPE)
	  {
	     SLang_Array_Type *at = obj->v.array_val;

	     if (at->num_dims == 1)
	       {
		  /* Note: Suppose a = Int_Type[10, 10].  Then the check
		   * on the range flag prevents this from being indexed by
		   * a range such as 'a[[0:99]]'
		   */
		  if (num_indices == 1)
		    {
		       if ((at_to_index->num_dims > 1)
			   || (0 == (at->flags & SLARR_DATA_VALUE_IS_RANGE)))
			 *is_index_array = 1;
		    }
	       }
	     else
	       {
		  SLang_verror (SL_INVALID_PARM, "expecting a 1-d index array");
		  goto return_error;
	       }
	  }
     }

   return 0;

   return_error:
   free_index_objects (index_objs, num_indices);
   return -1;
}

/* Here ind_at is a linear 1-d array of indices */
static int
check_index_array_ranges (SLang_Array_Type *at, SLang_Array_Type *ind_at)
{
   int *indices, *indices_max;
   unsigned int num_elements;

   num_elements = at->num_elements;
   indices = (int *) ind_at->data;
   indices_max = indices + ind_at->num_elements;

   while (indices < indices_max)
     {
	unsigned int d;

	d = (unsigned int) *indices++;
	if (d >= num_elements)
	  {
	     SLang_verror (SL_INVALID_PARM,
			   "index-array is out of range");
	     return -1;
	  }
     }
   return 0;
}

static int
transfer_n_elements (SLang_Array_Type *at, VOID_STAR dest_data, VOID_STAR src_data,
		     unsigned int sizeof_type, unsigned int n, int is_ptr)
{
   unsigned char data_type;
   SLang_Class_Type *cl;

   if (is_ptr == 0)
     {
	SLMEMCPY ((char *) dest_data, (char *)src_data, n * sizeof_type);
	return 0;
     }

   data_type = at->data_type;
   cl = at->cl;

   while (n != 0)
     {
	if (*(VOID_STAR *)dest_data != NULL)
	  {
	     (*cl->cl_destroy) (data_type, dest_data);
	     *(VOID_STAR *) dest_data = NULL;
	  }

	if (*(VOID_STAR *) src_data == NULL)
	  *(VOID_STAR *) dest_data = NULL;
	else
	  {
	     if (-1 == (*cl->cl_acopy) (data_type, src_data, dest_data))
	       /* No need to destroy anything */
	       return -1;
	  }

	src_data = (VOID_STAR) ((char *)src_data + sizeof_type);
	dest_data = (VOID_STAR) ((char *)dest_data + sizeof_type);

	n--;
     }

   return 0;
}

int
_SLarray_aget_transfer_elem (SLang_Array_Type *at, int *indices,
			     VOID_STAR new_data, unsigned int sizeof_type, int is_ptr)
{
   VOID_STAR at_data;

   /* Since 1 element is being transferred, there is not need to coerse
    * the array to linear.
    */
   if (NULL == (at_data = get_data_addr (at, indices)))
     return -1;

   return transfer_n_elements (at, new_data, at_data, sizeof_type, 1, is_ptr);
}

/* Here the ind_at index-array is a 1-d array of indices.  This function
 * creates a 1-d array of made up of values of 'at' at the locations
 * specified by the indices.  The result is pushed.
 */
static int
aget_from_index_array (SLang_Array_Type *at,
		       SLang_Array_Type *ind_at)
{
   SLang_Array_Type *new_at;
   int *indices, *indices_max;
   unsigned char *new_data, *src_data;
   unsigned int sizeof_type;
   int is_ptr;
   
   if (-1 == coerse_array_to_linear (at))
     return -1;

   if (-1 == coerse_array_to_linear (ind_at))
     return -1;

   if (-1 == check_index_array_ranges (at, ind_at))
     return -1;

   if (NULL == (new_at = SLang_create_array (at->data_type, 0, NULL, ind_at->dims, 1)))
     return -1;

   /* Since the index array is linear, I can address it directly */
   indices = (int *) ind_at->data;
   indices_max = indices + ind_at->num_elements;

   src_data = (unsigned char *) at->data;
   new_data = (unsigned char *) new_at->data;
   sizeof_type = new_at->sizeof_type;
   is_ptr = (new_at->flags & SLARR_DATA_VALUE_IS_POINTER);

   while (indices < indices_max)
     {
	unsigned int offset;

	offset = sizeof_type * (unsigned int)*indices;
	if (-1 == transfer_n_elements (at, (VOID_STAR) new_data,
				       (VOID_STAR) (src_data + offset),
				       sizeof_type, 1, is_ptr))
	  {
	     SLang_free_array (new_at);
	     return -1;
	  }

	new_data += sizeof_type;
	indices++;
     }

   return SLang_push_array (new_at, 1);
}

/* This is extremely ugly.  It is due to the fact that the index_objects
 * may contain ranges.  This is a utility function for the aget/aput
 * routines
 */
static int
convert_nasty_index_objs (SLang_Array_Type *at,
			  SLang_Object_Type *index_objs,
			  unsigned int num_indices,
			  int **index_data,
			  int *range_buf, int *range_delta_buf,
			  int *max_dims,
			  unsigned int *num_elements,
			  int *is_array, int is_dim_array[SLARRAY_MAX_DIMS])
{
   unsigned int i, total_num_elements;
   SLang_Array_Type *ind_at;

   if (num_indices != at->num_dims)
     {
	SLang_verror (SL_INVALID_PARM, "Array requires %u indices", at->num_dims);
	return -1;
     }

   *is_array = 0;
   total_num_elements = 1;
   for (i = 0; i < num_indices; i++)
     {
	int max_index, min_index;
	SLang_Object_Type *obj;
	int at_dims_i;

	at_dims_i = at->dims[i];
	obj = index_objs + i;
	range_delta_buf [i] = 0;

	if (obj->data_type == SLANG_INT_TYPE)
	  {
	     range_buf [i] = min_index = max_index = obj->v.int_val;
	     max_dims [i] = 1;
	     index_data[i] = range_buf + i;
	     is_dim_array[i] = 0;
	  }
	else
	  {
	     *is_array = 1;
	     is_dim_array[i] = 1;
	     ind_at = obj->v.array_val;

	     if (ind_at->flags & SLARR_DATA_VALUE_IS_RANGE)
	       {
		  SLarray_Range_Array_Type *r;
		  int delta;
		  int first_index, last_index;

		  r = (SLarray_Range_Array_Type *) ind_at->data;

		  /* In an array indexing context, range arrays have different
		   * semantics.  Consider a[[0:10]].  Clearly this means elements
		   * 0-10 of a.  But what does a[[0:-1]] mean?  By itself,
		   * [0:-1] is a null matrix [].  But, it is useful in an
		   * indexing context to allow -1 to refer to the last element
		   * of the array.  Similarly, [-3:-1] refers to the last 3
		   * elements.
		   * 
		   * However, [-1:-3] does not refer to any of the elements.
		   */
		  
		  /* FIXME: Priority=High; I think this is broken behavior
		   * and should be rethought.  That is, a[[0:-1]] should 
		   * specify no elements.   That is, the behavior should be:
		   * [0:9]     ==> first 9 elements
		   * [-3:-1]   ==> last 3 elements
		   * [0:-1]    ==> [] (no elements)
		   * [0:-1:-1] ==> [0, -1] ==> first and last elements
		   * [-1:-3]  ==> []
		   * 
		   * Unfortunately, this is going to be difficult to fix
		   * because of the way rubber ranges are stored:
		   *   [*] ==> [0:-1]
		   *
		   * Perhaps it is just best to document this behavior.
		   * Sigh.
		   */
		  if ((first_index = r->first_index) < 0)
		    {
		       if (at_dims_i != 0)
			 first_index = (at_dims_i + first_index) % at_dims_i;
		    }

		  if ((last_index = r->last_index) < 0)
		    {
		       if (at_dims_i != 0)
			 last_index = (at_dims_i + last_index) % at_dims_i;
		    }

		  delta = r->delta;

		  range_delta_buf [i] = delta;
		  range_buf[i] = first_index;

		  if (delta > 0)
		    {
		       if (first_index > last_index)
			 max_dims[i] = min_index = max_index = 0;
		       else
			 {
			    max_index = min_index = first_index;
			    while (max_index + delta <= last_index)
			      max_index += delta;
			    max_dims [i] = 1 + (max_index - min_index) / delta;
			 }
		    }
		  else
		    {
		       if (first_index < last_index)
			 max_dims[i] = min_index = max_index = 0;
		       else
			 {
			    min_index = max_index = first_index;
			    while (min_index + delta >= last_index)
			      min_index += delta;
			    max_dims [i] = 1 + (max_index - min_index) / (-delta);
			 }
		    }
	       }
	     else
	       {
		  int *tmp, *tmp_max;

		  if (0 == (max_dims[i] = ind_at->num_elements))
		    {
		       total_num_elements = 0;
		       break;
		    }

		  tmp = (int *) ind_at->data;
		  tmp_max = tmp + ind_at->num_elements;
		  index_data [i] = tmp;

		  min_index = max_index = *tmp;
		  while (tmp < tmp_max)
		    {
		       if (max_index > *tmp)
			 max_index = *tmp;
		       if (min_index < *tmp)
			 min_index = *tmp;

		       tmp++;
		    }
	       }
	  }

	if ((at_dims_i == 0) && (max_dims[i] == 0))
	  {
	     total_num_elements = 0;
	     continue;
	  }

	if (max_index < 0)
	  max_index += at_dims_i;
	if (min_index < 0)
	  min_index += at_dims_i;

	if ((min_index < 0) || (min_index >= at_dims_i)
	    || (max_index < 0) || (max_index >= at_dims_i))
	  {
	     SLang_verror (SL_INVALID_PARM, "Array index %u ([%d:%d]) out of allowed range [0->%d]",
			   i, min_index, max_index, at_dims_i);
	     return -1;
	  }

	total_num_elements = total_num_elements * max_dims[i];
     }

   *num_elements = total_num_elements;
   return 0;
}

/* This routine pushes a 1-d vector of values from 'at' indexed by
 * the objects 'index_objs'.  These objects can either be integers or
 * 1-d integer arrays.  The fact that the 1-d arrays can be ranges
 * makes this look ugly.
 */
static int
aget_from_indices (SLang_Array_Type *at,
		   SLang_Object_Type *index_objs, unsigned int num_indices)
{
   int *index_data [SLARRAY_MAX_DIMS];
   int range_buf [SLARRAY_MAX_DIMS];
   int range_delta_buf [SLARRAY_MAX_DIMS];
   int max_dims [SLARRAY_MAX_DIMS];
   unsigned int i, num_elements;
   SLang_Array_Type *new_at;
   int map_indices[SLARRAY_MAX_DIMS];
   int indices [SLARRAY_MAX_DIMS];
   unsigned int sizeof_type;
   int is_ptr, ret, is_array;
   char *new_data;
   SLang_Class_Type *cl;
   int is_dim_array[SLARRAY_MAX_DIMS];

   if (-1 == convert_nasty_index_objs (at, index_objs, num_indices,
				       index_data, range_buf, range_delta_buf,
				       max_dims, &num_elements, &is_array,
				       is_dim_array))
     return -1;

   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);
   sizeof_type = at->sizeof_type;

   cl = _SLclass_get_class (at->data_type);

   if ((is_array == 0) && (num_elements == 1))
     {
	new_data = (char *)cl->cl_transfer_buf;
	memset (new_data, 0, sizeof_type);
	new_at = NULL;
     }
   else
     {
	int i_num_elements = (int)num_elements;

	new_at = SLang_create_array (at->data_type, 0, NULL, &i_num_elements, 1);
	if (NULL == new_at)
	  return -1;
	if (num_elements == 0)
	  return SLang_push_array (new_at, 1);

	new_data = (char *)new_at->data;
     }

   SLMEMSET((char *) map_indices, 0, sizeof(map_indices));
   do
     {
	for (i = 0; i < num_indices; i++)
	  {
	     int j;

	     j = map_indices[i];

	     if (0 != range_delta_buf[i])
	       indices[i] = range_buf[i] + j * range_delta_buf[i];
	     else
	       indices[i] = index_data [i][j];
	  }

	if (-1 == _SLarray_aget_transfer_elem (at, indices, (VOID_STAR)new_data, sizeof_type, is_ptr))
	  {
	     SLang_free_array (new_at);
	     return -1;
	  }
	new_data += sizeof_type;
     }
   while (0 == _SLarray_next_index (map_indices, max_dims, num_indices));

   if (new_at != NULL)
     {
	int num_dims = 0;
	/* Fixup dimensions on array */
	for (i = 0; i < num_indices; i++)
	  {
	     if (is_dim_array[i]) /* was: (max_dims[i] > 1) */
	       {
		  new_at->dims[num_dims] = max_dims[i];
		  num_dims++;
	       }
	  }

	if (num_dims != 0) new_at->num_dims = num_dims;
	return SLang_push_array (new_at, 1);
     }

   /* Here new_data is a whole new copy, so free it after the push */
   new_data -= sizeof_type;
   if (is_ptr && (*(VOID_STAR *)new_data == NULL))
     ret = SLang_push_null ();
   else
     {
	ret = (*cl->cl_apush) (at->data_type, (VOID_STAR)new_data);
	(*cl->cl_adestroy) (at->data_type, (VOID_STAR)new_data);
     }

   return ret;
}

static int push_string_as_array (unsigned char *s, unsigned int len)
{
   int ilen;
   SLang_Array_Type *at;

   ilen = (int) len;

   at = SLang_create_array (SLANG_UCHAR_TYPE, 0, NULL, &ilen, 1);
   if (at == NULL)
     return -1;

   memcpy ((char *)at->data, (char *)s, len);
   return SLang_push_array (at, 1);
}

static int pop_array_as_string (char **sp)
{
   SLang_Array_Type *at;
   int ret;

   *sp = NULL;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_UCHAR_TYPE))
     return -1;

   ret = 0;

   if (NULL == (*sp = SLang_create_nslstring ((char *) at->data, at->num_elements)))
     ret = -1;

   SLang_free_array (at);
   return ret;
}

static int pop_array_as_bstring (SLang_BString_Type **bs)
{
   SLang_Array_Type *at;
   int ret;

   *bs = NULL;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_UCHAR_TYPE))
     return -1;

   ret = 0;

   if (NULL == (*bs = SLbstring_create ((unsigned char *) at->data, at->num_elements)))
     ret = -1;

   SLang_free_array (at);
   return ret;
}

static int aget_from_array (unsigned int num_indices)
{
   SLang_Array_Type *at;
   SLang_Object_Type index_objs [SLARRAY_MAX_DIMS];
   int ret;
   int is_index_array;
   unsigned int i;

   if (num_indices > SLARRAY_MAX_DIMS)
     {
	SLang_verror (SL_INVALID_PARM, "Number of dims must be less than %d", SLARRAY_MAX_DIMS);
	return -1;
     }

   if (-1 == pop_array (&at, 1))
     return -1;

   if (-1 == pop_indices (at, index_objs, num_indices, &is_index_array))
     {
	SLang_free_array (at);
	return -1;
     }

   if (is_index_array == 0)
     {
#if _SLANG_OPTIMIZE_FOR_SPEED
	if ((num_indices == 1) && (index_objs[0].data_type == SLANG_INT_TYPE)
	    && (0 == (at->flags & (SLARR_DATA_VALUE_IS_RANGE|SLARR_DATA_VALUE_IS_POINTER)))
	    && (1 == at->num_dims)
	    && (at->data != NULL))
	  {
	     int ofs = index_objs[0].v.int_val;
	     if (ofs < 0) ofs += at->dims[0];
	     if ((ofs >= at->dims[0]) || (ofs < 0))
	       ret = aget_from_indices (at, index_objs, num_indices);
	     else switch (at->data_type)
	       {
		case SLANG_INT_TYPE:
		  ret = SLclass_push_int_obj (SLANG_INT_TYPE, *((int *)at->data + ofs));
		  break;
#if SLANG_HAS_FLOAT
		case SLANG_DOUBLE_TYPE:
		  ret = SLclass_push_double_obj (SLANG_DOUBLE_TYPE, *((double *)at->data + ofs));
		  break;
#endif
		default:
		  ret = aget_from_indices (at, index_objs, num_indices);
	       }
	  }
	else
#endif
	ret = aget_from_indices (at, index_objs, num_indices);
     }
   else
     ret = aget_from_index_array (at, index_objs[0].v.array_val);

   SLang_free_array (at);
   for (i = 0; i < num_indices; i++)
     SLang_free_object (index_objs + i);

   return ret;
}

static int push_string_element (unsigned char type, unsigned char *s, unsigned int len)
{
   int i;

   if (SLang_peek_at_stack () == SLANG_ARRAY_TYPE)
     {
	char *str;

	/* The indices are array values.  So, do this: */
	if (-1 == push_string_as_array (s, len))
	  return -1;

	if (-1 == aget_from_array (1))
	  return -1;

	if (type == SLANG_BSTRING_TYPE)
	  {
	     SLang_BString_Type *bs;
	     int ret;

	     if (-1 == pop_array_as_bstring (&bs))
	       return -1;

	     ret = SLang_push_bstring (bs);
	     SLbstring_free (bs);
	     return ret;
	  }

	if (-1 == pop_array_as_string (&str))
	  return -1;
	return _SLang_push_slstring (str);   /* frees s upon error */
     }

   if (-1 == SLang_pop_integer (&i))
     return -1;

   if (i < 0) i = i + (int)len;
   if ((unsigned int) i > len)
     i = len;			       /* get \0 character --- bstrings include it as well */

   i = s[(unsigned int) i];

   return SLang_push_integer (i);
}

/* ARRAY[i, j, k] generates code: __args i j ...k ARRAY __aput/__aget
 * Here i, j, ... k may be a mixture of integers and 1-d arrays, or
 * a single array of indices.  The index array is generated by the
 * 'where' function.
 *
 * If ARRAY is of type DataType, then this function will create an array of
 * the appropriate type.  In that case, the indices i, j, ..., k must be
 * integers.
 */
int _SLarray_aget (void)
{
   unsigned int num_indices;
   int type;
   int (*aget_fun) (unsigned char, unsigned int);

   num_indices = (SLang_Num_Function_Args - 1);

   type = SLang_peek_at_stack ();
   switch (type)
     {
      case -1:
	return -1;		       /* stack underflow */

      case SLANG_DATATYPE_TYPE:
	return push_create_new_array ();

      case SLANG_BSTRING_TYPE:
	if (1 == num_indices)
	  {
	     SLang_BString_Type *bs;
	     int ret;
	     unsigned int len;
	     unsigned char *s;

	     if (-1 == SLang_pop_bstring (&bs))
	       return -1;

	     if (NULL == (s = SLbstring_get_pointer (bs, &len)))
	       ret = -1;
	     else
	       ret = push_string_element (type, s, len);

	     SLbstring_free (bs);
	     return ret;
	  }
	break;

      case SLANG_STRING_TYPE:
	if (1 == num_indices)
	  {
	     char *s;
	     int ret;

	     if (-1 == SLang_pop_slstring (&s))
	       return -1;

	     ret = push_string_element (type, (unsigned char *)s, strlen (s));
	     SLang_free_slstring (s);
	     return ret;
	  }
	break;

      case SLANG_ARRAY_TYPE:
	break;

      case SLANG_ASSOC_TYPE:
	return _SLassoc_aget (type, num_indices);

      default:
	aget_fun = _SLclass_get_class (type)->cl_aget;
	if (NULL != aget_fun)
	  return (*aget_fun) (type, num_indices);
     }

   return aget_from_array (num_indices);
}

int
_SLarray_aput_transfer_elem (SLang_Array_Type *at, int *indices,
			     VOID_STAR data_to_put, unsigned int sizeof_type, int is_ptr)
{
   VOID_STAR at_data;

   /* Since 1 element is being transferred, there is no need to coerse
    * the array to linear.
    */
   if (NULL == (at_data = get_data_addr (at, indices)))
     return -1;

   return transfer_n_elements (at, at_data, data_to_put, sizeof_type, 1, is_ptr);
}

static int
aput_get_array_to_put (SLang_Class_Type *cl, unsigned int num_elements, int allow_array,
		       SLang_Array_Type **at_ptr, char **data_to_put, unsigned int *data_increment)
{
   unsigned char data_type;
   int type;
   SLang_Array_Type *at;
   
   *at_ptr = NULL;

   data_type = cl->cl_data_type;
   type = SLang_peek_at_stack ();

   if (type != data_type)
     {
	if ((type != SLANG_NULL_TYPE)
	    || ((cl->cl_class_type != SLANG_CLASS_TYPE_PTR)
		&& (cl->cl_class_type != SLANG_CLASS_TYPE_MMT)))
	  {
	     if (-1 == SLclass_typecast (data_type, 1, allow_array))
	       return -1;
	  }
	else
	  {
	     /* This bit of code allows, e.g., a[10] = NULL; */
	     *data_increment = 0;
	     *data_to_put = (char *) cl->cl_transfer_buf;
	     *((char **)cl->cl_transfer_buf) = NULL;
	     return SLdo_pop ();
	  }
     }

   if ((data_type != SLANG_ARRAY_TYPE)
       && (data_type != SLANG_ANY_TYPE)
       && (SLANG_ARRAY_TYPE == SLang_peek_at_stack ()))
     {
	if (-1 == SLang_pop_array (&at, 0))
	  return -1;

	if ((at->num_elements != num_elements)
#if 0
	    || (at->num_dims != 1)
#endif
	    )
	  {
	     SLang_verror (SL_TYPE_MISMATCH, "Array size is inappropriate for use with index-array");
	     SLang_free_array (at);
	     return -1;
	  }

	*data_to_put = (char *) at->data;
	*data_increment = at->sizeof_type;
	*at_ptr = at;
	return 0;
     }

   *data_increment = 0;
   *data_to_put = (char *) cl->cl_transfer_buf;

   if (-1 == (*cl->cl_apop)(data_type, (VOID_STAR) *data_to_put))
     return -1;

   return 0;
}

static int
aput_from_indices (SLang_Array_Type *at,
		   SLang_Object_Type *index_objs, unsigned int num_indices)
{
   int *index_data [SLARRAY_MAX_DIMS];
   int range_buf [SLARRAY_MAX_DIMS];
   int range_delta_buf [SLARRAY_MAX_DIMS];
   int max_dims [SLARRAY_MAX_DIMS];
   unsigned int i, num_elements;
   SLang_Array_Type *bt;
   int map_indices[SLARRAY_MAX_DIMS];
   int indices [SLARRAY_MAX_DIMS];
   unsigned int sizeof_type;
   int is_ptr, is_array, ret;
   char *data_to_put;
   unsigned int data_increment;
   SLang_Class_Type *cl;
   int is_dim_array [SLARRAY_MAX_DIMS];

   if (-1 == convert_nasty_index_objs (at, index_objs, num_indices,
				       index_data, range_buf, range_delta_buf,
				       max_dims, &num_elements, &is_array,
				       is_dim_array))
     return -1;

   cl = at->cl;

   if (-1 == aput_get_array_to_put (cl, num_elements, is_array,
				    &bt, &data_to_put, &data_increment))
     return -1;

   sizeof_type = at->sizeof_type;
   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);

   ret = -1;

   SLMEMSET((char *) map_indices, 0, sizeof(map_indices));
   if (num_elements) do
     {
	for (i = 0; i < num_indices; i++)
	  {
	     int j;

	     j = map_indices[i];

	     if (0 != range_delta_buf[i])
	       indices[i] = range_buf[i] + j * range_delta_buf[i];
	     else
	       indices[i] = index_data [i][j];
	  }

	if (-1 == _SLarray_aput_transfer_elem (at, indices, (VOID_STAR)data_to_put, sizeof_type, is_ptr))
	  goto return_error;

	data_to_put += data_increment;
     }
   while (0 == _SLarray_next_index (map_indices, max_dims, num_indices));

   ret = 0;

   /* drop */

   return_error:
   if (bt == NULL)
     {
	if (is_ptr)
	  (*cl->cl_destroy) (cl->cl_data_type, (VOID_STAR) data_to_put);
     }
   else SLang_free_array (bt);

   return ret;
}

static int
aput_from_index_array (SLang_Array_Type *at, SLang_Array_Type *ind_at)
{
   int *indices, *indices_max;
   unsigned int sizeof_type;
   char *data_to_put, *dest_data;
   unsigned int data_increment;
   int is_ptr;
   SLang_Array_Type *bt;
   SLang_Class_Type *cl;
   int ret;

   if (-1 == coerse_array_to_linear (at))
     return -1;

   if (-1 == coerse_array_to_linear (ind_at))
     return -1;

   if (-1 == check_index_array_ranges (at, ind_at))
     return -1;

   sizeof_type = at->sizeof_type;

   cl = at->cl;

   /* Note that if bt is returned as non NULL, then the array is a linear
    * one.
    */
   if (-1 == aput_get_array_to_put (cl, ind_at->num_elements, 1,
				    &bt, &data_to_put, &data_increment))
     return -1;

   /* Since the index array is linear, I can address it directly */
   indices = (int *) ind_at->data;
   indices_max = indices + ind_at->num_elements;

   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);
   dest_data = (char *) at->data;

   ret = -1;
   while (indices < indices_max)
     {
	unsigned int offset;

	offset = sizeof_type * (unsigned int)*indices;

	if (-1 == transfer_n_elements (at, (VOID_STAR) (dest_data + offset),
				       (VOID_STAR) data_to_put, sizeof_type, 1,
				       is_ptr))
	  goto return_error;

	indices++;
	data_to_put += data_increment;
     }

   ret = 0;
   /* Drop */

   return_error:

   if (bt == NULL)
     {
	if (is_ptr)
	  (*cl->cl_destroy) (cl->cl_data_type, (VOID_STAR)data_to_put);
     }
   else SLang_free_array (bt);

   return ret;
}

/* ARRAY[i, j, k] = generates code: __args i j k ARRAY __aput
 */
int _SLarray_aput (void)
{
   unsigned int num_indices;
   SLang_Array_Type *at;
   SLang_Object_Type index_objs [SLARRAY_MAX_DIMS];
   int ret;
   int is_index_array;
   int (*aput_fun) (unsigned char, unsigned int);
   int type;

   ret = -1;
   num_indices = (SLang_Num_Function_Args - 1);

   type = SLang_peek_at_stack ();
   switch (type)
     {
      case -1:
	return -1;

      case SLANG_ARRAY_TYPE:
	break;

      case SLANG_ASSOC_TYPE:
	return _SLassoc_aput (type, num_indices);

      default:
	if (NULL != (aput_fun = _SLclass_get_class (type)->cl_aput))
	  return (*aput_fun) (type, num_indices);
	break;
     }

   if (-1 == SLang_pop_array (&at, 0))
     return -1;

   if (at->flags & SLARR_DATA_VALUE_IS_READ_ONLY)
     {
	SLang_verror (SL_READONLY_ERROR, "%s Array is read-only",
		      SLclass_get_datatype_name (at->data_type));
	SLang_free_array (at);
	return -1;
     }

   if (-1 == pop_indices (at, index_objs, num_indices, &is_index_array))
     {
	SLang_free_array (at);
	return -1;
     }

   if (is_index_array == 0)
     {
#if _SLANG_OPTIMIZE_FOR_SPEED
	if ((num_indices == 1) && (index_objs[0].data_type == SLANG_INT_TYPE)
	    && (0 == (at->flags & (SLARR_DATA_VALUE_IS_RANGE|SLARR_DATA_VALUE_IS_POINTER)))
	    && (1 == at->num_dims)
	    && (at->data != NULL))
	  {
	     int ofs = index_objs[0].v.int_val;
	     if (ofs < 0) ofs += at->dims[0];
	     if ((ofs >= at->dims[0]) || (ofs < 0))
	       ret = aput_from_indices (at, index_objs, num_indices);
	     else switch (at->data_type)
	       {
		case SLANG_INT_TYPE:
		  ret = SLang_pop_integer (((int *)at->data + ofs));
		  break;
#if SLANG_HAS_FLOAT
		case SLANG_DOUBLE_TYPE:
		  ret = SLang_pop_double ((double *)at->data + ofs, NULL, NULL);
		  break;
#endif
		default:
		  ret = aput_from_indices (at, index_objs, num_indices);
	       }
	  }
	else
#endif
	  ret = aput_from_indices (at, index_objs, num_indices);
     }
   else
     ret = aput_from_index_array (at, index_objs[0].v.array_val);

   SLang_free_array (at);
   free_index_objects (index_objs, num_indices);
   return ret;
}

/* This is for 1-d matrices only.  It is used by the sort function */
static int push_element_at_index (SLang_Array_Type *at, int indx)
{
   VOID_STAR data;

   if (NULL == (data = get_data_addr (at, &indx)))
     return -1;

   return push_element_at_addr (at, (VOID_STAR) data, 1);
}

static SLang_Name_Type *Sort_Function;
static SLang_Array_Type *Sort_Array;

#if _SLANG_OPTIMIZE_FOR_SPEED
static int double_sort_fun (int *a, int *b)
{
   double *da, *db;
   
   da = (double *) Sort_Array->data;
   db = da + *b;
   da = da + *a;

   if (*da > *db) return 1;
   if (*da == *db) return 0;
   return -1;
}
static int int_sort_fun (int *a, int *b)
{
   int *da, *db;
   
   da = (int *) Sort_Array->data;
   db = da + *b;
   da = da + *a;

   if (*da > *db) return 1;
   if (*da == *db) return 0;
   return -1;
}
#endif

static int sort_cmp_fun (int *a, int *b)
{
   int cmp;

   if (SLang_Error
       || (-1 == push_element_at_index (Sort_Array, *a))
       || (-1 == push_element_at_index (Sort_Array, *b))
       || (-1 == SLexecute_function (Sort_Function))
       || (-1 == SLang_pop_integer (&cmp)))
     {
	/* DO not allow qsort to loop forever.  Return something meaningful */
	if (*a > *b) return 1;
	if (*a < *b) return -1;
	return 0;
     }

   return cmp;
}

static int builtin_sort_cmp_fun (int *a, int *b)
{
   VOID_STAR a_data;
   VOID_STAR b_data;
   SLang_Class_Type *cl;
   
   cl = Sort_Array->cl;

   if ((SLang_Error == 0)
       && (NULL != (a_data = get_data_addr (Sort_Array, a)))
       && (NULL != (b_data = get_data_addr (Sort_Array, b))))
     {
	int cmp;

	if ((Sort_Array->flags & SLARR_DATA_VALUE_IS_POINTER)
	    && ((*(VOID_STAR *) a_data == NULL) || (*(VOID_STAR *) a_data == NULL)))
	  {
	     SLang_verror (SL_VARIABLE_UNINITIALIZED,
			   "%s array has unitialized element", cl->cl_name);
	  }
	else if (0 == (*cl->cl_cmp)(Sort_Array->data_type, a_data, b_data, &cmp))
	  return cmp;
     }
       
       
   if (*a > *b) return 1;
   if (*a == *b) return 0;
   return -1;
}

static void sort_array_internal (SLang_Array_Type *at_str, 
				 SLang_Name_Type *entry,
				 int (*sort_fun)(int *, int *))
{
   SLang_Array_Type *ind_at;
   /* This is a silly hack to make up for braindead compilers and the lack of
    * uniformity in prototypes for qsort.
    */
   void (*qsort_fun) (char *, unsigned int, int, int (*)(int *, int *));
   int *indx;
   int i, n;
   int dims[1];

   if (Sort_Array != NULL)
     {
	SLang_verror (SL_NOT_IMPLEMENTED, "array_sort is not recursive");
	return;
     }

   n = at_str->num_elements;

   if (at_str->num_dims != 1)
     {
	SLang_verror (SL_INVALID_PARM, "sort is restricted to 1 dim arrays");
	return;
     }

   dims [0] = n;

   if (NULL == (ind_at = SLang_create_array (SLANG_INT_TYPE, 0, NULL, dims, 1)))
     return;

   indx = (int *) ind_at->data;
   for (i = 0; i < n; i++) indx[i] = i;

   if (n > 1)
     {
	qsort_fun = (void (*)(char *, unsigned int, int, int (*)(int *,
								 int *)))
	  qsort;

	Sort_Array = at_str;
	Sort_Function = entry;
	(*qsort_fun) ((char *) indx, n, sizeof (int), sort_fun);
     }

   Sort_Array = NULL;
   (void) SLang_push_array (ind_at, 1);
}

static void sort_array (void)
{
   SLang_Name_Type *entry;
   SLang_Array_Type *at;
   int (*sort_fun) (int *, int *);

   if (SLang_Num_Function_Args != 1)
     {
	sort_fun = sort_cmp_fun;

	if (NULL == (entry = SLang_pop_function ()))
	  return;

	if (-1 == SLang_pop_array (&at, 1))
	  return;
     }
   else
     {
	if (-1 == SLang_pop_array (&at, 1))
	  return;
	
#if _SLANG_OPTIMIZE_FOR_SPEED
	if (at->data_type == SLANG_DOUBLE_TYPE)
	  sort_fun = double_sort_fun;
	else if (at->data_type == SLANG_INT_TYPE)
	  sort_fun = int_sort_fun;
	else
#endif
	  sort_fun = builtin_sort_cmp_fun;

	if (at->cl->cl_cmp == NULL)
	  {
	     SLang_verror (SL_NOT_IMPLEMENTED, 
			   "%s does not have a predefined sorting method",
			   at->cl->cl_name);
	     SLang_free_array (at);
	     return;
	  }
	entry = NULL;
     }

   sort_array_internal (at, entry, sort_fun);
   SLang_free_array (at);
   SLang_free_function (entry);
}

static void bstring_to_array (SLang_BString_Type *bs)
{
   unsigned char *s;
   unsigned int len;
   
   if (NULL == (s = SLbstring_get_pointer (bs, &len)))
     (void) SLang_push_null ();
   else
     (void) push_string_as_array (s, len);
}

static void array_to_bstring (SLang_Array_Type *at)
{
   unsigned int nbytes;
   SLang_BString_Type *bs;

   nbytes = at->num_elements * at->sizeof_type;
   bs = SLbstring_create ((unsigned char *)at->data, nbytes);
   (void) SLang_push_bstring (bs);
   SLbstring_free (bs);
}

static void init_char_array (void)
{
   SLang_Array_Type *at;
   char *s;
   unsigned int n, ndim;

   if (SLang_pop_slstring (&s)) return;

   if (-1 == SLang_pop_array (&at, 0))
     goto free_and_return;

   if (at->data_type != SLANG_CHAR_TYPE)
     {
	SLang_doerror("Operation requires character array");
	goto free_and_return;
     }

   n = strlen (s);
   ndim = at->num_elements;
   if (n > ndim)
     {
	SLang_doerror("String too big to init array");
	goto free_and_return;
     }

   strncpy((char *) at->data, s, ndim);
   /* drop */

   free_and_return:
   SLang_free_array (at);
   SLang_free_slstring (s);
}

static void array_info (void)
{
   SLang_Array_Type *at, *bt;
   int num_dims;

   if (-1 == pop_array (&at, 1))
     return;

   num_dims = (int)at->num_dims;

   if (NULL != (bt = SLang_create_array (SLANG_INT_TYPE, 0, NULL, &num_dims, 1)))
     {
	int *bdata;
	int i;
	int *a_dims;

	a_dims = at->dims;
	bdata = (int *) bt->data;
	for (i = 0; i < num_dims; i++) bdata [i] = a_dims [i];

	if (0 == SLang_push_array (bt, 1))
	  {
	     (void) SLang_push_integer ((int) at->num_dims);
	     (void) SLang_push_datatype (at->data_type);
	  }
     }

   SLang_free_array (at);
}

static VOID_STAR range_get_data_addr (SLang_Array_Type *at, int *dims)
{
   static int value;
   SLarray_Range_Array_Type *r;
   int d;

   d = *dims;
   r = (SLarray_Range_Array_Type *)at->data;

   if (d < 0)
     d += at->dims[0];

   value = r->first_index + d * r->delta;
   return (VOID_STAR) &value;
}

static SLang_Array_Type *inline_implicit_int_array (int *xminptr, int *xmaxptr, int *dxptr)
{
   int delta;
   SLang_Array_Type *at;
   int dims, idims;
   SLarray_Range_Array_Type *data;

   if (dxptr == NULL) delta = 1;
   else delta = *dxptr;

   if (delta == 0)
     {
	SLang_verror (SL_INVALID_PARM, "range-array increment must be non-zero");
	return NULL;
     }

   data = (SLarray_Range_Array_Type *) SLmalloc (sizeof (SLarray_Range_Array_Type));
   if (data == NULL)
     return NULL;

   SLMEMSET((char *) data, 0, sizeof (SLarray_Range_Array_Type));
   data->delta = delta;
   dims = 0;

   if (xminptr != NULL)
     data->first_index = *xminptr;
   else
     data->first_index = 0;

   if (xmaxptr != NULL)
     data->last_index = *xmaxptr;
   else
     data->last_index = -1;

/*   if ((xminptr != NULL) && (xmaxptr != NULL))
     { */
	idims = 1 + (data->last_index - data->first_index) / delta;
	if (idims > 0)
	  dims = idims;
    /* } */

   if (NULL == (at = SLang_create_array (SLANG_INT_TYPE, 0, (VOID_STAR) data, &dims, 1)))
     return NULL;

   at->index_fun = range_get_data_addr;
   at->flags |= SLARR_DATA_VALUE_IS_RANGE;

   return at;
}

#if SLANG_HAS_FLOAT
static SLang_Array_Type *inline_implicit_floating_array (unsigned char type,
							 double *xminptr, double *xmaxptr, double *dxptr)
{
   int n, i;
   SLang_Array_Type *at;
   int dims;
   double xmin, xmax, dx;

   if ((xminptr == NULL) || (xmaxptr == NULL))
     {
	SLang_verror (SL_INVALID_PARM, "range-array has unknown size");
	return NULL;
     }
   xmin = *xminptr;
   xmax = *xmaxptr;
   if (dxptr == NULL) dx = 1.0;
   else dx = *dxptr;

   if (dx == 0.0)
     {
	SLang_doerror ("range-array increment must be non-zero");
	return NULL;
     }

   /* I have convinced myself that it is better to use semi-open intervals
    * because of less ambiguities.  So, [a:b:c] will represent the set of
    * values a, a + c, a + 2c ... a + nc
    * such that a + nc < b.  That is, b lies outside the interval.
    */

   /* Allow for roundoff by adding 0.5 before truncation */
   n = (int)(1.5 + ((xmax - xmin) / dx));
   if (n <= 0)
     n = 0;
   else
     {
	double last = xmin + (n-1) * dx;

	if (dx > 0.0)
	  {
	     if (last >= xmax)
	       n -= 1;
	  }
	else if (last <= xmax)
	  n -= 1;
     }

   dims = n;
   if (NULL == (at = SLang_create_array1 (type, 0, NULL, &dims, 1, 1)))
     return NULL;

   if (type == SLANG_DOUBLE_TYPE)
     {
	double *ptr;

	ptr = (double *) at->data;

	for (i = 0; i < n; i++)
	  ptr[i] = xmin + i * dx;
     }
   else
     {
	float *ptr;

	ptr = (float *) at->data;

	for (i = 0; i < n; i++)
	  ptr[i] = (float) (xmin + i * dx);
     }
   return at;
}
#endif

/* FIXME: Priority=medium
 * This needs to be updated to work with all integer types.
 */
int _SLarray_inline_implicit_array (void)
{
   int int_vals[3];
#if SLANG_HAS_FLOAT
   double double_vals[3];
   int is_int;
#endif
   int has_vals[3];
   unsigned int i, count;
   SLang_Array_Type *at;
   int precedence;
   unsigned char type;

   count = SLang_Num_Function_Args;

   if (count == 2)
     has_vals [2] = 0;
   else if (count != 3)
     {
	SLang_doerror ("wrong number of arguments to __implicit_inline_array");
	return -1;
     }

#if SLANG_HAS_FLOAT
   is_int = 1;
#endif

   type = 0;
   precedence = 0;

   i = count;
   while (i--)
     {
	int this_type, this_precedence;

	if (-1 == (this_type = SLang_peek_at_stack ()))
	  return -1;

	this_precedence = _SLarith_get_precedence ((unsigned char) this_type);
	if (precedence < this_precedence)
	  {
	     type = (unsigned char) this_type;
	     precedence = this_precedence;
	  }

	has_vals [i] = 1;

	switch (this_type)
	  {
	   case SLANG_NULL_TYPE:
	     has_vals[i] = 0;
	     (void) SLdo_pop ();
	     break;

#if SLANG_HAS_FLOAT
	   case SLANG_DOUBLE_TYPE:
	   case SLANG_FLOAT_TYPE:
	     if (-1 == SLang_pop_double (double_vals + i, NULL, NULL))
	       return -1;
	     is_int = 0;
	     break;
#endif
	   default:
	     if (-1 == SLang_pop_integer (int_vals + i))
	       return -1;
#if SLANG_HAS_FLOAT
	     double_vals[i] = (double) int_vals[i];
#endif
	  }
     }

#if SLANG_HAS_FLOAT
   if (is_int == 0)
     at = inline_implicit_floating_array (type,
					  (has_vals[0] ? &double_vals[0] : NULL),
					  (has_vals[1] ? &double_vals[1] : NULL),
					  (has_vals[2] ? &double_vals[2] : NULL));
   else
#endif
     at = inline_implicit_int_array ((has_vals[0] ? &int_vals[0] : NULL),
				     (has_vals[1] ? &int_vals[1] : NULL),
				     (has_vals[2] ? &int_vals[2] : NULL));

   if (at == NULL)
     return -1;

   return SLang_push_array (at, 1);
}

int _SLarray_wildcard_array (void)
{
   SLang_Array_Type *at;
     
   if (NULL == (at = inline_implicit_int_array (NULL, NULL, NULL)))
     return -1;

   return SLang_push_array (at, 1);
}

static SLang_Array_Type *concat_arrays (unsigned int count)
{
   SLang_Array_Type **arrays;
   SLang_Array_Type *at, *bt;
   unsigned int i;
   int num_elements;
   unsigned char type;
   char *src_data, *dest_data;
   int is_ptr;
   unsigned int sizeof_type;
   int max_dims, min_dims, max_rows, min_rows;

   arrays = (SLang_Array_Type **)SLmalloc (count * sizeof (SLang_Array_Type *));
   if (arrays == NULL)
     {
	SLdo_pop_n (count);
	return NULL;
     }
   SLMEMSET((char *) arrays, 0, count * sizeof(SLang_Array_Type *));

   at = NULL;

   num_elements = 0;
   i = count;

   while (i != 0)
     {
	i--;

	if (-1 == SLang_pop_array (&bt, 1))
	  goto free_and_return;

	arrays[i] = bt;
	num_elements += (int)bt->num_elements;
     }

   type = arrays[0]->data_type;
   max_dims = min_dims = arrays[0]->num_dims;
   min_rows = max_rows = arrays[0]->dims[0];

   for (i = 1; i < count; i++)
     {
	SLang_Array_Type *ct;
	int num;

	bt = arrays[i];

	num = bt->num_dims;
	if (num > max_dims) max_dims = num;
	if (num < min_dims) min_dims = num;

	num = bt->dims[0];
	if (num > max_rows) max_rows = num;
	if (num < min_rows) min_rows = num;

	if (type == bt->data_type)
	  continue;

	if (1 != _SLarray_typecast (bt->data_type, (VOID_STAR) &bt, 1,
				    type, (VOID_STAR) &ct, 1))
	  goto free_and_return;

	SLang_free_array (bt);
	arrays [i] = ct;
     }

   if (NULL == (at = SLang_create_array (type, 0, NULL, &num_elements, 1)))
     goto free_and_return;

   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);
   sizeof_type = at->sizeof_type;
   dest_data = (char *) at->data;

   for (i = 0; i < count; i++)
     {
	bt = arrays[i];

	src_data = (char *) bt->data;
	num_elements = bt->num_elements;

	if (-1 == transfer_n_elements (bt, (VOID_STAR)dest_data, (VOID_STAR)src_data, sizeof_type,
				       num_elements, is_ptr))
	  {
	     SLang_free_array (at);
	     at = NULL;
	     goto free_and_return;
	  }

	dest_data += num_elements * sizeof_type;
     }

#if 0   
   /* If the arrays are all 1-d, and all the same size, then reshape to a
    * 2-d array.  This will allow us to do, e.g.
    * a = [[1,2], [3,4]]
    * to specifiy a 2-d.
    * Someday I will generalize this.
    */
   /* This is a bad idea.  Everyone using it expects concatenation to happen.
    * Perhaps I will extend the syntax to allow a 2-d array to be expressed
    * as [[1,2];[3,4]].
    */
   if ((max_dims == min_dims) && (max_dims == 1) && (min_rows == max_rows))
     {
	at->num_dims = 2;
	at->dims[0] = count;
	at->dims[1] = min_rows;
     }
#endif
   free_and_return:

   for (i = 0; i < count; i++)
     SLang_free_array (arrays[i]);
   SLfree ((char *) arrays);

   return at;
}

int _SLarray_inline_array (void)
{
   SLang_Object_Type *obj, *objmin;
   unsigned char type, this_type;
   unsigned int count;
   SLang_Array_Type *at;

   obj = _SLang_get_run_stack_pointer ();
   objmin = _SLang_get_run_stack_base ();

   count = SLang_Num_Function_Args;
   type = 0;

   while ((count > 0) && (--obj >= objmin))
     {
	this_type = obj->data_type;

	if (type == 0)
	  type = this_type;

	if ((type == this_type) || (type == SLANG_ARRAY_TYPE))
	  {
	     count--;
	     continue;
	  }

	switch (this_type)
	  {
	   case SLANG_ARRAY_TYPE:
	     type = SLANG_ARRAY_TYPE;
	     break;
#if SLANG_HAS_COMPLEX
	   case SLANG_COMPLEX_TYPE:
	     if (0 == _SLang_is_arith_type (type))
	       goto type_mismatch;
	     
	     type = this_type;
	     break;
#endif
	   default:
	     if (0 == _SLang_is_arith_type(this_type))
	       goto type_mismatch;
	     
	     if (type == SLANG_COMPLEX_TYPE)
	       break;
	     
	     if (0 == _SLang_is_arith_type (type))
	       goto type_mismatch;
	     
	     if (_SLarith_get_precedence (this_type) > _SLarith_get_precedence (type))
	       type = this_type;
	     break;
	  }
	count--;
     }

   if (count != 0)
     {
	SLang_Error = SL_STACK_UNDERFLOW;
	return -1;
     }

   count = SLang_Num_Function_Args;

   if (count == 0)
     {
	SLang_verror (SL_NOT_IMPLEMENTED, "Empty inline-arrays not supported");
	return -1;
     }

   if (type == SLANG_ARRAY_TYPE)
     {
	if (NULL == (at = concat_arrays (count)))
	  return -1;
     }
   else
     {
	SLang_Object_Type index_obj;
	int icount = (int) count;

	if (NULL == (at = SLang_create_array (type, 0, NULL, &icount, 1)))
	  return -1;

	index_obj.data_type = SLANG_INT_TYPE;
	while (count != 0)
	  {
	     count--;
	     index_obj.v.int_val = (int) count;
	     if (-1 == aput_from_indices (at, &index_obj, 1))
	       {
		  SLang_free_array (at);
		  SLdo_pop_n (count);
		  return -1;
	       }
	  }
     }

   return SLang_push_array (at, 1);

   type_mismatch:
   _SLclass_type_mismatch_error (type, this_type);
   return -1;
}

static int array_binary_op_result (int op, unsigned char a, unsigned char b,
				   unsigned char *c)
{
   (void) op;
   (void) a;
   (void) b;
   *c = SLANG_ARRAY_TYPE;
   return 1;
}

static int array_binary_op (int op,
			    unsigned char a_type, VOID_STAR ap, unsigned int na,
			    unsigned char b_type, VOID_STAR bp, unsigned int nb,
			    VOID_STAR cp)
{
   SLang_Array_Type *at, *bt, *ct;
   unsigned int i, num_dims;
   int (*binary_fun) (int,
		      unsigned char, VOID_STAR, unsigned int,
		      unsigned char, VOID_STAR, unsigned int,
		      VOID_STAR);
   SLang_Class_Type *a_cl, *b_cl, *c_cl;
   int no_init;

   if (a_type == SLANG_ARRAY_TYPE)
     {
	if (na != 1)
	  {
	     SLang_verror (SL_NOT_IMPLEMENTED, "Binary operation on multiple arrays not implemented");
	     return -1;
	  }

	at = *(SLang_Array_Type **) ap;
	if (-1 == coerse_array_to_linear (at))
	  return -1;
	ap = at->data;
	a_type = at->data_type;
	na = at->num_elements;
     }
   else
     {
	at = NULL;
     }

   if (b_type == SLANG_ARRAY_TYPE)
     {
	if (nb != 1)
	  {
	     SLang_verror (SL_NOT_IMPLEMENTED, "Binary operation on multiple arrays not implemented");
	     return -1;
	  }

	bt = *(SLang_Array_Type **) bp;
	if (-1 == coerse_array_to_linear (bt))
	  return -1;
	bp = bt->data;
	b_type = bt->data_type;
	nb = bt->num_elements;
     }
   else
     {
	bt = NULL;
     }

   if ((at != NULL) && (bt != NULL))
     {
	num_dims = at->num_dims;

	if (num_dims != bt->num_dims)
	  {
	     SLang_verror (SL_TYPE_MISMATCH, "Arrays must have same dim for binary operation");
	     return -1;
	  }

	for (i = 0; i < num_dims; i++)
	  {
	     if (at->dims[i] != bt->dims[i])
	       {
		  SLang_verror (SL_TYPE_MISMATCH, "Arrays must be the same for binary operation");
		  return -1;
	       }
	  }
     }

   a_cl = _SLclass_get_class (a_type);
   b_cl = _SLclass_get_class (b_type);

   if (NULL == (binary_fun = _SLclass_get_binary_fun (op, a_cl, b_cl, &c_cl, 1)))
     return -1;

   no_init = ((c_cl->cl_class_type == SLANG_CLASS_TYPE_SCALAR)
	      || (c_cl->cl_class_type == SLANG_CLASS_TYPE_VECTOR));

   ct = NULL;
#if _SLANG_USE_TMP_OPTIMIZATION
   /* If we are dealing with scalar (or vector) objects, and if the object
    * appears to be owned by the stack, then use it instead of creating a 
    * new version.  This can happen with code such as:
    * @  x = [1,2,3,4];
    * @  x = __tmp(x) + 1;
    */
   if (no_init)
     {
	if ((at != NULL) 
	    && (at->num_refs == 1)
	    && (at->data_type == c_cl->cl_data_type))
	  {
	     ct = at;
	     ct->num_refs = 2;
	  }
	else if ((bt != NULL) 
	    && (bt->num_refs == 1)
	    && (bt->data_type == c_cl->cl_data_type))
	  {
	     ct = bt;
	     ct->num_refs = 2;
	  }
     }
#endif				       /* _SLANG_USE_TMP_OPTIMIZATION */
   
   if (ct == NULL)
     {
	if (at != NULL) ct = at; else ct = bt;
	ct = SLang_create_array1 (c_cl->cl_data_type, 0, NULL, ct->dims, ct->num_dims, no_init);
	if (ct == NULL)
	  return -1;
     }


   if ((na == 0) || (nb == 0)	       /* allow empty arrays */
       || (1 == (*binary_fun) (op, a_type, ap, na, b_type, bp, nb, ct->data)))
     {
	*(SLang_Array_Type **) cp = ct;
	return 1;
     }

   SLang_free_array (ct);
   return -1;
}

static void is_null_intrinsic (SLang_Array_Type *at)
{
   SLang_Array_Type *bt;

   bt = SLang_create_array (SLANG_CHAR_TYPE, 0, NULL, at->dims, at->num_dims);
   if (bt == NULL)
     return;

   if (at->flags & SLARR_DATA_VALUE_IS_POINTER)
     {
	char *cdata, *cdata_max;
	char **data;

	if (-1 == coerse_array_to_linear (at))
	  {
	     SLang_free_array (bt);
	     return;
	  }
	
	cdata = (char *)bt->data;
	cdata_max = cdata + bt->num_elements;
	data = (char **)at->data;
	
	while (cdata < cdata_max)
	  {
	     if (*data == NULL)
	       *cdata = 1;
	     
	     data++;
	     cdata++;
	  }
     }
   
   SLang_push_array (bt, 1);
}
   
   
static void array_where (void)
{
   SLang_Array_Type *at, *bt;
   char *a_data;
   int *b_data;
   unsigned int i, num_elements;
   int b_num;

   if (-1 == SLang_pop_array (&at, 1))
     return;

   bt = NULL;

   if (at->data_type != SLANG_CHAR_TYPE)
     {
	int zero;
	SLang_Array_Type *tmp_at;

	tmp_at = at;
	zero = 0;
	if (1 != array_binary_op (SLANG_NE,
				  SLANG_ARRAY_TYPE, (VOID_STAR) &at, 1,
				  SLANG_CHAR_TYPE, (VOID_STAR) &zero, 1,
				  (VOID_STAR) &tmp_at))
	    goto return_error;

	SLang_free_array (at);
	at = tmp_at;
	if (at->data_type != SLANG_CHAR_TYPE)
	  {
	     SLang_Error = SL_TYPE_MISMATCH;
	     goto return_error;
	  }
     }

   a_data = (char *) at->data;
   num_elements = at->num_elements;

   b_num = 0;
   for (i = 0; i < num_elements; i++)
     if (a_data[i] != 0) b_num++;

   if (NULL == (bt = SLang_create_array1 (SLANG_INT_TYPE, 0, NULL, &b_num, 1, 1)))
     goto return_error;

   b_data = (int *) bt->data;

   i = 0;
   while (b_num)
     {
	if (a_data[i] != 0)
	  {
	     *b_data++ = i;
	     b_num--;
	  }

	i++;
     }

   (void) SLang_push_array (bt, 0);
   /* drop */

   return_error:
   SLang_free_array (at);
   SLang_free_array (bt);
}

static int do_array_reshape (SLang_Array_Type *at, SLang_Array_Type *ind_at)
{
   int *dims;
   unsigned int i, num_dims;
   unsigned int num_elements;

   if ((ind_at->data_type != SLANG_INT_TYPE)
       || (ind_at->num_dims != 1))
     {
	SLang_verror (SL_TYPE_MISMATCH, "Expecting 1-d integer array");
	return -1;
     }

   num_dims = ind_at->num_elements;
   dims = (int *) ind_at->data;

   num_elements = 1;
   for (i = 0; i < num_dims; i++)
     {
	int d = dims[i];
	if (d < 0)
	  {
	     SLang_verror (SL_INVALID_PARM, "reshape: dimension is less then 0");
	     return -1;
	  }

	num_elements = (unsigned int) d * num_elements;
     }

   if ((num_elements != at->num_elements)
       || (num_dims > SLARRAY_MAX_DIMS))
     {
	SLang_verror (SL_INVALID_PARM, "Unable to reshape array to specified size");
	return -1;
     }

   for (i = 0; i < num_dims; i++)
     at->dims [i] = dims[i];

   while (i < SLARRAY_MAX_DIMS)
     {
	at->dims [i] = 1;
	i++;
     }

   at->num_dims = num_dims;
   return 0;
}

static void array_reshape (SLang_Array_Type *at, SLang_Array_Type *ind_at)
{
   (void) do_array_reshape (at, ind_at);
}

static void _array_reshape (SLang_Array_Type *ind_at)
{
   SLang_Array_Type *at;
   SLang_Array_Type *new_at;

   if (-1 == SLang_pop_array (&at, 1))
     return;

   /* FIXME: Priority=low: duplicate_array could me modified to look at num_refs */

   /* Now try to avoid the overhead of creating a new array if possible */
   if (at->num_refs == 1)
     {
	/* Great, we are the sole owner of this array. */
	if ((-1 == do_array_reshape (at, ind_at))
	    || (-1 == SLclass_push_ptr_obj (SLANG_ARRAY_TYPE, (VOID_STAR)at)))
	  SLang_free_array (at);
	return;
     }

   new_at = SLang_duplicate_array (at);
   if (new_at != NULL)
     {
	if (0 == do_array_reshape (new_at, ind_at))
	  (void) SLang_push_array (new_at, 0);
	
	SLang_free_array (new_at);
     }
   SLang_free_array (at);
}

typedef struct
{
   SLang_Array_Type *at;
   unsigned int increment;
   char *addr;
}
Map_Arg_Type;
/* Usage: array_map (Return-Type, func, args,....); */
static void array_map (void)
{
   Map_Arg_Type *args;
   unsigned int num_args;
   unsigned int i, i_control;
   SLang_Name_Type *nt;
   unsigned int num_elements;
   SLang_Array_Type *at;
   char *addr;
   unsigned char type;

   at = NULL;
   args = NULL;
   nt = NULL;

   if (SLang_Num_Function_Args < 3)
     {
	SLang_verror (SL_INVALID_PARM,
		      "Usage: array_map (Return-Type, &func, args...)");
	SLdo_pop_n (SLang_Num_Function_Args);
	return;
     }

   num_args = (unsigned int)SLang_Num_Function_Args - 2;
   args = (Map_Arg_Type *) SLmalloc (num_args * sizeof (Map_Arg_Type));
   if (args == NULL)
     {
	SLdo_pop_n (SLang_Num_Function_Args);
	return;
     }
   memset ((char *) args, 0, num_args * sizeof (Map_Arg_Type));
   i = num_args;
   i_control = 0;
   while (i > 0)
     {
	i--;
	if (-1 == SLang_pop_array (&args[i].at, 1))
	  {
	     SLdo_pop_n (i + 2);
	     goto return_error;
	  }
	if (args[i].at->num_elements > 1)
	  i_control = i;
     }

   if (NULL == (nt = SLang_pop_function ()))
     {
	SLdo_pop_n (1);
	goto return_error;
     }

   num_elements = args[i_control].at->num_elements;

   if (-1 == SLang_pop_datatype (&type))
     goto return_error;

   if (type == SLANG_UNDEFINED_TYPE)   /* Void_Type */
     at = NULL;
   else
     {
	at = args[i_control].at;

	if (NULL == (at = SLang_create_array (type, 0, NULL, at->dims, at->num_dims)))
	  goto return_error;
     }
   

   for (i = 0; i < num_args; i++)
     {
	SLang_Array_Type *ati = args[i].at;
	/* FIXME: Priority = low: The actual dimensions should be compared. */
	if (ati->num_elements == num_elements)
	  args[i].increment = ati->sizeof_type;
	/* memset already guarantees increment to be zero */

	/* FIXME: array_map on an empty array should return an empty array
	 * and not generate an error.
	 */
	if (ati->num_elements == 0)
	  {
	     SLang_verror (0, "array_map: function argument %d of %d is an empty array", 
			   i+1, num_args);
	     goto return_error;
	  }

	args[i].addr = (char *) ati->data;
     }

   if (at == NULL)
     addr = NULL;
   else
     addr = (char *)at->data;

   for (i = 0; i < num_elements; i++)
     {
	unsigned int j;

	if (-1 == SLang_start_arg_list ())
	  goto return_error;

	for (j = 0; j < num_args; j++)
	  {
	     if (-1 == push_element_at_addr (args[j].at, 
					     (VOID_STAR) args[j].addr,
					     1))
	       {
		  SLdo_pop_n (j);
		  goto return_error;
	       }

	     args[j].addr += args[j].increment;
	  }

	if (-1 == SLang_end_arg_list ())
	  {
	     SLdo_pop_n (num_args);
	     goto return_error;
	  }

	if (-1 == SLexecute_function (nt))
	  goto return_error;

	if (at == NULL)
	  continue;

	if (-1 == at->cl->cl_apop (type, (VOID_STAR) addr))
	  goto return_error;

	addr += at->sizeof_type;
     }

   if (at != NULL)
     (void) SLang_push_array (at, 0);

   /* drop */

   return_error:
   SLang_free_array (at);
   SLang_free_function (nt);
   if (args != NULL)
     {
	for (i = 0; i < num_args; i++)
	  SLang_free_array (args[i].at);

	SLfree ((char *) args);
     }
}

static SLang_Intrin_Fun_Type Array_Table [] =
{
   MAKE_INTRINSIC_0("array_map", array_map, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("array_sort", sort_array, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_1("array_to_bstring", array_to_bstring, SLANG_VOID_TYPE, SLANG_ARRAY_TYPE),
   MAKE_INTRINSIC_1("bstring_to_array", bstring_to_array, SLANG_VOID_TYPE, SLANG_BSTRING_TYPE),
   MAKE_INTRINSIC("init_char_array", init_char_array, SLANG_VOID_TYPE, 0),
   MAKE_INTRINSIC_1("_isnull", is_null_intrinsic, SLANG_VOID_TYPE, SLANG_ARRAY_TYPE),
   MAKE_INTRINSIC_0("array_info", array_info, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("where", array_where, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_2("reshape", array_reshape, SLANG_VOID_TYPE, SLANG_ARRAY_TYPE, SLANG_ARRAY_TYPE),
   MAKE_INTRINSIC_1("_reshape", _array_reshape, SLANG_VOID_TYPE, SLANG_ARRAY_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

static char *array_string (unsigned char type, VOID_STAR v)
{
   SLang_Array_Type *at;
   char buf[512];
   unsigned int i, num_dims;
   int *dims;

   at = *(SLang_Array_Type **) v;
   type = at->data_type;
   num_dims = at->num_dims;
   dims = at->dims;

   sprintf (buf, "%s[%d", SLclass_get_datatype_name (type), at->dims[0]);

   for (i = 1; i < num_dims; i++)
     sprintf (buf + strlen(buf), ",%d", dims[i]);
   strcat (buf, "]");

   return SLmake_string (buf);
}

static void array_destroy (unsigned char type, VOID_STAR v)
{
   (void) type;
   SLang_free_array (*(SLang_Array_Type **) v);
}

static int array_push (unsigned char type, VOID_STAR v)
{
   SLang_Array_Type *at;

   (void) type;
   at = *(SLang_Array_Type **) v;
   return SLang_push_array (at, 0);
}

/* Intrinsic arrays are not stored in a variable. So, the address that
 * would contain the variable holds the array address.
 */
static int array_push_intrinsic (unsigned char type, VOID_STAR v)
{
   (void) type;
   return SLang_push_array ((SLang_Array_Type *) v, 0);
}

int _SLarray_add_bin_op (unsigned char type)
{
   SL_OOBinary_Type *ab;
   SLang_Class_Type *cl;

   cl = _SLclass_get_class (type);
   ab = cl->cl_binary_ops;

   while (ab != NULL)
     {
	if (ab->data_type == SLANG_ARRAY_TYPE)
	  return 0;
	ab = ab->next;
     }

   if ((-1 == SLclass_add_binary_op (SLANG_ARRAY_TYPE, type, array_binary_op, array_binary_op_result))
       || (-1 == SLclass_add_binary_op (type, SLANG_ARRAY_TYPE, array_binary_op, array_binary_op_result)))
     return -1;

   return 0;
}

static SLang_Array_Type *
do_array_math_op (int op, int unary_type,
		  SLang_Array_Type *at, unsigned int na)
{
   unsigned char a_type, b_type;
   int (*f) (int, unsigned char, VOID_STAR, unsigned int, VOID_STAR);
   SLang_Array_Type *bt;
   SLang_Class_Type *b_cl;
   int no_init;

   if (na != 1)
     {
	SLang_verror (SL_NOT_IMPLEMENTED, "Operation restricted to 1 array");
	return NULL;
     }

   a_type = at->data_type;
   if (NULL == (f = _SLclass_get_unary_fun (op, at->cl, &b_cl, unary_type)))
     return NULL;
   b_type = b_cl->cl_data_type;

   if (-1 == coerse_array_to_linear (at))
     return NULL;

   no_init = ((b_cl->cl_class_type == SLANG_CLASS_TYPE_SCALAR)
	      || (b_cl->cl_class_type == SLANG_CLASS_TYPE_VECTOR));

#if _SLANG_USE_TMP_OPTIMIZATION
   /* If we are dealing with scalar (or vector) objects, and if the object
    * appears to be owned by the stack, then use it instead of creating a 
    * new version.  This can happen with code such as:
    * @  x = [1,2,3,4];
    * @  x = UNARY_OP(__tmp(x));
    */
   if (no_init
       && (at->num_refs == 1)
       && (at->data_type == b_cl->cl_data_type))
     {
	bt = at;
	bt->num_refs = 2;
     }
   else
#endif				       /* _SLANG_USE_TMP_OPTIMIZATION */
     if (NULL == (bt = SLang_create_array1 (b_type, 0, NULL, at->dims, at->num_dims, no_init)))
       return NULL;

   if (1 != (*f)(op, a_type, at->data, at->num_elements, bt->data))
     {
	SLang_free_array (bt);
	return NULL;
     }
   return bt;
}

static int
array_unary_op_result (int op, unsigned char a, unsigned char *b)
{
   (void) op;
   (void) a;
   *b = SLANG_ARRAY_TYPE;
   return 1;
}

static int
array_unary_op (int op,
		unsigned char a, VOID_STAR ap, unsigned int na,
		VOID_STAR bp)
{
   SLang_Array_Type *at;

   (void) a;
   at = *(SLang_Array_Type **) ap;
   if (NULL == (at = do_array_math_op (op, _SLANG_BC_UNARY, at, na)))
     {
	if (SLang_Error) return -1;
	return 0;
     }
   *(SLang_Array_Type **) bp = at;
   return 1;
}

static int
array_math_op (int op,
	       unsigned char a, VOID_STAR ap, unsigned int na,
	       VOID_STAR bp)
{
   SLang_Array_Type *at;

   (void) a;
   at = *(SLang_Array_Type **) ap;
   if (NULL == (at = do_array_math_op (op, _SLANG_BC_MATH_UNARY, at, na)))
     {
	if (SLang_Error) return -1;
	return 0;
     }
   *(SLang_Array_Type **) bp = at;
   return 1;
}

static int
array_app_op (int op,
	      unsigned char a, VOID_STAR ap, unsigned int na,
	      VOID_STAR bp)
{
   SLang_Array_Type *at;

   (void) a;
   at = *(SLang_Array_Type **) ap;
   if (NULL == (at = do_array_math_op (op, _SLANG_BC_APP_UNARY, at, na)))
     {
	if (SLang_Error) return -1;
	return 0;
     }
   *(SLang_Array_Type **) bp = at;
   return 1;
}

int
_SLarray_typecast (unsigned char a_type, VOID_STAR ap, unsigned int na,
		   unsigned char b_type, VOID_STAR bp,
		   int is_implicit)
{
   SLang_Array_Type *at, *bt;
   SLang_Class_Type *b_cl;
   int no_init;
   int (*t) (unsigned char, VOID_STAR, unsigned int, unsigned char, VOID_STAR);

   if (na != 1)
     {
	SLang_verror (SL_NOT_IMPLEMENTED, "typecast of multiple arrays not implemented");
	return -1;
     }

   at = *(SLang_Array_Type **) ap;
   a_type = at->data_type;

   if (a_type == b_type)
     {
	at->num_refs += 1;
	*(SLang_Array_Type **) bp = at;
	return 1;
     }

   if (NULL == (t = _SLclass_get_typecast (a_type, b_type, is_implicit)))
     return -1;

   if (-1 == coerse_array_to_linear (at))
     return -1;

   b_cl = _SLclass_get_class (b_type);

   no_init = ((b_cl->cl_class_type == SLANG_CLASS_TYPE_SCALAR)
	      || (b_cl->cl_class_type == SLANG_CLASS_TYPE_VECTOR));

   if (NULL == (bt = SLang_create_array1 (b_type, 0, NULL, at->dims, at->num_dims, no_init)))
     return -1;

   if (1 == (*t) (a_type, at->data, at->num_elements, b_type, bt->data))
     {
	*(SLang_Array_Type **) bp = bt;
	return 1;
     }

   SLang_free_array (bt);
   return 0;
}

SLang_Array_Type *SLang_duplicate_array (SLang_Array_Type *at)
{
   SLang_Array_Type *bt;
   char *data, *a_data;
   unsigned int i, num_elements, sizeof_type;
   unsigned int size;
   int (*cl_acopy) (unsigned char, VOID_STAR, VOID_STAR);
   unsigned char type;

   if (-1 == coerse_array_to_linear (at))
     return NULL;

   type = at->data_type;
   num_elements = at->num_elements;
   sizeof_type = at->sizeof_type;
   size = num_elements * sizeof_type;

   if (NULL == (data = SLmalloc (size)))
     return NULL;

   if (NULL == (bt = SLang_create_array (type, 0, (VOID_STAR)data, at->dims, at->num_dims)))
     {
	SLfree (data);
	return NULL;
     }

   a_data = (char *) at->data;
   if (0 == (at->flags & SLARR_DATA_VALUE_IS_POINTER))
     {
	SLMEMCPY (data, a_data, size);
	return bt;
     }

   SLMEMSET (data, 0, size);

   cl_acopy = at->cl->cl_acopy;
   for (i = 0; i < num_elements; i++)
     {
	if (NULL != *(VOID_STAR *) a_data)
	  {
	     if (-1 == (*cl_acopy) (type, (VOID_STAR) a_data, (VOID_STAR) data))
	       {
		  SLang_free_array (bt);
		  return NULL;
	       }
	  }

	data += sizeof_type;
	a_data += sizeof_type;
     }

   return bt;
}

static int array_dereference (unsigned char type, VOID_STAR addr)
{
   SLang_Array_Type *at;

   (void) type;
   at = SLang_duplicate_array (*(SLang_Array_Type **) addr);
   if (at == NULL) return -1;
   return SLang_push_array (at, 1);
}

/* This function gets called via, e.g., @Array_Type (Double_Type, [10,20]);
 */
static int
array_datatype_deref (unsigned char type)
{
   SLang_Array_Type *ind_at;
   SLang_Array_Type *at;

#if 0
   /* The parser generated code for this as if a function call were to be
    * made.  However, the interpreter simply called the deref object routine
    * instead of the function call.  So, I must simulate the function call.
    * This needs to be formalized to hide this detail from applications
    * who wish to do the same.  So...
    * FIXME: Priority=medium
    */
   if (0 == _SL_increment_frame_pointer ())
     (void) _SL_decrement_frame_pointer ();
#endif

   if (-1 == SLang_pop_array (&ind_at, 1))
     return -1;

   if ((ind_at->data_type != SLANG_INT_TYPE)
       || (ind_at->num_dims != 1))
     {
	SLang_verror (SL_TYPE_MISMATCH, "Expecting 1-d integer array");
	goto return_error;
     }

   if (-1 == SLang_pop_datatype (&type))
     goto return_error;

   if (NULL == (at = SLang_create_array (type, 0, NULL,
					 (int *) ind_at->data,
					 ind_at->num_elements)))
     goto return_error;

   SLang_free_array (ind_at);
   return SLang_push_array (at, 1);

   return_error:
   SLang_free_array (ind_at);
   return -1;
}

static int array_length (unsigned char type, VOID_STAR v, unsigned int *len)
{
   SLang_Array_Type *at;

   (void) type;
   at = *(SLang_Array_Type **) v;
   *len = at->num_elements;
   return 0;
}

int
_SLarray_init_slarray (void)
{
   SLang_Class_Type *cl;

   if (-1 == SLadd_intrin_fun_table (Array_Table, NULL))
     return -1;

   if (NULL == (cl = SLclass_allocate_class ("Array_Type")))
     return -1;

   (void) SLclass_set_string_function (cl, array_string);
   (void) SLclass_set_destroy_function (cl, array_destroy);
   (void) SLclass_set_push_function (cl, array_push);
   cl->cl_push_intrinsic = array_push_intrinsic;
   cl->cl_dereference = array_dereference;
   cl->cl_datatype_deref = array_datatype_deref;
   cl->cl_length = array_length;

   if (-1 == SLclass_register_class (cl, SLANG_ARRAY_TYPE, sizeof (VOID_STAR),
				     SLANG_CLASS_TYPE_PTR))
     return -1;

   if ((-1 == SLclass_add_binary_op (SLANG_ARRAY_TYPE, SLANG_ARRAY_TYPE, array_binary_op, array_binary_op_result))
       || (-1 == SLclass_add_unary_op (SLANG_ARRAY_TYPE, array_unary_op, array_unary_op_result))
       || (-1 == SLclass_add_app_unary_op (SLANG_ARRAY_TYPE, array_app_op, array_unary_op_result))
       || (-1 == SLclass_add_math_op (SLANG_ARRAY_TYPE, array_math_op, array_unary_op_result)))
     return -1;

   return 0;
}

int SLang_pop_array (SLang_Array_Type **at_ptr, int convert_scalar)
{
   if (-1 == pop_array (at_ptr, convert_scalar))
     return -1;

   if (-1 == coerse_array_to_linear (*at_ptr))
     {
	SLang_free_array (*at_ptr);
	return -1;
     }
   return 0;
}

int SLang_pop_array_of_type (SLang_Array_Type **at, unsigned char type)
{
   if (-1 == SLclass_typecast (type, 1, 1))
     return -1;

   return SLang_pop_array (at, 1);
}

void (*_SLang_Matrix_Multiply)(void);

int _SLarray_matrix_multiply (void)
{
   if (_SLang_Matrix_Multiply != NULL)
     {
	(*_SLang_Matrix_Multiply)();
	return 0;
     }
   SLang_verror (SL_NOT_IMPLEMENTED, "Matrix multiplication not available");
   return -1;
}

struct _SLang_Foreach_Context_Type
{
   SLang_Array_Type *at;
   unsigned int next_element_index;
};

SLang_Foreach_Context_Type *
_SLarray_cl_foreach_open (SLtype type, unsigned int num)
{
   SLang_Foreach_Context_Type *c;

   if (num != 0)
     {
	SLdo_pop_n (num + 1);
	SLang_verror (SL_NOT_IMPLEMENTED,
		      "%s does not support 'foreach using' form",
		      SLclass_get_datatype_name (type));
	return NULL;
     }

   if (NULL == (c = (SLang_Foreach_Context_Type *) SLmalloc (sizeof (SLang_Foreach_Context_Type))))
     return NULL;

   memset ((char *) c, 0, sizeof (SLang_Foreach_Context_Type));

   if (-1 == pop_array (&c->at, 1))
     {
	SLfree ((char *) c);
	return NULL;
     }

   return c;
}

void _SLarray_cl_foreach_close (SLtype type, SLang_Foreach_Context_Type *c)
{
   (void) type;
   if (c == NULL) return;
   SLang_free_array (c->at);
   SLfree ((char *) c);
}

int _SLarray_cl_foreach (SLtype type, SLang_Foreach_Context_Type *c)
{
   SLang_Array_Type *at;
   VOID_STAR data;

   (void) type;

   if (c == NULL)
     return -1;

   at = c->at;
   if (at->num_elements == c->next_element_index)
     return 0;

   /* FIXME: Priority = low.  The following assumes linear arrays
    * or Integer range arrays.  Fixing it right requires a method to get the
    * nth element of a multidimensional array.
    */

   if (at->flags & SLARR_DATA_VALUE_IS_RANGE)
     {
	int d = (int) c->next_element_index;
	data = range_get_data_addr (at, &d);
     }
   else
     data = (VOID_STAR) ((char *)at->data + (c->next_element_index * at->sizeof_type));

   c->next_element_index += 1;

   if ((at->flags & SLARR_DATA_VALUE_IS_POINTER)
       && (*(VOID_STAR *) data == NULL))
     {
	if (-1 == SLang_push_null ())
	  return -1;
     }
   else if (-1 == (*at->cl->cl_apush)(at->data_type, data))
     return -1;

   /* keep going */
   return 1;
}

