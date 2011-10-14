/* SLang_read_line interface --- uses SLang tty stuff */
/* Copyright (c) 1992, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */

#include "slinclud.h"

#include "slang.h"
#include "_slang.h"

#ifdef REAL_UNIX_SYSTEM
int SLang_RL_EOF_Char = 4;
#else
int SLang_RL_EOF_Char = 26;
#endif

int SLang_Rline_Quit;
static SLang_RLine_Info_Type *This_RLI;

static unsigned char Char_Widths[256];
static void position_cursor (int);

static void rl_beep (void)
{
   putc(7, stdout);
   fflush (stdout);
}

/* editing functions */
static int rl_bol (void)
{
   if (This_RLI->point == 0) return 0;
   This_RLI->point = 0;
   return 1;
}

static int rl_eol (void)
{
   if (This_RLI->point == This_RLI->len) return 0;
   This_RLI->point = This_RLI->len;
   return 1;
}

static int rl_right (void)
{
   if (This_RLI->point == This_RLI->len) return 0;
   This_RLI->point++;
   return 1;
}

static int rl_left (void)
{
   if (This_RLI->point == 0) return 0;
   This_RLI->point--;
   return 1;
}

static int rl_self_insert (void)
{
   unsigned char *pmin, *p;

   if (This_RLI->len == This_RLI->buf_len)
     {
	rl_beep ();
	return 0;
     }

   pmin = This_RLI->buf + This_RLI->point;
   p = This_RLI->buf + This_RLI->len;
   while (p > pmin)
     {
	*p = *(p - 1);
	p--;
     }
   *pmin = SLang_Last_Key_Char;

   This_RLI->len++;
   This_RLI->point++;
   if ((This_RLI->curs_pos + 2 >= This_RLI->edit_width)
       || (This_RLI->tt_insert == NULL)
       || (Char_Widths[SLang_Last_Key_Char] != 1)) return 1;

   (*This_RLI->tt_insert)((char) SLang_Last_Key_Char);
   /* update screen buf */
   p = This_RLI->old_upd + (This_RLI->len - 1);
   pmin = This_RLI->old_upd + (This_RLI->point - 1);
   while (p > pmin)
     {
	*p = *(p - 1);
	p--;
     }
   *pmin = SLang_Last_Key_Char;
   return 0;
}

int SLang_rline_insert (char *s)
{
   unsigned char *pmin, *p;
   int n;

   n = strlen (s);
   if (n > This_RLI->buf_len - This_RLI->len)
     n = This_RLI->buf_len - This_RLI->len;

   if (n == 0) return 0;

   pmin = This_RLI->buf + This_RLI->point;
   p = This_RLI->buf + (This_RLI->len - 1);

   while (p >= pmin)
     {
	*(p + n) = *p;
	p--;
     }
   SLMEMCPY ((char *) pmin, s, n);

   This_RLI->len += n;
   This_RLI->point += n;
   return n;
}

static int rl_deln (int n)
{
   unsigned char *pmax, *p;

   p = This_RLI->buf + This_RLI->point;
   pmax = This_RLI->buf + This_RLI->len;

   if (p + n > pmax) n = (int) (pmax - p);
   while (p < pmax)
     {
	*p = *(p + n);
	p++;
     }
   This_RLI->len -= n;
   return n;
}

static int rl_del (void)
{
   return rl_deln(1);
}

static int rl_quote_insert (void)
{
   int err = SLang_Error;
   SLang_Error = 0;
   SLang_Last_Key_Char = (*This_RLI->getkey)();
   rl_self_insert ();
   if (SLang_Error == SL_USER_BREAK) SLang_Error = 0;
   else SLang_Error = err;
   return 1;
}

static int rl_trim (void)
{
   unsigned char *p, *pmax, *p1;
   p = This_RLI->buf + This_RLI->point;
   pmax = This_RLI->buf + This_RLI->len;

   if (p == pmax)
     {
	if (p == This_RLI->buf) return 0;
	p--;
     }

   if ((*p != ' ') && (*p != '\t')) return 0;
   p1 = p;
   while ((p1 < pmax) && ((*p1 == ' ') || (*p1 == '\t'))) p1++;
   pmax = p1;
   p1 = This_RLI->buf;

   while ((p >= p1) && ((*p == ' ') || (*p == '\t'))) p--;
   if (p == pmax) return 0;
   p++;

   This_RLI->point = (int) (p - p1);
   return rl_deln ((int) (pmax - p));
}

