/* Copyright (c) 1996, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

/*--------------------------------*-C-*---------------------------------*
 * File:	slprepr.c
 *
 * preprocessing routines
 */
/*{{{ notes: */
/*
 * various preprocessing tokens supported
 *
 * #ifdef  TOKEN1 TOKEN2 ...
 *	- True if any of TOKEN1 TOKEN2 ... are defined
 *
 * #ifndef TOKEN1 TOKEN2 ...
 *	- True if none of TOKEN1 TOKEN2 ... are defined
 *
 * #iftrue
 * #ifnfalse
 * #if true
 * #if !false
 *	- always True
 *
 * #iffalse
 * #ifntrue
 * #if false
 * #if !true
 *	- always False
 *
 * #if$ENV
 *	- True if the enviroment variable ENV is set
 *
 * #ifn$ENV
 * #if !$ENV
 *	- True if the enviroment variable ENV is not set
 *
 * #if$ENV TOKEN1 TOKEN2 ...
 *	- True if the contents of enviroment variable ENV match
 *	  any of TOKEN1 TOKEN2 ...
 *
 * #ifn$ENV TOKEN1 TOKEN2 ...
 * #if !$ENV TOKEN1 TOKEN2 ...
 *	- True if the contents of enviroment variable ENV do not match
 *	  any of TOKEN1 TOKEN2 ...
 *
 *	NB: For $ENV, the tokens may contain wildcard characters:
 *		'?' - match any single character
 *		'*' - match any number of characters
 *
 * #ifexists TOKEN
 * #ifnexists TOKEN
 * #if !exists TOKEN
 *	- check if a variable/function exists
 *
 * #ifeval EXPRESSION
 * #ifneval EXPRESSION
 * #if !eval TOKEN
 *	- evaluates the EXPRESSION as an SLang expression
 *
 * #if (EXPRESSION)
 * #if !(EXPRESSION)
 *	- as per '#ifeval' / '#ifneval',
 *	  evaluates the EXPRESSION as a SLang expression
 *	- using '#ifn (EXPRESSION)' is possible, but deprecated
 *
 * #elif...
 * #else
 * #endif
 *
 * #stop
 *	- stop reading the entire file here, provided that the line
 *	  would have been executed
 *	eg:
 *		#iffalse
 *		#  stop
 *		#endif
 *	would NEVER stop
 *
 * #<TAG>
 *	- start embedded text region
 * #</TAG>
 *	- end embedded text region
 *
 *	All text, include other preprocessing directives, that occurs between
 *	the '#<TAG>' and '#</TAG>' directives will be ignored.
 *	This is useful for embedding other code or documentation.
 *	eg:
 *		#<latex>
 *		\chapter{My Documentation Effort}
 *		#</latex>
 *	NB: * although the current implementation only looks for sequences
 *	      '#<' and '#</', it is advisable to use the full '<TAG>' form
 *	      for documentation purposes and to avoid future surprises.
 *          * do NOT attempt to nest these constructions
 *
 * GENERAL NOTES:
 *   Apart from the '#ifdef' and '#ifndef' constructions, we are quite
 *   generous with accepting whitespace and the alternative '!' form.
 *   eg.,
 *	#ifTEST
 *	#ifnTEST
 *	#if TEST
 *	#if !TEST
 *	#if ! TEST
 *
 * mj olesen
 *----------------------------------------------------------------------*/
/*}}}*/
/*{{{ includes: */
#include "slinclud.h"

#include "slang.h"
#include "_slang.h"
/*}}}*/

int (*SLprep_exists_hook) (char *, char);	/* in "slang.h" */
int (*_SLprep_eval_hook) (char *);		/* in "_slang.h" */

/*{{{ SLprep_open_prep (), SLprep_close_prep () */
int SLprep_open_prep (SLPreprocess_Type *pt)
{
   pt->this_level = 0;
   pt->exec_level = 0;
   pt->prev_exec_level = 0;
   pt->comment_char = '%';
   pt->preprocess_char = '#';
   pt->flags = 0;
   return 0;
}

