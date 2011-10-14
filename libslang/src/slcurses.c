/* Copyright (c) 1998, 1999, 2001, 2002, 2003 John E. Davis
 * This file is part of the S-Lang library.
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Perl Artistic License.
 */
#include "slinclud.h"

#include <signal.h>
#include <errno.h>

#include "slang.h"
#include "_slang.h"
#include "slcurses.h"

/* This file is meant to implement a primitive curses implementation in
 * terms of SLsmg calls.  The fact is that the interfaces are sufficiently
 * different that a 100% emulation is not possible.
 */

SLcurses_Window_Type *SLcurses_Stdscr;
int SLcurses_Esc_Delay = 150;	       /* 0.15 seconds */
SLtt_Char_Type SLcurses_Acs_Map [128];
int SLcurses_Is_Endwin = 1;
int SLcurses_Num_Colors = 8;

static void blank_line (SLsmg_Char_Type *b, unsigned int len, SLsmg_Char_Type color)
{
   SLsmg_Char_Type *bmax;

   bmax = b + len;
   color = SLSMG_BUILD_CHAR(' ', color);

   while (b < bmax) *b++ = color;
}

static int va_mvprintw (SLcurses_Window_Type *w, int r, int c, int do_move,
			char *fmt, va_list ap)
{
   char buf[1024];

   if (do_move) SLcurses_wmove (w, r, c);

   (void) _SLvsnprintf (buf, sizeof(buf), fmt, ap);

   SLcurses_waddnstr (w, buf, -1);
   return 0;
}

int SLcurses_mvprintw (int r, int c, char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   va_mvprintw (SLcurses_Stdscr, r, c, 1, fmt, ap);
   va_end(ap);

   return 0;
}

int SLcurses_mvwprintw (SLcurses_Window_Type *w, int r, int c, char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   va_mvprintw (w, r, c, 1, fmt, ap);
   va_end(ap);

   return 0;
}

int SLcurses_wprintw (SLcurses_Window_Type *w, char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   va_mvprintw (w, 0, 0, 0, fmt, ap);
   va_end(ap);

   return 0;
}

int SLcurses_printw (char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   va_mvprintw (SLcurses_Stdscr, 0, 0, 0, fmt, ap);
   va_end(ap);

   return 0;
}

int SLcurses_nil (void)
{
   return 0;
}

int SLcurses_has_colors(void)
{
   return SLtt_Use_Ansi_Colors;
}

int SLcurses_nodelay (SLcurses_Window_Type *w, int onoff)
{
   w->delay_off = (onoff ? 0 : -1);
   return 0;
}

static unsigned char Keyboard_Buffer[256];
static unsigned char *Keyboard_Buffer_Start = Keyboard_Buffer;
static unsigned char *Keyboard_Buffer_Stop = Keyboard_Buffer;

static int getkey_function (void)
{
   int ch;

   ch = SLang_getkey ();
   if (ch != SLANG_GETKEY_ERROR)
     {
	*Keyboard_Buffer_Stop++ = (unsigned char) ch;
	if (Keyboard_Buffer_Stop == Keyboard_Buffer + sizeof (Keyboard_Buffer))
	  Keyboard_Buffer_Stop = Keyboard_Buffer;
     }
   
   return ch;
}

static int get_buffered_key (void)
{
   int ch;
   
   if (Keyboard_Buffer_Stop == Keyboard_Buffer_Start)
     return SLANG_GETKEY_ERROR;

   ch = *Keyboard_Buffer_Start++;
   if (Keyboard_Buffer_Start == Keyboard_Buffer + sizeof (Keyboard_Buffer))
     Keyboard_Buffer_Start = Keyboard_Buffer;

   return ch;
}

static int get_keypad_key (void)
{
   int ch;
   
   if (Keyboard_Buffer_Stop != Keyboard_Buffer_Start)
     return get_buffered_key ();

   ch = SLang_getkey ();
   if (ch == '\033')
     {
	if (0 == SLang_input_pending (ESCDELAY / 100))
	  return ch;
     }
   else if (ch == SLANG_GETKEY_ERROR) return ERR;
   SLang_ungetkey (ch);
   ch = SLkp_getkey ();
   if (ch == SL_KEY_ERR)
     ch = get_buffered_key ();
   else
     Keyboard_Buffer_Stop = Keyboard_Buffer_Start;
   return ch;
}

