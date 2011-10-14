/* Copyright (c) 1998, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */
#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

#define MAX_TOKEN_LEN 254
#define MAX_FILE_LINE_LEN 256

static char Empty_Line[1] = {0};

static int Default_Compile_Line_Num_Info;
static char *Input_Line = Empty_Line;
static char *Input_Line_Pointer;

static SLPreprocess_Type *This_SLpp;

static SLang_Load_Type *LLT;

static char *map_token_to_string (_SLang_Token_Type *tok)
{
   char *s;
   static char numbuf [32];
   unsigned char type;
   s = NULL;

   if (tok != NULL) type = tok->type;
   else type = 0;

   switch (type)
     {
      case 0:
	s = "??";
	break;

      case CHAR_TOKEN:
      case SHORT_TOKEN:
      case INT_TOKEN:
      case LONG_TOKEN:
	s = numbuf;
	sprintf (s, "%ld", tok->v.long_val);
	break;

      case UCHAR_TOKEN:
      case USHORT_TOKEN:
      case UINT_TOKEN:
      case ULONG_TOKEN:
	s = numbuf;
	sprintf (s, "%lu", (unsigned long)tok->v.long_val);
	break;

      case OBRACKET_TOKEN: s = "["; break;
      case CBRACKET_TOKEN: s = "]"; break;
      case OPAREN_TOKEN: s = "("; break;
      case CPAREN_TOKEN: s = ")"; break;
      case OBRACE_TOKEN: s = "{"; break;
      case CBRACE_TOKEN: s = "}"; break;
      case DEREF_TOKEN: s = "@"; break;
      case POUND_TOKEN: s = "#"; break;
      case COMMA_TOKEN: s = ","; break;
      case SEMICOLON_TOKEN: s = ";"; break;
      case COLON_TOKEN: s = ":"; break;

#if SLANG_HAS_FLOAT
      case FLOAT_TOKEN:
      case DOUBLE_TOKEN:
      case COMPLEX_TOKEN:
#endif
      case IDENT_TOKEN:
	if ((tok->free_sval_flag == 0) || (tok->num_refs == 0))
	  break;
	/* drop */
      default:
	s = tok->v.s_val;
	break;
     }

   if (s == NULL)
     {
	s = numbuf;
	sprintf (s, "(0x%02X)", type);
     }

   return s;
}

static char *make_line_file_error (char *buf, unsigned int buflen,
				   _SLang_Token_Type *tok, char *dsc, int line, char *file)
{
#if _SLANG_HAS_DEBUG_CODE
   if (tok != NULL) line = tok->line_number;
#endif
   if (file == NULL) file = "??";

   (void) _SLsnprintf (buf, buflen, "%s: found '%s', line %d, file: %s",
		       dsc, map_token_to_string (tok), line, file);

   return buf;
}

void _SLparse_error(char *str, _SLang_Token_Type *tok, int flag)
{
   char buf [1024];

   if (str == NULL)
     str = "Parse Error";

   make_line_file_error (buf, sizeof (buf), tok, str, LLT->line_num, (char *) LLT->name);

   if ((flag == 0) && SLang_Error)
     return;

   SLang_verror (SL_SYNTAX_ERROR, "%s", buf);
}

static void do_line_file_error (int line, char *file)
{
   SLang_verror (SL_SYNTAX_ERROR,
		 "called from line %d, file: %s", line, file);
}

#define ALPHA_CHAR 	1
#define DIGIT_CHAR	2
#define EXCL_CHAR 	3
#define SEP_CHAR	4
#define OP_CHAR		5
#define DOT_CHAR	6
#define BOLDOT_CHAR	7
#define DQUOTE_CHAR	8
#define QUOTE_CHAR	9
#define COMMENT_CHAR	10
#define NL_CHAR		11
#define BAD_CHAR	12
#define WHITE_CHAR	13

#define CHAR_EOF	255

#define CHAR_CLASS(c)	(Char_Type_Table[(c)][0])
#define CHAR_DATA(c)	(Char_Type_Table[(c)][1])

/* In this table, if a single character can represent an operator, e.g.,
 * '&' (BAND_TOKEN), then it must be placed before multiple-character
 * operators that begin with the same character, e.g., "&=".  See
 * get_op_token to see how this is exploited.
 *
 * The third character null terminates the operator string.  This is for
 * the token structure.
 */
static SLCONST char Operators [29][4] =
{
#define OFS_EXCL	0
     {'!',	'=',	0, NE_TOKEN},
#define OFS_POUND	1
     {'#',	0,	0, POUND_TOKEN},
#define OFS_BAND	2
     {'&',	0,	0, BAND_TOKEN},
     {'&',	'&',	0, EOF_TOKEN},
     {'&',	'=',	0, BANDEQS_TOKEN},
#define OFS_STAR	5
     {'*',	0,	0, TIMES_TOKEN},
     {'*',	'=',	0, TIMESEQS_TOKEN},
#define OFS_PLUS	7
     {'+',	0,	0, ADD_TOKEN},
     {'+',	'+',	0, PLUSPLUS_TOKEN},
     {'+',	'=',	0, PLUSEQS_TOKEN},
#define OFS_MINUS	10
     {'-',	0,	0, SUB_TOKEN},
     {'-',	'-',	0, MINUSMINUS_TOKEN},
     {'-',	'=',	0, MINUSEQS_TOKEN},
     {'-',	'>',	0, NAMESPACE_TOKEN},
#define OFS_DIV		14
     {'/',	0,	0, DIV_TOKEN},
     {'/',	'=',	0, DIVEQS_TOKEN},
#define OFS_LT		16
     {'<',	0,	0, LT_TOKEN},
     {'<',	'=',	0, LE_TOKEN},
#define OFS_EQS		18
     {'=',	0,	0, ASSIGN_TOKEN},
     {'=',	'=',	0, EQ_TOKEN},
#define OFS_GT		20
     {'>',	0,	0, GT_TOKEN},
     {'>',	'=',	0, GE_TOKEN},
#define OFS_AT		22
     {'@',	0,	0, DEREF_TOKEN},
#define OFS_POW		23
     {'^',	0,	0, POW_TOKEN},
#define OFS_BOR		24
     {'|',	0,	0, BOR_TOKEN},
     {'|',	'|',	0, EOF_TOKEN},
     {'|',	'=',	0, BOREQS_TOKEN},
#define OFS_BNOT	27
     {'~',	0,	0, BNOT_TOKEN},
     {	0,	0,	0, EOF_TOKEN}
};

