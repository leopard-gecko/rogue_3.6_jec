/* UTF-8 output for the Windows PDCursesMod VT backend.
 *
 * PDCursesMod 4.5.4's VT renderer casts each Unicode scalar to wchar_t
 * before PDC_wc_to_utf8(). Windows wchar_t is 16 bits, so a non-BMP
 * code point U+1F34E is rendered as U+F34E. Even feeding waddch() a
 * 64-bit chtype therefore cannot render that glyph in this backend.
 *
 * Keep the correct character positions in the curses screen by writing
 * two spaces for a supplementary-plane glyph, and paint that UTF-8 glyph
 * directly via VT positioning sequences immediately after wrefresh().
 * This is only enabled with the Windows VT backend; other curses builds
 * continue to use ordinary curses rendering.
 */
#ifdef _WIN32
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jec/unicode_output.h"

#ifdef ROGUE_PDC_VT
#define ROGUE_VT_GLYPHS 256
struct rogue_vt_glyph {
    WINDOW *win;
    int y, x;
    char utf8[5];
};
static struct rogue_vt_glyph glyphs[ROGUE_VT_GLYPHS];
static size_t glyph_count;

void rogue_vt_clear_window(WINDOW *win)
{
    size_t i, dst;
    for (i = dst = 0; i < glyph_count; i++)
        if (glyphs[i].win != win)
            glyphs[dst++] = glyphs[i];
    glyph_count = dst;
}

void rogue_vt_clear_row(WINDOW *win, int row)
{
    size_t i, dst;
    for (i = dst = 0; i < glyph_count; i++)
        if (glyphs[i].win != win || glyphs[i].y != row)
            glyphs[dst++] = glyphs[i];
    glyph_count = dst;
}

static void rogue_vt_remember(WINDOW *win, int y, int x, const char *bytes)
{
    size_t i;
    /* Erase an older glyph occupying these cells, if any. */
    for (i = 0; i < glyph_count; )
        if (glyphs[i].win == win && glyphs[i].y == y &&
            glyphs[i].x >= x-1 && glyphs[i].x <= x+1)
        {
            memmove(glyphs+i, glyphs+i+1,
                    (glyph_count-i-1) * sizeof glyphs[0]);
            glyph_count--;
        }
        else
            i++;
    if (glyph_count < ROGUE_VT_GLYPHS)
    {
        struct rogue_vt_glyph *g = &glyphs[glyph_count++];
        g->win = win;
        g->y = y;
        g->x = x;
        memcpy(g->utf8, bytes, 4);
        g->utf8[4] = '\0';
    }
}

void rogue_vt_repaint(WINDOW *win)
{
    size_t i;
    int by, bx;
    int painted = 0;
    if (win == NULL)
        return;
    getbegyx(win, by, bx);
    for (i = 0; i < glyph_count; i++)
        if (glyphs[i].win == win)
        {
            if (!painted++)
                fputs("\033[s", stdout); /* save terminal cursor */
            fprintf(stdout, "\033[%d;%dH%s", by + glyphs[i].y + 1,
                    bx + glyphs[i].x + 1, glyphs[i].utf8);
        }
    if (painted)
    {
        fputs("\033[u", stdout); /* restore terminal cursor */
        fflush(stdout);
    }
}
#else
void rogue_vt_clear_window(WINDOW *win) { (void)win; }
void rogue_vt_clear_row(WINDOW *win, int row) { (void)win; (void)row; }
void rogue_vt_repaint(WINDOW *win) { (void)win; }
#endif

int rogue_draw_utf8(WINDOW *win)
{
    int result = wrefresh(win);
    if (result != ERR)
        rogue_vt_repaint(win);
    return result;
}

int rogue_refresh_utf8(void)
{
    return rogue_draw_utf8(stdscr);
}

int rogue_waddstr_utf8(WINDOW *win, const char *s)
{
    const unsigned char *p;
    size_t total, used;
    char *segment;
    int result;

    if (win == NULL || s == NULL)
        return ERR;
    total = strlen(s);
    segment = (char *)malloc(total + 1);
    if (segment == NULL)
        return waddstr(win, s);

    p = (const unsigned char *)s;
    used = 0;
    result = OK;
    while (*p != '\0')
    {
        unsigned int cp = 0;
        size_t offset = (size_t)(p - (const unsigned char *)s);
        if (p[0] >= 0xf0 && p[0] <= 0xf4 && offset + 4 <= total &&
            (p[1] & 0xc0) == 0x80 &&
            (p[2] & 0xc0) == 0x80 &&
            (p[3] & 0xc0) == 0x80 &&
            /* reject overlong UTF-8 and scalars beyond Unicode */
            (p[0] != 0xf0 || p[1] >= 0x90) &&
            (p[0] != 0xf4 || p[1] <= 0x8f))
        {
            cp = ((unsigned int)(p[0] & 7) << 18) |
                 ((unsigned int)(p[1] & 0x3f) << 12) |
                 ((unsigned int)(p[2] & 0x3f) << 6) |
                  (unsigned int)(p[3] & 0x3f);
        }
        if (cp >= 0x10000 && cp <= 0x10ffff)
        {
            if (used > 0)
            {
                segment[used] = '\0';
                if (waddstr(win, segment) == ERR)
                    result = ERR;
                used = 0;
            }
#ifdef ROGUE_PDC_VT
            {
                int y, x;
                getyx(win, y, x);
                if (waddstr(win, "  ") == ERR)
                    result = ERR;
                else
                    rogue_vt_remember(win, y, x, (const char *)p);
            }
#else
            if (waddch(win, (chtype)cp) == ERR)
                result = ERR;
#endif
            p += 4;
        }
        else
            segment[used++] = (char)*p++;
    }
    if (used > 0)
    {
        segment[used] = '\0';
        if (waddstr(win, segment) == ERR)
            result = ERR;
    }
    free(segment);
    return result;
}
#endif