int SLcurses_wgetch (SLcurses_Window_Type *w)
{
   if (w == NULL)
     return ERR;

   SLcurses_wrefresh (w);

   if ((Keyboard_Buffer_Start != Keyboard_Buffer_Stop)
       || (w->delay_off == -1)
       || SLang_input_pending (w->delay_off))
     {
	if (w->use_keypad)
	  return get_keypad_key ();

	return SLang_getkey ();
     }

   return ERR;
}

int SLcurses_getch (void)
{
   return SLcurses_wgetch (SLcurses_Stdscr);
}

/* This is a super hack.  That fact is that SLsmg and curses
 * are incompatible.
 */
static unsigned char Color_Objects[256];

static unsigned int map_attr_to_object (SLtt_Char_Type attr)
{
   unsigned int obj;
   SLtt_Char_Type at;

   obj = (attr >> 8) & 0xFF;

   if (SLtt_Use_Ansi_Colors)
     {
	if (Color_Objects[obj] != 0) return obj;

	at = SLtt_get_color_object (obj & 0xF);

	if (attr & A_BOLD) at |= SLTT_BOLD_MASK;
	if (attr & A_UNDERLINE) at |= SLTT_ULINE_MASK;
	if (attr & A_REVERSE) at |= SLTT_REV_MASK;

	SLtt_set_color_object (obj, at);

	Color_Objects[obj] = 1;
     }
   else obj = obj & 0xF0;

   return obj;

}

int SLcurses_start_color (void)
{
   int f, b;
   int obj;

   if (SLtt_Use_Ansi_Colors == 0) return -1;

   obj = 0;
   for (f = 0; f < 16; f++)
     {
	for (b = 0; b < 16; b++)
	  {
	     obj++;
	     SLtt_set_color_fgbg (obj, f, b);
	  }
     }
   return 0;
}

#ifdef SIGINT
static void sigint_handler (int sig)
{
   SLang_reset_tty ();
   SLsmg_reset_smg ();
   exit (sig);
}
#endif

/* Values are assumed to be 0, 1, 2.  This fact is exploited */
static int TTY_State;

static int init_tty (int suspend_ok)
{
   if (-1 == SLang_init_tty (-1, 1, 0))
     return -1;

#ifdef REAL_UNIX_SYSTEM
   if (suspend_ok) SLtty_set_suspend_state (1);
#endif
   return 0;
}

int SLcurses_raw (void)
{
   TTY_State = 1;
   return init_tty (0);
}

int SLcurses_cbreak (void)
{
   TTY_State = 2;
   return init_tty (1);
}

#if defined(SIGTSTP) && defined(SIGSTOP)
static void sigtstp_handler (int sig)
{
   sig = errno;

   SLsmg_suspend_smg ();

   if (TTY_State)
     SLang_reset_tty ();

   kill(getpid(),SIGSTOP);

   SLsmg_resume_smg ();

   if (TTY_State) init_tty (TTY_State - 1);

   signal (SIGTSTP, sigtstp_handler);
   errno = sig;
}
#endif

