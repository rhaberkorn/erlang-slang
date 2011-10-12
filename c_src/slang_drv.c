#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#include <slang.h>
#include <erl_driver.h>

#if (SLANG_VERSION < 10400 )
#define SLsmg_Char_Type unsigned short
#endif



/* Standard set of integer macros  .. */

#define get_int32(s) ntohl(*(uint32_t *)(s))

#define put_int32(i, s) do {				\
	*(uint32_t *)(s) = htonl((uint32_t)(i));	\
} while (0)

#define get_int16(s) ntohs(*(uint16_t *)(s))

#define put_int16(i, s) do {				\
	*(uint16_t *)(s) = htons((uint16_t)(i));	\
} while (0)

#define get_int8(s) (*(uint8_t *)(s))

#define put_int8(i, s) do {				\
	*(uint8_t *)(s) = (uint8_t)(i);			\
} while (0)



#define INIT_TTY           1
#define SET_ABORT_FUNCTION 2
#define GETKEY             3
#define RESET_TTY          4
#define KP_GETKEY          5
#define UNGETKEY           6
#define SETVAR             7
#define GETVAR             8
#define KP_INIT            9

/* screen mgmt  */

#define SMG_FILL_REGION       10
#define SMG_SET_CHAR_SET      11
#define SMG_SUSPEND_SMG       12
#define SMG_RESUME_SMG        13
#define SMG_ERASE_EOL         14
#define SMG_GOTORC            15
#define SMG_ERASE_EOS         16
#define SMG_REVERSE_VIDEO     17
#define SMG_SET_COLOR         18
#define SMG_NORMAL_VIDEO      19
#define SMG_PRINTF            20
#define SMG_VPRINTF           21
#define SMG_WRITE_STRING      22
#define SMG_WRITE_NSTRING     23
#define SMG_WRITE_CHAR        24
#define SMG_WRITE_NCHARS      25
#define SMG_WRITE_WRAPPED_STRING 26
#define SMG_CLS               27
#define SMG_REFRESH           28
#define SMG_TOUCH_LINES       29
#define SMG_TOUCH_SCREEN      30
#define SMG_INIT_SMG          31
#define SMG_REINIT_SMG        32
#define SMG_RESET_SMG         33
#define SMG_CHAR_AT            34
#define SMG_SET_SCREEN_START  35
#define SMG_DRAW_HLINE        36
#define SMG_DRAW_VLINE        37
#define SMG_DRAW_OBJECT       38
#define SMG_DRAW_BOX          39
#define SMG_GET_COLUMN        40
#define SMG_GET_ROW           41
#define SMG_FORWARD           42
#define SMG_WRITE_COLOR_CHARS 43
#define SMG_READ_RAW          44
#define SMG_WRITE_RAW         45
#define SMG_SET_COLOR_IN_REGION 46




/* ops for all the tt_ functions */

#define TT_FLUSH_OUTPUT        50
#define TT_SET_SCROLL_REGION   51
#define TT_RESET_SCROLL_REGION 52
#define TT_REVERSE_VIDEO       53
#define TT_BOLD_VIDEO          54
#define TT_BEGIN_INSERT        55
#define TT_END_INSERT          56
#define TT_DEL_EOL             57
#define TT_GOTO_RC             58
#define TT_DELETE_NLINES       59
#define TT_DELETE_CHAR         60
#define TT_ERASE_LINE          61
#define TT_NORMAL_VIDEO        62
#define TT_CLS                 63
#define TT_BEEP                64
#define TT_REVERSE_INDEX       65
#define TT_SMART_PUTS          66
#define TT_WRITE_STRING        67
#define TT_PUTCHAR             68
#define TT_INIT_VIDEO          69
#define TT_RESET_VIDEO         70
#define TT_GET_TERMINFO        71
#define TT_GET_SCREEN_SIZE     72
#define TT_SET_CURSOR_VISIBILITY 73
#define TT_SET_MOUSE_MODE      74

