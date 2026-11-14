/* Small curses display helper shared by message and restore screens. */
#ifndef ROGUE36_SCREEN_TEXT_H
#define ROGUE36_SCREEN_TEXT_H
#include <curses.h>
void rogue_screen_prepare_text_row(WINDOW *win, int row);
#endif /* ROGUE36_SCREEN_TEXT_H */