SLcurses_Window_Type *SLcurses_initscr (void)
{
   SLcurses_Is_Endwin = 0;
   SLsmg_Newline_Behavior = SLSMG_NEWLINE_MOVES;
   SLtt_get_terminfo ();

#if !defined(IBMPC_SYSTEM) && !defined(VMS)
   if (-1 == (SLcurses_Num_Colors = SLtt_tgetnum ("Co")))
#endif
     SLcurses_Num_Colors = 8;

   if ((-1 == SLkp_init ())
       || (-1 == SLcurses_cbreak ())
       || (NULL == (SLcurses_Stdscr = SLcurses_newwin (0, 0, 0, 0)))
       || (-1 == SLsmg_init_smg ()))
     {
	SLang_doerror (NULL);
	SLang_exit_error ("SLcurses_initscr: init failed\n");
	return NULL;
     }
   SLkp_set_getkey_function (getkey_function);

#ifdef SIGINT
   signal (SIGINT, sigint_handler);
#endif

#if defined(SIGTSTP) && defined(SIGSTOP)
   signal (SIGTSTP, sigtstp_handler);
#endif

   SLtt_set_mono (A_BOLD >> 8, NULL, SLTT_BOLD_MASK);
   SLtt_set_mono (A_UNDERLINE >> 8, NULL, SLTT_ULINE_MASK);
   SLtt_set_mono (A_REVERSE >> 8, NULL, SLTT_REV_MASK);
   /* SLtt_set_mono (A_BLINK >> 8, NULL, SLTT_BLINK_MASK); */
   SLtt_set_mono ((A_BOLD|A_UNDERLINE) >> 8, NULL, SLTT_ULINE_MASK|SLTT_BOLD_MASK);
   SLtt_set_mono ((A_REVERSE|A_UNDERLINE) >> 8, NULL, SLTT_ULINE_MASK|SLTT_REV_MASK);

   if (SLtt_Has_Alt_Charset)
     {
       SLcurses_Acs_Map[SLSMG_ULCORN_CHAR] = SLSMG_ULCORN_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_URCORN_CHAR] = SLSMG_URCORN_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_LLCORN_CHAR] = SLSMG_LLCORN_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_LRCORN_CHAR] = SLSMG_LRCORN_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_UTEE_CHAR] = SLSMG_UTEE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_DTEE_CHAR] = SLSMG_DTEE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_LTEE_CHAR] = SLSMG_LTEE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_RTEE_CHAR] = SLSMG_RTEE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_VLINE_CHAR] = SLSMG_VLINE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_HLINE_CHAR] = SLSMG_HLINE_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_PLUS_CHAR] = SLSMG_PLUS_CHAR | A_ALTCHARSET;
       SLcurses_Acs_Map[SLSMG_CKBRD_CHAR] = SLSMG_CKBRD_CHAR | A_ALTCHARSET;
     }
   else
     {
       /* ugly defaults to use on terminals which don't support graphics */
       SLcurses_Acs_Map[SLSMG_ULCORN_CHAR] = '+';
       SLcurses_Acs_Map[SLSMG_URCORN_CHAR] = '+';
       SLcurses_Acs_Map[SLSMG_LLCORN_CHAR] = '+';
       SLcurses_Acs_Map[SLSMG_LRCORN_CHAR] = '+';
       SLcurses_Acs_Map[SLSMG_UTEE_CHAR] = '+';
       SLcurses_Acs_Map[SLSMG_DTEE_CHAR] = '+';
       SLcurses_Acs_Map[SLSMG_LTEE_CHAR] = '+';
       SLcurses_Acs_Map[SLSMG_RTEE_CHAR] = '+';
       SLcurses_Acs_Map[SLSMG_VLINE_CHAR] = '|';
       SLcurses_Acs_Map[SLSMG_HLINE_CHAR] = '-';
       SLcurses_Acs_Map[SLSMG_PLUS_CHAR] = '+';
       SLcurses_Acs_Map[SLSMG_CKBRD_CHAR] = '#';
     }

   return SLcurses_Stdscr;
}

int SLcurses_wattrset (SLcurses_Window_Type *w, SLtt_Char_Type ch)
{
   unsigned int obj;

   obj = map_attr_to_object (ch);
   w->color = obj;
   w->attr = ch;
   return 0;
}

int SLcurses_wattroff (SLcurses_Window_Type *w, SLtt_Char_Type ch)
{
   if (SLtt_Use_Ansi_Colors)
     return SLcurses_wattrset (w, 0);

   w->attr &= ~ch;
   return SLcurses_wattrset (w, w->attr);
}

int SLcurses_wattron (SLcurses_Window_Type *w, SLtt_Char_Type ch)
{
   if (SLtt_Use_Ansi_Colors)
     return SLcurses_wattrset (w, ch);

   w->attr |= ch;
   return SLcurses_wattrset (w, w->attr);
}

