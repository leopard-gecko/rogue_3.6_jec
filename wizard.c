
/*
 * Special wizard commands (some of which are also non-wizard commands
 * under strange circumstances)
 *
 * @(#)wizard.c	3.8 (Berkeley) 6/3/81
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * whatis:
 *	What a certin object is
 */

void whatis(void)
{
    register struct object *obj;
    register struct linked_list *item;

    if ((item = get_item("identify", 0)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    switch (obj->o_type)
    {
        when SCROLL:
	    s_know[obj->o_which] = TRUE;
	    if (s_guess[obj->o_which])
	    {
		free(s_guess[obj->o_which]);
		s_guess[obj->o_which] = NULL;
	    }
        when POTION:
	    p_know[obj->o_which] = TRUE;
	    if (p_guess[obj->o_which])
	    {
		free(p_guess[obj->o_which]);
		p_guess[obj->o_which] = NULL;
	    }
	when STICK:
	    ws_know[obj->o_which] = TRUE;
	    obj->o_flags |= ISKNOW;
	    if (ws_guess[obj->o_which])
	    {
		free(ws_guess[obj->o_which]);
		ws_guess[obj->o_which] = NULL;
	    }
        when WEAPON:
        case ARMOR:
	    obj->o_flags |= ISKNOW;
        when RING:
	    r_know[obj->o_which] = TRUE;
	    obj->o_flags |= ISKNOW;
	    if (r_guess[obj->o_which])
	    {
		free(r_guess[obj->o_which]);
		r_guess[obj->o_which] = NULL;
	    }
    }
    msg(inv_name(obj, FALSE));
}

/*
 * create_obj:
 *	Wizard command for getting anything he wants
 */

void create_obj(void)
{
    register struct linked_list *item;
    register struct object *obj;
    register char ch, bless;

    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    msg(JSTR("アイテムの種類：", "Type of item: "));
    obj->o_type = readchar();
    mpos = 0;
    msg(JSTR("どの%cがほしいですか？（0-f）", "Which %c do you want? (0-f)"), obj->o_type);
    obj->o_which = (isdigit((ch = readchar())) ? ch - '0' : ch - 'a' + 10);
    obj->o_group = 0;
    obj->o_count = 1;
    mpos = 0;
    if (obj->o_type == WEAPON || obj->o_type == ARMOR)
    {
	msg(JSTR("祝福は？ (+,-,n)", "Blessing? (+,-,n)"));
	bless = readchar();
	mpos = 0;
	if (bless == '-')
	    obj->o_flags |= ISCURSED;
	if (obj->o_type == WEAPON)
	{
	    init_weapon(obj, obj->o_which);
	    if (bless == '-')
		obj->o_hplus -= rnd(3)+1;
	    if (bless == '+')
		obj->o_hplus += rnd(3)+1;
	}
	else
	{
	    obj->o_ac = a_class[obj->o_which];
	    if (bless == '-')
		obj->o_ac += rnd(3)+1;
	    if (bless == '+')
		obj->o_ac -= rnd(3)+1;
	}
    }
    else if (obj->o_type == RING)
	switch (obj->o_which)
	{
	    case R_PROTECT:
	    case R_ADDSTR:
	    case R_ADDHIT:
	    case R_ADDDAM:
		msg(JSTR("祝福は？ (+,-,n)", "Blessing? (+,-,n)"));
		bless = readchar();
		mpos = 0;
		if (bless == '-')
		    obj->o_flags |= ISCURSED;
		obj->o_ac = (bless == '-' ? -1 : rnd(2) + 1);
	}
    else if (obj->o_type == STICK)
	fix_stick(obj);
    add_pack(item, FALSE);
}

/*
 * telport:
 *	Bamf the hero someplace else
 */

int teleport(void)
{
    register int rm;
    coord c;

    c = hero;
    mvwaddch(cw, hero.y, hero.x, rogue_chtype_to_char(mvwinch(stdscr, hero.y, hero.x)));
    do
    {
	rm = rnd_room();
	rnd_pos(&rooms[rm], &hero);
    } until(winat(hero.y, hero.x) == FLOOR);
    light(&c);
    light(&hero);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
    /*
     * turn off ISHELD in case teleportation was done while fighting
     * a Fungi
     */
    if (on(player, ISHELD)) {
	player.t_flags &= ~ISHELD;
	fung_hit = 0;
	strcpy(fungi_damage, "000d0");
	monsters['F'-'A'].m_stats.s_dmg = fungi_damage;
    }
    count = 0;
    running = FALSE;
    rogue_flush_typeahead();
    return rm;
}

/*
 * passwd:
 *	see if user knows password
 */

int wizard_password_ok(char *plain)
{
    char *encoded;

#ifdef _WIN32
    encoded = rogue_windows_crypt(plain, "mT");
#else
    encoded = crypt(plain, "mT");
#endif
    if (encoded == NULL)
	return FALSE;
    return (strcmp(PASSWD, encoded) == 0);
}

/*
 * Read the wizard password using the same curses input path as ordinary
 * commands on all platforms.  Mixing getchar() with curses input can
 * consume or misroute keystrokes, and the original loop could overflow.
 * Wizard passwords use printable ASCII; special keys are ignored.
 */
int passwd(void)
{
    char buf[ROGUE_INPUT_SIZE];
    size_t len = 0;
    int c;

    msg(JSTR("魔法使いの合言葉は？", "Wizard's Password:"));
    mpos = 0;
    for (;;)
    {
        c = readchar();
        if (c == EOF || c == '\n' || c == '\r' || c == ESCAPE)
            break;
#ifdef KEY_ENTER
        if (c == KEY_ENTER)
            break;
#endif
        if (c == CTRL('U'))
        {
            len = 0;
            continue;
        }
        if (c == '\b' || c == 0177)
        {
            if (len != 0)
                len--;
            continue;
        }
        if (c < ' ' || c > '~')
            continue;
        if (len + 1 < sizeof buf)
            buf[len++] = (char)c;
    }
    if (len == 0)
        return FALSE;
    buf[len] = '\0';
    return wizard_password_ok(buf);
}
