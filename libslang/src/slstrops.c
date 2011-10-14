/* -*- mode: C; mode: fold; -*- */
/* string manipulation functions for S-Lang. */
/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"
/*{{{ Include Files */

#include <time.h>

#ifndef __QNX__
# if defined(__GO32__) || defined(__WATCOMC__)
#  include <dos.h>
#  include <bios.h>
# endif
#endif

#if SLANG_HAS_FLOAT
#include <math.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef isdigit
# define isdigit(x) (((x) >= '0') && ((x) <= '9'))
#endif

#include "slang.h"
#include "_slang.h"

/*}}}*/

#define USE_ALLOC_STSTRING 1

/*{{{ Utility Functions */

static unsigned char Utility_Char_Table [256];
static unsigned char WhiteSpace_Lut[256];

static void set_utility_char_table (char *pos) /*{{{*/
{
   register unsigned char *t = Utility_Char_Table;
   register unsigned char ch;

   memset ((char *) t, 0, sizeof (Utility_Char_Table));
   while ((ch = (unsigned char) *pos++) != 0) t[ch] = 1;
}

/*}}}*/

_INLINE_
static unsigned char *make_whitespace_lut (void)
{
   if (WhiteSpace_Lut[' '] != 1)
     {
	WhiteSpace_Lut[' '] = WhiteSpace_Lut['\r'] 
	  = WhiteSpace_Lut ['\n'] = WhiteSpace_Lut['\t']
	  = WhiteSpace_Lut ['\f'] = 1;
     }
   return WhiteSpace_Lut;
}

static unsigned char *make_lut (unsigned char *s, unsigned char *lut)
{
   int reverse = 0;
   
   if (*s == '^')
     {
	reverse = 1;
	s++;
     }
   SLmake_lut (lut, s, reverse);
   return lut;
}

static unsigned int do_trim (char **beg, int do_beg, 
			     char **end, int do_end, 
			     char *white) /*{{{*/
{
   unsigned int len;
   char *a, *b;
   unsigned char *lut;

   if (white == NULL)
     lut = make_whitespace_lut ();
   else
     {
	set_utility_char_table (white);
	lut = Utility_Char_Table;
     }

   a = *beg;
   len = strlen (a);
   b = a + len;

   if (do_beg)
     while (lut[(unsigned char) *a]) a++;

   if (do_end)
     {
	b--;
	while ((b >= a) && (lut[(unsigned char) *b])) b--;
	b++;
     }

   len = (unsigned int) (b - a);
   *beg = a;
   *end = b;
   return len;
}

/*}}}*/

/*}}}*/

static int pop_3_strings (char **a, char **b, char **c)
{
   *a = *b = *c = NULL;
   if (-1 == SLpop_string (c))
     return -1;
   
   if (-1 == SLpop_string (b))
     {
	SLfree (*c);
	*c = NULL;
	return -1;
     }

   if (-1 == SLpop_string (a))
     {
	SLfree (*b);
	SLfree (*c);
	*b = *c = NULL;
	return -1;
     }
   
   return 0;
}

static void free_3_strings (char *a, char *b, char *c)
{
   SLfree (a);
   SLfree (b);
   SLfree (c);
}

static void strcat_cmd (void) /*{{{*/
{
   char *c, *c1;
   int nargs;
   int i;
   char **ptrs;
   unsigned int len;
#if !USE_ALLOC_STSTRING
   char buf[256];
#endif
   nargs = SLang_Num_Function_Args;
   if (nargs <= 0) nargs = 2;

   if (NULL == (ptrs = (char **)SLmalloc (nargs * sizeof (char *))))
     return;

   memset ((char *) ptrs, 0, sizeof (char *) * nargs);

   c = NULL;
   i = nargs;
   len = 0;
   while (i != 0)
     {
	char *s;

	i--;
	if (-1 == SLang_pop_slstring (&s))
	  goto free_and_return;
	ptrs[i] = s;
	len += strlen (s);
     }
#if USE_ALLOC_STSTRING
   if (NULL == (c = _SLallocate_slstring (len)))
     goto free_and_return;
#else
   len++;			       /* \0 char */
   if (len <= sizeof (buf))
     c = buf;
   else if (NULL == (c = SLmalloc (len)))
     goto free_and_return;
#endif

   c1 = c;
   for (i = 0; i < nargs; i++)
     {
	strcpy (c1, ptrs[i]);
	c1 += strlen (c1);
     }
   
   free_and_return:
   for (i = 0; i < nargs; i++)
     SLang_free_slstring (ptrs[i]);
   SLfree ((char *) ptrs);

#if USE_ALLOC_STSTRING
   (void) _SLpush_alloced_slstring (c, len);
#else
   if (c != buf)
     (void) SLang_push_malloced_string (c);   /* NULL ok */
   else
     (void) SLang_push_string (c);
#endif
}

/*}}}*/

