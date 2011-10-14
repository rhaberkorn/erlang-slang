#include <stdio.h>
#include <slang.h>
#include <pcre.h>

SLANG_MODULE(pcre);

static int PCRE_Type_Id;

typedef struct
{
   pcre *p;
   pcre_extra *extra;
   int *ovector;
   unsigned int ovector_len;	       /* must be a multiple of 3 */
   unsigned int num_matches;	       /* return value of pcre_exec (>= 1)*/
}
PCRE_Type;

static void free_pcre_type (PCRE_Type *pt)
{
   if (pt->ovector != NULL)
     SLfree ((char *) pt->ovector);

   SLfree ((char *) pt);
}

static SLang_MMT_Type *allocate_pcre_type (pcre *p, pcre_extra *extra)
{
   PCRE_Type *pt;
   SLang_MMT_Type *mmt;
   int ovector_len;

   pt = (PCRE_Type *) SLmalloc (sizeof (PCRE_Type));
   if (pt == NULL)
     return NULL;
   memset ((char *) pt, 0, sizeof (PCRE_Type));

   pt->p = p;
   pt->extra = extra;

   if (0 != pcre_fullinfo (p, extra, PCRE_INFO_CAPTURECOUNT, &ovector_len))
     {
	free_pcre_type (pt);
	SLang_verror (SL_INTRINSIC_ERROR, "pcre_fullinfo failed");
	return NULL;
     }

   ovector_len += 1;		       /* allow for pattern matched */
   ovector_len *= 3;		       /* required to be multiple of 3 */
   if (NULL == (pt->ovector = (int *)SLmalloc (ovector_len * sizeof (int))))
     {
	free_pcre_type (pt);
	return NULL;
     }
   pt->ovector_len = ovector_len;

   if (NULL == (mmt = SLang_create_mmt (PCRE_Type_Id, (VOID_STAR) pt)))
     {
	free_pcre_type (pt);
	return NULL;
     }   
   return mmt;
}

static int _pcre_compile_1 (char *pattern, int options)
{
   pcre *p;
   pcre_extra *extra;
   SLCONST char *err;
   int erroffset;
   unsigned char *table;
   SLang_MMT_Type *mmt;

   table = NULL;
   p = pcre_compile (pattern, options, &err, &erroffset, table);
   if (NULL == p)
     {
	SLang_verror (SL_INTRINSIC_ERROR, "Error compiling pattern '%s' at offset %d: %s", 
		      pattern, erroffset, err);
	return -1;
     }

   extra = pcre_study (p, 0, &err);
   /* apparantly, a NULL return is ok */
   if (err != NULL)
     {
	SLang_verror (SL_INTRINSIC_ERROR, "pcre_study failed: %s", err);
	pcre_free (p);
	return -1;
     }
   
   if (NULL == (mmt = allocate_pcre_type (p, extra)))
     {
	pcre_free ((char *) p);
	pcre_free ((char *) extra);
	return -1;
     }

   if (-1 == SLang_push_mmt (mmt))
     {
	SLang_free_mmt (mmt);
	return -1;
     }
   return 0;
}

static void _pcre_compile (void)
{
   char *pattern;
   int options = 0;
   
   switch (SLang_Num_Function_Args)
     {
      case 2:
	if (-1 == SLang_pop_integer (&options))
	  return;
	/* drop */
      case 1:
      default:
	if (-1 == SLang_pop_slstring (&pattern))
	  return;
     }
   (void) _pcre_compile_1 (pattern, options);
   SLang_free_slstring (pattern);
}



/* returns number of matches */
static int _pcre_exec_1 (PCRE_Type *pt, char *str, int pos, int options)
{
   int rc;
   unsigned int len;
   
   pt->num_matches = 0;
   len = strlen (str);
   if ((unsigned int) pos > len)
     return 0;

   rc = pcre_exec (pt->p, pt->extra, str, len, pos,
		   options, pt->ovector, pt->ovector_len);

   if (rc == PCRE_ERROR_NOMATCH)
     return 0;

   if (rc <= 0)
     {
	SLang_verror (SL_INTRINSIC_ERROR, "pcre_exec returned %d", rc);
	return -1;
     }
   pt->num_matches = (unsigned int) rc;
   return rc;
}