int SLcurses_delwin (SLcurses_Window_Type *w)
{
   if (w == NULL) return 0;
   if (w->lines != NULL)
     {
	SLsmg_Char_Type **lines = w->lines;
	if (w->is_subwin == 0)
	  {
	     unsigned int r, rmax;

	     rmax = w->nrows;
	     for (r = 0; r < rmax; r++)
	       {
		  SLfree ((char *)lines[r]);
	       }
	  }

	SLfree ((char *)lines);
     }

   SLfree ((char *)w);
   if (w == SLcurses_Stdscr)
     SLcurses_Stdscr = NULL;
   return 0;
}

SLcurses_Window_Type *SLcurses_newwin (unsigned int nrows, unsigned int ncols,
				       unsigned int r, unsigned int c)
{
   SLcurses_Window_Type *win;
   SLsmg_Char_Type **lines;

   if (r >= (unsigned int) SLtt_Screen_Rows)
     return NULL;
   if (c >= (unsigned int) SLtt_Screen_Cols)
     return NULL;

   if (NULL == (win = (SLcurses_Window_Type *) SLmalloc (sizeof (SLcurses_Window_Type))))
     return NULL;

   SLMEMSET ((char *) win, 0, sizeof (SLcurses_Window_Type));

   if (nrows == 0)
     nrows = (unsigned int) SLtt_Screen_Rows - r;
   if (ncols == 0)
     ncols = (unsigned int) SLtt_Screen_Cols - c;

   lines = (SLsmg_Char_Type **) SLmalloc (nrows * sizeof (SLsmg_Char_Type *));
   if (lines == NULL)
     {
	SLcurses_delwin (win);
	return NULL;
     }

   SLMEMSET ((char *) lines, 0, nrows * sizeof (SLsmg_Char_Type *));

   win->lines = lines;
   win->scroll_max = win->nrows = nrows;
   win->ncols = ncols;
   win->_begy = r;
   win->_begx = c;
   win->_maxx = (c + ncols) - 1;
   win->_maxy = (r + nrows) - 1;
   win->modified = 1;
   win->delay_off = -1;

   for (r = 0; r < nrows; r++)
     {
	SLsmg_Char_Type *b;

	b = (SLsmg_Char_Type *) SLmalloc (ncols * sizeof (SLsmg_Char_Type));
	if (b == NULL)
	  {
	     SLcurses_delwin (win);
	     return NULL;
	  }
	lines [r] = b;
	blank_line (b, ncols, 0);
     }

   return win;
}

int SLcurses_wmove (SLcurses_Window_Type *win, unsigned int r, unsigned int c)
{
   if (win == NULL) return -1;
   win->_cury = r;
   win->_curx = c;
   win->modified = 1;
   return 0;
}

static int do_newline (SLcurses_Window_Type *w)
{
   w->_curx = 0;
   w->_cury += 1;
   if (w->_cury >= w->scroll_max)
     {
	w->_cury = w->scroll_max - 1;
	if (w->scroll_ok)
	  SLcurses_wscrl (w, 1);
     }

   return 0;
}

int SLcurses_waddch (SLcurses_Window_Type *win, SLtt_Char_Type attr)
{
   SLsmg_Char_Type *b, ch;
   SLsmg_Char_Type color;

   if (win == NULL) return -1;

   if (win->_cury >= win->nrows)
     {
	/* Curses seems to move current postion to top of window. */
	win->_cury = win->_curx = 0;
	return -1;
     }

   win->modified = 1;

   ch = SLSMG_EXTRACT_CHAR(attr);

   if (attr == ch)
     color = win->color;
   else
     {
	/* hack to pick up the default color for graphics chars */
	if (((attr & A_COLOR) == 0) && ((attr & A_ALTCHARSET) != 0))
	  {
	     /* FIXME: priority=medium: Use SLSMG_?? instead of << */
	     attr |= win->color << 8;
	  }
	color = map_attr_to_object (attr);
     }

   if (ch < ' ')
     {
	if (ch == '\n')
	  {
	     SLcurses_wclrtoeol (win);
	     return do_newline (win);
	  }

	if (ch == '\r')
	  {
	     win->_curx = 0;
	     return 0;
	  }

	if (ch == '\b')
	  {
	     if (win->_curx > 0)
	       win->_curx--;

	     return 0;
	  }

	/* HACK HACK!!!! */
	if (ch == '\t') ch = ' ';
     }

   if (win->_curx >= win->ncols)
     do_newline (win);

   b = win->lines[win->_cury] + win->_curx;
   *b = SLSMG_BUILD_CHAR(ch,color);
   win->_curx++;

   return 0;
}

