/* file stdio intrinsics for S-Lang */
/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#if defined(__unix__) || (defined (__os2__) && defined (__EMX__))
# include <sys/types.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_SYS_FCNTL_H
# include <sys/fcntl.h>
#endif

#ifdef __unix__
# include <sys/file.h>
#endif

#if defined(__BORLANDC__)
# include <io.h>
# include <dir.h>
#endif

#if defined(__DECC) && defined(VMS)
# include <unixio.h>
# include <unixlib.h>
#endif

#ifdef VMS
# include <stat.h>
#else
# include <sys/stat.h>
#endif

#include <errno.h>

#include <ctype.h>

/* #define SL_APP_WANTS_FOREACH */
#include "slang.h"
#include "_slang.h"

typedef struct
{
   FILE *fp;			       /* kind of obvious */
   char *file;			       /* file name associated with pointer */

   unsigned int flags;		       /* modes, etc... */
#define SL_READ		0x0001
#define SL_WRITE	0x0002
#define SL_BINARY	0x0004
#define SL_FDOPEN	0x2000
#define SL_PIPE		0x4000
#define SL_INUSE	0x8000
}
SL_File_Table_Type;

static SL_File_Table_Type *SL_File_Table;

static SL_File_Table_Type *get_free_file_table_entry (void)
{
   SL_File_Table_Type *t = SL_File_Table, *tmax;

   tmax = t + SL_MAX_FILES;
   while (t < tmax)
     {
	if (t->flags == 0)
	  {
	     memset ((char *) t, 0, sizeof (SL_File_Table_Type));
	     return t;
	  }
	t++;
     }

   return NULL;
}

static unsigned int file_process_flags (char *mode)
{
   char ch;
   unsigned int flags = 0;

   while (1)
     {
	ch = *mode++;
	switch (ch)
	  {
	   case 'r': flags |= SL_READ;
	     break;
	   case 'w':
	   case 'a':
	   case 'A':
	     flags |= SL_WRITE;
	     break;
	   case '+': flags |= SL_WRITE | SL_READ;
	     break;
	   case 'b': flags |= SL_BINARY;
	     break;
	   case 0:
	     return flags;

	   default:
	     SLang_verror (SL_INVALID_PARM, "File flag %c is not supported", ch);
	     return 0;
	  }
     }
}

static int open_file_type (char *file, int fd, char *mode,
			   FILE *(*open_fun)(char *, char *),
			   int (*close_fun)(FILE *),
			   unsigned int xflags)
{
   FILE *fp;
   SL_File_Table_Type *t;
   unsigned int flags;
   SLang_MMT_Type *mmt;

   fp = NULL;
   /* t = NULL; */
   mmt = NULL;

   if ((NULL == (t = get_free_file_table_entry ()))
       || (0 == (flags = file_process_flags(mode))))
     goto return_error;
   
   if (fd != -1)
     fp = fdopen (fd, mode);
   else
     fp = open_fun (file, mode);

   if (fp == NULL)
     {
	_SLerrno_errno = errno;
	goto return_error;
     }

   if (NULL == (mmt = SLang_create_mmt (SLANG_FILE_PTR_TYPE, (VOID_STAR) t)))
     goto return_error;

   t->fp = fp;
   t->flags = flags | xflags;
   fp = NULL;			       /* allow free_mmt to close fp */

   if ((NULL != (t->file = SLang_create_slstring (file)))
       && (0 == SLang_push_mmt (mmt)))
     return 0;

   /* drop */

   return_error:
   if (fp != NULL) (*close_fun) (fp);
   if (mmt != NULL) SLang_free_mmt (mmt);
   (void) SLang_push_null ();
   return -1;
}

/* Since some compilers do not have popen/pclose prototyped and in scope,
 * and pc compilers sometimes have silly prototypes involving PASCAL, etc.
 * use wrappers around the function to avoid compilation errors.
 */

static FILE *fopen_fun (char *f, char *m)
{
   return fopen (f, m);
}
static int fclose_fun (FILE *fp)
{
   return fclose (fp);
}

