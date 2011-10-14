/* This module implements and interface to the SLang SMG routines */
#include <stdio.h>
#include <slang.h>

SLANG_MODULE(smg);

/* If this is +1, the then it is ok to call the SLsmg routines.  If it is 
 * 0, then only SLsmg_init_smg may be called.  If it is -1, then SLsmg is
 * suspended and one must call SLsmg_resume_smg.
 */

static int Smg_Initialized;

static void smg_write_to_status_line (char *s)
{
   if (Smg_Initialized <= 0)
     return;
   
   (void) SLtt_write_to_status_line (s, 0);
}

   
static void smg_suspend_smg (void)
{
   if (Smg_Initialized <= 0)
     return;

   (void) SLsmg_suspend_smg ();
   Smg_Initialized = -1;
}

static void smg_resume_smg (void)
{
   if (Smg_Initialized != -1)
     return;

   (void) SLsmg_resume_smg ();
   Smg_Initialized = 1;
}

static void smg_erase_eol (void)
{
   if (Smg_Initialized <= 0)
     return;
   SLsmg_erase_eol ();
}

static void smg_gotorc (int *r, int *c)
{
   if (Smg_Initialized <= 0)
     return;
   SLsmg_gotorc (*r, *c);
}

static void smg_erase_eos (void)
{
   if (Smg_Initialized <= 0)
     return;
   SLsmg_erase_eos ();
}

static void smg_reverse_video (void)
{
   if (Smg_Initialized <= 0)
     return;
   SLsmg_reverse_video ();
}

static void smg_set_color (int *c)
{
   if (Smg_Initialized <= 0)
     return;
   SLsmg_set_color (*c);
}

static void smg_normal_video (void)
{
   if (Smg_Initialized <= 0)
     return;
   SLsmg_normal_video ();
}

static void smg_write_string (char *s)
{
   if (Smg_Initialized <= 0)
     return;
   SLsmg_write_string (s);
}

static void smg_write_nstring (char *s, int *len)
{
   if ((Smg_Initialized <= 0)
       || (*len < 0))
     return;
   
   SLsmg_write_nstring (s, (unsigned int) *len);
}

static void smg_write_wrapped_string (char *s, int *r, int *c, int *dr, int *dc, 
				      int *fill)
{
   if (Smg_Initialized <= 0)
     return;
   
   SLsmg_write_wrapped_string (s, *r, *c, *dr, *dc, *fill);
}

static int smg_char_at (void)
{
   if (Smg_Initialized <= 0) return -1;
   return (int) SLsmg_char_at ();
}

static void smg_set_screen_start (int *rp, int *cp)
{
   int r, c;
   
   if (Smg_Initialized <= 0) return;
   r = *rp;
   c = *cp;
   SLsmg_set_screen_start (&r, &c);
}

static void smg_draw_hline (int *dn)
{
   if (Smg_Initialized <= 0)
     return;
   
   SLsmg_draw_hline (*dn);
}

static void smg_draw_vline (int *dn)
{
   if (Smg_Initialized <= 0)
     return;
   
   SLsmg_draw_vline (*dn);
}

static void smg_draw_object (int *r, int *c, int *obj)
{
   if (Smg_Initialized <= 0) return;
   SLsmg_draw_object (*r, *c, *obj);
}

static void smg_draw_box (int *r, int *c,int *dr, int *dc)
{
   if (Smg_Initialized <= 0) return;
   SLsmg_draw_box (*r, *c, *dr, *dc);
}

static int smg_get_column (void)
{
   if (Smg_Initialized <= 0) return -1;
   return SLsmg_get_column();
}

static int smg_get_row (void)
{
   if (Smg_Initialized <= 0) return -1;
   return SLsmg_get_row();
}

static void smg_forward (int *n)
{
   if (Smg_Initialized <= 0) return;
   SLsmg_forward (*n);
}

static void smg_set_color_in_region (int *color, int *r, int *c, int *dr, int *dc)
{
   if (Smg_Initialized <= 0) return;
   SLsmg_set_color_in_region (*color, *r, *c, *dr, *dc);
}


static void smg_cls (void)
{
   if (Smg_Initialized <= 0)
     return;
   SLsmg_cls ();
}

static void smg_refresh (void)
{
   if (Smg_Initialized <= 0)
     return;
   SLsig_block_signals ();
   SLsmg_refresh ();
   SLsig_unblock_signals ();
}

static void smg_reset_smg (void)
{
   if (Smg_Initialized <= 0)
     return;
   SLsig_block_signals ();
   SLsmg_reset_smg ();
   SLsig_unblock_signals ();
   Smg_Initialized = 0;
}

static void smg_init_smg (void)
{
   if (Smg_Initialized != 0)
     return;
   SLsig_block_signals ();
   (void) SLsmg_init_smg ();
   SLsig_unblock_signals ();
   Smg_Initialized = 1;
}

static void smg_define_color (int *obj, char *fg, char *bg)
{
   SLtt_set_color (*obj, NULL, fg, bg);
}

