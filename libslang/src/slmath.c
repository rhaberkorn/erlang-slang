/* sin, cos, etc, for S-Lang */
/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include <math.h>

#if SLANG_HAS_FLOAT
#include "slang.h"
#include "_slang.h"

#ifdef PI
# undef PI
#endif
#define PI 3.14159265358979323846264338327950288

#if defined(__unix__)
#include <signal.h>
#include <errno.h>

#define SIGNAL  SLsignal

static void math_floating_point_exception (int sig)
{
   sig = errno;
   if (SLang_Error == 0) SLang_Error = SL_FLOATING_EXCEPTION;
   (void) SIGNAL (SIGFPE, math_floating_point_exception);
   errno = sig;
}
#endif

double SLmath_hypot (double x, double y)
{
   double fr, fi, ratio;

   fr = fabs(x);
   fi = fabs(y);

   if (fr > fi)
     {
	ratio = y / x;
	x = fr * sqrt (1.0 + ratio * ratio);
     }
   else if (fi == 0.0) x = 0.0;
   else
     {
	ratio = x / y;
	x = fi * sqrt (1.0 + ratio * ratio);
     }

   return x;
}

/* usage here is a1 a2 ... an n x ==> a1x^n + a2 x ^(n - 1) + ... + an */
static double math_poly (void)
{
   int n;
   double xn = 1.0, sum = 0.0;
   double an, x;

   if ((SLang_pop_double(&x, NULL, NULL))
       || (SLang_pop_integer(&n))) return(0.0);

   while (n-- > 0)
     {
	if (SLang_pop_double(&an, NULL, NULL)) break;
	sum += an * xn;
	xn = xn * x;
     }
   return (double) sum;
}

static int double_math_op_result (int op, unsigned char a, unsigned char *b)
{
   (void) op;

   if (a != SLANG_FLOAT_TYPE)
     *b = SLANG_DOUBLE_TYPE;
   else
     *b = a;

   return 1;
}

#ifdef HAVE_ASINH
# define ASINH_FUN	asinh
#else
# define ASINH_FUN	my_asinh
static double my_asinh (double x)
{
   return log (x + sqrt (x*x + 1));
}
#endif
#ifdef HAVE_ACOSH
# define ACOSH_FUN	acosh
#else
# define ACOSH_FUN	my_acosh
static double my_acosh (double x)
{
   return log (x + sqrt(x*x - 1));     /* x >= 1 */
}
#endif
#ifdef HAVE_ATANH
# define ATANH_FUN	atanh
#else
# define ATANH_FUN	my_atanh
static double my_atanh (double x)
{
   return 0.5 * log ((1.0 + x)/(1.0 - x)); /* 0 <= x^2 < 1 */
}
#endif

static int double_math_op (int op,
			   unsigned char type, VOID_STAR ap, unsigned int na,
			   VOID_STAR bp)
{
   double *a, *b;
   unsigned int i;
   double (*fun) (double);

   (void) type;
   a = (double *) ap;
   b = (double *) bp;

   switch (op)
     {
      default:
	return 0;

      case SLMATH_SINH:
	fun = sinh;
	break;
      case SLMATH_COSH:
	fun = cosh;
	break;
      case SLMATH_TANH:
	fun = tanh;
	break;
      case SLMATH_TAN:
	fun = tan;
	break;
      case SLMATH_ASIN:
	fun = asin;
	break;
      case SLMATH_ACOS:
	fun = acos;
	break;
      case SLMATH_ATAN:
	fun = atan;
	break;
      case SLMATH_EXP:
	fun = exp;
	break;
      case SLMATH_LOG:
	fun = log;
	break;
      case SLMATH_LOG10:
	fun = log10;
	break;
      case SLMATH_SQRT:
	fun = sqrt;
	break;
      case SLMATH_SIN:
	fun = sin;
	break;
      case SLMATH_COS:
	fun = cos;
	break;

      case SLMATH_ASINH:
	fun = ASINH_FUN;
	break;
      case SLMATH_ATANH:
	fun = ATANH_FUN;
	break;
      case SLMATH_ACOSH:
	fun = ACOSH_FUN;
	break;

      case SLMATH_CONJ:
      case SLMATH_REAL:
	for (i = 0; i < na; i++)
	  b[i] = a[i];
	return 1;
      case SLMATH_IMAG:
	for (i = 0; i < na; i++)
	  b[i] = 0.0;
	return 1;
     }

   for (i = 0; i < na; i++)
     b[i] = (*fun) (a[i]);

   return 1;
}

