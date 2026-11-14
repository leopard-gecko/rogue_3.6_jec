#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include "language.h"
#ifdef _WIN32
#include "windows/rogue_windows.h"
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

int jrogue_lang_is_japanese = DEFAULT_JAPANESE;


static int
contains_utf8(const char *s)
{
    const char *p;

    if (s == NULL)
	return FALSE;

    for (p = s; *p != '\0'; p++)
    {
	if ((p[0] == 'u' || p[0] == 'U') &&
	    (p[1] == 't' || p[1] == 'T') &&
	    (p[2] == 'f' || p[2] == 'F'))
	{
	    if (p[3] == '-' && p[4] == '8')
		return TRUE;
	    if (p[3] == '8')
		return TRUE;
	}
    }
    return FALSE;
}

void
rogue_init_locale(void)
{
#ifdef _WIN32
    /* Match Rogue 5.4.5's Windows UCRT locale spelling.  Unlike
     * POSIX locale names, .UTF8 is accepted by the Microsoft CRT. */
    if (setlocale(LC_ALL, ".UTF8") == NULL)
        (void)setlocale(LC_ALL, "");
#else
    setlocale(LC_ALL, "");
#endif
}

void
rogue_ensure_utf8_locale(void)
{
    const char *loc;

    if (!jrogue_lang_is_japanese)
	return;

#ifdef _WIN32
    /* Use the same Unicode locale as the 5.4.5 Windows build. */
    if (setlocale(LC_CTYPE, ".UTF8") != NULL)
        return;
#endif
    loc = setlocale(LC_CTYPE, NULL);
    if (contains_utf8(loc))
	return;

    loc = setlocale(LC_ALL, "C.UTF-8");
    if (contains_utf8(loc))
	return;

    loc = setlocale(LC_ALL, "ja_JP.UTF-8");
    if (contains_utf8(loc))
	return;

    loc = setlocale(LC_ALL, "en_US.UTF-8");
    if (contains_utf8(loc))
	return;

    (void) setlocale(LC_ALL, "");
}

/* Locale is consulted only when the selected startup mode is auto. */
static void
jrogue_detect_locale_language(void)
{
#ifndef _WIN32
    const char *lang;
#endif

#ifdef _WIN32
    /* Same as Rogue 5.4.5jec: use the Windows UI language, not MSYS2 LANG. */
    jrogue_lang_is_japanese = rogue_windows_ui_is_japanese();
#else
    lang = getenv("LC_ALL");
    if (lang == NULL || *lang == '\0')
        lang = getenv("LC_MESSAGES");
    if (lang == NULL || *lang == '\0')
        lang = getenv("LANG");
    jrogue_lang_is_japanese = lang != NULL &&
        (lang[0] == 'j' || lang[0] == 'J') &&
        (lang[1] == 'a' || lang[1] == 'A');
#endif
}

/* A ROGUEOPTS value may be followed by the next comma-separated option. */
static int
language_token_equals(const char *value, const char *expected)
{
    if (value == NULL)
        return FALSE;

    while (*expected != '\0')
    {
        if (*value == '\0' || *value == ',' ||
            tolower((unsigned char)*value) != (unsigned char)*expected)
            return FALSE;
        value++;
        expected++;
    }
    return *value == '\0' || *value == ',';
}

void
jrogue_detect_language(void)
{
#if DEFAULT_LANGUAGE == 1
    jrogue_lang_is_japanese = TRUE;
#elif DEFAULT_LANGUAGE == 2
    jrogue_lang_is_japanese = FALSE;
#else
    jrogue_detect_locale_language();
#endif
}

const char *
jrogue_str(const char *ja, const char *en)
{
    return jrogue_lang_is_japanese ? ja : en;
}

int
jrogue_chr(int ja, int en)
{
    return jrogue_lang_is_japanese ? ja : en;
}

const char *
jrogue_language_code(void)
{
    return jrogue_lang_is_japanese ? "ja" : "en";
}

int
jrogue_set_language(const char *lang)
{
    if (language_token_equals(lang, "ja"))
        jrogue_lang_is_japanese = TRUE;
    else if (language_token_equals(lang, "en"))
        jrogue_lang_is_japanese = FALSE;
    else if (language_token_equals(lang, "auto"))
        jrogue_detect_locale_language();
    else
        return FALSE;

    /* auto is resolved now; gameplay only ever uses Japanese or English. */
    rogue_ensure_utf8_locale();
    return TRUE;
}
