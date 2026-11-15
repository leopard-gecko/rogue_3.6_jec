/*
 * This file has all the code for the option command.
 * I would rather this command were not necessary, but
 * it is the only way to keep the wolves off of my back.
 *
 * @(#)options.c	3.3 (Berkeley) 5/25/81
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

#define	NUM_OPTS	(sizeof optlist / sizeof (OPTION))
#define OPT_REDRAW	3

/*
 * description of an option and what to do with it
 */
struct optstruct {
    char	*o_name;	/* option name */
    char	*o_prompt;	/* prompt for interactive entry */
    void	*o_opt;		/* pointer to thing to set */
    void	(*o_putfunc)(void *);	/* function to print value */
    int		(*o_getfunc)(void *, WINDOW *);	/* function to get value interactively */
};

typedef struct optstruct	OPTION;

static void pr_optname(OPTION *op);
static void option_redraw(void);
static void put_lang(void *vp);
static int get_lang(void *vp, WINDOW *win);

static size_t option_size(void *opt)
{
    if (opt == (void *)whoami)
        return sizeof whoami;
    if (opt == (void *)fruit)
        return sizeof fruit;
    if (opt == (void *)file_name)
        return sizeof file_name;
    if (opt == (void *)prbuf)
        return sizeof prbuf;
    return ROGUE_INPUT_SIZE;
}


OPTION	optlist[] = {
    {"terse",   JSTR_STATIC("簡潔な表示", "Terse output"),
		 &terse,        put_bool, get_bool },
    {"flush",   JSTR_STATIC("戦闘時に先行入力をキャンセル", "Flush typeahead during battle"),
		 &fight_flush,  put_bool, get_bool },
    {"jump",    JSTR_STATIC("移動中の表示を行わない", "Show position only at end of run"),
		 &jump,         put_bool, get_bool },
    {"step",    JSTR_STATIC("持ちものを１行ずつ表示", "Do inventories one line at a time"),
		 &slow_invent,  put_bool, get_bool },
    {"askme",   JSTR_STATIC("未識別の品物について尋ねる", "Ask me about unidentified things"),
		 &askme,        put_bool, get_bool },
    {"name",    JSTR_STATIC("プレイヤー名", "Name"),
		 whoami,        put_str,  get_str  },
    {"fruit",   JSTR_STATIC("好きな果物", "Fruit"),
		 fruit,         put_str,  get_str  },
    {"language", JSTR_STATIC("表示する言語", "Language"),
		 &jrogue_lang_is_japanese, put_lang, get_lang },
    {"file",    JSTR_STATIC("セーブファイル名", "Save file"),
		 file_name,     put_str,  get_str  }
};

void
jrogue_apply_options_language(void)
{
    optlist[0].o_prompt = (char *) JSTR("簡潔な表示", "Terse output");
    optlist[1].o_prompt = (char *) JSTR("戦闘時に先行入力をキャンセル", "Flush typeahead during battle");
    optlist[2].o_prompt = (char *) JSTR("移動中の表示を行わない", "Show position only at end of run");
    optlist[3].o_prompt = (char *) JSTR("持ちものを１行ずつ表示", "Do inventories one line at a time");
    optlist[4].o_prompt = (char *) JSTR("未識別の品物について尋ねる", "Ask me about unidentified things");
    optlist[5].o_prompt = (char *) JSTR("プレイヤー名", "Name");
    optlist[6].o_prompt = (char *) JSTR("好きな果物", "Fruit");
    optlist[7].o_prompt = (char *) JSTR("表示する言語", "Language");
    optlist[8].o_prompt = (char *) JSTR("セーブファイル名", "Save file");
}

/*
 * Read option keys from the displayed option window, not from cw.
 * readchar() uses cw (the game window) and wgetch(cw) can refresh it,
 * moving the physical cursor back to the top left while hw is visible.
 */
static int
option_readchar(WINDOW *win)
{
    int ch;

    do
        ch = wgetch(win);
    while (ch == ERR);
    return ch;
}