static int _SLang_push_nstring (char *a, unsigned int len)
{
   a = SLang_create_nslstring (a, len);
   if (a == NULL)
     return -1;
   
   return _SLang_push_slstring (a);
}


static void strtrim_cmd_internal (char *str, int do_beg, int do_end)
{
   char *beg, *end, *white;
   int free_str;
   unsigned int len;

   /* Go through SLpop_string to get a private copy since it will be
    * modified.
    */
   
   free_str = 0;
   if (SLang_Num_Function_Args == 2)
     {
	white = str;
	if (-1 == SLang_pop_slstring (&str))
	  return;
	free_str = 1;
     }
   else white = NULL;

   beg = str;
   len = do_trim (&beg, do_beg, &end, do_end, white);
   
   (void) _SLang_push_nstring (beg, len);
   if (free_str)
     SLang_free_slstring (str);
}

   
static void strtrim_cmd (char *str)
{
   strtrim_cmd_internal (str, 1, 1);
}

static void strtrim_beg_cmd (char *str)
{
   strtrim_cmd_internal (str, 1, 0);
}

static void strtrim_end_cmd (char *str)
{
   strtrim_cmd_internal (str, 0, 1);
}


static void strcompress_cmd (void) /*{{{*/
{
   char *str, *white, *c;
   unsigned char *s, *beg, *end;
   unsigned int len;
   char pref_char;

   if (SLpop_string (&white)) return;
   if (SLpop_string (&str))
     {
	SLfree (white);
	return;
     }

   /* The first character of white is the preferred whitespace character */
   pref_char = *white;

   beg = (unsigned char *) str;
   (void) do_trim ((char **) &beg, 1, (char **) &end, 1, white);
   SLfree (white);

   /* Determine the effective length */
   len = 0;
   s = (unsigned char *) beg;
   while (s < end)
     {
	len++;
	if (Utility_Char_Table[*s++])
	  {
	     while ((s < end) && Utility_Char_Table[*s]) s++;
	  }
     }

#if USE_ALLOC_STSTRING
   c = _SLallocate_slstring (len);
#else
   c = SLmalloc (len + 1);
#endif
   if (c == NULL)
     {
	SLfree (str);
	return;
     }
   
   s = (unsigned char *) c;

   while (beg < end)
     {
	unsigned char ch = *beg++;
	
	if (0 == Utility_Char_Table[ch])
	  {
	     *s++ = ch;
	     continue;
	  }
	
	*s++ = (unsigned char) pref_char;
	
	while ((beg < end) && Utility_Char_Table[*beg]) 
	  beg++;
     }

   *s = 0;
   
#if USE_ALLOC_STSTRING
   (void) _SLpush_alloced_slstring (c, len);
#else
   SLang_push_malloced_string(c);
#endif

   SLfree(str);
}

/*}}}*/

static int str_replace_cmd_1 (char *orig, char *match, char *rep, unsigned int max_num_replaces,
			      char **new_strp) /*{{{*/
{
   char *s, *t, *new_str;
   unsigned int rep_len, match_len, new_len;
   unsigned int num_replaces;

   *new_strp = NULL;

   match_len = strlen (match);

   if (match_len == 0)
     return 0;

   num_replaces = 0;
   s = orig;
   while (num_replaces < max_num_replaces)
     {
	s = strstr (s, match);
	if (s == NULL)
	  break;
	s += match_len;
	num_replaces++;
     }

   if (num_replaces == 0)
     return 0;

   max_num_replaces = num_replaces;

   rep_len = strlen (rep);

   new_len = (strlen (orig) - num_replaces * match_len) + num_replaces * rep_len;
   new_str = SLmalloc (new_len + 1);
   if (new_str == NULL)
     return -1;
   
   s = orig;
   t = new_str;
   
   for (num_replaces = 0; num_replaces < max_num_replaces; num_replaces++)
     {
	char *next_s;
	unsigned int len;

	next_s = strstr (s, match);    /* cannot be NULL */
	len = (unsigned int) (next_s - s);
	strncpy (t, s, len);
	t += len;
	strcpy (t, rep);
	t += rep_len;
	
	s = next_s + match_len;
     }
   strcpy (t, s);
   *new_strp = new_str;

   return (int) num_replaces;
}

/*}}}*/

static void reverse_string (char *a)
{
   char *b;
   
   b = a + strlen (a);
   while (b > a)
     {
	char ch;

	b--;
	ch = *a;
	*a++ = *b;
	*b = ch;
     }
}

