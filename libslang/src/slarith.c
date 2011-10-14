/* Copyright (c) 1998, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include <math.h>

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#include "slang.h"
#include "_slang.h"

/*
 * This file defines binary and unary operations on all integer types.
 * Supported types include:
 *
 *    SLANG_CHAR_TYPE     (char)
 *    SLANG_SHORT_TYPE    (short)
 *    SLANG_INT_TYPE      (int)
 *    SLANG_LONG_TYPE     (long)
 *    SLANG_FLOAT_TYPE    (float)
 *    SLANG_DOUBLE_TYPE   (double)
 *
 * as well as unsigned types.  The result-type of an arithmentic operation
 * will depend upon the data types involved.  I am going to distinguish
 * between the boolean operations such as `and' and `or' from the arithmetic
 * operations such as `plus'.  Since the result of a boolean operation is
 * either 1 or 0, a boolean result will be represented by SLANG_CHAR_TYPE.
 * Ordinarily I would use an integer but for arrays it makes more sense to
 * use a character data type.
 *
 * So, the following will be assumed (`+' is any arithmetic operator)
 *
 *    char + char = int
 *    char|short + short = int
 *    char|short|int + int = int
 *    char|short|int|long + long = long
 *    char|short|int|long|float + float = float
 *    char|short|int|long|float|double + double = double
 *
 * In the actual implementation, a brute force approach is avoided.  Such
 * an approach would mean defining different functions for all possible
 * combinations of types.  Including the unsigned types, and not including
 * the complex number type, there are 10 arithmetic types and 10*10=100
 * different combinations of types.  Clearly this would be too much.
 *
 * One approach would be to define binary functions only between operands of
 * the same type and then convert types as appropriate.  This would require
 * just 6 such functions (int, uint, long, ulong, float, double).
 * However, many conversion functions are going to be required, particularly
 * since we are going to allow typecasting from one arithmetic to another.
 * Since the bit pattern of signed and unsigned types are the same, and only
 * the interpretation differs, there will be no functions to convert between
 * signed and unsigned forms of a given type.
 */

#define MAX_ARITHMETIC_TYPES	10

unsigned char _SLarith_Arith_Types[] =
{
   SLANG_CHAR_TYPE,
   SLANG_UCHAR_TYPE,
   SLANG_SHORT_TYPE,
   SLANG_USHORT_TYPE,
   SLANG_INT_TYPE,
   SLANG_UINT_TYPE,
   SLANG_LONG_TYPE,
   SLANG_ULONG_TYPE,
   SLANG_FLOAT_TYPE,
   SLANG_DOUBLE_TYPE,
   0
};

/* Here are a bunch of functions to convert from one type to another.  To
 * facilitate the process, a macros will be used.
 */

#define DEFUN_1(f,from_type,to_type) \
static void f (to_type *y, from_type *x, unsigned int n) \
{ \
   unsigned int i; \
   for (i = 0; i < n; i++) y[i] = (to_type) x[i]; \
}

#define DEFUN_2(f,from_type,to_type,copy_fun) \
static VOID_STAR f (VOID_STAR xp, unsigned int n) \
{ \
   from_type *x; \
   to_type *y; \
   x = (from_type *) xp; \
   if (NULL == (y = (to_type *) SLmalloc (sizeof (to_type) * n))) return NULL; \
   copy_fun (y, x, n); \
   return (VOID_STAR) y; \
}
typedef VOID_STAR (*Convert_Fun_Type)(VOID_STAR, unsigned int);

DEFUN_1(copy_char_to_char,char,char)
#if SIZEOF_INT != SIZEOF_SHORT
DEFUN_1(copy_char_to_short,char,short)
DEFUN_1(copy_char_to_ushort,char,unsigned short)
#else
# define copy_char_to_short	copy_char_to_int
# define copy_char_to_ushort	copy_char_to_uint
#endif
DEFUN_1(copy_char_to_int,char,int)
DEFUN_1(copy_char_to_uint,char,unsigned int)
#if SIZEOF_INT != SIZEOF_LONG
DEFUN_1(copy_char_to_long,char,long)
DEFUN_1(copy_char_to_ulong,char,unsigned long)
#else
# define copy_char_to_long	copy_char_to_int
# define copy_char_to_ulong	copy_char_to_uint
#endif
#if SLANG_HAS_FLOAT
DEFUN_1(copy_char_to_float,char,float)
DEFUN_1(copy_char_to_double,char,double)
#endif

#if SIZEOF_INT != SIZEOF_SHORT
DEFUN_1(copy_uchar_to_short,unsigned char,short)
DEFUN_1(copy_uchar_to_ushort,unsigned char,unsigned short)
#else
# define copy_uchar_to_short	copy_uchar_to_int
# define copy_uchar_to_ushort	copy_uchar_to_uint
#endif
DEFUN_1(copy_uchar_to_int,unsigned char,int)
DEFUN_1(copy_uchar_to_uint,unsigned char,unsigned int)
#if SIZEOF_INT != SIZEOF_LONG
DEFUN_1(copy_uchar_to_long,unsigned char,long)
DEFUN_1(copy_uchar_to_ulong,unsigned char,unsigned long)
#else
# define copy_uchar_to_long	copy_uchar_to_int
# define copy_uchar_to_ulong	copy_uchar_to_uint
#endif
#if SLANG_HAS_FLOAT
DEFUN_1(copy_uchar_to_float,unsigned char,float)
DEFUN_1(copy_uchar_to_double,unsigned char,double)
#endif

#if SIZEOF_INT != SIZEOF_SHORT
DEFUN_1(copy_short_to_char,short,char)
DEFUN_1(copy_short_to_uchar,short,unsigned char)
DEFUN_1(copy_short_to_short,short,short)
DEFUN_1(copy_short_to_int,short,int)
DEFUN_1(copy_short_to_uint,short,unsigned int)
DEFUN_1(copy_short_to_long,short,long)
DEFUN_1(copy_short_to_ulong,short,unsigned long)
#if SLANG_HAS_FLOAT
DEFUN_1(copy_short_to_float,short,float)
DEFUN_1(copy_short_to_double,short,double)
#endif
DEFUN_1(copy_ushort_to_char,unsigned short,char)
DEFUN_1(copy_ushort_to_uchar,unsigned short,unsigned char)
DEFUN_1(copy_ushort_to_int,unsigned short,int)
DEFUN_1(copy_ushort_to_uint,unsigned short,unsigned int)
DEFUN_1(copy_ushort_to_long,unsigned short,long)
DEFUN_1(copy_ushort_to_ulong,unsigned short,unsigned long)
#if SLANG_HAS_FLOAT
DEFUN_1(copy_ushort_to_float,unsigned short,float)
DEFUN_1(copy_ushort_to_double,unsigned short,double)
#endif
#else
# define copy_short_to_char	copy_int_to_char
# define copy_short_to_uchar	copy_int_to_uchar
# define copy_short_to_short	copy_int_to_int
# define copy_short_to_int	copy_int_to_int
# define copy_short_to_uint	copy_int_to_int
# define copy_short_to_long	copy_int_to_long
# define copy_short_to_ulong	copy_int_to_ulong
#if SLANG_HAS_FLOAT
# define copy_short_to_float	copy_int_to_float
# define copy_short_to_double	copy_int_to_double
#endif
# define copy_ushort_to_char	copy_uint_to_char
# define copy_ushort_to_uchar	copy_uint_to_uchar
# define copy_ushort_to_int	copy_int_to_int
# define copy_ushort_to_uint	copy_int_to_int
# define copy_ushort_to_long	copy_uint_to_long
# define copy_ushort_to_ulong	copy_uint_to_ulong
#if SLANG_HAS_FLOAT
# define copy_ushort_to_float	copy_uint_to_float
# define copy_ushort_to_double	copy_uint_to_double
#endif
#endif

