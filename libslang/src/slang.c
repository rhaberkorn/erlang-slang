/* -*- mode: C; mode: fold; -*- */
/* slang.c  --- guts of S-Lang interpreter */
/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#if SLANG_HAS_FLOAT
# include <math.h>
#endif

#include "slang.h"
#include "_slang.h"

#define USE_COMBINED_BYTECODES	1

struct _SLBlock_Type;

typedef struct
{
   struct _SLBlock_Type *body;
   unsigned int num_refs;
}
_SLBlock_Header_Type;

typedef struct
{
   char *name;
   SLang_Name_Type *next;
   char name_type;

   union
     {
	_SLBlock_Header_Type *header;    /* body of function */
	char *autoload_filename;
     }
   v;
#if _SLANG_HAS_DEBUG_CODE
   char *file;
#endif
#define SLANG_MAX_LOCAL_VARIABLES 254
#define AUTOLOAD_NUM_LOCALS (SLANG_MAX_LOCAL_VARIABLES + 1)
   unsigned char nlocals;	       /* number of local variables */
   unsigned char nargs;		       /* number of arguments */
}
_SLang_Function_Type;

typedef struct
{
   char *name;
   SLang_Name_Type *next;
   char name_type;

   SLang_Object_Type obj;
}
SLang_Global_Var_Type;

typedef struct
{
   char *name;
   SLang_Name_Type *next;
   char name_type;

   int local_var_number;
}
SLang_Local_Var_Type;

typedef struct _SLBlock_Type
{
   unsigned char bc_main_type;
   unsigned char bc_sub_type;
   union
     {
	struct _SLBlock_Type *blk;
	int i_blk;

	SLang_Name_Type *nt_blk;
	SLang_App_Unary_Type *nt_unary_blk;
	SLang_Intrin_Var_Type *nt_ivar_blk;
	SLang_Intrin_Fun_Type *nt_ifun_blk;
	SLang_Global_Var_Type *nt_gvar_blk;
	SLang_IConstant_Type *iconst_blk;
	SLang_DConstant_Type *dconst_blk;
	_SLang_Function_Type *nt_fun_blk;

	VOID_STAR ptr_blk;
	char *s_blk;
	SLang_BString_Type *bs_blk;

#if SLANG_HAS_FLOAT
	double *double_blk;		       /*literal double is a pointer */
#endif
	float float_blk;
	long l_blk;
	struct _SLang_Struct_Type *struct_blk;
	int (*call_function)(void);
     }
   b;
}
SLBlock_Type;

static SLBlock_Type SLShort_Blocks[6];
/* These are initialized in add_table below.  I cannot init a Union!! */

/* Do not change these.  Odd values are for termination */
#define SHORT_BLOCK_RETURN_INDX		0
#define SHORT_BLOCK_BREAK_INDX		2
#define SHORT_BLOCK_CONTINUE_INDX	4

/* Debugging and tracing variables */

void (*SLang_Enter_Function)(char *) = NULL;
void (*SLang_Exit_Function)(char *) = NULL;
/* If non null, these call C functions before and after a slang function. */

int _SLang_Trace = 0;
/* If _SLang_Trace = -1, do not trace intrinsics */
static int Trace_Mode = 0;

static char *Trace_Function;	       /* function to be traced */
int SLang_Traceback = 0;
/* non zero means do traceback.  If less than 0, do not show local variables */

/* These variables handle _NARGS processing by the parser */
int SLang_Num_Function_Args;
static int *Num_Args_Stack;
static unsigned int Recursion_Depth;
static SLang_Object_Type *Frame_Pointer;
static int Next_Function_Num_Args;
static unsigned int Frame_Pointer_Depth;
static unsigned int *Frame_Pointer_Stack;

static int Lang_Break_Condition = 0;
/* true if any one below is true.  This keeps us from testing 3 variables.
 * I know this can be perfomed with a bitmapped variable, but...
 */
static int Lang_Break = 0;
static int Lang_Return = 0;
/* static int Lang_Continue = 0; */

static SLang_Object_Type *_SLRun_Stack;
static SLang_Object_Type *_SLStack_Pointer;
static SLang_Object_Type *_SLStack_Pointer_Max;

/* Might want to increase this. */
static SLang_Object_Type Local_Variable_Stack[SLANG_MAX_LOCAL_STACK];
static SLang_Object_Type *Local_Variable_Frame = Local_Variable_Stack;

static void free_function_header (_SLBlock_Header_Type *);

#if _SLANG_OPTIMIZE_FOR_SPEED
static SLtype Class_Type [256];
#endif
/* If 0, not an arith type.  Otherwise it is.  Also, value implies precedence 
 * See slarith.c for how this is used.
 */
static unsigned char Is_Arith_Type [256];

void (*SLang_Dump_Routine)(char *);

static void call_dump_routine (char *fmt, ...)
{
   char buf[1024];
   va_list ap;

   va_start (ap, fmt);
   if (SLang_Dump_Routine != NULL)
     {
	(void) _SLvsnprintf (buf, sizeof (buf), fmt, ap);
	(*SLang_Dump_Routine) (buf);
     }
   else
     {
	vfprintf (stderr, fmt, ap);
	fflush (stderr);
     }
   va_end (ap);
}

static void do_traceback (char *, unsigned int, char *);
static int init_interpreter (void);

/*{{{ push/pop/etc stack manipulation functions */

/* This routine is assumed to work even in the presence of a SLang_Error. */
_INLINE_
int SLang_pop (SLang_Object_Type *x)
{
   register SLang_Object_Type *y;

   y = _SLStack_Pointer;
   if (y == _SLRun_Stack)
     {
	if (SLang_Error == 0) SLang_Error = SL_STACK_UNDERFLOW;
	x->data_type = 0;
	return -1;
     }
   y--;
   *x = *y;

   _SLStack_Pointer = y;
   return 0;
}

_INLINE_
int SLang_peek_at_stack (void)
{
   if (_SLStack_Pointer == _SLRun_Stack)
     {
	if (SLang_Error == 0)
	  SLang_Error = SL_STACK_UNDERFLOW;
	return -1;
     }

   return (_SLStack_Pointer - 1)->data_type;
}

static int pop_ctrl_integer (int *i)
{
   int type;
   SLang_Class_Type *cl;
#if _SLANG_OPTIMIZE_FOR_SPEED
   register SLang_Object_Type *y;

   /* Most of the time, either an integer or a char will be on the stack.
    * Optimize these cases.
    */
   y = _SLStack_Pointer;
   if (y == _SLRun_Stack)
     {
	if (SLang_Error == 0) SLang_Error = SL_STACK_UNDERFLOW;
	return -1;
     }
   y--;
   
   type = y->data_type;
   if (type == SLANG_INT_TYPE)
     {
	_SLStack_Pointer = y;   
	*i = y->v.int_val;
	return 0;
     }
   if (type == SLANG_CHAR_TYPE)
     {
	_SLStack_Pointer = y;   
	*i = y->v.char_val;
	return 0;
     }
#else
   if (-1 == (type = SLang_peek_at_stack ()))
     return -1;
#endif

   cl = _SLclass_get_class ((unsigned char) type);
   if (cl->cl_to_bool == NULL)
     {
	SLang_verror (SL_TYPE_MISMATCH,
		      "%s cannot be used in a boolean context",
		      cl->cl_name);
	return -1;
     }
   return cl->cl_to_bool ((unsigned char) type, i);
}

int SLang_peek_at_stack1 (void)
{
   int type;

   type = SLang_peek_at_stack ();
   if (type == SLANG_ARRAY_TYPE)
     type = (_SLStack_Pointer - 1)->v.array_val->data_type;

   return type;
}

_INLINE_
void SLang_free_object (SLang_Object_Type *obj)
{
   unsigned char data_type;
   SLang_Class_Type *cl;

   if (obj == NULL) return;
   data_type = obj->data_type;
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR == Class_Type [data_type])
     return;
   if (data_type == SLANG_STRING_TYPE)
     {
	SLang_free_slstring (obj->v.s_val);
	return;
     }
#endif
   cl = _SLclass_get_class (data_type);
#if !_SLANG_OPTIMIZE_FOR_SPEED
   if (cl->cl_class_type != SLANG_CLASS_TYPE_SCALAR)
#endif
     (*cl->cl_destroy) (data_type, (VOID_STAR) &obj->v);
}

_INLINE_
int SLang_push (SLang_Object_Type *x)
{
   register SLang_Object_Type *y;
   y = _SLStack_Pointer;

   /* if there is a SLang_Error, probably not much harm will be done
    if it is ignored here */
   /* if (SLang_Error) return; */

   /* flag it now */
   if (y >= _SLStack_Pointer_Max)
     {
	if (!SLang_Error) SLang_Error = SL_STACK_OVERFLOW;
	return -1;
     }

   *y = *x;
   _SLStack_Pointer = y + 1;
   return 0;
}

/* _INLINE_ */
int SLclass_push_ptr_obj (unsigned char type, VOID_STAR pval)
{
   register SLang_Object_Type *y;
   y = _SLStack_Pointer;

   if (y >= _SLStack_Pointer_Max)
     {
	if (!SLang_Error) SLang_Error = SL_STACK_OVERFLOW;
	return -1;
     }

   y->data_type = type;
   y->v.ptr_val = pval;

   _SLStack_Pointer = y + 1;
   return 0;
}

_INLINE_
int SLclass_push_int_obj (unsigned char type, int x)
{
   register SLang_Object_Type *y;
   y = _SLStack_Pointer;

   if (y >= _SLStack_Pointer_Max)
     {
	if (!SLang_Error) SLang_Error = SL_STACK_OVERFLOW;
	return -1;
     }

   y->data_type = type;
   y->v.int_val = x;

   _SLStack_Pointer = y + 1;
   return 0;
}

#if SLANG_HAS_FLOAT
_INLINE_
int SLclass_push_double_obj (unsigned char type, double x)
{
   SLang_Object_Type obj;
   obj.data_type = type;
   obj.v.double_val = x;
   return SLang_push (&obj);
}
#endif

_INLINE_
int _SLang_pop_object_of_type (unsigned char type, SLang_Object_Type *obj,
			       int allow_arrays)
{
   register SLang_Object_Type *y;

   y = _SLStack_Pointer;
   if (y == _SLRun_Stack)
     return SLang_pop (obj);
   y--;
   if (y->data_type != type)
     {
#if _SLANG_OPTIMIZE_FOR_SPEED
	/* This is an implicit typecast.  We do not want to typecast
	 * floats to ints implicitly.  
	 */
	if (Is_Arith_Type [type]
	    && Is_Arith_Type [y->data_type]
	    && (Is_Arith_Type [type] >= Is_Arith_Type[y->data_type]))
	  {
	     /* This should not fail */
	     (void) _SLarith_typecast (y->data_type, (VOID_STAR)&y->v, 1, 
				       type, (VOID_STAR)&obj->v);
	     obj->data_type = type;
	     _SLStack_Pointer = y;
	     return 0;
	  }
#endif
	
	if ((allow_arrays == 0)
	    || (y->data_type != SLANG_ARRAY_TYPE)
	    || (y->v.array_val->data_type != type))
	  if (-1 == SLclass_typecast (type, 1, 0))
	    return -1;
     }
   *obj = *y;
   _SLStack_Pointer = y;
   return 0;
}

/*  This function reverses the top n items on the stack and returns a
 *  an offset from the start of the stack to the last item.
 */
int SLreverse_stack (int n)
{
   SLang_Object_Type *otop, *obot, tmp;

   otop = _SLStack_Pointer;
   if ((n > otop - _SLRun_Stack) || (n < 0))
     {
	SLang_Error = SL_STACK_UNDERFLOW;
	return -1;
     }
   obot = otop - n;
   otop--;
   while (otop > obot)
     {
	tmp = *obot;
	*obot = *otop;
	*otop = tmp;
	otop--;
	obot++;
     }
   return (int) ((_SLStack_Pointer - n) - _SLRun_Stack);
}

_INLINE_
int SLroll_stack (int np)
{
   int n, i;
   SLang_Object_Type *otop, *obot, tmp;

   if ((n = abs(np)) <= 1) return 0;    /* identity */

   obot = otop = _SLStack_Pointer;
   i = n;
   while (i != 0)
     {
	if (obot <= _SLRun_Stack)
	  {
	     SLang_Error = SL_STACK_UNDERFLOW;
	     return -1;
	  }
	obot--;
	i--;
     }
   otop--;

   if (np > 0)
     {
	/* Put top on bottom and roll rest up. */
	tmp = *otop;
	while (otop > obot)
	  {
	     *otop = *(otop - 1);
	     otop--;
	  }
	*otop = tmp;
     }
   else
     {
	/* Put bottom on top and roll rest down. */
	tmp = *obot;
	while (obot < otop)
	  {
	     *obot = *(obot + 1);
	     obot++;
	  }
	*obot = tmp;
     }
   return 0;
}

int _SLstack_depth (void)
{
   return (int) (_SLStack_Pointer - _SLRun_Stack);
}

int SLdup_n (int n)
{
   SLang_Object_Type *bot, *top;

   if (n <= 0)
     return 0;
   
   top = _SLStack_Pointer;
   if (top < _SLRun_Stack + n)
     {
	if (SLang_Error == 0)
	  SLang_Error = SL_STACK_UNDERFLOW;
	return -1;
     }
   if (top + n > _SLStack_Pointer_Max)
     {
	if (SLang_Error == 0)
	  SLang_Error = SL_STACK_OVERFLOW;
	return -1;
     }
   bot = top - n;

   while (bot < top)
     {
	SLang_Class_Type *cl;
	unsigned char data_type = bot->data_type;

#if _SLANG_OPTIMIZE_FOR_SPEED
	if (SLANG_CLASS_TYPE_SCALAR == Class_Type [data_type])
	  {
	     *_SLStack_Pointer++ = *bot++;
	     continue;
	  }
#endif
	cl = _SLclass_get_class (data_type);
	if (-1 == (*cl->cl_push) (data_type, (VOID_STAR) &bot->v))
	  return -1;
	bot++;
     }
   return 0;
}

/*}}}*/

/*{{{ inner interpreter and support functions */

_INLINE_
int _SL_increment_frame_pointer (void)
{
   if (Recursion_Depth >= SLANG_MAX_RECURSIVE_DEPTH)
     {
	SLang_verror (SL_STACK_OVERFLOW, "Num Args Stack Overflow");
	return -1;
     }
   Num_Args_Stack [Recursion_Depth] = SLang_Num_Function_Args;

   SLang_Num_Function_Args = Next_Function_Num_Args;
   Next_Function_Num_Args = 0;
   Recursion_Depth++;
   return 0;
}

_INLINE_
int _SL_decrement_frame_pointer (void)
{
   if (Recursion_Depth == 0)
     {
	SLang_verror (SL_STACK_UNDERFLOW, "Num Args Stack Underflow");
	return -1;
     }

   Recursion_Depth--;
   if (Recursion_Depth < SLANG_MAX_RECURSIVE_DEPTH)
     SLang_Num_Function_Args = Num_Args_Stack [Recursion_Depth];

   return 0;
}

_INLINE_
int SLang_start_arg_list (void)
{
   if (Frame_Pointer_Depth < SLANG_MAX_RECURSIVE_DEPTH)
     {
	Frame_Pointer_Stack [Frame_Pointer_Depth] = (unsigned int) (Frame_Pointer - _SLRun_Stack);
	Frame_Pointer = _SLStack_Pointer;
	Frame_Pointer_Depth++;
	Next_Function_Num_Args = 0;
	return 0;
     }

   SLang_verror (SL_STACK_OVERFLOW, "Frame Stack Overflow");
   return -1;
}

_INLINE_
int SLang_end_arg_list (void)
{
   if (Frame_Pointer_Depth == 0)
     {
	SLang_verror (SL_STACK_UNDERFLOW, "Frame Stack Underflow");
	return -1;
     }
   Frame_Pointer_Depth--;
   if (Frame_Pointer_Depth < SLANG_MAX_RECURSIVE_DEPTH)
     {
	Next_Function_Num_Args = (int) (_SLStack_Pointer - Frame_Pointer);
	Frame_Pointer = _SLRun_Stack + Frame_Pointer_Stack [Frame_Pointer_Depth];
     }
   return 0;
}

_INLINE_
static int do_bc_call_direct_frame (int (*f)(void))
{
   if ((0 == SLang_end_arg_list ())
       && (0 == _SL_increment_frame_pointer ()))
     {
	(void) (*f) ();
	_SL_decrement_frame_pointer ();
     }
   if (SLang_Error)
     return -1;
   return 0;
}

static int do_name_type_error (SLang_Name_Type *nt)
{
   char buf[256];
   if (nt != NULL)
     {
	(void) _SLsnprintf (buf, sizeof (buf), "(Error occurred processing %s)", nt->name);
	do_traceback (buf, 0, NULL);
     }
   return -1;
}

/* local and global variable assignments */

static int do_binary_ab (int op, SLang_Object_Type *obja, SLang_Object_Type *objb)
{
   SLang_Class_Type *a_cl, *b_cl, *c_cl;
   unsigned char b_data_type, a_data_type, c_data_type;
   int (*binary_fun) (int,
		      unsigned char, VOID_STAR, unsigned int,
		      unsigned char, VOID_STAR, unsigned int,
		      VOID_STAR);
   VOID_STAR pa;
   VOID_STAR pb;
   VOID_STAR pc;
   int ret;

   b_data_type = objb->data_type;
   a_data_type = obja->data_type;

#if _SLANG_OPTIMIZE_FOR_SPEED
   if (Is_Arith_Type[a_data_type]
       && Is_Arith_Type[b_data_type])
     {
	int status;
	status = _SLarith_bin_op (obja, objb, op);
	if (status != 1)
	  return status;
	/* drop and try it the hard way */
     }
#endif

   a_cl = _SLclass_get_class (a_data_type);
   if (a_data_type == b_data_type)
     b_cl = a_cl;
   else
     b_cl = _SLclass_get_class (b_data_type);

   if (NULL == (binary_fun = _SLclass_get_binary_fun (op, a_cl, b_cl, &c_cl, 1)))
     return -1;

   c_data_type = c_cl->cl_data_type;

#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR == Class_Type [a_data_type])
     pa = (VOID_STAR) &obja->v;
   else
#endif
     pa = _SLclass_get_ptr_to_value (a_cl, obja);

#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR == Class_Type [b_data_type])
     pb = (VOID_STAR) &objb->v;
   else
#endif
     pb = _SLclass_get_ptr_to_value (b_cl, objb);

   pc = c_cl->cl_transfer_buf;

   if (1 != (*binary_fun) (op,
			   a_data_type, pa, 1,
			   b_data_type, pb, 1,
			   pc))
     {
	SLang_verror (SL_NOT_IMPLEMENTED,
		      "Binary operation between %s and %s failed",
		      a_cl->cl_name, b_cl->cl_name);

	return -1;
     }

   /* apush will create a copy, so make sure we free after the push */
   ret = (*c_cl->cl_apush)(c_data_type, pc);
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR != Class_Type [c_data_type])
#endif
     (*c_cl->cl_adestroy)(c_data_type, pc);

   return ret;
}

_INLINE_
static int do_binary_ab_inc_ref (int op, SLang_Object_Type *obja, SLang_Object_Type *objb)
{
   int ret;
#if _SLANG_USE_TMP_OPTIMIZATION
   int inc = 0;
   
   if (obja->data_type == SLANG_ARRAY_TYPE)
     {
	inc |= 1;
	obja->v.array_val->num_refs++;
     }
   if (objb->data_type == SLANG_ARRAY_TYPE)
     {
	inc |= 2;
	objb->v.array_val->num_refs++;
     }
#endif
   ret = do_binary_ab (op, obja, objb);

#if _SLANG_USE_TMP_OPTIMIZATION
   if (inc & 1) obja->v.array_val->num_refs--;
   if (inc & 2) objb->v.array_val->num_refs--;
#endif
   
   return ret;
}

_INLINE_
static void do_binary (int op)
{
   SLang_Object_Type obja, objb;

   if (SLang_pop (&objb)) return;
   if (0 == SLang_pop (&obja))
     {
	(void) do_binary_ab (op, &obja, &objb);
#if _SLANG_OPTIMIZE_FOR_SPEED
	if (SLANG_CLASS_TYPE_SCALAR != Class_Type [obja.data_type])
#endif
	  SLang_free_object (&obja);
     }
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR != Class_Type [objb.data_type])
#endif
     SLang_free_object (&objb);
}

_INLINE_
static void do_binary_b (int op, SLang_Object_Type *bp)
{
   SLang_Object_Type a;

   if (SLang_pop (&a)) return;
   (void) do_binary_ab (op, &a, bp);
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR != Class_Type [a.data_type])
#endif
     SLang_free_object (&a);
}

_INLINE_
static void do_binary_b_inc_ref (int op, SLang_Object_Type *bp)
{
   SLang_Object_Type a;

   if (SLang_pop (&a)) return;
#if _SLANG_USE_TMP_OPTIMIZATION
   if (bp->data_type == SLANG_ARRAY_TYPE)
     {
	bp->v.array_val->num_refs++;
	(void) do_binary_ab (op, &a, bp);
	bp->v.array_val->num_refs--;
     }
   else
#endif
     (void) do_binary_ab (op, &a, bp);
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR != Class_Type [a.data_type])
#endif
     SLang_free_object (&a);
}

static int do_unary_op (int op, SLang_Object_Type *obj, int unary_type)
{
   int (*f) (int, unsigned char, VOID_STAR, unsigned int, VOID_STAR);
   VOID_STAR pa;
   VOID_STAR pb;
   SLang_Class_Type *a_cl, *b_cl;
   unsigned char a_type, b_type;
   int ret;

   a_type = obj->data_type;
   a_cl = _SLclass_get_class (a_type);

   if (NULL == (f = _SLclass_get_unary_fun (op, a_cl, &b_cl, unary_type)))
     return -1;

   b_type = b_cl->cl_data_type;

#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR == Class_Type [a_type])
     pa = (VOID_STAR) &obj->v;
   else
#endif
     pa = _SLclass_get_ptr_to_value (a_cl, obj);

   pb = b_cl->cl_transfer_buf;

   if (1 != (*f) (op, a_type, pa, 1, pb))
     {
	SLang_verror (SL_NOT_IMPLEMENTED,
		      "Unary operation for %s failed", a_cl->cl_name);
	return -1;
     }

   ret = (*b_cl->cl_apush)(b_type, pb);
   /* cl_apush creates a copy, so make sure we call cl_adestroy */
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR != Class_Type [b_type])
#endif
     (*b_cl->cl_adestroy)(b_type, pb);

   return ret;
}