static int float_math_op (int op,
			  unsigned char type, VOID_STAR ap, unsigned int na,
			  VOID_STAR bp)
{
   float *a, *b;
   unsigned int i;
   double (*fun) (double);

   (void) type;
   a = (float *) ap;
   b = (float *) bp;


   switch (op)
     {
      default:
	return 0;

      case SLMATH_SINH:
	fun = sinh;
	break;
      case SLMATH_COSH:
	fun = cosh;
	break;
      case SLMATH_TANH:
	fun = tanh;
	break;
      case SLMATH_TAN:
	fun = tan;
	break;
      case SLMATH_ASIN:
	fun = asin;
	break;
      case SLMATH_ACOS:
	fun = acos;
	break;
      case SLMATH_ATAN:
	fun = atan;
	break;
      case SLMATH_EXP:
	fun = exp;
	break;
      case SLMATH_LOG:
	fun = log;
	break;
      case SLMATH_LOG10:
	fun = log10;
	break;
      case SLMATH_SQRT:
	fun = sqrt;
	break;
      case SLMATH_SIN:
	fun = sin;
	break;
      case SLMATH_COS:
	fun = cos;
	break;

      case SLMATH_ASINH:
	fun = ASINH_FUN;
	break;
      case SLMATH_ATANH:
	fun = ATANH_FUN;
	break;
      case SLMATH_ACOSH:
	fun = ACOSH_FUN;
	break;

      case SLMATH_CONJ:
      case SLMATH_REAL:
	for (i = 0; i < na; i++)
	  b[i] = a[i];
	return 1;
      case SLMATH_IMAG:
	for (i = 0; i < na; i++)
	  b[i] = 0.0;
	return 1;
     }

   for (i = 0; i < na; i++)
     b[i] = (float) (*fun) ((double) a[i]);

   return 1;
}

static int generic_math_op (int op,
			    unsigned char type, VOID_STAR ap, unsigned int na,
			    VOID_STAR bp)
{
   double *b;
   unsigned int i;
   SLang_To_Double_Fun_Type to_double;
   double (*fun) (double);
   unsigned int da;
   char *a;

   if (NULL == (to_double = SLarith_get_to_double_fun (type, &da)))
     return 0;

   b = (double *) bp;
   a = (char *) ap;

   switch (op)
     {
      default:
	return 0;

      case SLMATH_SINH:
	fun = sinh;
	break;
      case SLMATH_COSH:
	fun = cosh;
	break;
      case SLMATH_TANH:
	fun = tanh;
	break;
      case SLMATH_TAN:
	fun = tan;
	break;
      case SLMATH_ASIN:
	fun = asin;
	break;
      case SLMATH_ACOS:
	fun = acos;
	break;
      case SLMATH_ATAN:
	fun = atan;
	break;
      case SLMATH_EXP:
	fun = exp;
	break;
      case SLMATH_LOG:
	fun = log;
	break;
      case SLMATH_LOG10:
	fun = log10;
	break;
      case SLMATH_SQRT:
	fun = sqrt;
	break;
      case SLMATH_SIN:
	fun = sin;
	break;
      case SLMATH_COS:
	fun = cos;
	break;

      case SLMATH_ASINH:
	fun = ASINH_FUN;
	break;
      case SLMATH_ATANH:
	fun = ATANH_FUN;
	break;
      case SLMATH_ACOSH:
	fun = ACOSH_FUN;
	break;


      case SLMATH_CONJ:
      case SLMATH_REAL:
	for (i = 0; i < na; i++)
	  {
	     b[i] = to_double((VOID_STAR) a);
	     a += da;
	  }
	return 1;

      case SLMATH_IMAG:
	for (i = 0; i < na; i++)
	  b[i] = 0.0;
	return 1;
     }

   for (i = 0; i < na; i++)
     {
	b[i] = (*fun) (to_double ((VOID_STAR) a));
	a += da;
     }
   
   return 1;
}

#if SLANG_HAS_COMPLEX
static int complex_math_op_result (int op, unsigned char a, unsigned char *b)
{
   (void) a;
   switch (op)
     {
      default:
	*b = SLANG_COMPLEX_TYPE;
	break;

      case SLMATH_REAL:
      case SLMATH_IMAG:
	*b = SLANG_DOUBLE_TYPE;
	break;
     }
   return 1;
}

