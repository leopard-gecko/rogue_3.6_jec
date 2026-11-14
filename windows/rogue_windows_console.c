/* Windows console input, direct-launch sizing, and plain-screen handling.
 * Moved from rogue_windows.c without changing their Windows behavior.
 */
#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* The Windows SDK and PDCurses both define MOUSE_MOVED. */
#ifdef MOUSE_MOVED
#undef MOUSE_MOVED
#endif
#include "rogue_windows_console.h"

void rogue_windows_init_console(void)
{
    /* PDCursesMod uses Unicode input/output; keep CRT text printf in UTF-8. */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

/* No universal Win32 flag identifies an Explorer double-click. Use a
 * conservative parent/console classifier; never resize an interactive shell.
 * Unknown parents may be accepted only for a sole console client (a common
 * Windows Terminal handoff). */
static int rogue_windows_is_shell_name(const wchar_t *name)
{
    return lstrcmpiW(name, L"cmd.exe") == 0 ||
           lstrcmpiW(name, L"powershell.exe") == 0 ||
           lstrcmpiW(name, L"pwsh.exe") == 0 ||
           lstrcmpiW(name, L"bash.exe") == 0 ||
           lstrcmpiW(name, L"sh.exe") == 0 ||
           lstrcmpiW(name, L"mintty.exe") == 0 ||
           lstrcmpiW(name, L"wsl.exe") == 0 ||
           lstrcmpiW(name, L"zsh.exe") == 0 ||
           lstrcmpiW(name, L"fish.exe") == 0 ||
           lstrcmpiW(name, L"nu.exe") == 0;
}

/* 1 = known direct parent, -1 = interactive shell, 0 = unknown. */
static int rogue_windows_launch_parent_kind(void)
{
    HANDLE snap;
    PROCESSENTRY32W entry;
    DWORD my_pid = GetCurrentProcessId();
    DWORD parent_pid = 0;
    int kind = 0;

    snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return 0;
    memset(&entry, 0, sizeof entry);
    entry.dwSize = sizeof entry;
    if (Process32FirstW(snap, &entry))
    {
        do
        {
            if (entry.th32ProcessID == my_pid)
            {
                parent_pid = entry.th32ParentProcessID;
                break;
            }
        } while (Process32NextW(snap, &entry));
    }
    memset(&entry, 0, sizeof entry);
    entry.dwSize = sizeof entry;
    if (parent_pid && Process32FirstW(snap, &entry))
    {
        do
        {
            const wchar_t *name;
            if (entry.th32ProcessID != parent_pid)
                continue;
            name = entry.szExeFile;
            if (lstrcmpiW(name, L"explorer.exe") == 0 ||
                lstrcmpiW(name, L"WindowsTerminal.exe") == 0 ||
                lstrcmpiW(name, L"WindowsTerminalPreview.exe") == 0 ||
                lstrcmpiW(name, L"OpenConsole.exe") == 0 ||
                lstrcmpiW(name, L"conhost.exe") == 0 ||
                lstrcmpiW(name, L"wt.exe") == 0)
                kind = 1;
            else if (rogue_windows_is_shell_name(name))
                kind = -1;
            break;
        } while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return kind;
}

static int rogue_windows_is_only_console_client(void)
{
    DWORD pids[16];
    DWORD count, i;
    DWORD my_pid = GetCurrentProcessId();
    int found = 0;

    count = GetConsoleProcessList(pids, 16);
    if (count == 0 || count > 16)
        return 0;
    for (i = 0; i < count; i++)
    {
        if (pids[i] == my_pid)
            found = 1;
        else if (pids[i] != 0)
            return 0;
    }
    return found;
}

/* A new ConPTY session can contain host-side helper processes in addition
 * to Rogue.  Requiring GetConsoleProcessList() == {our PID} can therefore
 * suppress the resize even when the user started Rogue directly in Explorer.
 * For a pseudoconsole, trust the known direct-launch parent, but do not
 * resize if another *interactive shell* is sharing this console.  An
 * unrecognized process name alone is not proof of an interactive shell. */
static int rogue_windows_console_has_shell(void)
{
    DWORD pids[64], count, i;
    HANDLE snap;
    PROCESSENTRY32W entry;
    int found_shell = 0;

    count = GetConsoleProcessList(pids, 64);
    if (count == 0 || count > 64)
        return 0;
    snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return 0;
    memset(&entry, 0, sizeof entry);
    entry.dwSize = sizeof entry;
    if (Process32FirstW(snap, &entry))
    {
        do
        {
            for (i = 0; i < count; i++)
            {
                if (entry.th32ProcessID != pids[i] ||
                    entry.th32ProcessID == GetCurrentProcessId())
                    continue;
                if (rogue_windows_is_shell_name(entry.szExeFile))
                {
                    found_shell = 1;
                    break;
                }
            }
            if (found_shell)
                break;
        } while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return found_shell;
}

/* Remember a safe direct-launch decision, so a delayed VT retry does not need to
 * rediscover parent PIDs which may have exited during startup. */
static int rogue_windows_retry_direct_vt_resize = 0;

/* For a visible classic Console Host window, resize both the viewport and
 * buffer. First shrink the viewport so the target 80x24 buffer is legal,
 * then expand the viewport again. Roll back on failure where possible. */
static void rogue_windows_resize_console_host(HANDLE output)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD largest, target;
    SMALL_RECT interim, final_rect;
    SHORT width, height;

    largest = GetLargestConsoleWindowSize(output);
    if (largest.X < 80 || largest.Y < 24 ||
        !GetConsoleScreenBufferInfo(output, &info))
        return;

    width = (SHORT)(info.srWindow.Right - info.srWindow.Left + 1);
    height = (SHORT)(info.srWindow.Bottom - info.srWindow.Top + 1);
    if (width <= 0 || height <= 0)
        return;
    if (info.dwSize.X == 80 && info.dwSize.Y == 24 &&
        width == 80 && height == 24)
        return;

    interim.Left = interim.Top = 0;
    interim.Right = (SHORT)((width < 80 ? width : 80) - 1);
    interim.Bottom = (SHORT)((height < 24 ? height : 24) - 1);
    if (!SetConsoleWindowInfo(output, TRUE, &interim))
        return;

    target.X = 80;
    target.Y = 24;
    if (!SetConsoleScreenBufferSize(output, target))
    {
        (void)SetConsoleWindowInfo(output, TRUE, &info.srWindow);
        return;
    }

    final_rect.Left = final_rect.Top = 0;
    final_rect.Right = 79;
    final_rect.Bottom = 23;
    if (!SetConsoleWindowInfo(output, TRUE, &final_rect))
    {
        (void)SetConsoleScreenBufferSize(output, info.dwSize);
        (void)SetConsoleWindowInfo(output, TRUE, &info.srWindow);
    }
}

/* GetConsoleWindow()/IsWindowVisible() do not identify the real terminal
 * frontend reliably. ConPTY can provide a proxy HWND; a default-terminal
 * Explorer launch can also have WT_SESSION unset. Send CSI 8;24;80t to any
 * directly launched console, including those with a visible console HWND.
 * This is harmless when the classic host has already been resized via API;
 * if the host does not understand it, the Win32 resize remains the fallback. */
static void rogue_windows_request_vt_resize(HANDLE output, DWORD mode)
{
    static const char request[] = "\033[8;24;80t";
    DWORD written = 0;

    /* Microsoft requires ENABLE_PROCESSED_OUTPUT together with VT parsing.
     * WriteFile passes the request to the ConPTY output stream. */
    if ((mode & (ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        != (ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
    {
        if (!SetConsoleMode(output, mode | ENABLE_PROCESSED_OUTPUT |
                           ENABLE_VIRTUAL_TERMINAL_PROCESSING))
            return;
    }
    (void)WriteFile(output, request, (DWORD)(sizeof request - 1),
                    &written, NULL);
}

void rogue_windows_resize_direct_launch(void)
{
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    HWND window;
    int parent_kind, sole_client, has_shell;

    if (output == NULL || output == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(output, &mode))
    {
        return;
    }
    parent_kind = rogue_windows_launch_parent_kind();
    /* A shell parent must NEVER resize, even if it has just exited or
     * the PIDs reported by a pseudoconsole are incomplete. */
    if (parent_kind < 0)
    {
        return;
    }
    sole_client = rogue_windows_is_only_console_client();
    has_shell = rogue_windows_console_has_shell();
    if (has_shell || (parent_kind == 0 && !sole_client))
    {
        return;
    }
    window = GetConsoleWindow();
    /* Preserve working classic Console Host behavior for a newly created
     * console. A visible GetConsoleWindow() is NOT proof of that frontend. */
    if (window != NULL && IsWindowVisible(window) && sole_client)
    {
        rogue_windows_resize_console_host(output);
    }

    /* HWND visibility and WT_SESSION do not identify the frontend: a
     * Windows Terminal direct launch may report a visible HWND with no
     * WT_SESSION. Always send the VT request once classified as safe. */
    rogue_windows_retry_direct_vt_resize = 1;
    rogue_windows_request_vt_resize(output, mode);
}

/* Windows Terminal can hand off the pseudoconsole before its UI is ready
 * for window-management sequences. Retry when the normal startup delay
 * has elapsed, or briefly after startup in the save-file restore path.
 * Never run this for command launches in an existing shell. */
void rogue_windows_retry_direct_launch_resize(int restoring)
{
    HANDLE output;
    DWORD mode;

    if (!rogue_windows_retry_direct_vt_resize)
        return;
    if (restoring)
        Sleep(350);
    output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output == NULL || output == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(output, &mode))
        return;
    rogue_windows_request_vt_resize(output, mode);
}


/* Use the cached direct-launch result without rechecking parent PIDs.
 * The launch menu and score handling must share this same decision. */
int rogue_windows_is_direct_launch(void)
{
    return rogue_windows_retry_direct_vt_resize != 0;
}

void rogue_windows_sleep(int ms)
{
    Sleep((DWORD)ms);
}

/* The VT backend can run with two completely different stdin types.
 * From Explorer/Windows Terminal, GetConsoleMode succeeds and we receive
 * UTF-16 KEY_EVENT_RECORD entries.  From MSYS2's mintty, stdin is normally
 * a pipe whose bytes are already UTF-8.  _getwch() / the VT curses decoder
 * may lose emoji on the latter route.  Keep these transports separate and
 * produce a single Unicode scalar for each character in either case. */
int rogue_windows_has_edit_input(void)
{
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    return h != NULL && h != INVALID_HANDLE_VALUE &&
           (GetConsoleMode(h, &mode) || GetFileType(h) == FILE_TYPE_PIPE);
}

/* Read a single byte from a mintty pipe.  The timeout is used only for
 * multi-byte/VT sequences so Escape by itself does not block forever. */
static int rogue_pipe_byte(HANDLE h, int timeout_ms)
{
    unsigned char b;
    DWORD got = 0, available = 0;
    int elapsed;

    if (timeout_ms < 0)
        return (ReadFile(h, &b, 1, &got, NULL) && got == 1) ? b : -1;
    for (elapsed = 0; elapsed <= timeout_ms; elapsed += 5)
    {
        if (!PeekNamedPipe(h, NULL, 0, NULL, &available, NULL))
            return -1;
        if (available)
            return (ReadFile(h, &b, 1, &got, NULL) && got == 1) ? b : -1;
        if (elapsed != timeout_ms)
            Sleep(5);
    }
    return -1;
}

/* Convert UTF-8 from a mintty pipe, including 4-byte characters such as
 * U+1F34E.  All bytes belong to the same transport; do not mix with
 * wgetch() or _getwch() for continuation bytes. */
static int rogue_pipe_edit_key(HANDLE h)
{
    int c, d, need, i;
    unsigned int cp, min_cp;

    for (;;)
    {
        c = rogue_pipe_byte(h, -1);
        if (c < 0)
            return -1;

        if (c == 27)
        {
            int first = rogue_pipe_byte(h, 65);
            if (first < 0)
                return 27;
            if (first == '[' || first == 'O')
            {
                int last = rogue_pipe_byte(h, 65);
                if (last == 'D') return KEY_LEFT;
                if (last == 'C') return KEY_RIGHT;
                if (last == '3')
                {
                    (void)rogue_pipe_byte(h, 65); /* '~' */
                    return KEY_DC;
                }
                /* Terminal bracketed paste: ESC[200~ and ESC[201~.
                 * Consume markers, but not their pasted UTF-8 payload. */
                if (first == '[' && last == '2')
                {
                    int a = rogue_pipe_byte(h, 65);
                    int b = rogue_pipe_byte(h, 65);
                    int e = rogue_pipe_byte(h, 65);
                    if (a == '0' && (b == '0' || b == '1') && e == '~')
                        continue;
                }
            }
            return 27;
        }
        if (c < 0x80)
            return c;
        if (c >= 0xc2 && c <= 0xdf)
            { need = 1; cp = (unsigned int)(c & 0x1f); min_cp = 0x80; }
        else if (c >= 0xe0 && c <= 0xef)
            { need = 2; cp = (unsigned int)(c & 0x0f); min_cp = 0x800; }
        else if (c >= 0xf0 && c <= 0xf4)
            { need = 3; cp = (unsigned int)(c & 7); min_cp = 0x10000; }
        else
            continue;
        for (i = 0; i < need; i++)
        {
            d = rogue_pipe_byte(h, 250);
            if (d < 0 || (d & 0xc0) != 0x80)
                break;
            cp = (cp << 6) | (unsigned int)(d & 0x3f);
        }
        if (i == need && cp >= min_cp && cp <= 0x10ffff &&
            (cp < 0xd800 || cp > 0xdfff))
        {
            return (int)cp;
        }
        /* Discard a malformed UTF-8 sequence instead of interpreting its
         * fragment as a completed option string. */
    }
}

/* Win32 console character events contain UTF-16.  Emoji insertion can
 * supply key-down and key-up separately for each surrogate half. */
static int rogue_console_edit_key(HANDLE h, DWORD mode)
{
    INPUT_RECORD record;
    DWORD got;
    static unsigned int high_surrogate = 0;
    /* Avoid decoding the key-up duplicate after a surrogate key-down. */
    static unsigned int down_high_surrogate = 0;
    static unsigned int down_low_surrogate = 0;
    unsigned int c;
    WORD vk;
    int down, surrogate;
    int vt_input = (mode & ENABLE_VIRTUAL_TERMINAL_INPUT) != 0;

    for (;;)
    {
        if (!ReadConsoleInputW(h, &record, 1, &got) || got != 1)
            return -1;
        if (record.EventType != KEY_EVENT)
            continue;
        down = record.Event.KeyEvent.bKeyDown != 0;
        c = (unsigned int)record.Event.KeyEvent.uChar.UnicodeChar;
        vk = record.Event.KeyEvent.wVirtualKeyCode;
        surrogate = c >= 0xd800 && c <= 0xdfff;

        /* mintty via ConPTY can expose the two UTF-16 code units of an
         * emoji ONLY on key-up records (observed with VT mode 0x200).
         * Process those units in VT-input mode, but never treat a released
         * Enter or other ordinary key as input.  Native Windows console
         * behavior is unchanged. */
        if (!down)
        {
            if (!vt_input || !surrogate)
                continue;
            /* A regular key-down/key-up pair represents just one unit. */
            if (c >= 0xd800 && c <= 0xdbff && down_high_surrogate == c)
            {
                down_high_surrogate = 0;
                continue;
            }
            if (c >= 0xdc00 && c <= 0xdfff && down_low_surrogate == c)
            {
                down_low_surrogate = 0;
                continue;
            }
        }
        else if (c >= 0xd800 && c <= 0xdbff)
            down_high_surrogate = c;
        else if (c >= 0xdc00 && c <= 0xdfff)
            down_low_surrogate = c;
        if (!c)
        {
            /* ConPTY injects zero-character key events between UTF-16
             * surrogate halves. Ignore these without losing the first half. */
            switch (vk)
            {
                case VK_LEFT: high_surrogate = 0; return KEY_LEFT;
                case VK_RIGHT: high_surrogate = 0; return KEY_RIGHT;
                case VK_DELETE: high_surrogate = 0; return KEY_DC;
                case VK_BACK: high_surrogate = 0; return KEY_BACKSPACE;
                case VK_RETURN: high_surrogate = 0; return 13;
                case VK_ESCAPE: high_surrogate = 0; return 27;
                default: continue;
            }
        }
        if (high_surrogate)
        {
            unsigned int high = high_surrogate;
            high_surrogate = 0;
            if (c >= 0xdc00 && c <= 0xdfff)
            {
                unsigned int cp = 0x10000u + ((high - 0xd800u) << 10)
                                  + (c - 0xdc00u);
                return (int)cp;
            }
        }
        if (c >= 0xd800 && c <= 0xdbff)
        {
            high_surrogate = c;
            continue;
        }
        if (c >= 0xdc00 && c <= 0xdfff)
            continue;
        return (int)c;
    }
}

int rogue_windows_edit_key(void)
{
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (h == NULL || h == INVALID_HANDLE_VALUE)
        return -1;
    if (GetConsoleMode(h, &mode))
    {
        return rogue_console_edit_key(h, mode);
    }
    if (GetFileType(h) == FILE_TYPE_PIPE)
    {
        return rogue_pipe_edit_key(h);
    }
    return -1;
}

/* Clear a direct-launch console outside curses, both for the launch
 * menu/save-file picker and before printing the final score list.
 * endwin() does not necessarily erase the dungeon image.  Use the cached
 * Explorer-style launch classification: do not clear a user's existing
 * Command Prompt, PowerShell or MSYS2 terminal.
 *
 * VT CSI 2J / CSI H works for Windows Terminal (including ConPTY with
 * WT_SESSION absent) and modern Console Host. Fall back to Win32 buffer
 * operations for older Console Host implementations without VT support.
 * Only use this before curses starts or after endwin(), never while
 * the curses screen is active. */
void rogue_windows_clear_score_screen(void)
{
    static const char erase_and_home[] = "\033[2J\033[H";
    HANDLE output;
    DWORD mode, written;
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD home;
    DWORD cells;

    if (!rogue_windows_retry_direct_vt_resize)
        return;
    output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output == NULL || output == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(output, &mode))
        return;

    /* Do not let any buffered score text appear before the erase. */
    fflush(stdout);
    if (SetConsoleMode(output, mode | ENABLE_PROCESSED_OUTPUT |
                              ENABLE_VIRTUAL_TERMINAL_PROCESSING) &&
        WriteFile(output, erase_and_home, (DWORD)(sizeof erase_and_home - 1),
                  &written, NULL) &&
        written == (DWORD)(sizeof erase_and_home - 1))
        return;

    if (!GetConsoleScreenBufferInfo(output, &info))
        return;
    home.X = 0;
    home.Y = 0;
    cells = (DWORD)info.dwSize.X * (DWORD)info.dwSize.Y;
    if (cells == 0 ||
        !FillConsoleOutputCharacterW(output, L' ', cells, home, &written))
        return;
    (void)FillConsoleOutputAttribute(output, info.wAttributes,
                                     cells, home, &written);
    (void)SetConsoleCursorPosition(output, home);
}


/* The score list is printed after endwin(), so curses input is no longer
 * active. Keep an Explorer-started console open until Enter, but do not
 * interfere with shells that launched Rogue as a command. Reuse the direct-
 * launch classification captured during startup; do not classify again at
 * exit, when the parent may already be gone.
 *
 * The existing Unicode console input reader handles both Windows Terminal
 * (ConPTY) and classic Console Host, and ignores key-release events. */
void rogue_windows_wait_for_score_exit(const char *prompt)
{
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    int ch;

    if (!rogue_windows_retry_direct_vt_resize ||
        input == NULL || input == INVALID_HANDLE_VALUE ||
        output == NULL || output == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(input, &mode) || !GetConsoleMode(output, &mode))
        return;

    /* Discard any keystrokes left over from the tombstone or quit prompt. */
    (void)FlushConsoleInputBuffer(input);
    printf("\n%s", prompt);
    fflush(stdout);
    do
    {
        ch = rogue_windows_edit_key();
    } while (ch >= 0 && ch != '\r' && ch != '\n');
}

#endif
