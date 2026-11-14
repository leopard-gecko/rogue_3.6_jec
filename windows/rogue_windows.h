#ifndef ROGUE36_WINDOWS_H
#define ROGUE36_WINDOWS_H
#ifdef _WIN32
#include <wchar.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "rogue_windows_console.h"
/* On an Explorer-style launch without arguments, show the keyboard menu
 * only if the default save file exists. Enter (like R) resumes that save.
 * Return 0 to quit, 1 for a new game, 2 for a selected save file.
 * Caller owns *selected_file (allocated UTF-8), if non-NULL. */
int rogue_windows_launch_menu(const char *default_save, int japanese,
                              char **selected_file);
/* Retrieve the last Windows command-line argument as UTF-8 for Unicode
 * save paths passed by drag-and-drop or by a command shell. Caller frees. */
char *rogue_windows_last_argument_utf8(void);
/* Unicode-aware save-file access (also used by the normal save command). */
FILE *rogue_windows_fopen_utf8(const char *path, const char *mode);
int rogue_windows_unlink_utf8(const char *path);
int rogue_windows_file_exists_utf8(const char *path);
time_t rogue_windows_file_mtime_utf8(const char *path);
void rogue_windows_get_home(char *dst, size_t size);
void rogue_windows_get_username(char *dst, size_t size);
char *rogue_windows_getenv_utf8(const char *name);
int rogue_windows_ui_is_japanese(void);
wchar_t *rogue_windows_utf8_to_wide(const char *s);
char *rogue_windows_crypt(const char *key, const char *salt);
int rogue_platform_pid(void);
#endif
#endif