static void stdio_fopen (char *file, char *mode)
{
   (void) open_file_type (file, -1, mode, fopen_fun, fclose_fun, 0);
}

int _SLstdio_fdopen (char *file, int fd, char *mode)
{
   if (fd == -1)
     {
	_SLerrno_errno = EBADF;
	(void) SLang_push_null ();
	return -1;
     }

   return open_file_type (file, fd, mode, NULL, fclose_fun, SL_FDOPEN);
}

#ifdef HAVE_POPEN
static int pclose_fun (FILE *fp)
{
   return pclose (fp);
}

static FILE *popen_fun (char *file, char *mode)
{
   return popen (file, mode);
}

static void stdio_popen (char *file, char *mode)
{
   (void) open_file_type (file, -1, mode, popen_fun, pclose_fun, SL_PIPE);
}
#endif

/* returns pointer to file entry if it is open and consistent with
   flags.  Returns NULL otherwise */
static SLang_MMT_Type *pop_fp (unsigned int flags, FILE **fp_ptr)
{
   SL_File_Table_Type *t;
   SLang_MMT_Type *mmt;

   *fp_ptr = NULL;

   if (NULL == (mmt = SLang_pop_mmt (SLANG_FILE_PTR_TYPE)))
     return NULL;

   t = (SL_File_Table_Type *) SLang_object_from_mmt (mmt);
   if ((t->flags & flags)
       && (NULL != (*fp_ptr = t->fp)))
     return mmt;

   SLang_free_mmt (mmt);
   return NULL;
}

static FILE *check_fp (SL_File_Table_Type *t, unsigned flags)
{
   if ((t != NULL) && (t->flags & flags))
     return t->fp;

   return NULL;
}

char *SLang_get_name_from_fileptr (SLang_MMT_Type *mmt)
{
   SL_File_Table_Type *ft;

   ft = (SL_File_Table_Type *) SLang_object_from_mmt (mmt);
   if (ft == NULL)
     return NULL;
   return ft->file;
}

int SLang_pop_fileptr (SLang_MMT_Type **mmt, FILE **fp)
{
   if (NULL == (*mmt = pop_fp (0xFFFF, fp)))
     {
#ifdef EBADF
	_SLerrno_errno = EBADF;
#endif
	return -1;
     }

   return 0;
}

static int close_file_type (SL_File_Table_Type *t)
{
   int ret = 0;
   FILE *fp;

   if (t == NULL)
     return -1;

   fp = t->fp;

   if (NULL == fp) ret = -1;
   else
     {
	if (0 == (t->flags & SL_PIPE))
	  {
	     if (EOF == (ret = fclose (fp)))
	       _SLerrno_errno = errno;
	  }
#ifdef HAVE_POPEN
	else
	  {
	     if (-1 == (ret = pclose (fp)))
	       _SLerrno_errno = errno;
	  }
#endif
     }

   if (t->file != NULL) SLang_free_slstring (t->file);
   memset ((char *) t, 0, sizeof (SL_File_Table_Type));
   return ret;
}

static int stdio_fclose (SL_File_Table_Type *t)
{
   int ret;

   if (NULL == check_fp (t, 0xFFFF))
     return -1;

   ret = close_file_type (t);

   t->flags = SL_INUSE;
   return ret;
}

