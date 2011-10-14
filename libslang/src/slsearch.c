/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */
#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

#ifdef upcase
# undef upcase
#endif

#define upcase(ch) (cs ? ch : UPPER_CASE(ch))

static unsigned char *search_forward (register unsigned char *beg,
				      unsigned char *end,
				      unsigned char *key,
				      register int key_len, int cs, int *ind)
{
   register unsigned char char1;
   unsigned char *pos;
   int j, str_len;
   register unsigned char ch;
   register int db;

   str_len = (int) (end - beg);
   if (str_len < key_len) return (NULL);
   
   if (key_len == 0)
     return NULL;

   char1 = key[key_len - 1];
   beg += (key_len - 1);

   while(1)
     {
	if (cs) while (beg < end)
	  {
	     ch = *beg;
	     db = ind[(unsigned char) ch];
	     if ((db < key_len) && (ch == char1)) break;
	     beg += db; /* ind[(unsigned char) ch]; */
	  }
	else while (beg < end)
	  {
	     ch = *beg;
	     db = ind[(unsigned char) ch];
	     if ((db < key_len) &&
		 (UPPER_CASE(ch) == char1)) break;
	     beg += db; /* ind[(unsigned char) ch]; */
	  }

	if (beg >= end) return(NULL);

	pos = beg - (key_len - 1);
	for (j = 0; j < key_len; j++)
	  {
	     ch = upcase(pos[j]);
	     if (ch != (unsigned char) key[j]) break;
	  }

	if (j == key_len) return(pos);
	beg += 1;
     }
}

static unsigned char *search_backward (unsigned char *beg,unsigned char *end,
				       unsigned char *key, int key_len,
				       int cs, int *ind)
{
   unsigned char ch, char1;
   int j, str_len, ofs;

    str_len = (int) (end - beg);
    if (str_len < key_len) return (NULL);
   
   if (key_len == 0)
     return NULL;
	
   /*  end -= (key_len - 1); */
   end -= key_len;

    char1 = key[0];

    while(1)
      {
	 while ((beg <= end) && (ch = *end, ch = upcase(ch), ch != char1))
	   {
	      ofs = ind[(unsigned char) ch];
#ifdef __MSDOS__
	      /* This is needed for msdos segment wrapping problems */
	      if (beg + ofs > end) return(NULL);
#endif
	      end -= ofs;
	   }
	 if (beg > end) return(NULL);
	 for (j = 1; j < key_len; j++)
	   {
	      ch = upcase(end[j]);
	      if (ch != key[j]) break;
	   }
	 if (j == key_len) return(end);
	 end--;
      }
}

unsigned char *SLsearch (unsigned char *pmin, unsigned char *pmax,
			 SLsearch_Type *st)
{
   if (st->dir > 0) return search_forward (pmin, pmax, st->key,
					   st->key_len, st->cs, st->ind);
   else return search_backward (pmin, pmax, st->key,
				st->key_len, st->cs, st->ind);
}

static int Case_Tables_Ok;

int SLsearch_init (char *str, int dir, int cs, SLsearch_Type *st)
{
   int i, maxi;
   register int max = strlen(str);
   unsigned char *w, *work = st->key;
   register int *indp, *indpm;
   int *ind = st->ind;

   if (max >= (int) sizeof (st->key))
     {
	SLang_doerror ("Search string too long.");
	return -1;
     }

   st->dir = dir; st->cs = cs;

   if (!Case_Tables_Ok) SLang_init_case_tables ();

   if (dir > 0)
     {
	w = work;
     }
   else
     {
	maxi = max - 1;
	str = str + maxi;
	w = work + maxi;
     }

   /* for (i = 0; i < 256; i++) ind[i] = max; */
   indp = ind; indpm = ind + 256;
   while (indp < indpm)
     {
	*indp++ = max;
	*indp++ = max;
	*indp++ = max;
	*indp++ = max;
     }

   i = 0;
   if (cs) while (i < max)
     {
	i++;
	maxi = max - i;
	*w = *str;
	ind[(unsigned char) *str] = maxi;
	str += dir; w += dir;
     }
   else while (i < max)
     {
	i++;
	maxi = max - i;
	*w = UPPER_CASE(*str);
	ind[(unsigned char) *w] = maxi;
	ind[(unsigned char) LOWER_CASE(*str)] = maxi;
	str += dir; w += dir;
     }

   work[max] = 0;
   st->key_len = max;
   return max;
}

/* 8bit clean upper and lowercase macros */
unsigned char _SLChg_LCase_Lut[256];
unsigned char _SLChg_UCase_Lut[256];

void SLang_define_case (int *u, int *l)
{
   unsigned char up = (unsigned char) *u, dn = (unsigned char) *l;

   _SLChg_LCase_Lut[up] = dn;
   _SLChg_LCase_Lut[dn] = dn;
   _SLChg_UCase_Lut[dn] = up;
   _SLChg_UCase_Lut[up] = up;
}

void SLang_init_case_tables (void)
{
   int i, j;
   if (Case_Tables_Ok) return;

   for (i = 0; i < 256; i++)
     {
	_SLChg_UCase_Lut[i] = i;
	_SLChg_LCase_Lut[i] = i;
     }

   for (i = 'A'; i <= 'Z'; i++)
     {
	j = i + 32;
	_SLChg_UCase_Lut[j] = i;
	_SLChg_LCase_Lut[i] = j;
     }
#ifdef PC_SYSTEM
   /* Initialize for DOS code page 437. */
   _SLChg_UCase_Lut[135] = 128; _SLChg_LCase_Lut[128] = 135;
   _SLChg_UCase_Lut[132] = 142; _SLChg_LCase_Lut[142] = 132;
   _SLChg_UCase_Lut[134] = 143; _SLChg_LCase_Lut[143] = 134;
   _SLChg_UCase_Lut[130] = 144; _SLChg_LCase_Lut[144] = 130;
   _SLChg_UCase_Lut[145] = 146; _SLChg_LCase_Lut[146] = 145;
   _SLChg_UCase_Lut[148] = 153; _SLChg_LCase_Lut[153] = 148;
   _SLChg_UCase_Lut[129] = 154; _SLChg_LCase_Lut[154] = 129;
   _SLChg_UCase_Lut[164] = 165; _SLChg_LCase_Lut[165] = 164;
#else
   /* ISO Latin */
   for (i = 192; i <= 221; i++)
     {
	j = i + 32;
	_SLChg_UCase_Lut[j] = i;
	_SLChg_LCase_Lut[i] = j;
     }
   _SLChg_UCase_Lut[215] = 215; _SLChg_LCase_Lut[215] = 215;
   _SLChg_UCase_Lut[223] = 223; _SLChg_LCase_Lut[223] = 223;
   _SLChg_UCase_Lut[247] = 247; _SLChg_LCase_Lut[247] = 247;
   _SLChg_UCase_Lut[255] = 255; _SLChg_LCase_Lut[255] = 255;
#endif
   Case_Tables_Ok = 1;
}