static int strreplace_cmd (int *np)
{   
   char *orig, *match, *rep;
   char *new_str;
   int max_num_replaces;
   int ret;

   max_num_replaces = *np;

   if (-1 == pop_3_strings (&orig, &match, &rep))
     return -1;

   if (max_num_replaces < 0)
     {
	reverse_string (orig);
	reverse_string (match);
	reverse_string (rep);
	ret = str_replace_cmd_1 (orig, match, rep, -max_num_replaces, &new_str);
	if (ret > 0) reverse_string (new_str);
	else if (ret == 0)
	  reverse_string (orig);
     }
   else ret = str_replace_cmd_1 (orig, match, rep, max_num_replaces, &new_str);
   
   if (ret == 0)
     {
	if (-1 == SLang_push_malloced_string (orig))
	  ret = -1;
	orig = NULL;
     }
   else if (ret > 0)
     {
	if (-1 == SLang_push_malloced_string (new_str))
	  ret = -1;
     }

   free_3_strings (orig, match, rep);
   return ret;
}

static int str_replace_cmd (char *orig, char *match, char *rep)
{
   char *s;
   int ret;

   ret = str_replace_cmd_1 (orig, match, rep, 1, &s);
   if (ret == 1)
     (void) SLang_push_malloced_string (s);
   return ret;
}

	
     
static void strtok_cmd (char *str)
{
   _SLString_List_Type sl;
   unsigned char white_buf[256];
   char *s;
   unsigned char *white;
   
   if (SLang_Num_Function_Args == 1)
     white = make_whitespace_lut ();
   else
     {
	white = white_buf;
	make_lut ((unsigned char *)str, white);
	if (-1 == SLang_pop_slstring (&str))
	  return;
     }

   if (-1 == _SLstring_list_init (&sl, 256, 1024))
     goto the_return;

   s = str;
   while (*s != 0)
     {
	char *s0;

	s0 = s;
	/* Skip whitespace */
	while ((*s0 != 0) && (0 != white[(unsigned char)*s0]))
	  s0++;

	if (*s0 == 0)
	  break;

	s = s0;
	while ((*s != 0) && (0 == white[(unsigned char) *s]))
	  s++;

	/* sl deleted upon failure */
	if (-1 == _SLstring_list_append (&sl, SLang_create_nslstring (s0, (unsigned int) (s - s0))))
	  goto the_return;
     }

   /* Deletes sl */
   (void) _SLstring_list_push (&sl);

   the_return:
   if (white == white_buf)
     SLang_free_slstring (str);
}

/* This routine returns the string with text removed between single character
   comment delimiters from the set b and e. */

static void str_uncomment_string_cmd (char *str, char *b, char *e) /*{{{*/
{
   unsigned char chb, che;
   unsigned char *s, *cbeg, *mark;

   if (strlen(b) != strlen(e))
     {
	SLang_doerror ("Comment delimiter length mismatch.");
	return;
     }

   set_utility_char_table (b);

   if (NULL == (str = (char *) SLmake_string(str))) return;

   s = (unsigned char *) str;

   while ((chb = *s++) != 0)
     {
	if (Utility_Char_Table [chb] == 0) continue;

	mark = s - 1;

	cbeg = (unsigned char *) b;
	while (*cbeg != chb) cbeg++;

	che = (unsigned char) *(e + (int) (cbeg - (unsigned char *) b));

	while (((chb = *s++) != 0) && (chb != che));

	if (chb == 0)
	  {
	     /* end of string and end not found.  Just truncate it a return; */
	     *mark = 0;
	     break;
	  }

	strcpy ((char *) mark, (char *)s);
	s = mark;
     }
   SLang_push_malloced_string (str);
}

/*}}}*/

static void str_quote_string_cmd (char *str, char *quotes, int *slash_ptr) /*{{{*/
{
   char *q;
   int slash;
   unsigned int len;
   char *s, *q1;
   unsigned char ch;
   unsigned char *t;

   slash = *slash_ptr;

   if ((slash > 255) || (slash < 0))
     {
	SLang_Error = SL_INVALID_PARM;
	return;
     }

   /* setup the utility table to have 1s at quote char postitions. */
   set_utility_char_table (quotes);

   t = Utility_Char_Table;
   t[(unsigned int) slash] = 1;

   /* calculate length */
   s = str;
   len = 0;
   while ((ch = (unsigned char) *s++) != 0) if (t[ch]) len++;
   len += (unsigned int) (s - str);

   if (NULL != (q = SLmalloc(len)))
     {
	s = str; q1 = q;
	while ((ch = (unsigned char) *s++) != 0)
	  {
	     if (t[ch]) *q1++ = slash;
	     *q1++ = (char) ch;
	  }
	*q1 = 0;
	SLang_push_malloced_string(q);
     }
}

/*}}}*/

/* returns the position of substrin in a string or null */
static int issubstr_cmd (char *a, char *b) /*{{{*/
{
   char *c;

   if (NULL == (c = (char *) strstr(a, b)))
     return 0;

   return 1 + (int) (c - a);
}

/*}}}*/