void SLprep_close_prep (SLPreprocess_Type *pt)
{
   (void) pt;
}
/*}}}*/

/*{{{ SLwildcard () */
/*----------------------------------------------------------------------*
 * Does `string' match `pattern' ?
 *
 * '*' in pattern matches any sub-string (including the null string)
 * '?' matches any single char.
 *
 * Code taken from that donated by Paul Hudson <paulh@harlequin.co.uk>
 * to the fvwm project.
 * It is public domain, no strings attached. No guarantees either.
 *----------------------------------------------------------------------*/
static int SLwildcard (char *pattern, char *string)
{
   if (pattern == NULL || *pattern == '\0' || !strcmp (pattern, "*"))
     return 1;
   else if (string == NULL)
     return 0;

   while (*pattern && *string) switch (*pattern)
     {
      case '?':
	/* match any single character */
	pattern++;
	string++;
	break;

      case '*':
	/* see if rest of pattern matches any trailing */
	/* substring of the string. */
	if (*++pattern == '\0')
	  return 1;	/* trailing * must match rest */

	while (*string)
	  {
	     if (SLwildcard (pattern, string)) return 1;
	     string++;
	  }
	return 0;

	/* break; */

      default:
	if (*pattern == '\\')
	  {
	     if (*++pattern == '\0')
	       pattern--;	/* don't skip trailing backslash */
	  }
	if (*pattern++ != *string++) return 0;
	break;
     }

   return ((*string == '\0')
	   && ((*pattern == '\0') || !strcmp (pattern, "*")));
}
/*}}}*/

#if defined(__16_BIT_SYSTEM__)
# define MAX_DEFINES 10
#else
# define MAX_DEFINES 128
#endif

/* The extra one is for NULL termination */
char *_SLdefines [MAX_DEFINES + 1];

int SLdefine_for_ifdef (char *s)	/*{{{*/
{
   unsigned int i;

   for (i = 0; i < MAX_DEFINES; i++)
     {
	char *s1 = _SLdefines [i];

	if (s1 == s)
	  return 0;		       /* already defined (hashed string) */

	if (s1 != NULL)
	  continue;

	s = SLang_create_slstring (s);
	if (s == NULL)
	  return -1;

	_SLdefines[i] = s;
	return 0;
     }
   return -1;
}
/*}}}*/

/*{{{ static functions */
static int is_any_defined(char *buf, char comment)	/*{{{*/
{
   char *sys;
   unsigned int i;

   while (1)
     {
	register char ch;

	/* Skip whitespace */
	while (((ch = *buf) == ' ') || (ch == '\t'))
	  buf++;

	if ((ch == '\n') || (ch == 0) || (ch == comment))
	  return 0;

	i = 0;
	while (NULL != (sys = _SLdefines [i++]))
	  {
	     unsigned int n;

	     if (*sys != ch)
	       continue;

	     n = strlen (sys);
	     if (0 == strncmp (buf, sys, n))
	       {
		  char ch1 = *(buf + n);

		  if ((ch1 == '\n') || (ch1 == 0) ||
		      (ch1 == ' ') || (ch1 == '\t') || (ch1 == comment))
		    return 1;
	       }
	  }

	/* Skip past word */
	while (((ch = *buf) != ' ')
	       && (ch != '\n')
	       && (ch != 0)
	       && (ch != '\t')
	       && (ch != comment))
	  buf++;
     }
}
/*}}}*/

static unsigned char *tokenize (unsigned char *buf, char *token, unsigned int len)
{
   register char *token_end;

   token_end = token + (len - 1);      /* allow room for \0 */

   while ((token < token_end) && (*buf > ' '))
     *token++ = *buf++;

   if (*buf > ' ') return NULL;	/* token too long */

   *token = '\0';

   while ((*buf == ' ') || (*buf == '\t')) buf++;

   return buf;
}

