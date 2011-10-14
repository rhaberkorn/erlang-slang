/* Copyright (c) 1998, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

static SLang_Load_Type *LLT;
int _SLang_Compile_Line_Num_Info;

static void free_token (_SLang_Token_Type *t)
{
   register unsigned int nrefs = t->num_refs;

   if (nrefs == 0)
     return;

   if (nrefs == 1)
     {
	if (t->free_sval_flag)
	  {
	     if (t->type == BSTRING_TOKEN)
	       SLbstring_free (t->v.b_val);
	     else
	       _SLfree_hashed_string (t->v.s_val, strlen (t->v.s_val), t->hash);
	     t->v.s_val = NULL;
	  }
     }

   t->num_refs = nrefs - 1;
}

static void init_token (_SLang_Token_Type *t)
{
   memset ((char *) t, 0, sizeof (_SLang_Token_Type));
#if _SLANG_HAS_DEBUG_CODE
   t->line_number = -1;
#endif
}

/* Allow room for one push back of a token.  This is necessary for
 * multiple assignment.
 */
static unsigned int Use_Next_Token;
static _SLang_Token_Type Next_Token;
#if _SLANG_HAS_DEBUG_CODE
static int Last_Line_Number = -1;
#endif

static int unget_token (_SLang_Token_Type *ctok)
{
   if (SLang_Error)
     return -1;
   if (Use_Next_Token != 0)
     {
	_SLparse_error ("unget_token failed", ctok, 0);
	return -1;
     }

   Use_Next_Token++;
   Next_Token = *ctok;
   init_token (ctok);
   return 0;
}

static int get_token (_SLang_Token_Type *ctok)
{
   if (ctok->num_refs)
     free_token (ctok);

   if (Use_Next_Token)
     {
	Use_Next_Token--;
	*ctok = Next_Token;
	return ctok->type;
     }

   return _SLget_token (ctok);
}

static int compile_token (_SLang_Token_Type *t)
{
#if _SLANG_HAS_DEBUG_CODE
   if (_SLang_Compile_Line_Num_Info
       && (t->line_number != Last_Line_Number)
       && (t->line_number != -1))
     {
	_SLang_Token_Type tok;
	tok.type = LINE_NUM_TOKEN;
	tok.v.long_val = Last_Line_Number = t->line_number;
	(*_SLcompile_ptr) (&tok);
     }
#endif
   (*_SLcompile_ptr) (t);
   return 0;
}

typedef struct
{
#define USE_PARANOID_MAGIC	0
#if USE_PARANOID_MAGIC
   unsigned long magic;
#endif
   _SLang_Token_Type *stack;
   unsigned int len;
   unsigned int size;
}
Token_List_Type;

#define MAX_TOKEN_LISTS 16
static Token_List_Type Token_List_Stack [MAX_TOKEN_LISTS];
static unsigned int Token_List_Stack_Depth = 0;
static Token_List_Type *Token_List = NULL;

static void init_token_list (Token_List_Type *t)
{
   t->size = 0;
   t->len = 0;
   t->stack = NULL;
#if USE_PARANOID_MAGIC
   t->magic = 0xABCDEF12;
#endif
}

static void free_token_list (Token_List_Type *t)
{
   _SLang_Token_Type *s;

   if (t == NULL)
     return;
#if USE_PARANOID_MAGIC
   if (t->magic != 0xABCDEF12)
     {
	SLang_doerror ("Magic error.");
	return;
     }
#endif
   s = t->stack;
   if (s != NULL)
     {
	_SLang_Token_Type *smax = s + t->len;
	while (s != smax)
	  {
	     if (s->num_refs) free_token (s);
	     s++;
	  }

	SLfree ((char *) t->stack);
     }

   memset ((char *) t, 0, sizeof (Token_List_Type));
}

static Token_List_Type *push_token_list (void)
{
   if (Token_List_Stack_Depth == MAX_TOKEN_LISTS)
     {
	_SLparse_error ("Token list stack size exceeded", NULL, 0);
	return NULL;
     }

   Token_List = Token_List_Stack + Token_List_Stack_Depth;
   Token_List_Stack_Depth++;
   init_token_list (Token_List);
   return Token_List;
}

static int pop_token_list (int do_free)
{
   if (Token_List_Stack_Depth == 0)
     {
	if (SLang_Error == 0)
	  _SLparse_error ("Token list stack underflow", NULL, 0);
	return -1;
     }
   Token_List_Stack_Depth--;

   if (do_free) free_token_list (Token_List);

   if (Token_List_Stack_Depth != 0)
     Token_List = Token_List_Stack + (Token_List_Stack_Depth - 1);
   else
     Token_List = NULL;

   return 0;
}

static int check_token_list_space (Token_List_Type *t, unsigned int delta_size)
{
   _SLang_Token_Type *st;
   unsigned int len;
#if USE_PARANOID_MAGIC
   if (t->magic != 0xABCDEF12)
     {
	SLang_doerror ("Magic error.");
	return -1;
     }
#endif
   len = t->len + delta_size;
   if (len <= t->size) return 0;

   if (delta_size < 4)
     {
	delta_size = 4;
	len = t->len + delta_size;
     }

   st = (_SLang_Token_Type *) SLrealloc((char *) t->stack,
					len * sizeof(_SLang_Token_Type));
   if (st == NULL)
     {
	_SLparse_error ("Malloc error", NULL, 0);
	return -1;
     }

   memset ((char *) (st + t->len), 0, delta_size);

   t->stack = st;
   t->size = len;
   return 0;
}

static int append_token (_SLang_Token_Type *t)
{
   if (-1 == check_token_list_space (Token_List, 1))
     return -1;

   Token_List->stack [Token_List->len] = *t;
   Token_List->len += 1;
   t->num_refs = 0;		       /* stealing it */
   return 0;
}

static int append_token_of_type (unsigned char t)
{
   _SLang_Token_Type *tok;

   if (-1 == check_token_list_space (Token_List, 1))
     return -1;

   /* The memset when the list was created ensures that the other fields
    * are properly initialized.
    */
   tok = Token_List->stack + Token_List->len;
   init_token (tok);
   tok->type = t;
   Token_List->len += 1;
   return 0;
}

static _SLang_Token_Type *get_last_token (void)
{
   unsigned int len;

   if ((Token_List == NULL)
       || (0 == (len = Token_List->len)))
     return NULL;

   len--;
   return Token_List->stack + len;
}