/* returns to stack string at pos n to n + m of a */
static void substr_cmd (char *a, int *n_ptr, int *m_ptr) /*{{{*/
{
   int n, m;
   int lena;

   n = *n_ptr;
   m = *m_ptr;

   lena = strlen (a);
   if (n > lena) n = lena + 1;
   if (n < 1)
     {
	SLang_Error = SL_INVALID_PARM;
	return;
     }

   n--;
   if (m < 0) m = lena;
   if (n + m > lena) m = lena - n;
   
   (void) _SLang_push_nstring (a + n, (unsigned int) m);
}

/*}}}*/

/* substitute char m at positin string n in string*/
static void strsub_cmd (int *nptr, int *mptr) /*{{{*/
{
   char *a;
   int n, m;
   unsigned int lena;

   if (-1 == SLpop_string (&a))
     return;

   n = *nptr;
   m = *mptr;

   lena = strlen (a);

   if ((n <= 0) || (lena < (unsigned int) n))
     {
	SLang_Error = SL_INVALID_PARM;
	SLfree(a);
	return;
     }

   a[n - 1] = (char) m;

   SLang_push_malloced_string (a);
}

/*}}}*/

static void strup_cmd(void) /*{{{*/
{
   unsigned char c, *a;
   char *str;

   if (SLpop_string (&str))
     return;

   a = (unsigned char *) str;
   while ((c = *a) != 0)
     {
	/* if ((*a >= 'a') && (*a <= 'z')) *a -= 32; */
	*a = UPPER_CASE(c);
	a++;
     }

   SLang_push_malloced_string (str);
}

/*}}}*/

static int isdigit_cmd (char *what) /*{{{*/
{
   return isdigit((unsigned char)*what);
}

/*}}}*/
static int toupper_cmd (int *ch) /*{{{*/
{
   return UPPER_CASE(*ch);
}

/*}}}*/

static int tolower_cmd (int *ch) /*{{{*/
{
   return LOWER_CASE(*ch);
}

/*}}}*/

static void strlow_cmd (void) /*{{{*/
{
   unsigned char c, *a;
   char *str;

   if (SLpop_string(&str)) return;
   a = (unsigned char *) str;
   while ((c = *a) != 0)
     {
	/* if ((*a >= 'a') && (*a <= 'z')) *a -= 32; */
	*a = LOWER_CASE(c);
	a++;
     }

   SLang_push_malloced_string ((char *) str);
}

/*}}}*/

static SLang_Array_Type *do_strchop (char *str, int delim, int quote)
{
   int count;
   char *s0, *elm;
   register char *s1;
   register unsigned char ch;
   int quoted;
   SLang_Array_Type *at;
   char **data;

   if ((quote < 0) || (quote > 255)
       || (delim <= 0) || (delim > 255))
     {
	SLang_Error = SL_INVALID_PARM;
	return NULL;
     }

   s1 = s0 = str;

   quoted = 0;
   count = 1;			       /* at least 1 */
   while (1)
     {
	ch = (unsigned char) *s1++;
	if ((ch == quote) && quote)
	  {
	     if (*s1 == 0)
	       break;

	     s1++;
	     continue;
	  }

	if (ch == delim)
	  {
	     count++;
	     continue;
	  }

	if (ch == 0)
	  break;
     }

   if (NULL == (at = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &count, 1)))
     return NULL;

   data = (char **)at->data;

   count = 0;
   s1 = s0;

   while (1)
     {
	ch = (unsigned char) *s1;

	if ((ch == quote) && quote)
	  {
	     s1++;
	     if (*s1 != 0) s1++;
	     quoted = 1;
	     continue;
	  }

	if ((ch == delim) || (ch == 0))
	  {
	     if (quoted == 0)
	       elm = SLang_create_nslstring (s0, (unsigned int) (s1 - s0));
	     else
	       {
		  register char ch1, *p, *p1;
		  char *tmp;

		  tmp = SLmake_nstring (s0, (unsigned int)(s1 - s0));
		  if (tmp == NULL)
		    break;

		  /* Now unquote it */
		  p = p1 = tmp;
		  do
		    {
		       ch1 = *p1++;
		       if (ch1 == '\\') ch1 = *p1++;
		       *p++ = ch1;
		    }
		  while (ch1 != 0);
		  quoted = 0;

		  elm = SLang_create_slstring (tmp);
		  SLfree (tmp);
	       }

	     if (elm == NULL)
	       break;

	     data[count] = elm;
	     count++;

	     if (ch == 0)
	       return at;

	     s1++;		       /* skip past delim */
	     s0 = s1;		       /* and reset */
	  }
	else s1++;
     }

   SLang_free_array (at);
   return NULL;
}

static void strchop_cmd (char *str, int *q, int *d)
{
   (void) SLang_push_array (do_strchop (str, *q, *d), 1);
}

static void strchopr_cmd (char *str, int *q, int *d)
{
   SLang_Array_Type *at;

   if (NULL != (at = do_strchop (str, *q, *d)))
     {
	char **d0, **d1;

	d0 = (char **) at->data;
	d1 = d0 + (at->num_elements - 1);

	while (d0 < d1)
	  {
	     char *tmp;

	     tmp = *d0;
	     *d0 = *d1;
	     *d1 = tmp;
	     d0++;
	     d1--;
	  }
     }
   SLang_push_array (at, 1);
}

