/* Platform-specific rendering API; leave VT glyph bookkeeping in
 * unicode_output.c and ordinary Unix curses drawing unchanged. */
#ifndef ROGUE36_UNICODE_OUTPUT_H
#define ROGUE36_UNICODE_OUTPUT_H
#include <curses.h>
#ifdef _WIN32
int rogue_waddstr_utf8(WINDOW *win, const char *text);
int rogue_draw_utf8(WINDOW *win);
int rogue_refresh_utf8(void);
void rogue_vt_repaint(WINDOW *win);
void rogue_vt_clear_window(WINDOW *win);
void rogue_vt_clear_row(WINDOW *win, int row);
#else
#define rogue_waddstr_utf8(win, text) waddstr((win), (text))
#define rogue_vt_clear_window(win) ((void)0)
#define rogue_vt_clear_row(win, row) ((void)0)
#endif
#endif /* ROGUE36_UNICODE_OUTPUT_H */