static int read_one_line (FILE *fp, char **strp, unsigned int *lenp, int trim_trailing)
{
   char buf[512];
   char *str;
   unsigned int len;

   *strp = NULL;
   len = 0;
   str = NULL;

   while (NULL != fgets (buf, sizeof (buf), fp))
     {
	unsigned int dlen;
	char *new_str;
	int done_flag;

	dlen = strlen (buf);
	/* Note: If the file contains embedded \0 characters, then this
	 * fails to work properly since dlen will not be correct.
	 */
	done_flag = ((dlen + 1 < sizeof (buf))
		     || (buf[dlen - 1] == '\n'));

	if (done_flag && (str == NULL))
	  {
	     /* Avoid the malloc */
	     str = buf;
	     len = dlen;
	     break;
	  }

	if (NULL == (new_str = SLrealloc (str, len + dlen + 1)))
	  {
	     SLfree (str);
	     return -1;
	  }

	str = new_str;
	strcpy (str + len, buf);
	len += dlen;

	if (done_flag) break;
     }

   if (str == NULL)
     return 0;

   if (trim_trailing)
     {
	unsigned int len1 = len;
	while (len1)
	  {
	     len1--;
	     if (0 == isspace(str[len1]))
	       {
		  len1++;
		  break;
	       }
	  }
	len = len1;
     }
	       
   *strp = SLang_create_nslstring (str, len);
   if (str != buf) SLfree (str);

   if (*strp == NULL) return -1;

   *lenp = len;
   return 1;
}

/* returns number of characters read and pushes the string to the stack.
   If it fails, it returns -1 */
static int stdio_fgets (SLang_Ref_Type *ref, SL_File_Table_Type *t)
{
   char *s;
   unsigned int len;
   FILE *fp;
   int status;

   if (NULL == (fp = check_fp (t, SL_READ)))
     return -1;

   status = read_one_line (fp, &s, &len, 0);
   if (status <= 0)
     return -1;

   status = SLang_assign_to_ref (ref, SLANG_STRING_TYPE, (VOID_STAR)&s);
   SLang_free_slstring (s);

   if (status == -1)
     return -1;

   return (int) len;
}

static void stdio_fgetslines_internal (FILE *fp, unsigned int n)
{
   unsigned int num_lines, max_num_lines;
   char **list;
   SLang_Array_Type *at;
   int inum_lines;

   if (n > 1024)
     max_num_lines = 1024;
   else 
     {
	max_num_lines = n;
	if (max_num_lines == 0)
	  max_num_lines++;
     }

   list = (char **) SLmalloc (sizeof (char *) * max_num_lines);
   if (list == NULL)
     return;

   num_lines = 0;
   while (num_lines < n)
     {
	int status;
	char *line;
	unsigned int len;

	status = read_one_line (fp, &line, &len, 0);
	if (status == -1)
	  goto return_error;

	if (status == 0)
	  break;

	if (max_num_lines == num_lines)
	  {
	     char **new_list;

	     if (max_num_lines + 4096 > n)
	       max_num_lines = n;
	     else
	       max_num_lines += 4096;

	     new_list = (char **) SLrealloc ((char *)list, sizeof (char *) * max_num_lines);
	     if (new_list == NULL)
	       {
		  SLang_free_slstring (line);
		  goto return_error;
	       }
	     list = new_list;
	  }

	list[num_lines] = line;
	num_lines++;
     }

   if (num_lines != max_num_lines)
     {
	char **new_list;

	new_list = (char **)SLrealloc ((char *)list, sizeof (char *) * (num_lines + 1));
	if (new_list == NULL)
	  goto return_error;

	list = new_list;
     }

   inum_lines = (int) num_lines;
   if (NULL == (at = SLang_create_array (SLANG_STRING_TYPE, 0, (VOID_STAR) list, &inum_lines, 1)))
     goto return_error;

   if (-1 == SLang_push_array (at, 1))
     SLang_push_null ();
   return;

   return_error:
   while (num_lines > 0)
     {
	num_lines--;
	SLfree (list[num_lines]);
     }
   SLfree ((char *)list);
   SLang_push_null ();
}

static void stdio_fgetslines (void)
{
   unsigned int n;
   FILE *fp;
   SLang_MMT_Type *mmt;

   n = (unsigned int)-1;

   if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_uinteger (&n))
	  return;
     }
   
   if (NULL == (mmt = pop_fp (SL_READ, &fp)))
     {
	SLang_push_null ();
	return;
     }

   stdio_fgetslines_internal (fp, n);
   SLang_free_mmt (mmt);
}


