/* Copyright (c) 1998, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

#define SLANG_HAS_DYNAMIC_LINKING 1

#ifndef HAVE_DLFCN_H
# undef SLANG_HAS_DYNAMIC_LINKING
# define SLANG_HAS_DYNAMIC_LINKING	0
#endif

/* The rest of this file is in the if block */
#if SLANG_HAS_DYNAMIC_LINKING

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

static char *Module_Path;
#define MODULE_PATH_ENV_NAME	"SLANG_MODULE_PATH"
#ifndef MODULE_INSTALL_DIR
# define MODULE_INSTALL_DIR "/usr/local/lib/slang/modules"
#endif

typedef struct _Handle_Type
{
   struct _Handle_Type *next;
   char *name;
   VOID_STAR handle;
   void (*deinit_fun) (void);
}
Handle_Type;

static Handle_Type *Handle_List;

static void delete_handles (void)
{
   while (Handle_List != NULL)
     {
	Handle_Type *next = Handle_List->next;

	if (Handle_List->deinit_fun != NULL)
	  Handle_List->deinit_fun ();
	(void) dlclose (Handle_List->handle);
	SLang_free_slstring (Handle_List->name);
	SLfree ((char *)Handle_List);
	Handle_List = next;
     }
}

static Handle_Type *save_handle (char *name, VOID_STAR h, void (*df)(void))
{
   Handle_Type *l;

   l = (Handle_Type *) SLmalloc (sizeof (Handle_Type));
   if (l == NULL)
     return NULL;
   memset ((char *) l, 0, sizeof(Handle_Type));
   if (NULL == (l->name = SLang_create_slstring (name)))
     {
	SLfree ((char *) l);
	return NULL;
     }
   l->handle = h;
   l->next = Handle_List;
   l->deinit_fun = df;
   Handle_List = l;

   return l;
}

static Handle_Type *find_handle (char *name)
{
   Handle_Type *l;

   l = Handle_List;
   while (l != NULL)
     {
	if (0 == strcmp (l->name, name))
	  break;
	l = l->next;
     }
   return l;
}

static int import_from_library (char *name, 
				char *init_fun_name, char *deinit_fun_name,
				char *file,
				char *ns,
				char *ns_init_fun_name)
{
   VOID_STAR handle;
   int (*init_fun) (void);
   int (*ns_init_fun) (char *);
   void (*deinit_fun) (void);
   char *err;
   char filebuf[1024];
   char *fun_name;
   char *save_file;
   char *save_err;

   if (NULL != find_handle (name))
     return 0;			       /* already loaded */
   
   save_err = NULL;
   save_file = file;
   while (1)
     {
#ifndef RTLD_GLOBAL
# define RTLD_GLOBAL 0
#endif
#ifdef RTLD_NOW
	handle = (VOID_STAR) dlopen (file, RTLD_NOW | RTLD_GLOBAL);
#else
	handle = (VOID_STAR) dlopen (file, RTLD_LAZY | RTLD_GLOBAL);
#endif

	if (handle != NULL)
	  {
	     if (save_err != NULL)
	       SLfree (save_err);
	     break;
	  }

	/* Purify reports that dlerror returns a pointer that generates UMR
	 * errors.  There is nothing that I can do about that....
	 */
	if ((NULL == strchr (file, '/'))
	    && (strlen(file) < sizeof(filebuf)))
	  {
	     err = (char *) dlerror ();
	     if (err != NULL)
	       save_err = SLmake_string (err);

	     _SLsnprintf (filebuf, sizeof (filebuf), "./%s", file);
	     file = filebuf;
	     continue;
	  }

	if ((NULL == (err = save_err))
	    && (NULL == (err = (char *) dlerror ())))
	  err = "UNKNOWN";

	SLang_verror (SL_INTRINSIC_ERROR,
		      "Error linking to %s: %s", save_file, err);
	
	if (save_err != NULL)
	  SLfree (save_err);

	return -1;
     }

   fun_name = ns_init_fun_name;
   ns_init_fun = (int (*)(char *)) dlsym (handle, fun_name);

   if ((ns == NULL) || (*ns == 0))
     ns = "Global";

   if (ns_init_fun == NULL) 
     {
	if (0 != strcmp (ns, "Global"))
	  goto return_error;

	fun_name = init_fun_name;
	init_fun = (int (*)(void)) dlsym (handle, fun_name);
	if (init_fun == NULL)
	  goto return_error;

	if (-1 == (*init_fun) ())
	  {
	     dlclose (handle);
	     return -1;
	  }
     }
   else if (-1 == (*ns_init_fun) (ns))
     {
	dlclose (handle);
	return -1;
     }


   deinit_fun = (void (*)(void)) dlsym (handle, deinit_fun_name);

   (void) save_handle (name, handle, deinit_fun);
   return 0;

   return_error:
   
   if (NULL == (err = (char *) dlerror ()))
     err = "UNKNOWN";

   dlclose (handle);
   SLang_verror (SL_INTRINSIC_ERROR,
		 "Unable to get symbol %s from %s: %s",
		 name, file, err);
   return -1;
}

