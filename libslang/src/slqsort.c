/******************************************************************/
/* qsort.c  --  Non-Recursive ANSI Quicksort function             */
/*                                                                */
/* Public domain by Raymond Gardner, Englewood CO  February 1991  */
/*                                                                */
/* Usage:                                                         */
/*     qsort(base, nbr_elements, width_bytes, compare_function);  */
/*        void *base;                                             */
/*        size_t nbr_elements, width_bytes;                       */
/*        int (*compare_function)(const void *, const void *);    */
/*                                                                */
/* Sorts an array starting at base, of length nbr_elements, each  */
/* element of size width_bytes, ordered via compare_function,     */
/* which is called as  (*compare_function)(ptr_to_element1,       */
/* ptr_to_element2) and returns < 0 if element1 < element2,       */
/* 0 if element1 = element2, > 0 if element1 > element2.          */
/* Most refinements are due to R. Sedgewick. See "Implementing    */
/* Quicksort Programs", Comm. ACM, Oct. 1978, and Corrigendum,    */
/* Comm. ACM, June 1979.                                          */
/******************************************************************/

/* John E. Davis: modifed to use my coding style and made gcc warning
 * clean.  Also, the name changed to reflect the fact that the function 
 * carries state data.
 */

#include <stddef.h>                     /* for size_t definition  */

/* prototypes */
extern void _SLqsort (void *, size_t, size_t,
		      int (*)(void *, void *, void *), void *);

static void swap_chars(char *, char *, size_t);

/*
 * Compile with -DSWAP_INTS if your machine can access an int at an
 * arbitrary location with reasonable efficiency.  Some machines
 * cannot access an int at an odd address at all, so be careful.
 */

#ifdef   SWAP_INTS
static void swap_ints(char *, char *, size_t);
# define  SWAP(a, b)  (swap_func((char *)(a), (char *)(b), width))
#else
# define  SWAP(a, b)  (swap_chars((char *)(a), (char *)(b), size))
#endif

#define  COMP(a, b, c)  ((*comp)((void *)(a), (void *)(b), (c)))

#define  T           7    
/* subfiles of T or fewer elements will be sorted by a simple insertion sort.
 * Note!  T must be at least 3
 */

void _SLqsort(void *basep, size_t nelems, size_t size,
	      int (*comp)(void *, void *, void *), void *cd)
{
   char *stack[40], **sp;       /* stack and stack pointer        */
   char *i, *j, *limit;         /* scan and limit pointers        */
   size_t thresh;               /* size of T elements in bytes    */
   char *base;                  /* base pointer as char *         */

#ifdef   SWAP_INTS
   size_t width;                /* width of array element         */
   void (*swap_func)(char *, char *, size_t); /* swap func pointer*/

   width = size;                /* save size for swap routine     */
   swap_func = swap_chars;      /* choose swap function           */
   if ( size % sizeof(int) == 0 )
     {
	/* size is multiple of ints */
	width /= sizeof(int);           /* set width in ints        */
	swap_func = swap_ints;          /* use int swap function    */
     }
#endif

   base = (char *)basep;        /* set up char * base pointer     */
   thresh = T * size;           /* init threshold                 */
   sp = stack;                  /* init stack pointer             */
   limit = base + nelems * size;/* pointer past end of array      */
   
   while (1)
     {
	/* repeat until break...          */
	if (limit > base + thresh)
	  {
	     /* if more than T elements,  swap base with middle  */
	     SWAP((((limit-base)/size)/2)*size+base, base);

	     i = base + size;	       /* i scans left to right    */
	     j = limit - size;	       /* j scans right to left    */
	     if (COMP(i, j, cd) > 0)   /* Sedgewick's              */
	       SWAP(i, j);	       /*    three-element sort    */
	     if (COMP(base, j, cd) > 0)/*        sets things up    */
	       SWAP(base, j);	       /*            so that       */
	     if (COMP(i, base,cd ) > 0)/*      *i <= *base <= *j   */
	       SWAP(i, base);	       /* *base is pivot element   */

	     while (1)
	       {
		  do
		    {
		       /* move i right until *i >= pivot */
		       i += size;  
		    }
		  while (COMP(i, base, cd) < 0);

		  /* move j left until *j <= pivot */
		  do
		    {
		       j -= size;
		    }
		  while (COMP(j, base, cd) > 0);
		  
		  /* if pointers crossed, break loop */
		  if (i > j) break;
		  
		  SWAP(i, j);	       /* else swap elements, keep scanning*/
	       }
	     
	     SWAP(base, j);         /* move pivot into correct place  */

	     if (j - base > limit - i)
	       {
		  /* if left subfile larger */
		  sp[0] = base;             /* stack left subfile base  */
		  sp[1] = j;                /*    and limit             */
		  base = i;                 /* sort the right subfile   */
	       }
	     else
	       {
		  /* else right subfile larger*/
		  sp[0] = i;                /* stack right subfile base */
		  sp[1] = limit;            /*    and limit             */
		  limit = j;                /* sort the left subfile    */
	       }
	     sp += 2;                     /* increment stack pointer  */
	  }
	else
	  {
	     /* else subfile is small, use insertion sort  */
	     j = base;
	     i = j + size;
	     
	     while (i < limit)
	       {
		  while (COMP(j, j + size, cd) > 0)
		    {
		       SWAP(j, j+size);
		       if (j == base)
			 break;
		       j -= size;
		    }

		  j = i;
		  i += size;
	       }
	     
	     if (sp == stack)	       /* done */
	       break;

	     /* if any entries on stack  */
	     sp -= 2;                  /* pop the base and limit   */
	     base = sp[0];
	     limit = sp[1];
	  }
     }
}

/*
**  swap nbytes between a and b
*/

static void swap_chars(char *a, char *b, size_t nbytes)
{
   char tmp;
   do
     {
	tmp = *a; *a++ = *b; *b++ = tmp;
     }
   while ( --nbytes );
}

#ifdef   SWAP_INTS

/*
**  swap nints between a and b
*/

static void swap_ints(char *ap, char *bp, size_t nints)
{
   int *a = (int *)ap, *b = (int *)bp;
   int tmp;
   do
     {
	tmp = *a; *a++ = *b; *b++ = tmp;
     }
   while ( --nints );
}

#endif

#ifdef TESTING

#include <stdio.h>
#include <math.h>

static int cmp_fun (void *a, void *b, void *c)
{
   double x, y;
   double *xp, *yp;
   double *data;
   
   data = (double *) c;

   xp = data + *(unsigned int *)a;
   yp = data + *(unsigned int *)b;

   x = *xp;
   y = *yp;

   if (x > y) return 1; else if (x < y) return -1;
   if (xp > yp) return 1;
   if (xp == yp) return 0;
   return -1;
}

#define ARRAY_SIZE 10000
int main (int argc, char **argv)
{
   unsigned int i;
   double x, dx;
   unsigned int index_array [ARRAY_SIZE];
   double double_array [ARRAY_SIZE];
   
   x = 0.0;
   dx = 6.28 / ARRAY_SIZE;
   for (i = 0; i < ARRAY_SIZE; i++)
     {
	double_array [i] = sin(x);
	index_array [i] = i;
	x += dx;
     }

   _SLqsort ((void *) index_array, ARRAY_SIZE, 
	     sizeof (unsigned int), cmp_fun, (void *) double_array);
   
   if (argc > 1)
     for (i = 0; i < ARRAY_SIZE; i++)
     {
	fprintf (stdout, "%f\t%f\n",
		 double_array[i], double_array[index_array[i]]);
     }
   
   return 0;
}
#endif
