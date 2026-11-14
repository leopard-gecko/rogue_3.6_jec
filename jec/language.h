#ifndef JROGUE_LANGUAGE_H
#define JROGUE_LANGUAGE_H

/*
 * Runtime language: Japanese or English (selected from the options screen).
 * Startup precedence:
 *   1. ROGUEOPTS language=ja/en/auto (when supplied)
 *   2. Build-time DEFAULT_LANGUAGE (0=auto, 1=ja, 2=en)
 *   3. OS locale LC_ALL, LC_MESSAGES, LANG (only for auto)
 *
 * "auto" resolves the initial language once; it is not a gameplay mode.
 */
#ifndef DEFAULT_LANGUAGE
#define DEFAULT_LANGUAGE 0
#endif

/*
 * DEFAULT_JAPANESE is derived internally from DEFAULT_LANGUAGE.
 * External build rules should define DEFAULT_LANGUAGE only; deriving this
 * helper here avoids inconsistent language defaults in static initializers.
 */
#ifdef DEFAULT_JAPANESE
#undef DEFAULT_JAPANESE
#endif
#if DEFAULT_LANGUAGE == 1
#define DEFAULT_JAPANESE 1
#else
#define DEFAULT_JAPANESE 0
#endif

extern int jrogue_lang_is_japanese;

void jrogue_detect_language(void);
void jrogue_apply_language(void);
void jrogue_apply_options_language(void);
void jrogue_apply_rip_language(void);
const char *jrogue_str(const char *ja, const char *en);
int jrogue_chr(int ja, int en);
const char *jrogue_language_code(void);
int jrogue_set_language(const char *lang);

/* Use these in normal code. */
#define JSTR(ja, en) jrogue_str((ja), (en))
#define JCHR(ja, en) jrogue_chr((ja), (en))

/*
 * Use these only in static initializers, where C requires compile-time
 * constant expressions.
 */
#if DEFAULT_JAPANESE
# define JSTR_STATIC(ja, en) ja
# define JCHR_STATIC(ja, en) ja
#else
# define JSTR_STATIC(ja, en) en
# define JCHR_STATIC(ja, en) en
#endif

#endif /* JROGUE_LANGUAGE_H */