static SLCONST unsigned char Char_Type_Table[256][2] =
{
 { NL_CHAR, 0 },	/* 0x0 */   { BAD_CHAR, 0 },	/* 0x1 */
 { BAD_CHAR, 0 },	/* 0x2 */   { BAD_CHAR, 0 },	/* 0x3 */
 { BAD_CHAR, 0 },	/* 0x4 */   { BAD_CHAR, 0 },	/* 0x5 */
 { BAD_CHAR, 0 },	/* 0x6 */   { BAD_CHAR, 0 },	/* 0x7 */
 { WHITE_CHAR, 0 },	/* 0x8 */   { WHITE_CHAR, 0 },	/* 0x9 */
 { NL_CHAR, 0 },	/* \n */   { WHITE_CHAR, 0 },	/* 0xb */
 { WHITE_CHAR, 0 },	/* 0xc */   { WHITE_CHAR, 0 },	/* \r */
 { BAD_CHAR, 0 },	/* 0xe */   { BAD_CHAR, 0 },	/* 0xf */
 { BAD_CHAR, 0 },	/* 0x10 */  { BAD_CHAR, 0 },	/* 0x11 */
 { BAD_CHAR, 0 },	/* 0x12 */  { BAD_CHAR, 0 },	/* 0x13 */
 { BAD_CHAR, 0 },	/* 0x14 */  { BAD_CHAR, 0 },	/* 0x15 */
 { BAD_CHAR, 0 },	/* 0x16 */  { BAD_CHAR, 0 },	/* 0x17 */
 { BAD_CHAR, 0 },	/* 0x18 */  { BAD_CHAR, 0 },	/* 0x19 */
 { BAD_CHAR, 0 },	/* 0x1a */  { BAD_CHAR, 0 },	/* 0x1b */
 { BAD_CHAR, 0 },	/* 0x1c */  { BAD_CHAR, 0 },	/* 0x1d */
 { BAD_CHAR, 0 },	/* 0x1e */  { BAD_CHAR, 0 },	/* 0x1f */
 { WHITE_CHAR, 0 },	/* 0x20 */  { EXCL_CHAR, OFS_EXCL },	/* ! */
 { DQUOTE_CHAR, 0 },	/* " */	    { OP_CHAR, OFS_POUND },	/* # */
 { ALPHA_CHAR, 0 },	/* $ */	    { NL_CHAR, 0 },/* % */
 { OP_CHAR, OFS_BAND },	/* & */	    { QUOTE_CHAR, 0 },	/* ' */
 { SEP_CHAR, OPAREN_TOKEN },	/* ( */	    { SEP_CHAR, CPAREN_TOKEN },	/* ) */
 { OP_CHAR, OFS_STAR },	/* * */	    { OP_CHAR, OFS_PLUS},	/* + */
 { SEP_CHAR, COMMA_TOKEN },	/* , */	    { OP_CHAR, OFS_MINUS },	/* - */
 { DOT_CHAR, 0 },	/* . */	    { OP_CHAR, OFS_DIV },	/* / */
 { DIGIT_CHAR, 0 },	/* 0 */	    { DIGIT_CHAR, 0 },	/* 1 */
 { DIGIT_CHAR, 0 },	/* 2 */	    { DIGIT_CHAR, 0 },	/* 3 */
 { DIGIT_CHAR, 0 },	/* 4 */	    { DIGIT_CHAR, 0 },	/* 5 */
 { DIGIT_CHAR, 0 },	/* 6 */	    { DIGIT_CHAR, 0 },	/* 7 */
 { DIGIT_CHAR, 0 },	/* 8 */	    { DIGIT_CHAR, 0 },	/* 9 */
 { SEP_CHAR, COLON_TOKEN },	/* : */	    { SEP_CHAR, SEMICOLON_TOKEN },	/* ; */
 { OP_CHAR, OFS_LT },	/* < */	    { OP_CHAR, OFS_EQS },	/* = */
 { OP_CHAR, OFS_GT },	/* > */	    { BAD_CHAR, 0 },	/* ? */
 { OP_CHAR, OFS_AT},	/* @ */	    { ALPHA_CHAR, 0 },	/* A */
 { ALPHA_CHAR, 0 },	/* B */	    { ALPHA_CHAR, 0 },	/* C */
 { ALPHA_CHAR, 0 },	/* D */	    { ALPHA_CHAR, 0 },	/* E */
 { ALPHA_CHAR, 0 },	/* F */	    { ALPHA_CHAR, 0 },	/* G */
 { ALPHA_CHAR, 0 },	/* H */	    { ALPHA_CHAR, 0 },	/* I */
 { ALPHA_CHAR, 0 },	/* J */	    { ALPHA_CHAR, 0 },	/* K */
 { ALPHA_CHAR, 0 },	/* L */	    { ALPHA_CHAR, 0 },	/* M */
 { ALPHA_CHAR, 0 },	/* N */	    { ALPHA_CHAR, 0 },	/* O */
 { ALPHA_CHAR, 0 },	/* P */	    { ALPHA_CHAR, 0 },	/* Q */
 { ALPHA_CHAR, 0 },	/* R */	    { ALPHA_CHAR, 0 },	/* S */
 { ALPHA_CHAR, 0 },	/* T */	    { ALPHA_CHAR, 0 },	/* U */
 { ALPHA_CHAR, 0 },	/* V */	    { ALPHA_CHAR, 0 },	/* W */
 { ALPHA_CHAR, 0 },	/* X */	    { ALPHA_CHAR, 0 },	/* Y */
 { ALPHA_CHAR, 0 },	/* Z */	    { SEP_CHAR, OBRACKET_TOKEN },	/* [ */
 { BAD_CHAR, 0 },	/* \ */	    { SEP_CHAR, CBRACKET_TOKEN },	/* ] */
 { OP_CHAR, OFS_POW },	/* ^ */	    { ALPHA_CHAR, 0 },	/* _ */
 { BAD_CHAR, 0 },	/* ` */	    { ALPHA_CHAR, 0 },	/* a */
 { ALPHA_CHAR, 0 },	/* b */	    { ALPHA_CHAR, 0 },	/* c */
 { ALPHA_CHAR, 0 },	/* d */	    { ALPHA_CHAR, 0 },	/* e */
 { ALPHA_CHAR, 0 },	/* f */	    { ALPHA_CHAR, 0 },	/* g */
 { ALPHA_CHAR, 0 },	/* h */	    { ALPHA_CHAR, 0 },	/* i */
 { ALPHA_CHAR, 0 },	/* j */	    { ALPHA_CHAR, 0 },	/* k */
 { ALPHA_CHAR, 0 },	/* l */	    { ALPHA_CHAR, 0 },	/* m */
 { ALPHA_CHAR, 0 },	/* n */	    { ALPHA_CHAR, 0 },	/* o */
 { ALPHA_CHAR, 0 },	/* p */	    { ALPHA_CHAR, 0 },	/* q */
 { ALPHA_CHAR, 0 },	/* r */	    { ALPHA_CHAR, 0 },	/* s */
 { ALPHA_CHAR, 0 },	/* t */	    { ALPHA_CHAR, 0 },	/* u */
 { ALPHA_CHAR, 0 },	/* v */	    { ALPHA_CHAR, 0 },	/* w */
 { ALPHA_CHAR, 0 },	/* x */	    { ALPHA_CHAR, 0 },	/* y */
 { ALPHA_CHAR, 0 },	/* z */	    { SEP_CHAR, OBRACE_TOKEN },	/* { */
 { OP_CHAR, OFS_BOR },	/* | */	    { SEP_CHAR, CBRACE_TOKEN },	/* } */
 { OP_CHAR, OFS_BNOT },	/* ~ */	    { BAD_CHAR, 0 },	/* 0x7f */
   
 { ALPHA_CHAR, 0 },	/* € */	    { ALPHA_CHAR, 0 },	/*  */
 { ALPHA_CHAR, 0 },	/* ‚ */	    { ALPHA_CHAR, 0 },	/* ƒ */
 { ALPHA_CHAR, 0 },	/* „ */	    { ALPHA_CHAR, 0 },	/* … */
 { ALPHA_CHAR, 0 },	/* † */	    { ALPHA_CHAR, 0 },	/* ‡ */
 { ALPHA_CHAR, 0 },	/* ˆ */	    { ALPHA_CHAR, 0 },	/* ‰ */
 { ALPHA_CHAR, 0 },	/* Š */	    { ALPHA_CHAR, 0 },	/* ‹ */
 { ALPHA_CHAR, 0 },	/* Œ */	    { ALPHA_CHAR, 0 },	/*  */
 { ALPHA_CHAR, 0 },	/* Ž */	    { ALPHA_CHAR, 0 },	/*  */
 { ALPHA_CHAR, 0 },	/*  */	    { ALPHA_CHAR, 0 },	/* ‘ */
 { ALPHA_CHAR, 0 },	/* ’ */	    { ALPHA_CHAR, 0 },	/* “ */
 { ALPHA_CHAR, 0 },	/* ” */	    { ALPHA_CHAR, 0 },	/* • */
 { ALPHA_CHAR, 0 },	/* – */	    { ALPHA_CHAR, 0 },	/* — */
 { ALPHA_CHAR, 0 },	/* ˜ */	    { ALPHA_CHAR, 0 },	/* ™ */
 { ALPHA_CHAR, 0 },	/* š */	    { ALPHA_CHAR, 0 },	/* › */
 { ALPHA_CHAR, 0 },	/* œ */	    { ALPHA_CHAR, 0 },	/*  */
 { ALPHA_CHAR, 0 },	/* ž */	    { ALPHA_CHAR, 0 },	/* Ÿ */
 { ALPHA_CHAR, 0 },	/*   */	    { ALPHA_CHAR, 0 },	/* ¡ */
 { ALPHA_CHAR, 0 },	/* ¢ */	    { ALPHA_CHAR, 0 },	/* £ */
 { ALPHA_CHAR, 0 },	/* ¤ */	    { ALPHA_CHAR, 0 },	/* ¥ */
 { ALPHA_CHAR, 0 },	/* ¦ */	    { ALPHA_CHAR, 0 },	/* § */
 { ALPHA_CHAR, 0 },	/* ¨ */	    { ALPHA_CHAR, 0 },	/* © */
 { ALPHA_CHAR, 0 },	/* ª */	    { ALPHA_CHAR, 0 },	/* « */
 { ALPHA_CHAR, 0 },	/* ¬ */	    { ALPHA_CHAR, 0 },	/* ­ */
 { ALPHA_CHAR, 0 },	/* ® */	    { ALPHA_CHAR, 0 },	/* ¯ */
 { ALPHA_CHAR, 0 },	/* ° */	    { ALPHA_CHAR, 0 },	/* ± */
 { ALPHA_CHAR, 0 },	/* ² */	    { ALPHA_CHAR, 0 },	/* ³ */
 { ALPHA_CHAR, 0 },	/* ´ */	    { ALPHA_CHAR, 0 },	/* µ */
 { ALPHA_CHAR, 0 },	/* ¶ */	    { ALPHA_CHAR, 0 },	/* · */
 { ALPHA_CHAR, 0 },	/* ¸ */	    { ALPHA_CHAR, 0 },	/* ¹ */
 { ALPHA_CHAR, 0 },	/* º */	    { ALPHA_CHAR, 0 },	/* » */
 { ALPHA_CHAR, 0 },	/* ¼ */	    { ALPHA_CHAR, 0 },	/* ½ */
 { ALPHA_CHAR, 0 },	/* ¾ */	    { ALPHA_CHAR, 0 },	/* ¿ */
 { ALPHA_CHAR, 0 },	/* À */	    { ALPHA_CHAR, 0 },	/* Á */
 { ALPHA_CHAR, 0 },	/* Â */	    { ALPHA_CHAR, 0 },	/* Ã */
 { ALPHA_CHAR, 0 },	/* Ä */	    { ALPHA_CHAR, 0 },	/* Å */
 { ALPHA_CHAR, 0 },	/* Æ */	    { ALPHA_CHAR, 0 },	/* Ç */
 { ALPHA_CHAR, 0 },	/* È */	    { ALPHA_CHAR, 0 },	/* É */
 { ALPHA_CHAR, 0 },	/* Ê */	    { ALPHA_CHAR, 0 },	/* Ë */
 { ALPHA_CHAR, 0 },	/* Ì */	    { ALPHA_CHAR, 0 },	/* Í */
 { ALPHA_CHAR, 0 },	/* Î */	    { ALPHA_CHAR, 0 },	/* Ï */
 { ALPHA_CHAR, 0 },	/* Ð */	    { ALPHA_CHAR, 0 },	/* Ñ */
 { ALPHA_CHAR, 0 },	/* Ò */	    { ALPHA_CHAR, 0 },	/* Ó */
 { ALPHA_CHAR, 0 },	/* Ô */	    { ALPHA_CHAR, 0 },	/* Õ */
 { ALPHA_CHAR, 0 },	/* Ö */	    { ALPHA_CHAR, 0 },	/* × */
 { ALPHA_CHAR, 0 },	/* Ø */	    { ALPHA_CHAR, 0 },	/* Ù */
 { ALPHA_CHAR, 0 },	/* Ú */	    { ALPHA_CHAR, 0 },	/* Û */
 { ALPHA_CHAR, 0 },	/* Ü */	    { ALPHA_CHAR, 0 },	/* Ý */
 { ALPHA_CHAR, 0 },	/* Þ */	    { ALPHA_CHAR, 0 },	/* ß */
 { ALPHA_CHAR, 0 },	/* à */	    { ALPHA_CHAR, 0 },	/* á */
 { ALPHA_CHAR, 0 },	/* â */	    { ALPHA_CHAR, 0 },	/* ã */
 { ALPHA_CHAR, 0 },	/* ä */	    { ALPHA_CHAR, 0 },	/* å */
 { ALPHA_CHAR, 0 },	/* æ */	    { ALPHA_CHAR, 0 },	/* ç */
 { ALPHA_CHAR, 0 },	/* è */	    { ALPHA_CHAR, 0 },	/* é */
 { ALPHA_CHAR, 0 },	/* ê */	    { ALPHA_CHAR, 0 },	/* ë */
 { ALPHA_CHAR, 0 },	/* ì */	    { ALPHA_CHAR, 0 },	/* í */
 { ALPHA_CHAR, 0 },	/* î */	    { ALPHA_CHAR, 0 },	/* ï */
 { ALPHA_CHAR, 0 },	/* ð */	    { ALPHA_CHAR, 0 },	/* ñ */
 { ALPHA_CHAR, 0 },	/* ò */	    { ALPHA_CHAR, 0 },	/* ó */
 { ALPHA_CHAR, 0 },	/* ô */	    { ALPHA_CHAR, 0 },	/* õ */
 { ALPHA_CHAR, 0 },	/* ö */	    { ALPHA_CHAR, 0 },	/* ÷ */
 { ALPHA_CHAR, 0 },	/* ø */	    { ALPHA_CHAR, 0 },	/* ù */
 { ALPHA_CHAR, 0 },	/* ú */	    { ALPHA_CHAR, 0 },	/* û */
 { ALPHA_CHAR, 0 },	/* ü */	    { ALPHA_CHAR, 0 },	/* ý */
 { ALPHA_CHAR, 0 },	/* þ */	    { ALPHA_CHAR, 0 },	/* ÿ */
};