/* This function does NOT free the list. */
static int compile_token_list_with_fun (int dir, Token_List_Type *list,
					int (*f)(_SLang_Token_Type *))
{
   _SLang_Token_Type *t0, *t1;

   if (list == NULL)
     return -1;

   if (f == NULL)
     f = compile_token;

   t0 = list->stack;
   t1 = t0 + list->len;

   if (dir < 0)
     {
	/* backwards */

	while ((SLang_Error == 0) && (t1 > t0))
	  {
	     t1--;
	     (*f) (t1);
	  }
	return 0;
     }

   /* forward */
   while ((SLang_Error == 0) && (t0 < t1))
     {
	(*f) (t0);
	t0++;
     }
   return 0;
}

static int compile_token_list (void)
{
   if (Token_List == NULL)
     return -1;

   compile_token_list_with_fun (1, Token_List, NULL);
   pop_token_list (1);
   return 0;
}

/* Take all elements in the list from pos2 to the end and exchange them
 * with the elements at pos1, e.g.,
 * ...ABCDEabc ==> ...abcABCDE
 * where pos1 denotes A and pos2 denotes a.
 */
static int token_list_element_exchange (unsigned int pos1, unsigned int pos2)
{
   _SLang_Token_Type *s, *s1, *s2;
   unsigned int len, nloops;

   if (Token_List == NULL)
     return -1;

   s = Token_List->stack;
   len = Token_List->len;

   if ((s == NULL) || (len == 0)
       || (pos2 >= len))
     return -1;

   /* This may not be the most efficient algorithm but the number to swap
    * is most-likely going to be small, e.g, 3
    * The algorithm is to rotate the list.  The particular rotation
    * direction was chosen to make insert_token fast.
    * It works like:
    * @ ABCabcde --> BCabcdeA --> CabcdeAB -->  abcdefAB
    * which is optimal for Abcdef sequence produced by function calls.
    *
    * Profiling indicates that nloops is almost always 1, whereas the inner
    * loop can loop many times (e.g., 9 times).
    */

   s2 = s + (len - 1);
   s1 = s + pos1;
   nloops = pos2 - pos1;

   while (nloops)
     {
	_SLang_Token_Type save;

	s = s1;
	save = *s;

	while (s < s2)
	  {
	     *s = *(s + 1);
	     s++;
	  }
	*s = save;

	nloops--;
     }
   return 0;
}

#if 0
static int insert_token (_SLang_Token_Type *t, unsigned int pos)
{
   if (-1 == append_token (t))
     return -1;

   return token_list_element_exchange (pos, Token_List->len - 1);
}
#endif
static void compile_token_of_type (unsigned char t)
{
   _SLang_Token_Type tok;

#if _SLANG_HAS_DEBUG_CODE
   tok.line_number = -1;
#endif
   tok.type = t;
   compile_token(&tok);
}

static void statement (_SLang_Token_Type *);
static void compound_statement (_SLang_Token_Type *);
static void expression_with_parenthesis (_SLang_Token_Type *);
static void handle_semicolon (_SLang_Token_Type *);
static void statement_list (_SLang_Token_Type *);
static void variable_list (_SLang_Token_Type *, unsigned char);
static void struct_declaration (_SLang_Token_Type *);
static void define_function_args (_SLang_Token_Type *);
static void typedef_definition (_SLang_Token_Type *);
static void function_args_expression (_SLang_Token_Type *, int);
static void expression (_SLang_Token_Type *);
static void expression_with_commas (_SLang_Token_Type *, int);
static void simple_expression (_SLang_Token_Type *);
static void unary_expression (_SLang_Token_Type *);
static void postfix_expression (_SLang_Token_Type *);
static int check_for_lvalue (unsigned char, _SLang_Token_Type *);
/* static void primary_expression (_SLang_Token_Type *); */
static void block (_SLang_Token_Type *);
static void inline_array_expression (_SLang_Token_Type *);
static void array_index_expression (_SLang_Token_Type *);
static void do_multiple_assignment (_SLang_Token_Type *);
static void try_multiple_assignment (_SLang_Token_Type *);
#if 0
static void not_implemented (char *what)
{
   char err [256];
   sprintf (err, "Expression not implemented: %s", what);
   _SLparse_error (err, NULL, 0);
}
#endif
static void rpn_parse_line (_SLang_Token_Type *tok)
{
   do
     {
	  /* multiple RPN tokens possible when the file looks like:
	   * . <end of line>
	   * . <end of line>
	   */
	if (tok->type != RPN_TOKEN)
	  compile_token (tok);
	free_token (tok);
     }
   while (EOF_TOKEN != _SLget_rpn_token (tok));
}

static int get_identifier_token (_SLang_Token_Type *tok)
{
   if (IDENT_TOKEN == get_token (tok))
     return IDENT_TOKEN;

   _SLparse_error ("Expecting identifier", tok, 0);
   return tok->type;
}

static void define_function (_SLang_Token_Type *ctok, unsigned char type)
{
   _SLang_Token_Type fname;
   
   switch (type)
     {
      case STATIC_TOKEN:
	type = DEFINE_STATIC_TOKEN;
	break;
	
      case PUBLIC_TOKEN:
	type = DEFINE_PUBLIC_TOKEN;
	break;
	
      case PRIVATE_TOKEN:
	type = DEFINE_PRIVATE_TOKEN;
     }

   init_token (&fname);
   if (IDENT_TOKEN != get_identifier_token (&fname))
     {
	free_token (&fname);
	return;
     }

   compile_token_of_type(OPAREN_TOKEN);
   get_token (ctok);
   define_function_args (ctok);
   compile_token_of_type(FARG_TOKEN);

   if (ctok->type == OBRACE_TOKEN)
     compound_statement(ctok);

   else if (ctok->type != SEMICOLON_TOKEN)
     {
	_SLparse_error("Expecting {", ctok, 0);
	free_token (&fname);
	return;
     }

   fname.type = type;
   compile_token (&fname);
   free_token (&fname);
}

/* statement:
 *	 compound-statement
 *	 if ( expression ) statement
 *	 if ( expression ) statement else statement
 *	 !if ( expression ) statement
 *	 loop ( expression ) statement
 *	 _for ( expression ) statement
 *       foreach ( expression ) statement
 *       foreach (expression ) using (expression-list) statement
 *	 while ( expression ) statement
 *	 do statement while (expression) ;
 *	 for ( expressionopt ; expressionopt ; expressionopt ) statement
 *	 ERROR_BLOCK statement
 *	 EXIT_BLOCK statement
 *	 USER_BLOCK0 statement
 *	 USER_BLOCK1 statement
 *	 USER_BLOCK2 statement
 *	 USER_BLOCK3 statement
 *	 USER_BLOCK4 statement
 *	 forever statement
 *	 break ;
 *	 continue ;
 *	 return expressionopt ;
 *	 variable variable-list ;
 *	 struct struct-decl ;
 *	 define identifier function-args ;
 *	 define identifier function-args compound-statement
 *	 switch ( expression ) statement
 *	 rpn-line
 *	 at-line
 *	 push ( expression )
 *	 ( expression ) = expression ;
 *	 expression ;
 *       expression :
 */

