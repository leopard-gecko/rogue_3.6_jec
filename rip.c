/*
 * File for the fun ends
 * Death or a total win
 *
 * @(#)rip.c	3.13 (Berkeley) 6/16/81
 *
 * CHANGE!  Mike Hibler @ New Mexico Tech
 *	Added constant NUMTOP which is the number of top scores to keep.
 *	NUMTOP is defined in mach_dep.h.
 */

#include <curses.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <pwd.h>
#endif
#include "mach_dep.h"
#include "rogue.h"

static int rogue_lock_fd = -1;

#ifdef _WIN32
/* An OS file lock, not an existence-based lock: safe across concurrent runs. */
static HANDLE rogue_score_handle = INVALID_HANDLE_VALUE;
static int rogue_score_lock_acquire(const char *path)
{
    wchar_t *wide = rogue_windows_utf8_to_wide(path);
    OVERLAPPED ov = {0};
    if (wide == NULL)
        return -1;
    rogue_score_handle = CreateFileW(wide, GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    free(wide);
    if (rogue_score_handle == INVALID_HANDLE_VALUE)
        return -1;
    if (!LockFileEx(rogue_score_handle, LOCKFILE_EXCLUSIVE_LOCK,
                    0, 1, 0, &ov))
    {
        CloseHandle(rogue_score_handle);
        rogue_score_handle = INVALID_HANDLE_VALUE;
        return -1;
    }
    return 1;
}
static void rogue_score_lock_release(int ignored)
{
    OVERLAPPED ov = {0};
    (void)ignored;
    if (rogue_score_handle != INVALID_HANDLE_VALUE)
    {
        UnlockFileEx(rogue_score_handle, 0, 1, 0, &ov);
        CloseHandle(rogue_score_handle);
        rogue_score_handle = INVALID_HANDLE_VALUE;
    }
}
#else
static int rogue_score_lock_acquire(const char *lock_path)
{
    struct flock fl;
    int fd;

    fd = open(lock_path, O_RDWR | O_CREAT, 0600);
    if (fd < 0)
        return -1;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

    if (fcntl(fd, F_SETLKW, &fl) < 0)
    {
        close(fd);
        return -1;
    }

    return fd;
}

static void rogue_score_lock_release(int fd)
{
    struct flock fl;

    if (fd < 0)
        return;

    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    (void) fcntl(fd, F_SETLK, &fl);
    close(fd);
}

#endif /* _WIN32 */

static char *rip[] = {
"                       __________",
"                      /          \\",
JSTR_STATIC("                     /            \\", "                     /    REST    \\"),
JSTR_STATIC("                    /              \\", "                    /      IN      \\"),
JSTR_STATIC("                   /                \\", "                   /     PEACE      \\"),
"                  /                  \\",
"                  |                  |",
"                  |                  |",
JSTR_STATIC("                  |                  |", "                  |   killed by a    |"),
"                  |                  |",
"                  |       1980       |",
"                 *|     *  *  *      | *",
"         ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______",
    0
};
void
jrogue_apply_rip_language(void)
{
    rip[2] = (char *) JSTR("                     /            \\", "                     /    REST    \\");
    rip[3] = (char *) JSTR("                    /              \\", "                    /      IN      \\");
    rip[4] = (char *) JSTR("                   /                \\", "                   /     PEACE      \\");
    rip[8] = (char *) JSTR("                  |                  |", "                  |   killed by a    |");
}

static void
grave_bounds(int row, int *left, int *right)
{
    switch (row)
    {
	case 9:  *left = 23; *right = 32; break;
	case 10: *left = 22; *right = 33; break;
	case 11: *left = 21; *right = 34; break;
	case 12: *left = 20; *right = 35; break;
	case 13: *left = 19; *right = 36; break;
	default: *left = 19; *right = 36; break;
    }
}

static void
grave_text(int row, const char *text)
{
    int left, right, width, col, i;

    grave_bounds(row, &left, &right);
    width = right - left + 1;
    col = rogue_utf8_center_column(left, width, text);
    move(row, left);
    for (i = 0; i < width; i++)
	addch(' ');
    move(row, col);
    rogue_waddstr_utf8(stdscr, text);
}


/*
 * death:
 *	Do something really fun when he dies
 */

void death(char monst)
{
    register char **dp = rip;
    register char *killer;
    register struct tm *lt;
    time_t date;
    char buf[80];

    time(&date);
    lt = localtime(&date);
    rogue_vt_clear_window(stdscr);
    clear();
    move(8, 0);
    while (*dp)
	printw("%s\n", *dp++);
    if (jrogue_lang_is_japanese)
    {
	grave_text(11, "安らかに");
	grave_text(12, "眠れ");
    }
    grave_text(14, whoami);
    purse -= purse/10;
    sprintf(buf, JSTR("＄%d", "%d Au"), purse);
    grave_text(15, buf);
    killer = killname(monst);
    if (jrogue_lang_is_japanese)
    {
	grave_text(16, killer);
	grave_text(17, isupper(monst) ? "と戦いて死す" : "により死す");
    }
    else
    {
	grave_text(17, killer);
	mvaddstr(16, 33, vowelstr(killer));
    }
    /* tm_year is years since 1900.  The original code wrote only
     * the last two digits into the tombstone's built-in "1980" text.
     * On modern systems tm_year is now 100 or greater, which produced
     * broken years such as "19126".  Write the full year instead.
     */
    sprintf(prbuf, "%4d", 1900 + lt->tm_year);
    mvaddstr(18, 26, prbuf);
    move(ROGUE_SCREEN_LINES-1, 0);
    addstr(JSTR("［リターンを押してください］", "[Press return to continue]"));
    draw(stdscr);
    wait_for_window(stdscr, '\n');
    score(purse, 0, monst);
    exit(0);
}

/*
 * The score list is printed after endwin(), on the normal terminal screen.
 * Clear remnants of earlier output before a game-over score display, but
 * leave a standalone "rogue -s" listing and redirected stdout untouched.
 * Windows keeps its existing direct-launch-only console clearing behavior.
 */
static void
score_clear_plain_screen(int flags)
{
    if (flags == -1)
        return;
#ifdef _WIN32
    rogue_windows_clear_score_screen();
#else
    if (isatty(fileno(stdout)))
    {
        fputs("\033[0m\033[H\033[2J", stdout);
        fflush(stdout);
    }
#endif
}

/*
 * score -- figure score and post it.
 */

/* VARARGS2 */
void score(int amount, int flags, char monst)
{
    static struct sc_ent {
	int sc_score;
	char sc_name[80];
	int sc_flags;
	int sc_level;
	int sc_uid;
	char sc_monster;
    } top_ten[NUMTOP];
    register struct sc_ent *scp;
    register int i;
    register struct sc_ent *sc2;
    register FILE *outf;
    register char *killer;
    register int prflags = 0;
    register int fd;
    char score_path[1024];
    char lock_path[1024];
    static char *reason[] = {
	"killed",
	"quit",
	"A total winner",
    };
    if (flags != -1)
	endwin();
    /*
     * Open file and read list
     */

    rogue_build_file_path(score_path, sizeof score_path, SCOREDIR, SCOREFILE);
    rogue_build_file_path(lock_path, sizeof lock_path, LOCKDIR, LOCKFILE);

    rogue_lock_fd = rogue_score_lock_acquire(lock_path);
    if (rogue_lock_fd < 0)
	return;

#ifdef _WIN32
    /* A Saved Games path may contain UTF-8 Japanese user/profile names.
     * Use the wide CRT open path, as the Windows file lock above already does. */
    {
        wchar_t *wide_score_path = rogue_windows_utf8_to_wide(score_path);
        fd = wide_score_path != NULL
            ? _wopen(wide_score_path, O_RDWR | O_CREAT | O_BINARY, 0600)
            : -1;
        free(wide_score_path);
    }
    if (fd < 0)
#else
    if ((fd = open(score_path, O_RDWR | O_CREAT, 0600)) < 0)
#endif
    {
	rogue_score_lock_release(rogue_lock_fd);
	rogue_lock_fd = -1;
	return;
    }
#ifdef _WIN32
    outf = _fdopen(fd, "r+b");
#else
    outf = fdopen(fd, "w");
#endif
    if (outf == NULL)
    {
        close(fd);
        rogue_score_lock_release(rogue_lock_fd);
        rogue_lock_fd = -1;
        return;
    }

    for (scp = top_ten; scp < &top_ten[NUMTOP]; scp++)
    {
	scp->sc_score = 0;
	for (i = 0; i < 80; i++)
	    scp->sc_name[i] = rnd(255);
	scp->sc_flags = RN;
	scp->sc_level = RN;
	scp->sc_monster = RN;
	scp->sc_uid = RN;
    }

    signal(SIGINT, SIG_DFL);
    /*
     * The original 3.6 score routine printed a bare
     * "[Press return to continue]" before showing the score list for
     * every non-death ending.  On modern curses this appears after
     * leaving the alternate screen and looks like an accidental hang
     * when the player quits with 'q'.
     *
     * Keep the prompt only for wizard score-list commands, where the
     * typed response may be "names" or "edit".  Normal quit now goes
     * straight to the score list.
     */
    if (wizard && flags != -1 && flags != 0)
    {
	printf("%s", JSTR("［リターンを押してください］", "[Press return to continue]"));
	fflush(stdout);
	fgets(prbuf, sizeof prbuf, stdin);
	prbuf[strcspn(prbuf, "\n")] = '\0';
    }
    else
	prbuf[0] = '\0';
    if (wizard)
    {
	if (strcmp(prbuf, "names") == 0)
	    prflags = 1;
	else if (strcmp(prbuf, "edit") == 0)
	    prflags = 2;
    }
    encread((char *) top_ten, sizeof top_ten, fd);
    /*
     * Insert her in list if need be
     */
    if (!waswizard)
    {
	for (scp = top_ten; scp < &top_ten[NUMTOP]; scp++)
	    if (amount > scp->sc_score)
		break;
	if (scp < &top_ten[NUMTOP])
	{
	    for (sc2 = &top_ten[NUMTOP-1]; sc2 > scp; sc2--)
		*sc2 = *(sc2-1);
	    scp->sc_score = amount;
	    rogue_utf8_copy(scp->sc_name, sizeof scp->sc_name, whoami);
	    scp->sc_flags = flags;
	    if (flags == 2)
		scp->sc_level = max_level;
	    else
		scp->sc_level = level;
	    scp->sc_monster = monst;
	    #ifdef _WIN32
	    scp->sc_uid = 0;
#else
	    scp->sc_uid = getuid();
#endif
	}
    }
    /*
     * Print the list
     */
    score_clear_plain_screen(flags);
    if (jrogue_lang_is_japanese)
	printf("\nトップ%dの冒険者:\n    得点 名前\n", NUMTOP);
    else
	printf("\nTop %d Adventurers:\nRank\tScore\tName\n", NUMTOP);
    for (scp = top_ten; scp < &top_ten[NUMTOP]; scp++) {
	if (scp->sc_score) {
	    if (jrogue_lang_is_japanese)
	    {
		if (scp->sc_flags == 0)
		{
		    killer = killname(scp->sc_monster);
		    printf("%2ld %5d %s: 地下%d階にて%s%s。",
			(long)(scp - top_ten + 1), scp->sc_score, scp->sc_name,
			scp->sc_level, killer,
			isupper(scp->sc_monster) ? "と戦いて死す" : "により死す");
		}
		else if (scp->sc_flags == 1)
		    printf("%2ld %5d %s: 地下%d階にて逃亡す。",
			(long)(scp - top_ten + 1), scp->sc_score, scp->sc_name,
			scp->sc_level);
		else
		    printf("%2ld %5d %s: 完全勝利者。",
			(long)(scp - top_ten + 1), scp->sc_score, scp->sc_name);
	    }
	    else
	    {
		printf("%ld\t%d\t%s: %s on level %d", (long)(scp - top_ten + 1),
		    scp->sc_score, scp->sc_name, reason[scp->sc_flags],
		    scp->sc_level);
		if (scp->sc_flags == 0) {
		    fputs(JSTR("", " by a"), stdout);
		    killer = killname(scp->sc_monster);
		    if (*killer == 'a' || *killer == 'e' || *killer == 'i' ||
			*killer == 'o' || *killer == 'u')
			    putchar('n');
		    printf(" %s", killer);
		}
		printf(".");
	    }
	    if (prflags == 1)
	    {
#ifdef _WIN32
                printf(" (%d)", scp->sc_uid);
#else
		struct passwd *pp;
		if ((pp = getpwuid(scp->sc_uid)) == NULL)
		    printf(" (%d)", scp->sc_uid);
		else
		    printf(" (%s)", pp->pw_name);
#endif
		putchar('\n');
	    }
	    else if (prflags == 2)
	    {
		fflush(stdout);
		fgets(prbuf, sizeof prbuf, stdin);
		prbuf[strcspn(prbuf, "\n")] = '\0';
		if (prbuf[0] == 'd')
		{
		    for (sc2 = scp; sc2 < &top_ten[NUMTOP-1]; sc2++)
			*sc2 = *(sc2 + 1);
		    top_ten[NUMTOP-1].sc_score = 0;
		    for (i = 0; i < 80; i++)
			top_ten[NUMTOP-1].sc_name[i] = rnd(255);
		    top_ten[NUMTOP-1].sc_flags = RN;
		    top_ten[NUMTOP-1].sc_level = RN;
		    top_ten[NUMTOP-1].sc_monster = RN;
		    scp--;
		}
	    }
	    else
		putchar('\n');
	}
    }
    fseek(outf, 0L, 0);
    /*
     * Update the list file
     */
    encwrite((char *) top_ten, sizeof top_ten, outf);
    fclose(outf);
    rogue_score_lock_release(rogue_lock_fd);
    rogue_lock_fd = -1;
#ifdef _WIN32
    rogue_windows_wait_for_score_exit(JSTR("［リターンを押すと終了します］",
                                          "[Press return to exit]"));
#endif
}

void total_winner(void)
{
    register struct linked_list *item;
    register struct object *obj;
    register int worth;
    register char c;
    register int oldpurse;

    rogue_vt_clear_window(stdscr);
    clear();
    standout();
    addstr("                                                               \n");
    addstr("  @   @               @   @           @          @@@  @     @  \n");
    addstr("  @   @               @@ @@           @           @   @     @  \n");
    addstr("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
    addstr("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
    addstr("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
    addstr("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
    addstr("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
    addstr("                                                               \n");
    addstr(JSTR("     おめでとう！  君はようやく太陽の光のもとにたどりついた。  \n", "     Congratulations, you have made it to the light of day!    \n"));
    standend();
    addstr(JSTR("\n     そして、運命の洞窟から生きて戻った精鋭の一人となった。    \n", "\nYou have joined the elite ranks of those who have escaped the\n"));
    addstr(JSTR("     君は故郷へ帰り、すべての戦利品を売って大金を手に入れ、    \n", "Dungeons of Doom alive.  You journey home and sell all your loot at\n"));
    addstr(JSTR("     戦士ギルドに迎え入れられたということである・・・。        \n", "a great profit and are admitted to the Fighters' Guild.\n"));
    mvaddstr(ROGUE_SCREEN_LINES - 1, 0, JSTR("--スペースを押してください--", "--Press space to continue--"));
#ifdef _WIN32
    rogue_refresh_utf8();
#else
    refresh();
#endif
    wait_for_window(stdscr, ' ');
    rogue_vt_clear_window(stdscr);
    clear();
    mvaddstr(0, 0, JSTR("    価値  持ちもの", "   Worth  Item"));
    oldpurse = purse;
    for (c = 'a', item = pack; item != NULL; c++, item = next(item))
    {
	worth = 0; /* Unknown item types have no resale value. */
	obj = (struct object *) ldata(item);
	switch (obj->o_type)
	{
	    when FOOD:
		worth = 2 * obj->o_count;
	    when WEAPON:
		switch (obj->o_which)
		{
		    when MACE: worth = 8;
		    when SWORD: worth = 15;
		    when BOW: worth = 75;
		    when ARROW: worth = 1;
		    when DAGGER: worth = 2;
		    when ROCK: worth = 1;
		    when TWOSWORD: worth = 30;
		    when SLING: worth = 1;
		    when DART: worth = 1;
		    when CROSSBOW: worth = 15;
		    when BOLT: worth = 1;
		    when SPEAR: worth = 2;
		    otherwise: worth = 0;
		}
		worth *= (1 + (10 * obj->o_hplus + 10 * obj->o_dplus));
		worth *= obj->o_count;
		obj->o_flags |= ISKNOW;
	    when ARMOR:
		switch (obj->o_which)
		{
		    when LEATHER: worth = 5;
		    when RING_MAIL: worth = 30;
		    when STUDDED_LEATHER: worth = 15;
		    when SCALE_MAIL: worth = 3;
		    when CHAIN_MAIL: worth = 75;
		    when SPLINT_MAIL: worth = 80;
		    when BANDED_MAIL: worth = 90;
		    when PLATE_MAIL: worth = 400;
		    otherwise: worth = 0;
		}
		worth *= (1 + (10 * (a_class[obj->o_which] - obj->o_ac)));
		obj->o_flags |= ISKNOW;
	    when SCROLL:
		s_know[obj->o_which] = TRUE;
		worth = s_magic[obj->o_which].mi_worth;
		worth *= obj->o_count;
	    when POTION:
		p_know[obj->o_which] = TRUE;
		worth = p_magic[obj->o_which].mi_worth;
		worth *= obj->o_count;
	    when RING:
		obj->o_flags |= ISKNOW;
		r_know[obj->o_which] = TRUE;
		worth = r_magic[obj->o_which].mi_worth;
		if (obj->o_which == R_ADDSTR || obj->o_which == R_ADDDAM ||
		    obj->o_which == R_PROTECT || obj->o_which == R_ADDHIT)
		{
			if (obj->o_ac > 0)
			    worth += obj->o_ac * 20;
			else
			    worth = 50;
		}
	    when STICK:
		obj->o_flags |= ISKNOW;
		ws_know[obj->o_which] = TRUE;
		worth = ws_magic[obj->o_which].mi_worth;
		worth += 20 * obj->o_charges;
	    when AMULET:
		worth = 1000;
	}
	move(c - 'a' + 1, 0);
	printw("%c) %5d  ", c, worth);
	rogue_waddstr_utf8(stdscr, inv_name(obj, FALSE));
	purse += worth;
    }
    mvprintw(c - 'a' + 1, 0, JSTR("   %5d  金塊          ", "   %5d  Gold Pieces          "), oldpurse);
#ifdef _WIN32
    rogue_refresh_utf8();
#else
    refresh();
#endif
    score(purse, 2, 0);
    exit(0);
}

char * killname(char monst)
{
    if (isupper(monst))
	return monsters[monst-'A'].m_name;
    else
	switch (monst)
	{
	    case 'a':
		return (char *) JSTR("矢", "arrow");
	    case 'd':
		return (char *) JSTR("毒投げ矢", "dart");
	    case 'b':
		return (char *) JSTR("ボルト", "bolt");
	}
    return (char *) JSTR("何者か", "something");
}