DEFUN_1(copy_int_to_char,int,char)
DEFUN_1(copy_int_to_uchar,int,unsigned char)
DEFUN_1(copy_uint_to_char,unsigned int,char)
DEFUN_1(copy_uint_to_uchar,unsigned int,unsigned char)
#if SIZEOF_INT != SIZEOF_SHORT
DEFUN_1(copy_int_to_short,int,short)
DEFUN_1(copy_int_to_ushort,int,unsigned short)
DEFUN_1(copy_uint_to_short,unsigned int,short)
DEFUN_1(copy_uint_to_ushort,unsigned int,unsigned short)
#else
# define copy_int_to_short	copy_int_to_int
# define copy_int_to_ushort	copy_int_to_int
# define copy_uint_to_short	copy_int_to_int
# define copy_uint_to_ushort	copy_int_to_int
#endif
DEFUN_1(copy_int_to_int,int,int)
#if SIZEOF_INT != SIZEOF_LONG
DEFUN_1(copy_int_to_long,int,long)
DEFUN_1(copy_int_to_ulong,int,unsigned long)
DEFUN_1(copy_uint_to_long,unsigned int,long)
DEFUN_1(copy_uint_to_ulong,unsigned int,unsigned long)
#else
# define copy_int_to_long	copy_int_to_int
# define copy_int_to_ulong	copy_int_to_int
# define copy_uint_to_long	copy_int_to_int
# define copy_uint_to_ulong	copy_int_to_int
#endif
#if SLANG_HAS_FLOAT
DEFUN_1(copy_int_to_float,int,float)
DEFUN_1(copy_int_to_double,int,double)
DEFUN_1(copy_uint_to_float,unsigned int,float)
DEFUN_1(copy_uint_to_double,unsigned int,double)
#endif

#if SIZEOF_INT != SIZEOF_LONG
DEFUN_1(copy_long_to_char,long,char)
DEFUN_1(copy_long_to_uchar,long,unsigned char)
DEFUN_1(copy_long_to_short,long,short)
DEFUN_1(copy_long_to_ushort,long,unsigned short)
DEFUN_1(copy_long_to_int,long,int)
DEFUN_1(copy_long_to_uint,long,unsigned int)
DEFUN_1(copy_long_to_long,long,long)
DEFUN_1(copy_ulong_to_char,unsigned long,char)
DEFUN_1(copy_ulong_to_uchar,unsigned long,unsigned char)
DEFUN_1(copy_ulong_to_short,unsigned long,short)
DEFUN_1(copy_ulong_to_ushort,unsigned long,unsigned short)
DEFUN_1(copy_ulong_to_int,unsigned long,int)
DEFUN_1(copy_ulong_to_uint,unsigned long,unsigned int)
#if SLANG_HAS_FLOAT
DEFUN_1(copy_long_to_float,long,float)
DEFUN_1(copy_long_to_double,long,double)
DEFUN_1(copy_ulong_to_float,unsigned long,float)
DEFUN_1(copy_ulong_to_double,unsigned long,double)
#endif
#else
#define copy_long_to_char	copy_int_to_char
#define copy_long_to_uchar	copy_int_to_uchar
#define copy_long_to_short	copy_int_to_short
#define copy_long_to_ushort	copy_int_to_ushort
#define copy_long_to_int	copy_int_to_int
#define copy_long_to_uint	copy_int_to_int
#define copy_long_to_long	copy_int_to_int
#define copy_long_to_float	copy_int_to_float
#define copy_long_to_double	copy_int_to_double
#define copy_ulong_to_char	copy_uint_to_char
#define copy_ulong_to_uchar	copy_uint_to_uchar
#define copy_ulong_to_short	copy_uint_to_short
#define copy_ulong_to_ushort	copy_uint_to_ushort
#define copy_ulong_to_int	copy_int_to_int
#define copy_ulong_to_uint	copy_int_to_int
#define copy_ulong_to_float	copy_uint_to_float
#define copy_ulong_to_double	copy_uint_to_double
#endif

#if SLANG_HAS_FLOAT
DEFUN_1(copy_float_to_char,float,char)
DEFUN_1(copy_float_to_uchar,float,unsigned char)
#if SIZEOF_INT != SIZEOF_SHORT
DEFUN_1(copy_float_to_short,float,short)
DEFUN_1(copy_float_to_ushort,float,unsigned short)
#else
# define copy_float_to_short	copy_float_to_int
# define copy_float_to_ushort	copy_float_to_uint
#endif
DEFUN_1(copy_float_to_int,float,int)
DEFUN_1(copy_float_to_uint,float,unsigned int)
#if SIZEOF_INT != SIZEOF_LONG
DEFUN_1(copy_float_to_long,float,long)
DEFUN_1(copy_float_to_ulong,float,unsigned long)
#else
# define copy_float_to_long	copy_float_to_int
# define copy_float_to_ulong	copy_float_to_uint
#endif

DEFUN_1(copy_float_to_float,float,float)

static void copy_float_to_double (double *y, float *x, unsigned int n) 
{ 
   unsigned int i;
   for (i = 0; i < n; i++) 
     y[i] = (double) x[i]; 
}


/* DEFUN_1(copy_float_to_double,float,double) */

DEFUN_1(copy_double_to_char,double,char)
DEFUN_1(copy_double_to_uchar,double,unsigned char)
#if SIZEOF_INT != SIZEOF_SHORT
DEFUN_1(copy_double_to_short,double,short)
DEFUN_1(copy_double_to_ushort,double,unsigned short)
#else
# define copy_double_to_short	copy_double_to_int
# define copy_double_to_ushort	copy_double_to_uint
#endif
DEFUN_1(copy_double_to_int,double,int)
DEFUN_1(copy_double_to_uint,double,unsigned int)
#if SIZEOF_INT != SIZEOF_LONG
DEFUN_1(copy_double_to_long,double,long)
DEFUN_1(copy_double_to_ulong,double,unsigned long)
#else
# define copy_double_to_long	copy_double_to_int
# define copy_double_to_ulong	copy_double_to_uint
#endif
DEFUN_1(copy_double_to_float,double,float)
DEFUN_1(copy_double_to_double,double,double)
#endif				       /* SLANG_HAS_FLOAT */
		    
DEFUN_2(char_to_int,char,int,copy_char_to_int)
DEFUN_2(char_to_uint,char,unsigned int,copy_char_to_uint)
#if SIZEOF_INT != SIZEOF_LONG
DEFUN_2(char_to_long,char,long,copy_char_to_long)
DEFUN_2(char_to_ulong,char,unsigned long,copy_char_to_ulong)
#else
# define char_to_long	char_to_int
# define char_to_ulong	char_to_uint
#endif
#if SLANG_HAS_FLOAT
DEFUN_2(char_to_float,char,float,copy_char_to_float)
DEFUN_2(char_to_double,char,double,copy_char_to_double)
#endif

DEFUN_2(uchar_to_int,unsigned char,int,copy_uchar_to_int)
DEFUN_2(uchar_to_uint,unsigned char,unsigned int,copy_uchar_to_uint)
#if SIZEOF_INT != SIZEOF_LONG
DEFUN_2(uchar_to_long,unsigned char,long,copy_uchar_to_long)
DEFUN_2(uchar_to_ulong,unsigned char,unsigned long,copy_uchar_to_ulong)
#else
# define uchar_to_long		uchar_to_int
# define uchar_to_ulong		uchar_to_uint
#endif
#if SLANG_HAS_FLOAT
DEFUN_2(uchar_to_float,unsigned char,float,copy_uchar_to_float)
DEFUN_2(uchar_to_double,unsigned char,double,copy_uchar_to_double)
#endif

#if SIZEOF_INT != SIZEOF_SHORT
DEFUN_2(short_to_int,short,int,copy_short_to_int)
DEFUN_2(short_to_uint,short,unsigned int,copy_short_to_uint)
DEFUN_2(short_to_long,short,long,copy_short_to_long)
DEFUN_2(short_to_ulong,short,unsigned long,copy_short_to_ulong)
#if SLANG_HAS_FLOAT
DEFUN_2(short_to_float,short,float,copy_short_to_float)
DEFUN_2(short_to_double,short,double,copy_short_to_double)
#endif
DEFUN_2(ushort_to_int,unsigned short,int,copy_ushort_to_int)
DEFUN_2(ushort_to_uint,unsigned short,unsigned int,copy_ushort_to_uint)
DEFUN_2(ushort_to_long,unsigned short,long,copy_ushort_to_long)
DEFUN_2(ushort_to_ulong,unsigned short,unsigned long,copy_ushort_to_ulong)
#if SLANG_HAS_FLOAT
DEFUN_2(ushort_to_float,unsigned short,float,copy_ushort_to_float)
DEFUN_2(ushort_to_double,unsigned short,double,copy_ushort_to_double)
#endif
#else
# define short_to_int		NULL
# define short_to_uint		NULL
# define short_to_long		int_to_long
# define short_to_ulong		int_to_ulong
#if SLANG_HAS_FLOAT
# define short_to_float		int_to_float
# define short_to_double	int_to_double
#endif
# define ushort_to_int		NULL
# define ushort_to_uint		NULL
# define ushort_to_long		uint_to_long
# define ushort_to_ulong	uint_to_ulong
#if SLANG_HAS_FLOAT
# define ushort_to_float	uint_to_float
# define ushort_to_double	uint_to_double
#endif
#endif

