/* Pathname and filename functions */
/* Copyright (c) 1998, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#ifdef HAVE_IO_H
# include <io.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

#include <errno.h>
#include <string.h>

#include "slang.h"
#include "_slang.h"

/* In this file, all file names are assumed to be specified in the Unix
 * format, or in the native format.
 *
 * Aboout VMS:
 * VMS pathnames are a mess.  In general, they look like
 * node::device:[dir.dir]file.ext;version
 * and I do not know of a well-defined Unix representation for them.  So,
 * I am going to punt and encourage users to stick to the native
 * representation.
 */

#if defined(IBMPC_SYSTEM)
# define PATH_SEP		'\\'
# define DRIVE_SPECIFIER	':'
# define SEARCH_PATH_DELIMITER	';'
# define THIS_DIR_STRING	"."
#else
# if defined(VMS)
#  define PATH_SEP		']'
#  define DRIVE_SPECIFIER	':'
#  define SEARCH_PATH_DELIMITER	' '
#  define THIS_DIR_STRING	"[]"   /* Is this correct?? */
# else
#  define PATH_SEP		'/'
#  define UNIX_PATHNAMES_OK
#  define SEARCH_PATH_DELIMITER	':'
#  define THIS_DIR_STRING	"."
# endif
#endif

#ifdef UNIX_PATHNAMES_OK
# define IS_PATH_SEP(x) ((x) == PATH_SEP)
#else
# define IS_PATH_SEP(x)	(((x) == PATH_SEP) || ((x) == '/'))
#endif

static char Path_Delimiter = SEARCH_PATH_DELIMITER;

/* If file is /a/b/c/basename, this function returns a pointer to basename */
char *SLpath_basename (char *file)
{
   char *b;

   if (file == NULL) return NULL;
   b = file + strlen (file);

   while (b != file)
     {
	b--;
	if (IS_PATH_SEP(*b))
	  return b + 1;
#ifdef DRIVE_SPECIFIER
	if (*b == DRIVE_SPECIFIER)
	  return b + 1;
#endif
     }

   return b;
}

/* Returns a malloced string */
char *SLpath_pathname_sans_extname (char *file)
{
   char *b;

   file = SLmake_string (file);
   if (file == NULL)
     return NULL;

   b = file + strlen (file);

   while (b != file)
     {
	b--;
	if (*b == '.')
	  {
	     *b = 0;
	     return file;
	  }
     }

   return file;
}

/* If path looks like: A/B/C/D/whatever, it returns A/B/C/D as a malloced 
 * string.
 */
char *SLpath_dirname (char *file)
{
   char *b;

   if (file == NULL) return NULL;
   b = file + strlen (file);

   while (b != file)
     {
	b--;
	if (IS_PATH_SEP(*b))
	  {
	     if (b == file) b++;
	     break;
	  }

#ifdef DRIVE_SPECIFIER
	if (*b == DRIVE_SPECIFIER)
	  {
	     b++;
	     break;
	  }
#endif
     }
   
   if (b == file)
     return SLmake_string (THIS_DIR_STRING);
   
   return SLmake_nstring (file, (unsigned int) (b - file));
}

/* Note: VMS filenames also contain version numbers.  The caller will have
 * to deal with that.
 * 
 * The extension includes the '.'.  If no extension is present, "" is returned.
 */
char *SLpath_extname (char *file)
{
   char *b;

   if (NULL == (file = SLpath_basename (file)))
     return NULL;

   b = file + strlen (file);
   while (b != file)
     {
	b--;
	if (*b == '.')
	  return b;
     }
   
   if (*b == '.')
     return b;

   /* Do not return a literal "" */
   return file + strlen (file);
}

#ifdef IBMPC_SYSTEM
static void convert_slashes (char *f)
{
   while (*f)
     {
	if (*f == '/') *f = PATH_SEP;
	f++;
     }
}
#endif

int SLpath_is_absolute_path (char *name)
{
   if (name == NULL)
     return -1;

#ifdef UNIX_PATHNAMES_OK
   return (*name == '/');
#else
   if (IS_PATH_SEP (*name))
     return 1;

# ifdef DRIVE_SPECIFIER
   /* Look for a drive specifier */
   while (*name)
     {
	if (*name == DRIVE_SPECIFIER)
	  return 1;

	name++;
     }
# endif

   return 0;
#endif
}


