/* Windows-native support for Rogue 3.6. */
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <io.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* The Windows SDK and PDCurses both define MOUSE_MOVED.
 * Keep PDCurses' definition for curses after including windows.h.
 * This matches the Windows header handling in Rogue 5.4.5.
 */
#ifdef MOUSE_MOVED
#undef MOUSE_MOVED
#endif
#include "rogue_windows.h"

static char *utf16_to_utf8(const wchar_t *wide)
{
    int n;
    char *str;
    if (wide == NULL) return NULL;
    n = WideCharToMultiByte(CP_UTF8, 0, wide, -1, NULL, 0, NULL, NULL);
    if (n <= 0) return NULL;
    str = (char *)malloc((size_t)n);
    if (str == NULL) return NULL;
    if (WideCharToMultiByte(CP_UTF8, 0, wide, -1, str, n, NULL, NULL) != n)
    {
        free(str);
        return NULL;
    }
    return str;
}

wchar_t *rogue_windows_utf8_to_wide(const char *str)
{
    int n;
    wchar_t *buf;
    if (str == NULL) return NULL;
    n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, -1, NULL, 0);
    if (n <= 0) return NULL;
    buf = (wchar_t *)malloc((size_t)n * sizeof *buf);
    if (buf == NULL) return NULL;
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str, -1, buf, n) != n)
    {
        free(buf);
        return NULL;
    }
    return buf;
}

char *rogue_windows_getenv_utf8(const char *name)
{
    wchar_t *wname, *value;
    char *utf8;
    DWORD need, got;
    wname = rogue_windows_utf8_to_wide(name);
    if (wname == NULL) return NULL;
    need = GetEnvironmentVariableW(wname, NULL, 0);
    if (need == 0 || need > 32767) { free(wname); return NULL; }
    value = (wchar_t *)malloc((size_t)need * sizeof *value);
    if (value == NULL) { free(wname); return NULL; }
    got = GetEnvironmentVariableW(wname, value, need);
    free(wname);
    if (got == 0 || got >= need) { free(value); return NULL; }
    utf8 = utf16_to_utf8(value);
    free(value);
    return utf8;
}

static void copy_utf8(char *dst, size_t size, const char *src)
{
    if (size == 0) return;
    if (src == NULL) src = "";
    snprintf(dst, size, "%s", src);
}

void rogue_windows_get_home(char *dst, size_t size)
{
    char *home = rogue_windows_getenv_utf8("USERPROFILE");
    if (home == NULL) home = rogue_windows_getenv_utf8("HOME");
    copy_utf8(dst, size, home != NULL ? home : ".");
    free(home);
}

void rogue_windows_get_username(char *dst, size_t size)
{
    DWORD n = 256;
    wchar_t buf[256];
    char *utf8 = NULL;
    if (GetUserNameW(buf, &n)) utf8 = utf16_to_utf8(buf);
    copy_utf8(dst, size, utf8 != NULL ? utf8 : "rogue");
    free(utf8);
}

int rogue_windows_ui_is_japanese(void)
{
    return PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_JAPANESE;
}

/* Explorer drag-and-drop passes the dropped file as a command-line
 * argument. Read the original Unicode command line instead of trusting
 * the CRT narrow argv (which may replace Japanese path characters with ?).
 * Only the final argument is relevant to Rogue's single-save-file syntax.
 * The caller owns the returned UTF-8 string. */
char *rogue_windows_last_argument_utf8(void)
{
    LPWSTR *args;
    int count;
    char *last = NULL;

    args = CommandLineToArgvW(GetCommandLineW(), &count);
    if (args == NULL)
        return NULL;
    if (count >= 2)
        last = utf16_to_utf8(args[count - 1]);
    LocalFree(args);
    return last;
}

/* Keep Windows paths as UTF-8 within Rogue and convert at the CRT boundary.
 * The common file picker returns UTF-16, while fopen/stat/unlink in the
 * previous port used the active ANSI code page and could not handle all
 * Japanese paths. These helpers cover both menu restores and saves. */
FILE *rogue_windows_fopen_utf8(const char *path, const char *mode)
{
    FILE *fp = NULL;
    wchar_t *wpath = rogue_windows_utf8_to_wide(path);
    wchar_t *wmode = rogue_windows_utf8_to_wide(mode);
    if (wpath != NULL && wmode != NULL)
        fp = _wfopen(wpath, wmode);
    free(wpath);
    free(wmode);
    return fp;
}

int rogue_windows_unlink_utf8(const char *path)
{
    int result = -1;
    wchar_t *wide = rogue_windows_utf8_to_wide(path);
    if (wide != NULL)
        result = _wunlink(wide);
    free(wide);
    return result;
}