static int strcmp_cmd (char *a, char *b) /*{{{*/
{
   return strcmp(a, b);
}

/*}}}*/

static int strncmp_cmd (char *a, char *b, int *n) /*{{{*/
{
   return strncmp(a, b, (unsigned int) *n);
}

/*}}}*/

static int strlen_cmd (char *s) /*{{{*/
{
   return (int) strlen (s);
}
/*}}}*/

static void extract_element_cmd (char *list, int *nth_ptr, int *delim_ptr)
{
   char buf[1024], *b;

   b = buf;
   if (-1 == SLextract_list_element (list, *nth_ptr, *delim_ptr, buf, sizeof(buf)))
     b = NULL;

   SLang_push_string (b);
}

/* sprintf functionality for S-Lang */

static char *SLdo_sprintf (char *fmt) /*{{{*/
{
   register char *p = fmt, ch;
   char *out = NULL, *outp = NULL;
   char dfmt[1024];	       /* used to hold part of format */
   char *f;
   VOID_STAR varp;
   int want_width, width, precis, use_varp, int_var;
   long long_var;
   unsigned int len = 0, malloc_len = 0, dlen;
   int do_free, guess_size;
#if SLANG_HAS_FLOAT
   int tmp1, tmp2, use_double;
   double x;
#endif
   int use_long = 0;

   while (1)
     {
	while ((ch = *p) != 0)
	  {
	     if (ch == '%')
	       break;
	     p++;
	  }

	/* p points at '%' or 0 */

	dlen = (unsigned int) (p - fmt);

	if (len + dlen >= malloc_len)
	  {
	     malloc_len = len + dlen;
	     if (out == NULL) outp = SLmalloc(malloc_len + 1);
	     else outp = SLrealloc(out, malloc_len + 1);
	     if (NULL == outp)
	       return out;
	     out = outp;
	     outp = out + len;
	  }

	strncpy(outp, fmt, dlen);
	len += dlen;
	outp = out + len;
	*outp = 0;
	if (ch == 0) break;

	/* bump it beyond '%' */
	++p;
	fmt = p;

	f = dfmt;
	*f++ = ch;
	/* handle flag char */
	ch = *p++;

	/* Make sure cases such as "% #g" can be handled. */
	if ((ch == '-') || (ch == '+') || (ch == ' ') || (ch == '#'))
	  {
	     *f++ = ch;
	     ch = *p++;
	     if ((ch == '-') || (ch == '+') || (ch == ' ') || (ch == '#'))
	       {
		  *f++ = ch;
		  ch = *p++;
	       }
	  }


	/* width */
	/* I have got to parse it myself so that I can see how big it needs
	 * to be.
	 */
	want_width = width = 0;
	if (ch == '*')
	  {
	     if (SLang_pop_integer(&width)) return (out);
	     want_width = 1;
	     ch = *p++;
	  }
	else
	  {
	     if (ch == '0')
	       {
		  *f++ = '0';
		  ch = *p++;
	       }

	     while ((ch <= '9') && (ch >= '0'))
	       {
		  width = width * 10 + (ch - '0');
		  ch = *p++;
		  want_width = 1;
	       }
	  }

	if (want_width)
	  {
	     sprintf(f, "%d", width);
	     f += strlen (f);
	  }
	precis = 0;
	/* precision -- also indicates max number of chars from string */
	if (ch == '.')
	  {
	     *f++ = ch;
	     ch = *p++;
	     want_width = 0;
	     if (ch == '*')
	       {
		  if (SLang_pop_integer(&precis)) return (out);
		  ch = *p++;
		  want_width = 1;
	       }
	     else while ((ch <= '9') && (ch >= '0'))
	       {
		  precis = precis * 10 + (ch - '0');
		  ch = *p++;
		  want_width = 1;
	       }
	     if (want_width)
	       {
		  sprintf(f, "%d", precis);
		  f += strlen (f);
	       }
	     else precis = 0;
	  }

	long_var = 0;
	int_var = 0;
	varp = NULL;
	guess_size = 32;
#if SLANG_HAS_FLOAT
	use_double = 0;
#endif
	use_long = 0;
	use_varp = 0;
	do_free = 0;

	if (ch == 'l')
	  {
	     use_long = 1;
	     ch = *p++;
	  }
	else if (ch == 'h') ch = *p++; /* not supported */

	/* Now the actual format specifier */
	switch (ch)
	  {
	   case 'S':
	     _SLstring_intrinsic ();
	     ch = 's';
	     /* drop */
	   case 's':
	     if (SLang_pop_slstring((char **) &varp)) return (out);
	     do_free = 1;
	     guess_size = strlen((char *) varp);
	     use_varp = 1;
	     break;

	   case '%':
	     guess_size = 1;
	     do_free = 0;
	     use_varp = 1;
	     varp = (VOID_STAR) "%";
	     break;

	   case 'c': guess_size = 1;
	     use_long = 0;
	     /* drop */
	   case 'd':
	   case 'i':
	   case 'o':
	   case 'u':
	   case 'X':
	   case 'x':
	     if (SLang_pop_long (&long_var)) return(out);
	     if (use_long == 0)
	       int_var = (int) long_var;
	     else
	       *f++ = 'l';
	     break;

	   case 'f':
	   case 'e':
	   case 'g':
	   case 'E':
	   case 'G':
#if SLANG_HAS_FLOAT
	     if (SLang_pop_double(&x, &tmp1, &tmp2)) return (out);
	     use_double = 1;
	     guess_size = 256;
	     (void) tmp1; (void) tmp2;
	     use_long = 0;
	     break;
#endif
	   case 'p':
	     guess_size = 32;
	     /* Pointer type?? Why?? */
	     if (-1 == SLdo_pop ())
	       return out;
	     varp = (VOID_STAR) _SLang_get_run_stack_pointer ();
	     use_varp = 1;
	     use_long = 0;
	     break;

	   default:
	     SLang_doerror("Invalid Format.");
	     return(out);
	  }
	*f++ = ch; *f = 0;

	width = width + precis;
	if (width > guess_size) guess_size = width;

	if (len + guess_size > malloc_len)
	  {
	     outp = (char *) SLrealloc(out, len + guess_size + 1);
	     if (outp == NULL)
	       {
		  SLang_Error = SL_MALLOC_ERROR;
		  return (out);
	       }
	     out = outp;
	     outp = out + len;
	     malloc_len = len + guess_size;
	  }

	if (use_varp)
	  {
	     sprintf(outp, dfmt, varp);
	     if (do_free) SLang_free_slstring ((char *)varp);
	  }
#if SLANG_HAS_FLOAT
	else if (use_double) sprintf(outp, dfmt, x);
#endif
	else if (use_long) sprintf (outp, dfmt, long_var);
	else sprintf(outp, dfmt, int_var);

	len += strlen(outp);
	outp = out + len;
	fmt = p;
     }

   if (out != NULL)
     {
	outp = SLrealloc (out, (unsigned int) (outp - out) + 1);
	if (outp != NULL) out = outp;
     }

   return (out);
}