_INLINE_
static int do_unary (int op, int unary_type)
{
   SLang_Object_Type obj;
   int ret;

   if (-1 == SLang_pop (&obj)) return -1;
   ret = do_unary_op (op, &obj, unary_type);
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR != Class_Type [obj.data_type])
#endif
     SLang_free_object (&obj);
   return ret;
}

static int do_assignment_binary (int op, SLang_Object_Type *obja_ptr)
{
   SLang_Object_Type objb;
   int ret;

   if (SLang_pop (&objb))
     return -1;

   ret = do_binary_ab (op, obja_ptr, &objb);
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR != Class_Type [objb.data_type])
#endif
     SLang_free_object (&objb);
   return ret;
}

/* The order of these is assumed to match the binary operators
 * defined in slang.h
 */
static int
map_assignment_op_to_binary (unsigned char op_type, int *op, int *is_unary)
{
   *is_unary = 0;
   switch (op_type)
     {
      case _SLANG_BCST_PLUSEQS:
      case _SLANG_BCST_MINUSEQS:
      case _SLANG_BCST_TIMESEQS:
      case _SLANG_BCST_DIVEQS:
	*op = SLANG_PLUS + (op_type - _SLANG_BCST_PLUSEQS);
	break;

      case _SLANG_BCST_BOREQS:
	*op = SLANG_BOR;
	break;

      case _SLANG_BCST_BANDEQS:
	*op = SLANG_BAND;
	break;

      case _SLANG_BCST_POST_MINUSMINUS:
      case _SLANG_BCST_MINUSMINUS:
	*op = SLANG_MINUS;
	*is_unary = 1;
	break;

      case _SLANG_BCST_PLUSPLUS:
      case _SLANG_BCST_POST_PLUSPLUS:
	*op = SLANG_PLUS;
	*is_unary = 1;
	break;

      default:
	SLang_verror (SL_NOT_IMPLEMENTED, "Assignment operator not implemented");
	return -1;
     }
   return 0;
}

static int
perform_lvalue_operation (unsigned char op_type, SLang_Object_Type *obja_ptr)
{
   switch (op_type)
     {
      case _SLANG_BCST_ASSIGN:
	break;

	/* The order of these is assumed to match the binary operators
	 * defined in slang.h
	 */
      case _SLANG_BCST_PLUSEQS:
      case _SLANG_BCST_MINUSEQS:
      case _SLANG_BCST_TIMESEQS:
      case _SLANG_BCST_DIVEQS:
	if (-1 == do_assignment_binary (SLANG_PLUS + (op_type - _SLANG_BCST_PLUSEQS), obja_ptr))
	  return -1;
	break;

      case _SLANG_BCST_BOREQS:
	if (-1 == do_assignment_binary (SLANG_BOR, obja_ptr))
	  return -1;
	break;

      case _SLANG_BCST_BANDEQS:
	if (-1 == do_assignment_binary (SLANG_BAND, obja_ptr))
	  return -1;
	break;

      case _SLANG_BCST_PLUSPLUS:
      case _SLANG_BCST_POST_PLUSPLUS:
#if _SLANG_OPTIMIZE_FOR_SPEED
	if (obja_ptr->data_type == SLANG_INT_TYPE)
	  return SLclass_push_int_obj (SLANG_INT_TYPE, obja_ptr->v.int_val + 1);
#endif
	if (-1 == do_unary_op (SLANG_PLUSPLUS, obja_ptr, _SLANG_BC_UNARY))
	  return -1;
	break;

      case _SLANG_BCST_MINUSMINUS:
      case _SLANG_BCST_POST_MINUSMINUS:
#if _SLANG_OPTIMIZE_FOR_SPEED
	if (obja_ptr->data_type == SLANG_INT_TYPE)
	  return SLclass_push_int_obj (SLANG_INT_TYPE, obja_ptr->v.int_val - 1);
#endif
	if (-1 == do_unary_op (SLANG_MINUSMINUS, obja_ptr, _SLANG_BC_UNARY))
	  return -1;
	break;

      default:
	SLang_Error = SL_INTERNAL_ERROR;
	return -1;
     }
   return 0;
}

_INLINE_
static int
set_lvalue_obj (unsigned char op_type, SLang_Object_Type *obja_ptr)
{
   if (op_type != _SLANG_BCST_ASSIGN)
     {
	if (-1 == perform_lvalue_operation (op_type, obja_ptr))
	  return -1;
     }
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR != Class_Type [obja_ptr->data_type])
#endif
     SLang_free_object (obja_ptr);

   return SLang_pop(obja_ptr);
}

static int
set_struct_lvalue (SLBlock_Type *bc_blk)
{
   int type;
   SLang_Class_Type *cl;
   char *name;
   int op;

   if (-1 == (type = SLang_peek_at_stack ()))
     return -1;

   cl = _SLclass_get_class (type);
   if ((cl->cl_sput == NULL)
       || (cl->cl_sget == NULL))
     {
	SLang_verror (SL_NOT_IMPLEMENTED,
		      "%s does not support structure access",
		      cl->cl_name);
	SLdo_pop_n (2);		       /* object plus what was to be assigned */
	return -1;
     }
   name = bc_blk->b.s_blk;
   op = bc_blk->bc_sub_type;

   if (op != _SLANG_BCST_ASSIGN)
     {
	/* We have something like (A.x += b) or (A.x++).  In either case,
	 * we need A.x.
	 */
	SLang_Object_Type obj_A;
	SLang_Object_Type obj;

	if (-1 == SLang_pop (&obj_A))
	  return -1;

	if ((-1 == _SLpush_slang_obj (&obj_A))
	    || (-1 == cl->cl_sget ((unsigned char) type, name))
	    || (-1 == SLang_pop (&obj)))
	  {
	     SLang_free_object (&obj_A);
	     return -1;
	  }
	/* Now the value of A.x is in obj. */
	if (-1 == perform_lvalue_operation (op, &obj))
	  {
	     SLang_free_object (&obj);
	     SLang_free_object (&obj_A);
	     return -1;
	  }
	SLang_free_object (&obj);
	/* The result of the operation is now on the stack.
	 * Perform assignment */
	if (-1 == SLang_push (&obj_A))
	  {
	     SLang_free_object (&obj_A);
	     return -1;
	  }
     }

   return (*cl->cl_sput) ((unsigned char) type, name);
}

static int make_unit_object (SLang_Object_Type *a, SLang_Object_Type *u)
{
   unsigned char type;
   
   type = a->data_type;
   if (type == SLANG_ARRAY_TYPE)
     type = a->v.array_val->data_type;
   
   u->data_type = type;
   switch (type)
     {
      case SLANG_UCHAR_TYPE:
      case SLANG_CHAR_TYPE:
	u->v.char_val = 1;
	break;

      case SLANG_SHORT_TYPE:
      case SLANG_USHORT_TYPE:
	u->v.short_val = 1;
	break;

      case SLANG_LONG_TYPE:
      case SLANG_ULONG_TYPE:
	u->v.long_val = 1;
	break;

#if SLANG_HAS_FLOAT
      case SLANG_FLOAT_TYPE:
	u->v.float_val = 1;
	break;
	
      case SLANG_COMPLEX_TYPE:
	u->data_type = SLANG_DOUBLE_TYPE;
      case SLANG_DOUBLE_TYPE:
	u->v.double_val = 1;
	break;
#endif
      default:
	u->data_type = SLANG_INT_TYPE;
	u->v.int_val = 1;
     }
   return 0;
}


/* We want to convert 'A[i] op X' to 'A[i] = A[i] op X'.  The code that
 * has been generated is:  X __args i A __aput-op
 * where __aput-op represents this function.  We need to generate:
 * __args i A __eargs __aget X op __args i A __eargs __aput
 * Here, __eargs implies a call to do_bc_call_direct_frame with either
 * the aput or aget function.  In addition, __args represents a call to 
 * SLang_start_arg_list.  Of course, i represents a set of indices.
 * 
 * Note: If op is an unary operation (e.g., ++ or --), then X will not
 * b present an will have to be taken to be 1.
 * 
 * Implementation note: For efficiency, calls to setup the frame, start
 * arg list will be omitted and SLang_Num_Function_Args will be set.
 * This is ugly but the alternative is much less efficient rendering these
 * assignment operators useless.  So, the plan is to roll the stack to get X,
 * then duplicate the next N values, call __aget followed by op X, finally
 * calling __aput.  Hence, the sequence is:
 * 
 *     start:   X i .. j A 
 *      dupN:   X i .. j A i .. j A
 *    __aget:   X i .. j A Y
 *      roll:   i .. j A Y X
 *        op:   i .. j A Z
 *      roll:   Z i .. j A
 *    __aput:
 */
static int
set_array_lvalue (int op)
{
   SLang_Object_Type x, y;
   int num_args, is_unary;

   if (-1 == map_assignment_op_to_binary (op, &op, &is_unary))
     return -1;

   /* Grab the indices and the array.  Do not start a new frame. */
   if (-1 == SLang_end_arg_list ())
     return -1;
   num_args = Next_Function_Num_Args;
   Next_Function_Num_Args = 0;

   if (-1 == SLdup_n (num_args))
     return -1;

   SLang_Num_Function_Args = num_args;
   if (-1 == _SLarray_aget ())
     return -1;

   if (-1 == SLang_pop (&y))
     return -1;
   
   if (is_unary == 0)
     {
	if ((-1 == SLroll_stack (-(num_args + 1)))
	    || (-1 == SLang_pop (&x)))
	  {
	     SLang_free_object (&y);
	     return -1;
	  }
     }
   else if (-1 == make_unit_object (&y, &x))
     {
	SLang_free_object (&y);
	return -1;
     }
   
   if (-1 == do_binary_ab (op, &y, &x))
     {
	SLang_free_object (&y);
	SLang_free_object (&x);
	return -1;
     }
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR != Class_Type [y.data_type])
#endif
     SLang_free_object (&y);
   
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR != Class_Type [x.data_type])
#endif
     SLang_free_object (&x);

   if (-1 == SLroll_stack (num_args + 1))
     return -1;

   SLang_Num_Function_Args = num_args;
   return _SLarray_aput ();
}


static int
set_intrin_lvalue (SLBlock_Type *bc_blk)
{
   unsigned char op_type;
   SLang_Object_Type obja;
   SLang_Class_Type *cl;
   SLang_Intrin_Var_Type *ivar;
   VOID_STAR intrinsic_addr;
   unsigned char intrinsic_type;

   ivar = bc_blk->b.nt_ivar_blk;

   intrinsic_type = ivar->type;
   intrinsic_addr = ivar->addr;

   op_type = bc_blk->bc_sub_type;

   cl = _SLclass_get_class (intrinsic_type);

   if (op_type != _SLANG_BCST_ASSIGN)
     {
	/* We want to get the current value into obja.  This is the
	 * easiest way.
	 */
	if ((-1 == (*cl->cl_push) (intrinsic_type, intrinsic_addr))
	    || (-1 == SLang_pop (&obja)))
	  return -1;

	(void) perform_lvalue_operation (op_type, &obja);
	SLang_free_object (&obja);

	if (SLang_Error)
	  return -1;
     }

   return (*cl->cl_pop) (intrinsic_type, intrinsic_addr);
}

int _SLang_deref_assign (SLang_Ref_Type *ref)
{
   SLang_Object_Type *objp;
   SLang_Name_Type *nt;
   SLBlock_Type blk;

   if (ref->is_global == 0)
     {
	objp = ref->v.local_obj;
	if (objp > Local_Variable_Frame)
	  {
	     SLang_verror (SL_UNDEFINED_NAME, "Local variable reference is out of scope");
	     return -1;
	  }
	return set_lvalue_obj (_SLANG_BCST_ASSIGN, objp);
     }

   nt = ref->v.nt;
   switch (nt->name_type)
     {
      case SLANG_GVARIABLE:
      case SLANG_PVARIABLE:
	if (-1 == set_lvalue_obj (_SLANG_BCST_ASSIGN,
				  &((SLang_Global_Var_Type *)nt)->obj))
	  {
	     do_name_type_error (nt);
	     return -1;
	  }
	break;

      case SLANG_IVARIABLE:
	blk.b.nt_blk = nt;
	blk.bc_sub_type = _SLANG_BCST_ASSIGN;
	if (-1 == set_intrin_lvalue (&blk))
	  {
	     do_name_type_error (nt);
	     return -1;
	  }
	break;

      case SLANG_LVARIABLE:
	SLang_Error = SL_INTERNAL_ERROR;
	/* set_intrin_lvalue (&blk); */
	return -1;

      case SLANG_RVARIABLE:
      default:
	SLang_verror (SL_READONLY_ERROR, "deref assignment to %s not allowed", nt->name);
	return -1;
     }

   return 0;
}

static void set_deref_lvalue (SLBlock_Type *bc_blk)
{
   SLang_Object_Type *objp;
   SLang_Ref_Type *ref;

   switch (bc_blk->bc_sub_type)
     {
      case SLANG_LVARIABLE:
	objp =  (Local_Variable_Frame - bc_blk->b.i_blk);
	break;
      case SLANG_GVARIABLE:
      case SLANG_PVARIABLE:
	objp = &bc_blk->b.nt_gvar_blk->obj;
	break;
      default:
	SLang_Error = SL_INTERNAL_ERROR;
	return;
     }

   if (-1 == _SLpush_slang_obj (objp))
     return;

   if (-1 == SLang_pop_ref (&ref))
     return;
   (void) _SLang_deref_assign (ref);
   SLang_free_ref (ref);
}

static int push_struct_field (char *name)
{
   int type;
   SLang_Class_Type *cl;

   if (-1 == (type = SLang_peek_at_stack ()))
     return -1;

   cl = _SLclass_get_class ((unsigned char) type);
   if (cl->cl_sget == NULL)
     {
	SLang_verror (SL_NOT_IMPLEMENTED,
		      "%s does not permit structure access",
		      cl->cl_name);
	SLdo_pop_n (2);
	return -1;
     }

   return (*cl->cl_sget) ((unsigned char) type, name);
}

static void trace_dump (char *format, char *name, SLang_Object_Type *objs, int n, int dir)
{
   unsigned int len;
   char prefix [52];

   len = Trace_Mode - 1;
   if (len + 2 >= sizeof (prefix))
     len = sizeof (prefix) - 2;

   SLMEMSET (prefix, ' ', len);
   prefix[len] = 0;

   call_dump_routine (prefix);
   call_dump_routine (format, name, n);

   if (n > 0)
     {
	prefix[len] = ' ';
	len++;
	prefix[len] = 0;

	_SLdump_objects (prefix, objs, n, dir);
     }
}

/*  Pop a data item from the stack and return a pointer to it.
 *  Strings are not freed from stack so use another routine to do it.
 */
static VOID_STAR pop_pointer (SLang_Object_Type *obj, unsigned char type)
{
#ifndef _SLANG_OPTIMIZE_FOR_SPEED
   SLang_Class_Type *cl;
#endif

   SLang_Array_Type *at;

   /* Arrays are special.  Allow scalars to automatically convert to arrays.
    */
   if (type == SLANG_ARRAY_TYPE)
     {
	if (-1 == SLang_pop_array (&at, 1))
	  return NULL;
	obj->data_type = SLANG_ARRAY_TYPE;
	return obj->v.ptr_val = (VOID_STAR) at;
     }

   if (type == 0)
     {
	/* This happens when an intrinsic is declared without any information
	 * regarding parameter types.
	 */
	if (-1 == SLang_pop (obj))
	  return NULL;
	type = obj->data_type;
     }
   else if (-1 == _SLang_pop_object_of_type (type, obj, 0))
     return NULL;

#if _SLANG_OPTIMIZE_FOR_SPEED
   type = Class_Type [type];
#else
   type = _SLclass_get_class (type)->cl_class_type;
#endif

   if (type == SLANG_CLASS_TYPE_SCALAR)
     return (VOID_STAR) &obj->v;
   else if (type == SLANG_CLASS_TYPE_MMT)
     return SLang_object_from_mmt (obj->v.ref);
   else
     return obj->v.ptr_val;
}

/* This is ugly.  Does anyone have a advice for a cleaner way of doing
 * this??
 */
