#include <stdio.h>
#include <stdio.h>
#include "sl-feat.h"
#include "slang.h"
#include "_slang.h"

static void dump_token (_SLang_Token_Type *t)
{
   char buf [256], *b;
   
   b = buf;
   
   if (SLang_Error)
     return;
   
   switch (t->type)
     {
      case IDENT_TOKEN:
	b = t->v.s_val;
	break;
	
      case CHAR_TOKEN:
      case INT_TOKEN:
	sprintf (buf, "%ld", t->v.long_val);
	break;
	
      case DOUBLE_TOKEN:
        b = t->v.s_val;
	break;
	
      case STRING_TOKEN:
	sprintf (buf, "\"%s\"", t->v.s_val);
	break;
	
      case PLUSPLUS_TOKEN:
	sprintf (buf, "++%s", t->v.s_val);
	break;
	
      case POST_PLUSPLUS_TOKEN:
	sprintf (buf, "%s++", t->v.s_val);
	break;
	
      case MINUSMINUS_TOKEN:
	sprintf (buf, "--%s", t->v.s_val);
	break;
	
      case POST_MINUSMINUS_TOKEN:
	sprintf (buf, "%s--", t->v.s_val);
	break;
	
      case MINUSEQS_TOKEN:
	sprintf (buf, "-=%s", t->v.s_val);
	break;
	
      case PLUSEQS_TOKEN:
	sprintf (buf, "+=%s", t->v.s_val);
	break;
	
      case ASSIGN_TOKEN:
	sprintf (buf, "=%s", t->v.s_val);
	break;
	
      case EOF_TOKEN:
	b = "EOF_TOKEN";
	break;
	
      case NOP_TOKEN:
	b = "NOP_TOKEN";
	break;
	
      case FOREVER_TOKEN:
	b = "forever";
	break;
	
      case ARG_TOKEN:
	b = "__args";
	break;
	
      case EARG_TOKEN:
	b = "__eargs";
	break;
	
      case FARG_TOKEN:
	b = "__farg";
	break;

      case _INLINE_ARRAY_TOKEN:
	b = "__inline_array";
	break;
	
      case _INLINE_IMPLICIT_ARRAY_TOKEN:
	b = "__inline_implicit_array";
	break;
	
      case IFNOT_TOKEN:
	b = "!if";
	break;
	
      case ABS_TOKEN:
	b = "abs";
	break;
	
      case LT_TOKEN:
	b = "<";
	break;
	
      case LE_TOKEN:
	b = "<=";
	break;
	
      case GT_TOKEN:
	b = ">";
	break;
	
      case GE_TOKEN:
	b = ">=";
	break;
	
      case EQ_TOKEN:
	b = "==";
	break;
	
      case NE_TOKEN:
	b = "!=";
	break;
	
      case AND_TOKEN:
	b = "and";
	break;
	
      case IF_TOKEN:
	b = "if";
	break;
	
      case POP_TOKEN:
	b = "pop";
	break;
	
      case ANDELSE_TOKEN:
	b = "andelse";
	break;
	
      case BXOR_TOKEN:
	b = "xor";
	break;
	
      case BAND_TOKEN:
	b = "&";
	break;
	
      case BOR_TOKEN:
	b = "|";
	break;
	
      case BNOT_TOKEN:
	b = "~";
	break;
	
      case SHR_TOKEN:
	b = "shr";
	break;
	
      case CHS_TOKEN:
	b = "chs";
	break;
	
      case SHL_TOKEN:
	b = "shl";
	break;
	
      case SQR_TOKEN:
	b = "sqr";
	break;
	
      case CASE_TOKEN:
	b = "case";
	break;
	
      case SIGN_TOKEN:
	b = "sign";
	break;
	
      case BREAK_TOKEN:
	b = "break";
	break;
	
      case STATIC_TOKEN:
	b = "static";
	break;
	
      case STRUCT_TOKEN:
	b = "struct";
	break;
	
      case RETURN_TOKEN:
	b = "return";
	break;
	
      case SWITCH_TOKEN:
	b = "switch";
	break;
	
      case EXCH_TOKEN:
	b = "exch";
	break;
	
      case CONT_TOKEN:
	b = "continue";
	break;
	
      case EXITBLK_TOKEN:
	b = "EXIT_BLOCK";
	break;
	
      case ERRBLK_TOKEN:
	b = "ERROR_BLOCK";
	break;
	
      case USRBLK0_TOKEN:
	b = "USER_BLOCK0";
	break;
	
      case USRBLK1_TOKEN:
	b = "USER_BLOCK1";
	break;
	
      case USRBLK2_TOKEN:
	b = "USER_BLOCK2";
	break;
	
      case USRBLK3_TOKEN:
	b = "USER_BLOCK3";
	break;
	
      case USRBLK4_TOKEN:
	b = "USER_BLOCK4";
	break;
	
      case ELSE_TOKEN:
	b = "else";
	break;
	
      case MUL2_TOKEN:
	b = "mul2";
	break;
	
      case DEFINE_TOKEN:
	b = ")";
	break;

      case DEFINE_STATIC_TOKEN:
	b = ")static";
	break;
	
      case LOOP_TOKEN:
	b = "loop";
	break;
	
      case MOD_TOKEN:
	b = "mod";
	break;
	
      case DO_TOKEN:
	b = "do";
	break;
	
      case DOWHILE_TOKEN:
	b = "do_while";
	break;
	
      case WHILE_TOKEN:
	b = "while";
	break;
	
      case OR_TOKEN:
	b = "or";
	break;
	
      case VARIABLE_TOKEN:
	b = "variable";
	break;
	
      case _SCALAR_ASSIGN_TOKEN:
	sprintf (buf, "=%s", t->v.s_val);
	break;

      case _SCALAR_PLUSEQS_TOKEN:
	sprintf (buf, "+=%s", t->v.s_val);
	break;

      case _SCALAR_MINUSEQS_TOKEN:
	sprintf (buf, "-=%s", t->v.s_val);
	break;

      case _SCALAR_PLUSPLUS_TOKEN:
	sprintf (buf, "++%s", t->v.s_val);
	break;

      case _SCALAR_POST_PLUSPLUS_TOKEN:
	sprintf (buf, "%s++", t->v.s_val);
	break;

      case _SCALAR_MINUSMINUS_TOKEN:
	sprintf (buf, "--%s", t->v.s_val);
	break;

      case _SCALAR_POST_MINUSMINUS_TOKEN:
	sprintf (buf, "%s--", t->v.s_val);
	break;

      case _DEREF_ASSIGN_TOKEN:
	sprintf (buf, "=@%s", t->v.s_val);
	break;

      case _REF_TOKEN:
	sprintf (buf, "%s __ref", t->v.s_val);
	break;
	
      case ORELSE_TOKEN:
	b = "orelse";
	break;
	
      case _FOR_TOKEN:
	b = "_for";
	break;
	
      case FOR_TOKEN:
	b = "for";
	break;
	
      case NOT_TOKEN:
	b = "not";
	break;
	
      case OBRACKET_TOKEN:
	b = "[";
	break;
	
      case CBRACKET_TOKEN:
	b = "]";
	break;
	
      case OPAREN_TOKEN:
	b = "(";
	break;
	
      case CPAREN_TOKEN:
	b = ")";
	break;
	
      case OBRACE_TOKEN:
	b = "{";
	break;
	
      case CBRACE_TOKEN:
	b = "}";
	break;
	
      case DEREF_TOKEN:
	b = "@";
	break;
	
      case COMMA_TOKEN:
	b = ",";
	break;
	
      case SEMICOLON_TOKEN:
	b = ";";
	break;
	
      case COLON_TOKEN:
	b = ":";
	break;
	
      case ADD_TOKEN:
	b = "+";
	break;
	
      case SUB_TOKEN:
	b = "-";
	break;

      /* case MUL_TOKEN: */
      /* 	b = "*"; */
      /* 	break; */
	
      case DIV_TOKEN:
	b = "/";
	break;
	
      case ARRAY_TOKEN:
	b = "__aget";
	break;
	
      case _ARRAY_ASSIGN_TOKEN:
	b = "__aput";
	break;
	
      case DOT_TOKEN:
	sprintf (buf, "%s .", t->v.s_val);
	break;

      case METHOD_TOKEN:
	sprintf (buf, "%s __eargs __method_call", t->v.s_val);
	break;

      case _STRUCT_ASSIGN_TOKEN:
	b = "__struct_eqs"; break;
      case _STRUCT_PLUSEQS_TOKEN:
	b = "__struct_pluseqs"; break;
      case _STRUCT_MINUSEQS_TOKEN:
	b = "__struct_minuseqs"; break;
      case _STRUCT_PLUSPLUS_TOKEN:
	b = "__struct_plusplus"; break;
      case _STRUCT_POST_PLUSPLUS_TOKEN:
	b = "__struct_pplusplus"; break;
      case _STRUCT_MINUSMINUS_TOKEN:
	b = "__struct_minusminus"; break;
      case _STRUCT_POST_MINUSMINUS_TOKEN:
	b = "__struct_pminusminus"; break;

      case _NULL_TOKEN: b = "NULL"; break;
	
      case USING_TOKEN:
	b = "__using__"; break;
      case FOREACH_TOKEN:
	b = "__foreach__"; break;

      default:
	sprintf (buf, "____UNKNOWN___0x%X", t->type);
	break;
     }
   
   fprintf (stdout, "0x%2X: %s\n", t->type, b);
}
   

int main (int argc, char **argv)
{
   char *file;

   if (argc == 2)
     {
	/* fprintf (stderr, "Usage: %s <filename>\n", argv[0]); */
	file = argv[1];
	/* return 1; */
     }
   else file = NULL;
   
   if (-1 == SLang_init_slang ())
     return 1;
   
   _SLcompile_ptr = dump_token;
   SLang_load_file (file);
   
   return SLang_Error;
}