static int stdio_fputs (char *s, SL_File_Table_Type *t)
{
   FILE *fp;

   if (NULL == (fp = check_fp (t, SL_WRITE)))
     return -1;

   if (EOF == fputs(s, fp)) return -1;
   return (int) strlen (s);
}

static int stdio_fflush (SL_File_Table_Type *t)
{
   FILE *fp;

   if (NULL == (fp = check_fp (t, SL_WRITE)))
     return -1;

   if (EOF == fflush (fp))
     {
	_SLerrno_errno = errno;
	return -1;
     }

   return 0;
}

/* Usage: n = fread (&str, data-type, nelems, fp); */
static void stdio_fread (SLang_Ref_Type *ref, int *data_typep, unsigned int *num_elemns, SL_File_Table_Type *t)
{
   char *s;
   FILE *fp;
   int ret;
   unsigned int num_read, num_to_read;
   unsigned int nbytes;
   SLang_Class_Type *cl;
   unsigned int sizeof_type;
   int data_type;

   ret = -1;
   s = NULL;
   /* cl = NULL; */

   if (NULL == (fp = check_fp (t, SL_READ)))
     goto the_return;

   /* FIXME: priority = low : I should add some mechanism to support
    * other types.
    */
   data_type = *data_typep;

   cl = _SLclass_get_class ((unsigned char) data_type);

   if (cl->cl_fread == NULL)
     {
	SLang_verror (SL_NOT_IMPLEMENTED,
		      "fread does not support %s objects",
		      cl->cl_name);
	goto the_return;
     }

   sizeof_type = cl->cl_sizeof_type;

   num_to_read = *num_elemns;
   nbytes = (unsigned int) num_to_read * sizeof_type;

   s = SLmalloc (nbytes + 1);
   if (s == NULL)
     goto the_return;

   ret = cl->cl_fread (data_type, fp, (VOID_STAR)s, num_to_read, &num_read);

   if ((num_read == 0)
       && (num_read != num_to_read))
     ret = -1;

   if ((ret == -1) && ferror (fp))
     _SLerrno_errno = errno;

   if ((ret == 0)
       && (num_read != num_to_read))
     {
	char *new_s;

	nbytes = num_read * sizeof_type;
	new_s = SLrealloc (s, nbytes + 1);
	if (new_s == NULL)
	  ret = -1;
	else
	  s = new_s;
     }

   if (ret == 0)
     {
	if (num_read == 1)
	  {
	     ret = SLang_assign_to_ref (ref, data_type, (VOID_STAR)s);
	     SLfree (s);
	  }
	else if ((data_type == SLANG_CHAR_TYPE)
		 || (data_type == SLANG_UCHAR_TYPE))
	  {
	     SLang_BString_Type *bs;

	     bs = SLbstring_create_malloced ((unsigned char *)s, num_read, 1);
	     ret = SLang_assign_to_ref (ref, SLANG_BSTRING_TYPE, (VOID_STAR)&bs);
	     SLbstring_free (bs);
	  }
	else
	  {
	     SLang_Array_Type *at;
	     int inum_read = (int) num_read;
	     at = SLang_create_array (data_type, 0, (VOID_STAR)s, &inum_read, 1);
	     ret = SLang_assign_to_ref (ref, SLANG_ARRAY_TYPE, (VOID_STAR)&at);
	     SLang_free_array (at);
	  }
	s = NULL;
     }

   the_return:

   if (s != NULL)
     SLfree (s);
   
   if (ret == -1)
     SLang_push_integer (ret);
   else
     SLang_push_uinteger (num_read);
}