static int _pcre_exec (void)
{
   PCRE_Type *p;
   SLang_MMT_Type *mmt;
   char *str;
   int pos = 0;
   int options = 0;
   int ret = -1;

   switch (SLang_Num_Function_Args)
     {
      case 4:
	if (-1 == SLang_pop_integer (&options))
	  return -1;
      case 3:
	if (-1 == SLang_pop_integer (&pos))
	  return -1;
      default:
	if (-1 == SLang_pop_slstring (&str))
	  return -1;

	if (NULL == (mmt = SLang_pop_mmt (PCRE_Type_Id)))
	  goto free_and_return;
	p = SLang_object_from_mmt (mmt);
     }
   ret = _pcre_exec_1 (p, str, pos, options);
   
   free_and_return:
   SLang_free_slstring (str);
   SLang_free_mmt (mmt);
   return ret;
}


static int get_nth_start_stop (PCRE_Type *pt, unsigned int n, 
			       unsigned int *a, unsigned int *b)
{
   int start, stop;

   if (n >= pt->num_matches) 
     return -1;
   
   start = pt->ovector[2*n];
   stop = pt->ovector[2*n+1];
   if ((start < 0) || (stop < start))
     return -1;
   
   *a = (unsigned int) start;
   *b = (unsigned int) stop;
   return 0;
}

static void _pcre_nth_match (PCRE_Type *pt, int *np)
{
   unsigned int start, stop;
   SLang_Array_Type *at;
   int two = 2;
   int *data;

   if (-1 == get_nth_start_stop (pt, (unsigned int) *np, &start, &stop))
     {
	SLang_push_null ();
	return;
     }
   
   if (NULL == (at = SLang_create_array (SLANG_INT_TYPE, 0, NULL, &two, 1)))
     return;
   
   data = (int *)at->data;
   data[0] = (int)start;
   data[1] = (int)stop;
   (void) SLang_push_array (at, 1);
}

static void _pcre_nth_substr (PCRE_Type *pt, char *str, int *np)
{
   unsigned int start, stop;
   unsigned int len;
   
   len = strlen (str);

   if ((-1 == get_nth_start_stop (pt, (unsigned int) *np, &start, &stop))
       || (start > len) || (stop > len))
     {
	SLang_push_null ();
	return;
     }
   
   str = SLang_create_nslstring (str + start, stop - start);
   (void) SLang_push_string (str);
   SLang_free_slstring (str);
}

/* This function converts a slang RE to a pcre expression.  It performs the
 * following transformations:
 *    (     -->   \(
 *    )     -->   \)
 *    #     -->   \#
 *    |     -->   \|
 *    {     -->   \{
 *    }     -->   \}
 *   \<     -->   \b
 *   \>     -->   \b
 *   \C     -->   (?i)
 *   \c     -->   (?-i)
 *   \(     -->   (
 *   \)     -->   )
 *   \{     -->   {
 *   \}     -->   }
 * Anything else?
 */