#define TT_INITIALIZE          75
#define TT_ENABLE_CURSOR_KEYS  76
#define TT_SET_TERM_VTXXX      77
#define TT_SET_COLOR_ESC       78
#define TT_WIDE_WIDTH          79
#define TT_NARROW_WIDTH        80
#define TT_SET_ALT_CHAR_SET    81
#define TT_WRITE_TO_STATUS_LINE 82
#define TT_DISABLE_STATUS_LINE  83


#define TT_TGETSTR             84
#define TT_TGETNUM             85
#define TT_TGETFLAG            86
#define TT_TIGETENT            87
#define TT_TIGETSTR            88
#define TT_TIGETNUM            89

#define SLTT_GET_COLOR_OBJECT  90
#define TT_SET_COLOR_OBJECT    91
#define TT_SET_COLOR           92
#define TT_SET_MONO            93
#define TT_ADD_COLOR_ATTRIBUTE 94
#define TT_SET_COLOR_FGBG      95


/* aux tty functions */
#define ISATTY                 100
#define EFORMAT                101
#define SIGNAL                 102
#define SIGNAL_CHECK           103




/*  read/write global variables */
#define esl_baud_rate         1
#define esl_read_fd           2
#define esl_abort_char        3
#define esl_ignore_user_abort 4
#define esl_input_buffer_len  5
#define esl_keyboard_quit     6
#define esl_last_key_char     7
#define esl_rl_eof_char       8
#define esl_rline_quit        9
#define esl_screen_rows       10
#define esl_screen_cols       11
#define esl_tab_width         12
#define esl_newline_behaviour 13
#define esl_error             14
#define esl_version           15
#define esl_backspace_moves   16
#define esl_display_eight_bit 17

/* signals */
#define SL_SIGINT    1
#define SL_SIGTSTP   2
#define SL_SIGQUIT   3
#define SL_SIGTTOU   4
#define SL_SIGTTIN   5
#define SL_SIGWINCH  6





static ErlDrvEntry sl_erl_drv_entry;


static int wait_for = 0;
static int signal_cought = 0;



static int sig_to_x(int x)
{
    switch (x ) {
    case SIGINT:    return SL_SIGINT;
    case SIGTSTP:  return SL_SIGTSTP;
    case SIGQUIT: return SL_SIGQUIT;
    case SIGTTOU: return SL_SIGTTOU;
    case SIGTTIN: return SL_SIGTTIN;
    case SIGWINCH: return SL_SIGWINCH;
    default: return -1;
    }
}


static void sig_handler(int sig)
{
    signal_cought = sig_to_x(sig);
}



static int x_to_sig(int x)
{
    switch (x ) {
    case SL_SIGINT:    return SIGINT;
    case SL_SIGTSTP:  return SIGTSTP;
    case SL_SIGQUIT: return SIGQUIT;
    case SL_SIGTTOU: return SIGTTOU;
    case SL_SIGTTIN: return SIGTTIN;
    case SL_SIGWINCH: return SIGWINCH;
    default: return -1;
    }
}


SLsmg_Char_Type *decode_smg_char_type(char **buf)
{
    static SLsmg_Char_Type mbuf[256];
    int i;
    int len = get_int32(*buf); *buf+=4;
    for(i=0; i<len; i++) {
	mbuf[i++] =  get_int16(*buf); *buf+=2;
    }
    return mbuf;
}


static ErlDrvData sl_start(ErlDrvPort port, char *buf)
{
    return (ErlDrvData)port;
}


static void sl_stop(ErlDrvData port)
{
    return;
}

static int ret_int_int(ErlDrvPort port, int i, int j)
{
    char buf[9];
    buf[0] = 1;
    put_int32(i, buf+1);
    put_int32(j, buf+5);
    driver_output(port, buf, 9);
    return i;
}