/* Usage: n = fwrite (str, fp); */
static void stdio_fwrite (SL_File_Table_Type *t)
{
   FILE *fp;
   unsigned char *s;
   unsigned int num_to_write, num_write;
   int ret;
   SLang_BString_Type *b;
   SLang_Array_Type *at;
   SLang_Class_Type *cl;

   ret = -1;
   b = NULL;
   at = NULL;

   switch (SLang_peek_at_stack ())
     {
      case SLANG_BSTRING_TYPE:
      case SLANG_STRING_TYPE:
	if (-1 == SLang_pop_bstring (&b))
	  goto the_return;

	if (NULL == (s = SLbstring_get_pointer (b, &num_to_write)))
	  goto the_return;

	cl = _SLclass_get_class (SLANG_UCHAR_TYPE);
	break;

      default:
	if (-1 == SLang_pop_array (&at, 1))
	  goto the_return;

	cl = at->cl;
	num_to_write = at->num_elements;
	s = (unsigned char *) at->data;
     }

   if (NULL == (fp = check_fp (t, SL_WRITE)))
     goto the_return;

   if (cl->cl_fwrite == NULL)
     {
	SLang_verror (SL_NOT_IMPLEMENTED,
		      "fwrite does not support %s objects", cl->cl_name);
	goto the_return;
     }

   ret = cl->cl_fwrite (cl->cl_data_type, fp, s, num_to_write, &num_write);

   if ((ret == -1) && ferror (fp))
     _SLerrno_errno = errno;

   /* drop */
   the_return:
   if (b != NULL)
     SLbstring_free (b);
   if (at != NULL)
     SLang_free_array (at);

   if (ret == -1)
     SLang_push_integer (ret);
   else
     SLang_push_uinteger (num_write);
}

static int stdio_fseek (SL_File_Table_Type *t, int *ofs, int *whence)
{
   FILE *fp;

   if (NULL == (fp = check_fp (t, 0xFFFF)))
     return -1;

   if (-1  == fseek (fp, (long) *ofs, *whence))
     {
	_SLerrno_errno = errno;
	return -1;
     }

   return 0;
}

static int stdio_ftell (SL_File_Table_Type *t)
{
   FILE *fp;
   long ofs;

   if (NULL == (fp = check_fp (t, 0xFFFF)))
     return -1;

   if (-1L == (ofs = ftell (fp)))
     {
	_SLerrno_errno = errno;
	return -1;
     }

   return (int) ofs;
}

static int stdio_feof (SL_File_Table_Type *t)
{
   FILE *fp;

   if (NULL == (fp = check_fp (t, 0xFFFF)))
     return -1;

   return feof (fp);
}

static int stdio_ferror (SL_File_Table_Type *t)
{
   FILE *fp;

   if (NULL == (fp = check_fp (t, 0xFFFF)))
     return -1;

   return ferror (fp);
}

static void stdio_clearerr (SL_File_Table_Type *t)
{
   FILE *fp;

   if (NULL != (fp = check_fp (t, 0xFFFF)))
     clearerr (fp);
}

/* () = fprintf (fp, "FORMAT", arg...); */
static int stdio_fprintf (void)
{
   char *s;
   FILE *fp;
   SLang_MMT_Type *mmt;
   int status;

   if (-1 == _SLstrops_do_sprintf_n (SLang_Num_Function_Args - 2))
     return -1;
   
   if (-1 == SLang_pop_slstring (&s))
     return -1;
   
   if (NULL == (mmt = pop_fp (SL_WRITE, &fp)))
     {
	SLang_free_slstring (s);
	return -1;
     }
   
   if (EOF == fputs(s, fp))
     status = -1;
   else
     status = (int) strlen (s);

   SLang_free_mmt (mmt);
   SLang_free_slstring (s);
   return status;
}