typedef void (*VF0_Type)(void);
typedef void (*VF1_Type)(VOID_STAR);
typedef void (*VF2_Type)(VOID_STAR, VOID_STAR);
typedef void (*VF3_Type)(VOID_STAR, VOID_STAR, VOID_STAR);
typedef void (*VF4_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef void (*VF5_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef void (*VF6_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef void (*VF7_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef long (*LF0_Type)(void);
typedef long (*LF1_Type)(VOID_STAR);
typedef long (*LF2_Type)(VOID_STAR, VOID_STAR);
typedef long (*LF3_Type)(VOID_STAR, VOID_STAR, VOID_STAR);
typedef long (*LF4_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef long (*LF5_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef long (*LF6_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef long (*LF7_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
#if SLANG_HAS_FLOAT
typedef double (*FF0_Type)(void);
typedef double (*FF1_Type)(VOID_STAR);
typedef double (*FF2_Type)(VOID_STAR, VOID_STAR);
typedef double (*FF3_Type)(VOID_STAR, VOID_STAR, VOID_STAR);
typedef double (*FF4_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef double (*FF5_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef double (*FF6_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef double (*FF7_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
#endif

static int execute_intrinsic_fun (SLang_Intrin_Fun_Type *objf)
{
#if SLANG_HAS_FLOAT
   double xf;
#endif
   VOID_STAR p[SLANG_MAX_INTRIN_ARGS];
   SLang_Object_Type objs[SLANG_MAX_INTRIN_ARGS];
   long ret;
   unsigned char type;
   unsigned int argc;
   unsigned int i;
   FVOID_STAR fptr;
   unsigned char *arg_types;
   int stk_depth;

   fptr = objf->i_fun;
   argc = objf->num_args;
   type = objf->return_type;
   arg_types = objf->arg_types;

   if (argc > SLANG_MAX_INTRIN_ARGS)
     {
	SLang_verror(SL_APPLICATION_ERROR,
		     "Intrinsic function %s requires too many parameters", objf->name);
	return -1;
     }

   if (-1 == _SL_increment_frame_pointer ())
     return -1;

   stk_depth = -1;
   if (Trace_Mode && (_SLang_Trace > 0))
     {
	int nargs;

	stk_depth = _SLstack_depth ();

	nargs = SLang_Num_Function_Args;
	if (nargs == 0)
	  nargs = (int)argc;

	stk_depth -= nargs;

	if (stk_depth >= 0)
	  trace_dump (">>%s (%d args)\n",
		      objf->name,
		      _SLStack_Pointer - nargs,
		      nargs,
		      1);
     }

   i = argc;
   while (i != 0)
     {
	i--;
	if (NULL == (p[i] = pop_pointer (objs + i, arg_types[i])))
	  {
	     i++;
	     goto free_and_return;
	  }
     }

   ret = 0;
#if SLANG_HAS_FLOAT
   xf = 0.0;
#endif

   switch (argc)
     {
      case 0:
	if (type == SLANG_VOID_TYPE) ((VF0_Type) fptr) ();
#if SLANG_HAS_FLOAT
	else if (type == SLANG_DOUBLE_TYPE) xf = ((FF0_Type) fptr)();
#endif
	else ret = ((LF0_Type) fptr)();
	break;

      case 1:
	if (type == SLANG_VOID_TYPE) ((VF1_Type) fptr)(p[0]);
#if SLANG_HAS_FLOAT
	else if (type == SLANG_DOUBLE_TYPE) xf =  ((FF1_Type) fptr)(p[0]);
#endif
	else ret =  ((LF1_Type) fptr)(p[0]);
	break;

      case 2:
	if (type == SLANG_VOID_TYPE)  ((VF2_Type) fptr)(p[0], p[1]);
#if SLANG_HAS_FLOAT
	else if (type == SLANG_DOUBLE_TYPE) xf = ((FF2_Type) fptr)(p[0], p[1]);
#endif
	else ret = ((LF2_Type) fptr)(p[0], p[1]);
	break;

      case 3:
	if (type == SLANG_VOID_TYPE) ((VF3_Type) fptr)(p[0], p[1], p[2]);
#if SLANG_HAS_FLOAT
	else if (type == SLANG_DOUBLE_TYPE) xf = ((FF3_Type) fptr)(p[0], p[1], p[2]);
#endif
	else ret = ((LF3_Type) fptr)(p[0], p[1], p[2]);
	break;

      case 4:
	if (type == SLANG_VOID_TYPE) ((VF4_Type) fptr)(p[0], p[1], p[2], p[3]);
#if SLANG_HAS_FLOAT
	else if (type == SLANG_DOUBLE_TYPE) xf = ((FF4_Type) fptr)(p[0], p[1], p[2], p[3]);
#endif
	else ret = ((LF4_Type) fptr)(p[0], p[1], p[2], p[3]);
	break;

      case 5:
	if (type == SLANG_VOID_TYPE) ((VF5_Type) fptr)(p[0], p[1], p[2], p[3], p[4]);
#if SLANG_HAS_FLOAT
	else if (type == SLANG_DOUBLE_TYPE) xf = ((FF5_Type) fptr)(p[0], p[1], p[2], p[3], p[4]);
#endif
	else ret = ((LF5_Type) fptr)(p[0], p[1], p[2], p[3], p[4]);
	break;

      case 6:
	if (type == SLANG_VOID_TYPE) ((VF6_Type) fptr)(p[0], p[1], p[2], p[3], p[4], p[5]);
#if SLANG_HAS_FLOAT
	else if (type == SLANG_DOUBLE_TYPE) xf = ((FF6_Type) fptr)(p[0], p[1], p[2], p[3], p[4], p[5]);
#endif
	else ret = ((LF6_Type) fptr)(p[0], p[1], p[2], p[3], p[4], p[5]);
	break;

      case 7:
	if (type == SLANG_VOID_TYPE) ((VF7_Type) fptr)(p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
#if SLANG_HAS_FLOAT
	else if (type == SLANG_DOUBLE_TYPE) xf = ((FF7_Type) fptr)(p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
#endif
	else ret = ((LF7_Type) fptr)(p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
	break;
     }

   switch (type)
     {
      case SLANG_VOID_TYPE:
	break;
	
#if SLANG_HAS_FLOAT
      case SLANG_DOUBLE_TYPE:
	(void) SLclass_push_double_obj (SLANG_DOUBLE_TYPE, xf);
	break;
#endif
      case SLANG_UINT_TYPE:
      case SLANG_INT_TYPE: (void) SLclass_push_int_obj (type, (int) ret);
	break;
	
      case SLANG_CHAR_TYPE:
      case SLANG_UCHAR_TYPE: (void) SLclass_push_char_obj (type, (char) ret);
	break;

      case SLANG_SHORT_TYPE:
      case SLANG_USHORT_TYPE: (void) SLclass_push_short_obj (type, (short) ret);
	break;

      case SLANG_LONG_TYPE:
      case SLANG_ULONG_TYPE: (void) SLclass_push_long_obj (type, ret);
	break;
	
      case SLANG_STRING_TYPE:
	if (NULL == (char *)ret)
	  {
	     if (SLang_Error == 0) SLang_Error = SL_INTRINSIC_ERROR;
	  }
	else (void) SLang_push_string ((char *)ret);
	break;
	
      default:
	SLang_verror (SL_NOT_IMPLEMENTED,
		      "Support for intrinsic functions returning %s is not provided",
		      SLclass_get_datatype_name (type));
     }

   if (stk_depth >= 0)
     {
	stk_depth = _SLstack_depth () - stk_depth;

	trace_dump ("<<%s (returning %d values)\n",
		      objf->name,
		      _SLStack_Pointer - stk_depth,
		      stk_depth,
		      1);
     }

   free_and_return:
   while (i < argc)
     {
	SLang_free_object (objs + i);
	i++;
     }

   return _SL_decrement_frame_pointer ();
}

static int inner_interp(register SLBlock_Type *);

/* Switch_Obj_Ptr points to the NEXT available free switch object */
static SLang_Object_Type Switch_Objects[SLANG_MAX_NESTED_SWITCH];
static SLang_Object_Type *Switch_Obj_Ptr = Switch_Objects;
static SLang_Object_Type *Switch_Obj_Max = Switch_Objects + SLANG_MAX_NESTED_SWITCH;

static void
lang_do_loops (unsigned char stype, SLBlock_Type *block, unsigned int num_blocks)
{
   int i, ctrl;
   int first, last;
   SLBlock_Type *blks[4];
   char *loop_name;
   SLang_Foreach_Context_Type *foreach_context;
   SLang_Class_Type *cl;
   int type;
   unsigned int j;

   j = 0;
   for (i = 0; i < (int) num_blocks; i++)
     {
	if (block[i].bc_main_type != _SLANG_BC_BLOCK)
	  {
	     if (block[i].bc_main_type == _SLANG_BC_LINE_NUM)
	       continue;

	     SLang_verror (SL_SYNTAX_ERROR, "Bytecode is not a looping block");
	     return;
	  }
	blks[j] = block[i].b.blk;
	j++;
     }

   num_blocks = j;
   block = blks[0];

   switch (stype)
     {
	int next_fn_args;

      case _SLANG_BCST_FOREACH:
	loop_name = "foreach";
	if (num_blocks != 1)
	  goto wrong_num_blocks_error;

	/* We should find Next_Function_Num_Args + 1 items on the stack.
	 * The first Next_Function_Num_Args items represent the arguments to
	 * to USING.  The last item (deepest in stack) is the object to loop
	 * over.  So, roll the stack up and grab it.
	 */
	next_fn_args = Next_Function_Num_Args;
	Next_Function_Num_Args = 0;
	if ((-1 == SLroll_stack (-(next_fn_args + 1)))
	    || (-1 == (type = SLang_peek_at_stack ())))
	  goto return_error;

	cl = _SLclass_get_class ((unsigned char) type);
	if ((cl->cl_foreach == NULL)
	    || (cl->cl_foreach_open == NULL)
	    || (cl->cl_foreach_close == NULL))
	  {
	     SLang_verror (SL_NOT_IMPLEMENTED, "%s does not permit foreach", cl->cl_name);
	     SLdo_pop_n (next_fn_args + 1);
	     goto return_error;
	  }

	if (NULL == (foreach_context = (*cl->cl_foreach_open) ((unsigned char)type, next_fn_args)))
	  goto return_error;

	while (1)
	  {
	     int status;

	     if (SLang_Error)
	       {
		  (*cl->cl_foreach_close) ((unsigned char) type, foreach_context);
		  goto return_error;
	       }

	     status = (*cl->cl_foreach) ((unsigned char) type, foreach_context);
	     if (status <= 0)
	       {
		  if (status == 0)
		    break;

		  (*cl->cl_foreach_close) ((unsigned char) type, foreach_context);
		  goto return_error;
	       }

	     inner_interp (block);
	     if (Lang_Break) break;
	     Lang_Break_Condition = /* Lang_Continue = */ 0;
	  }
	(*cl->cl_foreach_close) ((unsigned char) type, foreach_context);
	break;

      case _SLANG_BCST_WHILE:
	loop_name = "while";

	if (num_blocks != 2)
	  goto wrong_num_blocks_error;

	type = blks[1]->bc_main_type;
	while (1)
	  {
	     if (SLang_Error)
	       goto return_error;

	     inner_interp (block);
	     if (Lang_Break) break;

	     if (-1 == pop_ctrl_integer (&ctrl))
	       goto return_error;

	     if (ctrl == 0) break;

	     if (type)
	       {
		  inner_interp (blks[1]);
		  if (Lang_Break) break;
		  Lang_Break_Condition = /* Lang_Continue = */ 0;
	       }
	  }
	break;

      case _SLANG_BCST_DOWHILE:
	loop_name = "do...while";

	if (num_blocks != 2)
	  goto wrong_num_blocks_error;

	while (1)
	  {
	     if (SLang_Error)
	       goto return_error;

	     Lang_Break_Condition = /* Lang_Continue = */ 0;
	     inner_interp (block);
	     if (Lang_Break) break;
	     Lang_Break_Condition = /* Lang_Continue = */ 0;
	     inner_interp (blks[1]);
	     if (-1 == pop_ctrl_integer (&ctrl))
	       goto return_error;

	     if (ctrl == 0) break;
	  }
	break;

      case _SLANG_BCST_CFOR:
	loop_name = "for";

	/* we need 4 blocks: first 3 control, the last is code */
	if (num_blocks != 4) goto wrong_num_blocks_error;

	inner_interp (block);
	while (1)
	  {
	     if (SLang_Error)
	       goto return_error;

	     inner_interp(blks[1]);       /* test */
	     if (-1 == pop_ctrl_integer (&ctrl))
	       goto return_error;

	     if (ctrl == 0) break;
	     inner_interp(blks[3]);       /* code */
	     if (Lang_Break) break;
	     inner_interp(blks[2]);       /* bump */
	     Lang_Break_Condition = /* Lang_Continue = */ 0;
	  }
	break;

      case _SLANG_BCST_FOR:
	loop_name = "_for";

	if (num_blocks != 1)
	  goto wrong_num_blocks_error;

	/* 3 elements: first, last, step */
	if ((-1 == SLang_pop_integer (&ctrl))
	    || (-1 == SLang_pop_integer (&last))
	    || (-1 == SLang_pop_integer (&first)))
	  goto return_error;

	i = first;
	while (1)
	  {
	     /* It is ugly to have this test here but I do not know of a
	      * simple way to do this without using two while loops.
	      */
	     if (ctrl >= 0)
	       {
		  if (i > last) break;
	       }
	     else if (i < last) break;

	     if (SLang_Error) goto return_error;

	     SLclass_push_int_obj (SLANG_INT_TYPE, i);
	     inner_interp (block);
	     if (Lang_Break) break;
	     Lang_Break_Condition = /* Lang_Continue = */ 0;

	     i += ctrl;
	  }
	break;

      case _SLANG_BCST_LOOP:
	loop_name = "loop";
	if (num_blocks != 1)
	  goto wrong_num_blocks_error;

	if (-1 == SLang_pop_integer (&ctrl))
	  goto return_error;
	while (ctrl > 0)
	  {
	     ctrl--;

	     if (SLang_Error)
	       goto return_error;

	     inner_interp (block);
	     if (Lang_Break) break;
	     Lang_Break_Condition = /* Lang_Continue = */ 0;
	  }
	break;

      case _SLANG_BCST_FOREVER:
	loop_name = "forever";

	if (num_blocks != 1)
	  goto wrong_num_blocks_error;

	while (1)
	  {
	     if (SLang_Error)
	       goto return_error;

	     inner_interp (block);
	     if (Lang_Break) break;
	     Lang_Break_Condition = /* Lang_Continue = */ 0;
	  }
	break;

      default:  SLang_verror(SL_INTERNAL_ERROR, "Unknown loop type");
	return;
     }
   Lang_Break = /* Lang_Continue = */ 0;
   Lang_Break_Condition = Lang_Return;
   return;

   wrong_num_blocks_error:
   SLang_verror (SL_SYNTAX_ERROR, "Wrong number of blocks for '%s' construct", loop_name);

   /* drop */
   return_error:
   do_traceback (loop_name, 0, NULL);
}

static void lang_do_and_orelse (unsigned char stype, SLBlock_Type *addr, SLBlock_Type *addr_max)
{
   int test = 0;
   int is_or;

   is_or = (stype == _SLANG_BCST_ORELSE);

   while (addr <= addr_max)
     {
	if (addr->bc_main_type == _SLANG_BC_LINE_NUM)
	  {
	     addr++;
	     continue;
	  }

	inner_interp (addr->b.blk);
	if (SLang_Error
	    || Lang_Break_Condition
	    || (-1 == pop_ctrl_integer (&test)))
	  return;

	if (is_or == (test != 0))
	  break;

	/* if (((stype == _SLANG_BCST_ANDELSE) && (test == 0))
	 *   || ((stype == _SLANG_BCST_ORELSE) && test))
	 * break;
	 */

	addr++;
     }
   SLclass_push_int_obj (SLANG_INT_TYPE, test);
}

static void do_else_if (SLBlock_Type *zero_block, SLBlock_Type *non_zero_block)
{
   int test;

   if (-1 == pop_ctrl_integer (&test))
     return;

   if (test == 0)
     non_zero_block = zero_block;

   if (non_zero_block != NULL)
     inner_interp (non_zero_block->b.blk);
}

int _SLang_trace_fun (char *f)
{
   if (NULL == (f = SLang_create_slstring (f)))
     return -1;

   SLang_free_slstring (Trace_Function);
   Trace_Function = f;
   _SLang_Trace = 1;
   return 0;
}

int _SLdump_objects (char *prefix, SLang_Object_Type *x, unsigned int n, int dir)
{
   char *s;
   SLang_Class_Type *cl;

   while (n)
     {
	cl = _SLclass_get_class (x->data_type);

	if (NULL == (s = _SLstringize_object (x)))
	  s = "??";

	call_dump_routine ("%s[%s]:%s\n", prefix, cl->cl_name, s);

	SLang_free_slstring (s);

	x += dir;
	n--;
     }
   return 0;
}

static SLBlock_Type *Exit_Block_Ptr;
static SLBlock_Type *Global_User_Block[5];
static SLBlock_Type **User_Block_Ptr = Global_User_Block;
static char *Current_Function_Name = NULL;

static int execute_slang_fun (_SLang_Function_Type *fun)
{
   register unsigned int i;
   register SLang_Object_Type *frame, *lvf;
   register unsigned int n_locals;
   _SLBlock_Header_Type *header;
   /* SLBlock_Type *val; */
   SLBlock_Type *exit_block_save;
   SLBlock_Type **user_block_save;
   SLBlock_Type *user_blocks[5];
   char *save_fname;

   exit_block_save = Exit_Block_Ptr;
   user_block_save = User_Block_Ptr;
   User_Block_Ptr = user_blocks;
   *(user_blocks) = NULL;
   *(user_blocks + 1) = NULL;
   *(user_blocks + 2) = NULL;
   *(user_blocks + 3) = NULL;
   *(user_blocks + 4) = NULL;

   Exit_Block_Ptr = NULL;

   save_fname = Current_Function_Name;
   Current_Function_Name = fun->name;

   _SL_increment_frame_pointer ();

   /* need loaded?  */
   if (fun->nlocals == AUTOLOAD_NUM_LOCALS)
     {
	/* header = NULL; */
	if (-1 == SLang_load_file(fun->v.autoload_filename))
	  goto the_return;

	if (fun->nlocals == AUTOLOAD_NUM_LOCALS)
	  {
	     SLang_verror (SL_UNDEFINED_NAME, "%s: Function did not autoload",
			   Current_Function_Name);
             goto the_return;
	  }
     }

   n_locals = fun->nlocals;

   /* let the error propagate through since it will do no harm
    and allow us to restore stack. */

   /* set new stack frame */
   lvf = frame = Local_Variable_Frame;
   i = n_locals;
   if ((lvf + i) > Local_Variable_Stack + SLANG_MAX_LOCAL_STACK)
     {
	SLang_verror(SL_STACK_OVERFLOW, "%s: Local Variable Stack Overflow",
		     Current_Function_Name);
	goto the_return;
     }

   /* Make sure we do not allow this header to get destroyed by something
    * like:  define crash () { eval ("define crash ();") }
    */
   header = fun->v.header;
   header->num_refs++;

   while (i--)
     {
	lvf++;
	lvf->data_type = SLANG_UNDEFINED_TYPE;
     }
   Local_Variable_Frame = lvf;

   /* read values of function arguments */
   i = fun->nargs;
   while (i > 0)
     {
	i--;
	(void) SLang_pop (Local_Variable_Frame - i);
     }

   if (SLang_Enter_Function != NULL) (*SLang_Enter_Function)(Current_Function_Name);

   if (_SLang_Trace)
     {
	int stack_depth;

	stack_depth = _SLstack_depth ();

	if ((Trace_Function != NULL)
	    && (0 == strcmp (Trace_Function, Current_Function_Name))
	    && (Trace_Mode == 0))
	  Trace_Mode = 1;

	if (Trace_Mode)
	  {
	     /* The local variable frame grows backwards */
	     trace_dump (">>%s (%d args)\n",
			 Current_Function_Name,
			 Local_Variable_Frame,
			 (int) fun->nargs,
			 -1);
	     Trace_Mode++;
	  }

	inner_interp (header->body);
	Lang_Break_Condition = Lang_Return = Lang_Break = 0;
	if (Exit_Block_Ptr != NULL) inner_interp(Exit_Block_Ptr);

	if (Trace_Mode)
	  {
	     Trace_Mode--;
	     stack_depth = _SLstack_depth () - stack_depth;

	     trace_dump ("<<%s (returning %d values)\n",
			 Current_Function_Name,
			 _SLStack_Pointer - stack_depth,
			 stack_depth,
			 1);

	     if (Trace_Mode == 1)
	       Trace_Mode = 0;
	  }
     }
   else
     {
	inner_interp (header->body);
	Lang_Break_Condition = Lang_Return = Lang_Break = 0;
	if (Exit_Block_Ptr != NULL) inner_interp(Exit_Block_Ptr);
     }

   if (SLang_Exit_Function != NULL) (*SLang_Exit_Function)(Current_Function_Name);

   if (SLang_Error)
     do_traceback(fun->name, n_locals,
#if _SLANG_HAS_DEBUG_CODE
		  fun->file
#else
		  NULL
#endif
		  );

   /* free local variables.... */
   lvf = Local_Variable_Frame;
   while (lvf > frame)
     {
#if _SLANG_OPTIMIZE_FOR_SPEED
	if (SLANG_CLASS_TYPE_SCALAR != Class_Type [lvf->data_type])
#endif
	  SLang_free_object (lvf);
	lvf--;
     }
   Local_Variable_Frame = lvf;

   if (header->num_refs == 1)
     free_function_header (header);
   else
     header->num_refs--;

   the_return:

   Lang_Break_Condition = Lang_Return = Lang_Break = 0;
   Exit_Block_Ptr = exit_block_save;
   User_Block_Ptr = user_block_save;
   Current_Function_Name = save_fname;
   _SL_decrement_frame_pointer ();

   if (SLang_Error)
     return -1;

   return 0;
}

static void do_traceback (char *name, unsigned int locals, char *file)
{
   char *s;
   unsigned int i;
   SLang_Object_Type *objp;
   unsigned short stype;

   /* FIXME: Priority=low
    * I need to make this configurable!!! That is, let the
    * application decide whether or not a usage error should result in a
    * traceback.
    */
   if (SLang_Error == SL_USAGE_ERROR)
     return;

   if (SLang_Traceback == 0)
     return;

   call_dump_routine ("S-Lang Traceback: %s\n", name);
   if (SLang_Traceback < 0)
     return;

   if (file != NULL)
     call_dump_routine ("File: %s\n", file);

   if (locals == 0)
     return;

   call_dump_routine ("  Local Variables:\n");

   for (i = 0; i < locals; i++)
     {
	SLang_Class_Type *cl;
	char *class_name;

	objp = Local_Variable_Frame - i;
	stype = objp->data_type;

	s = _SLstringize_object (objp);
	cl = _SLclass_get_class (stype);
	class_name = cl->cl_name;

	call_dump_routine ("\t$%d: Type: %s,\tValue:\t", i, class_name);

	if (s == NULL) call_dump_routine("??\n");
	else
	  {
	     char *q = "";
#ifndef HAVE_VSNPRINTF
	     char buf[256];
	     if (strlen (s) >= sizeof (buf))
	       {
		  strncpy (buf, s, sizeof(buf));
		  s = buf;
		  s[sizeof(buf) - 1] = 0;
	       }
#endif
	     if (SLANG_STRING_TYPE == stype) q = "\"";
	     call_dump_routine ("%s%s%s\n", q, s, q);
	  }
     }
}

static void do_app_unary (SLang_App_Unary_Type *nt)
{
   if (-1 == do_unary (nt->unary_op, nt->name_type))
     do_traceback (nt->name, 0, NULL);
}

static int inner_interp_nametype (SLang_Name_Type *nt)
{
   SLBlock_Type bc_blks[2];

   bc_blks[0].b.nt_blk = nt;
   bc_blks[0].bc_main_type = nt->name_type;
   bc_blks[1].bc_main_type = 0;
   return inner_interp(bc_blks);
}

int _SLang_dereference_ref (SLang_Ref_Type *ref)
{
   if (ref == NULL)
     {
	SLang_Error = SL_INTERNAL_ERROR;
	return -1;
     }

   if (ref->is_global == 0)
     {
	SLang_Object_Type *obj = ref->v.local_obj;
	if (obj > Local_Variable_Frame)
	  {
	     SLang_verror (SL_UNDEFINED_NAME, "Local variable deref is out of scope");
	     return -1;
	  }
	return _SLpush_slang_obj (ref->v.local_obj);
     }

   (void) inner_interp_nametype (ref->v.nt);
   return 0;
}

int _SLang_is_ref_initialized (SLang_Ref_Type *ref)
{
   unsigned char type;

   if (ref == NULL)
     {
	SLang_Error = SL_INTERNAL_ERROR;
	return -1;
     }

   if (ref->is_global == 0)
     {
	SLang_Object_Type *obj = ref->v.local_obj;
	if (obj > Local_Variable_Frame)
	  {
	     SLang_verror (SL_UNDEFINED_NAME, "Local variable deref is out of scope");
	     return -1;
	  }
	type = ref->v.local_obj->data_type;
     }
   else
     {
	SLang_Name_Type *nt = ref->v.nt;
	if ((nt->name_type != SLANG_GVARIABLE)
	    && (nt->name_type != SLANG_PVARIABLE))
	  return 1;
	type = ((SLang_Global_Var_Type *)nt)->obj.data_type;
     }
   return type != SLANG_UNDEFINED_TYPE;
}

int _SLang_uninitialize_ref (SLang_Ref_Type *ref)
{
   SLang_Object_Type *obj;

   if (ref == NULL)
     {
	SLang_Error = SL_INTERNAL_ERROR;
	return -1;
     }

   if (ref->is_global == 0)
     {
	obj = ref->v.local_obj;
	if (obj > Local_Variable_Frame)
	  {
	     SLang_verror (SL_UNDEFINED_NAME, "Local variable deref is out of scope");
	     return -1;
	  }
	obj = ref->v.local_obj;
     }
   else
     {
	SLang_Name_Type *nt = ref->v.nt;
	if ((nt->name_type != SLANG_GVARIABLE)
	    && (nt->name_type != SLANG_PVARIABLE))
	  return -1;
	obj = &((SLang_Global_Var_Type *)nt)->obj;
     }
   SLang_free_object (obj);
   obj->data_type = SLANG_UNDEFINED_TYPE;
   obj->v.ptr_val = NULL;
   return 0;
}

void (*SLang_Interrupt)(void);
static int Last_Error;
void (*SLang_User_Clear_Error)(void);
void _SLang_clear_error (void)
{
   if (Last_Error <= 0)
     {
	Last_Error = 0;
	return;
     }
   Last_Error--;
   if (SLang_User_Clear_Error != NULL) (*SLang_User_Clear_Error)();
}

int _SLpush_slang_obj (SLang_Object_Type *obj)
{
   unsigned char subtype;
   SLang_Class_Type *cl;

   if (obj == NULL) return SLang_push_null ();

   subtype = obj->data_type;

#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR == Class_Type[subtype])
     return SLang_push (obj);
#endif

   cl = _SLclass_get_class (subtype);
   return (*cl->cl_push) (subtype, (VOID_STAR) &obj->v);
}

_INLINE_
static int push_local_variable (int i)
{
   SLang_Class_Type *cl;
   SLang_Object_Type *obj;
   unsigned char subtype;

   obj = Local_Variable_Frame - i;
   subtype = obj->data_type;

#if _SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR == Class_Type[subtype])
     return SLang_push (obj);
   if (subtype == SLANG_STRING_TYPE)
     return _SLang_dup_and_push_slstring (obj->v.s_val);
#endif

   cl = _SLclass_get_class (subtype);
   return (*cl->cl_push) (subtype, (VOID_STAR) &obj->v);
}

static int push_intrinsic_variable (SLang_Intrin_Var_Type *ivar)
{
   SLang_Class_Type *cl;
   unsigned char stype;

   stype = ivar->type;
   cl = _SLclass_get_class (stype);

   if (-1 == (*cl->cl_push_intrinsic) (stype, ivar->addr))
     {
	do_name_type_error ((SLang_Name_Type *) ivar);
	return -1;
     }
   return 0;
}

static int dereference_object (void)
{
   SLang_Object_Type obj;
   SLang_Class_Type *cl;
   unsigned char type;
   int ret;

   if (-1 == SLang_pop (&obj))
     return -1;

   type = obj.data_type;

   cl = _SLclass_get_class (type);
   ret = (*cl->cl_dereference)(type, (VOID_STAR) &obj.v);

   SLang_free_object (&obj);
   return ret;
}

static int case_function (void)
{
   unsigned char type;
   SLang_Object_Type obj;
   SLang_Object_Type *swobjptr;

   swobjptr = Switch_Obj_Ptr - 1;

   if ((swobjptr < Switch_Objects)
       || (0 == (type = swobjptr->data_type)))
     {
	SLang_verror (SL_SYNTAX_ERROR, "Misplaced 'case' keyword");
	return -1;
     }

   if (-1 == SLang_pop (&obj))
     return -1;

   if (obj.data_type != type)
     {
	SLang_Class_Type *a_cl, *b_cl;

	a_cl = _SLclass_get_class (obj.data_type);
	b_cl = _SLclass_get_class (type);

	if (NULL == _SLclass_get_binary_fun (SLANG_EQ, a_cl, b_cl, &a_cl, 0))
	  {
	     (void) SLclass_push_int_obj (SLANG_INT_TYPE, 0);
	     SLang_free_object (&obj);
	     return 0;
	  }
     }

   (void) do_binary_ab (SLANG_EQ, swobjptr, &obj);
   SLang_free_object (&obj);
   return 0;
}

static void tmp_variable_function (SLBlock_Type *addr)
{
   SLang_Object_Type *obj;

   switch (addr->bc_sub_type)
     {
      case SLANG_GVARIABLE:
      case SLANG_PVARIABLE:
	obj = &addr->b.nt_gvar_blk->obj;
	break;

      case SLANG_LVARIABLE:
	obj = Local_Variable_Frame - addr->b.i_blk;
	break;

      default:
	SLang_Error = SL_INTERNAL_ERROR;
	return;
     }

   /* There is no need to go through higher level routines since we are
    * not creating or destroying extra copies.
    */
   if (-1 == SLang_push (obj))
     return;

   obj->data_type = SLANG_UNDEFINED_TYPE;
   obj->v.ptr_val = NULL;
}


static int
do_inner_interp_error (SLBlock_Type *err_block,
		       SLBlock_Type *addr_start,
		       SLBlock_Type *addr)
{
   int save_err, slerr;

   /* Someday I can use the these variable to provide extra information
    * about what went wrong.
    */
   (void) addr_start;
   (void) addr;

   if (err_block == NULL)
     goto return_error;

   if (SLang_Error < 0)		       /* errors less than 0 are severe */
     goto return_error;

   save_err = Last_Error++;
   slerr = SLang_Error;
   SLang_Error = 0;
   inner_interp (err_block->b.blk);

   if (Last_Error <= save_err)
     {
	/* Caught error and cleared it */
	Last_Error = save_err;
	if ((Lang_Break_Condition == 0)
	    /* An error may have cleared the error and then caused the
	     * function to return.  We will allow that but let's not allow
	     * 'break' nor 'continue' statements until later.
	     */
	    || Lang_Return)
	  return 0;

	/* drop--- either a break or continue was called */
     }

   Last_Error = save_err;
   SLang_Error = slerr;

   return_error:
#if _SLANG_HAS_DEBUG_CODE
   while (addr >= addr_start)
     {
	if (addr->bc_main_type == _SLANG_BC_LINE_NUM)
	  {
	     char buf[256];
	     sprintf (buf, "(Error occurred on line %lu)", addr->b.l_blk);
	     do_traceback (buf, 0, NULL);
	     break;
	  }
	/* Special hack for 16 bit systems to prevent pointer wrapping. */
#if defined(__16_BIT_SYSTEM__)
	if (addr == addr_start)
	  break;
#endif
	addr--;
     }
#endif
   return -1;
}


#define GATHER_STATISTICS 0
#if GATHER_STATISTICS
static unsigned int Bytecodes[0xFFFF];

static void print_stats (void)
{
   unsigned int i;
   unsigned long total;
   FILE *fp = fopen ("stats.txt", "w");
   if (fp == NULL)
     return;
   
   total = 0;
   for (i = 0; i < 0xFFFF; i++)
     total += Bytecodes[i];
   
   if (total == 0)
     total = 1;

   for (i = 0; i < 0xFFFF; i++)
     {
	if (Bytecodes[i])
	  fprintf (fp, "0x%04X %9u %e\n", i, Bytecodes[i], Bytecodes[i]/(double) total);
     }
   fclose (fp);
}

static void add_to_statistics (SLBlock_Type *b)
{
   unsigned short x, y;
   
   x = b->bc_main_type;
   if (x == 0)
     {
	Bytecodes[0] += 1;
	return;
     }
   b++;
   y = b->bc_main_type;

   Bytecodes[(x << 8) | y] += 1;
}

#endif

/* inner interpreter */
/* The return value from this function is only meaningful when it is used
 * to process blocks for the switch statement.  If it returns 0, the calling
 * routine should pass the next block to it.  Otherwise it will
 * return non-zero, with or without error.
 */
static int inner_interp (SLBlock_Type *addr_start)
{
   SLBlock_Type *block, *err_block, *addr;
#if GATHER_STATISTICS
   static int inited = 0;

   if (inited == 0)
     {
	(void) SLang_add_cleanup_function (print_stats);
	inited = 1;
     }
#endif

   /* for systems that have no real interrupt facility (e.g. go32 on dos) */
   if (SLang_Interrupt != NULL) (*SLang_Interrupt)();

   block = err_block = NULL;
   addr = addr_start;

#if GATHER_STATISTICS
   add_to_statistics (addr);
#endif
   while (1)
     {
	switch (addr->bc_main_type)
	  {
	   case 0:
	     return 1;
	   case _SLANG_BC_LVARIABLE:
	     push_local_variable (addr->b.i_blk);
	     break;
	   case _SLANG_BC_GVARIABLE:
	     if (-1 == _SLpush_slang_obj (&addr->b.nt_gvar_blk->obj))
	       do_name_type_error (addr->b.nt_blk);
	     break;

	   case _SLANG_BC_IVARIABLE:
	   case _SLANG_BC_RVARIABLE:
	     push_intrinsic_variable (addr->b.nt_ivar_blk);
	     break;

	   case _SLANG_BC_INTRINSIC:
	     execute_intrinsic_fun (addr->b.nt_ifun_blk);
	     if (SLang_Error)
	       do_traceback(addr->b.nt_ifun_blk->name, 0, NULL);
	     break;

	   case _SLANG_BC_FUNCTION:
	     execute_slang_fun (addr->b.nt_fun_blk);
	     if (Lang_Break_Condition) goto handle_break_condition;
	     break;

	   case _SLANG_BC_MATH_UNARY:
	   case _SLANG_BC_APP_UNARY:
	     /* Make sure we treat these like function calls since the
	      * parser took sin(x) to be a function call.
	      */
	     if (0 == _SL_increment_frame_pointer ())
	       {
		  do_app_unary (addr->b.nt_unary_blk);
		  (void) _SL_decrement_frame_pointer ();
	       }
	     break;

	   case _SLANG_BC_ICONST:
	     SLclass_push_int_obj (SLANG_INT_TYPE, addr->b.iconst_blk->i);
	     break;

#if SLANG_HAS_FLOAT
	   case _SLANG_BC_DCONST:
	     SLclass_push_double_obj (SLANG_DOUBLE_TYPE, addr->b.dconst_blk->d);
	     break;
#endif

	   case _SLANG_BC_PVARIABLE:
	     if (-1 == _SLpush_slang_obj (&addr->b.nt_gvar_blk->obj))
	       do_name_type_error (addr->b.nt_blk);
	     break;

	   case _SLANG_BC_PFUNCTION:
	     execute_slang_fun (addr->b.nt_fun_blk);
	     if (Lang_Break_Condition) goto handle_break_condition;
	     break;

	   case _SLANG_BC_BINARY:
	     do_binary (addr->b.i_blk);
	     break;
	     
	   case _SLANG_BC_LITERAL:
#if !_SLANG_OPTIMIZE_FOR_SPEED
	   case _SLANG_BC_LITERAL_INT:
	   case _SLANG_BC_LITERAL_STR:
	   case _SLANG_BC_LITERAL_DBL:
#endif
	       {
		  SLang_Class_Type *cl = _SLclass_get_class (addr->bc_sub_type);
		  (*cl->cl_push_literal) (addr->bc_sub_type, (VOID_STAR) &addr->b.ptr_blk);
	       }
	     break;
#if _SLANG_OPTIMIZE_FOR_SPEED
	   case _SLANG_BC_LITERAL_INT:
	     SLclass_push_int_obj (addr->bc_sub_type, (int) addr->b.l_blk);
	     break;
#if SLANG_HAS_FLOAT
	   case _SLANG_BC_LITERAL_DBL:
	     SLclass_push_double_obj (addr->bc_sub_type, *addr->b.double_blk);
	     break;
#endif
	   case _SLANG_BC_LITERAL_STR:
	     _SLang_dup_and_push_slstring (addr->b.s_blk);
	     break;
#endif
	   case _SLANG_BC_BLOCK:
	     switch (addr->bc_sub_type)
	       {
		case _SLANG_BCST_ERROR_BLOCK:
		  err_block = addr;
		  break;

		case _SLANG_BCST_EXIT_BLOCK:
		  Exit_Block_Ptr = addr->b.blk;
		  break;

		case _SLANG_BCST_USER_BLOCK0:
		case _SLANG_BCST_USER_BLOCK1:
		case _SLANG_BCST_USER_BLOCK2:
		case _SLANG_BCST_USER_BLOCK3:
		case _SLANG_BCST_USER_BLOCK4:
		  User_Block_Ptr[addr->bc_sub_type - _SLANG_BCST_USER_BLOCK0] = addr->b.blk;
		  break;

		case _SLANG_BCST_LOOP:
		case _SLANG_BCST_WHILE:
		case _SLANG_BCST_FOR:
		case _SLANG_BCST_FOREVER:
		case _SLANG_BCST_CFOR:
		case _SLANG_BCST_DOWHILE:
		case _SLANG_BCST_FOREACH:
		  if (block == NULL) block = addr;
		  lang_do_loops(addr->bc_sub_type, block, 1 + (unsigned int) (addr - block));
		  block = NULL;
		  break;

		case _SLANG_BCST_IFNOT:
#if _SLANG_OPTIMIZE_FOR_SPEED
		    {
		       int i;
		       
		       if ((0 == pop_ctrl_integer (&i)) && (i == 0))
			 inner_interp (addr->b.blk);
		    }
#else
		  do_else_if (addr, NULL);
#endif
		  break;

		case _SLANG_BCST_IF:
#if _SLANG_OPTIMIZE_FOR_SPEED
		    {
		       int i;
		       
		       if ((0 == pop_ctrl_integer (&i)) && i)
			 inner_interp (addr->b.blk);
		    }
#else
		  do_else_if (NULL, addr);
#endif
		  break;

		case _SLANG_BCST_NOTELSE:
		  do_else_if (block, addr);
		  block = NULL;
		  break;

		case _SLANG_BCST_ELSE:
		  do_else_if (addr, block);
		  block = NULL;
		  break;

		case _SLANG_BCST_SWITCH:
		  if (Switch_Obj_Ptr == Switch_Obj_Max)
		    {
		       SLang_doerror("switch nesting too deep");
		       break;
		    }
		  (void) SLang_pop (Switch_Obj_Ptr);
		  Switch_Obj_Ptr++;

		  if (block == NULL) block = addr;
		  while ((SLang_Error == 0)
			 && (block <= addr)
			 && (Lang_Break_Condition == 0)
			 && (0 == inner_interp (block->b.blk)))
		    block++;
		  Switch_Obj_Ptr--;
		  SLang_free_object (Switch_Obj_Ptr);
		  Switch_Obj_Ptr->data_type = 0;
		  block = NULL;
		  break;

		case _SLANG_BCST_ANDELSE:
		case _SLANG_BCST_ORELSE:
		  if (block == NULL) block = addr;
		  lang_do_and_orelse (addr->bc_sub_type, block, addr);
		  block = NULL;
		  break;

		default:
		  if (block == NULL) block =  addr;
		  break;
	       }
	     if (Lang_Break_Condition) goto handle_break_condition;
	     break;

	   case _SLANG_BC_RETURN:
	     Lang_Break_Condition = Lang_Return = Lang_Break = 1; return 1;
	   case _SLANG_BC_BREAK:
	     Lang_Break_Condition = Lang_Break = 1; return 1;
	   case _SLANG_BC_CONTINUE:
	     Lang_Break_Condition = /* Lang_Continue = */ 1; return 1;

	   case _SLANG_BC_EXCH:
	     (void) SLreverse_stack (2);
	     break;

	   case _SLANG_BC_LABEL:
	       {
		  int test;
		  if ((0 == SLang_pop_integer (&test))
		      && (test == 0))
		    return 0;
	       }
	     break;

	   case _SLANG_BC_LOBJPTR:
	     (void)_SLang_push_ref (0, (VOID_STAR)(Local_Variable_Frame - addr->b.i_blk));
	     break;

	   case _SLANG_BC_GOBJPTR:
	     (void)_SLang_push_ref (1, (VOID_STAR)addr->b.nt_blk);
	     break;

	   case _SLANG_BC_X_ERROR:
	     if (err_block != NULL)
	       {
		  inner_interp(err_block->b.blk);
		  if (SLang_Error) err_block = NULL;
	       }
	     else SLang_verror(SL_SYNTAX_ERROR, "No ERROR_BLOCK");
	     if (Lang_Break_Condition) goto handle_break_condition;
	     break;

	   case _SLANG_BC_X_USER0:
	   case _SLANG_BC_X_USER1:
	   case _SLANG_BC_X_USER2:
	   case _SLANG_BC_X_USER3:
	   case _SLANG_BC_X_USER4:
	     if (User_Block_Ptr[addr->bc_main_type - _SLANG_BC_X_USER0] != NULL)
	       {
		  inner_interp(User_Block_Ptr[addr->bc_main_type - _SLANG_BC_X_USER0]);
	       }
	     else SLang_verror(SL_SYNTAX_ERROR, "No block for X_USERBLOCK");
	     if (Lang_Break_Condition) goto handle_break_condition;
	     break;

	   case _SLANG_BC_CALL_DIRECT:
	     (*addr->b.call_function) ();
	     break;

	   case _SLANG_BC_CALL_DIRECT_FRAME:
	     do_bc_call_direct_frame (addr->b.call_function);
	     break;

	   case _SLANG_BC_UNARY:
	     do_unary (addr->b.i_blk, _SLANG_BC_UNARY);
	     break;
	     
	   case _SLANG_BC_UNARY_FUNC:
	     /* Make sure we treat these like function calls since the
	      * parser took abs(x) to be a function call.
	      */
	     if (0 == _SL_increment_frame_pointer ())
	       {
		  do_unary (addr->b.i_blk, _SLANG_BC_UNARY);
		  (void) _SL_decrement_frame_pointer ();
	       }
	     break;
	     
	   case _SLANG_BC_DEREF_ASSIGN:
	     set_deref_lvalue (addr);
	     break;
	   case _SLANG_BC_SET_LOCAL_LVALUE:
	     set_lvalue_obj (addr->bc_sub_type, Local_Variable_Frame - addr->b.i_blk);
	     break;
	   case _SLANG_BC_SET_GLOBAL_LVALUE:
	     if (-1 == set_lvalue_obj (addr->bc_sub_type, &addr->b.nt_gvar_blk->obj))
	       do_name_type_error (addr->b.nt_blk);
	     break;
	   case _SLANG_BC_SET_INTRIN_LVALUE:
	     set_intrin_lvalue (addr);
	     break;
	   case _SLANG_BC_SET_STRUCT_LVALUE:
	     set_struct_lvalue (addr);
	     break;

	   case _SLANG_BC_FIELD:
	     (void) push_struct_field (addr->b.s_blk);
	     break;

	   case _SLANG_BC_SET_ARRAY_LVALUE:
	     set_array_lvalue (addr->bc_sub_type);
	     break;

#if _SLANG_HAS_DEBUG_CODE
	   case _SLANG_BC_LINE_NUM:
	     break;
#endif
	     
	   case _SLANG_BC_TMP:
	     tmp_variable_function (addr);
	     break;

#if _SLANG_OPTIMIZE_FOR_SPEED
	   case _SLANG_BC_LVARIABLE_AGET:
	     if (0 == push_local_variable (addr->b.i_blk))
	       do_bc_call_direct_frame (_SLarray_aget);
	     break;

	   case _SLANG_BC_LVARIABLE_APUT:
	     if (0 == push_local_variable (addr->b.i_blk))
	       do_bc_call_direct_frame (_SLarray_aput);
	     break;
	   case _SLANG_BC_INTEGER_PLUS:
	     if (0 == SLclass_push_int_obj (addr->bc_sub_type, (int) addr->b.l_blk))
	       do_binary (SLANG_PLUS);
	     break;

	   case _SLANG_BC_INTEGER_MINUS:
	     if (0 == SLclass_push_int_obj (addr->bc_sub_type, (int) addr->b.l_blk))
	       do_binary (SLANG_MINUS);
	     break;
#endif
#if 0
	   case _SLANG_BC_ARG_LVARIABLE:
	     (void) SLang_start_arg_list ();
	     push_local_variable (addr->b.i_blk);
	     break;
#endif
	   case _SLANG_BC_EARG_LVARIABLE:
	     push_local_variable (addr->b.i_blk);
	     (void) SLang_end_arg_list ();
	     break;

#if USE_COMBINED_BYTECODES
	   case _SLANG_BC_CALL_DIRECT_INTRINSIC:
	     (*addr->b.call_function) ();
	     addr++;
	     execute_intrinsic_fun (addr->b.nt_ifun_blk);
	     if (SLang_Error)
	       do_traceback(addr->b.nt_ifun_blk->name, 0, NULL);
	     break;

	   case _SLANG_BC_INTRINSIC_CALL_DIRECT:
	     execute_intrinsic_fun (addr->b.nt_ifun_blk);
	     if (SLang_Error)
	       {
		  do_traceback(addr->b.nt_ifun_blk->name, 0, NULL);
		  break;
	       }
	     addr++;
	     (*addr->b.call_function) ();
	     break;

	   case _SLANG_BC_CALL_DIRECT_LSTR:
	     (*addr->b.call_function) ();
	     addr++;
	     _SLang_dup_and_push_slstring (addr->b.s_blk);
	     break;

	   case _SLANG_BC_CALL_DIRECT_SLFUN:
	     (*addr->b.call_function) ();
	     addr++;
	     execute_slang_fun (addr->b.nt_fun_blk);
	     if (Lang_Break_Condition) goto handle_break_condition;
	     break;

	   case _SLANG_BC_CALL_DIRECT_INTRSTOP:
	     (*addr->b.call_function) ();
	     addr++;
	     /* drop */
	   case _SLANG_BC_INTRINSIC_STOP:
	     execute_intrinsic_fun (addr->b.nt_ifun_blk);
	     if (SLang_Error == 0)
	       return 1;
	     do_traceback(addr->b.nt_ifun_blk->name, 0, NULL);
	     break;

	   case _SLANG_BC_CALL_DIRECT_EARG_LVAR:
	     (*addr->b.call_function) ();
	     addr++;
	     push_local_variable (addr->b.i_blk);
	     (void) SLang_end_arg_list ();
	     break;

	   case _SLANG_BC_CALL_DIRECT_LINT:
	     (*addr->b.call_function) ();
	     addr++;
	     SLclass_push_int_obj (addr->bc_sub_type, (int) addr->b.l_blk);
	     break;

	   case _SLANG_BC_CALL_DIRECT_LVAR:
	     (*addr->b.call_function) ();
	     addr++;
	     push_local_variable (addr->b.i_blk);
	     break;
	     
	   case _SLANG_BC_LLVARIABLE_BINARY:
	     do_binary_ab_inc_ref (addr->b.i_blk, 
			   Local_Variable_Frame - (addr+1)->b.i_blk,
			   Local_Variable_Frame - (addr+2)->b.i_blk);
	     addr += 2;
	     break;
	   case _SLANG_BC_LGVARIABLE_BINARY:
	     do_binary_ab_inc_ref (addr->b.i_blk, 
			   Local_Variable_Frame - (addr+1)->b.i_blk,
			   &(addr+2)->b.nt_gvar_blk->obj);
	     addr += 2;
	     break;

	   case _SLANG_BC_GLVARIABLE_BINARY:
	     do_binary_ab_inc_ref (addr->b.i_blk, 
			   &(addr+1)->b.nt_gvar_blk->obj,
			   Local_Variable_Frame - (addr+2)->b.i_blk);
	     addr += 2;
	     break;
	   case _SLANG_BC_GGVARIABLE_BINARY:
	     do_binary_ab_inc_ref (addr->b.i_blk,
			   &(addr+1)->b.nt_gvar_blk->obj,
			   &(addr+2)->b.nt_gvar_blk->obj);
	     addr += 2;
	     break;
	     
	   case _SLANG_BC_LIVARIABLE_BINARY:
	       {
		  SLang_Object_Type o;
		  o.data_type = SLANG_INT_TYPE;
		  o.v.int_val = (int) (addr+2)->b.l_blk;

		  do_binary_ab_inc_ref (addr->b.i_blk, 
				Local_Variable_Frame - (addr+1)->b.i_blk,
				&o);
	       }
	     addr += 2;
	     break;
#if SLANG_HAS_FLOAT
	   case _SLANG_BC_LDVARIABLE_BINARY:
	       {
		  SLang_Object_Type o;
		  o.data_type = SLANG_DOUBLE_TYPE;
		  o.v.double_val = *(addr+2)->b.double_blk;

		  do_binary_ab_inc_ref (addr->b.i_blk, 
				Local_Variable_Frame - (addr+1)->b.i_blk,
				&o);
	       }
	     addr += 2;
	     break;
#endif
	   case _SLANG_BC_ILVARIABLE_BINARY:
	       {
		  SLang_Object_Type o;
		  o.data_type = SLANG_INT_TYPE;
		  o.v.int_val = (int) (addr+1)->b.l_blk;

		  do_binary_ab_inc_ref (addr->b.i_blk, 
				&o,
				Local_Variable_Frame - (addr+2)->b.i_blk);
	       }
	     addr += 2;
	     break;
#if SLANG_HAS_FLOAT
	   case _SLANG_BC_DLVARIABLE_BINARY:
	       {
		  SLang_Object_Type o;
		  o.data_type = SLANG_DOUBLE_TYPE;
		  o.v.double_val = *(addr+1)->b.double_blk;

		  do_binary_ab_inc_ref (addr->b.i_blk, 
				&o,
				Local_Variable_Frame - (addr+2)->b.i_blk);
	       }
	     addr += 2;
	     break;
#endif
	   case _SLANG_BC_LVARIABLE_BINARY:
	     do_binary_b_inc_ref (addr->b.i_blk, 
			  Local_Variable_Frame - (addr+1)->b.i_blk);
	     addr++;
	     break;

	   case _SLANG_BC_GVARIABLE_BINARY:
	     do_binary_b_inc_ref (addr->b.i_blk,
			  &(addr+1)->b.nt_gvar_blk->obj);
	     addr++;
	     break;

	   case _SLANG_BC_LITERAL_INT_BINARY:
	       {
		  SLang_Object_Type o;
		  o.data_type = SLANG_INT_TYPE;
		  o.v.int_val = (int) (addr+1)->b.l_blk;
		  do_binary_b (addr->b.i_blk, &o);
	       }
	     addr++;
	     break;
#if SLANG_HAS_FLOAT
	   case _SLANG_BC_LITERAL_DBL_BINARY:
	       {
		  SLang_Object_Type o;
		  o.data_type = SLANG_DOUBLE_TYPE;
		  o.v.double_val = *(addr+1)->b.double_blk;
		  do_binary_b (addr->b.i_blk, &o);
	       }
	     addr++;
	     break;
#endif
#endif				       /* USE_COMBINED_BYTECODES */
	   default:
	     SLang_verror (SL_INTERNAL_ERROR, "Byte-Code 0x%X is not valid", addr->bc_main_type);
	  }

	/* Someday I plan to add a 'signal' intrinsic function.  Then when a
	 * signal is caught, a variable will be set to one and that value of
	 * that variable will need to be monitored here, e.g.,
	 * if (Handle_Signal) handle_signal ();
	 * It would be nice to check only one variable instead of Handle_Signal
	 * and SLang_Error.  Perhaps I should phase out SLang_Error = xxx
	 * and used something like: SLang_set_error (code);  Then, I could
	 * use:
	 * if (Handle_Condition)
	 *   {
	 *      Handle_Condition = 0;
	 *      if (SLang_Error) ....
	 *      else if (Handle_Signal) handle_signal ();
	 *      else....
	 *   }
	 */
	if (SLang_Error)
	  {
	     if (-1 == do_inner_interp_error (err_block, addr_start, addr))
	       return 1;
	     if (SLang_Error)
	       return 1;

	     /* Otherwise, error cleared.  Continue onto next bytecode.
	      * Someday I need to add something to indicate where the
	      * next statement begins since continuing on the next
	      * bytecode is not really what is desired.
	      */
	     if (Lang_Break_Condition) goto handle_break_condition;
	  }
	addr++;
     }

   handle_break_condition:
   /* Get here if Lang_Break_Condition != 0, which implies that either
    * Lang_Return, Lang_Break, or Lang_Continue is non zero
    */
   if (Lang_Return)
     Lang_Break = 1;

   return 1;
}

/*}}}*/

/* The functions below this point are used to implement the parsed token
 * to byte-compiled code.
 */
/* static SLang_Name_Type **Static_Hash_Table; */

static SLang_Name_Type **Locals_Hash_Table;
static int Local_Variable_Number;
static unsigned int Function_Args_Number;
int _SLang_Auto_Declare_Globals = 0;
int (*SLang_Auto_Declare_Var_Hook) (char *);

static SLang_NameSpace_Type *This_Static_NameSpace;
static SLang_NameSpace_Type *Global_NameSpace;

#if _SLANG_HAS_DEBUG_CODE
static char *This_Compile_Filename;
#endif

static int Lang_Defining_Function;
static void (*Default_Variable_Mode) (_SLang_Token_Type *);
static void (*Default_Define_Function) (char *, unsigned long);
static int setup_default_compile_linkage (int);

static int push_compile_context (char *);
static int pop_compile_context (void);

typedef struct
{
   int block_type;
   SLBlock_Type *block;		       /* beginning of block definition */
   SLBlock_Type *block_ptr;	       /* current location */
   SLBlock_Type *block_max;	       /* end of definition */
   SLang_NameSpace_Type *static_namespace;
}
Block_Context_Type;

static Block_Context_Type Block_Context_Stack [SLANG_MAX_BLOCK_STACK_LEN];
static unsigned int Block_Context_Stack_Len;

static SLBlock_Type *Compile_ByteCode_Ptr;
static SLBlock_Type *This_Compile_Block;
static SLBlock_Type *This_Compile_Block_Max;
static int This_Compile_Block_Type;
#define COMPILE_BLOCK_TYPE_FUNCTION	1
#define COMPILE_BLOCK_TYPE_BLOCK	2
#define COMPILE_BLOCK_TYPE_TOP_LEVEL	3

/* If it returns 0, DO NOT FREE p */
static int lang_free_branch (SLBlock_Type *p)
{
   /* Note: we look at 0,2,4, since these blocks are 0 terminated */
   if ((p == SLShort_Blocks)
       || (p == SLShort_Blocks + 2)
       || (p == SLShort_Blocks + 4)
       )
     return 0;

   while (1)
     {
	SLang_Class_Type *cl;

        switch (p->bc_main_type)
	  {
	   case _SLANG_BC_BLOCK:
	     if (lang_free_branch(p->b.blk))
	       SLfree((char *)p->b.blk);
	     break;

	   case _SLANG_BC_LITERAL:
	   case _SLANG_BC_LITERAL_STR:
	   case _SLANG_BC_LITERAL_DBL:
	   case _SLANG_BC_LITERAL_COMBINED:
	     /* No user types should be here. */
	     cl = _SLclass_get_class (p->bc_sub_type);
	     (*cl->cl_byte_code_destroy) (p->bc_sub_type, (VOID_STAR) &p->b.ptr_blk);
	     break;

	   case _SLANG_BC_FIELD:
	   case _SLANG_BC_SET_STRUCT_LVALUE:
	     SLang_free_slstring (p->b.s_blk);
	     break;

	   default:
	     break;

	   case 0:
	     return 1;
	  }
	p++;
     }
}

static void free_function_header (_SLBlock_Header_Type *h)
{
   if (h->num_refs > 1)
     {
	h->num_refs--;
	return;
     }

   if (h->body != NULL)
     {
	if (lang_free_branch (h->body))
	  SLfree ((char *) h->body);
     }

   SLfree ((char *) h);
}

static int push_block_context (int type)
{
   Block_Context_Type *c;
   unsigned int num;
   SLBlock_Type *b;

   if (Block_Context_Stack_Len == SLANG_MAX_BLOCK_STACK_LEN)
     {
	SLang_verror (SL_STACK_OVERFLOW, "Block stack overflow");
	return -1;
     }

   num = 5;    /* 40 bytes */
   if (NULL == (b = (SLBlock_Type *) SLcalloc (num, sizeof (SLBlock_Type))))
     return -1;

   c = Block_Context_Stack + Block_Context_Stack_Len;
   c->block = This_Compile_Block;
   c->block_ptr = Compile_ByteCode_Ptr;
   c->block_max = This_Compile_Block_Max;
   c->block_type = This_Compile_Block_Type;
   c->static_namespace = This_Static_NameSpace;

   Compile_ByteCode_Ptr = This_Compile_Block = b;
   This_Compile_Block_Max = b + num;
   This_Compile_Block_Type = type;

   Block_Context_Stack_Len += 1;
   return 0;
}

static int pop_block_context (void)
{
   Block_Context_Type *c;

   if (Block_Context_Stack_Len == 0)
     return -1;

   Block_Context_Stack_Len -= 1;
   c = Block_Context_Stack + Block_Context_Stack_Len;

   This_Compile_Block = c->block;
   This_Compile_Block_Max = c->block_max;
   This_Compile_Block_Type = c->block_type;
   Compile_ByteCode_Ptr = c->block_ptr;
   This_Static_NameSpace = c->static_namespace;

   return 0;
}

static int implements_ns (char *name)
{
   SLang_NameSpace_Type *ns;

   ns = This_Static_NameSpace;
   if (ns == NULL)
     {
	/* This error should never happen */
	SLang_verror (SL_INTRINSIC_ERROR, "No namespace available");
	return -1;
     }

   if (ns->namespace_name != NULL)
     {
	/* create a new namespace and use it */
	ns = _SLns_allocate_namespace (NULL, SLSTATIC_HASH_TABLE_SIZE);
	if (ns == NULL)
	  return -1;
     }
   
   if (-1 == _SLns_set_namespace_name (ns, name))
     return -1;

   This_Static_NameSpace = ns;
   (void) setup_default_compile_linkage (0);
   return 0;
}

int _SLcompile_push_context (SLang_Load_Type *load_object)
{
   char *ns_name;
   SLang_NameSpace_Type *ns;

   if (-1 == push_compile_context (load_object->name))
     return -1;
   
   ns_name = load_object->namespace_name;
   
   if ((ns_name != NULL)
       && (NULL != (ns = _SLns_find_namespace (ns_name))))
     {
	This_Static_NameSpace = ns;
	setup_default_compile_linkage (ns == Global_NameSpace);
     }
   else
     {
	ns = _SLns_allocate_namespace (load_object->name, SLSTATIC_HASH_TABLE_SIZE);
	if (ns == NULL)
	  {
	     pop_compile_context ();
	     return -1;
	  }
	
	This_Static_NameSpace = ns;
	if ((ns_name != NULL)
	    && (-1 == implements_ns (ns_name)))
	  {
	     pop_compile_context ();
	     return -1;
	  }
     }

   if (-1 == push_block_context (COMPILE_BLOCK_TYPE_TOP_LEVEL))
     {
	pop_compile_context ();
	return -1;
     }

   return 0;
}

int _SLcompile_pop_context (void)
{
   if (This_Compile_Block_Type == COMPILE_BLOCK_TYPE_TOP_LEVEL)
     {
	Compile_ByteCode_Ptr->bc_main_type = 0;
	if (lang_free_branch (This_Compile_Block))
	  SLfree ((char *) This_Compile_Block);
     }

   (void) pop_block_context ();
   (void) pop_compile_context ();

   if (This_Compile_Block == NULL)
     return 0;

#if 0
   if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_TOP_LEVEL)
     {
	SLang_verror (SL_INTERNAL_ERROR, "Not at top-level");
	return -1;
     }
#endif

   return 0;
}

/*{{{ Hash and Name Table Functions */

static SLang_Name_Type *locate_name_in_table (char *name, unsigned long hash,
					      SLang_Name_Type **table, unsigned int table_size)
{
   SLang_Name_Type *t;
   char ch;

   t = table [(unsigned int) (hash % table_size)];
   ch = *name++;

   while (t != NULL)
     {
	if ((ch == t->name[0])
	    && (0 == strcmp (t->name + 1, name)))
	  break;

	t = t->next;
     }

   return t;
}

static SLang_Name_Type *locate_namespace_encoded_name (char *name, int err_on_bad_ns)
{
   char *ns, *ns1;
   SLang_NameSpace_Type *table;
   SLang_Name_Type *nt;

   ns = name;
   name = strchr (name, '-');
   if ((name == NULL) || (name [1] != '>'))
     name = ns;

   ns1 = SLang_create_nslstring (ns, (unsigned int) (name - ns));
   if (ns1 == NULL)
     return NULL;
   if (ns != name)
     name += 2;
   ns = ns1;

   if (*ns == 0)
     {
	/* Use Global Namespace */
	SLang_free_slstring (ns);
	return locate_name_in_table (name, _SLcompute_string_hash (name),
				     Global_NameSpace->table, Global_NameSpace->table_size);
     }

   if (NULL == (table = _SLns_find_namespace (ns)))
     {
	if (err_on_bad_ns)
	  SLang_verror (SL_SYNTAX_ERROR, "Unable to find namespace called %s", ns);
	SLang_free_slstring (ns);
	return NULL;
     }
   SLang_free_slstring (ns);

   /* FIXME: the hash table size should be stored in the hash table itself */
   nt = locate_name_in_table (name, _SLcompute_string_hash (name),
			      table->table, table->table_size);
   if (nt == NULL)
     return NULL;

   switch (nt->name_type)
     {
	/* These are private and cannot be accessed through the namespace. */
      case SLANG_PVARIABLE:
      case SLANG_PFUNCTION:
	return NULL;
     }
   return nt;
}

static SLang_Name_Type *locate_hashed_name (char *name, unsigned long hash)
{
   SLang_Name_Type *t;

   if (Lang_Defining_Function)
     {
	t = locate_name_in_table (name, hash, Locals_Hash_Table, SLLOCALS_HASH_TABLE_SIZE);
	if (t != NULL)
	  return t;
     }

   if ((This_Static_NameSpace != NULL)
       && (NULL != (t = locate_name_in_table (name, hash, This_Static_NameSpace->table, This_Static_NameSpace->table_size))))
     return t;

   t = locate_name_in_table (name, hash, Global_NameSpace->table, Global_NameSpace->table_size);
   if (NULL != t)
     return t;

   return locate_namespace_encoded_name (name, 1);
}

SLang_Name_Type *_SLlocate_name (char *name)
{
   return locate_hashed_name (name, _SLcompute_string_hash (name));
}

static SLang_Name_Type *
add_name_to_hash_table (char *name, unsigned long hash,
			unsigned int sizeof_obj, unsigned char name_type,
			SLang_Name_Type **table, unsigned int table_size,
			int check_existing)
{
   SLang_Name_Type *t;

   if (check_existing)
     {
	t = locate_name_in_table (name, hash, table, table_size);
	if (t != NULL)
	  return t;
     }

   if (-1 == _SLcheck_identifier_syntax (name))
     return NULL;

   t = (SLang_Name_Type *) SLmalloc (sizeof_obj);
   if (t == NULL)
     return t;

   memset ((char *) t, 0, sizeof_obj);
   if (NULL == (t->name = _SLstring_dup_hashed_string (name, hash)))
     {
	SLfree ((char *) t);
	return NULL;
     }
   t->name_type = name_type;

   hash = hash % table_size;
   t->next = table [(unsigned int)hash];
   table [(unsigned int) hash] = t;

   return t;
}

static SLang_Name_Type *
add_global_name (char *name, unsigned long hash,
		 unsigned char name_type, unsigned int sizeof_obj,
		 SLang_NameSpace_Type *ns)
{
   SLang_Name_Type *nt;
   SLang_Name_Type **table;
   unsigned int table_size;
   
   table = ns->table;
   table_size = ns->table_size;

   nt = locate_name_in_table (name, hash, table, table_size);
   if (nt != NULL)
     {
	if (nt->name_type == name_type)
	  return nt;

	SLang_verror (SL_DUPLICATE_DEFINITION, "%s cannot be re-defined", name);
	return NULL;
     }

   return add_name_to_hash_table (name, hash, sizeof_obj, name_type,
				  table, table_size, 0);
}

static int add_intrinsic_function (SLang_NameSpace_Type *ns,
				   char *name, FVOID_STAR addr, unsigned char ret_type,
				   unsigned int nargs, va_list ap)
{
   SLang_Intrin_Fun_Type *f;
   unsigned int i;

   if (-1 == init_interpreter ())
     return -1;
   
   if (ns == NULL) ns = Global_NameSpace;

   if (nargs > SLANG_MAX_INTRIN_ARGS)
     {
	SLang_verror (SL_APPLICATION_ERROR, "Function %s requires too many arguments", name);
	return -1;
     }

   if (ret_type == SLANG_FLOAT_TYPE)
     {
	SLang_verror (SL_NOT_IMPLEMENTED, "Function %s is not permitted to return float", name);
	return -1;
     }

   f = (SLang_Intrin_Fun_Type *) add_global_name (name, _SLcompute_string_hash (name),
						  SLANG_INTRINSIC, sizeof (SLang_Intrin_Fun_Type),
						  ns);

   if (f == NULL)
     return -1;

   f->i_fun = addr;
   f->num_args = nargs;
   f->return_type = ret_type;

   for (i = 0; i < nargs; i++)
     f->arg_types [i] = va_arg (ap, unsigned int);

   return 0;
}

int SLadd_intrinsic_function (char *name, FVOID_STAR addr, unsigned char ret_type,
			      unsigned int nargs, ...)
{
   va_list ap;
   int status;

   va_start (ap, nargs);
   status = add_intrinsic_function (NULL, name, addr, ret_type, nargs, ap);
   va_end (ap);

   return status;
}

int SLns_add_intrinsic_function (SLang_NameSpace_Type *ns, 
				 char *name, FVOID_STAR addr, unsigned char ret_type,
				 unsigned int nargs, ...)
{
   va_list ap;
   int status;

   va_start (ap, nargs);
   status = add_intrinsic_function (ns, name, addr, ret_type, nargs, ap);
   va_end (ap);

   return status;
}

int SLns_add_intrinsic_variable (SLang_NameSpace_Type *ns,
				   char *name, VOID_STAR addr, unsigned char data_type, int ro)
{
   SLang_Intrin_Var_Type *v;

   if (-1 == init_interpreter ())
     return -1;
   
   if (ns == NULL) ns = Global_NameSpace;

   v = (SLang_Intrin_Var_Type *)add_global_name (name,
						 _SLcompute_string_hash (name),
						 (ro ? SLANG_RVARIABLE : SLANG_IVARIABLE),
						 sizeof (SLang_Intrin_Var_Type),
						 ns);
   if (v == NULL)
     return -1;

   v->addr = addr;
   v->type = data_type;
   return 0;
}
  
int SLadd_intrinsic_variable (char *name, VOID_STAR addr, unsigned char data_type, int ro)
{
   return SLns_add_intrinsic_variable (NULL, name, addr, data_type, ro);
}

static int
add_slang_function (char *name, unsigned char type, unsigned long hash,
		    unsigned int num_args, unsigned int num_locals,
#if _SLANG_HAS_DEBUG_CODE
		    char *file,
#endif
		    _SLBlock_Header_Type *h, 
		    SLang_NameSpace_Type *ns)
{
   _SLang_Function_Type *f;
   
#if _SLANG_HAS_DEBUG_CODE
   if ((file != NULL)
       && (NULL == (file = SLang_create_slstring (file))))
     return -1;
#endif

   f = (_SLang_Function_Type *)add_global_name (name, hash,
						type,
						sizeof (_SLang_Function_Type),
						ns);
   if (f == NULL)
     {
#if _SLANG_HAS_DEBUG_CODE
	SLang_free_slstring (file);    /* NULL ok */
#endif
	return -1;
     }

   if (f->v.header != NULL)
     {
	if (f->nlocals == AUTOLOAD_NUM_LOCALS)
	  SLang_free_slstring ((char *)f->v.autoload_filename); /* autoloaded filename */
	else
	  free_function_header (f->v.header);
     }

#if _SLANG_HAS_DEBUG_CODE
   if (f->file != NULL) SLang_free_slstring (f->file);
   f->file = file;
#endif
   f->v.header = h;
   f->nlocals = num_locals;
   f->nargs = num_args;

   return 0;
}

int SLang_autoload (char *name, char *file)
{
   _SLang_Function_Type *f;
   unsigned long hash;

   hash = _SLcompute_string_hash (name);
   f = (_SLang_Function_Type *)locate_name_in_table (name, hash, Global_NameSpace->table, Global_NameSpace->table_size);

   if ((f != NULL)
       && (f->name_type == SLANG_FUNCTION)
       && (f->v.header != NULL)
       && (f->nlocals != AUTOLOAD_NUM_LOCALS))
     {
	/* already loaded */
	return 0;
     }

   file = SLang_create_slstring (file);
   if (-1 == add_slang_function (name, SLANG_FUNCTION, hash, 0, AUTOLOAD_NUM_LOCALS,
#if _SLANG_HAS_DEBUG_CODE
				 file,
#endif
				 (_SLBlock_Header_Type *) file,
				 Global_NameSpace))
     {
	SLang_free_slstring (file);
	return -1;
     }

   return 0;
}

SLang_Name_Type *_SLlocate_global_name (char *name)
{
   unsigned long hash;

   hash = _SLcompute_string_hash (name);
   return locate_name_in_table (name, hash, Global_NameSpace->table, 
				Global_NameSpace->table_size);
}

/*}}}*/

static void free_local_variable_table (void)
{
   unsigned int i;
   SLang_Name_Type *t, *t1;

   for (i = 0; i < SLLOCALS_HASH_TABLE_SIZE; i++)
     {
	t = Locals_Hash_Table [i];
	while (t != NULL)
	  {
	     SLang_free_slstring (t->name);
	     t1 = t->next;
	     SLfree ((char *) t);
	     t = t1;
	  }
	Locals_Hash_Table [i] = NULL;
     }
   Local_Variable_Number = 0;
}

/* call inner interpreter or return for more */
static void lang_try_now(void)
{
   Compile_ByteCode_Ptr++;
   if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_TOP_LEVEL)
     return;

   Compile_ByteCode_Ptr->bc_main_type = 0;  /* so next command stops after this */

   /* now do it */
   inner_interp (This_Compile_Block);
   (void) lang_free_branch (This_Compile_Block);
   Compile_ByteCode_Ptr = This_Compile_Block;
}

SLang_Name_Type *SLang_get_fun_from_ref (SLang_Ref_Type *ref)
{
   if (ref->is_global)
     {
	SLang_Name_Type *nt = ref->v.nt;

	switch (nt->name_type)
	  {
	   case SLANG_PFUNCTION:
	   case SLANG_FUNCTION:
	   case SLANG_INTRINSIC:
	   case SLANG_MATH_UNARY:
	   case SLANG_APP_UNARY:
	     return nt;
	  }
	SLang_verror (SL_TYPE_MISMATCH,
		      "Reference to a function expected.  Found &%s", 
		      nt->name);
     }

   SLang_verror (SL_TYPE_MISMATCH,
		 "Reference to a function expected");
   return NULL;
}

int SLexecute_function (SLang_Name_Type *nt)
{
   unsigned char type;
   char *name;

   if (SLang_Error)
     return -1;

   type = nt->name_type;
   name = nt->name;

   switch (type)
     {
      case SLANG_PFUNCTION:
      case SLANG_FUNCTION:
	execute_slang_fun ((_SLang_Function_Type *) nt);
	break;

      case SLANG_INTRINSIC:
	execute_intrinsic_fun ((SLang_Intrin_Fun_Type *) nt);
	break;

      case SLANG_MATH_UNARY:
      case SLANG_APP_UNARY:
	inner_interp_nametype (nt);
	break;

      default:
	SLang_verror (SL_TYPE_MISMATCH, "%s is not a function", name);
	return -1;
     }

   if (SLang_Error)
     {
	SLang_verror (SLang_Error, "Error while executing %s", name);
	return -1;
     }

   return 1;
}

int SLang_execute_function (char *name)
{
   SLang_Name_Type *entry;

   if (NULL == (entry = SLang_get_function (name)))
     return 0;

   return SLexecute_function (entry);
}

/* return S-Lang function or NULL */
SLang_Name_Type *SLang_get_function (char *name)
{
   SLang_Name_Type *entry;

   if (NULL == (entry = locate_namespace_encoded_name (name, 0)))
     return NULL;

   if ((entry->name_type == SLANG_FUNCTION)
       || (entry->name_type == SLANG_INTRINSIC))
     return entry;

   return NULL;
}

static void lang_begin_function (void)
{
   if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_TOP_LEVEL)
     {
	SLang_verror (SL_SYNTAX_ERROR, "Function nesting is illegal");
	return;
     }
   Lang_Defining_Function = 1;
   (void) push_block_context (COMPILE_BLOCK_TYPE_FUNCTION);
}

#if USE_COMBINED_BYTECODES
static void rearrange_optimized_binary (SLBlock_Type *b, unsigned char t1, unsigned char t2, unsigned char t3)
{
   SLBlock_Type tmp;

   b->bc_main_type = t1;
   (b-1)->bc_main_type = t3;
   (b-2)->bc_main_type = t2;

   tmp = *b;
   *b = *(b-1);
   *(b-1) = *(b-2);
   *(b-2) = tmp;
}

static void rearrange_optimized_unary (SLBlock_Type *b, unsigned char t1, unsigned char t2)
{
   SLBlock_Type tmp;

   b->bc_main_type = t1;
   (b-1)->bc_main_type = t2;

   tmp = *b;
   *b = *(b-1);
   *(b-1) = tmp;
}

static void optimize_block (SLBlock_Type *b)
{
   SLBlock_Type *bstart, *b1, *b2;
   SLtype b2_main_type;


   bstart = b;
   while (1)
     {
	switch (b->bc_main_type)
	  {
	   case 0:
	     return;
	     
	   default:
	     b++;
	     break;

	   case _SLANG_BC_BINARY:
	     if (bstart + 2 > b)
	       {
		  b++;
		  break;
	       }
	     b2 = b-1;
	     b1 = b2-1;
	     b2_main_type = b2->bc_main_type;

	     switch (b1->bc_main_type)
	       {
		case SLANG_LVARIABLE:
		  if (b2_main_type == SLANG_LVARIABLE)
		    rearrange_optimized_binary (b,
						_SLANG_BC_LLVARIABLE_BINARY, 
						_SLANG_BC_LVARIABLE_COMBINED,
						_SLANG_BC_LVARIABLE_COMBINED);
		  else if (b2_main_type == SLANG_GVARIABLE)
		    rearrange_optimized_binary (b, 
						_SLANG_BC_LGVARIABLE_BINARY,
						_SLANG_BC_LVARIABLE_COMBINED,
						_SLANG_BC_GVARIABLE_COMBINED);
		  else if (b2_main_type == _SLANG_BC_LITERAL_INT)
		    rearrange_optimized_binary (b,
						_SLANG_BC_LIVARIABLE_BINARY,
						_SLANG_BC_LVARIABLE_COMBINED,
						_SLANG_BC_LITERAL_COMBINED);
		  else if (b2_main_type == _SLANG_BC_LITERAL_DBL)
		    rearrange_optimized_binary (b,
						_SLANG_BC_LDVARIABLE_BINARY,
						_SLANG_BC_LVARIABLE_COMBINED,
						_SLANG_BC_LITERAL_COMBINED);
		  break;

		case SLANG_GVARIABLE:
		  if (b2_main_type == SLANG_LVARIABLE)
		    rearrange_optimized_binary (b,
						_SLANG_BC_GLVARIABLE_BINARY, 
						_SLANG_BC_GVARIABLE_COMBINED,
						_SLANG_BC_LVARIABLE_COMBINED);
		  else if (b2_main_type == SLANG_GVARIABLE)
		    rearrange_optimized_binary (b, 
						_SLANG_BC_GGVARIABLE_BINARY,
						_SLANG_BC_GVARIABLE_COMBINED,
						_SLANG_BC_GVARIABLE_COMBINED);
		  break;
		  
		case _SLANG_BC_LITERAL_INT:
		  if (b2_main_type == SLANG_LVARIABLE)
		    rearrange_optimized_binary (b,
						_SLANG_BC_ILVARIABLE_BINARY, 
						_SLANG_BC_LITERAL_COMBINED,
						_SLANG_BC_LVARIABLE_COMBINED);
		  break;
		  
		case _SLANG_BC_LITERAL_DBL:
		  if (b2_main_type == SLANG_LVARIABLE)
		    rearrange_optimized_binary (b,
						_SLANG_BC_DLVARIABLE_BINARY, 
						_SLANG_BC_LITERAL_COMBINED,
						_SLANG_BC_LVARIABLE_COMBINED);
		  break;
		  
		default:
		  if (b2_main_type == SLANG_LVARIABLE)
		    rearrange_optimized_unary (b,
					       _SLANG_BC_LVARIABLE_BINARY,
					       _SLANG_BC_LVARIABLE_COMBINED);
		  else if (b2_main_type == SLANG_GVARIABLE)
		    rearrange_optimized_unary (b, 
					       _SLANG_BC_GVARIABLE_BINARY,
					       _SLANG_BC_GVARIABLE_COMBINED);
		  else if (b2_main_type == _SLANG_BC_LITERAL_INT)
		    rearrange_optimized_unary (b,
					       _SLANG_BC_LITERAL_INT_BINARY,
					       _SLANG_BC_LITERAL_COMBINED);
		  else if (b2_main_type == _SLANG_BC_LITERAL_DBL)
		    rearrange_optimized_unary (b,
					       _SLANG_BC_LITERAL_DBL_BINARY,
					       _SLANG_BC_LITERAL_COMBINED);
	       }
	     b++;
	     break;

	   case _SLANG_BC_CALL_DIRECT:
	     b++;
	     switch (b->bc_main_type)
	       {
		case 0:
		  return;
		case _SLANG_BC_INTRINSIC:
		  if ((b+1)->bc_main_type == 0)
		    {
		       (b-1)->bc_main_type = _SLANG_BC_CALL_DIRECT_INTRSTOP;
		       return;
		    }
		  (b-1)->bc_main_type = _SLANG_BC_CALL_DIRECT_INTRINSIC;
		  b++;
		  break;
		case _SLANG_BC_LITERAL_STR:
		  (b-1)->bc_main_type = _SLANG_BC_CALL_DIRECT_LSTR;
		  b++;
		  break;
		case _SLANG_BC_FUNCTION:
		case _SLANG_BC_PFUNCTION:
		  (b-1)->bc_main_type = _SLANG_BC_CALL_DIRECT_SLFUN;
		  b++;
		  break;
		case _SLANG_BC_EARG_LVARIABLE:
		  (b-1)->bc_main_type = _SLANG_BC_CALL_DIRECT_EARG_LVAR;
		  b++;
		  break;
		case _SLANG_BC_LITERAL_INT:
		  b->bc_main_type = _SLANG_BC_LITERAL_COMBINED;
		  (b-1)->bc_main_type = _SLANG_BC_CALL_DIRECT_LINT;
		  b++;
		  break;
		case _SLANG_BC_LVARIABLE:
		  b->bc_main_type = _SLANG_BC_LVARIABLE_COMBINED;
		  (b-1)->bc_main_type = _SLANG_BC_CALL_DIRECT_LVAR;
		  b++;
		  break;
	       }
	     break;
	     
	   case _SLANG_BC_INTRINSIC:
	     b++;
	     switch (b->bc_main_type)
	       {
		case _SLANG_BC_CALL_DIRECT:
		  (b-1)->bc_main_type = _SLANG_BC_INTRINSIC_CALL_DIRECT;
		  b++;
		  break;
#if 0
		case _SLANG_BC_BLOCK:
		  (b-1)->bc_main_type = _SLANG_BC_INTRINSIC_BLOCK;
		  b++;
		  break;
#endif

		case 0:
		  (b-1)->bc_main_type = _SLANG_BC_INTRINSIC_STOP;
		  return;
	       }	     
	     break;
	  }
     }
}

#endif


/* name will be NULL if the object is to simply terminate the function
 * definition.  See SLang_restart.
 */
static int lang_define_function (char *name, unsigned char type, unsigned long hash,
				 SLang_NameSpace_Type *ns)
{
   if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_FUNCTION)
     {
	SLang_verror (SL_SYNTAX_ERROR, "Premature end of function");
	return -1;
     }

   /* terminate function */
   Compile_ByteCode_Ptr->bc_main_type = 0;

   if (name != NULL)
     {
	_SLBlock_Header_Type *h;

	h = (_SLBlock_Header_Type *)SLmalloc (sizeof (_SLBlock_Header_Type));
	if (h != NULL)
	  {
	     h->num_refs = 1;
	     h->body = This_Compile_Block;
	     
#if USE_COMBINED_BYTECODES
	     optimize_block (h->body);
#endif

	     if (-1 == add_slang_function (name, type, hash,
					   Function_Args_Number,
					   Local_Variable_Number,
#if _SLANG_HAS_DEBUG_CODE
					   This_Compile_Filename,
#endif
					   h, ns))
	       SLfree ((char *) h);
	  }
	/* Drop through for clean-up */
     }

   free_local_variable_table ();

   Function_Args_Number = 0;
   Lang_Defining_Function = 0;

   if (SLang_Error) return -1;
   /* SLang_restart will finish this if there is a slang error. */

   pop_block_context ();

   /* A function is only defined at top-level */
   if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_TOP_LEVEL)
     {
	SLang_verror (SL_INTERNAL_ERROR, "Not at top-level");
	return -1;
     }
   Compile_ByteCode_Ptr = This_Compile_Block;
   return 0;
}

static void define_static_function (char *name, unsigned long hash)
{
   (void) lang_define_function (name, SLANG_FUNCTION, hash, This_Static_NameSpace);
}

static void define_private_function (char *name, unsigned long hash)
{
   (void) lang_define_function (name, SLANG_PFUNCTION, hash, This_Static_NameSpace);
}

static void define_public_function (char *name, unsigned long hash)
{
   (void) lang_define_function (name, SLANG_FUNCTION, hash, Global_NameSpace);
}

static void lang_end_block (void)
{
   SLBlock_Type *node, *branch;
   unsigned char mtype;

   if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_BLOCK)
     {
	SLang_verror (SL_SYNTAX_ERROR, "Not defining a block");
	return;
     }

   /* terminate the block */
   Compile_ByteCode_Ptr->bc_main_type = 0;
   branch = This_Compile_Block;

   /* Try to save some space by using the cached blocks. */
   if (Compile_ByteCode_Ptr == branch + 1)
     {
	mtype = branch->bc_main_type;
	if (((mtype == _SLANG_BC_BREAK)
	     || (mtype == _SLANG_BC_CONTINUE)
	     || (mtype == _SLANG_BC_RETURN))
	    && (SLang_Error == 0))
	  {
	     SLfree ((char *)branch);
	     branch = SLShort_Blocks + 2 * (int) (mtype - _SLANG_BC_RETURN);
	  }
     }

#if USE_COMBINED_BYTECODES
   optimize_block (branch);
#endif

   pop_block_context ();
   node = Compile_ByteCode_Ptr++;

   node->bc_main_type = _SLANG_BC_BLOCK;
   node->bc_sub_type = 0;
   node->b.blk = branch;
}

static int lang_begin_block (void)
{
   return push_block_context (COMPILE_BLOCK_TYPE_BLOCK);
}

static int lang_check_space (void)
{
   unsigned int n;
   SLBlock_Type *p;

   if (NULL == (p = This_Compile_Block))
     {
	SLang_verror (SL_INTERNAL_ERROR, "Top-level block not present");
	return -1;
     }

   /* Allow 1 extra for terminator */
   if (Compile_ByteCode_Ptr + 1 < This_Compile_Block_Max)
     return 0;

   n = (unsigned int) (This_Compile_Block_Max - p);

   /* enlarge the space by 2 objects */
   n += 2;

   if (NULL == (p = (SLBlock_Type *) SLrealloc((char *)p, n * sizeof(SLBlock_Type))))
     return -1;

   This_Compile_Block_Max = p + n;
   n = (unsigned int) (Compile_ByteCode_Ptr - This_Compile_Block);
   This_Compile_Block = p;
   Compile_ByteCode_Ptr = p + n;

   return 0;
}

/* returns positive number if name is a function or negative number if it
 is a variable.  If it is intrinsic, it returns magnitude of 1, else 2 */
int SLang_is_defined(char *name)
{
   SLang_Name_Type *t;

   if (-1 == init_interpreter ())
     return -1;

   t = locate_namespace_encoded_name (name, 0);
   if (t == NULL)
     return 0;

   switch (t->name_type)
     {
      case SLANG_FUNCTION:
      /* case SLANG_PFUNCTION: */
	return 2;
      case SLANG_GVARIABLE:
      /* case SLANG_PVARIABLE: */
	return -2;

      case SLANG_ICONSTANT:
      case SLANG_DCONSTANT:
      case SLANG_RVARIABLE:
      case SLANG_IVARIABLE:
	return -1;

      case SLANG_INTRINSIC:
      default:
	return 1;
     }
}

static int add_global_variable (char *name, char name_type, unsigned long hash,
				SLang_NameSpace_Type *ns)
{
   SLang_Name_Type *g;

   /* Note the importance of checking if it is already defined or not.  For example,
    * suppose X is defined as an intrinsic variable.  Then S-Lang code like:
    * !if (is_defined("X")) { variable X; }
    * will not result in a global variable X.  On the other hand, this would
    * not be an issue if 'variable' statements always were not processed
    * immediately.  That is, as it is now, 'if (0) {variable ZZZZ;}' will result
    * in the variable ZZZZ being defined because of the immediate processing.
    * The current solution is to do: if (0) { eval("variable ZZZZ;"); }
    */
   /* hash = _SLcompute_string_hash (name); */
   g = locate_name_in_table (name, hash, ns->table, ns->table_size);

   if (g != NULL)
     {
	if (g->name_type == name_type)
	  return 0;
     }

   if (NULL == add_global_name (name, hash, name_type,
				sizeof (SLang_Global_Var_Type), ns))
     return -1;

   return 0;
}

int SLadd_global_variable (char *name)
{
   if (-1 == init_interpreter ())
     return -1;

   return add_global_variable (name, SLANG_GVARIABLE,
			       _SLcompute_string_hash (name),
			       Global_NameSpace);
}

static int add_local_variable (char *name, unsigned long hash)
{
   SLang_Local_Var_Type *t;

   /* local variable */
   if (Local_Variable_Number >= SLANG_MAX_LOCAL_VARIABLES)
     {
	SLang_verror (SL_SYNTAX_ERROR, "Too many local variables");
	return -1;
     }

   if (NULL != locate_name_in_table (name, hash, Locals_Hash_Table, SLLOCALS_HASH_TABLE_SIZE))
     {
	SLang_verror (SL_SYNTAX_ERROR, "Local variable %s has already been defined", name);
	return -1;
     }

   t = (SLang_Local_Var_Type *)
     add_name_to_hash_table (name, hash,
			     sizeof (SLang_Local_Var_Type), SLANG_LVARIABLE,
			     Locals_Hash_Table, SLLOCALS_HASH_TABLE_SIZE, 0);
   if (t == NULL)
     return -1;

   t->local_var_number = Local_Variable_Number;
   Local_Variable_Number++;
   return 0;
}

static void (*Compile_Mode_Function) (_SLang_Token_Type *);
static void compile_basic_token_mode (_SLang_Token_Type *);

/* if an error occurs, discard current object, block, function, etc... */
void SLang_restart (int localv)
{
   int save = SLang_Error;

   SLang_Error = SL_UNKNOWN_ERROR;

   _SLcompile_ptr = _SLcompile;
   Compile_Mode_Function = compile_basic_token_mode;

   Lang_Break = /* Lang_Continue = */ Lang_Return = 0;
   Trace_Mode = 0;

   while (This_Compile_Block_Type == COMPILE_BLOCK_TYPE_BLOCK)
     lang_end_block();

   if (This_Compile_Block_Type == COMPILE_BLOCK_TYPE_FUNCTION)
     {
	/* Terminate function definition and free variables */
	lang_define_function (NULL, SLANG_FUNCTION, 0, Global_NameSpace);
	if (lang_free_branch (This_Compile_Block))
	  SLfree((char *)This_Compile_Block);
     }
   Lang_Defining_Function = 0;

   SLang_Error = save;

   if (SLang_Error == SL_STACK_OVERFLOW)
     {
	/* This loop guarantees that the stack is properly cleaned. */
	while (_SLStack_Pointer != _SLRun_Stack)
	  {
	     SLdo_pop ();
	  }
     }

   while ((This_Compile_Block_Type != COMPILE_BLOCK_TYPE_TOP_LEVEL)
	  && (0 == pop_block_context ()))
     ;

   if (localv)
     {
	Next_Function_Num_Args = SLang_Num_Function_Args = 0;
	Local_Variable_Frame = Local_Variable_Stack;
	Recursion_Depth = 0;
	Frame_Pointer = _SLStack_Pointer;
	Frame_Pointer_Depth = 0;
	Switch_Obj_Ptr = Switch_Objects;
	while (Switch_Obj_Ptr < Switch_Obj_Max)
	  {
	     SLang_free_object (Switch_Obj_Ptr);
	     Switch_Obj_Ptr++;
	  }
	Switch_Obj_Ptr = Switch_Objects;
     }
}

static void compile_directive (unsigned char sub_type)
{
   /* This function is called only from compile_directive_mode which is
    * only possible when a block is available.
    */

   /* use BLOCK */
   Compile_ByteCode_Ptr--;
   Compile_ByteCode_Ptr->bc_sub_type = sub_type;

   lang_try_now ();
}

static void compile_unary (int op, unsigned char mt)
{
   Compile_ByteCode_Ptr->bc_main_type = mt;
   Compile_ByteCode_Ptr->b.i_blk = op;
   Compile_ByteCode_Ptr->bc_sub_type = 0;

   lang_try_now ();
}


static void compile_binary (int op)
{
   Compile_ByteCode_Ptr->bc_main_type = _SLANG_BC_BINARY;
   Compile_ByteCode_Ptr->b.i_blk = op;
   Compile_ByteCode_Ptr->bc_sub_type = 0;

   lang_try_now ();
}

#if _SLANG_OPTIMIZE_FOR_SPEED
static int try_compressed_bytecode (unsigned char last_bc, unsigned char bc)
{
   if (Compile_ByteCode_Ptr != This_Compile_Block)
     {
	SLBlock_Type *b;
	b = Compile_ByteCode_Ptr - 1;
	if (b->bc_main_type == last_bc)
	  {
	     Compile_ByteCode_Ptr = b;
	     b->bc_main_type = bc;
	     lang_try_now ();
	     return 0;
	  }
     }
   return -1;
}
#endif

static void compile_fast_binary (int op, unsigned char bc)
{
#if _SLANG_OPTIMIZE_FOR_SPEED
# if 0
   if (0 == try_compressed_bytecode (_SLANG_BC_LITERAL_INT, bc))
     return;
# endif
#else
#endif
   (void) bc;
   compile_binary (op);
}

/* This is a hack */
typedef struct _Special_NameTable_Type
{
   char *name;
   int (*fun) (struct _Special_NameTable_Type *, _SLang_Token_Type *);
   VOID_STAR blk_data;
   unsigned char main_type;
}
Special_NameTable_Type;

static int handle_special (Special_NameTable_Type *nt, _SLang_Token_Type *tok)
{
   (void) tok;
   Compile_ByteCode_Ptr->bc_main_type = nt->main_type;
   Compile_ByteCode_Ptr->b.ptr_blk = nt->blk_data;
   return 0;
}

static int handle_special_file (Special_NameTable_Type *nt, _SLang_Token_Type *tok)
{
   char *name;

   (void) nt; (void) tok;

   if (This_Static_NameSpace == NULL) name = "***Unknown***";
   else
     name = This_Static_NameSpace->name;

   name = SLang_create_slstring (name);
   if (name == NULL)
     return -1;

   Compile_ByteCode_Ptr->b.s_blk = name;
   Compile_ByteCode_Ptr->bc_main_type = _SLANG_BC_LITERAL_STR;
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_STRING_TYPE;
   return 0;
}

static int handle_special_line (Special_NameTable_Type *nt, _SLang_Token_Type *tok)
{
   (void) nt;

#if _SLANG_HAS_DEBUG_CODE
   Compile_ByteCode_Ptr->b.l_blk = (long) tok->line_number;
#endif
   Compile_ByteCode_Ptr->bc_main_type = _SLANG_BC_LITERAL;
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_UINT_TYPE;

   return 0;
}

static Special_NameTable_Type Special_Name_Table [] =
{
     {"EXECUTE_ERROR_BLOCK", handle_special, NULL, _SLANG_BC_X_ERROR},
     {"X_USER_BLOCK0", handle_special, NULL, _SLANG_BC_X_USER0},
     {"X_USER_BLOCK1", handle_special, NULL, _SLANG_BC_X_USER1},
     {"X_USER_BLOCK2", handle_special, NULL, _SLANG_BC_X_USER2},
     {"X_USER_BLOCK3", handle_special, NULL, _SLANG_BC_X_USER3},
     {"X_USER_BLOCK4", handle_special, NULL, _SLANG_BC_X_USER4},
     {"__FILE__", handle_special_file, NULL, 0},
     {"__LINE__", handle_special_line, NULL, 0},
#if 0
     {"__NAMESPACE__", handle_special_namespace, NULL, 0},
#endif
     {NULL, NULL, NULL, 0}
};

static void compile_hashed_identifier (char *name, unsigned long hash, _SLang_Token_Type *tok)
{
   SLang_Name_Type *entry;
   unsigned char name_type;

   entry = locate_hashed_name (name, hash);

   if (entry == NULL)
     {
	Special_NameTable_Type *nt = Special_Name_Table;

	while (nt->name != NULL)
	  {
	     if (strcmp (name, nt->name))
	       {
		  nt++;
		  continue;
	       }

	     if (0 == (*nt->fun)(nt, tok))
	       lang_try_now ();
	     return;
	  }

	SLang_verror (SL_UNDEFINED_NAME, "%s is undefined", name);
	return;
     }

   name_type = entry->name_type;
   Compile_ByteCode_Ptr->bc_main_type = name_type;

   if (name_type == SLANG_LVARIABLE)   /* == _SLANG_BC_LVARIABLE */
     Compile_ByteCode_Ptr->b.i_blk = ((SLang_Local_Var_Type *) entry)->local_var_number;
   else
     Compile_ByteCode_Ptr->b.nt_blk = entry;

   lang_try_now ();
}

static void compile_tmp_variable (char *name, unsigned long hash)
{
   SLang_Name_Type *entry;
   unsigned char name_type;

   if (NULL == (entry = locate_hashed_name (name, hash)))
     {
	SLang_verror (SL_UNDEFINED_NAME, "%s is undefined", name);
	return;
     }

   name_type = entry->name_type;
   switch (name_type)
     {
      case SLANG_LVARIABLE:
	Compile_ByteCode_Ptr->b.i_blk = ((SLang_Local_Var_Type *) entry)->local_var_number;
	break;

      case SLANG_GVARIABLE:
      case SLANG_PVARIABLE:
	Compile_ByteCode_Ptr->b.nt_blk = entry;
	break;

      default:
	SLang_verror (SL_SYNTAX_ERROR, "__tmp(%s) does not specifiy a variable", name);
	return;
     }

   Compile_ByteCode_Ptr->bc_main_type = _SLANG_BC_TMP;
   Compile_ByteCode_Ptr->bc_sub_type = name_type;

   lang_try_now ();
}

static void compile_simple (unsigned char main_type)
{
   Compile_ByteCode_Ptr->bc_main_type = main_type;
   Compile_ByteCode_Ptr->bc_sub_type = 0;
   Compile_ByteCode_Ptr->b.blk = NULL;
   lang_try_now ();
}

static void compile_identifier (char *name, _SLang_Token_Type *tok)
{
   compile_hashed_identifier (name, _SLcompute_string_hash (name), tok);
}

static void compile_call_direct (int (*f) (void), unsigned char byte_code)
{
   Compile_ByteCode_Ptr->b.call_function = f;
   Compile_ByteCode_Ptr->bc_main_type = byte_code;
   Compile_ByteCode_Ptr->bc_sub_type = 0;
   lang_try_now ();
}

static void compile_lvar_call_direct (int (*f)(void), unsigned char bc,
				      unsigned char frame_op)
{
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (0 == try_compressed_bytecode (_SLANG_BC_LVARIABLE, bc))
     return;
#else
   (void) bc;
#endif

   compile_call_direct (f, frame_op);
}

static void compile_integer (long i, unsigned char bc_main_type, unsigned char bc_sub_type)
{
   Compile_ByteCode_Ptr->b.l_blk = i;
   Compile_ByteCode_Ptr->bc_main_type = bc_main_type;
   Compile_ByteCode_Ptr->bc_sub_type = bc_sub_type;

   lang_try_now ();
}

#if SLANG_HAS_FLOAT
static void compile_double (char *str, unsigned char main_type, SLtype type)
{
   double d;
   unsigned int factor = 1;
   double *ptr;

#if 1
   d = _SLang_atof (str);
#else
   if (1 != sscanf (str, "%lf", &d))
     {
	SLang_verror (SL_SYNTAX_ERROR, "Unable to convert %s to double", str);
	return;
     }
#endif

#if SLANG_HAS_COMPLEX
   if (type == SLANG_COMPLEX_TYPE) factor = 2;
#endif
   if (NULL == (ptr = (double *) SLmalloc(factor * sizeof(double))))
     return;

   Compile_ByteCode_Ptr->b.double_blk = ptr;
#if SLANG_HAS_COMPLEX
   if (type == SLANG_COMPLEX_TYPE)
     *ptr++ = 0;
#endif
   *ptr = d;

   Compile_ByteCode_Ptr->bc_main_type = main_type;
   Compile_ByteCode_Ptr->bc_sub_type = type;
   lang_try_now ();
}

static void compile_float (char *s)
{
   float x;

#if 1
   x = (float) _SLang_atof (s);
#else
   if (1 != sscanf (s, "%f", &x))
     {
	SLang_verror (SL_SYNTAX_ERROR, "Unable to convert %s to float", s);
	return;
     }
#endif
   Compile_ByteCode_Ptr->b.float_blk = x;
   Compile_ByteCode_Ptr->bc_main_type = _SLANG_BC_LITERAL;
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_FLOAT_TYPE;
   lang_try_now ();
}

#endif

static void compile_string (char *s, unsigned long hash)
{
   if (NULL == (Compile_ByteCode_Ptr->b.s_blk = _SLstring_dup_hashed_string (s, hash)))
     return;

   Compile_ByteCode_Ptr->bc_main_type = _SLANG_BC_LITERAL_STR;
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_STRING_TYPE;

   lang_try_now ();
}

static void compile_bstring (SLang_BString_Type *s)
{
   if (NULL == (Compile_ByteCode_Ptr->b.bs_blk = SLbstring_dup (s)))
     return;

   Compile_ByteCode_Ptr->bc_main_type = _SLANG_BC_LITERAL;
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_BSTRING_TYPE;

   lang_try_now ();
}

/* assign_type is one of _SLANG_BCST_ASSIGN, ... values */
static void compile_assign (unsigned char assign_type,
			    char *name, unsigned long hash)
{
   SLang_Name_Type *v;
   unsigned char main_type;
   SLang_Class_Type *cl;

   v = locate_hashed_name (name, hash);
   if (v == NULL)
     {
	if ((_SLang_Auto_Declare_Globals == 0)
	    || (NULL != strchr (name, '-'))   /* namespace->name form */
	    || Lang_Defining_Function
	    || (assign_type != _SLANG_BCST_ASSIGN)
	    || (This_Static_NameSpace == NULL))
	  {
	     SLang_verror (SL_UNDEFINED_NAME, "%s is undefined", name);
	     return;
	  }
	/* Note that function local variables are not at top level */

	/* Variables that are automatically declared are given static
	 * scope.
	 */
	if ((NULL != SLang_Auto_Declare_Var_Hook)
	    && (-1 == (*SLang_Auto_Declare_Var_Hook) (name)))
	  return;

	if ((-1 == add_global_variable (name, SLANG_GVARIABLE, hash, This_Static_NameSpace))
	    || (NULL == (v = locate_hashed_name (name, hash))))
	  return;
     }

   switch (v->name_type)
     {
      case SLANG_LVARIABLE:
	main_type = _SLANG_BC_SET_LOCAL_LVALUE;
	Compile_ByteCode_Ptr->b.i_blk = ((SLang_Local_Var_Type *) v)->local_var_number;
	break;

      case SLANG_GVARIABLE:
      case SLANG_PVARIABLE:
	main_type = _SLANG_BC_SET_GLOBAL_LVALUE;
	Compile_ByteCode_Ptr->b.nt_blk = v;
	break;

      case SLANG_IVARIABLE:
	cl = _SLclass_get_class (((SLang_Intrin_Var_Type *)v)->type);
	if (cl->cl_class_type != SLANG_CLASS_TYPE_SCALAR)
	  {
	     SLang_verror (SL_SYNTAX_ERROR, "Assignment to %s is not allowed", name);
	     return;
	  }
	main_type = _SLANG_BC_SET_INTRIN_LVALUE;
	Compile_ByteCode_Ptr->b.nt_blk = v;
	break;

      case SLANG_RVARIABLE:
	SLang_verror (SL_READONLY_ERROR, "%s is read-only", name);
	return;

      default:
	SLang_verror (SL_DUPLICATE_DEFINITION, "%s may not be used as an lvalue", name);
	return;
     }

   Compile_ByteCode_Ptr->bc_sub_type = assign_type;
   Compile_ByteCode_Ptr->bc_main_type = main_type;

   lang_try_now ();
}

static void compile_deref_assign (char *name, unsigned long hash)
{
   SLang_Name_Type *v;

   v = locate_hashed_name (name, hash);

   if (v == NULL)
     {
	SLang_verror (SL_UNDEFINED_NAME, "%s is undefined", name);
	return;
     }

   switch (v->name_type)
     {
      case SLANG_LVARIABLE:
	Compile_ByteCode_Ptr->b.i_blk = ((SLang_Local_Var_Type *) v)->local_var_number;
	break;

      case SLANG_GVARIABLE:
      case SLANG_PVARIABLE:
	Compile_ByteCode_Ptr->b.nt_blk = v;
	break;

      default:
	/* FIXME: Priority=low
	 * This could be made to work.  It is not a priority because
	 * I cannot imagine application intrinsics which are references.
	 */
	SLang_verror (SL_NOT_IMPLEMENTED, "Deref assignment to %s is not allowed", name);
	return;
     }

   Compile_ByteCode_Ptr->bc_sub_type = v->name_type;
   Compile_ByteCode_Ptr->bc_main_type = _SLANG_BC_DEREF_ASSIGN;

   lang_try_now ();
}

static void
compile_struct_assign (_SLang_Token_Type *t)
{
   Compile_ByteCode_Ptr->bc_sub_type = _SLANG_BCST_ASSIGN + (t->type - _STRUCT_ASSIGN_TOKEN);
   Compile_ByteCode_Ptr->bc_main_type = _SLANG_BC_SET_STRUCT_LVALUE;
   Compile_ByteCode_Ptr->b.s_blk = _SLstring_dup_hashed_string (t->v.s_val, t->hash);
   lang_try_now ();
}

static void
compile_array_assign (_SLang_Token_Type *t)
{
   Compile_ByteCode_Ptr->bc_sub_type = _SLANG_BCST_ASSIGN + (t->type - _ARRAY_ASSIGN_TOKEN);
   Compile_ByteCode_Ptr->bc_main_type = _SLANG_BC_SET_ARRAY_LVALUE;
   Compile_ByteCode_Ptr->b.s_blk = NULL;
   lang_try_now ();
}

static void compile_dot(_SLang_Token_Type *t)
{
   Compile_ByteCode_Ptr->bc_main_type = _SLANG_BC_FIELD;
   Compile_ByteCode_Ptr->b.s_blk = _SLstring_dup_hashed_string(t->v.s_val, t->hash);
   lang_try_now ();
}

static void compile_ref (char *name, unsigned long hash)
{
   SLang_Name_Type *entry;
   unsigned char main_type;

   if (NULL == (entry = locate_hashed_name (name, hash)))
     {
	SLang_verror (SL_UNDEFINED_NAME, "%s is undefined", name);
	return;
     }

   main_type = entry->name_type;

   if (main_type == SLANG_LVARIABLE)
     {
	main_type = _SLANG_BC_LOBJPTR;
	Compile_ByteCode_Ptr->b.i_blk = ((SLang_Local_Var_Type *)entry)->local_var_number;
     }
   else
     {
	main_type = _SLANG_BC_GOBJPTR;
	Compile_ByteCode_Ptr->b.nt_blk = entry;
     }

   Compile_ByteCode_Ptr->bc_main_type = main_type;
   lang_try_now ();
}

static void compile_break (unsigned char break_type,
			   int requires_block, int requires_fun,
			   char *str)
{
   if ((requires_fun
	&& (Lang_Defining_Function == 0))
       || (requires_block
	   && (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_BLOCK)))
     {
	SLang_verror (SL_SYNTAX_ERROR, "misplaced %s", str);
	return;
     }

   Compile_ByteCode_Ptr->bc_main_type = break_type;
   Compile_ByteCode_Ptr->bc_sub_type = 0;

   lang_try_now ();
}

static void compile_public_variable_mode (_SLang_Token_Type *t)
{
   if (t->type == IDENT_TOKEN)
     {
	/* If the variable is already defined in the static hash table,
	 * generate an error.
	 */
	if ((This_Static_NameSpace != NULL)
	    && (NULL != locate_name_in_table (t->v.s_val, t->hash, This_Static_NameSpace->table, This_Static_NameSpace->table_size)))
	  {
	     SLang_verror (SL_DUPLICATE_DEFINITION,
			   "%s already has static or private linkage in this unit",
			   t->v.s_val);
	     return;
	  }
	add_global_variable (t->v.s_val, SLANG_GVARIABLE, t->hash, Global_NameSpace);
     }
   else if (t->type == CBRACKET_TOKEN)
     Compile_Mode_Function = compile_basic_token_mode;
   else
     SLang_verror (SL_SYNTAX_ERROR, "Misplaced token in variable list");
}

static void compile_local_variable_mode (_SLang_Token_Type *t)
{
   if (t->type == IDENT_TOKEN)
     add_local_variable (t->v.s_val, t->hash);
   else if (t->type == CBRACKET_TOKEN)
     Compile_Mode_Function = compile_basic_token_mode;
   else
     SLang_verror (SL_SYNTAX_ERROR, "Misplaced token in variable list");
}

static void compile_static_variable_mode (_SLang_Token_Type *t)
{
   if (t->type == IDENT_TOKEN)
     add_global_variable (t->v.s_val, SLANG_GVARIABLE, t->hash, This_Static_NameSpace);
   else if (t->type == CBRACKET_TOKEN)
     Compile_Mode_Function = compile_basic_token_mode;
   else
     SLang_verror (SL_SYNTAX_ERROR, "Misplaced token in variable list");
}

static void compile_private_variable_mode (_SLang_Token_Type *t)
{
   if (t->type == IDENT_TOKEN)
     add_global_variable (t->v.s_val, SLANG_PVARIABLE, t->hash, This_Static_NameSpace);
   else if (t->type == CBRACKET_TOKEN)
     Compile_Mode_Function = compile_basic_token_mode;
   else
     SLang_verror (SL_SYNTAX_ERROR, "Misplaced token in variable list");
}

static void compile_function_mode (_SLang_Token_Type *t)
{
   if (-1 == lang_check_space ())
     return;

   if (t->type != IDENT_TOKEN)
     SLang_verror (SL_SYNTAX_ERROR, "Expecting function name");
   else
     lang_define_function (t->v.s_val, SLANG_FUNCTION, t->hash, Global_NameSpace);

   Compile_Mode_Function = compile_basic_token_mode;
}

/* An error block is not permitted to contain continue or break statements.
 * This restriction may be removed later but for now reject them.
 */
static int check_error_block (void)
{
   SLBlock_Type *p;
   unsigned char t;

   /* Back up to the block and then scan it. */
   p = (Compile_ByteCode_Ptr - 1)->b.blk;

   while (0 != (t = p->bc_main_type))
     {
	if ((t == _SLANG_BC_BREAK)
	    || (t == _SLANG_BC_CONTINUE))
	  {
	     SLang_verror (SL_SYNTAX_ERROR,
			   "An ERROR_BLOCK is not permitted to contain continue or break statements");
	     return -1;
	  }
	p++;
     }
   return 0;
}

/* The only allowed tokens are the directives and another block start.
 * The mode is only active if a block is available.  The inner_interp routine
 * expects such safety checks.
 */
static void compile_directive_mode (_SLang_Token_Type *t)
{
   int bc_sub_type;

   if (-1 == lang_check_space ())
     return;

   bc_sub_type = -1;

   switch (t->type)
     {
      case FOREVER_TOKEN:
	bc_sub_type = _SLANG_BCST_FOREVER;
	break;

      case IFNOT_TOKEN:
	bc_sub_type = _SLANG_BCST_IFNOT;
	break;

      case IF_TOKEN:
	bc_sub_type = _SLANG_BCST_IF;
	break;

      case ANDELSE_TOKEN:
	bc_sub_type = _SLANG_BCST_ANDELSE;
	break;

      case SWITCH_TOKEN:
	bc_sub_type = _SLANG_BCST_SWITCH;
	break;

      case EXITBLK_TOKEN:
	if (Lang_Defining_Function == 0)
	  {
	     SLang_verror (SL_SYNTAX_ERROR, "misplaced EXIT_BLOCK");
	     break;
	  }
	bc_sub_type = _SLANG_BCST_EXIT_BLOCK;
	break;

      case ERRBLK_TOKEN:
	if (This_Compile_Block_Type == COMPILE_BLOCK_TYPE_TOP_LEVEL)
	  {
	     SLang_verror (SL_SYNTAX_ERROR, "misplaced ERROR_BLOCK");
	     break;
	  }
	if (0 == check_error_block ())
	  bc_sub_type = _SLANG_BCST_ERROR_BLOCK;
	break;

      case USRBLK0_TOKEN:
      case USRBLK1_TOKEN:
      case USRBLK2_TOKEN:
      case USRBLK3_TOKEN:
      case USRBLK4_TOKEN:
	if (This_Compile_Block_Type == COMPILE_BLOCK_TYPE_TOP_LEVEL)
	  {
	     SLang_verror (SL_SYNTAX_ERROR, "misplaced USER_BLOCK");
	     break;
	  }
	bc_sub_type = _SLANG_BCST_USER_BLOCK0 + (t->type - USRBLK0_TOKEN);
	break;

      case NOTELSE_TOKEN:
	bc_sub_type = _SLANG_BCST_NOTELSE;
	break;

      case ELSE_TOKEN:
	bc_sub_type = _SLANG_BCST_ELSE;
	break;

      case LOOP_TOKEN:
	bc_sub_type = _SLANG_BCST_LOOP;
	break;

      case DOWHILE_TOKEN:
	bc_sub_type = _SLANG_BCST_DOWHILE;
	break;

      case WHILE_TOKEN:
	bc_sub_type = _SLANG_BCST_WHILE;
	break;

      case ORELSE_TOKEN:
	bc_sub_type = _SLANG_BCST_ORELSE;
	break;

      case _FOR_TOKEN:
	bc_sub_type = _SLANG_BCST_FOR;
	break;

      case FOR_TOKEN:
	bc_sub_type = _SLANG_BCST_CFOR;
	break;

      case FOREACH_TOKEN:
	bc_sub_type = _SLANG_BCST_FOREACH;
	break;

      case OBRACE_TOKEN:
	lang_begin_block ();
	break;

      default:
	SLang_verror (SL_SYNTAX_ERROR, "Expecting directive token.  Found 0x%X", t->type);
	break;
     }

   /* Reset this pointer first because compile_directive may cause a
    * file to be loaded.
    */
   Compile_Mode_Function = compile_basic_token_mode;

   if (bc_sub_type != -1)
     compile_directive (bc_sub_type);
}

static unsigned int Assign_Mode_Type;
static void compile_assign_mode (_SLang_Token_Type *t)
{
   if (t->type != IDENT_TOKEN)
     {
	SLang_verror (SL_SYNTAX_ERROR, "Expecting identifier for assignment");
	return;
     }

   compile_assign (Assign_Mode_Type, t->v.s_val, t->hash);
   Compile_Mode_Function = compile_basic_token_mode;
}

static void compile_basic_token_mode (_SLang_Token_Type *t)
{
   if (-1 == lang_check_space ())
     return;

   switch (t->type)
     {
      case PUSH_TOKEN:
      case NOP_TOKEN:
      case EOF_TOKEN:
      case READONLY_TOKEN:
      case DO_TOKEN:
      case VARIABLE_TOKEN:
      case SEMICOLON_TOKEN:
      default:
	SLang_verror (SL_SYNTAX_ERROR, "Unknown or unsupported token type 0x%X", t->type);
	break;

      case DEREF_TOKEN:
	compile_call_direct (dereference_object, _SLANG_BC_CALL_DIRECT);
	break;

      case STRUCT_TOKEN:
	compile_call_direct (_SLstruct_define_struct, _SLANG_BC_CALL_DIRECT);
	break;

      case TYPEDEF_TOKEN:
	compile_call_direct (_SLstruct_define_typedef, _SLANG_BC_CALL_DIRECT);
	break;

      case TMP_TOKEN:
	compile_tmp_variable (t->v.s_val, t->hash);
	break;

      case DOT_TOKEN:		       /* X . field */
	compile_dot (t);
	break;

      case COMMA_TOKEN:
	break;			       /* do nothing */

      case IDENT_TOKEN:
	compile_hashed_identifier (t->v.s_val, t->hash, t);
	break;

      case _REF_TOKEN:
	compile_ref (t->v.s_val, t->hash);
	break;

      case ARG_TOKEN:
	compile_call_direct (SLang_start_arg_list, _SLANG_BC_CALL_DIRECT);
	break;

      case EARG_TOKEN:
	compile_lvar_call_direct (SLang_end_arg_list, _SLANG_BC_EARG_LVARIABLE, _SLANG_BC_CALL_DIRECT);
	break;

      case COLON_TOKEN:
	if (This_Compile_Block_Type == COMPILE_BLOCK_TYPE_BLOCK)
	  compile_simple (_SLANG_BC_LABEL);
	else SLang_Error = SL_SYNTAX_ERROR;
	break;

      case POP_TOKEN:
	compile_call_direct (SLdo_pop, _SLANG_BC_CALL_DIRECT);
	break;

      case CASE_TOKEN:
	if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_BLOCK)
	  SLang_verror (SL_SYNTAX_ERROR, "Misplaced 'case'");
	else
	  compile_call_direct (case_function, _SLANG_BC_CALL_DIRECT);
	break;

      case CHAR_TOKEN:
	compile_integer (t->v.long_val, _SLANG_BC_LITERAL, SLANG_CHAR_TYPE);
	break;
      case SHORT_TOKEN:
	compile_integer (t->v.long_val, _SLANG_BC_LITERAL, SLANG_SHORT_TYPE);
	break;
      case INT_TOKEN:
	compile_integer (t->v.long_val, _SLANG_BC_LITERAL_INT, SLANG_INT_TYPE);
	break;
      case UCHAR_TOKEN:
	compile_integer (t->v.long_val, _SLANG_BC_LITERAL, SLANG_UCHAR_TYPE);
	break;
      case USHORT_TOKEN:
	compile_integer (t->v.long_val, _SLANG_BC_LITERAL, SLANG_USHORT_TYPE);
	break;
      case UINT_TOKEN:
	compile_integer (t->v.long_val, _SLANG_BC_LITERAL_INT, SLANG_UINT_TYPE);
	break;
      case LONG_TOKEN:
	compile_integer (t->v.long_val, _SLANG_BC_LITERAL, SLANG_LONG_TYPE);
	break;
      case ULONG_TOKEN:
	compile_integer (t->v.long_val, _SLANG_BC_LITERAL, SLANG_ULONG_TYPE);
	break;

#if SLANG_HAS_FLOAT
      case FLOAT_TOKEN:
	compile_float (t->v.s_val);
	break;

      case DOUBLE_TOKEN:
	compile_double (t->v.s_val, _SLANG_BC_LITERAL_DBL, SLANG_DOUBLE_TYPE);
	break;
#endif
#if SLANG_HAS_COMPLEX
      case COMPLEX_TOKEN:
	compile_double (t->v.s_val, _SLANG_BC_LITERAL, SLANG_COMPLEX_TYPE);
	break;
#endif

      case STRING_TOKEN:
	compile_string (t->v.s_val, t->hash);
	break;

      case _BSTRING_TOKEN:
	compile_bstring (SLbstring_create ((unsigned char *)t->v.s_val, (unsigned int) t->hash));
	break;

      case BSTRING_TOKEN:
	compile_bstring (t->v.b_val);
	break;

      case _NULL_TOKEN:
	compile_identifier ("NULL", t);
	break;

      case _INLINE_WILDCARD_ARRAY_TOKEN:
	compile_call_direct (_SLarray_wildcard_array, _SLANG_BC_CALL_DIRECT);
	break;

      case _INLINE_ARRAY_TOKEN:
	compile_call_direct (_SLarray_inline_array, _SLANG_BC_CALL_DIRECT_FRAME);
	break;

      case _INLINE_IMPLICIT_ARRAY_TOKEN:
	compile_call_direct (_SLarray_inline_implicit_array, _SLANG_BC_CALL_DIRECT_FRAME);
	break;

      case ARRAY_TOKEN:
	compile_lvar_call_direct (_SLarray_aget, _SLANG_BC_LVARIABLE_AGET, _SLANG_BC_CALL_DIRECT_FRAME);
	break;

	/* Note: I need to add the other _ARRAY assign tokens. */
      case _ARRAY_PLUSEQS_TOKEN:
      case _ARRAY_MINUSEQS_TOKEN:
      case _ARRAY_TIMESEQS_TOKEN:
      case _ARRAY_DIVEQS_TOKEN:
      case _ARRAY_BOREQS_TOKEN:
      case _ARRAY_BANDEQS_TOKEN:
      case _ARRAY_POST_MINUSMINUS_TOKEN:
      case _ARRAY_MINUSMINUS_TOKEN:
      case _ARRAY_POST_PLUSPLUS_TOKEN:
      case _ARRAY_PLUSPLUS_TOKEN:
	compile_array_assign (t);
	break;

      case _ARRAY_ASSIGN_TOKEN:
	compile_lvar_call_direct (_SLarray_aput, _SLANG_BC_LVARIABLE_APUT, _SLANG_BC_CALL_DIRECT_FRAME);
	break;

      case _STRUCT_ASSIGN_TOKEN:
      case _STRUCT_PLUSEQS_TOKEN:
      case _STRUCT_MINUSEQS_TOKEN:
      case _STRUCT_TIMESEQS_TOKEN:
      case _STRUCT_DIVEQS_TOKEN:
      case _STRUCT_BOREQS_TOKEN:
      case _STRUCT_BANDEQS_TOKEN:
      case _STRUCT_POST_MINUSMINUS_TOKEN:
      case _STRUCT_MINUSMINUS_TOKEN:
      case _STRUCT_POST_PLUSPLUS_TOKEN:
      case _STRUCT_PLUSPLUS_TOKEN:
	compile_struct_assign (t);
	break;

      case _SCALAR_ASSIGN_TOKEN:
      case _SCALAR_PLUSEQS_TOKEN:
      case _SCALAR_MINUSEQS_TOKEN:
      case _SCALAR_TIMESEQS_TOKEN:
      case _SCALAR_DIVEQS_TOKEN:
      case _SCALAR_BOREQS_TOKEN:
      case _SCALAR_BANDEQS_TOKEN:
      case _SCALAR_POST_MINUSMINUS_TOKEN:
      case _SCALAR_MINUSMINUS_TOKEN:
      case _SCALAR_POST_PLUSPLUS_TOKEN:
      case _SCALAR_PLUSPLUS_TOKEN:
	compile_assign (_SLANG_BCST_ASSIGN + (t->type - _SCALAR_ASSIGN_TOKEN),
			t->v.s_val, t->hash);
	break;

      case _DEREF_ASSIGN_TOKEN:
	compile_deref_assign (t->v.s_val, t->hash);
	break;

 	/* For processing RPN tokens */
      case ASSIGN_TOKEN:
      case PLUSEQS_TOKEN:
      case MINUSEQS_TOKEN:
      case TIMESEQS_TOKEN:
      case DIVEQS_TOKEN:
      case BOREQS_TOKEN:
      case BANDEQS_TOKEN:
      case POST_MINUSMINUS_TOKEN:
      case MINUSMINUS_TOKEN:
      case POST_PLUSPLUS_TOKEN:
      case PLUSPLUS_TOKEN:
	Compile_Mode_Function = compile_assign_mode;
	Assign_Mode_Type = _SLANG_BCST_ASSIGN + (t->type - ASSIGN_TOKEN);
	break;

      case LT_TOKEN:
	compile_binary (SLANG_LT);
	break;

      case LE_TOKEN:
	compile_binary (SLANG_LE);
	break;

      case GT_TOKEN:
	compile_binary (SLANG_GT);
	break;

      case GE_TOKEN:
	compile_binary (SLANG_GE);
	break;

      case EQ_TOKEN:
	compile_binary (SLANG_EQ);
	break;

      case NE_TOKEN:
	compile_binary (SLANG_NE);
	break;

      case AND_TOKEN:
	compile_binary (SLANG_AND);
	break;

      case ADD_TOKEN:
	compile_fast_binary (SLANG_PLUS, _SLANG_BC_INTEGER_PLUS);
	break;

      case SUB_TOKEN:
	compile_fast_binary (SLANG_MINUS, _SLANG_BC_INTEGER_MINUS);
	break;

      case TIMES_TOKEN:
	compile_binary (SLANG_TIMES);
	break;

      case DIV_TOKEN:
	compile_binary (SLANG_DIVIDE);
	break;

      case POW_TOKEN:
	compile_binary (SLANG_POW);
	break;

      case BXOR_TOKEN:
	compile_binary (SLANG_BXOR);
	break;

      case BAND_TOKEN:
	compile_binary (SLANG_BAND);
	break;

      case BOR_TOKEN:
	compile_binary (SLANG_BOR);
	break;

      case SHR_TOKEN:
	compile_binary (SLANG_SHR);
	break;

      case SHL_TOKEN:
	compile_binary (SLANG_SHL);
	break;

      case MOD_TOKEN:
	compile_binary (SLANG_MOD);
	break;

      case OR_TOKEN:
	compile_binary (SLANG_OR);
	break;

      case NOT_TOKEN:
	compile_unary (SLANG_NOT, _SLANG_BC_UNARY);
	break;

      case BNOT_TOKEN:
	compile_unary (SLANG_BNOT, _SLANG_BC_UNARY);
	break;

      case MUL2_TOKEN:
	compile_unary (SLANG_MUL2, _SLANG_BC_UNARY_FUNC);
	break;

      case CHS_TOKEN:
	compile_unary (SLANG_CHS, _SLANG_BC_UNARY_FUNC);
	break;

      case ABS_TOKEN:
	compile_unary (SLANG_ABS, _SLANG_BC_UNARY_FUNC);
	break;

      case SQR_TOKEN:
	compile_unary (SLANG_SQR, _SLANG_BC_UNARY_FUNC);
	break;

      case SIGN_TOKEN:
	compile_unary (SLANG_SIGN, _SLANG_BC_UNARY_FUNC);
	break;

      case BREAK_TOKEN:
	compile_break (_SLANG_BC_BREAK, 1, 0, "break");
	break;

      case RETURN_TOKEN:
	compile_break (_SLANG_BC_RETURN, 0, 1, "return");
	break;

      case CONT_TOKEN:
	compile_break (_SLANG_BC_CONTINUE, 1, 0, "continue");
	break;

      case EXCH_TOKEN:
	compile_break (_SLANG_BC_EXCH, 0, 0, "");   /* FIXME: Priority=low */
	break;

      case STATIC_TOKEN:
	if (Lang_Defining_Function == 0)
	  Compile_Mode_Function = compile_static_variable_mode;
	else
	  SLang_verror (SL_NOT_IMPLEMENTED, "static variables not permitted in functions");
	break;

      case PRIVATE_TOKEN:
	if (Lang_Defining_Function == 0)
	  Compile_Mode_Function = compile_private_variable_mode;
	else
	  SLang_verror (SL_NOT_IMPLEMENTED, "private variables not permitted in functions");
	break;

      case PUBLIC_TOKEN:
	if (Lang_Defining_Function == 0)
	  Compile_Mode_Function = compile_public_variable_mode;
	else
	  SLang_verror (SL_NOT_IMPLEMENTED, "public variables not permitted in functions");
	break;

      case OBRACKET_TOKEN:
	if (Lang_Defining_Function == 0)
	  Compile_Mode_Function = Default_Variable_Mode;
	else
	  Compile_Mode_Function = compile_local_variable_mode;
	break;

      case OPAREN_TOKEN:
	lang_begin_function ();
	break;

      case DEFINE_STATIC_TOKEN:
	if (Lang_Defining_Function)
	  define_static_function (t->v.s_val, t->hash);
	else SLang_Error = SL_SYNTAX_ERROR;
	break;

      case DEFINE_PRIVATE_TOKEN:
	if (Lang_Defining_Function)
	  define_private_function (t->v.s_val, t->hash);
	else SLang_Error = SL_SYNTAX_ERROR;
	break;

      case DEFINE_PUBLIC_TOKEN:
	if (Lang_Defining_Function)
	  define_public_function (t->v.s_val, t->hash);
	else SLang_Error = SL_SYNTAX_ERROR;
	break;

      case DEFINE_TOKEN:
	if (Lang_Defining_Function)
	  (*Default_Define_Function) (t->v.s_val, t->hash);
	else
	  SLang_Error = SL_SYNTAX_ERROR;
	break;

      case CPAREN_TOKEN:
	if (Lang_Defining_Function)
	  Compile_Mode_Function = compile_function_mode;
	else SLang_Error = SL_SYNTAX_ERROR;
	break;

      case CBRACE_TOKEN:
	lang_end_block ();
	Compile_Mode_Function = compile_directive_mode;
	break;

      case OBRACE_TOKEN:
	lang_begin_block ();
	break;

      case FARG_TOKEN:
	Function_Args_Number = Local_Variable_Number;
	break;

#if _SLANG_HAS_DEBUG_CODE
      case LINE_NUM_TOKEN:
	Compile_ByteCode_Ptr->bc_main_type = _SLANG_BC_LINE_NUM;
	Compile_ByteCode_Ptr->b.l_blk = t->v.long_val;
	lang_try_now ();
	break;
#endif
      case POUND_TOKEN:
	compile_call_direct (_SLarray_matrix_multiply, _SLANG_BC_CALL_DIRECT);
	break;
     }
}

void _SLcompile (_SLang_Token_Type *t)
{
   if (SLang_Error == 0)
     {
	if (Compile_Mode_Function != compile_basic_token_mode)
	  {
	     if (Compile_Mode_Function == NULL)
	       Compile_Mode_Function = compile_basic_token_mode;
#if _SLANG_HAS_DEBUG_CODE
	     if (t->type == LINE_NUM_TOKEN)
	       {
		  compile_basic_token_mode (t);
		  return;
	       }
#endif
	  }

	(*Compile_Mode_Function) (t);
     }

   if (SLang_Error)
     {
	Compile_Mode_Function = compile_basic_token_mode;
	SLang_restart (0);
     }
}

void (*_SLcompile_ptr)(_SLang_Token_Type *) = _SLcompile;

typedef struct _Compile_Context_Type
{
   struct _Compile_Context_Type *next;
   SLang_NameSpace_Type *static_namespace;
   void (*compile_variable_mode) (_SLang_Token_Type *);
   void (*define_function) (char *, unsigned long);
   int lang_defining_function;
   int local_variable_number;
   unsigned int function_args_number;
   SLang_Name_Type **locals_hash_table;
   void (*compile_mode_function)(_SLang_Token_Type *);
#if _SLANG_HAS_DEBUG_CODE
   char *compile_filename;
#endif
}
Compile_Context_Type;

static Compile_Context_Type *Compile_Context_Stack;

/* The only way the push/pop_context functions can get called is via
 * an eval type function.  That can only happen when executed from a
 * top level block.  This means that Compile_ByteCode_Ptr can always be
 * rest back to the beginning of a block.
 */

static int pop_compile_context (void)
{
   Compile_Context_Type *cc;

   if (NULL == (cc = Compile_Context_Stack))
     return -1;

   This_Static_NameSpace = cc->static_namespace;
   Compile_Context_Stack = cc->next;
   Default_Variable_Mode = cc->compile_variable_mode;
   Default_Define_Function = cc->define_function;
   Compile_Mode_Function = cc->compile_mode_function;

   Lang_Defining_Function = cc->lang_defining_function;
   Local_Variable_Number = cc->local_variable_number;
   Function_Args_Number = cc->function_args_number;

#if _SLANG_HAS_DEBUG_CODE
   SLang_free_slstring (This_Compile_Filename);
   This_Compile_Filename = cc->compile_filename;
#endif

   SLfree ((char *) Locals_Hash_Table);
   Locals_Hash_Table = cc->locals_hash_table;

   SLfree ((char *) cc);

   return 0;
}

static int push_compile_context (char *name)
{
   Compile_Context_Type *cc;
   SLang_Name_Type **lns;

   cc = (Compile_Context_Type *)SLmalloc (sizeof (Compile_Context_Type));
   if (cc == NULL)
     return -1;
   memset ((char *) cc, 0, sizeof (Compile_Context_Type));

   lns = (SLang_Name_Type **) SLcalloc (sizeof (SLang_Name_Type *), SLLOCALS_HASH_TABLE_SIZE);
   if (lns == NULL)
     {
	SLfree ((char *) cc);
	return -1;
     }

#if _SLANG_HAS_DEBUG_CODE
   if ((name != NULL)
       && (NULL == (name = SLang_create_slstring (name))))
     {
	SLfree ((char *) cc);
	SLfree ((char *) lns);
	return -1;
     }
		
   cc->compile_filename = This_Compile_Filename;
   This_Compile_Filename = name;
#endif

   cc->static_namespace = This_Static_NameSpace;
   cc->compile_variable_mode = Default_Variable_Mode;
   cc->define_function = Default_Define_Function;
   cc->locals_hash_table = Locals_Hash_Table;

   cc->lang_defining_function = Lang_Defining_Function;
   cc->local_variable_number = Local_Variable_Number;
   cc->function_args_number = Function_Args_Number;
   cc->locals_hash_table = Locals_Hash_Table;
   cc->compile_mode_function = Compile_Mode_Function;

   cc->next = Compile_Context_Stack;
   Compile_Context_Stack = cc;

   Compile_Mode_Function = compile_basic_token_mode;
   Default_Variable_Mode = compile_public_variable_mode;
   Default_Define_Function = define_public_function;
   Lang_Defining_Function = 0;
   Local_Variable_Number = 0;
   Function_Args_Number = 0;
   Locals_Hash_Table = lns;
   return 0;
}

static int init_interpreter (void)
{
   SLang_NameSpace_Type *ns;

   if (Global_NameSpace != NULL)
     return 0;

   if (NULL == (ns = _SLns_allocate_namespace ("***GLOBAL***", SLGLOBALS_HASH_TABLE_SIZE)))
     return -1;
   if (-1 == _SLns_set_namespace_name (ns, "Global"))
     return -1;
   Global_NameSpace = ns;

   _SLRun_Stack = (SLang_Object_Type *) SLcalloc (SLANG_MAX_STACK_LEN,
						  sizeof (SLang_Object_Type));
   if (_SLRun_Stack == NULL)
     return -1;

   _SLStack_Pointer = _SLRun_Stack;
   _SLStack_Pointer_Max = _SLRun_Stack + SLANG_MAX_STACK_LEN;

   SLShort_Blocks[SHORT_BLOCK_RETURN_INDX].bc_main_type = _SLANG_BC_RETURN;
   SLShort_Blocks[SHORT_BLOCK_BREAK_INDX].bc_main_type = _SLANG_BC_BREAK;
   SLShort_Blocks[SHORT_BLOCK_CONTINUE_INDX].bc_main_type = _SLANG_BC_CONTINUE;

   Num_Args_Stack = (int *) SLmalloc (sizeof (int) * SLANG_MAX_RECURSIVE_DEPTH);
   if (Num_Args_Stack == NULL)
     {
	SLfree ((char *) _SLRun_Stack);
	return -1;
     }
   Recursion_Depth = 0;
   Frame_Pointer_Stack = (unsigned int *) SLmalloc (sizeof (unsigned int) * SLANG_MAX_RECURSIVE_DEPTH);
   if (Frame_Pointer_Stack == NULL)
     {
	SLfree ((char *) _SLRun_Stack);
	SLfree ((char *)Num_Args_Stack);
	return -1;
     }
   Frame_Pointer_Depth = 0;
   Frame_Pointer = _SLRun_Stack;

   (void) setup_default_compile_linkage (1);
   return 0;
}

static int add_generic_table (SLang_NameSpace_Type *ns,
			      SLang_Name_Type *table, char *pp_name,
			      unsigned int entry_len)
{
   SLang_Name_Type *t, **ns_table;
   char *name;
   unsigned int table_size;
   
   if (-1 == init_interpreter ())
     return -1;
   
   if (ns == NULL) 
     ns = Global_NameSpace;
   
   ns_table = ns->table;
   table_size = ns->table_size;
	
   if ((pp_name != NULL)
       && (-1 == SLdefine_for_ifdef (pp_name)))
     return -1;

   t = table;
   while (NULL != (name = t->name))
     {
	unsigned long hash;

	/* Backward compatibility: '.' WAS used as hash marker */
	if (*name == '.')
	  {
	     name++;
	     t->name = name;
	  }

	if (NULL == (name = SLang_create_slstring (name)))
	  return -1;

	t->name = name;

	hash = _SLcompute_string_hash (name);
	hash = hash % table_size;

	/* First time.  Make sure this has not already been added */
	if (t == table)
	  {
	     SLang_Name_Type *tt = ns_table[(unsigned int) hash];
	     while (tt != NULL)
	       {
		  if (tt == t)
		    {
		       SLang_verror (SL_APPLICATION_ERROR, 
				     "An intrinsic symbol table may not be added twice. [%s]", 
				     pp_name == NULL ? "" : pp_name);
		       return -1;
		    }
		  tt = tt->next;
	       }
	  }

	t->next = ns_table [(unsigned int) hash];
	ns_table [(unsigned int) hash] = t;

	t = (SLang_Name_Type *) ((char *)t + entry_len);
     }

   return 0;
}

int SLadd_intrin_fun_table (SLang_Intrin_Fun_Type *tbl, char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Intrin_Fun_Type));
}

int SLadd_intrin_var_table (SLang_Intrin_Var_Type *tbl, char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Intrin_Var_Type));
}

int SLadd_app_unary_table (SLang_App_Unary_Type *tbl, char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_App_Unary_Type));
}

int SLadd_math_unary_table (SLang_Math_Unary_Type *tbl, char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Math_Unary_Type));
}