int SLcurses_wnoutrefresh (SLcurses_Window_Type *w)
{
   unsigned int len;
   unsigned int r, c;
   unsigned int i, imax;

   if (SLcurses_Is_Endwin)
     {
	if (TTY_State) init_tty (TTY_State - 1);
       	SLsmg_resume_smg ();
	SLcurses_Is_Endwin = 0;
     }

   if (w == NULL)
     {
	SLsmg_refresh ();
	return -1;
     }

   if (w->modified == 0)
     return 0;

   r = w->_begy;
   c = w->_begx;

   len = w->ncols;
   imax = w->nrows;

   for (i = 0; i < imax; i++)
     {
	SLsmg_gotorc (r, c);
	SLsmg_write_color_chars (w->lines[i], len);
	r++;
     }

   if (w->has_box)
     SLsmg_draw_box(w->_begy, w->_begx, w->nrows, w->ncols);

   SLsmg_gotorc (w->_begy + w->_cury, w->_begx + w->_curx);
   w->modified = 0;
   return 0;
}

int SLcurses_wrefresh (SLcurses_Window_Type *w)
{
   if (w == NULL)
     return -1;

   if (w->modified == 0)
     return 0;

   SLcurses_wnoutrefresh (w);
   SLsmg_refresh ();
   return 0;
}

int SLcurses_wclrtoeol (SLcurses_Window_Type *w)
{
   SLsmg_Char_Type *b, *bmax;
   SLsmg_Char_Type blank;

   if (w == NULL) return -1;
   if (w->_cury >= w->nrows)
     return 0;

   w->modified = 1;

   blank = SLSMG_BUILD_CHAR(' ',w->color);

   b = w->lines[w->_cury];
   bmax = b + w->ncols;
   b += w->_curx;

   while (b < bmax) *b++ = blank;
   return 0;
}

int SLcurses_wclrtobot (SLcurses_Window_Type *w)
{
   SLsmg_Char_Type *b, *bmax;
   SLsmg_Char_Type blank;
   unsigned int r;

   if (w == NULL) return -1;

   w->modified = 1;
   blank = SLSMG_BUILD_CHAR(' ',w->color);
   SLcurses_wclrtoeol (w);
   for (r = w->_cury + 1; r < w->nrows; r++)
     {
	b = w->lines [r];
	bmax = b + w->ncols;

	while (b < bmax) *b++ = blank;
     }

   return 0;
}

int SLcurses_wscrl (SLcurses_Window_Type *w, int n)
{
   SLsmg_Char_Type **lines;
   unsigned int r, rmax, rmin, ncols;
   SLsmg_Char_Type color;

   if ((w == NULL) || (w->scroll_ok == 0))
     return -1;

   w->modified = 1;
#if 0
   if (w->is_subwin)
     {
	SLang_reset_tty ();
	SLsmg_reset_smg ();
	fprintf (stderr, "\rAttempt to scroll a subwindow\n");
	exit (1);
     }
#endif

   color = w->color;
   ncols = w->ncols;
   lines = w->lines;
   rmax = w->scroll_max;
   rmin = w->scroll_min;
   if (rmax > w->nrows)
     rmax = w->nrows;
   if (rmin >= rmax)
     return 0;

   while (n > 0)
     {
	for (r = rmin + 1; r < rmax; r++)
	  {
	     /* lines[r - 1] = lines[r]; */
	     memcpy ((char *)lines[r - 1], (char *)lines[r],
		     sizeof (SLsmg_Char_Type) * ncols);
	  }
	blank_line (lines[rmax - 1], ncols, color);
	n--;
     }

   rmax--;
   while (n < 0)
     {
	for (r = rmax; r > rmin; r--)
	  {
	     memcpy ((char *)lines[r], (char *)lines[r - 1],
		     sizeof (SLsmg_Char_Type) * ncols);
	  }
	blank_line (lines[rmin], ncols, color);
	n++;
     }

   /* wmove (w, w->nrows - 1, 0); */
   /* wclrtobot (w); */
   return 0;
}