int _SLcheck_identifier_syntax (char *name)
{
   unsigned char *p;
   
   p = (unsigned char *) name;
   if (ALPHA_CHAR == Char_Type_Table[*p][0]) while (1)
     {
	unsigned ch;
	unsigned char type;

	ch = *++p;

	type = Char_Type_Table [ch][0];
	if ((type != ALPHA_CHAR) && (type != DIGIT_CHAR))
	  {
	     if (ch == 0)
	       return 0;
	     break;
	  }
     }
   
   SLang_verror (SL_SYNTAX_ERROR, 
		 "Identifier or structure field name '%s' contains an illegal character", name);
   return -1;
}

static unsigned char prep_get_char (void)
{
   register unsigned char ch;

   if (0 != (ch = *Input_Line_Pointer++))
     return ch;

   Input_Line_Pointer--;
   return 0;
}

static void unget_prep_char (unsigned char ch)
{
   if ((Input_Line_Pointer != Input_Line)
       && (ch != 0))
     Input_Line_Pointer--;
   /* *Input_Line_Pointer = ch; -- Do not modify the Input_Line */
}

#include "keywhash.c"

static int get_ident_token (_SLang_Token_Type *tok, unsigned char *s, unsigned int len)
{
   unsigned char ch;
   unsigned char type;
   Keyword_Table_Type *table;

   while (1)
     {
	ch = prep_get_char ();
	type = CHAR_CLASS (ch);
	if ((type != ALPHA_CHAR) && (type != DIGIT_CHAR))
	  {
	     unget_prep_char (ch);
	     break;
	  }
	s [len++] = ch;
     }

   s[len] = 0;

   /* check if keyword */
   table = is_keyword ((char *) s, len);
   if (table != NULL)
     {
	tok->v.s_val = table->name;
	return (tok->type = table->type);
     }

   tok->v.s_val = _SLstring_make_hashed_string ((char *)s, len, &tok->hash);
   tok->free_sval_flag = 1;
   return (tok->type = IDENT_TOKEN);
}

