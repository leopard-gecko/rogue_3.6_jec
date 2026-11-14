/*
 * Various input/output functions
 *
 * @(#)io.c	3.10 (Berkeley) 6/15/81
 */

#include <curses.h>
#include <ctype.h>
#include <stdarg.h>
#include "rogue.h"

/*
 * Initialize the physical terminal, then create 80x24 logical windows.
 * A larger terminal remains larger, but the dungeon and status line stay
 * in the upper-left 80x24 area.  Never use LINES/COLS as map dimensions.
 */
int rogue_init_screen(void)
{
    int terminal_lines, terminal_cols;

    if (initscr() == NULL)
        return FALSE;
    getmaxyx(stdscr, terminal_lines, terminal_cols);
    if (terminal_lines < ROGUE_SCREEN_LINES ||
        terminal_cols < ROGUE_SCREEN_COLS)
    {
        endwin();
        fprintf(stderr, "%s", JSTR(
            "Rogue 3.6には80列×24行以上の端末が必要です。\n",
            "Rogue 3.6 requires a terminal of at least 80 columns by 24 lines.\n"));
        return FALSE;
    }

    cw = newwin(ROGUE_SCREEN_LINES, ROGUE_SCREEN_COLS, 0, 0);
    mw = newwin(ROGUE_SCREEN_LINES, ROGUE_SCREEN_COLS, 0, 0);
    hw = newwin(ROGUE_SCREEN_LINES, ROGUE_SCREEN_COLS, 0, 0);
    if (cw == NULL || mw == NULL || hw == NULL)
    {
        if (cw != NULL) delwin(cw);
        if (mw != NULL) delwin(mw);
        if (hw != NULL) delwin(hw);
        cw = mw = hw = NULL;
        endwin();
        fprintf(stderr, "%s", JSTR(
            "Rogueの画面を作成できませんでした。\n",
            "Cannot create Rogue screen windows.\n"));
        return FALSE;
    }
    setup();
    return TRUE;
}

/*
 * msg:
 *	Display a message at the top of the screen.
 */

static char msgbuf[BUFSIZ];
static int newpos = 0;
static int msglen = 0;

/*VARARGS1*/
void
msg(const char *fmt, ...)
{
    va_list ap;

    if (*fmt == '\0')
    {
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	mpos = 0;
	newpos = 0;
	msglen = 0;
	msgbuf[0] = '\0';
	return;
    }
    va_start(ap, fmt);
    doadd(fmt, ap);
    va_end(ap);
    endmsg();
}

/*
 * add things to the current message
 */
void
addmsg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    doadd(fmt, ap);
    va_end(ap);
}

/*
 * Display a new msg (giving him a chance to see the previous one if it
 * is up there with the --More--)
 */
static int
more_col(void)
{
    int prompt_width, maxcol;

    prompt_width = rogue_display_width(JSTR(" --続く--", " --More--"));
    maxcol = ROGUE_SCREEN_COLS - prompt_width;
    if (maxcol < 0)
	maxcol = 0;
    if (mpos > maxcol)
	return maxcol;
    return mpos;
}

void endmsg(void)
{
    rogue_utf8_copy_message_history(huh, sizeof huh, msgbuf);
    if (mpos)
    {
	wmove(cw, 0, more_col());
	waddstr(cw, JSTR(" --続く--", " --More--"));
	draw(cw);
	wait_for(' ');
    }
    rogue_screen_prepare_text_row(cw, 0);
    rogue_waddstr_utf8(cw, msgbuf);
    wclrtoeol(cw);
    mpos = newpos;
    newpos = 0;
    msglen = 0;
    msgbuf[0] = '\0';
    draw(cw);
}

void
doadd(const char *fmt, va_list ap)
{
    int len;

    if (msglen < 0 || msglen >= (int) sizeof msgbuf)
    {
	msglen = 0;
	msgbuf[0] = '\0';
    }
    len = vsnprintf(&msgbuf[msglen], sizeof msgbuf - (size_t)msglen, fmt, ap);
    if (len < 0)
	return;
    msglen += len;
    if (msglen >= (int) sizeof msgbuf)
    {
	msglen = (int) sizeof msgbuf - 1;
	msgbuf[msglen] = '\0';
    }
    newpos = rogue_display_width(msgbuf);
}

/*
 * step_ok:
 *	returns true if it is ok to step on ch
 */

int step_ok(int ch)
{
    switch (ch)
    {
	case ' ':
	case '|':
	case '-':
	case SECRETDOOR:
	    return FALSE;
	default:
	    return (!isalpha(ch));
    }
}

/*
 * readchar:
 *	return a single command character from the game window.
 */

int readchar(void)
{
    int c;

    /*
     * Keep command input on the player-visible game window.  In Rogue 3.6,
     * stdscr is used as the hidden full-level map; reading from stdscr can
     * expose that map on modern curses implementations.
     */
    for (;;)
    {
        c = wgetch(cw);
        if (c == ERR)
            continue;
#ifdef KEY_BACKSPACE
        if (c == KEY_BACKSPACE)
            return '\b';
#endif
        if (c == 0x7f)
            return '\b';
        return c;
    }
}




/*
 * rogue_delay_ms:
 *	Sleep for a small number of milliseconds, preserving interrupted sleeps.
 */
