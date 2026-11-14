/*
 * Portable password input for systems where getpass() is obsolete or not
 * declared by the system headers.  This keeps the wizard password path
 * buildable without relying on an implicit declaration.
 */
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

char *
rogue_getpass(const char *prompt)
{
    static char buf[128];
#ifndef _WIN32
    struct termios oldt, newt;
    int have_tty;
    char *nl;
#else
    size_t i;
    int c;
#endif

    if (prompt != NULL) {
        fputs(prompt, stdout);
        fflush(stdout);
    }

#ifdef _WIN32
    i = 0;
    for (;;)
    {
        c = _getch();
        if (c == '\r' || c == '\n' || c == 27)
            break;
        if ((c == '\b' || c == 127) && i > 0)
            i--;
        else if (c >= 32 && c <= 126 && i + 1 < sizeof buf)
            buf[i++] = (char)c;
    }
    buf[i] = '\0';
    putchar('\n');
    return buf;
#else
    have_tty = (tcgetattr(STDIN_FILENO, &oldt) == 0);
    if (have_tty) {
        newt = oldt;
        newt.c_lflag &= (tcflag_t) ~ECHO;
        (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &newt);
    }

    if (fgets(buf, sizeof buf, stdin) == NULL)
        buf[0] = '\0';

    if (have_tty) {
        (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldt);
        putchar('\n');
    }

    nl = strchr(buf, '\n');
    if (nl != NULL)
        *nl = '\0';

    return buf;
#endif
}