static int rl_bdel (void)
{
   if (rl_left()) return rl_del();
   return 0;
}

static int rl_deleol (void)
{
   if (This_RLI->point == This_RLI->len) return 0;
   *(This_RLI->buf + This_RLI->point) = 0;
   This_RLI->len = This_RLI->point;
   return 1;
}

static int rl_delete_line (void)
{
   This_RLI->point = 0;
   *(This_RLI->buf + This_RLI->point) = 0;
   This_RLI->len = 0;
   return 1;
}

static int rl_enter (void)
{
   *(This_RLI->buf + This_RLI->len) = 0;
   SLang_Rline_Quit = 1;
   return 1;
}

static SLKeyMap_List_Type *RL_Keymap;

/* This update is designed for dumb terminals.  It assumes only that the
 * terminal can backspace via ^H, and move cursor to start of line via ^M.
 * There is a hook so the user can provide a more sophisticated update if
 * necessary.
 */

static void position_cursor (int col)
{
   unsigned char *p, *pmax;
   int dc;

   if (col == This_RLI->curs_pos)
     {
	fflush (stdout);
	return;
     }

   if (This_RLI->tt_goto_column != NULL)
     {
	(*This_RLI->tt_goto_column)(col);
	This_RLI->curs_pos = col;
	fflush (stdout);
	return;
     }

   dc = This_RLI->curs_pos - col;
   if (dc < 0)
     {
	p = This_RLI->new_upd + This_RLI->curs_pos;
	pmax = This_RLI->new_upd + col;
	while (p < pmax) putc((char) *p++, stdout);
     }
   else
     {
	if (dc < col)
	  {
	     while (dc--) putc(8, stdout);
	  }
	else
	  {
	     putc('\r', stdout);
	     p = This_RLI->new_upd;
	     pmax = This_RLI->new_upd + col;
	     while (p < pmax) putc((char) *p++, stdout);
	  }
     }
   This_RLI->curs_pos = col;
   fflush (stdout);
}

static void erase_eol (SLang_RLine_Info_Type *rli)
{
   unsigned char *p, *pmax;

   p = rli->old_upd + rli->curs_pos;
   pmax = rli->old_upd + rli->old_upd_len;

   while (p++ < pmax) putc(' ', stdout);

   rli->curs_pos = rli->old_upd_len;
}

static unsigned char *spit_out(SLang_RLine_Info_Type *rli, unsigned char *p)
{
   unsigned char *pmax;
   position_cursor ((int) (p - rli->new_upd));
   pmax = rli->new_upd + rli->new_upd_len;
   while (p < pmax) putc((char) *p++, stdout);
   rli->curs_pos = rli->new_upd_len;
   return pmax;
}

static void really_update (SLang_RLine_Info_Type *rli, int new_curs_position)
{
   unsigned char *b = rli->old_upd, *p = rli->new_upd, chb, chp;
   unsigned char *pmax;

   if (rli->update_hook != NULL)
     {
	(*rli->update_hook)(p, rli->edit_width, new_curs_position);
     }
   else
     {
	pmax = p + rli->edit_width;
	while (p < pmax)
	  {
	     chb = *b++; chp = *p++;
	     if (chb == chp) continue;

	     if (rli->old_upd_len <= rli->new_upd_len)
	       {
		  /* easy one */
		  (void) spit_out (rli, p - 1);
		  break;
	       }
	     spit_out(rli, p - 1);
	     erase_eol (rli);
	     break;
	  }
	position_cursor (new_curs_position);
     }

   /* update finished, so swap */

   rli->old_upd_len = rli->new_upd_len;
   p = rli->old_upd;
   rli->old_upd = rli->new_upd;
   rli->new_upd = p;
}

