/*
 * Rogue
 * Exploring the dungeons of doom
 * Copyright (C) 1980 by Michael Toy and Glenn Wichman
 * All rights reserved
 *
 * @(#)main.c	3.27 (Berkeley) 6/15/81
 */

#include <curses.h>
#include <signal.h>
#ifndef _WIN32
#include <pwd.h>
#endif
#include "mach_dep.h"
#include "rogue.h"

#ifdef CHECKTIME
static int num_checks;		/* times we've gone over in checkout() */
#endif

int main(int argc, char **argv, char **envp)
{
    register char *env;
#ifndef _WIN32
    register struct passwd *pw;
#endif
    register struct linked_list *item;
    register struct object *obj;
    int lowtime;
    time_t now;
    char startup_message[sizeof whoami + 128];
#ifdef _WIN32
    char *menu_save = NULL;
    char *argument_save = NULL;
    int menu_action;
#endif

    rogue_init_locale();
#ifdef _WIN32
    rogue_windows_init_console();
    rogue_windows_resize_direct_launch();
#endif

    /*
     * Use the build-time default (or locale if auto). ROGUEOPTS language=
     * overrides this after the other startup options are parsed.
     */
    jrogue_detect_language();
    jrogue_apply_language();

    /*
     * get home and options from environment
     */
#ifdef _WIN32
    rogue_windows_get_home(home, sizeof home);
#else
    if ((env = getenv("HOME")) != NULL)
	snprintf(home, sizeof home, "%s/", env);
    else if ((pw = getpwuid(getuid())) != NULL)
	snprintf(home, sizeof home, "%s/", pw->pw_dir);
    else
	snprintf(home, sizeof home, "/");
#endif

    rogue_build_file_path(file_name, sizeof file_name, ROGUE_SAVEDIR, ROGUE_SAVEFILE);

#ifdef _WIN32
    env = rogue_windows_getenv_utf8("ROGUEOPTS");
#else
    env = getenv("ROGUEOPTS");
#endif
    if (env != NULL)
	parse_opts(env);
#ifdef _WIN32
    free(env);
#endif
    rogue_ensure_utf8_locale();

    /*
     * check for print-score option
     */
    if (argc == 2 && strcmp(argv[1], "-s") == 0)
    {
	waswizard = TRUE;
	score(0, -1, 0);
	exit(0);
    }
    /*
     * Check to see if he is a wizard
     */
    if (argc >= 2 && argv[1][0] == '\0')
	if (wizard_password_ok(rogue_getpass(JSTR("魔法使いの合言葉は？", "Wizard\'s password: "))))
	{
	    wizard = TRUE;
	    argv++;
	    argc--;
	}
    if (whoami[0] == '\0')
    {
#ifdef _WIN32
	rogue_windows_get_username(whoami, sizeof whoami);
#else
	if ((pw = getpwuid(getuid())) == NULL)
	{
	    printf("%s", JSTR("あなたはいったい誰ですか？\n", "Say, who the hell are you?\n"));
	    exit(1);
	}
	else
	    rogue_utf8_copy(whoami, sizeof whoami, pw->pw_name);
#endif
    }
    if (env == NULL || fruit[0] == '\0')
	rogue_utf8_copy(fruit, sizeof fruit, JSTR("ネバネバかび", "slime-mold"));

#if MAXLOAD|MAXUSERS
    if (too_much() && !wizard && !author())
    {
	printf(JSTR("すみません %s、今はシステムの負荷が高すぎます。\n", "Sorry, %s, but the system is too loaded now.\n"), whoami);
	printf(JSTR("後でもう一度試してください。待つあいだ、%s%sでもどうですか？\n", "Try again later.  Meanwhile, why not enjoy a%s %s?\n"),
	    jrogue_lang_is_japanese ? "" : vowelstr(fruit), fruit);
	exit(1);
    }
#endif
#ifdef _WIN32
    /* Only a direct Explorer-style launch without arguments gets the menu.
     * Shell launches and explicit save paths keep their existing behavior. */
    if (argc == 1)
    {
        menu_action = rogue_windows_launch_menu(file_name,
                                                 jrogue_lang_is_japanese,
                                                 &menu_save);
        if (menu_action == 0)
            return 0;
        if (menu_action < 0)
            return 1;
    }
    if (menu_save != NULL || argc == 2)
    {
        const char *save_path;
        if (menu_save == NULL)
            argument_save = rogue_windows_last_argument_utf8();
        save_path = menu_save != NULL ? menu_save :
                    argument_save != NULL ? argument_save : argv[1];
        rogue_windows_retry_direct_launch_resize(1);
        /* The direct-launch file picker and resize must not leave the
         * restored dungeon offset from the console's upper-left corner.
         * No reset is needed for a restore started from a normal shell. */
        rogue_windows_clear_score_screen();
        if (!restore((char *)save_path, envp))
        {
            rogue_windows_wait_for_score_exit(JSTR(
                "［リターンを押すと終了します］",
                "[Press Enter to exit]"));
            free(argument_save);
            free(menu_save);
            return 1;
        }
        free(argument_save);
        free(menu_save);
    }
#else
    if (argc == 2)
    {
        if (!restore(argv[1], envp)) /* Note: restore will never return */
            exit(1);
    }
#endif
    time(&now);
    lowtime = (int) now;
    dnum = (wizard && getenv("SEED") != NULL ?
	atoi(getenv("SEED")) :
	lowtime + rogue_platform_pid());
    seed = dnum;

    init_player();			/* Roll up the rogue */
    init_things();			/* Set up probabilities of things */
    init_names();			/* Set up names of scrolls */
    init_colors();			/* Set up colors of potions */
    init_stones();			/* Set up stone settings of rings */
    init_materials();			/* Set up materials of wands */
    if (!rogue_init_screen())
        return 1;
#ifdef _WIN32
    /* The terminal frontend may only become ready when the curses screen
     * is active. Re-send the VT request after PDCurses has initialized. */
    rogue_windows_retry_direct_launch_resize(0);
#endif
    /* Display the original greeting on the curses game screen, not on the
     * shell's normal screen.  When endwin() restores the normal screen,
     * neither the greeting nor its erase operation can affect the score
     * list (including macOS launches from AppleScript).  new_level()
     * clears this screen before drawing the dungeon. */
    if (wizard)
        snprintf(startup_message, sizeof startup_message,
                 JSTR("やあ、%s。#%dのダンジョンへようこそ・・・",
                      "Hello %s, welcome to dungeon #%d"), whoami, dnum);
    else
        snprintf(startup_message, sizeof startup_message,
                 JSTR("やあ、%s。 ダンジョンを掘っている間、ちょっと待っててね...",
                      "Hello %s, just a moment while I dig the dungeon..."), whoami);
    wmove(cw, 0, 0);
    rogue_waddstr_utf8(cw, startup_message);
    draw(cw);
    rogue_delay_ms(ROGUE_STARTUP_DELAY_MS);

    waswizard = wizard;
    new_level();			/* Draw current level */
    /*
     * Start up daemons and fuses
     */
    daemon(doctor, 0, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    daemon(stomach, 0, AFTER);
    daemon(runners, 0, AFTER);
    /*
     * Give the rogue his weaponry.  First a mace.
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = WEAPON;
    obj->o_which = MACE;
    init_weapon(obj, MACE);
    obj->o_hplus = 1;
    obj->o_dplus = 1;
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    cur_weapon = obj;
    /*
     * Now a +1 bow
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = WEAPON;
    obj->o_which = BOW;
    init_weapon(obj, BOW);
    obj->o_hplus = 1;
    obj->o_dplus = 0;
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    /*
     * Now some arrows
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = WEAPON;
    obj->o_which = ARROW;
    init_weapon(obj, ARROW);
    obj->o_count = 25+rnd(15);
    obj->o_hplus = obj->o_dplus = 0;
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    /*
     * And his suit of armor
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = ARMOR;
    obj->o_which = RING_MAIL;
    obj->o_ac = a_class[RING_MAIL] - 1;
    obj->o_flags |= ISKNOW;
    cur_armor = obj;
    add_pack(item, TRUE);
    /*
     * Give him some food too
     */
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    obj->o_type = FOOD;
    obj->o_count = 1;
    obj->o_which = 0;
    add_pack(item, TRUE);
    playit();
    return 0;
}

/*
 * endit:
 *	Exit the program abnormally.
 */

void endit(int sig)
{
    (void) sig;
    fatal((char *) JSTR("そこまで終了したいなら、仕方ありません。\n", "Ok, if you want to exit that badly, I\'ll have to allow it\n"));
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */

void fatal(const char *s)
{
    rogue_vt_clear_window(stdscr);
    clear();
    move(ROGUE_SCREEN_LINES-2, 0);
    printw("%s", s);
    draw(stdscr);
    endwin();
    exit(0);
}

/*
 * rnd:
 *	Pick a very random number.
 */

int rnd(int range)
{
    return range <= 0 ? 0 : abs(RN) % range;
}

/*
 * roll:
 *	roll a number of dice
 */

int roll(int number, int sides)
{
    register int dtotal = 0;

    while(number--)
	dtotal += rnd(sides)+1;
    return dtotal;
}
# if defined(SIGTSTP) && !defined(_WIN32)
/*
 * handle stop and start signals
 */
void tstp(int sig)
{
    (void) sig;
    move(ROGUE_SCREEN_LINES - 1, 0);
#ifdef _WIN32
    rogue_refresh_utf8();
#else
    refresh();
#endif
    endwin();
    fflush(stdout);
    kill(0, SIGTSTP);
    signal(SIGTSTP, tstp);
    crmode();
    noecho();
    clearok(curscr, TRUE);
    touchwin(cw);
    draw(cw);
    rogue_flush_typeahead();
}
# endif

void setup(void)
{
#ifdef CHECKTIME
    int  checkout();
#endif

#ifndef DUMP
#ifdef SIGHUP
    signal(SIGHUP, auto_save);
#endif
    signal(SIGILL, auto_save);
#ifdef SIGTRAP
    signal(SIGTRAP, auto_save);
#endif
#ifdef SIGIOT
    signal(SIGIOT, auto_save);
#elif defined(SIGABRT)
    signal(SIGABRT, auto_save);
#endif
#ifdef SIGEMT
    signal(SIGEMT, auto_save);
#endif
    signal(SIGFPE, auto_save);
#ifdef SIGBUS
    signal(SIGBUS, auto_save);
#endif
    signal(SIGSEGV, auto_save);
#ifdef SIGSYS
    signal(SIGSYS, auto_save);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, auto_save);
#endif
    signal(SIGTERM, auto_save);
#endif

    signal(SIGINT, quit);
#ifndef DUMP
#ifdef SIGQUIT
    signal(SIGQUIT, endit);
#endif
#endif
#if defined(SIGTSTP) && !defined(_WIN32)
    signal(SIGTSTP, tstp);
#endif
#ifdef CHECKTIME
    if (!author())
    {
	signal(SIGALRM, checkout);
	alarm(CHECKTIME * 60);
	num_checks = 0;
    }
#endif
    crmode();				/* Cbreak mode */
    noecho();				/* Echo off */
}

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */

void playit(void)
{
    register char *opts;

    /*
     * Modern curses no longer exposes the old sgtty speed field here.
     * Keep the normal defaults and let the ROGUEOPTS environment variable
     * control terse/jump.
     */

    /*
     * Reapply startup options after restoring a save, if applicable.
     * The auto value is resolved here rather than becoming a game mode.
     */
#ifdef _WIN32
    opts = rogue_windows_getenv_utf8("ROGUEOPTS");
#else
    opts = getenv("ROGUEOPTS");
#endif
    if (opts != NULL)
	parse_opts(opts);
#ifdef _WIN32
    free(opts);
#endif
    rogue_ensure_utf8_locale();

    oldpos = hero;
    oldrp = roomin(&hero);
    while (playing)
	command();			/* Command execution */
    endit(0);
}

#if MAXLOAD|MAXUSERS
/*
 * see if the system is being used too much for this game
 */
int too_much(void)
{
#ifdef MAXLOAD
    double avec[3];
#else
    register int cnt;
#endif

#ifdef MAXLOAD
    loadav(avec);
    return (avec[2] > (MAXLOAD / 10.0));
#else
    return (ucount() > MAXUSERS);
#endif
}

/*
 * see if a user is an author of the program
 */
int author(void)
{
    switch (getuid())
    {
	case 24601:
	    return TRUE;
	default:
	    return FALSE;
    }
}
#endif

#ifdef CHECKTIME
void checkout(void)
{
    const char *msgs[3];
    int checktime;

    msgs[0] = JSTR("負荷が高すぎます。%d分以内に終了してください。", "The load is too high to be playing.  Please leave in %d minutes");
    msgs[1] = JSTR("ゲームをセーブしてください。残り%d分です。", "Please save your game.  You have %d minutes");
    msgs[2] = JSTR("最後の警告です。残り%d分です。", "Last warning.  You have %d minutes to leave");

    signal(SIGALRM, checkout);
    if (too_much())
    {
	if (num_checks == 3)
	    fatal((char *) JSTR("残念。長くかかりすぎました。あなたは死にました。\n", "Sorry.  You took to long.  You are dead\n"));
	checktime = CHECKTIME / (num_checks + 1);
	chmsg(msgs[num_checks++], checktime);
	alarm(checktime * 60);
    }
    else
    {
	if (num_checks)
	{
	    chmsg(JSTR("負荷が下がりました。猶予が与えられました。", "The load has dropped back down.  You have a reprieve."));
	    num_checks = 0;
	}
	alarm(CHECKTIME * 60);
    }
}

/*
 * checkout()'s version of msg.  If we are in the middle of a shell, do a
 * printf instead of a msg to avoid the refresh.
 */
void chmsg(const char *fmt, int arg)
{
    if (in_shell)
    {
	printf(fmt, arg);
	putchar('\n');
	fflush(stdout);
    }
    else
	msg(fmt, arg);
}
#endif

#ifdef LOADAV

#include <nlist.h>

struct nlist avenrun =
{
    "_avenrun"
};

void loadav(double *avg)
{
    register int kmem;

    if ((kmem = open("/dev/kmem", 0)) < 0)
	goto bad;
    nlist(NAMELIST, &avenrun);
    if (avenrun.n_type == 0)
    {
bad:
	avg[0] = avg[1] = avg[2] = 0.0;
	return;
    }

    lseek(kmem, (long) avenrun.n_value, 0);
    read(kmem, avg, 3 * sizeof (double));
}
#endif

#ifdef UCOUNT

#include <utmp.h>

struct utmp buf;

int ucount(void)
{
    register struct utmp *up;
    register FILE *utmp;
    register int count;

    if ((utmp = fopen(UTMP, "r")) == NULL)
	return 0;

    up = &buf;
    count = 0;

    while (fread(up, 1, sizeof (*up), utmp) > 0)
	if (buf.ut_name[0] != '\0')
	    count++;
    fclose(utmp);
    return count;
}
#endif