static int get_number_token (_SLang_Token_Type *tok, unsigned char *s, unsigned int len)
{
   unsigned char ch;
   unsigned char type;

   /* Look for pattern  [0-9.xX]*([eE][-+]?[digits])?[ijfhul]? */
   while (1)
     {
	ch = prep_get_char ();

	type = CHAR_CLASS (ch);
	if ((type != DIGIT_CHAR) && (type != DOT_CHAR))
	  {
	     if ((ch != 'x') && (ch != 'X'))
	       break;
	     /* It must be hex */
	     do
	       {
		  if (len == (MAX_TOKEN_LEN - 1))
		    goto too_long_return_error;

		  s[len++] = ch;
		  ch = prep_get_char ();
		  type = CHAR_CLASS (ch);
	       }
	     while ((type == DIGIT_CHAR) || (type == ALPHA_CHAR));
	     break;
	  }
	if (len == (MAX_TOKEN_LEN - 1))
	  goto too_long_return_error;
	s [len++] = ch;
     }

   /* At this point, type and ch are synchronized */

   if ((ch == 'e') || (ch == 'E'))
     {
	if (len == (MAX_TOKEN_LEN - 1))
	  goto too_long_return_error;
	s[len++] = ch;
	ch = prep_get_char ();
	if ((ch == '+') || (ch == '-'))
	  {
	     if (len == (MAX_TOKEN_LEN - 1))
	       goto too_long_return_error;
	     s[len++] = ch;
	     ch = prep_get_char ();
	  }

	while (DIGIT_CHAR == (type = CHAR_CLASS(ch)))
	  {
	     if (len == (MAX_TOKEN_LEN - 1))
	       goto too_long_return_error;
	     s[len++] = ch;
	     ch = prep_get_char ();
	  }
     }

   while (ALPHA_CHAR == type)
     {
	if (len == (MAX_TOKEN_LEN - 1))
	  goto too_long_return_error;
	s[len++] = ch;
	ch = prep_get_char ();
	type = CHAR_CLASS(ch);
     }

   unget_prep_char (ch);
   s[len] = 0;

   switch (SLang_guess_type ((char *) s))
     {
      default:
	tok->v.s_val = (char *) s;
	_SLparse_error ("Not a number", tok, 0);
	return (tok->type = EOF_TOKEN);

#if SLANG_HAS_FLOAT
      case SLANG_FLOAT_TYPE:
	tok->v.s_val = _SLstring_make_hashed_string ((char *)s, len, &tok->hash);
	tok->free_sval_flag = 1;
	return (tok->type = FLOAT_TOKEN);

      case SLANG_DOUBLE_TYPE:
	tok->v.s_val = _SLstring_make_hashed_string ((char *)s, len, &tok->hash);
	tok->free_sval_flag = 1;
	return (tok->type = DOUBLE_TOKEN);
#endif
#if SLANG_HAS_COMPLEX
      case SLANG_COMPLEX_TYPE:
	tok->v.s_val = _SLstring_make_hashed_string ((char *)s, len, &tok->hash);
	tok->free_sval_flag = 1;
	return (tok->type = COMPLEX_TOKEN);
#endif
      case SLANG_CHAR_TYPE:
	tok->v.long_val = (char)SLatol (s);
	return tok->type = CHAR_TOKEN;
      case SLANG_UCHAR_TYPE:
	tok->v.long_val = (unsigned char)SLatol (s);
	return tok->type = UCHAR_TOKEN;
      case SLANG_SHORT_TYPE:
	tok->v.long_val = (short)SLatol (s);
	return tok->type = SHORT_TOKEN;
      case SLANG_USHORT_TYPE:
	tok->v.long_val = (unsigned short)SLatoul (s);
	return tok->type = USHORT_TOKEN;
      case SLANG_INT_TYPE:
	tok->v.long_val = (int)SLatol (s);
	return tok->type = INT_TOKEN;
      case SLANG_UINT_TYPE:
	tok->v.long_val = (unsigned int)SLatoul (s);
	return tok->type = UINT_TOKEN;
      case SLANG_LONG_TYPE:
	tok->v.long_val = SLatol (s);
	return tok->type = LONG_TOKEN;
      case SLANG_ULONG_TYPE:
	tok->v.long_val = SLatoul (s);
	return tok->type = ULONG_TOKEN;
     }

   too_long_return_error:
   _SLparse_error ("Number too long for buffer", NULL, 0);
   return (tok->type == EOF_TOKEN);
}