static void RLupdate (SLang_RLine_Info_Type *rli)
{
   int len, dlen, start_len, prompt_len = 0, tw = 0, count;
   int want_cursor_pos;
   unsigned char *b, chb, *b_point, *p;
   int no_echo;

   no_echo = rli->flags & SL_RLINE_NO_ECHO;

   b_point = (unsigned char *) (rli->buf + rli->point);
   *(rli->buf + rli->len) = 0;

   /* expand characters for output buffer --- handle prompt first.
    * Do two passes --- first to find out where to begin upon horiz
    * scroll and the second to actually fill the buffer. */
   len = 0;
   count = 2;			       /* once for prompt and once for buf */

   b = (unsigned char *) rli->prompt;
   while (count--)
     {
	if ((count == 0) && no_echo)
	  break;

	/* The prompt could be NULL */
	if (b != NULL) while ((chb = *b) != 0)
	  {
	     /* This will ensure that the screen is scrolled a third of the edit
	      * width each time */
	     if (b_point == b) break;
	     dlen = Char_Widths[chb];
	     if ((chb == '\t') && tw)
	       {
		  dlen = tw * ((len - prompt_len) / tw + 1) - (len - prompt_len);
	       }
	     len += dlen;
	     b++;
	  }
	tw = rli->tab;
	b = (unsigned char *) rli->buf;
	if (count == 1) prompt_len = len;
     }

   if (len < rli->edit_width - rli->dhscroll) start_len = 0;
   else if ((rli->start_column > len)
	    || (rli->start_column + rli->edit_width <= len))
     {
	start_len = len - (rli->edit_width - rli->dhscroll);
	if (start_len < 0) start_len = 0;
     }
   else start_len = rli->start_column;
   rli->start_column = start_len;

   want_cursor_pos = len - start_len;

   /* second pass */
   p = rli->new_upd;

   len = 0;
   count = 2;
   b = (unsigned char *) rli->prompt;
   if (b == NULL) b = (unsigned char *) "";

   while ((len < start_len) && (*b))
     {
	len += Char_Widths[*b++];
     }

   tw = 0;
   if (*b == 0)
     {
	b = (unsigned char *) rli->buf;
	while (len < start_len)
	  {
	     len += Char_Widths[*b++];
	  }
	tw = rli->tab;
	count--;
     }

   len = 0;
   while (count--)
     {
	if ((count == 0) && (no_echo))
	  break;

	while ((len < rli->edit_width) && ((chb = *b++) != 0))
	  {
	     dlen = Char_Widths[chb];
	     if (dlen == 1) *p++ = chb;
	     else
	       {
		  if ((chb == '\t') && tw)
		    {
		       dlen = tw * ((len + start_len - prompt_len) / tw + 1) - (len + start_len - prompt_len);
		       len += dlen;	       /* ok since dlen comes out 0  */
		       if (len > rli->edit_width) dlen = len - rli->edit_width;
		       while (dlen--) *p++ = ' ';
		       dlen = 0;
		    }
		  else
		    {
		       if (dlen == 3)
			 {
			    chb &= 0x7F;
			    *p++ = '~';
			 }

		       *p++ = '^';
		       if (chb == 127)  *p++ = '?';
		       else *p++ = chb + '@';
		    }
	       }
	     len += dlen;
	  }
	/* if (start_len > prompt_len) break; */
	tw = rli->tab;
	b = (unsigned char *) rli->buf;
     }

   rli->new_upd_len = (int) (p - rli->new_upd);
   while (p < rli->new_upd + rli->edit_width) *p++ = ' ';
   really_update (rli, want_cursor_pos);
}

void SLrline_redraw (SLang_RLine_Info_Type *rli)
{
   unsigned char *p = rli->new_upd;
   unsigned char *pmax = p + rli->edit_width;
   while (p < pmax) *p++ = ' ';
   rli->new_upd_len = rli->edit_width;
   really_update (rli, 0);
   RLupdate (rli);
}

static int rl_eof_insert (void)
{
   if (This_RLI->len == 0)
     {
	SLang_Last_Key_Char = SLang_RL_EOF_Char;
	/* rl_self_insert (); */
	return rl_enter ();
     }
   return 0;
}

/* This is very naive.  It knows very little about nesting and nothing
 * about quoting.
 */