#if SIZEOF_INT != SIZEOF_LONG
DEFUN_2(int_to_long,int,long,copy_int_to_long)
DEFUN_2(int_to_ulong,int,unsigned long,copy_int_to_ulong)
#else
# define int_to_long		NULL
# define int_to_ulong		NULL
#endif
#if SLANG_HAS_FLOAT
DEFUN_2(int_to_float,int,float,copy_int_to_float)
DEFUN_2(int_to_double,int,double,copy_int_to_double)
#endif

#if SIZEOF_INT != SIZEOF_LONG
DEFUN_2(uint_to_long,unsigned int,long,copy_uint_to_long)
DEFUN_2(uint_to_ulong,unsigned int,unsigned long,copy_uint_to_ulong)
#else
# define uint_to_long		NULL
# define uint_to_ulong		NULL
#endif
#if SLANG_HAS_FLOAT
DEFUN_2(uint_to_float,unsigned int,float,copy_uint_to_float)
DEFUN_2(uint_to_double,unsigned int,double,copy_uint_to_double)

#if SIZEOF_INT != SIZEOF_LONG
DEFUN_2(long_to_float,long,float,copy_long_to_float)
DEFUN_2(long_to_double,long,double,copy_long_to_double)
DEFUN_2(ulong_to_float,unsigned long,float,copy_ulong_to_float)
DEFUN_2(ulong_to_double,unsigned long,double,copy_ulong_to_double)
#else
# define long_to_float		int_to_float
# define long_to_double		int_to_double
# define ulong_to_float		uint_to_float
# define ulong_to_double	uint_to_double
#endif

DEFUN_2(float_to_double,float,double,copy_float_to_double)

#define TO_DOUBLE_FUN(name,type) \
static double name (VOID_STAR x) { return (double) *(type *) x; }
TO_DOUBLE_FUN(char_to_one_double,char)
TO_DOUBLE_FUN(uchar_to_one_double,unsigned char)
#if SIZEOF_INT != SIZEOF_SHORT
TO_DOUBLE_FUN(short_to_one_double,short)
TO_DOUBLE_FUN(ushort_to_one_double,unsigned short)
#else
# define short_to_one_double	int_to_one_double
# define ushort_to_one_double	uint_to_one_double
#endif
TO_DOUBLE_FUN(int_to_one_double,int)
TO_DOUBLE_FUN(uint_to_one_double,unsigned int)
#if SIZEOF_INT != SIZEOF_LONG
TO_DOUBLE_FUN(long_to_one_double,long)
TO_DOUBLE_FUN(ulong_to_one_double,unsigned long)
#else
# define long_to_one_double	int_to_one_double
# define ulong_to_one_double	uint_to_one_double
#endif
TO_DOUBLE_FUN(float_to_one_double,float)
TO_DOUBLE_FUN(double_to_one_double,double)

SLang_To_Double_Fun_Type
SLarith_get_to_double_fun (unsigned char type, unsigned int *sizeof_type)
{
   unsigned int da;
   SLang_To_Double_Fun_Type to_double;

   switch (type)
     {
      default:
	return NULL;

      case SLANG_CHAR_TYPE:
	da = sizeof (char); to_double = char_to_one_double;
	break;
      case SLANG_UCHAR_TYPE:
	da = sizeof (unsigned char); to_double = uchar_to_one_double;
	break;
      case SLANG_SHORT_TYPE:
	da = sizeof (short); to_double = short_to_one_double;
	break;
      case SLANG_USHORT_TYPE:
	da = sizeof (unsigned short); to_double = ushort_to_one_double;
	break;
      case SLANG_INT_TYPE:
	da = sizeof (int); to_double = int_to_one_double;
	break;
      case SLANG_UINT_TYPE:
	da = sizeof (unsigned int); to_double = uint_to_one_double;
	break;
      case SLANG_LONG_TYPE:
	da = sizeof (long); to_double = long_to_one_double;
	break;
      case SLANG_ULONG_TYPE:
	da = sizeof (unsigned long); to_double = ulong_to_one_double;
	break;
      case SLANG_FLOAT_TYPE:
	da = sizeof (float); to_double = float_to_one_double;
	break;
     case SLANG_DOUBLE_TYPE:
	da = sizeof (double); to_double = double_to_one_double;
	break;
     }

   if (sizeof_type != NULL) *sizeof_type = da;
   return to_double;
}
#endif				       /* SLANG_HAS_FLOAT */
/* Each element of the matrix determines how the row maps onto the column.
 * That is, let the matrix be B_ij.  Where the i,j indices refer to
 * precedence of the type.  Then,
 * B_ij->copy_function copies type i to type j.  Similarly,
 * B_ij->convert_function mallocs a new array of type j and copies i to it.
 *
 * Since types are always converted to higher levels of precedence for binary
 * operations, many of the elements are NULL.
 *
 * Is the idea clear?
 */
typedef struct
{
   FVOID_STAR copy_function;
   Convert_Fun_Type convert_function;
}
Binary_Matrix_Type;