static int get_op_token (_SLang_Token_Type *tok, char ch)
{
   unsigned int offset;
   char second_char;
   unsigned char type;
   SLCONST char *name;

   /* operators are: + - / * ++ -- += -= = == != > < >= <= | etc..
    * These lex to the longest valid operator token.
    */

   offset = CHAR_DATA((unsigned char) ch);
   if (0 == Operators [offset][1])
     {
	name = Operators [offset];
	type = name [3];
     }
   else
     {
	type = EOF_TOKEN;
	name = NULL;
     }

   second_char = prep_get_char ();
   do
     {
	if (second_char == Operators[offset][1])
	  {
	     name = Operators [offset];
	     type = name [3];
	     break;
	  }
	offset++;
     }
   while (ch == Operators[offset][0]);

   tok->type = type;

   if (type == EOF_TOKEN)
     {
	_SLparse_error ("Operator not supported", NULL, 0);
	return type;
     }

   tok->v.s_val = (char *)name;

   if (name[1] == 0)
     unget_prep_char (second_char);

   return type;
}

/* If this returns non-zero, then it is a binary string */
static int expand_escaped_string (register char *s,
				  register char *t, register char *tmax,
				  unsigned int *lenp)
{
   char *s0;
   int is_binary = 0;
   char ch;

   s0 = s;
   while (t < tmax)
     {
	ch = *t++;
	if (ch == '\\')
	  {
	     t = _SLexpand_escaped_char (t, &ch);
	     if (ch == 0) is_binary = 1;
	  }
	*s++ = ch;
     }
   *s = 0;

   *lenp = (unsigned char) (s - s0);
   return is_binary;
}

static int get_string_token (_SLang_Token_Type *tok, unsigned char quote_char,
			     unsigned char *s)
{
   unsigned char ch;
   unsigned int len = 0;
   int has_quote = 0;
   int is_binary;

   while (1)
     {
	ch = prep_get_char ();
	if (ch == 0)
	  {
	     _SLparse_error("Expecting quote-character", NULL, 0);
	     return (tok->type = EOF_TOKEN);
	  }
	if (ch == quote_char) break;

	s[len++] = ch;

	if (len == (MAX_TOKEN_LEN - 1))
	  {
	     _SLparse_error ("String too long for buffer", NULL, 0);
	     return (tok->type == EOF_TOKEN);
	  }

	if (ch == '\\')
	  {
	     has_quote = 1;
	     ch = prep_get_char ();
	     s[len++] = ch;
	  }
     }

   s[len] = 0;

   if (has_quote)
     is_binary = expand_escaped_string ((char *) s, (char *)s, (char *)s + len, &len);
   else is_binary = 0;

   if ('"' == quote_char)
     {
	tok->free_sval_flag = 1;
	if (is_binary)
	  {
	     tok->v.b_val = SLbstring_create (s, len);
	     return tok->type = BSTRING_TOKEN;
	  }
	else
	  {
	     tok->v.s_val = _SLstring_make_hashed_string ((char *)s,
							  len,
							  &tok->hash);
	     tok->free_sval_flag = 1;
	     return (tok->type = STRING_TOKEN);
	  }
     }

   /* else single character */
   if ((len == 0) || (s[1] != 0))
     {
	_SLparse_error("Single char expected", NULL, 0);
	return (tok->type = EOF_TOKEN);
     }

   tok->v.long_val = s[0];
   return (tok->type = UCHAR_TOKEN);
}

static int extract_token (_SLang_Token_Type *tok, unsigned char ch, unsigned char t)
{
   unsigned char s [MAX_TOKEN_LEN];
   unsigned int slen;

   s[0] = (char) ch;
   slen = 1;

   switch (t)
     {
      case ALPHA_CHAR:
	return get_ident_token (tok, s, slen);

      case OP_CHAR:
	return get_op_token (tok, ch);

      case DIGIT_CHAR:
	return get_number_token (tok, s, slen);

      case EXCL_CHAR:
	ch = prep_get_char ();
	s [slen++] = ch;
	t = CHAR_CLASS(ch);
	if (t == ALPHA_CHAR) return get_ident_token (tok, s, slen);
	if (t == OP_CHAR)
	  {
	     unget_prep_char (ch);
	     return get_op_token (tok, '!');
	  }
	_SLparse_error("Misplaced !", NULL, 0);
	return -1;

      case DOT_CHAR:
	ch = prep_get_char ();
	if (DIGIT_CHAR == CHAR_CLASS(ch))
	  {
	     s [slen++] = ch;
	     return get_number_token (tok, s, slen);
	  }
	unget_prep_char (ch);
	return (tok->type = DOT_TOKEN);

      case SEP_CHAR:
	return (tok->type = CHAR_DATA(ch));

      case DQUOTE_CHAR:
      case QUOTE_CHAR:
	return get_string_token (tok, ch, s);

      default:
	_SLparse_error("Invalid character", NULL, 0);
	return (tok->type = EOF_TOKEN);
     }
}

int _SLget_rpn_token (_SLang_Token_Type *tok)
{
   unsigned char ch;

   tok->v.s_val = "??";
   while ((ch = *Input_Line_Pointer) != 0)
     {
	unsigned char t;

	Input_Line_Pointer++;
	if (WHITE_CHAR == (t = CHAR_CLASS(ch)))
	  continue;

	if (NL_CHAR == t)
	  break;

	return extract_token (tok, ch, t);
     }
   Input_Line_Pointer = Empty_Line;
   return EOF_TOKEN;
}