/* Note: This function does not return with a new token.  It is up to the
 * calling routine to handle that.
 */
static void statement (_SLang_Token_Type *ctok)
{
   unsigned char type;

   if (SLang_Error)
     return;

   LLT->parse_level += 1;

   switch (ctok->type)
     {
      case OBRACE_TOKEN:
	compound_statement (ctok);
	break;

      case IF_TOKEN:
      case IFNOT_TOKEN:
	type = ctok->type;
	get_token (ctok);
	expression_with_parenthesis (ctok);
	block (ctok);

	if (ELSE_TOKEN != get_token (ctok))
	  {
	     compile_token_of_type (type);
	     unget_token (ctok);
	     break;
	  }
	get_token (ctok);
	block (ctok);
	if (type == IF_TOKEN) type = ELSE_TOKEN; else type = NOTELSE_TOKEN;
	compile_token_of_type (type);
	break;

      /* case IFNOT_TOKEN: */
      case LOOP_TOKEN:
      case _FOR_TOKEN:
	type = ctok->type;
	get_token (ctok);
	expression_with_parenthesis (ctok);
	block (ctok);
	compile_token_of_type (type);
	break;

      case FOREACH_TOKEN:
	get_token (ctok);
	expression_with_parenthesis (ctok);

	if (NULL == push_token_list ())
	  break;

	append_token_of_type (ARG_TOKEN);
	if (ctok->type == USING_TOKEN)
	  {
	     if (OPAREN_TOKEN != get_token (ctok))
	       {
		  _SLparse_error ("Expected 'using ('", ctok, 0);
		  break;
	       }
	     get_token (ctok);
	     function_args_expression (ctok, 0);
	  }
	append_token_of_type (EARG_TOKEN);

	compile_token_list ();

	block (ctok);
	compile_token_of_type (FOREACH_TOKEN);
	break;

      case WHILE_TOKEN:
	get_token (ctok);
	compile_token_of_type (OBRACE_TOKEN);
	expression_with_parenthesis (ctok);
	compile_token_of_type (CBRACE_TOKEN);
	block (ctok);
	compile_token_of_type (WHILE_TOKEN);
	break;

      case DO_TOKEN:
	get_token (ctok);
	block (ctok);

	if (WHILE_TOKEN != get_token (ctok))
	  {
	     _SLparse_error("Expecting while", ctok, 0);
	     break;
	  }

	get_token (ctok);

	compile_token_of_type (OBRACE_TOKEN);
	expression_with_parenthesis (ctok);
	compile_token_of_type (CBRACE_TOKEN);
	compile_token_of_type (DOWHILE_TOKEN);
	handle_semicolon (ctok);
	break;

      case FOR_TOKEN:

	/* Look for (exp_opt ; exp_opt ; exp_opt ) */

	if (OPAREN_TOKEN != get_token (ctok))
	  {
	     _SLparse_error("Expecting (.", ctok, 0);
	     break;
	  }

	if (NULL == push_token_list ())
	  break;

	append_token_of_type (OBRACE_TOKEN);
	if (SEMICOLON_TOKEN != get_token (ctok))
	  {
	     expression (ctok);
	     if (ctok->type != SEMICOLON_TOKEN)
	       {
		  _SLparse_error("Expecting ;", ctok, 0);
		  break;
	       }
	  }
	append_token_of_type (CBRACE_TOKEN);

	append_token_of_type (OBRACE_TOKEN);
	if (SEMICOLON_TOKEN != get_token (ctok))
	  {
	     expression (ctok);
	     if (ctok->type != SEMICOLON_TOKEN)
	       {
		  _SLparse_error("Expecting ;", ctok, 0);
		  break;
	       }
	  }
	append_token_of_type (CBRACE_TOKEN);

	append_token_of_type (OBRACE_TOKEN);
	if (CPAREN_TOKEN != get_token (ctok))
	  {
	     expression (ctok);
	     if (ctok->type != CPAREN_TOKEN)
	       {
		  _SLparse_error("Expecting ).", ctok, 0);
		  break;
	       }
	  }
	append_token_of_type (CBRACE_TOKEN);

	compile_token_list ();

	get_token (ctok);
	block (ctok);
	compile_token_of_type (FOR_TOKEN);
	break;

      case ERRBLK_TOKEN:
      case EXITBLK_TOKEN:
      case USRBLK0_TOKEN:
      case USRBLK1_TOKEN:
      case USRBLK2_TOKEN:
      case USRBLK3_TOKEN:
      case USRBLK4_TOKEN:
      case FOREVER_TOKEN:
	type = ctok->type;
	get_token (ctok);
	block (ctok);
	compile_token_of_type (type);
	break;

      case BREAK_TOKEN:
      case CONT_TOKEN:
	compile_token_of_type (ctok->type);
	get_token (ctok);
	handle_semicolon (ctok);
	break;

      case RETURN_TOKEN:
	if (SEMICOLON_TOKEN != get_token (ctok))
	  {
	     if (NULL == push_token_list ())
	       break;

	     expression (ctok);

	     if (ctok->type != SEMICOLON_TOKEN)
	       {
		  _SLparse_error ("Expecting ;", ctok, 0);
		  break;
	       }
	     compile_token_list ();
	  }
	compile_token_of_type (RETURN_TOKEN);
	handle_semicolon (ctok);
	break;

      case STATIC_TOKEN:
      case PRIVATE_TOKEN:
      case PUBLIC_TOKEN:
	type = ctok->type;
	get_token (ctok);
	if (ctok->type == VARIABLE_TOKEN)
	  {
	     get_token (ctok);
	     variable_list (ctok, type);
	     handle_semicolon (ctok);
	     break;
	  }
	if (ctok->type == DEFINE_TOKEN)
	  {
	     define_function (ctok, type);
	     break;
	  }
	_SLparse_error ("Expecting 'variable' or 'define'", ctok, 0);
	break;

      case VARIABLE_TOKEN:
	get_token (ctok);
	variable_list (ctok, OBRACKET_TOKEN);
	handle_semicolon (ctok);
	break;

      case TYPEDEF_TOKEN:
	get_token (ctok);
	if (NULL == push_token_list ())
	  break;
	typedef_definition (ctok);
	compile_token_list ();

	handle_semicolon (ctok);
	break;

      case DEFINE_TOKEN:
	define_function (ctok, DEFINE_TOKEN);
	break;

      case SWITCH_TOKEN:
	get_token (ctok);
	expression_with_parenthesis (ctok);

	while ((SLang_Error == 0)
	       && (OBRACE_TOKEN == ctok->type))
	  {
	     compile_token_of_type (OBRACE_TOKEN);
	     compound_statement (ctok);
	     compile_token_of_type (CBRACE_TOKEN);
	     get_token (ctok);
	  }
	compile_token_of_type (SWITCH_TOKEN);
	unget_token (ctok);
	break;

      case EOF_TOKEN:
	break;
#if 0
      case PUSH_TOKEN:
	get_token (ctok);
	expression_list_with_parenthesis (ctok);
	handle_semicolon (ctok);
	break;
#endif

      case SEMICOLON_TOKEN:
	handle_semicolon (ctok);
	break;

      case RPN_TOKEN:
	if (POUND_TOKEN == get_token (ctok))
	  _SLcompile_byte_compiled ();
	else if (ctok->type != EOF_TOKEN)
	  rpn_parse_line (ctok);
	break;

      case OPAREN_TOKEN:	       /* multiple assignment */
	try_multiple_assignment (ctok);
	if (ctok->type == COLON_TOKEN)
	  compile_token_of_type (COLON_TOKEN);
	else handle_semicolon (ctok);
	break;

      default:

	if (NULL == push_token_list ())
	  break;

	expression (ctok);
	compile_token_list ();

	if (ctok->type == COLON_TOKEN)
	  compile_token_of_type (COLON_TOKEN);
	else handle_semicolon (ctok);
	break;
     }

   LLT->parse_level -= 1;
}

