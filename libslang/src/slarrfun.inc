/* -*- mode: C -*- */

/* Some "inline" functions for generic scalar types */

#ifdef TRANSPOSE_2D_ARRAY
static SLang_Array_Type *TRANSPOSE_2D_ARRAY (SLang_Array_Type *at, SLang_Array_Type *bt)
{
   GENERIC_TYPE *a_data, *b_data;
   int nr, nc, i;

   nr = at->dims[0];
   nc = at->dims[1];

   a_data = (GENERIC_TYPE *) at->data;
   b_data = (GENERIC_TYPE *) bt->data;

   for (i = 0; i < nr; i++)
     {
	GENERIC_TYPE *offset = b_data + i;
	int j;
	for (j = 0; j < nc; j++)
	  {
	     *offset = *a_data++;
	     offset += nr;
	  }
     }
   return bt;
}
#undef TRANSPOSE_2D_ARRAY
#endif


#ifdef INNERPROD_FUNCTION

static void INNERPROD_FUNCTION
  (SLang_Array_Type *at, SLang_Array_Type *bt, SLang_Array_Type *ct,
   unsigned int a_loops, unsigned int a_stride,
   unsigned int b_loops, unsigned int b_inc,
   unsigned int inner_loops)
{
   GENERIC_TYPE_A *a;
   GENERIC_TYPE_B *b;
   GENERIC_TYPE_C *c;

   c = (GENERIC_TYPE_C *) ct->data;
   b = (GENERIC_TYPE_B *) bt->data;
   a = (GENERIC_TYPE_A *) at->data;
   
   while (a_loops--)
     {
	GENERIC_TYPE_B *bb;
	unsigned int j;

	bb = b;

	for (j = 0; j < inner_loops; j++)
	  {
	     double x = (double) a[j];

	     if (x != 0.0)
	       {
		  unsigned int k;

		  for (k = 0; k < b_loops; k++)
		    c[k] += x * bb[k];
	       }
	     bb += b_inc;
	  }
	c += b_loops;
	a += a_stride;
     }
}
#undef INNERPROD_FUNCTION

#undef GENERIC_TYPE_A
#undef GENERIC_TYPE_B
#undef GENERIC_TYPE_C
#endif

#ifdef INNERPROD_COMPLEX_A
static void INNERPROD_COMPLEX_A
  (SLang_Array_Type *at, SLang_Array_Type *bt, SLang_Array_Type *ct,
   unsigned int a_loops, unsigned int a_stride,
   unsigned int b_loops, unsigned int b_inc,
   unsigned int inner_loops)
{
   double *a;
   GENERIC_TYPE *b;
   double *c;

   c = (double *) ct->data;
   b = (GENERIC_TYPE *) bt->data;
   a = (double *) at->data;
   
   a_stride *= 2;

   while (a_loops--)
     {
	GENERIC_TYPE *bb;
	unsigned int bb_loops;

	bb = b;
	bb_loops = b_loops;
	
	while (bb_loops--)
	  {
	     double real_sum;
	     double imag_sum;
	     unsigned int iloops;
	     double *aa;
	     GENERIC_TYPE *bbb;
	     
	     aa = a;
	     bbb = bb;
	     iloops = inner_loops;

	     real_sum = 0.0;
	     imag_sum = 0.0;
	     while (iloops--)
	       {
		  real_sum += aa[0] * (double)bbb[0];
		  imag_sum += aa[1] * (double)bbb[0];
		  aa += 2;
		  bbb += b_inc;
	       }

	     *c++ = real_sum;
	     *c++ = imag_sum;
	     bb++;
	  }

	a += a_stride;
     }
}

static void INNERPROD_A_COMPLEX
  (SLang_Array_Type *at, SLang_Array_Type *bt, SLang_Array_Type *ct,
   unsigned int a_loops, unsigned int a_stride,
   unsigned int b_loops, unsigned int b_inc,
   unsigned int inner_loops)
{
   GENERIC_TYPE *a;
   double *b;
   double *c;

   c = (double *) ct->data;
   b = (double *) bt->data;
   a = (GENERIC_TYPE *) at->data;
   
   b_inc *= 2;

   while (a_loops--)
     {
	double *bb;
	unsigned int bb_loops;

	bb = b;
	bb_loops = b_loops;
	
	while (bb_loops--)
	  {
	     double real_sum;
	     double imag_sum;
	     unsigned int iloops;
	     GENERIC_TYPE *aa;
	     double *bbb;

	     aa = a;
	     bbb = bb;
	     iloops = inner_loops;

	     real_sum = 0.0;
	     imag_sum = 0.0;
	     while (iloops--)
	       {
		  real_sum += (double)aa[0] * bbb[0];
		  imag_sum += (double)aa[0] * bbb[1];
		  aa += 1;
		  bbb += b_inc;
	       }

	     *c++ = real_sum;
	     *c++ = imag_sum;
	     bb += 2;
	  }

	a += a_stride;
     }
}