static int complex_math_op (int op,
			    unsigned char type, VOID_STAR ap, unsigned int na,
			    VOID_STAR bp)
{
   double *a, *b;
   unsigned int i;
   unsigned int na2 = na * 2;
   double *(*fun) (double *, double *);

   (void) type;
   a = (double *) ap;
   b = (double *) bp;

   switch (op)
     {
      default:
	return 0;

      case SLMATH_REAL:
	for (i = 0; i < na; i++)
	  b[i] = a[2 * i];
	return 1;

      case SLMATH_IMAG:
	for (i = 0; i < na; i++)
	  b[i] = a[2 * i + 1];
	return 1;

      case SLMATH_CONJ:
	for (i = 0; i < na2; i += 2)
	  {
	     b[i] = a[i];
	     b[i+1] = -a[i+1];
	  }
	return 1;

      case SLMATH_ATANH:
	fun = SLcomplex_atanh;
	break;
      case SLMATH_ACOSH:
	fun = SLcomplex_acosh;
	break;
      case SLMATH_ASINH:
	fun = SLcomplex_asinh;
	break;
      case SLMATH_EXP:
	fun = SLcomplex_exp;
	break;
      case SLMATH_LOG:
	fun = SLcomplex_log;
	break;
      case SLMATH_LOG10:
	fun = SLcomplex_log10;
	break;
      case SLMATH_SQRT:
	fun = SLcomplex_sqrt;
	break;
      case SLMATH_SIN:
	fun = SLcomplex_sin;
	break;
      case SLMATH_COS:
	fun = SLcomplex_cos;
	break;
      case SLMATH_SINH:
	fun = SLcomplex_sinh;
	break;
      case SLMATH_COSH:
	fun = SLcomplex_cosh;
	break;
      case SLMATH_TANH:
	fun = SLcomplex_tanh;
	break;
      case SLMATH_TAN:
	fun = SLcomplex_tan;
	break;
      case SLMATH_ASIN:
	fun = SLcomplex_asin;
	break;
      case SLMATH_ACOS:
	fun = SLcomplex_acos;
	break;
      case SLMATH_ATAN:
	fun = SLcomplex_atan;
	break;
     }

   for (i = 0; i < na2; i += 2)
     (void) (*fun) (b + i, a + i);

   return 1;
}
#endif

static SLang_DConstant_Type DConst_Table [] =
{
   MAKE_DCONSTANT("E", 2.718281828459045),
   MAKE_DCONSTANT("PI", 3.14159265358979323846264338327950288),
   SLANG_END_DCONST_TABLE
};

static SLang_Math_Unary_Type SLmath_Table [] =
{
   MAKE_MATH_UNARY("sinh", SLMATH_SINH),
   MAKE_MATH_UNARY("asinh", SLMATH_ASINH),
   MAKE_MATH_UNARY("cosh", SLMATH_COSH),
   MAKE_MATH_UNARY("acosh", SLMATH_ACOSH),
   MAKE_MATH_UNARY("tanh", SLMATH_TANH),
   MAKE_MATH_UNARY("atanh", SLMATH_ATANH),
   MAKE_MATH_UNARY("sin", SLMATH_SIN),
   MAKE_MATH_UNARY("cos", SLMATH_COS),
   MAKE_MATH_UNARY("tan", SLMATH_TAN),
   MAKE_MATH_UNARY("atan", SLMATH_ATAN),
   MAKE_MATH_UNARY("acos", SLMATH_ACOS),
   MAKE_MATH_UNARY("asin", SLMATH_ASIN),
   MAKE_MATH_UNARY("exp", SLMATH_EXP),
   MAKE_MATH_UNARY("log", SLMATH_LOG),
   MAKE_MATH_UNARY("sqrt", SLMATH_SQRT),
   MAKE_MATH_UNARY("log10", SLMATH_LOG10),
#if SLANG_HAS_COMPLEX
   MAKE_MATH_UNARY("Real", SLMATH_REAL),
   MAKE_MATH_UNARY("Imag", SLMATH_IMAG),
   MAKE_MATH_UNARY("Conj", SLMATH_CONJ),
#endif
   SLANG_END_MATH_UNARY_TABLE
};

static SLang_Intrin_Fun_Type SLang_Math_Table [] =
{
   MAKE_INTRINSIC_0("polynom", math_poly, SLANG_DOUBLE_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

int SLang_init_slmath (void)
{
   unsigned char *int_types;

#if defined(__unix__)
   (void) SIGNAL (SIGFPE, math_floating_point_exception);
#endif

#if SLANG_HAS_COMPLEX
   if (-1 == _SLinit_slcomplex ())
     return -1;
#endif
   int_types = _SLarith_Arith_Types;

   while (*int_types != SLANG_FLOAT_TYPE)
     {
	if (-1 == SLclass_add_math_op (*int_types, generic_math_op, double_math_op_result))
	  return -1;
	int_types++;
     }

   if ((-1 == SLclass_add_math_op (SLANG_FLOAT_TYPE, float_math_op, double_math_op_result))
       || (-1 == SLclass_add_math_op (SLANG_DOUBLE_TYPE, double_math_op, double_math_op_result))
#if SLANG_HAS_COMPLEX
       || (-1 == SLclass_add_math_op (SLANG_COMPLEX_TYPE, complex_math_op, complex_math_op_result))
#endif
       )
     return -1;

   if ((-1 == SLadd_math_unary_table (SLmath_Table, "__SLMATH__"))
       || (-1 == SLadd_intrin_fun_table (SLang_Math_Table, NULL))
       || (-1 == SLadd_dconstant_table (DConst_Table, NULL)))
     return -1;

   return 0;
}
#endif				       /* SLANG_HAS_FLOAT */