static int is_env_defined (char *buf, char comment)	/*{{{*/
{
   char * env, token [32];

   if ((*buf <= ' ') || (*buf == comment)) return 0;	/* no token */

   if (NULL == (buf = (char *) tokenize ((unsigned char *) buf,
					 token, sizeof (token))))
     return 0;

   if (NULL == (env = getenv (token)))
     return 0;		/* ENV not defined */

   if ((*buf == '\0') || (*buf == '\n') || (*buf == comment))
     return 1;			/* no tokens, but getenv() worked */

   do
     {
	buf = (char *) tokenize ((unsigned char *) buf, token, sizeof (token));
	if (buf == NULL) return 0;

	if (SLwildcard (token, env))
	  return 1;
     }
   while (*buf && (*buf != '\n') && (*buf != comment));

   return 0;
}
/*}}}*/
/*}}}*/

int SLprep_line_ok (char *buf, SLPreprocess_Type *pt)	/*{{{*/
{
   int level, prev_exec_level, exec_level;
   unsigned int flags;

   if ((buf == NULL) || (pt == NULL)) return 1;

   /* the '#stop' marker was already reached */
   if (pt->flags & SLPREP_STOP_READING)
     return 0;

   /* local bookkeeping */
   level           = pt->this_level;
   exec_level      = pt->exec_level;
   prev_exec_level = pt->prev_exec_level;
   flags = pt->flags;

   if (*buf != pt->preprocess_char)
     {
	/* ignore out-of-context or embedded text */
	if ((level != exec_level) || (flags & SLPREP_EMBEDDED_TEXT))
	  return 0;

	if (*buf == '\n') return flags & SLPREP_BLANK_LINES_OK;
	if (*buf == pt->comment_char) return flags & SLPREP_COMMENT_LINES_OK;
	
	return 1;
     }
   
   buf++;
   
   /*
    * Always allow '#!' to pass. This could be a shell script
    * with something like '#! /usr/local/bin/slang'
    */
   if ((*buf == '!') && (pt->preprocess_char == '#'))
     return 0;
   
   /* Allow whitespace as in '#  ifdef'  */
   while ((*buf == ' ') || (*buf == '\t')) buf++;
   
   /*
    * quick and dirty coding for '#<TAG>' and '#</TAG>'
    * only bothers to differentiate between '#<' and '#</'
    * never attempt to nest these constructions!!
    */
   if (*buf == '<')
     {
	buf++;
	if (*buf == '/') 	/* likely a '#</TAG>' */
	  pt->flags &= ~SLPREP_EMBEDDED_TEXT;	
	else			/* likely a '#<TAG>' */
	  pt->flags |= SLPREP_EMBEDDED_TEXT;
	
	return 0;	    
     }
   
   if (pt->flags & SLPREP_EMBEDDED_TEXT)
     return 0;		/* embedded text - ignore everything */
   
   
   if ((*buf < 'a') || (*buf > 'z'))	/* something weird */
     return (level == exec_level);
   
   if ( !strncmp(buf, "stop", 4) )
     {
	if (level == exec_level)	/* signal stop if we're in scope */
	  pt->flags |= SLPREP_STOP_READING;
	return 0;	/* swallow this tag */
     }
   
   if (!strncmp(buf, "endif", 5)) 
     {
	if (level == exec_level)
	  {
	     exec_level--;
	     prev_exec_level = exec_level;
	  }
	level--;
	if (level < prev_exec_level) prev_exec_level = level;
	goto done;
     }
   
   if ((buf[0] == 'e') && (buf[1] == 'l'))  /* else, elifdef, ... */
     {
	if ((level == exec_level + 1)
	    && (prev_exec_level != level)) 
	  {
	     /* We are in position to execute */
	     buf += 2;
	     if ((buf[0] == 's') && (buf[1] == 'e'))
	       {
		  /* 'else' */
		  exec_level = level;
		  goto done;
	       }
	     
	     /*
	      * drop through to 'if' testing.
	      * First set variable to value appropriate for 'if' testing.
	      */
	     level--;		       /* now == to exec level */
	  } 
	else
	  {
	     if (level == exec_level) 
	       {
		  exec_level--;
	       }
	     goto done;
	  }
     }
   
   if ((buf[0] == 'i') && (buf[1] == 'f')) 
     {
	int test  = 0;			/* fallback value */
	int truth = 1;
	buf += 2;
	
	if (level != exec_level) 
	  {
	     level++;
	     goto done;			/* Not interested */
	  }
	level++;
	
	if (buf[0] == 'n') 
	  {
	     truth = !truth;
	     buf++;
	  }
	
	/* for 'ifdef' and 'ifndef' we are done */
	if (!strncmp (buf, "def", 3))
	  {
	     test = is_any_defined(buf + 3, pt->comment_char);
	  } 
	else 
	  {
	     /*
	      * the '#ifn' construction cannot have whitespace or '!'
	      * for the other forms, we can also accept
	      * 'if!..' instead of 'ifn...', or even 'if ...' or 'if ! ...'
	      */
	     if (truth)
	       {
		  /* Allow some whitespace */
		  while ((*buf == ' ') || (*buf == '\t')) buf++;
		  
		  if (*buf == '!') 
		    {
		       /* the 'if !' form */	
		       truth = !truth;
		       buf++;
		       while ((*buf == ' ') || (*buf == '\t')) buf++;
		    }
	       }
	     
	     if (*buf == '$') 
	       test = is_env_defined (buf + 1, pt->comment_char);
	     else if (*buf == '('
		      && (_SLprep_eval_hook != NULL)) 
	       test = (*_SLprep_eval_hook) (buf);
	     else if (!strncmp (buf, "eval", 4)
		      && (_SLprep_eval_hook != NULL))
	       test = (*_SLprep_eval_hook) (buf + 4);
	     else if (!strncmp (buf, "exists", 6)
		      && (SLprep_exists_hook != NULL))
	       test = (*SLprep_exists_hook) (buf + 6, pt->comment_char);
	     else if (!strncmp (buf, "true", 4))
	       test = 1;
	     else if (strncmp (buf, "false", 5))
	       return 1;	/* unknown - let it bomb */
	  }
	
	if (truth == test) prev_exec_level = exec_level = level;
     }
   else
     {
	return 1;  /* let it bomb */
     }

   done:
   
   if (exec_level < 0) return 1;	/* bad level - let it bomb */

   pt->this_level      = level;
   pt->exec_level      = exec_level;
   pt->prev_exec_level = prev_exec_level;
   return 0;
}
/*}}}*/