static Binary_Matrix_Type Binary_Matrix [MAX_ARITHMETIC_TYPES][MAX_ARITHMETIC_TYPES] =
{
     {
	  {(FVOID_STAR)copy_char_to_char, NULL},
	  {(FVOID_STAR)copy_char_to_char, NULL},
	{(FVOID_STAR) copy_char_to_short, NULL},
	{(FVOID_STAR) copy_char_to_ushort, NULL},
	{(FVOID_STAR) copy_char_to_int, char_to_int},
	{(FVOID_STAR) copy_char_to_uint, char_to_uint},
	{(FVOID_STAR) copy_char_to_long, char_to_long},
	{(FVOID_STAR) copy_char_to_ulong, char_to_ulong},
#if SLANG_HAS_FLOAT
	{(FVOID_STAR) copy_char_to_float, char_to_float},
	{(FVOID_STAR) copy_char_to_double, char_to_double},
#endif
     },

     {
	  {(FVOID_STAR)copy_char_to_char, NULL},
	  {(FVOID_STAR)copy_char_to_char, NULL},
	{(FVOID_STAR) copy_uchar_to_short, NULL},
	{(FVOID_STAR) copy_uchar_to_ushort, NULL},
	{(FVOID_STAR) copy_uchar_to_int, uchar_to_int},
	{(FVOID_STAR) copy_uchar_to_uint, uchar_to_uint},
	{(FVOID_STAR) copy_uchar_to_long, uchar_to_long},
	{(FVOID_STAR) copy_uchar_to_ulong, uchar_to_ulong},
#if SLANG_HAS_FLOAT
	{(FVOID_STAR) copy_uchar_to_float, uchar_to_float},
	{(FVOID_STAR) copy_uchar_to_double, uchar_to_double},
#endif
     },

     {
	{(FVOID_STAR) copy_short_to_char, NULL},
	{(FVOID_STAR) copy_short_to_uchar, NULL},
	{(FVOID_STAR) copy_short_to_short, NULL},
	{(FVOID_STAR) copy_short_to_short, NULL},
	{(FVOID_STAR) copy_short_to_int, short_to_int},
	{(FVOID_STAR) copy_short_to_uint, short_to_uint},
	{(FVOID_STAR) copy_short_to_long, short_to_long},
	{(FVOID_STAR) copy_short_to_ulong, short_to_ulong},
#if SLANG_HAS_FLOAT
	{(FVOID_STAR) copy_short_to_float, short_to_float},
	{(FVOID_STAR) copy_short_to_double, short_to_double},
#endif
     },

     {
	{(FVOID_STAR) copy_ushort_to_char, NULL},
	{(FVOID_STAR) copy_ushort_to_uchar, NULL},
	{(FVOID_STAR) copy_short_to_short, NULL},
	{(FVOID_STAR) copy_short_to_short, NULL},
	{(FVOID_STAR) copy_ushort_to_int, ushort_to_int},
	{(FVOID_STAR) copy_ushort_to_uint, ushort_to_uint},
	{(FVOID_STAR) copy_ushort_to_long, ushort_to_long},
	{(FVOID_STAR) copy_ushort_to_ulong, ushort_to_ulong},
#if SLANG_HAS_FLOAT
	{(FVOID_STAR) copy_ushort_to_float, ushort_to_float},
	{(FVOID_STAR) copy_ushort_to_double, ushort_to_double},
#endif
     },

     {
	{(FVOID_STAR) copy_int_to_char, NULL},
	{(FVOID_STAR) copy_int_to_uchar, NULL},
	{(FVOID_STAR) copy_int_to_short, NULL},
	{(FVOID_STAR) copy_int_to_ushort, NULL},
	{(FVOID_STAR) copy_int_to_int, NULL},
	{(FVOID_STAR) copy_int_to_int, NULL},
	{(FVOID_STAR) copy_int_to_long, int_to_long},
	{(FVOID_STAR) copy_int_to_ulong, int_to_ulong},
#if SLANG_HAS_FLOAT
	{(FVOID_STAR) copy_int_to_float, int_to_float},
	{(FVOID_STAR) copy_int_to_double, int_to_double},
#endif
     },

     {
	{(FVOID_STAR) copy_uint_to_char, NULL},
	{(FVOID_STAR) copy_uint_to_uchar, NULL},
	{(FVOID_STAR) copy_uint_to_short, NULL},
	{(FVOID_STAR) copy_uint_to_ushort, NULL},
	{(FVOID_STAR) copy_int_to_int, NULL},
	{(FVOID_STAR) copy_int_to_int, NULL},
	{(FVOID_STAR) copy_uint_to_long, uint_to_long},
	{(FVOID_STAR) copy_uint_to_ulong, uint_to_ulong},
#if SLANG_HAS_FLOAT
	{(FVOID_STAR) copy_uint_to_float, uint_to_float},
	{(FVOID_STAR) copy_uint_to_double, uint_to_double},
#endif
     },

     {
	{(FVOID_STAR) copy_long_to_char, NULL},
	{(FVOID_STAR) copy_long_to_uchar, NULL},
	{(FVOID_STAR) copy_long_to_short, NULL},
	{(FVOID_STAR) copy_long_to_ushort, NULL},
	{(FVOID_STAR) copy_long_to_int, NULL},
	{(FVOID_STAR) copy_long_to_uint, NULL},
	{(FVOID_STAR) copy_long_to_long, NULL},
	{(FVOID_STAR) copy_long_to_long, NULL},
#if SLANG_HAS_FLOAT
	{(FVOID_STAR) copy_long_to_float, long_to_float},
	{(FVOID_STAR) copy_long_to_double, long_to_double},
#endif
     },

     {
	{(FVOID_STAR) copy_ulong_to_char, NULL},
	{(FVOID_STAR) copy_ulong_to_uchar, NULL},
	{(FVOID_STAR) copy_ulong_to_short, NULL},
	{(FVOID_STAR) copy_ulong_to_ushort, NULL},
	{(FVOID_STAR) copy_ulong_to_int, NULL},
	{(FVOID_STAR) copy_ulong_to_uint, NULL},
	{(FVOID_STAR) copy_long_to_long, NULL},
	{(FVOID_STAR) copy_long_to_long, NULL},
#if SLANG_HAS_FLOAT
	{(FVOID_STAR) copy_ulong_to_float, ulong_to_float},
	{(FVOID_STAR) copy_ulong_to_double, ulong_to_double},
#endif
     },

#if SLANG_HAS_FLOAT
     {
	{(FVOID_STAR) copy_float_to_char, NULL},
	{(FVOID_STAR) copy_float_to_uchar, NULL},
	{(FVOID_STAR) copy_float_to_short, NULL},
	{(FVOID_STAR) copy_float_to_ushort, NULL},
	{(FVOID_STAR) copy_float_to_int, NULL},
	{(FVOID_STAR) copy_float_to_uint, NULL},
	{(FVOID_STAR) copy_float_to_long, NULL},
	{(FVOID_STAR) copy_float_to_ulong, NULL},
	{(FVOID_STAR) copy_float_to_float, NULL},
	{(FVOID_STAR) copy_float_to_double, float_to_double},
     },

     {
	{(FVOID_STAR) copy_double_to_char, NULL},
	{(FVOID_STAR) copy_double_to_uchar, NULL},
	{(FVOID_STAR) copy_double_to_short, NULL},
	{(FVOID_STAR) copy_double_to_ushort, NULL},
	{(FVOID_STAR) copy_double_to_int, NULL},
	{(FVOID_STAR) copy_double_to_uint, NULL},
	{(FVOID_STAR) copy_double_to_long, NULL},
	{(FVOID_STAR) copy_double_to_ulong, NULL},
	{(FVOID_STAR) copy_double_to_float, NULL},
	{(FVOID_STAR) copy_double_to_double, NULL},
     }
#endif
};

#define GENERIC_BINARY_FUNCTION int_int_bin_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE int
#define POW_FUNCTION(a,b) pow((double)(a),(double)(b))
#define POW_RESULT_TYPE double
#define ABS_FUNCTION abs
#define MOD_FUNCTION(a,b) ((a) % (b))
#define GENERIC_UNARY_FUNCTION int_unary_op
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#if _SLANG_OPTIMIZE_FOR_SPEED
# define SCALAR_BINARY_FUNCTION int_int_scalar_bin_op
#endif
#define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_int_obj(SLANG_INT_TYPE,(x))
#define PUSH_POW_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE, (x))
#define CMP_FUNCTION int_cmp_function
#include "slarith.inc"

#define GENERIC_BINARY_FUNCTION uint_uint_bin_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE unsigned int
#define POW_FUNCTION(a,b) pow((double)(a),(double)(b))
#define POW_RESULT_TYPE double
#define MOD_FUNCTION(a,b) ((a) % (b))
#define GENERIC_UNARY_FUNCTION uint_unary_op
#define ABS_FUNCTION(a) (a)
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : 0)
#if _SLANG_OPTIMIZE_FOR_SPEED
# define SCALAR_BINARY_FUNCTION uint_uint_scalar_bin_op
#endif
#define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_int_obj(SLANG_UINT_TYPE,(int)(x))
#define PUSH_POW_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE, (x))
#define CMP_FUNCTION uint_cmp_function
#include "slarith.inc"

#if SIZEOF_LONG != SIZEOF_INT
#define GENERIC_BINARY_FUNCTION long_long_bin_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE long
#define POW_FUNCTION(a,b) pow((double)(a),(double)(b))
#define POW_RESULT_TYPE double
#define MOD_FUNCTION(a,b) ((a) % (b))
#define GENERIC_UNARY_FUNCTION long_unary_op
#define ABS_FUNCTION(a) (((a) >= 0) ? (a) : -(a))
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#if _SLANG_OPTIMIZE_FOR_SPEED
# define SCALAR_BINARY_FUNCTION long_long_scalar_bin_op
#endif
#define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_long_obj(SLANG_LONG_TYPE,(x))
#define PUSH_POW_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE, (x))
#define CMP_FUNCTION long_cmp_function
#include "slarith.inc"

#define GENERIC_BINARY_FUNCTION ulong_ulong_bin_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE unsigned long
#define POW_FUNCTION(a,b) pow((double)(a),(double)(b))
#define POW_RESULT_TYPE double
#define MOD_FUNCTION(a,b) ((a) % (b))
#define GENERIC_UNARY_FUNCTION ulong_unary_op
#define ABS_FUNCTION(a) (a)
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : 0)
#if _SLANG_OPTIMIZE_FOR_SPEED
# define SCALAR_BINARY_FUNCTION ulong_ulong_scalar_bin_op
#endif
#define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_long_obj(SLANG_ULONG_TYPE,(long)(x))
#define PUSH_POW_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE, (x))
#define CMP_FUNCTION ulong_cmp_function
#include "slarith.inc"
#else
#define long_long_bin_op	int_int_bin_op
#define ulong_ulong_bin_op	uint_uint_bin_op
#define long_unary_op		int_unary_op
#define ulong_unary_op		uint_unary_op
#define long_cmp_function	int_cmp_function
#define ulong_cmp_function	uint_cmp_function
#endif				       /* SIZEOF_INT != SIZEOF_LONG */

#if SLANG_HAS_FLOAT
#define GENERIC_BINARY_FUNCTION float_float_bin_op
#define GENERIC_TYPE float
#define POW_FUNCTION(a,b) (float)pow((double)(a),(double)(b))
#define POW_RESULT_TYPE float
#define MOD_FUNCTION(a,b) (float)fmod((a),(b))
#define GENERIC_UNARY_FUNCTION float_unary_op
#define ABS_FUNCTION(a) (float)fabs((double) a)
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#if _SLANG_OPTIMIZE_FOR_SPEED
# define SCALAR_BINARY_FUNCTION float_float_scalar_bin_op
#endif
#define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_float_obj(SLANG_FLOAT_TYPE,(x))
#define PUSH_POW_OBJ_FUN(x) SLclass_push_float_obj(SLANG_FLOAT_TYPE, (x))
#define CMP_FUNCTION float_cmp_function
#include "slarith.inc"