static void blink_match (SLang_RLine_Info_Type *rli)
{
   unsigned char bra, ket;
   unsigned int delta_column;
   unsigned char *p, *pmin;
   int dq_level, sq_level;
   int level;

   pmin = rli->buf;
   p = pmin + rli->point;
   if (pmin == p)
     return;

   ket = SLang_Last_Key_Char;
   switch (ket)
     {
      case ')':
	bra = '(';
	break;
      case ']':
	bra = '[';
	break;
      case '}':
	bra = '{';
	break;
      default:
	return;
     }

   level = 0;
   sq_level = dq_level = 0;

   delta_column = 0;
   while (p > pmin)
     {
	char ch;

	p--;
	delta_column++;
	ch = *p;

	if (ch == ket)
	  {
	     if ((dq_level == 0) && (sq_level == 0))
	       level++;
	  }
	else if (ch == bra)
	  {
	     if ((dq_level != 0) || (sq_level != 0))
	       continue;

	     level--;
	     if (level == 0)
	       {
		  rli->point -= delta_column;
		  RLupdate (rli);
		  (*rli->input_pending)(10);
		  rli->point += delta_column;
		  RLupdate (rli);
		  break;
	       }
	     if (level < 0)
	       break;
	  }
	else if (ch == '"') dq_level = !dq_level;
	else if (ch == '\'') sq_level = !sq_level;
     }
}

int SLang_read_line (SLang_RLine_Info_Type *rli)
{
   unsigned char *p, *pmax;
   SLang_Key_Type *key;

   SLang_Rline_Quit = 0;
   This_RLI = rli;
   p = rli->old_upd; pmax = p + rli->edit_width;
   while (p < pmax) *p++ = ' ';

   /* Sanity checking */
   rli->len = strlen ((char *) rli->buf);
   if (rli->len >= rli->buf_len)
     {
	rli->len = 0;
	*rli->buf = 0;
     }
   if (rli->point > rli->len) rli->point = rli->len;
   if (rli->point < 0) rli->point = 0;

   rli->curs_pos = rli->start_column = 0;
   rli->new_upd_len = rli->old_upd_len = 0;

   This_RLI->last_fun = NULL;
   if (rli->update_hook == NULL)
     putc ('\r', stdout);

   RLupdate (rli);

   while (1)
     {
	key = SLang_do_key (RL_Keymap, (int (*)(void)) rli->getkey);

	if ((key == NULL) || (key->f.f == NULL))
	  rl_beep ();
	else
	  {
	     if ((SLang_Last_Key_Char == SLang_RL_EOF_Char)
		 && (*key->str == 2)
		 && (This_RLI->len == 0))
	       rl_eof_insert ();
	     else if (key->type == SLKEY_F_INTRINSIC)
	       {
		  if ((key->f.f)())
		    RLupdate (rli);

		  if ((rli->flags & SL_RLINE_BLINK_MATCH)
		      && (rli->input_pending != NULL))
		    blink_match (rli);
	       }

	     if (SLang_Rline_Quit)
	       {
		  This_RLI->buf[This_RLI->len] = 0;
		  if (SLang_Error == SL_USER_BREAK)
		    {
		       SLang_Error = 0;
		       return -1;
		    }
		  return This_RLI->len;
	       }
	  }
	if (key != NULL)
	  This_RLI->last_fun = key->f.f;
     }
}

static int rl_abort (void)
{
   rl_delete_line ();
   return rl_enter ();
}

/* TTY interface --- ANSI */

static void ansi_goto_column (int n)
{
   putc('\r', stdout);
   if (n) fprintf(stdout, "\033[%dC", n);
}

static void rl_select_line (SLang_Read_Line_Type *p)
{
   This_RLI->last = p;
   strcpy ((char *) This_RLI->buf, (char *) p->buf);
   This_RLI->point = This_RLI->len = strlen((char *) p->buf);
}
static int rl_next_line (void);
static int rl_prev_line (void)
{
   SLang_Read_Line_Type *prev;

   if (((This_RLI->last_fun != (FVOID_STAR) rl_prev_line)
	&& (This_RLI->last_fun != (FVOID_STAR) rl_next_line))
       || (This_RLI->last == NULL))
     {
	prev = This_RLI->tail;
     }
   else prev = This_RLI->last->prev;

   if (prev == NULL)
     {
	rl_beep ();
	return 0;
     }

   rl_select_line (prev);
   return 1;
}
static int rl_redraw (void)
{
   SLrline_redraw (This_RLI);
   return 1;
}