static void block (_SLang_Token_Type *ctok)
{
   compile_token_of_type (OBRACE_TOKEN);
   statement (ctok);
   compile_token_of_type (CBRACE_TOKEN);
}

/*
 * statement-list:
 *	 statement
 *	 statement-list statement
 */
static void statement_list (_SLang_Token_Type *ctok)
{
   while ((SLang_Error == 0)
	  && (ctok->type != CBRACE_TOKEN)
	  && (ctok->type != EOF_TOKEN))
     {
	statement(ctok);
	get_token (ctok);
     }
}

/* compound-statement:
 *	 { statement-list }
 */
static void compound_statement (_SLang_Token_Type *ctok)
{
   /* ctok->type is OBRACE_TOKEN here */
   get_token (ctok);
   statement_list(ctok);
   if (CBRACE_TOKEN != ctok->type)
     {
	_SLparse_error ("Expecting '}'", ctok, 0);
	return;
     }
}

/* This function is only called from statement. */
static void expression_with_parenthesis (_SLang_Token_Type *ctok)
{
   if (ctok->type != OPAREN_TOKEN)
     {
	_SLparse_error("Expecting (", ctok, 0);
	return;
     }

   if (NULL == push_token_list ())
     return;

   get_token (ctok);
   expression (ctok);

   if (ctok->type != CPAREN_TOKEN)
     _SLparse_error("Expecting )", ctok, 0);

   compile_token_list ();

   get_token (ctok);
}

static void handle_semicolon (_SLang_Token_Type *ctok)
{
   if ((ctok->type == SEMICOLON_TOKEN)
       || (ctok->type == EOF_TOKEN))
     return;

   _SLparse_error ("Expecting ;", ctok, 0);
}

void _SLparse_start (SLang_Load_Type *llt)
{
   _SLang_Token_Type ctok;
   SLang_Load_Type *save_llt;
   unsigned int save_use_next_token;
   _SLang_Token_Type save_next_token;
   Token_List_Type *save_list;
#if _SLANG_HAS_DEBUG_CODE
   int save_last_line_number = Last_Line_Number;

   Last_Line_Number = -1;
#endif
   save_use_next_token = Use_Next_Token;
   save_next_token = Next_Token;
   save_list = Token_List;
   save_llt = LLT;
   LLT = llt;

   init_token (&Next_Token);
   Use_Next_Token = 0;
   init_token (&ctok);
   get_token (&ctok);

   llt->parse_level = 0;
   statement_list (&ctok);

   if ((SLang_Error == 0)
       && (ctok.type != EOF_TOKEN))
     _SLparse_error ("Parse ended prematurely", &ctok, 0);
   

   if (SLang_Error)
     {
	if (SLang_Error < 0)	       /* severe error */
	  save_list = NULL;

	while (Token_List != save_list)
	  {
	     if (-1 == pop_token_list (1))
	       break;		       /* ??? when would this happen? */
	  }
     }

   free_token (&ctok);
   LLT = save_llt;
   if (Use_Next_Token)
     free_token (&Next_Token);
   Use_Next_Token = save_use_next_token;
   Next_Token = save_next_token;
#if _SLANG_HAS_DEBUG_CODE
   Last_Line_Number = save_last_line_number;
#endif
}

/* variable-list:
 * 	variable-decl
 * 	variable-decl variable-list
 *
 * variable-decl:
 * 	identifier
 * 	identifier = simple-expression
 */
static void variable_list (_SLang_Token_Type *name_token, unsigned char variable_type)
{
   int declaring;
   _SLang_Token_Type tok;

   if (name_token->type != IDENT_TOKEN)
     {
	_SLparse_error ("Expecting a variable name", name_token, 0);
	return;
     }

   declaring = 0;
   do
     {
	if (declaring == 0)
	  {
	     declaring = 1;
	     compile_token_of_type (variable_type);
	  }

	compile_token (name_token);

	init_token (&tok);
	if (ASSIGN_TOKEN == get_token (&tok))
	  {
	     compile_token_of_type (CBRACKET_TOKEN);
	     declaring = 0;

	     get_token (&tok);

	     push_token_list ();
	     simple_expression (&tok);
	     compile_token_list ();

	     name_token->type = _SCALAR_ASSIGN_TOKEN;
	     compile_token (name_token);
	  }

	free_token (name_token);
	*name_token = tok;
     }
   while ((name_token->type == COMMA_TOKEN)
	  && (IDENT_TOKEN == get_token (name_token)));

   if (declaring) compile_token_of_type (CBRACKET_TOKEN);
}

