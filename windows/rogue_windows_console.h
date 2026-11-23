/* Windows-only terminal and keyboard services.
 * The original declarations remain reachable through rogue_windows.h.
 */
#ifndef ROGUE36_WINDOWS_CONSOLE_H
#define ROGUE36_WINDOWS_CONSOLE_H
#ifdef _WIN32
#include <curses.h>
/* Cache the classification once; commands started in a shell are unchanged. */
void rogue_windows_init_console(void);
void rogue_windows_restore_cursor_shape(void);
void rogue_windows_resume_cursor_from_shell(void);
void rogue_windows_cursor_after_redraw(void);
void rogue_windows_resize_direct_launch(void);
void rogue_windows_retry_direct_launch_resize(int restoring);
int rogue_windows_is_direct_launch(void);
void rogue_windows_clear_score_screen(void);
void rogue_windows_wait_for_score_exit(const char *prompt);
void rogue_windows_sleep(int ms);
int rogue_windows_edit_key(void);
int rogue_windows_has_edit_input(void);
#endif
#endif /* ROGUE36_WINDOWS_CONSOLE_H */