static void
pr_optname(OPTION *op)
{
    wprintw(hw, "%s (\"%s\"): ", op->o_prompt, op->o_name);
}

/*
 * Calculate the beginning of the value in terminal cells, not UTF-8 bytes.
 * In particular, do not find it by printing the Japanese label twice:
 * overwriting the second cell of a wide character can corrupt PDCursesMod's
 * window buffer when launched under the MSYS2/mintty terminal.
 */
static int
option_value_column(const OPTION *op)
{
    return rogue_display_width(op->o_prompt) + (int)strlen(op->o_name) + 7;
}

static void
option_redraw(void)
{
    OPTION *op;
    int row;

    rogue_vt_clear_window(hw);
    wclear(hw);
    clearok(hw, TRUE);
    touchwin(hw);
    for (op = optlist, row = 0; op < &optlist[NUM_OPTS]; op++, row++)
    {
        /* Do not rely on newline's column handling for wide strings. */
        wmove(hw, row, 0);
	pr_optname(op);
	(*op->o_putfunc)(op->o_opt);
    }
    touchwin(hw);
    draw(hw);
}

/*
 * print and then set options from the terminal
 */
void option(void)
{
    register OPTION	*op;
    register int	retval;

    msg("");
    clearok(cw, TRUE);
    touchwin(cw);
    draw(cw);

    option_redraw();

    /*
     * Set values
     */
    for (op = optlist; op < &optlist[NUM_OPTS]; op++)
    {
        /* The label was already rendered by option_redraw().  Writing it
         * again would overlap its double-width Japanese cells on Windows. */
        wmove(hw, (int)(op - optlist), option_value_column(op));
	if ((retval = (*op->o_getfunc)(op->o_opt, hw)))
	{
	    if (retval == QUIT)
		break;
	    else if (retval == OPT_REDRAW)
	    {
		option_redraw();
		wmove(hw, (int)(op - optlist) + 1, 0);
	    }
	    else if (op > optlist) {	/* MINUS */
		wmove(hw, (int)(op - optlist) - 1, 0);
		op -= 2;
	    }
	    else	/* trying to back up beyond the top */
	    {
		waddch(hw, '\007');
		wmove(hw, 0, 0);
		op--;
	    }
	}
    }
    /*
     * Switch back to original screen
     */
    mvwaddstr(hw, ROGUE_SCREEN_LINES-1, 0, JSTR("--スペースを押してください--", "--Press space to continue--"));
    draw(hw);
    wait_for_window(hw, ' ');
    clearok(cw, TRUE);
    touchwin(cw);
    after = FALSE;
}

/*
 * put out a boolean
 */
void put_bool(void *b)
{
    bool *bp = (bool *) b;
    waddstr(hw, *bp ? JSTR("有効(T)", "True") : JSTR("無効(F)", "False"));
}

/*
 * put out a string
 */
void put_str(void *str)
{
    rogue_waddstr_utf8(hw, (const char *) str);
}

static void
put_lang(void *vp)
{
    int *jp = (int *) vp;

    if (*jp)
	waddstr(hw, JSTR("日本語(J)", "Japanese(J)"));
    else
	waddstr(hw, JSTR("英語(E)", "English(E)"));
}

static int
get_lang(void *vp, WINDOW *win)
{
    int *jp = (int *) vp;
    int old_lang;
    int oy, ox;

    old_lang = *jp;
    getyx(win, oy, ox);
    put_lang(vp);

    for (;;)
    {
	wmove(win, oy, ox);
	draw(win);
	switch (option_readchar(win))
	{
	    case 'j':
	    case 'J':
		*jp = TRUE;
		break;
	    case 'e':
	    case 'E':
		*jp = FALSE;
		break;
	    case '\n':
	    case '\r':
		wmove(win, oy, ox);
		put_lang(vp);
		wclrtoeol(win);
		return NORM;
	    case '\033':
	    case '\007':
		return QUIT;
	    case '-':
		return MINUS;
	    default:
		mvwaddstr(win, oy, ox + 15, JSTR("［日本語(J)、英語(E)］", "(J or E)"));
		continue;
	}
	break;
    }

    jrogue_apply_language();
    wmove(win, oy, ox);
    put_lang(vp);
    wclrtoeol(win);

    return (*jp != old_lang) ? OPT_REDRAW : NORM;
}