/* struct-declaration:
 * 	struct { struct-field-list };
 *
 * struct-field-list:
 * 	struct-field-name , struct-field-list
 * 	struct-field-name
 *
 * Generates code: "field-name-1" ... "field-name-N" N STRUCT_TOKEN
 */
static void struct_declaration (_SLang_Token_Type *ctok)
{
   int n;
   _SLang_Token_Type num_tok;

   if (ctok->type != OBRACE_TOKEN)
     {
	_SLparse_error ("Expecting {", ctok, 0);
	return;
     }

   n = 0;
   while (IDENT_TOKEN == get_token (ctok))
     {
	n++;
	ctok->type = STRING_TOKEN;
	append_token (ctok);
	if (COMMA_TOKEN != get_token (ctok))
	  break;
     }

   if (ctok->type != CBRACE_TOKEN)
     {
	_SLparse_error ("Expecting }", ctok, 0);
	return;
     }
   if (n == 0)
     {
	_SLparse_error ("struct requires at least 1 field", ctok, 0);
	return;
     }

   init_token (&num_tok);
   num_tok.type = INT_TOKEN;
   num_tok.v.long_val = n;
   append_token (&num_tok);
   append_token_of_type (STRUCT_TOKEN);

   get_token (ctok);
}

/* struct-declaration:
 * 	typedef struct { struct-field-list } Type_Name;
 *
 * struct-field-list:
 * 	struct-field-name , struct-field-list
 * 	struct-field-name
 *
 * Generates code: "field-name-1" ... "field-name-N" N STRUCT_TOKEN typedef
 */
static void typedef_definition (_SLang_Token_Type *t)
{

   if (t->type != STRUCT_TOKEN)
     {
	_SLparse_error ("Expecting `struct'", t, 0);
	return;
     }
   get_token (t);

   struct_declaration (t);
   if (t->type != IDENT_TOKEN)
     {
	_SLparse_error ("Expecting identifier", t, 0);
	return;
     }

   t->type = STRING_TOKEN;
   append_token (t);
   append_token_of_type (TYPEDEF_TOKEN);

   get_token (t);
}

/* function-args:
 * 	( args-dec-opt )
 *
 * args-decl-opt:
 * 	identifier
 * 	args-decl , identifier
 */
static void define_function_args (_SLang_Token_Type *ctok)
{
   if (CPAREN_TOKEN == get_token (ctok))
     {
	get_token (ctok);
	return;
     }

   compile_token_of_type(OBRACKET_TOKEN);

   while ((SLang_Error == 0)
	  && (ctok->type == IDENT_TOKEN))
     {
	compile_token (ctok);
	if (COMMA_TOKEN != get_token (ctok))
	  break;

	get_token (ctok);
     }

   if (CPAREN_TOKEN != ctok->type)
     {
	_SLparse_error("Expecting )", ctok, 0);
	return;
     }
   compile_token_of_type(CBRACKET_TOKEN);

   get_token (ctok);
}

void try_multiple_assignment (_SLang_Token_Type *ctok)
{
   /* This is called with ctok->type == OPAREN_TOKEN.  We have no idea
    * what follows this.  There are various possibilities such as:
    * @  () = x;
    * @  ( expression ) = x;
    * @  ( expression ) ;
    * @  ( expression ) OP expression;
    * @  ( expression ) [expression] = expression;
    * and only the first two constitute a multiple assignment.  The last
    * two forms create the difficulty.
    *
    * Here is the plan.  First parse (expression) and then check next token.
    * If it is an equal operator, then it will be parsed as a multiple
    * assignment.  In fact, that is the easy part.
    *
    * The hard part stems from the fact that by parsing (expression), we
    * have effectly truncated the parse if (expression) is part of a binary
    * or unary expression.  Somehow, the parsing must be resumed.  The trick
    * here is to use a dummy literal that generates no code: NO_OP_LITERAL
    * Using it, we just call 'expression' and proceed.
    */

   if (NULL == push_token_list ())
     return;

   get_token (ctok);

   if (ctok->type != CPAREN_TOKEN)
     {
	expression_with_commas (ctok, 1);
	if (ctok->type != CPAREN_TOKEN)
	  {
	     _SLparse_error ("Expecting )", ctok, 0);
	     return;
	  }
     }

   switch (get_token (ctok))
     {
      case ASSIGN_TOKEN:
      case PLUSEQS_TOKEN:
      case MINUSEQS_TOKEN:
      case TIMESEQS_TOKEN:
      case DIVEQS_TOKEN:
      case BOREQS_TOKEN:
      case BANDEQS_TOKEN:
	do_multiple_assignment (ctok);
	pop_token_list (1);
	break;

      default:
	unget_token (ctok);
	ctok->type = NO_OP_LITERAL;
	expression (ctok);
	compile_token_list ();
	break;
     }
}

/* Note:  expression never gets compiled directly.  Rather, it gets
 *        appended to the token list and then compiled by a calling
 *        routine.
 */

/* expression:
 *	 simple_expression
 *	 simple-expression , expression
 *       <none>
 */
static void expression_with_commas (_SLang_Token_Type *ctok, int save_comma)
{
   while (SLang_Error == 0)
     {
	if (ctok->type != COMMA_TOKEN)
	  {
	     if (ctok->type == CPAREN_TOKEN)
	       return;

	     simple_expression (ctok);

	     if (ctok->type != COMMA_TOKEN)
	       break;
	  }
	if (save_comma) append_token (ctok);
	get_token (ctok);
     }
}

static void expression (_SLang_Token_Type *ctok)
{
   expression_with_commas (ctok, 0);
}

/* priority levels of binary operations */
static unsigned char Binop_Level[] =
{
/* ADD_TOKEN */		2,
/* SUB_TOKEN */		2,
/* MUL_TOKEN */		1,
/* DIV_TOKEN */		1,
/* LT_TOKEN */		4,
/* LE_TOKEN */		4,
/* GT_TOKEN */		4,
/* GE_TOKEN */		4,
/* EQ_TOKEN */		5,
/* NE_TOKEN */		5,
/* AND_TOKEN */		9,
/* OR_TOKEN */		10,
/* MOD_TOKEN */		1,
/* BAND_TOKEN */	6,
/* SHL_TOKEN */		3,
/* SHR_TOKEN */		3,
/* BXOR_TOKEN */	7,
/* BOR_TOKEN */		8,
/* POUND_TOKEN */	1  /* Matrix Multiplication */
};