#define GENERIC_BINARY_FUNCTION double_double_bin_op
#define GENERIC_TYPE double
#define POW_FUNCTION(a,b) pow((double)(a),(double)(b))
#define POW_RESULT_TYPE double
#define MOD_FUNCTION(a,b) (float)fmod((a),(b))
#define GENERIC_UNARY_FUNCTION double_unary_op
#define ABS_FUNCTION(a) fabs(a)
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#if _SLANG_OPTIMIZE_FOR_SPEED
# define SCALAR_BINARY_FUNCTION double_double_scalar_bin_op
#endif
#define PUSH_SCALAR_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE,(x))
#define PUSH_POW_OBJ_FUN(x) SLclass_push_double_obj(SLANG_DOUBLE_TYPE, (x))
#define CMP_FUNCTION double_cmp_function
#include "slarith.inc"
#endif				       /* SLANG_HAS_FLOAT */

#define GENERIC_UNARY_FUNCTION char_unary_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE signed char
#define ABS_FUNCTION abs
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#define CMP_FUNCTION char_cmp_function
#include "slarith.inc"

#define GENERIC_UNARY_FUNCTION uchar_unary_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE unsigned char
#define ABS_FUNCTION(x) (x)
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : 0)
#define CMP_FUNCTION uchar_cmp_function
#include "slarith.inc"

#if SIZEOF_SHORT != SIZEOF_INT
#define GENERIC_UNARY_FUNCTION short_unary_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE short
#define ABS_FUNCTION abs
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))
#define CMP_FUNCTION short_cmp_function
#include "slarith.inc"

#define GENERIC_UNARY_FUNCTION ushort_unary_op
#define GENERIC_BIT_OPERATIONS
#define GENERIC_TYPE unsigned short
#define ABS_FUNCTION(x) (x)
#define SIGN_FUNCTION(x) (((x) > 0) ? 1 : 0)
#define CMP_FUNCTION ushort_cmp_function
#include "slarith.inc"
#endif				       /* SIZEOF_INT != SIZEOF_SHORT */

/* Unfortunately, the numbers that were assigned to the data-types were
 * not well thought out.  So, I need to use the following table.
 */
#define MAXIMUM_ARITH_TYPE_VALUE	SLANG_FLOAT_TYPE

#define IS_INTEGER_TYPE(x) \
  (((x) <= MAXIMUM_ARITH_TYPE_VALUE) \
      && (Type_Precedence_Table[x] < 8) && (Type_Precedence_Table[x] != -1))
#define IS_ARITHMETIC_TYPE(x) \
  (((x) <= MAXIMUM_ARITH_TYPE_VALUE) && (Type_Precedence_Table[x] != -1))

#define LONG_PRECEDENCE_VALUE	6
#define FLOAT_PRECEDENCE_VALUE	8

static signed char Type_Precedence_Table [MAXIMUM_ARITH_TYPE_VALUE + 1] =
{
   -1,				       /* SLANG_UNDEFINED_TYPE */
   -1,				       /* SLANG_VOID_TYPE */
   4,				       /* SLANG_INT_TYPE */
#if SLANG_HAS_FLOAT
   9,				       /* SLANG_DOUBLE_TYPE */
#else
   -1,
#endif
   0,				       /* SLANG_CHAR_TYPE */
   -1,				       /* SLANG_INTP_TYPE */
   -1,				       /* SLANG_REF_TYPE */
   -1,				       /* SLANG_COMPLEX_TYPE */
   -1,				       /* SLANG_NULL_TYPE */
   1,				       /* SLANG_UCHAR_TYPE */
   2,				       /* SLANG_SHORT_TYPE */
   3,				       /* SLANG_USHORT_TYPE */
   5,				       /* SLANG_UINT_TYPE */
   6,				       /* SLANG_LONG_TYPE */
   7,				       /* SLANG_ULONG_TYPE */
   -1,				       /* SLANG_STRING_TYPE */
#if SLANG_HAS_FLOAT
   8				       /* SLANG_FLOAT_TYPE */
#else
  -1
#endif
};

int _SLarith_get_precedence (unsigned char type)
{
   if (type > MAXIMUM_ARITH_TYPE_VALUE)
     return -1;

   return Type_Precedence_Table[type];
}

unsigned char _SLarith_promote_type (unsigned char t)
{
   switch (t)
     {
      case SLANG_FLOAT_TYPE:
      case SLANG_DOUBLE_TYPE:
      case SLANG_LONG_TYPE:
      case SLANG_ULONG_TYPE:
      case SLANG_INT_TYPE:
      case SLANG_UINT_TYPE:
	break;

      case SLANG_USHORT_TYPE:
#if SIZEOF_INT == SIZEOF_SHORT
	t = SLANG_UINT_TYPE;
	break;
#endif
	/* drop */
      case SLANG_CHAR_TYPE:
      case SLANG_UCHAR_TYPE:
      case SLANG_SHORT_TYPE:
      default:
	t = SLANG_INT_TYPE;
     }

   return t;
}

static unsigned char promote_to_common_type (unsigned char a, unsigned char b)
{
   a = _SLarith_promote_type (a);
   b = _SLarith_promote_type (b);

   return (Type_Precedence_Table[a] > Type_Precedence_Table[b]) ? a : b;
}

static int arith_bin_op_result (int op, unsigned char a_type, unsigned char b_type,
				unsigned char *c_type)
{
   switch (op)
     {
      case SLANG_EQ:
      case SLANG_NE:
      case SLANG_GT:
      case SLANG_GE:
      case SLANG_LT:
      case SLANG_LE:
      case SLANG_OR:
      case SLANG_AND:
	*c_type = SLANG_CHAR_TYPE;
	return 1;
#if SLANG_HAS_FLOAT
      case SLANG_POW:
	if (SLANG_FLOAT_TYPE == promote_to_common_type (a_type, b_type))
	  *c_type = SLANG_FLOAT_TYPE;
	else
	  *c_type = SLANG_DOUBLE_TYPE;
	return 1;
#endif
      case SLANG_BAND:
      case SLANG_BXOR:
      case SLANG_BOR:
      case SLANG_SHL:
      case SLANG_SHR:
	/* The bit-level operations are defined just for integer types */
	if ((0 == IS_INTEGER_TYPE (a_type))
	    || (0 == IS_INTEGER_TYPE(b_type)))
	  return 0;
	break;

      default:
	break;
     }

   *c_type = promote_to_common_type (a_type, b_type);
   return 1;
}

typedef int (*Bin_Fun_Type) (int,
			     unsigned char, VOID_STAR, unsigned int,
			     unsigned char, VOID_STAR, unsigned int,
			     VOID_STAR);

/* This array of functions must be indexed by precedence after arithmetic
 * promotions.
 */
static Bin_Fun_Type Bin_Fun_Map [MAX_ARITHMETIC_TYPES] =
{
   NULL,
   NULL,
   NULL,
   NULL,
   int_int_bin_op,
   uint_uint_bin_op,
   long_long_bin_op,
   ulong_ulong_bin_op,
#if SLANG_HAS_FLOAT
   float_float_bin_op,
   double_double_bin_op
#else
     NULL, NULL
#endif
};

static int arith_bin_op (int op,
			 unsigned char a_type, VOID_STAR ap, unsigned int na,
			 unsigned char b_type, VOID_STAR bp, unsigned int nb,
			 VOID_STAR cp)
{
   Convert_Fun_Type af, bf;
   Bin_Fun_Type binfun;
   int a_indx, b_indx, c_indx;
   unsigned char c_type;
   int ret;

   c_type = promote_to_common_type (a_type, b_type);

   a_indx = Type_Precedence_Table [a_type];
   b_indx = Type_Precedence_Table [b_type];
   c_indx = Type_Precedence_Table [c_type];

   af = Binary_Matrix[a_indx][c_indx].convert_function;
   bf = Binary_Matrix[b_indx][c_indx].convert_function;
   binfun = Bin_Fun_Map[c_indx];

   if ((af != NULL)
       && (NULL == (ap = (VOID_STAR) (*af) (ap, na))))
     return -1;

   if ((bf != NULL)
       && (NULL == (bp = (VOID_STAR) (*bf) (bp, nb))))
     {
	if (af != NULL) SLfree ((char *) ap);
	return -1;
     }

   ret = (*binfun) (op, a_type, ap, na, b_type, bp, nb, cp);
   if (af != NULL) SLfree ((char *) ap);
   if (bf != NULL) SLfree ((char *) bp);

   return ret;
}