static void import_module (void)
{
#define MAX_MODULE_NAME_SIZE 256
   char module_name[MAX_MODULE_NAME_SIZE];
   char symbol_name[MAX_MODULE_NAME_SIZE];
   char deinit_name[MAX_MODULE_NAME_SIZE];
   char ns_init_name[MAX_MODULE_NAME_SIZE];
   char *path;
   char *file;
   char *module;
   char *ns = NULL;

   if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_slstring (&ns))
	  return;	
     }
   
   if (-1 == SLang_pop_slstring (&module))
     {
	SLang_free_slstring (ns);      /* NULL ok */
	return;
     }

   if (strlen (module) >= MAX_MODULE_NAME_SIZE)
     {
	SLang_verror (SL_INVALID_PARM, "module name too long");
	SLang_free_slstring (module);
	SLang_free_slstring (ns);
	return;
     }
   
   _SLsnprintf (symbol_name, sizeof(symbol_name), "init_%s_module", module);
   _SLsnprintf (module_name, sizeof(module_name), "%s-module.so", module);
   _SLsnprintf (deinit_name, sizeof(deinit_name), "deinit_%s_module", module);
   _SLsnprintf (ns_init_name, sizeof (ns_init_name), "init_%s_module_ns", module);

   if (Module_Path != NULL)
     file = SLpath_find_file_in_path (Module_Path, module_name);
   else file = NULL;

   if ((file == NULL)
       && (NULL != (path = _SLsecure_getenv (MODULE_PATH_ENV_NAME))))
     file = SLpath_find_file_in_path (path, module_name);

   if (file == NULL)
     file = SLpath_find_file_in_path (MODULE_INSTALL_DIR, module_name);

   if (file != NULL)
     {
	(void) import_from_library (symbol_name, symbol_name, deinit_name, file, ns, ns_init_name);
	SLfree (file);
     }
   else
     {
	/* Maybe the system loader can find it in LD_LIBRARY_PATH */
	(void) import_from_library (symbol_name, symbol_name, deinit_name, module_name, ns, ns_init_name);
     }
}

static void set_import_module_path (char *path)
{
   (void) SLang_set_module_load_path (path);
}

static char *get_import_module_path (void)
{
   char *path;
   if (Module_Path != NULL)
     return Module_Path;
   if (NULL != (path = _SLsecure_getenv (MODULE_PATH_ENV_NAME)))
     return path;
   return MODULE_INSTALL_DIR;
}

static SLang_Intrin_Fun_Type Module_Intrins [] =
{
   MAKE_INTRINSIC_0("import", import_module, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("set_import_module_path", set_import_module_path, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("get_import_module_path", get_import_module_path, SLANG_STRING_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

#endif				       /* SLANG_HAS_DYNAMIC_LINKING */

int SLang_set_module_load_path (char *path)
{
#if SLANG_HAS_DYNAMIC_LINKING
   if (NULL == (path = SLang_create_slstring (path)))
     return -1;
   SLang_free_slstring (Module_Path);
   Module_Path = path;
   return 0;
#else
   (void) path;
   return -1;
#endif
}

int SLang_init_import (void)
{
#if SLANG_HAS_DYNAMIC_LINKING
   (void) SLang_add_cleanup_function (delete_handles);
   return SLadd_intrin_fun_table (Module_Intrins, "__IMPORT__");
#else
   return 0;
#endif
}