/* % Note: simple-expression groups operators OP1 at same level.  The
 * % actual implementation will not do this.
 * simple-expression:
 *	 unary-expression
 *	 binary-expression BINARY-OP unary-expression
 *       andelse xxelse-expression-list
 *       orelse xxelse-expression-list
 *
 * xxelse-expression-list:
 * 	{ expression }
 * 	xxelse-expression-list { expression }
 * binary-expression:
 *      unary-expression
 *      unary-expression BINARY-OP binary-expression
 */
static void simple_expression (_SLang_Token_Type *ctok)
{
   unsigned char type;
   unsigned char op_stack [64];
   unsigned char level_stack [64];
   unsigned char level;
   unsigned int op_num;

   switch (ctok->type)
     {
      case ANDELSE_TOKEN:
      case ORELSE_TOKEN:
	type = ctok->type;
	if (OBRACE_TOKEN != get_token (ctok))
	  {
	     _SLparse_error ("Expecting '{'", ctok, 0);
	     return;
	  }

	while (ctok->type == OBRACE_TOKEN)
	  {
	     append_token (ctok);
	     get_token (ctok);
	     expression (ctok);
	     if (CBRACE_TOKEN != ctok->type)
	       {
		  _SLparse_error("Expecting }", ctok, 0);
		  return;
	       }
	     append_token (ctok);
	     get_token (ctok);
	  }
	append_token_of_type (type);
	return;

	/* avoid unary-expression if possible */
      case STRING_TOKEN:
	append_token (ctok);
	get_token (ctok);
	break;

      default:
	unary_expression (ctok);
	break;
     }

   if (SEMICOLON_TOKEN == (type = ctok->type))
     return;

   op_num = 0;

   while ((SLang_Error == 0)
	  && (IS_BINARY_OP(type)))
     {
	level = Binop_Level[type - FIRST_BINARY_OP];

	while ((op_num > 0) && (level_stack [op_num - 1] <= level))
	  append_token_of_type (op_stack [--op_num]);

	if (op_num >= sizeof (op_stack) - 1)
	  {
	     _SLparse_error ("Binary op stack overflow", ctok, 0);
	     return;
	  }

	op_stack [op_num] = type;
	level_stack [op_num] = level;
	op_num++;

	get_token (ctok);
	unary_expression (ctok);
	type = ctok->type;
     }

   while (op_num > 0)
     append_token_of_type(op_stack[--op_num]);
}

/* unary-expression:
 *	 postfix-expression
 *	 ++ postfix-expression
 *	 -- postfix-expression
 *	 case unary-expression
 *	 OP3 unary-expression
 *	 (OP3: + - ~ & not @)
 *
 * Note:  This grammar permits: case case case WHATEVER
 */
static void unary_expression (_SLang_Token_Type *ctok)
{
   unsigned char save_unary_ops [16];
   unsigned int num_unary_ops;
   unsigned char type;
   _SLang_Token_Type *last_token;

   num_unary_ops = 0;
   while (SLang_Error == 0)
     {
	type = ctok->type;

	switch (type)
	  {
	   case PLUSPLUS_TOKEN:
	   case MINUSMINUS_TOKEN:
	     get_token (ctok);
	     postfix_expression (ctok);
	     check_for_lvalue (type, NULL);
	     goto out_of_switch;

	   case ADD_TOKEN:
	     get_token (ctok);	       /* skip it-- it's unary here */
	     break;

	   case SUB_TOKEN:
	     (void) get_token (ctok);
	     if (IS_INTEGER_TOKEN (ctok->type))
	       {
		  ctok->v.long_val = -ctok->v.long_val;
		  break;
	       }

	     if (num_unary_ops == 16)
	       goto stack_overflow_error;
	     save_unary_ops [num_unary_ops++] = CHS_TOKEN;
	     break;

	   case DEREF_TOKEN:
	   case BNOT_TOKEN:
	   case NOT_TOKEN:
	   case CASE_TOKEN:
	     if (num_unary_ops == 16)
	       goto stack_overflow_error;

	     save_unary_ops [num_unary_ops++] = type;
	     get_token (ctok);
	     break;

	     /* Try to avoid ->postfix_expression->primary_expression
	      * subroutine calls.
	      */
	   case STRING_TOKEN:
	     append_token (ctok);
	     get_token (ctok);
	     goto out_of_switch;

	   default:
	     postfix_expression (ctok);
	     goto out_of_switch;
	  }
     }

   out_of_switch:
   if (num_unary_ops == 0)
     return;

   if ((DEREF_TOKEN == save_unary_ops[num_unary_ops - 1])
       && (NULL != (last_token = get_last_token ()))
       && (IS_ASSIGN_TOKEN(last_token->type)))
     {
	/* FIXME: Priority=medium
	 * This needs generalized so that things like @a.y = 1 will work properly.
	 */
	if ((num_unary_ops != 1)
	    || (last_token->type != _SCALAR_ASSIGN_TOKEN))
	  {
	     SLang_verror (SL_NOT_IMPLEMENTED, 
			   "Only derefence assignments to simple variables are possible");
	     return;
	  }

	last_token->type += (_DEREF_ASSIGN_TOKEN - _SCALAR_ASSIGN_TOKEN);
	return;
     }

   while (num_unary_ops)
     {
	num_unary_ops--;
	append_token_of_type (save_unary_ops [num_unary_ops]);
     }
   return;

   stack_overflow_error:
   _SLparse_error ("Too many unary operators.", ctok, 0);
}

static int combine_namespace_tokens (_SLang_Token_Type *a, _SLang_Token_Type *b)
{
   char *sa, *sb, *sc;
   unsigned int lena, lenb;
   unsigned long hash;

   /* This is somewhat of a hack.  Combine the TWO identifier names
    * (NAMESPACE) and (name) into the form NAMESPACE->name.  Then when the
    * byte compiler compiles the object it will not be found.  It will then
    * check for this hack and make the appropriate namespace lookup.
    */

   sa = a->v.s_val;
   sb = b->v.s_val;

   lena = strlen (sa);
   lenb = strlen (sb);

   sc = SLmalloc (lena + lenb + 3);
   if (sc == NULL)
     return -1;

   strcpy (sc, sa);
   strcpy (sc + lena, "->");
   strcpy (sc + lena + 2, sb);

   sb = _SLstring_make_hashed_string (sc, lena + lenb + 2, &hash);
   SLfree (sc);
   if (sb == NULL)
     return -1;

   /* I can free this string because no other token should be referencing it.
    * (num_refs == 1).
    */
   _SLfree_hashed_string (sa, lena, a->hash);
   a->v.s_val = sb;
   a->hash = hash;

   return 0;
}