/*}}}*/

int _SLstrops_do_sprintf_n (int n) /*{{{*/
{
   char *p;
   char *fmt;
   SLang_Object_Type *ptr;
   int ofs;

   if (-1 == (ofs = SLreverse_stack (n + 1)))
     return -1;

   ptr = _SLang_get_run_stack_base () + ofs;

   if (SLang_pop_slstring(&fmt))
     return -1;

   p = SLdo_sprintf (fmt);
   SLang_free_slstring (fmt);

   SLdo_pop_n (_SLang_get_run_stack_pointer () - ptr);

   if (SLang_Error)
     {
	SLfree (p);
	return -1;
     }
   
   return SLang_push_malloced_string (p);
}

/*}}}*/

static void sprintf_n_cmd (int *n)
{
   _SLstrops_do_sprintf_n (*n);
}

static void sprintf_cmd (void)
{
   _SLstrops_do_sprintf_n (SLang_Num_Function_Args - 1);    /* do not include format */
}

/* converts string s to a form that can be used in an eval */
static void make_printable_string(char *s) /*{{{*/
{
   unsigned int len;
   register char *s1 = s, ch, *ss1;
   char *ss;

   /* compute length */
   len = 3;
   while ((ch = *s1++) != 0)
     {
	if ((ch == '\n') || (ch == '\\') || (ch == '"')) len++;
	len++;
     }

   if (NULL == (ss = SLmalloc(len)))
     return;

   s1 = s;
   ss1 = ss;
   *ss1++ = '"';
   while ((ch = *s1++) != 0)
     {
	if (ch == '\n')
	  {
	     ch = 'n';
	     *ss1++ = '\\';
	  }
	else if ((ch == '\\') || (ch == '"'))
	  {
	     *ss1++ = '\\';
	  }
	*ss1++ = ch;
     }
   *ss1++ = '"';
   *ss1 = 0;
   if (-1 == SLang_push_string (ss))
     SLfree (ss);
}

/*}}}*/

static int is_list_element_cmd (char *list, char *elem, int *d_ptr)
{
   char ch;
   int d, n;
   unsigned int len;
   char *lbeg, *lend;

   d = *d_ptr;

   len = strlen (elem);

   n = 1;
   lend = list;

   while (1)
     {
	lbeg = lend;
	while ((0 != (ch = *lend)) && (ch != (char) d)) lend++;

	if ((lbeg + len == lend)
	    && (0 == strncmp (elem, lbeg, len)))
	  break;

	if (ch == 0)
	  {
	     n = 0;
	     break;
	  }
	lend++;			       /* skip delim */
	n++;
     }

   return n;
}