/*
 * allow changing a boolean option and print it out
 */

int get_bool(void *vbp, WINDOW *win)
{
    bool *bp = (bool *) vbp;
    register int oy, ox;
    register bool op_bad;

    op_bad = TRUE;
    getyx(win, oy, ox);
    waddstr(win, *bp ? JSTR("有効(T)", "True") : JSTR("無効(F)", "False"));
    while(op_bad)	
    {
	wmove(win, oy, ox);
	draw(win);
	switch (option_readchar(win))
	{
	    case 't':
	    case 'T':
		*bp = TRUE;
		op_bad = FALSE;
		break;
	    case 'f':
	    case 'F':
		*bp = FALSE;
		op_bad = FALSE;
		break;
	    case '\n':
	    case '\r':
		op_bad = FALSE;
		break;
	    case '\033':
	    case '\007':
		return QUIT;
	    case '-':
		return MINUS;
	    default:
		mvwaddstr(win, oy, ox + 10, JSTR("［有効(T)、無効(F)］", "(T or F)"));
	}
    }
    wmove(win, oy, ox);
    waddstr(win, *bp ? JSTR("有効(T)", "True") : JSTR("無効(F)", "False"));
    return NORM;
}

/*
 * set a string option
 */

/* Windows VT string editing uses two input transports:
 * - Win32 console: KEY_EVENT_RECORD with UTF-16 code units.
 * - mintty/MSYS2: a pipe carrying UTF-8 bytes (NOT a Win32 console).
 * These are handled by rogue_windows_edit_key(), which returns Unicode
 * scalar values to the editor.  Do not use the CRT _getwch() for mintty:
 * supplementary-plane characters can be lost in that conversion. */
static int
str_input_byte(WINDOW *win)
{
    return wgetch(win);
}

static int
str_input_key(WINDOW *win)
{
#ifdef _WIN32
    if (rogue_windows_has_edit_input())
        return rogue_windows_edit_key();
#endif
    return wgetch(win);
}

static int
str_read_utf8_char(WINDOW *win, int first, char *out)
{
    int i, need, c;

#ifdef _WIN32
    /* Windows VT input is decoded as Unicode scalars from console
     * UTF-16 events or mintty UTF-8 bytes. */
    if (first >= 0x80 && first <= 0x10ffff)
    {
        unsigned int u = (unsigned int)first;
        if (u >= 0xd800 && u <= 0xdfff)
            return 0;
        if (u < 0x800)
        {
            out[0] = (char)(0xc0 | (u >> 6));
            out[1] = (char)(0x80 | (u & 0x3f));
            return 2;
        }
        if (u < 0x10000)
        {
            out[0] = (char)(0xe0 | (u >> 12));
            out[1] = (char)(0x80 | ((u >> 6) & 0x3f));
            out[2] = (char)(0x80 | (u & 0x3f));
            return 3;
        }
        out[0] = (char)(0xf0 | (u >> 18));
        out[1] = (char)(0x80 | ((u >> 12) & 0x3f));
        out[2] = (char)(0x80 | ((u >> 6) & 0x3f));
        out[3] = (char)(0x80 | (u & 0x3f));
        return 4;
    }
#endif
    if (first < 0 || first > 255)
        return 0;
    need = rogue_utf8_lead_bytes((unsigned char)first);
    if (need == 0)
        return 0;
    out[0] = (char)first;

    for (i = 1; i < need; i++)
    {
        c = str_input_byte(win);
        if (c < 0 || c > 255 || (((unsigned char)c & 0xc0) != 0x80))
        {
            if (c >= 0)
                ungetch(c);
            return 0;
        }
        out[i] = (char)c;
    }
    /* Reject overlong UTF-8, surrogates and values above U+10FFFF.
     * The continuation-byte checks alone are not sufficient. */
    out[need] = '\0';
    if (rogue_utf8_char_bytes(out, (size_t)need) != need)
        return 0;
    return need;
}