/* Note: if len is < 0, entire string will be used.
 */
int SLcurses_waddnstr (SLcurses_Window_Type *w, char *str, int len)
{
   SLsmg_Char_Type *b;
   SLsmg_Char_Type color;
   unsigned char ch;
   unsigned int nrows, ncols, crow, ccol;

   if ((w == NULL)
       || (str == NULL))
     return -1;

   w->modified = 1;
   nrows = w->nrows;
   ncols = w->ncols;
   crow = w->_cury;
   ccol = w->_curx;
   color = w->color;

   if (w->scroll_max <= nrows)
     nrows = w->scroll_max;

   if (crow >= nrows)
     crow = 0;			       /* wrap back to top */

   b = w->lines [crow] + ccol;

   while (len && ((ch = (unsigned char) *str++) != 0))
     {
	len--;

	if (ch == '\n')
	  {
	     w->_cury = crow;
	     w->_curx = ccol;
	     SLcurses_wclrtoeol (w);
	     do_newline (w);
	     crow = w->_cury;
	     ccol = w->_curx;
	     b = w->lines[crow];
	     continue;
	  }

	if (ccol >= ncols)
	  {
	     ccol = 0;
	     crow++;
	     if (crow >= nrows)
	       {
		  w->_curx = 0;
		  w->_cury = crow;
		  do_newline (w);
		  crow = w->_cury;
		  ccol = w->_curx;
	       }

	     b = w->lines [crow];
	  }

	if (ch == '\t')
	  {
	     unsigned int n = ccol;
	     n += SLsmg_Tab_Width;
	     n = SLsmg_Tab_Width - (n % SLsmg_Tab_Width);
	     if (ccol + n > ncols) n = ncols - len;
	     ccol += n;
	     while (n--)
	       *b++ = SLSMG_BUILD_CHAR(' ',color);
	     continue;
	  }

	*b++ = SLSMG_BUILD_CHAR(ch, color);
	ccol++;
     }

   w->_curx = ccol;
   w->_cury = crow;

   return 0;
}

/* This routine IS NOT CORRECT.  It needs to compute the proper overlap
 * and copy accordingly.  Here, I just assume windows are same size.
 */
#if 0
int SLcurses_overlay (SLcurses_Window_Type *swin, SLcurses_Window_Type *dwin)
{
   SLsmg_Char_Type *s, *smax, *d, *dmax;

   if ((swin == NULL) || (dwin == NULL))
     return -1;

   s = swin->buf;
   smax = swin->bufmax;
   d = dwin->buf;
   dmax = dwin->bufmax;

   while ((s < smax) && (d < dmax))
     {
	SLsmg_Char_Type ch = *s++;
	if (SLSMG_EXTRACT_CHAR(ch) != ' ')
	  *d = ch;
	d++;
     }

   return -1;			       /* not implemented */
}

#endif