static void append_identifier_token (_SLang_Token_Type *ctok)
{
   _SLang_Token_Type *last_token;

   append_token (ctok);

   if (NAMESPACE_TOKEN != get_token (ctok))
     return;

   if (IDENT_TOKEN != get_token (ctok))
     {
	_SLparse_error ("Expecting name-space identifier", ctok, 0);
	return;
     }

   last_token = get_last_token ();
   if (-1 == combine_namespace_tokens (last_token, ctok))
     return;

   (void) get_token (ctok);
}

static int get_identifier_expr_token (_SLang_Token_Type *ctok)
{
   _SLang_Token_Type next_token;

   if (IDENT_TOKEN != get_identifier_token (ctok))
     return -1;

   init_token (&next_token);
   if (NAMESPACE_TOKEN != get_token (&next_token))
     {
	unget_token (&next_token);
	return IDENT_TOKEN;
     }

   if (IDENT_TOKEN != get_identifier_token (&next_token))
     {
	free_token (&next_token);
	return -1;
     }

   if (-1 == combine_namespace_tokens (ctok, &next_token))
     {
	free_token (&next_token);
	return -1;
     }
   free_token (&next_token);
   return IDENT_TOKEN;
}

/* postfix-expression:
 *	 primary-expression
 *	 postfix-expression [ expression ]
 *	 postfix-expression ( function-args-expression )
 *	 postfix-expression . identifier
 *       postfix-expression ^ unary-expression
 *	 postfix-expression ++
 *	 postfix-expression --
 *	 postfix-expression = simple-expression
 *	 postfix-expression += simple-expression
 *	 postfix-expression -= simple-expression
 *
 * primary-expression:
 *	literal
 *	identifier-expr
 *	( expression_opt )
 * 	[ inline-array-expression ]
 * 	&identifier-expr
 *      struct-definition
 *      __tmp(identifier-expr)
 *
 * identifier-expr:
 *      identifier
 *      identifier->identifier
 */
static void postfix_expression (_SLang_Token_Type *ctok)
{
   unsigned int start_pos, end_pos;
   unsigned char type;

   if (Token_List == NULL)
     return;

   start_pos = Token_List->len;

   switch (ctok->type)
     {
      case IDENT_TOKEN:
	append_identifier_token (ctok);
	break;

      case CHAR_TOKEN:
      case SHORT_TOKEN:
      case INT_TOKEN:
      case LONG_TOKEN:
      case UCHAR_TOKEN:
      case USHORT_TOKEN:
      case UINT_TOKEN:
      case ULONG_TOKEN:
      case STRING_TOKEN:
      case BSTRING_TOKEN:
#ifdef SLANG_HAS_FLOAT
      case DOUBLE_TOKEN:
      case FLOAT_TOKEN:
#endif
#ifdef SLANG_HAS_COMPLEX
      case COMPLEX_TOKEN:
#endif
	append_token (ctok);
	get_token (ctok);
	break;

      case OPAREN_TOKEN:
	if (CPAREN_TOKEN != get_token (ctok))
	  {
	     expression (ctok);
	     if (ctok->type != CPAREN_TOKEN)
	       _SLparse_error("Expecting )", ctok, 0);
	  }
	get_token (ctok);
	break;

      case BAND_TOKEN:
	if (IDENT_TOKEN != get_identifier_expr_token (ctok))
	  break;

	ctok->type = _REF_TOKEN;
	append_token (ctok);
	get_token (ctok);
	break;

      case OBRACKET_TOKEN:
	get_token (ctok);
	inline_array_expression (ctok);
	break;

      case NO_OP_LITERAL:
	/* This token was introduced by try_multiple_assignment.  There,
	 * a new token_list was pushed and (expression) was evaluated.
	 * NO_OP_LITERAL represents the result of expression.  However,
	 * we need to tweak the start_pos variable to point to the beginning
	 * of the token list to complete the equivalence.
	 */
	start_pos = 0;
	get_token (ctok);
	break;

      case STRUCT_TOKEN:
	get_token (ctok);
	struct_declaration (ctok);
	break;

      case TMP_TOKEN:
	get_token (ctok);
	if (ctok->type == OPAREN_TOKEN)
	  {
	     if (IDENT_TOKEN == get_identifier_expr_token (ctok))
	       {
		  ctok->type = TMP_TOKEN;
		  append_token (ctok);
		  get_token (ctok);
		  if (ctok->type == CPAREN_TOKEN)
		    {
		       get_token (ctok);
		       break;
		    }
	       }
	  }
	_SLparse_error ("Expecting form __tmp(NAME)", ctok, 0);
	break;

      default:
	if (IS_INTERNAL_FUNC(ctok->type))
	  {
	     append_token (ctok);
	     get_token (ctok);
	  }
	else
	  _SLparse_error("Expecting a PRIMARY", ctok, 0);
     }

   while (SLang_Error == 0)
     {
	end_pos = Token_List->len;
	type = ctok->type;
	switch (type)
	  {
	   case OBRACKET_TOKEN:	       /* X[args] ==> [args] X ARRAY */
	     get_token (ctok);
	     append_token_of_type (ARG_TOKEN);
	     if (ctok->type != CBRACKET_TOKEN) 
	       array_index_expression (ctok);

	     if (ctok->type != CBRACKET_TOKEN)
	       {
		  _SLparse_error ("Expecting ']'", ctok, 0);
		  return;
	       }
	     get_token (ctok);
	     /* append_token_of_type (EARG_TOKEN); -- ARRAY_TOKEN implicitely does this */
	     token_list_element_exchange (start_pos, end_pos);
	     append_token_of_type (ARRAY_TOKEN);
	     break;

	   case OPAREN_TOKEN:
	     /* f(args) ==> args f */
	     if (CPAREN_TOKEN != get_token (ctok))
	       {
		  function_args_expression (ctok, 1);
		  token_list_element_exchange (start_pos, end_pos);
	       }
	     else get_token (ctok);
	     break;

	   case DOT_TOKEN:
	     /* S.a ==> "a" S DOT
	      * This means that if S is X[b], then X[b].a ==> a b X ARRAY DOT
	      * and f(a).X[b].c ==> "c" b "X" a f . ARRAY .
	      * Also, f(a).X[b] = g(x); ==> x g b "X" a f .
	      */
	     if (IDENT_TOKEN != get_identifier_token (ctok))
	       return;

	     ctok->type = DOT_TOKEN;
	     append_token (ctok);
	     get_token (ctok);
	     break;

	   case PLUSPLUS_TOKEN:
	   case MINUSMINUS_TOKEN:
	     check_for_lvalue (type, NULL);
	     get_token (ctok);
	     break;

	   case ASSIGN_TOKEN:
	   case PLUSEQS_TOKEN:
	   case MINUSEQS_TOKEN:
	   case TIMESEQS_TOKEN:
	   case DIVEQS_TOKEN:
	   case BOREQS_TOKEN:
	   case BANDEQS_TOKEN:
	     check_for_lvalue (type, NULL);
	     get_token (ctok);
	     simple_expression (ctok);
	     token_list_element_exchange (start_pos, end_pos);
	     break;

	   case POW_TOKEN:
	     get_token (ctok);
	     unary_expression (ctok);
	     append_token_of_type (POW_TOKEN);
	     break;

	   default:
	     return;
	  }
     }
}