static int arith_unary_op_result (int op, unsigned char a, unsigned char *b)
{
   (void) a;
   switch (op)
     {
      default:
	return 0;

      case SLANG_SQR:
      case SLANG_MUL2:
      case SLANG_PLUSPLUS:
      case SLANG_MINUSMINUS:
      case SLANG_CHS:
      case SLANG_ABS:
	*b = a;
	break;

      case SLANG_NOT:
      case SLANG_BNOT:
	if (0 == IS_INTEGER_TYPE(a))
	  return 0;
	*b = a;
	break;

      case SLANG_SIGN:
	*b = SLANG_INT_TYPE;
	break;
     }
   return 1;
}

static int integer_pop (unsigned char type, VOID_STAR ptr)
{
   SLang_Object_Type obj;
   int i, j;
   void (*f)(VOID_STAR, VOID_STAR, unsigned int);

   if (-1 == SLang_pop (&obj))
     return -1;

   if ((obj.data_type > MAXIMUM_ARITH_TYPE_VALUE)
       || ((j = Type_Precedence_Table[obj.data_type]) == -1)
       || (j >= FLOAT_PRECEDENCE_VALUE))
     {
	_SLclass_type_mismatch_error (type, obj.data_type);
       	SLang_free_object (&obj);
	return -1;
     }

   i = Type_Precedence_Table[type];
   f = (void (*)(VOID_STAR, VOID_STAR, unsigned int))
     Binary_Matrix[j][i].copy_function;

   (*f) (ptr, (VOID_STAR)&obj.v, 1);

   return 0;
}

static int integer_push (unsigned char type, VOID_STAR ptr)
{
   SLang_Object_Type obj;
   int i;
   void (*f)(VOID_STAR, VOID_STAR, unsigned int);

   i = Type_Precedence_Table[type];
   f = (void (*)(VOID_STAR, VOID_STAR, unsigned int))
     Binary_Matrix[i][i].copy_function;

   obj.data_type = type;

   (*f) ((VOID_STAR)&obj.v, ptr, 1);

   return SLang_push (&obj);
}

int SLang_pop_char (char *i)
{
   return integer_pop (SLANG_CHAR_TYPE, (VOID_STAR) i);
}

int SLang_pop_uchar (unsigned char *i)
{
   return integer_pop (SLANG_UCHAR_TYPE, (VOID_STAR) i);
}

int SLang_pop_short (short *i)
{
   return integer_pop (SLANG_SHORT_TYPE, (VOID_STAR) i);
}

int SLang_pop_ushort (unsigned short *i)
{
   return integer_pop (SLANG_USHORT_TYPE, (VOID_STAR) i);
}

int SLang_pop_long (long *i)
{
   return integer_pop (SLANG_LONG_TYPE, (VOID_STAR) i);
}

int SLang_pop_ulong (unsigned long *i)
{
   return integer_pop (SLANG_ULONG_TYPE, (VOID_STAR) i);
}

int SLang_pop_integer (int *i)
{
#if _SLANG_OPTIMIZE_FOR_SPEED
   SLang_Object_Type obj;

   if (-1 == _SLang_pop_object_of_type (SLANG_INT_TYPE, &obj, 0))
     return -1;
   *i = obj.v.int_val;
   return 0;
#else
  return integer_pop (SLANG_INT_TYPE, (VOID_STAR) i);
#endif
}

int SLang_pop_uinteger (unsigned int *i)
{
   return integer_pop (SLANG_UINT_TYPE, (VOID_STAR) i);
}

int SLang_push_integer (int i)
{
   return SLclass_push_int_obj (SLANG_INT_TYPE, i);
}
int SLang_push_uinteger (unsigned int i)
{
   return SLclass_push_int_obj (SLANG_UINT_TYPE, (int) i);
}
int SLang_push_char (char i)
{
   return SLclass_push_char_obj (SLANG_CHAR_TYPE, i);
}
int SLang_push_uchar (unsigned char i)
{
   return SLclass_push_char_obj (SLANG_UCHAR_TYPE, (char) i);
}
int SLang_push_short (short i)
{
   return SLclass_push_short_obj (SLANG_SHORT_TYPE, i);
}
int SLang_push_ushort (unsigned short i)
{
   return SLclass_push_short_obj (SLANG_USHORT_TYPE, (unsigned short) i);
}
int SLang_push_long (long i)
{
   return SLclass_push_long_obj (SLANG_LONG_TYPE, i);
}
int SLang_push_ulong (unsigned long i)
{
   return SLclass_push_long_obj (SLANG_ULONG_TYPE, (long) i);
}

_INLINE_
int _SLarith_typecast (unsigned char a_type, VOID_STAR ap, unsigned int na,
		       unsigned char b_type, VOID_STAR bp)
{
   int i, j;

   void (*copy)(VOID_STAR, VOID_STAR, unsigned int);

   i = Type_Precedence_Table[a_type];
   j = Type_Precedence_Table[b_type];

   copy = (void (*)(VOID_STAR, VOID_STAR, unsigned int))
     Binary_Matrix[i][j].copy_function;

   (*copy) (bp, ap, na);
   return 1;
}

#if SLANG_HAS_FLOAT

int SLang_pop_double(double *x, int *convertp, int *ip)
{
   SLang_Object_Type obj;
   int i, convert;

   if (0 != SLang_pop (&obj))
     return -1;

   i = 0;
   convert = 0;

   switch (obj.data_type)
     {
      case SLANG_FLOAT_TYPE:
	*x = (double) obj.v.float_val;
	break;

      case SLANG_DOUBLE_TYPE:
	*x = obj.v.double_val;
	break;

      case SLANG_INT_TYPE:
	i = obj.v.int_val;
	*x = (double) i;
	convert = 1;
	break;

      case SLANG_CHAR_TYPE: *x = (double) obj.v.char_val; break;
      case SLANG_UCHAR_TYPE: *x = (double) obj.v.uchar_val; break;
      case SLANG_SHORT_TYPE: *x = (double) obj.v.short_val; break;
      case SLANG_USHORT_TYPE: *x = (double) obj.v.ushort_val; break;
      case SLANG_UINT_TYPE: *x = (double) obj.v.uint_val; break;
      case SLANG_LONG_TYPE: *x = (double) obj.v.long_val; break;
      case SLANG_ULONG_TYPE: *x = (double) obj.v.ulong_val; break;
	
      default:
	_SLclass_type_mismatch_error (SLANG_DOUBLE_TYPE, obj.data_type);
	SLang_free_object (&obj);
	return -1;
     }

   if (convertp != NULL) *convertp = convert;
   if (ip != NULL) *ip = i;

   return 0;
}

int SLang_push_double (double x)
{
   return SLclass_push_double_obj (SLANG_DOUBLE_TYPE, x);
}

int SLang_pop_float (float *x)
{
   double d;

   /* Pop it as a double and let the double function do all the typcasting */
   if (-1 == SLang_pop_double (&d, NULL, NULL))
     return -1;

   *x = (float) d;
   return 0;
}

int SLang_push_float (float f)
{
   return SLclass_push_float_obj (SLANG_FLOAT_TYPE, (double) f);
}

/* Double */
static int double_push (SLtype type, VOID_STAR ptr)
{
#if _SLANG_OPTIMIZE_FOR_SPEED
   SLang_Object_Type obj;
   obj.data_type = type;
   obj.v.double_val = *(double *)ptr;
   return SLang_push (&obj);
#else
   return SLclass_push_double_obj (type, *(double *) ptr);
#endif
}

static int double_push_literal (unsigned char type, VOID_STAR ptr)
{
   (void) type;
   return SLang_push_double (**(double **)ptr);
}

static int double_pop (unsigned char unused, VOID_STAR ptr)
{
   (void) unused;
   return SLang_pop_double ((double *) ptr, NULL, NULL);
}

static void double_byte_code_destroy (unsigned char unused, VOID_STAR ptr)
{
   (void) unused;
   SLfree (*(char **) ptr);
}

static int float_push (unsigned char unused, VOID_STAR ptr)
{
   (void) unused;
   SLang_push_float (*(float *) ptr);
   return 0;
}

static int float_pop (unsigned char unused, VOID_STAR ptr)
{
   (void) unused;
   return SLang_pop_float ((float *) ptr);
}

#endif				       /* SLANG_HAS_FLOAT */

#if SLANG_HAS_FLOAT
static char Double_Format[16] = "%g";