int SLadd_iconstant_table (SLang_IConstant_Type *tbl, char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_IConstant_Type));
}

#if SLANG_HAS_FLOAT
int SLadd_dconstant_table (SLang_DConstant_Type *tbl, char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_DConstant_Type));
}
#endif

/* ----------- */
int SLns_add_intrin_fun_table (SLang_NameSpace_Type *ns, SLang_Intrin_Fun_Type *tbl, char *pp)
{
   return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Intrin_Fun_Type));
}

int SLns_add_intrin_var_table (SLang_NameSpace_Type *ns, SLang_Intrin_Var_Type *tbl, char *pp)
{
   return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Intrin_Var_Type));
}

int SLns_add_app_unary_table (SLang_NameSpace_Type *ns, SLang_App_Unary_Type *tbl, char *pp)
{
   return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_App_Unary_Type));
}

int SLns_add_math_unary_table (SLang_NameSpace_Type *ns, SLang_Math_Unary_Type *tbl, char *pp)
{
   return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Math_Unary_Type));
}

int SLns_add_iconstant_table (SLang_NameSpace_Type *ns, SLang_IConstant_Type *tbl, char *pp)
{
   return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_IConstant_Type));
}

#if SLANG_HAS_FLOAT
int SLns_add_dconstant_table (SLang_NameSpace_Type *ns, SLang_DConstant_Type *tbl, char *pp)
{
   return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_DConstant_Type));
}
#endif