/*}}}*/

/* Regular expression routines for strings */
static SLRegexp_Type regexp_reg;

static int string_match_cmd (char *str, char *pat, int *nptr) /*{{{*/
{
   int n;
   unsigned int len;
   unsigned char rbuf[512], *match;

   n = *nptr;

   regexp_reg.case_sensitive = 1;
   regexp_reg.buf = rbuf;
   regexp_reg.pat = (unsigned char *) pat;
   regexp_reg.buf_len = sizeof (rbuf);

   if (SLang_regexp_compile (&regexp_reg))
     {
	SLang_verror (SL_INVALID_PARM, "Unable to compile pattern");
	return -1;
     }

   n--;
   len = strlen(str);
   if ((n < 0) || ((unsigned int) n > len))
     {
	/* SLang_Error = SL_INVALID_PARM; */
	return 0;
     }

   str += n;
   len -= n;

   if (NULL == (match = SLang_regexp_match((unsigned char *) str, len, &regexp_reg)))
     return 0;

   /* adjust offsets */
   regexp_reg.offset = n;

   return (1 + (int) ((char *) match - str));
}

/*}}}*/

static int string_match_nth_cmd (int *nptr) /*{{{*/
{
   int n, beg;

   n = *nptr;

   if ((n < 0) || (n > 9) || (regexp_reg.pat == NULL)
       || ((beg = regexp_reg.beg_matches[n]) == -1))
     {
	SLang_Error = SL_INVALID_PARM;
	return -1;
     }
   SLang_push_integer(beg + regexp_reg.offset);
   return regexp_reg.end_matches[n];
}

/*}}}*/

static char *create_delimited_string (char **list, unsigned int n, 
				      char *delim)
{
   unsigned int len, dlen;
   unsigned int i;
   unsigned int num;
   char *str, *s;

   len = 1;			       /* allow room for \0 char */
   num = 0;
   for (i = 0; i < n; i++)
     {
	if (list[i] == NULL) continue;
	len += strlen (list[i]);
	num++;
     }

   dlen = strlen (delim);
   if (num > 1)
     len += (num - 1) * dlen;

   if (NULL == (str = SLmalloc (len)))
     return NULL;

   *str = 0;
   s = str;
   i = 0;
	
   while (num > 1)
     {
	while (list[i] == NULL)
	  i++;
	
	strcpy (s, list[i]);
	s += strlen (list[i]);
	strcpy (s, delim);
	s += dlen;
	i++;
	num--;
     }
   
   if (num)
     {
	while (list[i] == NULL)
	  i++;
	
	strcpy (s, list[i]);
     }
   
   return str;
}

static void create_delimited_string_cmd (int *nptr)
{
   unsigned int n, i;
   char **strings;
   char *str;

   str = NULL;

   n = 1 + (unsigned int) *nptr;       /* n includes delimiter */

   if (NULL == (strings = (char **)SLmalloc (n * sizeof (char *))))
     {
	SLdo_pop_n (n);
	return;
     }
   memset((char *)strings, 0, n * sizeof (char *));

   i = n;
   while (i != 0)
     {
	i--;
	if (-1 == SLang_pop_slstring (strings + i))
	  goto return_error;
     }

   str = create_delimited_string (strings + 1, (n - 1), strings[0]);
   /* drop */
   return_error:
   for (i = 0; i < n; i++) SLang_free_slstring (strings[i]);
   SLfree ((char *)strings);

   (void) SLang_push_malloced_string (str);   /* NULL Ok */
}

static void strjoin_cmd (char *delim)
{
   SLang_Array_Type *at;
   char *str;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
     return;
   
   str = create_delimited_string ((char **)at->data, at->num_elements, delim);
   SLang_free_array (at);
   (void) SLang_push_malloced_string (str);   /* NULL Ok */
}

static void str_delete_chars_cmd (char *s, char *d)
{
   unsigned char lut[256];
   unsigned char *s1, *s2;
   unsigned char ch;

   make_lut ((unsigned char *)d, lut);
   if (NULL == (s = SLmake_string (s)))
     return;

   s1 = s2 = (unsigned char *) s;
   while ((ch = *s2++) != 0)
     {
	if (0 == lut[ch])
	  *s1++ = ch;
     }
   *s1 = 0;
   
   (void) SLang_push_malloced_string (s);
}

static unsigned char *make_lut_string (unsigned char *s)
{
   unsigned char lut[256];
   unsigned char *l;
   unsigned int i;

   /* Complement-- a natural order is imposed */
   make_lut (s, lut);
   l = lut;
   for (i = 1; i < 256; i++)
     {
	if (lut[i])
	  *l++ = (unsigned char) i;
     }
   *l = 0;
   return (unsigned char *) SLmake_string ((char *)lut);
}