int _SLget_token (_SLang_Token_Type *tok)
{
   unsigned char ch;
   unsigned char t;

   tok->num_refs = 1;
   tok->free_sval_flag = 0;
   tok->v.s_val = "??";
#if _SLANG_HAS_DEBUG_CODE
   tok->line_number = LLT->line_num;
#endif
   if (SLang_Error || (Input_Line == NULL))
     return (tok->type = EOF_TOKEN);

   while (1)
     {
	ch = *Input_Line_Pointer++;
	if (WHITE_CHAR == (t = CHAR_CLASS (ch)))
	  continue;

	if (t != NL_CHAR)
	  return extract_token (tok, ch, t);

	do
	  {
	     LLT->line_num++;
#if _SLANG_HAS_DEBUG_CODE
	     tok->line_number++;
#endif
	     Input_Line = LLT->read(LLT);
	     if ((NULL == Input_Line) || SLang_Error)
	       {
		  Input_Line_Pointer = Input_Line = NULL;
		  return (tok->type = EOF_TOKEN);
	       }
	  }
	while (0 == SLprep_line_ok(Input_Line, This_SLpp));

	Input_Line_Pointer = Input_Line;
	if (*Input_Line_Pointer == '.')
	  {
	     Input_Line_Pointer++;
	     return tok->type = RPN_TOKEN;
	  }
     }
}

static int prep_exists_function (char *line, char comment)
{
   char buf[MAX_FILE_LINE_LEN], *b, *bmax;
   unsigned char ch;

   bmax = buf + (sizeof (buf) - 1);

   while (1)
     {
	/* skip whitespace */
	while ((ch = (unsigned char) *line),
	       ch && (ch != '\n') && (ch <= ' '))
	  line++;

	if ((ch <= '\n')
	    || (ch == (unsigned char) comment)) break;

	b = buf;
	while ((ch = (unsigned char) *line) > ' ')
	  {
	     if (b < bmax) *b++ = (char) ch;
	     line++;
	  }
	*b = 0;

	if (SLang_is_defined (buf))
	  return 1;
     }

   return 0;
}

static int prep_eval_expr (char *expr)
{
   int ret;
   char *end;

   end = strchr (expr, '\n');
   if (end == NULL)
     end = expr + strlen (expr);
   expr = SLmake_nstring (expr, (unsigned int) (end - expr));
   if (expr == NULL)
     return -1;
	
   if ((0 != SLang_load_string (expr))
       || (-1 == SLang_pop_integer (&ret)))
     ret = -1;
   else
     ret = (ret != 0);

   SLfree (expr);
   return ret;
}


int SLang_load_object (SLang_Load_Type *x)
{
   SLPreprocess_Type this_pp;
   SLPreprocess_Type *save_this_pp;
   SLang_Load_Type *save_llt;
   char *save_input_line, *save_input_line_ptr;
#if _SLANG_HAS_DEBUG_CODE
   int save_compile_line_num_info;
#endif
   int save_auto_declare_variables;

   if (SLprep_exists_hook == NULL)
     SLprep_exists_hook = prep_exists_function;

   if (_SLprep_eval_hook == NULL)
     _SLprep_eval_hook = prep_eval_expr;

   if (-1 == SLprep_open_prep (&this_pp)) return -1;

   if (-1 == _SLcompile_push_context (x))
     return -1;

#if _SLANG_HAS_DEBUG_CODE
   save_compile_line_num_info = _SLang_Compile_Line_Num_Info;
#endif
   save_this_pp = This_SLpp;
   save_input_line = Input_Line;
   save_input_line_ptr = Input_Line_Pointer;
   save_llt = LLT;
   save_auto_declare_variables = _SLang_Auto_Declare_Globals;

   This_SLpp = &this_pp;
   Input_Line_Pointer = Input_Line = Empty_Line;
   LLT = x;

   x->line_num = 0;
   x->parse_level = 0;
   _SLang_Auto_Declare_Globals = x->auto_declare_globals;

#if _SLANG_HAS_DEBUG_CODE
   _SLang_Compile_Line_Num_Info = Default_Compile_Line_Num_Info;
#endif

   _SLparse_start (x);
   if (SLang_Error)
     do_line_file_error (x->line_num, x->name);

   _SLang_Auto_Declare_Globals = save_auto_declare_variables;

   if (SLang_Error) SLang_restart (0);

   (void) _SLcompile_pop_context ();

   Input_Line = save_input_line;
   Input_Line_Pointer = save_input_line_ptr;
   LLT = save_llt;
   This_SLpp = save_this_pp;

#if _SLANG_HAS_DEBUG_CODE
   _SLang_Compile_Line_Num_Info = save_compile_line_num_info;
#endif

   if (SLang_Error) return -1;
   return 0;
}

SLang_Load_Type *SLns_allocate_load_type (char *name, char *namespace_name)
{
   SLang_Load_Type *x;

   if (NULL == (x = (SLang_Load_Type *)SLmalloc (sizeof (SLang_Load_Type))))
     return NULL;
   memset ((char *) x, 0, sizeof (SLang_Load_Type));

   if (name == NULL) name = "";

   if (NULL == (x->name = SLang_create_slstring (name)))
     {
	SLfree ((char *) x);
	return NULL;
     }
   
   if (namespace_name != NULL)
     {
	if (NULL == (x->namespace_name = SLang_create_slstring (namespace_name)))
	  {
	     SLang_free_slstring (x->name);
	     SLfree ((char *) x);
	     return NULL;
	  }
     }
   
   return x;
}

SLang_Load_Type *SLallocate_load_type (char *name)
{
   return SLns_allocate_load_type (name, NULL);
}

void SLdeallocate_load_type (SLang_Load_Type *x)
{
   if (x != NULL)
     {
	SLang_free_slstring (x->name);
	SLang_free_slstring (x->namespace_name);
	SLfree ((char *) x);
     }
}

typedef struct
{
   char *string;
   char *ptr;
}
String_Client_Data_Type;

static char *read_from_string (SLang_Load_Type *x)
{
   String_Client_Data_Type *data;
   char *s, *s1, ch;

   data = (String_Client_Data_Type *)x->client_data;
   s1 = s = data->ptr;

   if (*s == 0)
     return NULL;

   while ((ch = *s) != 0)
     {
	s++;
	if (ch == '\n')
	  break;
     }

   data->ptr = s;
   return s1;
}

int SLang_load_string (char *string)
{
   return SLns_load_string (string, NULL);
}

int SLns_load_string (char *string, char *ns_name)
{
   SLang_Load_Type *x;
   String_Client_Data_Type data;
   int ret;

   if (string == NULL)
     return -1;

   /* Grab a private copy in case loading modifies string */
   if (NULL == (string = SLang_create_slstring (string)))
     return -1;

   /* To avoid creating a static data space for every string loaded,
    * all string objects will be regarded as identical.  So, identify
    * all of them by ***string***
    */
   if (NULL == (x = SLns_allocate_load_type ("***string***", ns_name)))
     {
	SLang_free_slstring (string);
	return -1;
     }

   x->client_data = (VOID_STAR) &data;
   x->read = read_from_string;

   data.ptr = data.string = string;
   if (-1 == (ret = SLang_load_object (x)))
     SLang_verror (SLang_Error, "called from eval: %s", string);

   SLang_free_slstring (string);
   SLdeallocate_load_type (x);
   return ret;
}

typedef struct
{
   char *buf;
   FILE *fp;
}
File_Client_Data_Type;