static int stdio_printf (void)
{
   char *s;
   int status;

   if (-1 == _SLstrops_do_sprintf_n (SLang_Num_Function_Args - 1))
     return -1;

   if (-1 == SLang_pop_slstring (&s))
     return -1;
   
   if (EOF == fputs(s, stdout))
     status = -1;
   else
     status = (int) strlen (s);

   SLang_free_slstring (s);
   return status;
}

   
#define F SLANG_FILE_PTR_TYPE
#define R SLANG_REF_TYPE
#define I SLANG_INT_TYPE
#define V SLANG_VOID_TYPE
#define S SLANG_STRING_TYPE
#define B SLANG_BSTRING_TYPE
#define U SLANG_UINT_TYPE
#define D SLANG_DATATYPE_TYPE
static SLang_Intrin_Fun_Type Stdio_Name_Table[] =
{
   MAKE_INTRINSIC_0("fgetslines", stdio_fgetslines, V),
   MAKE_INTRINSIC_SS("fopen", stdio_fopen, V),
   MAKE_INTRINSIC_1("feof", stdio_feof, I, F),
   MAKE_INTRINSIC_1("ferror", stdio_ferror, I, F),
   MAKE_INTRINSIC_1("fclose", stdio_fclose, I, F),
   MAKE_INTRINSIC_2("fgets", stdio_fgets, I, R, F),
   MAKE_INTRINSIC_1("fflush", stdio_fflush, I, F),
   MAKE_INTRINSIC_2("fputs", stdio_fputs, I, S, F),
   MAKE_INTRINSIC_0("fprintf", stdio_fprintf, I),
   MAKE_INTRINSIC_0("printf", stdio_printf, I),
   MAKE_INTRINSIC_3("fseek", stdio_fseek, I, F, I, I),
   MAKE_INTRINSIC_1("ftell", stdio_ftell, I, F),
   MAKE_INTRINSIC_1("clearerr", stdio_clearerr, V, F),
   MAKE_INTRINSIC_4("fread", stdio_fread, V, R, D, U, F),
   MAKE_INTRINSIC_1("fwrite", stdio_fwrite, V, F),
#ifdef HAVE_POPEN
   MAKE_INTRINSIC_SS("popen", stdio_popen, V),
   MAKE_INTRINSIC_1("pclose", stdio_fclose, I, F),
#endif
   SLANG_END_INTRIN_FUN_TABLE
};
#undef F
#undef I
#undef R
#undef V
#undef S
#undef B
#undef U
#undef D

#ifndef SEEK_SET
# define SEEK_SET 0
#endif
#ifndef SEEK_CUR
# define SEEK_CUR 1
#endif
#ifndef SEEK_END
# define SEEK_END 2
#endif

static SLang_IConstant_Type Stdio_Consts [] =
{
   MAKE_ICONSTANT("SEEK_SET", SEEK_SET),
   MAKE_ICONSTANT("SEEK_END", SEEK_END),
   MAKE_ICONSTANT("SEEK_CUR", SEEK_CUR),
   SLANG_END_ICONST_TABLE
};

static void destroy_file_type (unsigned char type, VOID_STAR ptr)
{
   (void) type;
   (void) close_file_type ((SL_File_Table_Type *) ptr);
}


struct _SLang_Foreach_Context_Type
{
   SLang_MMT_Type *mmt;
   FILE *fp;
#define CTX_USE_LINE		1
#define CTX_USE_CHAR		2
#define CTX_USE_LINE_WS		3
   unsigned char type;
};


static SLang_Foreach_Context_Type *
cl_foreach_open (unsigned char type, unsigned int num)
{
   SLang_Foreach_Context_Type *c;
   SLang_MMT_Type *mmt;
   FILE *fp;

   if (NULL == (mmt = pop_fp (SL_READ, &fp)))
     return NULL;

   /* type = CTX_USE_LINE; */

   switch (num)
     {
	char *s;

      case 0:
	type = CTX_USE_LINE;
	break;
	
      case 1:
	if (-1 == SLang_pop_slstring (&s))
	  {
	     SLang_free_mmt (mmt);
	     return NULL;
	  }
	if (0 == strcmp (s, "char"))
	  type = CTX_USE_CHAR;
	else if (0 == strcmp (s, "line"))
	  type = CTX_USE_LINE;
	else if (0 == strcmp (s, "wsline"))
	  type = CTX_USE_LINE_WS;
	else
	  {
	     SLang_verror (SL_NOT_IMPLEMENTED,
			   "using '%s' not supported by File_Type",
			   s);
	     SLang_free_slstring (s);
	     SLang_free_mmt (mmt);
	     return NULL;
	  }
	SLang_free_slstring (s);
	break;

      default:
	SLdo_pop_n (num);
	SLang_verror (SL_NOT_IMPLEMENTED, 
		      "Usage: foreach (File_Type) using ([line|char])");
	SLang_free_mmt (mmt);
	return NULL;
     }

   if (NULL == (c = (SLang_Foreach_Context_Type *) SLmalloc (sizeof (SLang_Foreach_Context_Type))))
     {
	SLang_free_mmt (mmt);
	return NULL;
     }
   memset ((char *) c, 0, sizeof (SLang_Foreach_Context_Type));

   c->type = type;
   c->mmt = mmt;
   c->fp = fp;

   return c;
}