static void
str_redraw_edit(WINDOW *win, int y, int x, const char *buf, int pos)
{
    /* An edit replaces all previous emoji overlays on this value row. */
    rogue_vt_clear_row(win, y);
    wmove(win, y, x);
    rogue_waddstr_utf8(win, buf);
    wclrtoeol(win);
    wmove(win, y, x + rogue_utf8_width_n(buf, pos));
    draw(win);
}

int get_str(void *vopt, WINDOW *win)
{
    char *opt = (char *) vopt;
    int c, oy, ox, len, pos, clen, editing;
    char chbuf[5];
    char buf[ROGUE_INPUT_SIZE];

    draw(win);
    getyx(win, oy, ox);
    keypad(win, TRUE);
    buf[0] = '\0';
    len = 0;
    pos = 0;
    editing = FALSE;

    /*
     * Keep the historical 3.6 behavior: the current value remains visible
     * while waiting for input, and the option is replaced only after the
     * user types a non-empty new string.  Once replacement input begins,
     * edit that new UTF-8 string safely.
     */
    for (;;)
    {
	if (editing)
	    str_redraw_edit(win, oy, ox, buf, pos);
	else
	{
	    wmove(win, oy, ox);
	    draw(win);
	}
	c = str_input_key(win);

	if (c == '\n' || c == '\r' || c == '\033' || c == '\007')
	    break;
	if (c == -1)
	    continue;
#ifdef KEY_LEFT
	if (c == KEY_LEFT)
	{
	    if (editing)
		pos = rogue_utf8_prev_char(buf, pos);
	    continue;
	}
#endif
#ifdef KEY_RIGHT
	if (c == KEY_RIGHT)
	{
	    if (editing)
		pos = rogue_utf8_next_char(buf, pos, len);
	    continue;
	}
#endif
#if defined(KEY_BACKSPACE)
	if (c == KEY_BACKSPACE || c == '\b' || c == 0177)
#else
	if (c == '\b' || c == 0177)
#endif
	{
	    if (editing && pos > 0)
	    {
		int prev = rogue_utf8_prev_char(buf, pos);
		memmove(&buf[prev], &buf[pos], (size_t)(len - pos + 1));
		len -= pos - prev;
		pos = prev;
	    }
	    continue;
	}
#ifdef KEY_DC
	if (c == KEY_DC)
	{
	    if (editing && pos < len)
	    {
		int next = rogue_utf8_next_char(buf, pos, len);
		memmove(&buf[pos], &buf[next], (size_t)(len - next + 1));
		len -= next - pos;
	    }
	    continue;
	}
#endif
	if (c == CTRL('U'))	/* process kill character */
	{
	    if (editing)
	    {
		len = 0;
		pos = 0;
		buf[0] = '\0';
	    }
	    continue;
	}
	if (!editing)
	{
	    if (c == '-')
		break;
	    editing = TRUE;
	    len = 0;
	    pos = 0;
	    buf[0] = '\0';
	    wmove(win, oy, ox);
	    wclrtoeol(win);
	}
	if (len == 0 && pos == 0 && c == '~')
	{
	    rogue_utf8_copy(buf, sizeof buf, home);
	    len = (int)strlen(buf);
	    pos = len;
	    continue;
	}
	if (c < 0
#ifndef _WIN32
            || c > 255
#endif
            )
	{
	    putchar('\007');
	    continue;
	}
	clen = str_read_utf8_char(win, c, chbuf);
	if (clen <= 0)
	{
	    putchar('\007');
	    continue;
	}
	chbuf[clen] = '\0';
	if (len + clen >= (int)sizeof buf)
	{
	    putchar('\007');
	    continue;
	}
	/* Width must be checked AFTER insertion: a combining accent, flag
	 * pair or keycap may merge with neighboring characters. */
	{
	    char candidate[ROGUE_INPUT_SIZE];
	    memcpy(candidate, buf, (size_t)pos);
	    memcpy(candidate + pos, chbuf, (size_t)clen);
	    memcpy(candidate + pos + clen, buf + pos,
	           (size_t)(len - pos + 1));
	    if (rogue_display_width(candidate) > ROGUE_INPUT_MAX_WIDTH)
	    {
		putchar('\007');
		continue;
	    }
	}
	memmove(&buf[pos + clen], &buf[pos], (size_t)(len - pos + 1));
	memcpy(&buf[pos], chbuf, (size_t)clen);
	len += clen;
	pos += clen;
	/* The inserted scalar can join the following cluster. Keep the
	 * cursor on a complete grapheme boundary. */
	pos = rogue_utf8_next_char(buf, pos - 1, len);
    }
    if (len > 0)	/* only change option if something has been typed */
	rogue_utf8_copy(opt, option_size(opt), buf);
    rogue_vt_clear_row(win, oy);
    wmove(win, oy, ox);
    rogue_waddstr_utf8(win, opt);
    wclrtoeol(win);
    if (win != hw)
        waddch(win, '\n');
    draw(win);
    if (win == cw)
	mpos += rogue_utf8_width_n(buf, len);
    if (c == '-')
	return MINUS;
    else if (c == '\033' || c == '\007')
	return QUIT;
    else
	return NORM;
}