/* This returns a MALLOCED string */
char *SLpath_dircat (char *dir, char *name)
{
   unsigned int len, dirlen;
   char *file;
#ifndef VMS
   int requires_fixup;
#endif

   if (name == NULL)
     name = "";

   if ((dir == NULL) || (SLpath_is_absolute_path (name)))
     dir = "";

   /* Both VMS and MSDOS have default directories associated with each drive.
    * That is, the meaning of something like C:X depends upon more than just
    * the syntax of the string.  Since this concept has more power under VMS
    * it will be honored here.  However, I am going to treat C:X as C:\X
    * under MSDOS.
    *
    * Note!!!
    * VMS has problems of its own regarding path names, so I am simply
    * going to strcat.  Hopefully the VMS RTL is smart enough to deal with
    * the result.
    */
   dirlen = strlen (dir);
#ifndef VMS
   requires_fixup = (dirlen && (0 == IS_PATH_SEP(dir[dirlen - 1])));
#endif

   len = dirlen + strlen (name) + 2;
   if (NULL == (file = SLmalloc (len)))
     return NULL;

   strcpy (file, dir);

#ifndef VMS
   if (requires_fixup)
     file[dirlen++] = PATH_SEP;
#endif

   strcpy (file + dirlen, name);

#if defined(IBMPC_SYSTEM)
   convert_slashes (file);
#endif

   return file;
}

int SLpath_file_exists (char *file)
{
   struct stat st;
   int m;

#if defined(__os2__) && !defined(S_IFMT)
/* IBM VA3 doesn't declare S_IFMT */
# define	S_IFMT	(S_IFDIR | S_IFCHR | S_IFREG)
#endif

#ifdef _S_IFDIR
# ifndef S_IFDIR
#  define S_IFDIR _S_IFDIR
# endif
#endif

#ifndef S_ISDIR
# ifdef S_IFDIR
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
# else
#  define S_ISDIR(m) 0
# endif
#endif

   if (file == NULL)
     return -1;

   if (stat(file, &st) < 0) return 0;
   m = st.st_mode;

   if (S_ISDIR(m)) return (2);
   return 1;
}

/* By relatively-absolute, I mean paths of the form ./foo,
 * and ../foo/bar.  But not foo/bar.
 */
static int is_relatively_absolute (char *file)
{
   if (file == NULL)
     return -1;
   if (SLpath_is_absolute_path (file))
     return 1;
   
#if defined(VMS)
   return (*file == '[');
#else
   if (*file == '.') file++;
   if (*file == '.') file++;
   return (*file == PATH_SEP);
#endif
}

char *SLpath_find_file_in_path (char *path, char *name)
{
   unsigned int max_path_len;
   unsigned int this_path_len;
   char *file, *dir;
   char *p;
   unsigned int nth;

   if ((path == NULL) || (*path == 0)
       || (name == NULL) || (*name == 0))
     return NULL;
   
   if (is_relatively_absolute (name))
     {
	if (0 == SLpath_file_exists (name))
	  return NULL;
	return SLmake_string (name);
     }

   /* Allow "." to mean the current directory on all systems */
   if ((path[0] == '.') && (path[1] == 0))
     {
	if (0 == SLpath_file_exists (name))
	  return NULL;
	return SLpath_dircat (THIS_DIR_STRING, name);
     }
   
   max_path_len = 0;
   this_path_len = 0;
   p = path;
   while (*p != 0)
     {
	if (*p++ == Path_Delimiter)
	  {
	     if (this_path_len > max_path_len) max_path_len = this_path_len;
	     this_path_len = 0;
	  }
	else this_path_len++;
     }
   if (this_path_len > max_path_len) max_path_len = this_path_len;
   max_path_len++;

   if (NULL == (dir = SLmalloc (max_path_len)))
     return NULL;

   nth = 0;
   while (-1 != SLextract_list_element (path, nth, Path_Delimiter,
					dir, max_path_len))
     {
	nth++;
	if (*dir == 0)
	  continue;

	if (NULL == (file = SLpath_dircat (dir, name)))
	  {
	     SLfree (dir);
	     return NULL;
	  }

	if (1 == SLpath_file_exists (file))
	  {
	     SLfree (dir);
	     return file;
	  }

	SLfree (file);
     }

   SLfree (dir);
   return NULL;
}

int SLpath_get_delimiter (void)
{
   return Path_Delimiter;
}

int SLpath_set_delimiter (int d)
{
   char ch = (char) d;
   if (ch == 0)
     return -1;
   
   Path_Delimiter = ch;
   return 0;
}