static void cl_foreach_close (unsigned char type, SLang_Foreach_Context_Type *c)
{
   (void) type;
   if (c == NULL) return;
   SLang_free_mmt (c->mmt);
   SLfree ((char *) c);
}

static int cl_foreach (unsigned char type, SLang_Foreach_Context_Type *c)
{
   int status;
   int ch;
   unsigned int len;
   char *s;

   (void) type;

   if (c == NULL)
     return -1;
   
   switch (c->type)
     {
      case CTX_USE_CHAR:
	if (EOF == (ch = getc (c->fp)))
	  return 0;
	if (-1 == SLang_push_uchar ((unsigned char) ch))
	  return -1;
	return 1;

      case CTX_USE_LINE:
      case CTX_USE_LINE_WS:
	status = read_one_line (c->fp, &s, &len, (c->type==CTX_USE_LINE_WS));
	if (status <= 0)
	  return status;
	if (0 == _SLang_push_slstring (s))
	  return 1;
	return -1;
     }
   
   return -1;
}

static int Stdio_Initialized;
static SLang_MMT_Type *Stdio_Mmts[3];

int SLang_init_stdio (void)
{
   unsigned int i;
   SL_File_Table_Type *s;
   SLang_Class_Type *cl;
   char *names[3];

   if (Stdio_Initialized)
     return 0;

   SL_File_Table = (SL_File_Table_Type *)SLcalloc(sizeof (SL_File_Table_Type), SL_MAX_FILES);
   if (SL_File_Table == NULL)
     return -1;

   if (NULL == (cl = SLclass_allocate_class ("File_Type")))
     return -1;
   cl->cl_destroy = destroy_file_type;
   cl->cl_foreach_open = cl_foreach_open;
   cl->cl_foreach_close = cl_foreach_close;
   cl->cl_foreach = cl_foreach;


   if (-1 == SLclass_register_class (cl, SLANG_FILE_PTR_TYPE, sizeof (SL_File_Table_Type), SLANG_CLASS_TYPE_MMT))
     return -1;

   if ((-1 == SLadd_intrin_fun_table(Stdio_Name_Table, "__STDIO__"))
       || (-1 == SLadd_iconstant_table (Stdio_Consts, NULL))
       || (-1 == _SLerrno_init ()))
     return -1;

   names[0] = "stdin";
   names[1] = "stdout";
   names[2] = "stderr";

   s = SL_File_Table;
   s->fp = stdin;  s->flags = SL_READ;

   s++;
   s->fp = stdout;  s->flags = SL_WRITE;

   s++;
   s->fp = stderr;  s->flags = SL_WRITE|SL_READ;

   s = SL_File_Table;
   for (i = 0; i < 3; i++)
     {
	if (NULL == (s->file = SLang_create_slstring (names[i])))
	  return -1;

	if (NULL == (Stdio_Mmts[i] = SLang_create_mmt (SLANG_FILE_PTR_TYPE, (VOID_STAR) s)))
	  return -1;
	SLang_inc_mmt (Stdio_Mmts[i]);

	if (-1 == SLadd_intrinsic_variable (s->file, (VOID_STAR)&Stdio_Mmts[i], SLANG_FILE_PTR_TYPE, 1))
	  return -1;
	s++;
     }

   Stdio_Initialized = 1;
   return 0;
}