static char *_slang_to_pcre (char *slpattern)
{
   char *pattern, *p, *s;
   unsigned int len;
   int in_bracket;
   char ch;

   len = strlen (slpattern);
   pattern = SLmalloc (3*len + 1);
   if (pattern == NULL)
     return NULL;
   
   p = pattern;
   s = slpattern;
   in_bracket = 0;
   while ((ch = *s++) != 0)
     {
	switch (ch)
	  {
	   case '{':
	   case '}':
	   case '(':
	   case ')':
	   case '#':
	   case '|':
	     if (0 == in_bracket) *p++ = '\\';
	     *p++ = ch;
	     break;

	   case '[':
	     in_bracket = 1;
	     *p++ = ch;
	     break;
	     
	   case ']':
	     in_bracket = 0;
	     *p++ = ch;
	     break;

	   case '\\':
	     ch = *s++;
	     switch (ch)
	       {
		case 0:
		  s--;
		  break;
		  
		case '<':
		case '>':
		  *p++ = '\\'; *p++ = 'b';
		  break;

		case '(':
		case ')':
		case '{':
		case '}':
		  *p++ = ch;
		  break;

		case 'C':
		  *p++ = '('; *p++ = '?'; *p++ = 'i'; *p++ = ')';
		  break;
		case 'c':
		  *p++ = '('; *p++ = '?'; *p++ = '-'; *p++ = 'i'; *p++ = ')';
		  break;
		  
		default:
		  *p++ = '\\';
		  *p++ = ch;
	       }
	     break;
	     
	   default:
	     *p++ = ch;
	     break;
	  }
     }
   *p = 0;
   
   s = SLang_create_slstring (pattern);
   SLfree (pattern);
   return s;
}

static void slang_to_pcre (char *pattern)
{
   /* NULL ok in code below */
   pattern = _slang_to_pcre (pattern);
   (void) SLang_push_string (pattern);
   SLang_free_slstring (pattern);
}

static void destroy_pcre (SLtype type, VOID_STAR f)
{
   PCRE_Type *pt;
   (void) type;
   
   pt = (PCRE_Type *) f;
   if (pt->extra != NULL)
     pcre_free ((char *) pt->extra);
   if (pt->p != NULL)
     pcre_free ((char *) pt->p);
   free_pcre_type (pt);
}