void _SLset_double_format (char *s)
{
   strncpy (Double_Format, s, 15);
   Double_Format[15] = 0;
}
#endif

static char *arith_string (unsigned char type, VOID_STAR v)
{
   char buf [256];
   char *s;

   s = buf;

   switch (type)
     {
      default:
	s = SLclass_get_datatype_name (type);
	break;

      case SLANG_CHAR_TYPE:
	sprintf (s, "%d", *(char *) v);
	break;
      case SLANG_UCHAR_TYPE:
	sprintf (s, "%u", *(unsigned char *) v);
	break;
      case SLANG_SHORT_TYPE:
	sprintf (s, "%d", *(short *) v);
	break;
      case SLANG_USHORT_TYPE:
	sprintf (s, "%u", *(unsigned short *) v);
	break;
      case SLANG_INT_TYPE:
	sprintf (s, "%d", *(int *) v);
	break;
      case SLANG_UINT_TYPE:
	sprintf (s, "%u", *(unsigned int *) v);
	break;
      case SLANG_LONG_TYPE:
	sprintf (s, "%ld", *(long *) v);
	break;
      case SLANG_ULONG_TYPE:
	sprintf (s, "%lu", *(unsigned long *) v);
	break;
#if SLANG_HAS_FLOAT
      case SLANG_FLOAT_TYPE:
	if (EOF == _SLsnprintf (buf, sizeof (buf), Double_Format, *(float *) v))
	  sprintf (s, "%e", *(float *) v);
	break;
      case SLANG_DOUBLE_TYPE:
	if (EOF == _SLsnprintf (buf, sizeof (buf), Double_Format, *(double *) v))
	  sprintf (s, "%e", *(double *) v);
	break;
#endif
     }

   return SLmake_string (s);
}

static int integer_to_bool (unsigned char type, int *t)
{
   (void) type;
   return SLang_pop_integer (t);
}

/* Note that integer literals are all stored in the byte-code as longs.  This
 * is why it is necessary to use *(long*).
 */
static int push_int_literal (unsigned char type, VOID_STAR ptr)
{
   return SLclass_push_int_obj (type, (int) *(long *) ptr);
}

static int push_char_literal (unsigned char type, VOID_STAR ptr)
{
   return SLclass_push_char_obj (type, (char) *(long *) ptr);
}

#if SIZEOF_SHORT != SIZEOF_INT
static int push_short_literal (unsigned char type, VOID_STAR ptr)
{
   return SLclass_push_short_obj (type, (short) *(long *) ptr);
}
#endif

#if SIZEOF_INT != SIZEOF_LONG
static int push_long_literal (unsigned char type, VOID_STAR ptr)
{
   return SLclass_push_long_obj (type, *(long *) ptr);
}
#endif

typedef struct
{
   char *name;
   unsigned char data_type;
   unsigned int sizeof_type;
   int (*unary_fun)(int, unsigned char, VOID_STAR, unsigned int, VOID_STAR);
   int (*push_literal) (unsigned char, VOID_STAR);
   int (*cmp_fun) (unsigned char, VOID_STAR, VOID_STAR, int *);
}
Integer_Info_Type;

static Integer_Info_Type Integer_Types [8] =
{
     {"Char_Type", SLANG_CHAR_TYPE, sizeof (char), char_unary_op, push_char_literal, char_cmp_function},
     {"UChar_Type", SLANG_UCHAR_TYPE, sizeof (unsigned char), uchar_unary_op, push_char_literal, uchar_cmp_function},
#if SIZEOF_INT != SIZEOF_SHORT
     {"Short_Type", SLANG_SHORT_TYPE, sizeof (short), short_unary_op, push_short_literal, short_cmp_function},
     {"UShort_Type", SLANG_USHORT_TYPE, sizeof (unsigned short), ushort_unary_op, push_short_literal, ushort_cmp_function},
#else
     {NULL, SLANG_SHORT_TYPE},
     {NULL, SLANG_USHORT_TYPE},
#endif

     {"Integer_Type", SLANG_INT_TYPE, sizeof (int), int_unary_op, push_int_literal, int_cmp_function},
     {"UInteger_Type", SLANG_UINT_TYPE, sizeof (unsigned int), uint_unary_op, push_int_literal, uint_cmp_function},

#if SIZEOF_INT != SIZEOF_LONG
     {"Long_Type", SLANG_LONG_TYPE, sizeof (long), long_unary_op, push_long_literal, long_cmp_function},
     {"ULong_Type", SLANG_ULONG_TYPE, sizeof (unsigned long), ulong_unary_op, push_long_literal, ulong_cmp_function}
#else
     {NULL, SLANG_LONG_TYPE, 0, NULL, NULL, NULL},
     {NULL, SLANG_ULONG_TYPE, 0, NULL, NULL, NULL}
#endif
};

static int create_synonyms (void)
{
   static char *names[8] =
     {
	"Int16_Type", "UInt16_Type", "Int32_Type", "UInt32_Type",
	"Int64_Type", "UInt64_Type",
	"Float32_Type", "Float64_Type"
     };
   int types[8];
   unsigned int i;

   memset ((char *) types, 0, sizeof (types));

   /* The assumption is that sizeof(unsigned X) == sizeof (X) */
#if SIZEOF_INT == 2
   types[0] = SLANG_INT_TYPE;
   types[1] = SLANG_UINT_TYPE;
#else
# if SIZEOF_SHORT == 2
   types[0] = SLANG_SHORT_TYPE;
   types[1] = SLANG_USHORT_TYPE;
# else
#  if SIZEOF_LONG == 2
   types[0] = SLANG_LONG_TYPE;
   types[1] = SLANG_ULONG_TYPE;
#  endif
# endif
#endif

#if SIZEOF_INT == 4
   types[2] = SLANG_INT_TYPE;
   types[3] = SLANG_UINT_TYPE;
#else
# if SIZEOF_SHORT == 4
   types[2] = SLANG_SHORT_TYPE;
   types[3] = SLANG_USHORT_TYPE;
# else
#  if SIZEOF_LONG == 4
   types[2] = SLANG_LONG_TYPE;
   types[3] = SLANG_ULONG_TYPE;
#  endif
# endif
#endif

#if SIZEOF_INT == 8
   types[4] = SLANG_INT_TYPE;
   types[5] = SLANG_UINT_TYPE;
#else
# if SIZEOF_SHORT == 8
   types[4] = SLANG_SHORT_TYPE;
   types[5] = SLANG_USHORT_TYPE;
# else
#  if SIZEOF_LONG == 8
   types[4] = SLANG_LONG_TYPE;
   types[5] = SLANG_ULONG_TYPE;
#  endif
# endif
#endif

#if SLANG_HAS_FLOAT

#if SIZEOF_FLOAT == 4
   types[6] = SLANG_FLOAT_TYPE;
#else
# if SIZEOF_DOUBLE == 4
   types[6] = SLANG_DOUBLE_TYPE;
# endif
#endif
#if SIZEOF_FLOAT == 8
   types[7] = SLANG_FLOAT_TYPE;
#else
# if SIZEOF_DOUBLE == 8
   types[7] = SLANG_DOUBLE_TYPE;
# endif
#endif

#endif

   if ((-1 == SLclass_create_synonym ("Int_Type", SLANG_INT_TYPE))
       || (-1 == SLclass_create_synonym ("UInt_Type", SLANG_UINT_TYPE)))
     return -1;

   for (i = 0; i < 8; i++)
     {
	if (types[i] == 0) continue;

	if (-1 == SLclass_create_synonym (names[i], types[i]))
	  return -1;
     }

#if SIZEOF_INT == SIZEOF_SHORT
   if ((-1 == SLclass_create_synonym ("Short_Type", SLANG_INT_TYPE))
       || (-1 == SLclass_create_synonym ("UShort_Type", SLANG_UINT_TYPE))
       || (-1 == _SLclass_copy_class (SLANG_SHORT_TYPE, SLANG_INT_TYPE))
       || (-1 == _SLclass_copy_class (SLANG_USHORT_TYPE, SLANG_UINT_TYPE)))
     return -1;
#endif
#if SIZEOF_INT == SIZEOF_LONG
   if ((-1 == SLclass_create_synonym ("Long_Type", SLANG_INT_TYPE))
       || (-1 == SLclass_create_synonym ("ULong_Type", SLANG_UINT_TYPE))
       || (-1 == _SLclass_copy_class (SLANG_LONG_TYPE, SLANG_INT_TYPE))
       || (-1 == _SLclass_copy_class (SLANG_ULONG_TYPE, SLANG_UINT_TYPE)))
     return -1;
#endif
   return 0;
}