void rogue_delay_ms(int ms)
{
    if (ms <= 0)
	return;
#ifdef _WIN32
    rogue_windows_sleep(ms);
#else
    {
        struct timespec ts;
        struct timespec rem;
        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (long)(ms % 1000) * 1000000L;
        while (nanosleep(&ts, &rem) == -1 && errno == EINTR)
            ts = rem;
    }
#endif
}

/*
 * rogue_animation_delay:
 *	Make short visual effects visible on modern fast terminals.
 */
void rogue_animation_delay(void)
{
    rogue_delay_ms(ROGUE_ANIMATION_DELAY_MS);
}

/*
 * rogue_flush_typeahead:
 *	Discard any pending keyboard input without changing the terminal mode.
 */
void rogue_flush_typeahead(void)
{
    flushinp();
}

/*
 * unctrl:
 *	Print a readable version of a certain character
 */

char * unctrl(char ch)
{
    static char buf[5];
    unsigned int c;

    c = ((unsigned int) ch) & 0177;
    if (c < 040) {
        buf[0] = '^';
        buf[1] = (char) (c + '@');
        buf[2] = '\0';
        return buf;
    }
    if (c == 0177) {
        return "^?";
    }
    buf[0] = (char) c;
    buf[1] = '\0';
    return buf;
}

/*
 * status:
 *	Display the important stats line.  Keep the cursor where it was.
 */

void status(void)
{
    register int oy, ox, temp;
    int n;
    size_t off;
    static char buf[128];
    static int hpwidth = 0, s_hungry = -1, s_lang = -1;
    static int s_lvl = -1, s_pur, s_hp = -1, s_str, s_add, s_ac = 0;
    static long s_exp = 0;

    /*
     * If nothing has changed since the last status, don't
     * bother.
     */
    if (s_hp == pstats.s_hpt && s_exp == pstats.s_exp && s_pur == purse
	&& s_ac == (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm)
	&& s_str == pstats.s_str.st_str && s_add == pstats.s_str.st_add
	&& s_lvl == level && s_hungry == hungry_state
	&& s_lang == jrogue_lang_is_japanese)
	    return;
	
    getyx(cw, oy, ox);
    if (s_hp != max_hp)
    {
	temp = s_hp = max_hp;
	for (hpwidth = 0; temp; hpwidth++)
	    temp /= 10;
    }
    off = 0;
    n = snprintf(buf + off, sizeof buf - off,
	JSTR("階: %d  金塊: %-5d  体力: %*d(%*d)  強さ: %-2d",
	     "Level: %d  Gold: %-5d  Hp: %*d(%*d)  Str: %-2d"),
	level, purse, hpwidth, pstats.s_hpt, hpwidth, max_hp,
	pstats.s_str.st_str);
    if (n > 0)
	off += (n < (int)(sizeof buf - off)) ? (size_t)n : sizeof buf - off - 1;
    if (pstats.s_str.st_add != 0 && off < sizeof buf - 1)
    {
	n = snprintf(buf + off, sizeof buf - off, "/%d", pstats.s_str.st_add);
	if (n > 0)
	    off += (n < (int)(sizeof buf - off)) ? (size_t)n : sizeof buf - off - 1;
    }
    if (off < sizeof buf - 1)
    {
	n = snprintf(buf + off, sizeof buf - off,
	    JSTR("  守備: %-2d  経験: %d/%ld",
	         "  Ac: %-2d  Exp: %d/%ld"),
	    cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm,
	    pstats.s_lvl, pstats.s_exp);
	if (n > 0)
	    off += (n < (int)(sizeof buf - off)) ? (size_t)n : sizeof buf - off - 1;
    }
    /*
     * Save old status
     */
    s_lvl = level;
    s_pur = purse;
    s_hp = pstats.s_hpt;
    s_str = pstats.s_str.st_str;
    s_add = pstats.s_str.st_add;
    s_exp = pstats.s_exp; 
    s_ac = (cur_armor != NULL ? cur_armor->o_ac : pstats.s_arm);
    mvwaddstr(cw, ROGUE_SCREEN_LINES - 1, 0, buf);
    switch (hungry_state)
    {
	when 0: ;
	when 1:
	    waddstr(cw, JSTR("  空腹", "  Hungry"));
	when 2:
	    waddstr(cw, JSTR("  飢餓", "  Weak"));
	when 3:
	    waddstr(cw, JSTR("  瀕死", "  Fainting"));
    }
    wclrtoeol(cw);
    s_hungry = hungry_state;
    s_lang = jrogue_lang_is_japanese;
    wmove(cw, oy, ox);
}

/*
 * wait_for
 *	Sit around until the guy types the right key
 */

/*
 * Wait on the window which is actually being displayed.  Calling readchar()
 * while an inventory, help or options window is visible reads from cw and
 * refreshes the game window, moving the visible cursor to its old position.
 */
void wait_for_window(WINDOW *win, char ch)
{
    int c;

    for (;;)
    {
        c = wgetch(win);
        if (c == ERR)
            continue;
        if (ch == '\n' ? (c == '\n' || c == '\r') : (c == ch))
            break;
    }
}

void wait_for(char ch)
{
    wait_for_window(cw, ch);
}

/*
 * show_win:
 *	function used to display a window and wait before returning
 */

void show_win(WINDOW *scr, char *message)
{
    mvwaddstr(scr, 0, 0, message);
    touchwin(scr);
    wmove(scr, hero.y, hero.x);
    draw(scr);
    wait_for_window(scr, ' ');
    clearok(cw, TRUE);
    touchwin(cw);
}