static unsigned char *make_str_range (unsigned char *s)
{
   unsigned char *s1, *range;
   unsigned int num;
   unsigned char ch;
   int len;

   if (*s == '^')
     return make_lut_string (s);

   num = 0;
   s1 = s;
   while ((ch = *s1++) != 0)
     {
	unsigned char ch1;

	ch1 = *s1;
	if (ch1 == '-')
	  {
	     s1++;
	     ch1 = *s1;
	     len = (int)ch1 - (int)ch;
	     if (len < 0)
	       len = -len;
	     
	     num += (unsigned int) len;
	     if (ch1 != 0)
	       s1++;
	  }

	num++;
     }
   
   range = (unsigned char *)SLmalloc (num + 1);
   if (range == NULL)
     return NULL;
   
   s1 = s;
   s = range;
   while ((ch = *s1++) != 0)
     {
	unsigned char ch1;
	unsigned int i;

	ch1 = *s1;
	if (ch1 != '-')
	  {
	     *s++ = ch;
	     continue;
	  }

	s1++;
	ch1 = *s1;
	
	if (ch > ch1)
	  {
	     if (ch1 == 0)
	       ch1 = 1;

	     for (i = (unsigned int) ch; i >= (unsigned int) ch1; i--)
	       *s++ = (unsigned char) i;
	     
	     if (*s1 == 0)
	       break;
	  }
	else
	  {
	     for (i = (unsigned int) ch; i <= (unsigned int) ch1; i++)
	       *s++ = (unsigned char) i;
	  }
	s1++;
     }
   
#if 0
   if (range + num != s)
     SLang_verror (SL_INTERNAL_ERROR, "make_str_range: num wrong");
#endif
   *s = 0;

   return range;
}

static void strtrans_cmd (char *s, unsigned char *from, unsigned char *to)
{
   unsigned char map[256];
   char *s1;
   unsigned int i;
   unsigned char ch;
   unsigned char last_to;
   unsigned char *from_range, *to_range;

   for (i = 0; i < 256; i++) map[i] = (unsigned char) i;

   if (*to == 0)
     {
	str_delete_chars_cmd (s, (char *)from);
	return;
     }

   from_range = make_str_range (from);
   if (from_range == NULL)
     return;
   to_range = make_str_range (to);
   if (to_range == NULL)
     {
	SLfree ((char *)from_range);
	return;
     }

   from = from_range;
   to = to_range;

   last_to = 0;
   while ((ch = *from++) != 0)
     {
	unsigned char to_ch;

	if (0 == (to_ch = *to++))
	  {
	     do
	       {
		  map[ch] = last_to;
	       }
	     while (0 != (ch = *from++));
	     break;
	  }
	
	last_to = map[ch] = to_ch;
     }

   SLfree ((char *)from_range);
   SLfree ((char *)to_range);

   s = SLmake_string (s);
   if (s == NULL)
     return;

   s1 = s;
   while ((ch = (unsigned char) *s1) != 0)
     *s1++ = (char) map[ch];
   
   (void) SLang_push_malloced_string (s);
}


static SLang_Intrin_Fun_Type Strops_Table [] = /*{{{*/
{
   MAKE_INTRINSIC_I("create_delimited_string",  create_delimited_string_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SS("strcmp",  strcmp_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_SSI("strncmp",  strncmp_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("strcat",  strcat_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strlen",  strlen_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_SII("strchop", strchop_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SII("strchopr", strchopr_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_I("strreplace", strreplace_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_SSS("str_replace", str_replace_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_SII("substr",  substr_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SS("is_substr",  issubstr_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_II("strsub",  strsub_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SII("extract_element", extract_element_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SSI("is_list_element", is_list_element_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_SSI("string_match", string_match_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_I("string_match_nth", string_match_nth_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("strlow", strlow_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_I("tolower", tolower_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_I("toupper", toupper_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("strup", strup_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("isdigit",  isdigit_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_S("strtrim", strtrim_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strtrim_end", strtrim_end_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strtrim_beg", strtrim_beg_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strcompress", strcompress_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_I("Sprintf", sprintf_n_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("sprintf", sprintf_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("sscanf", _SLang_sscanf, SLANG_INT_TYPE),
   MAKE_INTRINSIC_S("make_printable_string", make_printable_string, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SSI("str_quote_string", str_quote_string_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SSS("str_uncomment_string", str_uncomment_string_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_II("define_case", SLang_define_case, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strtok", strtok_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strjoin", strjoin_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SSS("strtrans", strtrans_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SS("str_delete_chars", str_delete_chars_cmd, SLANG_VOID_TYPE),

   SLANG_END_INTRIN_FUN_TABLE
};

/*}}}*/

int _SLang_init_slstrops (void)
{
   return SLadd_intrin_fun_table (Strops_Table, NULL);
}