/*{{{ main() - for testing only */
#if 0
int main (int argc, char *argv[])
{
   int i;
   char buf[1024];
   SLPreprocess_Type pt;

   SLprep_open_prep (&pt);
   SLdefine_for_ifdef ("UNIX");

   /* super cheap getopts */
   for (i = 1; i < argc; i++) 
     {
	char *p = argv[i];
	if (*p++ != '-') break;
	if (*p == '-')   break;

	if (*p == 'D')
	  {
	     SLdefine_for_ifdef (p + 1);
	     continue;
	  }
	
	while (*p) {
	   switch (*p) {
	    case 'B': pt.flags |= (SLPREP_BLANK_LINES_OK);	break;
	    case 'C': pt.flags |= (SLPREP_COMMENT_LINES_OK);	break;
	    case 'c': if (p[1]) pt.comment_char = *++p;		break;
	    case 'p': if (p[1]) pt.preprocess_char = *++p;	break;
	    default:
	      fprintf (stderr, "unknown flag '%c'\n", *p);
	      break;
	   }
	   p++;
	}
     }
   
   while (NULL != fgets (buf, sizeof (buf) - 1, stdin))
     {
	if (SLprep_line_ok (buf, &pt))
	  fputs (buf, stdout);
     }
   
   SLprep_close_prep (&pt);
   return 0;
}
#endif
/*}}}*/