int _SLarith_register_types (void)
{
   SLang_Class_Type *cl;
   int a_type, b_type;
   int i, j;

#if defined(HAVE_SETLOCALE) && defined(LC_NUMERIC)
   /* make sure decimal point it used --- the parser requires it */
   (void) setlocale (LC_NUMERIC, "C"); 
#endif

   for (i = 0; i < 8; i++)
     {
	Integer_Info_Type *info;

	info = Integer_Types + i;

	if (info->name == NULL)
	  {
	     /* This happens when the object is the same size as an integer
	      * For this case, we really want to copy the integer class.
	      * We will handle that when the synonym is created.
	      */
	     continue;
	  }

	if (NULL == (cl = SLclass_allocate_class (info->name)))
	  return -1;

	(void) SLclass_set_string_function (cl, arith_string);
	(void) SLclass_set_push_function (cl, integer_push);
	(void) SLclass_set_pop_function (cl, integer_pop);
	cl->cl_push_literal = info->push_literal;
	cl->cl_to_bool = integer_to_bool;

	cl->cl_cmp = info->cmp_fun;

	if (-1 == SLclass_register_class (cl, info->data_type, info->sizeof_type,
					  SLANG_CLASS_TYPE_SCALAR))
	  return -1;
	if (-1 == SLclass_add_unary_op (info->data_type, info->unary_fun, arith_unary_op_result))
	  return -1;
	
	_SLang_set_arith_type (info->data_type, 1);
     }

#if SLANG_HAS_FLOAT
   if (NULL == (cl = SLclass_allocate_class ("Double_Type")))
     return -1;
   (void) SLclass_set_push_function (cl, double_push);
   (void) SLclass_set_pop_function (cl, double_pop);
   (void) SLclass_set_string_function (cl, arith_string);
   cl->cl_byte_code_destroy = double_byte_code_destroy;
   cl->cl_push_literal = double_push_literal;
   cl->cl_cmp = double_cmp_function;

   if (-1 == SLclass_register_class (cl, SLANG_DOUBLE_TYPE, sizeof (double),
				     SLANG_CLASS_TYPE_SCALAR))
     return -1;
   if (-1 == SLclass_add_unary_op (SLANG_DOUBLE_TYPE, double_unary_op, arith_unary_op_result))
     return -1;
   _SLang_set_arith_type (SLANG_DOUBLE_TYPE, 2);

   if (NULL == (cl = SLclass_allocate_class ("Float_Type")))
     return -1;
   (void) SLclass_set_string_function (cl, arith_string);
   (void) SLclass_set_push_function (cl, float_push);
   (void) SLclass_set_pop_function (cl, float_pop);
   cl->cl_cmp = float_cmp_function;

   if (-1 == SLclass_register_class (cl, SLANG_FLOAT_TYPE, sizeof (float),
				     SLANG_CLASS_TYPE_SCALAR))
     return -1;
   if (-1 == SLclass_add_unary_op (SLANG_FLOAT_TYPE, float_unary_op, arith_unary_op_result))
     return -1;
   _SLang_set_arith_type (SLANG_FLOAT_TYPE, 2);
#endif

   if (-1 == create_synonyms ())
     return -1;

   for (a_type = 0; a_type <= MAXIMUM_ARITH_TYPE_VALUE; a_type++)
     {
	if (-1 == (i = Type_Precedence_Table [a_type]))
	  continue;

	for (b_type = 0; b_type <= MAXIMUM_ARITH_TYPE_VALUE; b_type++)
	  {
	     int implicit_ok;

	     if (-1 == (j = Type_Precedence_Table [b_type]))
	       continue;

	     /* Allow implicit typecast, except from into to float */
	     implicit_ok = ((j >= FLOAT_PRECEDENCE_VALUE)
			    || (i < FLOAT_PRECEDENCE_VALUE));

	     if (-1 == SLclass_add_binary_op (a_type, b_type, arith_bin_op, arith_bin_op_result))
	       return -1;

	     if (i != j)
	       if (-1 == SLclass_add_typecast (a_type, b_type, _SLarith_typecast, implicit_ok))
		 return -1;
	  }
     }

   return 0;
}

#if _SLANG_OPTIMIZE_FOR_SPEED

static void promote_objs (SLang_Object_Type *a, SLang_Object_Type *b,
			  SLang_Object_Type *c, SLang_Object_Type *d)
{
   unsigned char ia, ib, ic, id;
   int i, j;
   void (*copy)(VOID_STAR, VOID_STAR, unsigned int);

   ia = a->data_type;
   ib = b->data_type;
   
   ic = _SLarith_promote_type (ia);

   if (ic == ib) id = ic;	       /* already promoted */
   else id = _SLarith_promote_type (ib);

   i = Type_Precedence_Table[ic];
   j = Type_Precedence_Table[id];
   if (i > j)
     {
	id = ic;
	j = i;
     }

   c->data_type = d->data_type = id;

   i = Type_Precedence_Table[ia];
   copy = (void (*)(VOID_STAR, VOID_STAR, unsigned int))
     Binary_Matrix[i][j].copy_function;
   (*copy) ((VOID_STAR) &c->v, (VOID_STAR)&a->v, 1);

   i = Type_Precedence_Table[ib];
   copy = (void (*)(VOID_STAR, VOID_STAR, unsigned int))
     Binary_Matrix[i][j].copy_function;
   (*copy) ((VOID_STAR) &d->v, (VOID_STAR)&b->v, 1);
}

int _SLarith_bin_op (SLang_Object_Type *oa, SLang_Object_Type *ob, int op)
{
   unsigned char a_type, b_type;

   a_type = oa->data_type;
   b_type = ob->data_type;

   if (a_type != b_type)
     {
	SLang_Object_Type obj_a, obj_b;
	
	/* Handle common cases */
#if SLANG_HAS_FLOAT
	if ((a_type == SLANG_INT_TYPE)
	    && (b_type == SLANG_DOUBLE_TYPE))
	  return double_double_scalar_bin_op (oa->v.int_val, ob->v.double_val, op);

	if ((a_type == SLANG_DOUBLE_TYPE)
	    && (b_type == SLANG_INT_TYPE))
	  return double_double_scalar_bin_op (oa->v.double_val, ob->v.int_val, op);
#endif
	/* Otherwise do it the hard way */
	promote_objs (oa, ob, &obj_a, &obj_b);
	oa = &obj_a;
	ob = &obj_b;
	
	a_type = oa->data_type;
	/* b_type = ob->data_type; */
     }
   
	  
   switch (a_type)
     {
      case SLANG_CHAR_TYPE:
	return int_int_scalar_bin_op (oa->v.char_val, ob->v.char_val, op);

      case SLANG_UCHAR_TYPE:
	return int_int_scalar_bin_op (oa->v.uchar_val, ob->v.uchar_val, op);

      case SLANG_SHORT_TYPE:
	return int_int_scalar_bin_op (oa->v.short_val, ob->v.short_val, op);

      case SLANG_USHORT_TYPE:
# if SIZEOF_INT == SIZEOF_SHORT
	return uint_uint_scalar_bin_op (oa->v.ushort_val, ob->v.ushort_val, op);
# else
	return int_int_scalar_bin_op ((int)oa->v.ushort_val, (int)ob->v.ushort_val, op);
# endif

#if SIZEOF_LONG == SIZEOF_INT
      case SLANG_LONG_TYPE:
#endif
      case SLANG_INT_TYPE:
	return int_int_scalar_bin_op (oa->v.int_val, ob->v.int_val, op);

#if SIZEOF_LONG == SIZEOF_INT
      case SLANG_ULONG_TYPE:
#endif
      case SLANG_UINT_TYPE:
	return uint_uint_scalar_bin_op (oa->v.uint_val, ob->v.uint_val, op);
	
#if SIZEOF_LONG != SIZEOF_INT
      case SLANG_LONG_TYPE:
	return long_long_scalar_bin_op (oa->v.long_val, ob->v.long_val, op);
      case SLANG_ULONG_TYPE:
	return ulong_ulong_scalar_bin_op (oa->v.ulong_val, ob->v.ulong_val, op);
#endif
#if SLANG_HAS_FLOAT
      case SLANG_FLOAT_TYPE:
	return float_float_scalar_bin_op (oa->v.float_val, ob->v.float_val, op);
      case SLANG_DOUBLE_TYPE:
	return double_double_scalar_bin_op (oa->v.double_val, ob->v.double_val, op);
#endif
     }
   
   return 1;
}
#endif