static int rl_next_line (void)
{
   SLang_Read_Line_Type *next;

   if (((This_RLI->last_fun != (FVOID_STAR) rl_prev_line)
	&& (This_RLI->last_fun != (FVOID_STAR) rl_next_line))
       || (This_RLI->last == NULL))
      {
	 rl_beep ();
	 return 0;
      }

   next = This_RLI->last->next;

   if (next == NULL)
     {
	This_RLI->len = This_RLI->point = 0;
	*This_RLI->buf = 0;
	This_RLI->last = NULL;
     }
   else rl_select_line (next);
   return 1;
}

static SLKeymap_Function_Type SLReadLine_Functions[] =
{
   {"up", rl_prev_line},
   {"down", rl_next_line},
   {"bol", rl_bol},
   {"eol", rl_eol},
   {"right", rl_right},
   {"left", rl_left},
   {"self_insert", rl_self_insert},
   {"bdel", rl_bdel},
   {"del", rl_del},
   {"deleol", rl_deleol},
   {"enter", rl_enter},
   {"trim", rl_trim},
   {"quoted_insert", rl_quote_insert},
   {(char *) NULL, NULL}
};

int SLang_init_readline (SLang_RLine_Info_Type *rli)
{
   int ch;
   char simple[2];

   if (RL_Keymap == NULL)
     {
	simple[1] = 0;
	if (NULL == (RL_Keymap = SLang_create_keymap ("ReadLine", NULL)))
	  return -1;

	RL_Keymap->functions = SLReadLine_Functions;

	/* This breaks under some DEC ALPHA compilers (scary!) */
#ifndef __DECC
	for (ch = ' '; ch < 256; ch++)
	  {
	     simple[0] = (char) ch;
	     SLkm_define_key (simple, (FVOID_STAR) rl_self_insert, RL_Keymap);
	  }
#else
	ch = ' ';
	while (1)
	  {
	     simple[0] = (char) ch;
	     SLkm_define_key (simple, (FVOID_STAR) rl_self_insert, RL_Keymap);
	     ch = ch + 1;
	     if (ch == 256) break;
	  }
#endif				       /* NOT __DECC */

	simple[0] = SLang_Abort_Char;
	SLkm_define_key (simple, (FVOID_STAR) rl_abort, RL_Keymap);
	simple[0] = SLang_RL_EOF_Char;
	SLkm_define_key (simple, (FVOID_STAR) rl_eof_insert, RL_Keymap);

#ifndef IBMPC_SYSTEM
	SLkm_define_key  ("^[[A", (FVOID_STAR) rl_prev_line, RL_Keymap);
	SLkm_define_key  ("^[[B", (FVOID_STAR) rl_next_line, RL_Keymap);
	SLkm_define_key  ("^[[C", (FVOID_STAR) rl_right, RL_Keymap);
	SLkm_define_key  ("^[[D", (FVOID_STAR) rl_left, RL_Keymap);
	SLkm_define_key  ("^[OA", (FVOID_STAR) rl_prev_line, RL_Keymap);
	SLkm_define_key  ("^[OB", (FVOID_STAR) rl_next_line, RL_Keymap);
	SLkm_define_key  ("^[OC", (FVOID_STAR) rl_right, RL_Keymap);
	SLkm_define_key  ("^[OD", (FVOID_STAR) rl_left, RL_Keymap);
#else
	SLkm_define_key  ("^@H", (FVOID_STAR) rl_prev_line, RL_Keymap);
	SLkm_define_key  ("^@P", (FVOID_STAR) rl_next_line, RL_Keymap);
	SLkm_define_key  ("^@M", (FVOID_STAR) rl_right, RL_Keymap);
	SLkm_define_key  ("^@K", (FVOID_STAR) rl_left, RL_Keymap);
	SLkm_define_key  ("^@S", (FVOID_STAR) rl_del, RL_Keymap);
	SLkm_define_key  ("^@O", (FVOID_STAR) rl_eol, RL_Keymap);
	SLkm_define_key  ("^@G", (FVOID_STAR) rl_bol, RL_Keymap);

	SLkm_define_key  ("\xE0H", (FVOID_STAR) rl_prev_line, RL_Keymap);
	SLkm_define_key  ("\xE0P", (FVOID_STAR) rl_next_line, RL_Keymap);
	SLkm_define_key  ("\xE0M", (FVOID_STAR) rl_right, RL_Keymap);
	SLkm_define_key  ("\xE0K", (FVOID_STAR) rl_left, RL_Keymap);
	SLkm_define_key  ("\xE0S", (FVOID_STAR) rl_del, RL_Keymap);
	SLkm_define_key  ("\xE0O", (FVOID_STAR) rl_eol, RL_Keymap);
	SLkm_define_key  ("\xE0G", (FVOID_STAR) rl_bol, RL_Keymap);
#endif
	SLkm_define_key  ("^C", (FVOID_STAR) rl_abort, RL_Keymap);
	SLkm_define_key  ("^E", (FVOID_STAR) rl_eol, RL_Keymap);
	SLkm_define_key  ("^G", (FVOID_STAR) rl_abort, RL_Keymap);
	SLkm_define_key  ("^I", (FVOID_STAR) rl_self_insert, RL_Keymap);
	SLkm_define_key  ("^A", (FVOID_STAR) rl_bol, RL_Keymap);
	SLkm_define_key  ("\r", (FVOID_STAR) rl_enter, RL_Keymap);
	SLkm_define_key  ("\n", (FVOID_STAR) rl_enter, RL_Keymap);
	SLkm_define_key  ("^K", (FVOID_STAR) rl_deleol, RL_Keymap);
	SLkm_define_key  ("^L", (FVOID_STAR) rl_deleol, RL_Keymap);
	SLkm_define_key  ("^V", (FVOID_STAR) rl_del, RL_Keymap);
	SLkm_define_key  ("^D", (FVOID_STAR) rl_del, RL_Keymap);
	SLkm_define_key  ("^F", (FVOID_STAR) rl_right, RL_Keymap);
	SLkm_define_key  ("^B", (FVOID_STAR) rl_left, RL_Keymap);
	SLkm_define_key  ("^?", (FVOID_STAR) rl_bdel, RL_Keymap);
	SLkm_define_key  ("^H", (FVOID_STAR) rl_bdel, RL_Keymap);
	SLkm_define_key  ("^P", (FVOID_STAR) rl_prev_line, RL_Keymap);
	SLkm_define_key  ("^N", (FVOID_STAR) rl_next_line, RL_Keymap);
	SLkm_define_key  ("^R", (FVOID_STAR) rl_redraw, RL_Keymap);
	SLkm_define_key  ("`", (FVOID_STAR) rl_quote_insert, RL_Keymap);
	SLkm_define_key  ("\033\\", (FVOID_STAR) rl_trim, RL_Keymap);
	if (SLang_Error) return -1;
     }

   if (rli->prompt == NULL) rli->prompt = "";
   if (rli->keymap == NULL) rli->keymap = RL_Keymap;
   rli->old_upd = rli->upd_buf1;
   rli->new_upd = rli->upd_buf2;
   *rli->buf = 0;
   rli->point = 0;

   if (rli->flags & SL_RLINE_USE_ANSI)
     {
	if (rli->tt_goto_column == NULL) rli->tt_goto_column = ansi_goto_column;
     }

   if (Char_Widths[0] == 2) return 0;

   for (ch = 0; ch < 32; ch++) Char_Widths[ch] = 2;
   for (ch = 32; ch < 256; ch++) Char_Widths[ch] = 1;
   Char_Widths[127] = 2;
#ifndef IBMPC_SYSTEM
   for (ch = 128; ch < 160; ch++) Char_Widths[ch] = 3;
#endif

   return 0;
}

SLang_Read_Line_Type *SLang_rline_save_line (SLang_RLine_Info_Type *rli)
{
   SLang_Read_Line_Type *rl;
   unsigned char *buf;

   if ((rli == NULL) || (rli->buf == NULL))
     return NULL;

   if (NULL == (rl = (SLang_Read_Line_Type *) SLmalloc (sizeof (SLang_Read_Line_Type)))
       || (NULL == (buf = (unsigned char *) SLmake_string ((char *)rli->buf))))
     {
	SLfree ((char *)rl);
	return NULL;
     }
   rl->buf = buf;
   rl->buf_len = strlen ((char *)buf);
   rl->num = rl->misc = 0;
   rl->next = rl->prev = NULL;

   if (rli->tail != NULL)
     {
	rli->tail->next = rl;
	rl->prev = rli->tail;
     }
   rli->tail = rl;

   return rl;
}
