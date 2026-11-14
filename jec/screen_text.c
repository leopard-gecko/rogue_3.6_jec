/* Shared curses message-row preparation.  All rendering, including
 * Windows VT supplementary glyph support, stays in unicode_output.c.
 * Calling code owns the final draw()/refresh() timing. */
#include "screen_text.h"
#include "unicode_output.h"

void rogue_screen_prepare_text_row(WINDOW *win, int row)
{
    rogue_vt_clear_row(win, row);
    wmove(win, row, 0);
    wclrtoeol(win);
    wmove(win, row, 0);
}