static void function_args_expression (_SLang_Token_Type *ctok, int handle_num_args)
{
   unsigned char last_type, this_type;

   if (handle_num_args) append_token_of_type (ARG_TOKEN);

   last_type = COMMA_TOKEN;

   while (SLang_Error == 0)
     {
	this_type = ctok->type;

	switch (this_type)
	  {
	   case COMMA_TOKEN:
	     if (last_type == COMMA_TOKEN)
	       append_token_of_type (_NULL_TOKEN);
	     get_token (ctok);
	     break;

	   case CPAREN_TOKEN:
	     if (last_type == COMMA_TOKEN)
	       append_token_of_type (_NULL_TOKEN);
	     if (handle_num_args) append_token_of_type (EARG_TOKEN);
	     get_token (ctok);
	     return;

	   default:
	     simple_expression (ctok);
	     if ((ctok->type != COMMA_TOKEN)
		 && (ctok->type != CPAREN_TOKEN))
	       {
		  _SLparse_error ("Expecting ')'", ctok, 0);
		  break;
	       }
	  }
	last_type = this_type;
     }
}

static int check_for_lvalue (unsigned char eqs_type, _SLang_Token_Type *ctok)
{
   unsigned char type;

   if ((ctok == NULL)
       && (NULL == (ctok = get_last_token ())))
     return -1;

   type = ctok->type;

   eqs_type -= ASSIGN_TOKEN;

   if (type == IDENT_TOKEN)
     eqs_type += _SCALAR_ASSIGN_TOKEN;
   else if (type == ARRAY_TOKEN)
     eqs_type += _ARRAY_ASSIGN_TOKEN;
   else if (type == DOT_TOKEN)
     eqs_type += _STRUCT_ASSIGN_TOKEN;
   else
     {
	_SLparse_error ("Expecting LVALUE", ctok, 0);
	return -1;
     }

   ctok->type = eqs_type;
   return 0;
}

static void array_index_expression (_SLang_Token_Type *ctok)
{
   unsigned int num_commas;

   num_commas = 0;
   while (1)
     {
	switch (ctok->type)
	  {
	   case COLON_TOKEN:
	     if (num_commas)
	       _SLparse_error ("Misplaced ':'", ctok, 0);
	     return;
	     
	   case TIMES_TOKEN:
	     append_token_of_type (_INLINE_WILDCARD_ARRAY_TOKEN);
	     get_token (ctok);
	     break;
	     
	   case COMMA_TOKEN:
	     _SLparse_error ("Misplaced ','", ctok, 0);
	     return;
	     
	   default:
	     simple_expression (ctok);
	  }
	
	if (ctok->type != COMMA_TOKEN)
	  return;
	num_commas++;
	get_token (ctok);
     }
}

/* inline-array-expression:
 *    array_index_expression
 *    simple_expression : simple_expression
 *    simple_expression : simple_expression : simple_expression
 */
static void inline_array_expression (_SLang_Token_Type *ctok)
{
   int num_colons = 0;

   append_token_of_type (ARG_TOKEN);

   if (ctok->type == COLON_TOKEN)	       /* [:...] */
     append_token_of_type (_NULL_TOKEN);
   else if (ctok->type != CBRACKET_TOKEN) 
     array_index_expression (ctok);

   if (ctok->type == COLON_TOKEN)
     {
	num_colons++;
	if ((COLON_TOKEN == get_token (ctok))
	    || (ctok->type == CBRACKET_TOKEN))
	  append_token_of_type (_NULL_TOKEN);
	else
	  simple_expression (ctok);

	if (ctok->type == COLON_TOKEN)
	  {
	     num_colons++;
	     get_token (ctok);
	     simple_expression (ctok);
	  }
     }

   if (ctok->type != CBRACKET_TOKEN)
     {
	_SLparse_error ("Expecting ']'", ctok, 0);
	return;
     }

   /* append_token_of_type (EARG_TOKEN); */
   if (num_colons)
     append_token_of_type (_INLINE_IMPLICIT_ARRAY_TOKEN);
   else
     append_token_of_type (_INLINE_ARRAY_TOKEN);
   get_token (ctok);
}

static void do_multiple_assignment (_SLang_Token_Type *ctok)
{
   _SLang_Token_Type *s;
   unsigned int i, k, len;
   unsigned char assign_type;

   assign_type = ctok->type;

   /* The LHS token list has already been pushed.  Here we do the RHS
    * so push to another token list, process it, then come back to
    * LHS for assignment.
    */
   if (NULL == push_token_list ())
     return;

   get_token (ctok);
   expression (ctok);
   compile_token_list ();

   if (SLang_Error)
     return;

   /* Finally compile the LHS of the assignment expression
    * that has been saved.
    */
   s = Token_List->stack;
   len = Token_List->len;

   if (len == 0)
     {
	compile_token_of_type (POP_TOKEN);
	return;
     }

   while (len > 0)
     {
	/* List is of form:
	 *    a , b, c d e, f , g , , , h ,
	 * The missing expressions will be replaced by a POP
	 * ,,a
	 */

	/* Start from back looking for a COMMA */
	k = len - 1;
	if (s[k].type == COMMA_TOKEN)
	  {
	     compile_token_of_type (POP_TOKEN);
	     len = k;
	     continue;
	  }

	if (-1 == check_for_lvalue (assign_type, s + k))
	  return;

	i = 0;
	while (1)
	  {
	     if (s[k].type == COMMA_TOKEN)
	       {
		  i = k + 1;
		  break;
	       }

	     if (k == 0)
	       break;

	     k--;
	  }

	while (i < len)
	  {
	     compile_token (s + i);
	     i++;
	  }

	len = k;
     }

   if (s[0].type == COMMA_TOKEN)
     compile_token_of_type (POP_TOKEN);
}