SLcurses_Window_Type *SLcurses_subwin (SLcurses_Window_Type *orig,
				       unsigned int nlines, unsigned int ncols,
				       unsigned int begin_y, unsigned int begin_x)
{
   SLcurses_Window_Type *sw;
   int r, c;
   unsigned int i;

   if (orig == NULL)
     return NULL;

   sw = (SLcurses_Window_Type *) SLmalloc (sizeof (SLcurses_Window_Type));
   if (sw == NULL)
     return NULL;

   SLMEMSET ((char *)sw, 0, sizeof (SLcurses_Window_Type));
#if 1
   r = begin_y - orig->_begy;
#else
   r = 1 + ((int)orig->nrows - (int)nlines) / 2;
#endif
   if (r < 0) r = 0;
   if (r + nlines > orig->nrows) nlines = orig->nrows - r;

   c = ((int)orig->ncols - (int)ncols) / 2;
   if (c < 0) c = 0;
   if (c + ncols > orig->ncols) ncols = orig->ncols - c;

   sw->scroll_min = 0;
   sw->scroll_max = sw->nrows = nlines;
   sw->ncols = ncols;
   sw->_begy = begin_y;
   sw->_begx = begin_x;
   sw->_maxx = (begin_x + ncols) - 1;
   sw->_maxy = (begin_y + nlines) - 1;

   sw->lines = (SLsmg_Char_Type **) SLmalloc (nlines * sizeof (SLsmg_Char_Type *));
   if (sw->lines == NULL)
     {
	SLcurses_delwin (sw);
	return NULL;
     }

   for (i = 0; i < nlines; i++)
     {
	sw->lines [i] = orig->lines [r + i] + c;
     }

   sw->is_subwin = 1;
   return sw;
}

int SLcurses_wclear (SLcurses_Window_Type *w)
{
   unsigned int i;

   if (w != NULL) w->modified = 1;
   for (i=0; i < w->nrows; i++)
     blank_line (w->lines[i], w->ncols, w->color);
   return 0;
}

int SLcurses_wdelch (SLcurses_Window_Type *w)
{
   SLsmg_Char_Type *p, *p1, *pmax;

   p = w->lines[w->_cury];
   pmax = p + w->ncols;
   p += w->_curx;
   p1 = p + 1;

   while (p1 < pmax)
     {
	*p = *p1;
	p = p1;
	p1++;
     }

   if (p < pmax)
     *p = SLSMG_BUILD_CHAR(' ',w->color);

   w->modified = 1;
   return 0;
}

int SLcurses_winsch (SLcurses_Window_Type *w, int ch)
{
   SLsmg_Char_Type *p, *p1, *pmax;

   p = w->lines[w->_cury];
   pmax = p + w->ncols;
   p += w->_curx;
   p1 = pmax - 1;

   while (pmax > p)
     {
	*pmax = *p1;
	pmax = p1;
	p1--;
     }

   if (p < pmax)
     *p = SLSMG_BUILD_CHAR(ch, w->color);

   w->modified = 1;
   return 0;
}

int SLcurses_endwin (void)
{
   SLcurses_Is_Endwin = 1;
   SLsmg_suspend_smg ();
   SLang_reset_tty ();
   return 0;
}

#if 0
int SLcurses_mvwscanw (SLcurses_Window_Type *w, unsigned int r, unsigned int c,
		       char *fmt, ...)
{
#if HAVE_VFSCANF
   int ret;
   va_list ap;

   SLcurses_wmove (w, r, c);
   SLcurses_wrefresh (w);

   va_start(ap, fmt);
   ret = vfscanf (stdin, fmt, ap);
   va_end(ap);
   return ret;
#else
   return 0;
#endif
}

int SLcurses_wscanw (SLcurses_Window_Type *w, char *fmt, ...)
{
#if HAVE_VFSCANF
  va_list ap;
   int ret;

   SLcurses_wrefresh (w);

   va_start(ap, fmt);
   ret = vfscanf (stdin, fmt, ap);
   va_end(ap);

   return ret;
#else
   return 0;
#endif
}

int SLcurses_scanw (char *fmt, ...)
{
#ifdef HAVE_VFSCANF
   va_list ap;
   int ret;

   SLcurses_wrefresh (SLcurses_Stdscr);

   va_start(ap, fmt);
   ret = vfscanf (stdin, fmt, ap);
   va_end(ap);

   return ret;
#else
   return 0;
#endif
}
#endif

int SLcurses_clearok (SLcurses_Window_Type *w, int bf)
{
   if (bf)
     {
	SLsmg_cls ();
	w->modified = 1;
     }
   return 0;
}