char *SLang_User_Prompt;
static char *read_from_file (SLang_Load_Type *x)
{
   FILE *fp;
   File_Client_Data_Type *c;

   c = (File_Client_Data_Type *)x->client_data;
   fp = c->fp;

   if ((fp == stdin) && (SLang_User_Prompt != NULL))
     {
	fputs (SLang_User_Prompt, stdout);
	fflush (stdout);
     }

   return fgets (c->buf, MAX_FILE_LINE_LEN, c->fp);
}

static int Load_File_Verbose = 0;
int SLang_load_file_verbose (int v)
{
   int v1 = Load_File_Verbose;
   Load_File_Verbose = v;
   return v1;
}

/* Note that file could be freed from Slang during run of this routine
 * so get it and store it !! (e.g., autoloading)
 */
int (*SLang_Load_File_Hook) (char *);
int (*SLns_Load_File_Hook) (char *, char *);
int SLang_load_file (char *f)
{
   return SLns_load_file (f, NULL);
}

int SLns_load_file (char *f, char *ns_name)
{
   File_Client_Data_Type client_data;
   SLang_Load_Type *x;
   char *name, *buf;
   FILE *fp;

   if ((ns_name == NULL) && (NULL != SLang_Load_File_Hook))
     return (*SLang_Load_File_Hook) (f);

   if (SLns_Load_File_Hook != NULL)
     return (*SLns_Load_File_Hook) (f, ns_name);

   if (f == NULL) 
     name = SLang_create_slstring ("<stdin>");
   else
     name = _SLpath_find_file (f);
	
   if (name == NULL)
     return -1;

   if (NULL == (x = SLns_allocate_load_type (name, ns_name)))
     {
	SLang_free_slstring (name);
	return -1;
     }

   buf = NULL;

   if (f != NULL)
     {
	fp = fopen (name, "r");
	if (Load_File_Verbose)
	  SLang_vmessage ("Loading %s", name);
     }
   else
     fp = stdin;

   if (fp == NULL)
     SLang_verror (SL_OBJ_NOPEN, "Unable to open %s", name);
   else if (NULL != (buf = SLmalloc (MAX_FILE_LINE_LEN + 1)))
     {
	client_data.fp = fp;
	client_data.buf = buf;
	x->client_data = (VOID_STAR) &client_data;
	x->read = read_from_file;

	(void) SLang_load_object (x);
     }

   if ((fp != NULL) && (fp != stdin))
     fclose (fp);

   SLfree (buf);
   SLang_free_slstring (name);
   SLdeallocate_load_type (x);

   if (SLang_Error)
     return -1;

   return 0;
}

static char *check_byte_compiled_token (char *buf)
{
   unsigned int len_lo, len_hi, len;

   len_lo = (unsigned char) *Input_Line_Pointer++;
   if ((len_lo < 32)
       || ((len_hi = (unsigned char)*Input_Line_Pointer++) < 32)
       || ((len = (len_lo - 32) | ((len_hi - 32) << 7)) >= MAX_TOKEN_LEN))
     {
	SLang_doerror ("Byte compiled file appears corrupt");
	return NULL;
     }

   SLMEMCPY (buf, Input_Line_Pointer, len);
   buf += len;
   Input_Line_Pointer += len;
   *buf = 0;
   return buf;
}

void _SLcompile_byte_compiled (void)
{
   unsigned char type;
   _SLang_Token_Type tok;
   char buf[MAX_TOKEN_LEN];
   char *ebuf;
   unsigned int len;

   memset ((char *) &tok, 0, sizeof (_SLang_Token_Type));

   while (SLang_Error == 0)
     {
	top_of_switch:
	type = (unsigned char) *Input_Line_Pointer++;
	switch (type)
	  {
	   case '\n':
	   case 0:
	     if (NULL == (Input_Line = LLT->read(LLT)))
	       {
		  Input_Line_Pointer = Input_Line = NULL;
		  return;
	       }
	     Input_Line_Pointer = Input_Line;
	     goto top_of_switch;

	   case LINE_NUM_TOKEN:
	   case CHAR_TOKEN:
	   case UCHAR_TOKEN:
	   case SHORT_TOKEN:
	   case USHORT_TOKEN:
	   case INT_TOKEN:
	   case UINT_TOKEN:
	   case LONG_TOKEN:
	   case ULONG_TOKEN:
	     if (NULL == check_byte_compiled_token (buf))
	       return;
	     tok.v.long_val = atol (buf);
	     break;

	   case COMPLEX_TOKEN:
	   case FLOAT_TOKEN:
	   case DOUBLE_TOKEN:
	     if (NULL == check_byte_compiled_token (buf))
	       return;
	     tok.v.s_val = buf;
	     break;

	   case ESC_STRING_TOKEN:
	     if (NULL == (ebuf = check_byte_compiled_token (buf)))
	       return;
	     tok.v.s_val = buf;
	     if (expand_escaped_string (buf, buf, ebuf, &len))
	       {
		  tok.hash = len;
		  type = _BSTRING_TOKEN;
	       }
	     else
	       {
		  tok.hash = _SLstring_hash ((unsigned char *)buf, (unsigned char *)buf + len);
		  type = STRING_TOKEN;
	       }
	     break;

	   case TMP_TOKEN:
	   case DEFINE_TOKEN:
	   case DEFINE_STATIC_TOKEN:
	   case DEFINE_PRIVATE_TOKEN:
	   case DEFINE_PUBLIC_TOKEN:
	   case DOT_TOKEN:
	   case STRING_TOKEN:
	   case IDENT_TOKEN:
	   case _REF_TOKEN:
	   case _DEREF_ASSIGN_TOKEN:
	   case _SCALAR_ASSIGN_TOKEN:
	   case _SCALAR_PLUSEQS_TOKEN:
	   case _SCALAR_MINUSEQS_TOKEN:
	   case _SCALAR_TIMESEQS_TOKEN:
	   case _SCALAR_DIVEQS_TOKEN:
	   case _SCALAR_BOREQS_TOKEN:
	   case _SCALAR_BANDEQS_TOKEN:
	   case _SCALAR_PLUSPLUS_TOKEN:
	   case _SCALAR_POST_PLUSPLUS_TOKEN:
	   case _SCALAR_MINUSMINUS_TOKEN:
	   case _SCALAR_POST_MINUSMINUS_TOKEN:
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
	     if (NULL == (ebuf = check_byte_compiled_token (buf)))
	       return;
	     tok.v.s_val = buf;
	     tok.hash = _SLstring_hash ((unsigned char *)buf, (unsigned char *)ebuf);
	     break;

	   default:
	     break;
	  }
	tok.type = type;

	(*_SLcompile_ptr) (&tok);
     }
}