int rogue_windows_file_exists_utf8(const char *path)
{
    DWORD attr;
    wchar_t *wide = rogue_windows_utf8_to_wide(path);
    if (wide == NULL)
        return 0;
    attr = GetFileAttributesW(wide);
    free(wide);
    return attr != INVALID_FILE_ATTRIBUTES &&
           (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

time_t rogue_windows_file_mtime_utf8(const char *path)
{
    WIN32_FILE_ATTRIBUTE_DATA info;
    ULARGE_INTEGER ticks;
    wchar_t *wide = rogue_windows_utf8_to_wide(path);
    int ok = 0;
    if (wide != NULL)
        ok = GetFileAttributesExW(wide, GetFileExInfoStandard, &info) != 0;
    free(wide);
    if (!ok)
        return (time_t)0;
    ticks.LowPart = info.ftLastWriteTime.dwLowDateTime;
    ticks.HighPart = info.ftLastWriteTime.dwHighDateTime;
    if (ticks.QuadPart < 116444736000000000ULL)
        return (time_t)0;
    return (time_t)((ticks.QuadPart - 116444736000000000ULL) /
                    10000000ULL);
}

/* Return UTF-8 pathname on success, NULL when the user cancels. Use the
 * Windows common dialog only when F is explicitly selected: the game and
 * normal terminal launches remain text-mode. */
static char *rogue_windows_choose_save_file(const char *default_save,
                                            int *dialog_error)
{
    OPENFILENAMEW ofn;
    wchar_t *buffer, *initial_dir;
    wchar_t *sep, *p;
    char *result = NULL;
    const DWORD max_chars = 32768;
    static const wchar_t filter[] =
        L"Rogue save files (*.save)\0*.save\0All files (*.*)\0*.*\0";

    *dialog_error = 0;
    buffer = (wchar_t *)calloc(max_chars, sizeof *buffer);
    if (buffer == NULL)
    {
        *dialog_error = 1;
        return NULL;
    }
    initial_dir = rogue_windows_utf8_to_wide(default_save);
    if (initial_dir != NULL)
    {
        sep = NULL;
        for (p = initial_dir; *p; p++)
            if (*p == L'\\' || *p == L'/')
                sep = p;
        if (sep != NULL)
            *sep = L'\0';
        else
            initial_dir[0] = L'\0';
    }

    memset(&ofn, 0, sizeof ofn);
    ofn.lStructSize = sizeof ofn;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = max_chars;
    if (initial_dir != NULL && initial_dir[0] != L'\0')
        ofn.lpstrInitialDir = initial_dir;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST |
                OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
    if (GetOpenFileNameW(&ofn))
    {
        result = utf16_to_utf8(buffer);
        if (result == NULL)
            *dialog_error = 1;
    }
    else if (CommDlgExtendedError() != 0)
        *dialog_error = 1;

    free(initial_dir);
    free(buffer);
    return result;
}

/* Only the direct-Explorer launch is interactive here. The decision is
 * cached by rogue_windows_resize_direct_launch() before entering this
 * function; never present a menu in an existing shell. */
int rogue_windows_launch_menu(const char *default_save, int japanese,
                              char **selected_file)
{
    int ch, has_default, dialog_error;
    char *chosen;
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;

    *selected_file = NULL;
    if (!rogue_windows_is_direct_launch() ||
        input == NULL || input == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(input, &mode))
        return 1;

    /* Windows Terminal's UI may still be opening at the start of a
     * double-click launch. Keep the delayed resize even if we skip the menu. */
    rogue_windows_retry_direct_launch_resize(1);

    /* Most players do not save. With no default save, launch immediately:
     * there is no reason to ask them to press N every time. A different
     * save file can still be restored by dropping it onto the EXE. */
    if (!rogue_windows_file_exists_utf8(default_save))
        return 1;
    for (;;)
    {
        has_default = rogue_windows_file_exists_utf8(default_save);
        rogue_windows_clear_score_screen();
        printf("\n                 Rogue 3.6\n\n");
        printf("%s\n\n", japanese ?
            (has_default ? "セーブした冒険が見つかりました。" :
                           "新しい冒険を始めるか、セーブファイルを選んでください。") :
            (has_default ? "A saved adventure was found." :
                           "Start a new adventure or select a save file."));
        if (has_default)
            printf("[R/Enter] %s\n", japanese ?
                "セーブした冒険を再開する" : "Resume saved adventure");
        printf("[N]       %s\n", japanese ?
            "新しい冒険を始める" : "Start a new adventure");
        printf("[F]       %s\n", japanese ?
            (has_default ? "別のセーブファイルを選ぶ" :
                           "セーブファイルを選ぶ") :
            "Choose another save file");
        printf("[Q]       %s\n\n", japanese ? "終了する" : "Quit");
        printf("%s", japanese ? "キーを押してください: " : "Press a key: ");
        fflush(stdout);

        ch = rogue_windows_edit_key();
        if (ch < 0)
            return 0;
        if (ch >= 'a' && ch <= 'z')
            ch = ch - 'a' + 'A';
        if (ch == 'Q' || ch == 27)
            return 0;
        if (ch == 'N')
        {
            (void)FlushConsoleInputBuffer(input);
            rogue_windows_clear_score_screen();
            return 1;
        }
        if ((ch == 'R' || ch == '\r' || ch == '\n') && has_default)
        {
            chosen = (char *)malloc(strlen(default_save) + 1);
            if (chosen == NULL)
                return -1;
            strcpy(chosen, default_save);
            *selected_file = chosen;
            (void)FlushConsoleInputBuffer(input);
            rogue_windows_clear_score_screen();
            return 2;
        }
        if (ch != 'F')
            continue;

        chosen = rogue_windows_choose_save_file(default_save, &dialog_error);
        if (chosen != NULL)
        {
            *selected_file = chosen;
            (void)FlushConsoleInputBuffer(input);
            rogue_windows_clear_score_screen();
            return 2;
        }
        if (dialog_error)
        {
            rogue_windows_clear_score_screen();
            printf("%s\n", japanese ?
                "ファイル選択画面を開けませんでした。" :
                "Could not open the file selection dialog.");
            printf("%s", japanese ?
                "何かキーを押すとメニューに戻ります。" :
                "Press any key to return to the menu.");
            fflush(stdout);
            if (rogue_windows_edit_key() < 0)
                return 0;
        }
        /* A cancelled file dialog returns to the menu without touching
         * any saved data. */
    }
}

int rogue_platform_pid(void)
{
    return _getpid();
}

/* DES crypt(3) implementation from the FreeSec xcrypt.c in the 5.4.5 tree. */
extern char *xcrypt(const char *, const char *);
char *rogue_windows_crypt(const char *key, const char *salt)
{
    return xcrypt(key, salt);
}
#endif