static int setup_default_compile_linkage (int is_public)
{
   if (is_public)
     {
	Default_Define_Function = define_public_function;
	Default_Variable_Mode = compile_public_variable_mode;
     }
   else
     {
	Default_Define_Function = define_static_function;
	Default_Variable_Mode = compile_static_variable_mode;
     }
   
   return 0;
}

/* what is a bitmapped value:
 * 1 intrin fun
 * 2 user fun
 * 4 intrin var
 * 8 user defined var
 */
SLang_Array_Type *_SLang_apropos (char *namespace_name, char *pat, unsigned int what)
{
   SLang_NameSpace_Type *ns;

   if (namespace_name == NULL)
     namespace_name = "Global";

   if (*namespace_name == 0)
     ns = This_Static_NameSpace;
   else ns = _SLns_find_namespace (namespace_name);

   return _SLnspace_apropos (ns, pat, what);
}

   
void _SLang_implements_intrinsic (char *name)
{
   (void) implements_ns (name);
}

   
void _SLang_use_namespace_intrinsic (char *name)
{
   SLang_NameSpace_Type *ns;
   
   if (NULL == (ns = _SLns_find_namespace (name)))
     {
	SLang_verror (SL_INTRINSIC_ERROR, "Namespace %s does not exist", name);
	return;
     }
   This_Static_NameSpace = ns;
   (void) setup_default_compile_linkage (ns == Global_NameSpace);
}