/*
 * parse options from string, usually taken from the environment.
 * the string is a series of comma seperated values, with booleans
 * being stated as "name" (true) or "noname" (false), and strings
 * being "name=....", with the string being defined up to a comma
 * or the end of the entire option string.
 */

void parse_opts(char *str)
{
    register char *sp;
    register OPTION *op;
    register int len;

    while (*str)
    {
	/*
	 * Get option name
	 */
	for (sp = str; isalpha(*sp); sp++)
	    continue;
	len = sp - str;
	/*
	 * Look it up and deal with it
	 */
	for (op = optlist; op < &optlist[NUM_OPTS]; op++)
	    if (EQSTR(str, op->o_name, len)
	      && (op->o_putfunc != put_lang ||
	          len == (int)strlen(op->o_name)))
	    {
		if (op->o_putfunc == put_bool)	/* if option is a boolean */
		    *(bool *)op->o_opt = TRUE;
		else if (op->o_putfunc == put_lang)
		{
		    register char *start;
		    for (start = sp; *start == '='; start++)
			continue;
		    if (jrogue_set_language(start))
			jrogue_apply_language();
		    for (sp = start; *sp && *sp != ','; sp++)
			continue;
		}
		else				/* string option */
		{
		    register char *start;
		    /*
		     * Skip to start of string value
		     */
		    for (str = sp + 1; *str == '='; str++)
			continue;
		    if (*str == '~')
		    {
			rogue_utf8_copy((char *) op->o_opt, option_size(op->o_opt), home);
			start = (char *) op->o_opt + strlen((char *) op->o_opt);
			while (*++str == '/')
			    continue;
		    }
		    else
			start = (char *) op->o_opt;
		    /*
		     * Skip to end of string value
		     */
		    for (sp = str + 1; *sp && *sp != ','; sp++)
			continue;
		    rogue_utf8_copy_n(start, option_size(op->o_opt) - (size_t)(start - (char *)op->o_opt), str, (size_t)(sp - str));
		}
		break;
	    }
	    /*
	     * check for "noname" for booleans
	     */
	    else if (op->o_putfunc == put_bool
	      && EQSTR(str, "no", 2) && EQSTR(str + 2, op->o_name, len - 2))
	    {
		*(bool *)op->o_opt = FALSE;
		break;
	    }

	/*
	 * skip to start of next option name
	 */
	while (*sp && !isalpha(*sp))
	    sp++;
	str = sp;
    }
}