#define DUMMY_PCRE_TYPE 255
#define P DUMMY_PCRE_TYPE
#define I SLANG_INT_TYPE
#define V SLANG_VOID_TYPE
#define S SLANG_STRING_TYPE
static SLang_Intrin_Fun_Type PCRE_Intrinsics [] =
{
   MAKE_INTRINSIC_0("pcre_compile", _pcre_compile, V),
/*%+
 *\function{pcre_compile}
 *\synopsis{Compile a regular expression}
 *\usage{PCRE_Type pcre_compile (String_Type pattern [, Int_Type options])}
 *\description
 * The \var{pcre_compile} function compiles a PCRE style regular expression
 * and returns the result.  The optional \var{options} argument may be used
 * to provide addition information affecting the compilation of the pattern.
 * Specifically, it is a bit-mapped value formed from the logical-or of zero 
 * or more of the following symbolic constants:
 *#v+
 *    PCRE_ANCHORED     Force the match to be at the start of a string
 *    PCRE_CASELESS     Matches are to be case-insensitive
 *    PCRE_DOLLAR_ENDONLY (See PCRE docs for more information)
 *    PCRE_DOTALL       The dot pattern matches all characters
 *    PCRE_EXTENDED     Ignore whitespace in the pattern
 *    PCRE_EXTRA        (See PCRE docs for features this activates)
 *    PCRE_MULTILINE    Treat the subject string as multi-lines
 *    PCRE_UNGREEDY     Make the matches greedy
 *    PCRE_UTF8         Regard the pattern and subject strings as UTF-8
 *#v-
 * Many of these flags may be set within the pattern itself.   See the PCRE
 * library documentation for more information about the precise details 
 * of these flags and the supported regular expressions.
 * 
 * Upon success, this function returns a \var{PCRE_Type} object representing
 * the compiled patterned.  If compilation fails, an error will be thrown.
 *\seealso{pcre_exec, pcre_nth_match, pcre_nth_substr}
 *%-
 */
   MAKE_INTRINSIC_0("pcre_exec", _pcre_exec, I),
/*%+
 *\function{pcre_exec}
 *\synopsis{Match a string against a compiled PCRE pattern}
 *\usage{Int_Type pcre_exec(p, str [,pos [,options]]);
 *#v+
 *   PCRE_Type p;
 *   String_Type str;
 *   Int_Type pos, options;
 *#v-
 *\description
 * The \var{pcre_exec} function applies a pre-compiled pattern \var{p} to a 
 * string \var{str} and returns the result of the match.  The optional third 
 * argument \var{pos} may be used to specify the point, as an offset from the
 * start of the string, where matching is to start.  The fourth argument, if
 * present, may be used to provide additional information about how matching
 * is to take place.  Its value may be specified as a logical-or of zero or
 * more of the following flags:
 *#v+
 *   PCRE_NOTBOL      
 *        The first character in the string is not at the beginning of a line.
 *   PCRE_NOTEOL
 *        The last character in the string is not at the end of a line.
 *   PCRE_NOTEMPTY
 *        An empty string is not a valid match.
 *#v-
 * See the PCRE library documentation for more information about the meaning 
 * of these flags.
 * 
 * Upon success, this function returns a positive integer equal to 1 plus the
 * number of so-called captured substrings.  It returns 0 if the pattern
 * fails to match the string.
 *\seealso{pcre_compile, pcre_nth_match, pcre_nth_substr}
 *%-
 */
   MAKE_INTRINSIC_2("pcre_nth_match", _pcre_nth_match, V, P, I),
/*%+
 *\function{pcre_nth_match}
 *\synopsis{Return the location of the nth match of a PCRE}
 *\usage{Int_Type[2] pcre_nth_match (PCRE_Type p, Int_Type nth)}
 *\description
 * The \var{pcre_nth_match} function returns an integer array whose values
 * specify the locations of the beginning and end of the \var{nth} captured 
 * substrings of the most recent call to \var{pcre_exec} with the compiled
 * pattern.  A value of \var{nth} equal to 0 represents the substring 
 * representing the entire match of the pattern.
 * 
 * If the \var{nth} match did not take place, the function returns \var{NULL}.
 *\example
 * After the execution of:
 *#v+
 *    str = "Error in file foo.c, line 127, column 10";
 *    pattern = "file ([^,]+), line (\\d+)";
 *    p = pcre_compile (pattern);
 *    if (pcre_exec (p, str))
 *      { 
 *         match_pos = pcre_nth_match (p, 0);
 *         file_pos = pcre_nth_match (p, 1);
 *         line_pos = pcre_nth_match (p, 2);
 *      }
 *#v-
 * \exmp{match_pos} will be set to \exmp{[9,29]}, \exmp{file_pos} to \exmp{[14,19,]}
 * and \exmp{line_pos} to \exmp{[26,29]}.  These integer arrays may be used to 
 * extract the substrings matched by the pattern, e.g.,
 *#v+
 *     file = substr (str, file_pos[0]+1, file_pos[1]-file_pos[0]);
 *#v-
 * Alternatively, the function \var{pcre_nth_substr} may be used to get the 
 * matched substrings:
 *#v+
 *     file = pcre_nth_substr (p, str, 0);
 *#v-
 *\seealso{pcre_compile, pcre_exec, pcre_nth_substr}
 *%-
 */
   MAKE_INTRINSIC_3("pcre_nth_substr", _pcre_nth_substr, V, P, S, I),
/*%+
 *\function{pcre_nth_substr}
 *\synopsis{Extract the nth substring from a PCRE match}
 *\usage{String_Type pcre_nth_substr (PCRE_Type p, String_Type str, Int_Type nth)}
 *\description
 * This function may be used to extract the \var{nth} captured substring
 * resulting from the most recent use of the compiled pattern \var{p} by the
 * \var{pcre_exec} function.  Unlike \var{pcre_nth_match}, this function returns
 * the specified captured substring itself and not the position of the substring.
 * For this reason, the subject string of the pattern is a required argument.
 *\seealso{pcre_compile, pcre_exec, pcre_nth_match}
 *%-
 */
   MAKE_INTRINSIC_1("slang_to_pcre", slang_to_pcre, V, S),
/*%+
 *\function{slang_to_pcre}
 *\synopsis{Convert a S-Lang regular expression to a PCRE one}
 *\usage{String_Type slang_to_pcre (String_Type pattern)}
 *\description
 * This function may be used to convert a slang regular expression to a PCRE
 * compatible one.  The converted is returned.
 *\seealso{pcre_compile, string_match}
 *%-
 */
   SLANG_END_INTRIN_FUN_TABLE
};