char *_SLang_cur_namespace_intrinsic (void)
{
   if (This_Static_NameSpace == NULL)
     return "Global";

   if (This_Static_NameSpace->namespace_name == NULL)
     return "";
   
   return This_Static_NameSpace->namespace_name;
}

char *_SLang_current_function_name (void)
{
   return Current_Function_Name;
}

SLang_Object_Type *_SLang_get_run_stack_pointer (void)
{
   return _SLStack_Pointer;
}

SLang_Object_Type *_SLang_get_run_stack_base (void)
{
   return _SLRun_Stack;
}


int _SLang_dump_stack (void) /*{{{*/
{
   char buf[32];
   unsigned int n;

   n = (unsigned int) (_SLStack_Pointer - _SLRun_Stack);
   while (n)
     {
	n--;
	sprintf (buf, "(%u)", n);
	_SLdump_objects (buf, _SLRun_Stack + n, 1, 1);
     }
   return 0;
}

/*}}}*/

int _SLang_is_arith_type (SLtype t)
{
   return (int) Is_Arith_Type[t];
}
void _SLang_set_arith_type (SLtype t, unsigned char v)
{
   Is_Arith_Type[t] = v;
}

#if _SLANG_OPTIMIZE_FOR_SPEED
int _SLang_get_class_type (SLtype t)
{
   return Class_Type[t];
}
void _SLang_set_class_type (SLtype t, SLtype ct)
{
   Class_Type[t] = ct;
}

#endif