#undef INNERPROD_A_COMPLEX
#undef INNERPROD_COMPLEX_A
#endif				       /* INNERPROD_COMPLEX_A */


#ifdef INNERPROD_COMPLEX_COMPLEX
static void INNERPROD_COMPLEX_COMPLEX
  (SLang_Array_Type *at, SLang_Array_Type *bt, SLang_Array_Type *ct,
   unsigned int a_loops, unsigned int a_stride,
   unsigned int b_loops, unsigned int b_inc,
   unsigned int inner_loops)
{
   double *a;
   double *b;
   double *c;

   c = (double *) ct->data;
   b = (double *) bt->data;
   a = (double *) at->data;
   
   a_stride *= 2;
   b_inc *= 2;

   while (a_loops--)
     {
	double *bb;
	unsigned int bb_loops;

	bb = b;
	bb_loops = b_loops;
	
	while (bb_loops--)
	  {
	     double real_sum;
	     double imag_sum;
	     unsigned int iloops;
	     double *aa;
	     double *bbb;

	     aa = a;
	     bbb = bb;
	     iloops = inner_loops;

	     real_sum = 0.0;
	     imag_sum = 0.0;
	     while (iloops--)
	       {
		  real_sum += aa[0]*bbb[0] - aa[1]*bbb[1];
		  imag_sum += aa[0]*bbb[1] + aa[1]*bbb[0];
		  aa += 2;
		  bbb += b_inc;
	       }

	     *c++ = real_sum;
	     *c++ = imag_sum;
	     bb += 2;
	  }

	a += a_stride;
     }
}
#undef INNERPROD_COMPLEX_COMPLEX
#endif

#ifdef SUM_FUNCTION
#if SLANG_HAS_FLOAT
static int SUM_FUNCTION (VOID_STAR xp, unsigned int inc, unsigned int num, VOID_STAR yp)
{
   GENERIC_TYPE *x, *xmax;
   double sum;
   
   sum = 0.0;
   x = (GENERIC_TYPE *) xp;
   xmax = x + num;
#if _SLANG_OPTIMIZE_FOR_SPEED
   if (inc == 1)
     {
	while (x < xmax)
	  {
	     sum += (double) *x;
	     x++;
	  }
     }
   else
#endif
     while (x < xmax)
     {
	sum += (double) *x;
	x += inc;
     }
   *(SUM_RESULT_TYPE *)yp = (SUM_RESULT_TYPE) sum;
   return 0;
}
#endif				       /* SLANG_HAS_FLOAT */
#undef SUM_FUNCTION
#undef SUM_RESULT_TYPE
#endif

#ifdef MIN_FUNCTION
static int 
MIN_FUNCTION (VOID_STAR ip, unsigned int inc, unsigned int num, VOID_STAR sp)
{
   unsigned int n;
   GENERIC_TYPE m;
   GENERIC_TYPE *i = (GENERIC_TYPE *)ip;

   if (-1 == check_for_empty_array ("min", num))
     return -1;

   m = i[0];

   for (n = inc; n < num; n += inc)
     if (m > i[n]) m = i[n];
   
   *(GENERIC_TYPE *)sp = m;
   return 0;
}
#undef MIN_FUNCTION
#endif

#ifdef MAX_FUNCTION
static int
MAX_FUNCTION (VOID_STAR ip, unsigned int inc, unsigned int num, VOID_STAR s)
{
   unsigned int n;
   GENERIC_TYPE m;
   GENERIC_TYPE *i = (GENERIC_TYPE *) ip;

   if (-1 == check_for_empty_array ("max", num))
     return -1;

   m = i[0];

   for (n = inc; n < num; n += inc)
     if (m < i[n]) m = i[n];
   
   *(GENERIC_TYPE *)s = m;
   return 0;
}
#undef MAX_FUNCTION
#endif


#ifdef CUMSUM_FUNCTION
#ifdef SLANG_HAS_FLOAT
static int 
CUMSUM_FUNCTION (SLtype xtype, VOID_STAR xp, unsigned int inc, 
		 unsigned int num,
		 SLtype ytype, VOID_STAR yp, VOID_STAR clientdata)
{
   GENERIC_TYPE *x, *xmax;
   CUMSUM_RESULT_TYPE *y;
   double c;

   (void) xtype;
   (void) ytype;
   (void) clientdata;

   x = (GENERIC_TYPE *) xp;
   y = (CUMSUM_RESULT_TYPE *) yp;
   xmax = x + num;

   c = 0.0;
   while (x < xmax)
     {
	c += (double) *x;
	*y = (CUMSUM_RESULT_TYPE) c;
	x += inc;
	y += inc;
     }
   return 0;
}
#endif				       /* SLANG_HAS_FLOAT */
#undef CUMSUM_FUNCTION
#undef CUMSUM_RESULT_TYPE
#endif

#ifdef GENERIC_TYPE
# undef GENERIC_TYPE
#endif