static int escape_string (unsigned char *s, unsigned char *smax,
			  unsigned char *buf, unsigned char *buf_max,
			  int *is_escaped)
{
   unsigned char ch;

   *is_escaped = 0;
   while (buf < buf_max)
     {
	if (s == smax)
	  {
	     *buf = 0;
	     return 0;
	  }

	ch = *s++;
	switch (ch)
	  {
	   default:
	     *buf++ = ch;
	     break;

	   case 0:
	     *buf++ = '\\';
	     if (buf < buf_max) *buf++ = 'x';
	     if (buf < buf_max) *buf++ = '0';
	     if (buf < buf_max) *buf++ = '0';
	     *is_escaped = 1;
	     break; /* return 0; */

	   case '\n':
	     *buf++ = '\\';
	     if (buf < buf_max) *buf++ = 'n';
	     *is_escaped = 1;
	     break;

	   case '\r':
	     *buf++ = '\\';
	     if (buf < buf_max) *buf++ = 'r';
	     *is_escaped = 1;
	     break;

	   case 0x1A:		       /* ^Z */
	     *buf++ = '\\';
	     if (buf < buf_max) *buf++ = 'x';
	     if (buf < buf_max) *buf++ = '1';
	     if (buf < buf_max) *buf++ = 'A';
	     *is_escaped = 1;
	     break;

	   case '\\':
	     *buf++ = ch;
	     if (buf < buf_max) *buf++ = ch;
	     *is_escaped = 1;
	     break;
	  }
     }
   _SLparse_error ("String too long to byte-compile", NULL, 0);
   return -1;
}

static FILE *Byte_Compile_Fp;
static unsigned int Byte_Compile_Line_Len;

static int bytecomp_write_data (char *buf, unsigned int len)
{
   char *err = "Write Error";

   if ((Byte_Compile_Line_Len + len + 1) >= MAX_FILE_LINE_LEN)
     {
	if (EOF == fputs ("\n", Byte_Compile_Fp))
	  {
	     SLang_doerror (err);
	     return -1;
	  }
	Byte_Compile_Line_Len = 0;
     }

   if (EOF == fputs (buf, Byte_Compile_Fp))
     {
	SLang_doerror (err);
	return -1;
     }
   Byte_Compile_Line_Len += len;
   return 0;
}

static void byte_compile_token (_SLang_Token_Type *tok)
{
   unsigned char buf [MAX_TOKEN_LEN + 4], *buf_max;
   unsigned int len;
   char *b3;
   int is_escaped;
   unsigned char *s;

   if (SLang_Error) return;

   buf [0] = (unsigned char) tok->type;
   buf [1] = 0;

   buf_max = buf + sizeof(buf);
   b3 = (char *) buf + 3;

   switch (tok->type)
     {
      case LINE_NUM_TOKEN:
      case CHAR_TOKEN:
      case SHORT_TOKEN:
      case INT_TOKEN:
      case LONG_TOKEN:
	sprintf (b3, "%ld", tok->v.long_val);
	break;

      case UCHAR_TOKEN:
      case USHORT_TOKEN:
      case UINT_TOKEN:
      case ULONG_TOKEN:
	sprintf (b3, "%lu", tok->v.long_val);
	break;

      case _BSTRING_TOKEN:
	s = (unsigned char *) tok->v.s_val;
	len = (unsigned int) tok->hash;

	if (-1 == escape_string (s, s + len,
				 (unsigned char *)b3, buf_max,
				 &is_escaped))
	    return;

	buf[0] = ESC_STRING_TOKEN;
	break;

      case BSTRING_TOKEN:
	if (NULL == (s = SLbstring_get_pointer (tok->v.b_val, &len)))
	  return;

	if (-1 == escape_string (s, s + len,
				 (unsigned char *)b3, buf_max,
				 &is_escaped))
	    return;
	buf[0] = ESC_STRING_TOKEN;
	break;

      case STRING_TOKEN:
	s = (unsigned char *)tok->v.s_val;

	if (-1 == escape_string (s, s + strlen ((char *)s),
				 (unsigned char *)b3, buf_max,
				 &is_escaped))
	    return;

	if (is_escaped)
	  buf[0] = ESC_STRING_TOKEN;
	break;

	/* a _SCALAR_* token is attached to an identifier. */
      case _DEREF_ASSIGN_TOKEN:
      case _SCALAR_ASSIGN_TOKEN:
      case _SCALAR_PLUSEQS_TOKEN:
      case _SCALAR_MINUSEQS_TOKEN:
      case _SCALAR_TIMESEQS_TOKEN:
      case _SCALAR_DIVEQS_TOKEN:
      case _SCALAR_BOREQS_TOKEN:
      case _SCALAR_BANDEQS_TOKEN:
      case _SCALAR_PLUSPLUS_TOKEN:
      case _SCALAR_POST_PLUSPLUS_TOKEN:
      case _SCALAR_MINUSMINUS_TOKEN:
      case _SCALAR_POST_MINUSMINUS_TOKEN:
      case DOT_TOKEN:
      case TMP_TOKEN:
      case DEFINE_TOKEN:
      case DEFINE_STATIC_TOKEN:
      case DEFINE_PRIVATE_TOKEN:
      case DEFINE_PUBLIC_TOKEN:
      case FLOAT_TOKEN:
      case DOUBLE_TOKEN:
      case COMPLEX_TOKEN:
      case IDENT_TOKEN:
      case _REF_TOKEN:
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
	strcpy (b3, tok->v.s_val);
	break;

      default:
	b3 = NULL;
     }

   if (b3 != NULL)
     {
	len = strlen (b3);
	buf[1] = (unsigned char) ((len & 0x7F) + 32);
	buf[2] = (unsigned char) (((len >> 7) & 0x7F) + 32);
	len += 3;
     }
   else len = 1;

   (void) bytecomp_write_data ((char *)buf, len);
}

int SLang_byte_compile_file (char *name, int method)
{
   char file [1024];

   (void) method;
   if (strlen (name) + 2 >= sizeof (file))
     {
	SLang_verror (SL_INVALID_PARM, "Filename too long");
	return -1;
     }
   sprintf (file, "%sc", name);
   if (NULL == (Byte_Compile_Fp = fopen (file, "w")))
     {
	SLang_verror(SL_OBJ_NOPEN, "%s: unable to open", file);
	return -1;
     }

   Byte_Compile_Line_Len = 0;
   if (-1 != bytecomp_write_data (".#", 2))
     {
	_SLcompile_ptr = byte_compile_token;
	(void) SLang_load_file (name);
	_SLcompile_ptr = _SLcompile;

	(void) bytecomp_write_data ("\n", 1);
     }

   if (EOF == fclose (Byte_Compile_Fp))
     SLang_doerror ("Write Error");

   if (SLang_Error)
     {
	SLang_verror (0, "Error processing %s", name);
	return -1;
     }
   return 0;
}

int SLang_generate_debug_info (int x)
{
   int y = Default_Compile_Line_Num_Info;
   Default_Compile_Line_Num_Info = x;
   return y;
}