static SLang_IConstant_Type PCRE_Consts [] =
{
   /* compile options */
   MAKE_ICONSTANT("PCRE_ANCHORED", PCRE_ANCHORED),
   MAKE_ICONSTANT("PCRE_CASELESS", PCRE_CASELESS),
   MAKE_ICONSTANT("PCRE_DOLLAR_ENDONLY", PCRE_DOLLAR_ENDONLY),
   MAKE_ICONSTANT("PCRE_DOTALL", PCRE_DOTALL),
   MAKE_ICONSTANT("PCRE_EXTENDED", PCRE_EXTENDED),
   MAKE_ICONSTANT("PCRE_EXTRA", PCRE_EXTRA),
   MAKE_ICONSTANT("PCRE_MULTILINE", PCRE_MULTILINE),
   MAKE_ICONSTANT("PCRE_UNGREEDY", PCRE_UNGREEDY),
   MAKE_ICONSTANT("PCRE_UTF8", PCRE_UTF8),
   
   /* exec options */
   MAKE_ICONSTANT("PCRE_NOTBOL", PCRE_NOTBOL),
   MAKE_ICONSTANT("PCRE_NOTEOL", PCRE_NOTEOL),
   MAKE_ICONSTANT("PCRE_NOTEMPTY", PCRE_NOTEMPTY),
   SLANG_END_ICONST_TABLE
};

#undef P
#undef I
#undef V
#undef S

static void patchup_intrinsic_table (SLang_Intrin_Fun_Type *table, 
				     unsigned char dummy, unsigned char type)
{
   while (table->name != NULL)
     {
	unsigned int i, nargs;
	unsigned char *args;
	
	nargs = table->num_args;
	args = table->arg_types;
	for (i = 0; i < nargs; i++)
	  {
	     if (args[i] == dummy)
	       args[i] = type;
	  }
	
	/* For completeness */
	if (table->return_type == dummy)
	  table->return_type = type;

	table++;
     }
}


static int register_pcre_type (void)
{
   SLang_Class_Type *cl;

   if (NULL == (cl = SLclass_allocate_class ("PCRE_Type")))
     return -1;

   if (-1 == SLclass_set_destroy_function (cl, destroy_pcre))
     return -1;

   /* By registering as SLANG_VOID_TYPE, slang will dynamically allocate a
    * type.
    */
   if (-1 == SLclass_register_class (cl, SLANG_VOID_TYPE, sizeof (PCRE_Type), SLANG_CLASS_TYPE_MMT))
     return -1;

   PCRE_Type_Id = SLclass_get_class_id (cl);
   patchup_intrinsic_table (PCRE_Intrinsics, DUMMY_PCRE_TYPE, PCRE_Type_Id);

   return 0;
}

static void *do_malloc (size_t n)
{
   return (void *) SLmalloc (n);
}

static void do_free (void *x)
{
   SLfree ((char *) x);
}

int init_pcre_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if (-1 == register_pcre_type ())
     return -1;

   pcre_malloc = do_malloc;
   pcre_free = do_free;

   if ((-1 == SLns_add_intrin_fun_table (ns, PCRE_Intrinsics, "__PCRE__"))
       || (-1 == SLns_add_iconstant_table (ns, PCRE_Consts, NULL)))
     return -1;

   return 0;
}


/* This function is optional */
void deinit_pcre_module (void)
{
}