static int ret_int(ErlDrvPort port, int ret)
{
    char buf[5];
    buf[0] = 1;
    put_int32(ret, buf+1);
    driver_output(port, buf, 5);
    return ret;
}


static int ret_string(ErlDrvPort port, char *str)
{
    str[-1] = 1;
    driver_output(port, str, 1+strlen(str));
    return 1;
}


static void sl_output(ErlDrvData drv_data, char *buf, int len)
{
    ErlDrvPort port = (ErlDrvPort)drv_data;
    int x,y,z,v,w;
    char *str, *t1, *t2, *t3;
    int ret;
    char ch;


    /* check for signals */

    if (signal_cought != 0) {  /* async out signal */
	char xxx[5];

	xxx[0] = 0;
	put_int32(signal_cought, xxx+1);
	driver_output(port, xxx, 5);
	signal_cought = 0;
    }

    switch (*buf++) {
    case INIT_TTY: {
	int abort_char, flow_ctl, opost;
	abort_char = get_int32(buf); buf+=4;
	flow_ctl = get_int32(buf); buf+= 4;
	opost = get_int32(buf); buf+= 4;
	ret = SLang_init_tty (abort_char,flow_ctl, opost);
	ret_int(port, ret);
	return;
    }

    case SET_ABORT_FUNCTION: {
	SLang_set_abort_signal (NULL);
	ret_int(port, 0);
	return;
    }
    case GETKEY: {
	if (SLang_input_pending (0) == 0) {
	    wait_for = GETKEY;
	    driver_select(port, 0, DO_READ, 1);
	    return;
	}
	x = SLang_getkey ();
	ret_int(port, x);
	return;
    }
    /* read a symbol */
    case KP_GETKEY: {
	if (SLang_input_pending (0) == 0) {
	    wait_for = KP_GETKEY;
	    driver_select(port, 0, DO_READ, 1);
	    return;
	}
	x = SLkp_getkey ();
	ret_int(port, x);
	return;
    }
    case UNGETKEY: {
	unsigned char  key =  (unsigned char) *buf;
	SLang_ungetkey (key);
	return;
    }
    case RESET_TTY: {
	SLang_reset_tty();
	return;
    }
    case KP_INIT: {
	ret_int(port, SLkp_init ());
	return;
    }
    case SETVAR: {
	x = get_int32(buf);buf+= 4;
	y = get_int32(buf);
	switch (x) {
	case  esl_baud_rate:
	    SLang_TT_Baud_Rate = y; return;
	case esl_read_fd:
	    return;
	case esl_abort_char:
	    SLang_Abort_Char = y; return;
	case esl_ignore_user_abort:
	    SLang_Ignore_User_Abort=y; return;
	case esl_input_buffer_len :
	    SLang_Input_Buffer_Len=y; return;
	case  esl_keyboard_quit:
	    SLKeyBoard_Quit=y; return;
	case esl_last_key_char:
	    SLang_Last_Key_Char=y; return;
	case esl_rl_eof_char:
	    SLang_RL_EOF_Char=y; return;
	case esl_rline_quit:
	    SLang_Rline_Quit=y; return;
	case esl_screen_rows:
	case  esl_screen_cols :
	    return;
	case esl_tab_width:
	    SLsmg_Tab_Width=y; return;
	case  esl_newline_behaviour:
	    SLsmg_Newline_Behavior=y; return;
	case esl_error:
	    SLang_Error=y; return;
	case esl_version:
	    return;
	case  esl_backspace_moves :
	    SLsmg_Backspace_Moves=y; return;
	case esl_display_eight_bit:
	    SLsmg_Display_Eight_Bit=y; return;
	default:
	    return;
	}
    }

    case GETVAR: {
	x = get_int32(buf);
	switch (x) {
	case  esl_baud_rate:
	    ret_int(port, SLang_TT_Baud_Rate);
	    return;
	case esl_read_fd:
	    ret_int(port,  SLang_TT_Read_FD);
	    return;
	case esl_abort_char:
	    ret_int(port, SLang_Abort_Char);
	    return;
	case esl_ignore_user_abort:
	    ret_int(port, SLang_Ignore_User_Abort);
	    return;
	case esl_input_buffer_len :
	    ret_int(port, SLang_Input_Buffer_Len);
	    return;
	case  esl_keyboard_quit:
	    ret_int(port, SLKeyBoard_Quit);
	    return;
	case esl_last_key_char:
	    ret_int(port, SLang_Last_Key_Char);
	    return;
	case esl_rl_eof_char:
	    ret_int(port, SLang_RL_EOF_Char);
	    return;
	case esl_rline_quit:
	    ret_int(port, SLang_Rline_Quit);
	    return;
	case esl_screen_rows:
	    ret_int(port, SLtt_Screen_Rows);
	    return;
	case  esl_screen_cols :
	    ret_int(port, SLtt_Screen_Cols);
	    return;
	case esl_tab_width:
	    ret_int(port, SLsmg_Tab_Width);
	    return;
	case  esl_newline_behaviour:
	    ret_int(port, SLsmg_Newline_Behavior);
	    return;
	case esl_error:
	    ret_int(port, SLang_Error);
	    return;
	case esl_version:
	    ret_int(port, SLang_Version);
	    return;
	case  esl_backspace_moves :
	    ret_int(port, SLsmg_Backspace_Moves);
	    return;
	case esl_display_eight_bit:
	    ret_int(port, SLsmg_Display_Eight_Bit);
	    return;
	default:
	    ret_int(port, -1);
	    return;
	}
    }



    /*{{{ SLsmg Screen Management Functions */



    case SMG_FILL_REGION: {
	x = get_int32(buf); buf+= 4;
	y =  get_int32(buf); buf+= 4;
	z = get_int32(buf); buf+= 4;
	v = get_int32(buf); buf+= 4;
	ch = *buf;
	SLsmg_fill_region(x, y,z,v,ch);
	return;
    }
    case SMG_SET_CHAR_SET: {
	x = get_int32(buf); buf+= 4;
	SLsmg_set_char_set(x);
	return;
    }
    case SMG_SUSPEND_SMG: {
	ret_int(port, SLsmg_suspend_smg());
	return;
    }
    case SMG_RESUME_SMG: {
	ret_int(port, SLsmg_resume_smg());
    }
    case SMG_ERASE_EOL: {
	SLsmg_erase_eol();
	return;
    }
    case SMG_GOTORC: {
	x = get_int32(buf); buf+= 4;
	y = get_int32(buf); buf+= 4;
	SLsmg_gotorc(x,  y);
	return;
    }
    case SMG_ERASE_EOS: {
	SLsmg_erase_eos();
	return;
    }
    case SMG_REVERSE_VIDEO: {
	SLsmg_reverse_video();
	return;
    }
    case SMG_SET_COLOR: {
	x = get_int32(buf); buf+= 4;
	SLsmg_set_color(x);
	return;
    }
    case SMG_NORMAL_VIDEO: {
	SLsmg_normal_video();
	return;
    }
    case SMG_WRITE_STRING: {
	SLsmg_write_string(buf);
	return;
    }
    case SMG_WRITE_CHAR: {
	ch = *buf;
	SLsmg_write_char(ch);
	return;
    }
    case SMG_WRITE_WRAPPED_STRING: {
	t1 = buf;
	buf += strlen(t1) + 1;
	x = get_int32(buf); buf+= 4;
	y = get_int32(buf); buf+= 4;
	z = get_int32(buf); buf+= 4;
	v = get_int32(buf); buf+= 4;
	w = get_int32(buf); buf+= 4;
	SLsmg_write_wrapped_string(t1, x,y,z,v,w);
	return;
    }
    case SMG_CLS: {
	SLsmg_cls();
	return;
    }
    case SMG_REFRESH: {
	SLsmg_refresh();
	return;
    }
    case SMG_TOUCH_LINES: {
	x = get_int32(buf); buf+= 4;
	y = get_int32(buf); buf+= 4;
	SLsmg_touch_lines(x, y);
	return;
    }
    case SMG_TOUCH_SCREEN: {
#if (SLANG_VERSION < 10400 )
	ret_int(port, -1);
	return;
#else
	SLsmg_touch_screen();
#endif
	return;
    }
    case SMG_INIT_SMG: {
	ret_int(port,  SLsmg_init_smg());
	return;
    }
    case SMG_REINIT_SMG: {
#if (SLANG_VERSION < 10400 )
	ret_int(port, -1);
	return;
#else
	ret_int(port, SLsmg_reinit_smg());
	return;
#endif
    }
    case SMG_RESET_SMG: {
	SLsmg_reset_smg();
	return;
    }
    case SMG_CHAR_AT: {
	ret_int(port, SLsmg_char_at());
	return;
    }
    case SMG_SET_SCREEN_START: {
	int ip1, ip2;
	ip1 = get_int32(buf); buf+= 4;
	ip2 = get_int32(buf); buf+= 4;

	SLsmg_set_screen_start(&ip1, &ip2);
	ret_int_int(port, ip1, ip2);
	return;
    }
    case SMG_DRAW_HLINE: {
	x = get_int32(buf); buf+= 4;
	SLsmg_draw_hline(x);
	return;
    }
    case SMG_DRAW_VLINE: {
	x = get_int32(buf); buf+= 4;
	SLsmg_draw_vline(x);
	return;
    }
    case SMG_DRAW_OBJECT: {
	x = get_int32(buf); buf+= 4;
	y = get_int32(buf); buf+= 4;
	z = get_int32(buf); buf+= 4;
	SLsmg_draw_object(x, y, z);
	return;
    }
    case SMG_DRAW_BOX: {
	x = get_int32(buf); buf+= 4;
	y = get_int32(buf); buf+= 4;
	z = get_int32(buf); buf+= 4;
	v = get_int32(buf); buf+= 4;
	SLsmg_draw_box(x, y,z,v);
	return;
    }
    case SMG_GET_COLUMN: {
	ret_int(port, SLsmg_get_column());
	return;
    }
    case SMG_GET_ROW: {
	ret_int(port, SLsmg_get_row());
	return;
    }

    case SMG_FORWARD: {
	x = get_int32(buf); buf+= 4;
	SLsmg_forward(x);
	return;
    }
    case SMG_WRITE_COLOR_CHARS: {
	SLsmg_Char_Type * sl;
	sl = decode_smg_char_type(&buf);
	x = get_int32(buf); buf+= 4;
	SLsmg_write_color_chars(sl, x);
	return;
    }
    case SMG_READ_RAW: {
	x = get_int32(buf); buf+= 4;
	t1 = malloc((2*x) + 2 + 1);
	y = SLsmg_read_raw((unsigned short*)t1 +1, x);
	t1[1] = 1;
	driver_output(port, t1, y+1);
	free(t1);
	return;
    }
    case SMG_WRITE_RAW: {
	SLsmg_Char_Type * sl;
	sl = decode_smg_char_type(&buf);
	x = get_int32(buf);
	y = SLsmg_write_raw(sl, x);
	ret_int(port, y);
	return;
    }
    case SMG_SET_COLOR_IN_REGION: {
	x = get_int32(buf); buf+= 4;
	y = get_int32(buf); buf+= 4;
	z = get_int32(buf); buf+= 4;
	v = get_int32(buf); buf+= 4;
	w = get_int32(buf); buf+= 4;
	SLsmg_set_color_in_region(x, y,z,v,w);
	return;
    }




    /* all the tt_functions  */

    case TT_FLUSH_OUTPUT: {
	ret = SLtt_flush_output();
	ret_int(port, ret);
	return;
    }
    case TT_SET_SCROLL_REGION: {

	x = get_int32(buf); buf+=4;
	y = get_int32(buf); buf+=4;
	SLtt_set_scroll_region(x, y);
	return;
    }
    case TT_RESET_SCROLL_REGION: {
	SLtt_reset_scroll_region();
	return;
    }
    case TT_REVERSE_VIDEO: {
	SLtt_reverse_video (get_int32(buf));
	return;
    }
    case TT_BOLD_VIDEO: {
	SLtt_begin_insert();
	return;
    }
    case TT_BEGIN_INSERT: {
	SLtt_begin_insert();
	return;
    }
    case TT_END_INSERT: {
	SLtt_end_insert();
	return;
    }
    case TT_DEL_EOL: {
	SLtt_del_eol();
	return;
    }
    case TT_GOTO_RC: {
	x = get_int32(buf); buf+=4;
	y = get_int32(buf); buf+=4;
	SLtt_goto_rc (x, y);
	return;
    }
    case TT_DELETE_NLINES: {
	SLtt_delete_nlines(get_int32(buf));
	return;
    }
    case TT_DELETE_CHAR: {
	SLtt_delete_char();
	return;
    }
    case TT_ERASE_LINE: {
	SLtt_erase_line();
	return;
    }
    case TT_NORMAL_VIDEO: {
	SLtt_normal_video();
	return;
    }
    case TT_CLS: {
	SLtt_cls();
	return;
    }
    case TT_BEEP: {
	SLtt_beep();
	return;
    }
    case TT_REVERSE_INDEX: {
	SLtt_reverse_index(get_int32(buf));
	return;
    }
    case TT_SMART_PUTS: {
	SLsmg_Char_Type *t1 ;
	SLsmg_Char_Type *t2;

	t1 = decode_smg_char_type(&buf);
	t2 = decode_smg_char_type(&buf);
	x = get_int32(buf); buf+=4;
	y = get_int32(buf); buf+=4;
	SLtt_smart_puts(t1, t2,x,y);
	return;
    }
    case TT_WRITE_STRING: {
	SLtt_write_string (buf);
	return;
    }
    case TT_PUTCHAR: {
	SLtt_putchar((char) get_int32(buf));
	return;
    }
    case TT_INIT_VIDEO: {
	ret = SLtt_init_video ();
	ret_int(port, ret);
	return;
    }
    case TT_RESET_VIDEO: {
	SLtt_reset_video ();
	return;
    }
    case TT_GET_TERMINFO: {
	SLtt_get_terminfo();
	return;
    }
    case TT_GET_SCREEN_SIZE: {
	SLtt_get_screen_size ();
	return;
    }
    case TT_SET_CURSOR_VISIBILITY: {
	ret = SLtt_set_cursor_visibility (get_int32(buf));
	ret_int(port, ret);
	return;
    }
    case TT_SET_MOUSE_MODE: {
	x = get_int32(buf); buf+=4;
	y = get_int32(buf); buf+=4;
	ret = SLtt_set_mouse_mode (x,y);
	ret_int(port, ret);
	return;
    }

    case TT_INITIALIZE: {
	ret =SLtt_initialize (buf);
	ret_int(port, ret);
	return;
    }
    case TT_ENABLE_CURSOR_KEYS: {
	SLtt_enable_cursor_keys();
	return;
    }
    case TT_SET_TERM_VTXXX: {
	return;
    }
    case TT_SET_COLOR_ESC: {
	x = get_int32(buf); buf+=4;
	SLtt_set_color_esc (x, buf);
	return;
    }
    case TT_WIDE_WIDTH: {
	SLtt_narrow_width();
	return;
    }
    case TT_NARROW_WIDTH: {
	SLtt_narrow_width();
	return;
    }
    case TT_SET_ALT_CHAR_SET: {
	SLtt_set_alt_char_set (get_int32(buf));
	return;
    }
    case TT_WRITE_TO_STATUS_LINE: {
	x = get_int32(buf); buf+=4;
	SLtt_write_to_status_line (buf, x);
	return;
    }
    case TT_DISABLE_STATUS_LINE: {
	SLtt_disable_status_line ();
	return;
    }


    case TT_TGETSTR: {
	str = SLtt_tgetstr (buf);
	ret_string(port, str);
	return;
    }
    case TT_TGETNUM: {
	x = SLtt_tgetnum (buf);
	ret_int(port, x);
	return;
    }
    case TT_TGETFLAG: {
	x = SLtt_tgetflag (buf);
	ret_int(port, x);
	return;
    }
    case TT_TIGETENT: {
	str = SLtt_tigetent (buf);
	ret_string(port, str);
	return;
    }
    case TT_TIGETSTR: {
	return;
    }
    case TT_TIGETNUM: {
	return;
    }

    case SLTT_GET_COLOR_OBJECT: {
	x = get_int32(buf); buf+=4;
	y = SLtt_get_color_object (x);
	ret_int(port, y);
	return;
    }
    case TT_SET_COLOR_OBJECT: {
	x = get_int32(buf); buf+=4;
	y = get_int32(buf); buf+=4;
	SLtt_set_color_object (x, y);
	return;
    }
    case TT_SET_COLOR: {
	x = get_int32(buf); buf+=4;
	t1 = buf;
	t2 = buf + (strlen(t1) + 1);
	t3 = buf + (strlen(t1) + strlen(t2) + 2);
	SLtt_set_color (x, t1, t2, t3);
	return;
    }
    case TT_SET_MONO: {
	x = get_int32(buf); buf+=4;
	t1 = buf;
	buf += strlen(t1) + 1;
	y = get_int32(buf);
	SLtt_set_mono (x, t1, y);
	return;
    }
    case TT_ADD_COLOR_ATTRIBUTE: {
	x = get_int32(buf); buf+=4;
	y = get_int32(buf); buf+=4;
	SLtt_add_color_attribute (x, y);
	return;
    }
    case TT_SET_COLOR_FGBG: {
	x = get_int32(buf); buf+=4;
	y = get_int32(buf); buf+=4;
	z = get_int32(buf); buf+=4;
	SLtt_set_color_fgbg (x, y, z);
	return;
    }
    case ISATTY: {
	x = get_int32(buf); buf+=4;
	ret_int(port, isatty(x));
	return;
    }
    case EFORMAT: {
	fputs(buf, stderr);
	fflush(stderr);
	return;
    }
    case SIGNAL: {
	x = get_int32(buf); buf+=4;
	SLsignal(x_to_sig(x), sig_handler);
	return;
    }
    case SIGNAL_CHECK: {
	/* polled */
	ret_int(port, signal_cought);
	signal_cought = 0;
	return;
    }
    }
}




/* pending getkey request */
void sl_ready_input(ErlDrvData drv_data, ErlDrvEvent fd)
{
    ErlDrvPort port = (ErlDrvPort)drv_data;
    unsigned int key;
    driver_select(port, 0, DO_READ, 0);
    switch (wait_for) {
    case GETKEY: {
	key = SLang_getkey ();
	ret_int(port, key);
	return;
    }
    case KP_GETKEY: {
	key = SLkp_getkey ();
	ret_int(port, key);
	return;
    }
    }
}

/*
 * Initialize and return a driver entry struct
 */

DRIVER_INIT(slang_drv)
{
    memset(&sl_erl_drv_entry, 0, sizeof(sl_erl_drv_entry));

    sl_erl_drv_entry.start = sl_start;
    sl_erl_drv_entry.stop = sl_stop;
    sl_erl_drv_entry.output = sl_output;
    sl_erl_drv_entry.ready_input = sl_ready_input;
    sl_erl_drv_entry.driver_name = "slang_drv";

    return &sl_erl_drv_entry;
}