#define I SLANG_INT_TYPE
#define S SLANG_STRING_TYPE
static SLang_Intrin_Fun_Type Smg_Intrinsics [] =
{
   MAKE_INTRINSIC_0("smg_suspend_smg", smg_suspend_smg, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("smg_resume_smg", smg_resume_smg, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("smg_erase_eol", smg_erase_eol, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_II("smg_gotorc", smg_gotorc, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("smg_erase_eos", smg_erase_eos, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("smg_reverse_video", smg_reverse_video, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_I("smg_set_color", smg_set_color, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("smg_normal_video", smg_normal_video, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("smg_write_string", smg_write_string, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("smg_cls", smg_cls, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("smg_refresh", smg_refresh, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("smg_reset_smg", smg_reset_smg, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("smg_init_smg", smg_init_smg, SLANG_VOID_TYPE),

   MAKE_INTRINSIC_SI("smg_write_nstring", smg_write_nstring, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_6("smg_write_wrapped_string", smg_write_wrapped_string, SLANG_VOID_TYPE, S,I,I,I,I,I),
   MAKE_INTRINSIC_0("smg_char_at", smg_char_at, SLANG_INT_TYPE),
   MAKE_INTRINSIC_II("smg_set_screen_start", smg_set_screen_start, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_I("smg_draw_hline", smg_draw_hline, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_I("smg_draw_vline", smg_draw_vline, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_III("smg_draw_object", smg_draw_object, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_4("smg_draw_box", smg_draw_box, SLANG_VOID_TYPE,I,I,I,I),
   MAKE_INTRINSIC_0("smg_get_column", smg_get_column, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("smg_get_row", smg_get_row, SLANG_INT_TYPE),
   MAKE_INTRINSIC_I("smg_forward", smg_forward, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_5("smg_set_color_in_region", smg_set_color_in_region, SLANG_VOID_TYPE, I, I, I, I, I),

   MAKE_INTRINSIC_ISS("smg_define_color", smg_define_color, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("smg_write_to_status_line", smg_write_to_status_line, SLANG_VOID_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

static SLang_Intrin_Var_Type Smg_Variables [] =
{
   MAKE_VARIABLE("Smg_Display_Eight_Bit", &SLsmg_Display_Eight_Bit, I, 0),
   MAKE_VARIABLE("Smg_Tab_Width", &SLsmg_Tab_Width, I, 0),
   MAKE_VARIABLE("Smg_Newline_Behavior", &SLsmg_Newline_Behavior, I, 0),
   MAKE_VARIABLE("Smg_Backspace_Moves", &SLsmg_Backspace_Moves, I, 0),
   MAKE_VARIABLE("Smg_Screen_Rows", &SLtt_Screen_Rows, I, 1),
   MAKE_VARIABLE("Smg_Screen_Cols", &SLtt_Screen_Cols, I, 1),
   SLANG_END_INTRIN_VAR_TABLE
};

static SLang_IConstant_Type Smg_Constants [] =
{
   MAKE_ICONSTANT("SMG_NEWLINE_IGNORED", SLSMG_NEWLINE_IGNORED),
   MAKE_ICONSTANT("SMG_NEWLINE_MOVES", SLSMG_NEWLINE_MOVES),
   MAKE_ICONSTANT("SMG_NEWLINE_SCROLLS", SLSMG_NEWLINE_SCROLLS),
   MAKE_ICONSTANT("SMG_NEWLINE_PRINTABLE", SLSMG_NEWLINE_PRINTABLE),

   MAKE_ICONSTANT("SMG_HLINE_CHAR", SLSMG_HLINE_CHAR),
   MAKE_ICONSTANT("SMG_VLINE_CHAR", SLSMG_VLINE_CHAR),
   MAKE_ICONSTANT("SMG_ULCORN_CHAR", SLSMG_ULCORN_CHAR),
   MAKE_ICONSTANT("SMG_URCORN_CHAR", SLSMG_URCORN_CHAR),
   MAKE_ICONSTANT("SMG_LLCORN_CHAR", SLSMG_LLCORN_CHAR),
   MAKE_ICONSTANT("SMG_LRCORN_CHAR", SLSMG_LRCORN_CHAR),
   MAKE_ICONSTANT("SMG_CKBRD_CHAR", SLSMG_CKBRD_CHAR),
   MAKE_ICONSTANT("SMG_RTEE_CHAR", SLSMG_RTEE_CHAR),
   MAKE_ICONSTANT("SMG_LTEE_CHAR", SLSMG_LTEE_CHAR),
   MAKE_ICONSTANT("SMG_UTEE_CHAR", SLSMG_UTEE_CHAR),
   MAKE_ICONSTANT("SMG_DTEE_CHAR", SLSMG_DTEE_CHAR),
   MAKE_ICONSTANT("SMG_PLUS_CHAR", SLSMG_PLUS_CHAR),

   SLANG_END_ICONST_TABLE
};
#undef I
#undef S

int init_smg_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if ((-1 == SLns_add_intrin_fun_table (ns, Smg_Intrinsics, "__SMG__"))
       || (-1 == SLns_add_intrin_var_table (ns, Smg_Variables, NULL))
       || (-1 == SLns_add_iconstant_table (ns, Smg_Constants, NULL)))
     return -1;

   SLtt_get_terminfo ();
   Smg_Initialized = 0;
   return 0;
}

#if 0
int init_smg_module (void)
{
   if ((-1 == SLadd_intrin_fun_table (Smg_Intrinsics, "__SMG__"))
       || (-1 == SLadd_intrin_var_table (Smg_Variables, NULL))
       || (-1 == SLadd_iconstant_table (Smg_Constants, NULL)))
     return -1;

   SLtt_get_terminfo ();
   Smg_Initialized = 0;
   return 0;
}
#endif

/* This function is optional */
void deinit_smg_module (void)
{
   smg_reset_smg ();
}
