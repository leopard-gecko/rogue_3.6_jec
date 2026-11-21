/*
 * File with various monster functions in it
 *
 * @(#)monsters.c	3.18 (Berkeley) 6/15/81
 */

#include <curses.h>
#include "rogue.h"
#include <ctype.h>

/*
 * List of monsters in rough order of vorpalness
 */
static char lvl_mons[] =  "KJBSHEAOZGLCRQNYTWFIXUMVDP";
static char wand_mons[] = "KJBSH AOZG CRQ Y W IXU V  ";

/*
 * The portable save stores genocided species as bits A=0 through Z=25.
 * Derive the bits from the original generation table so genocide() remains
 * the single place that changes the live monster selection rules.
 */
unsigned int rogue_genocide_mask(void)
{
    unsigned int mask = 0;
    int c;

    for (c = 'A'; c <= 'Z'; c++)
        if (strchr(lvl_mons, c) == NULL)
            mask |= 1U << (c - 'A');
    return mask;
}

/* Apply the saved exclusions to both normal and wandering generation. */
void rogue_restore_genocide_mask(unsigned int mask)
{
    int i;

    for (i = 0; i < 26; i++)
        if (lvl_mons[i] >= 'A' && lvl_mons[i] <= 'Z' &&
            (mask & (1U << (lvl_mons[i] - 'A'))) != 0)
        {
            lvl_mons[i] = ' ';
            wand_mons[i] = ' ';
        }
}

/*
 * randmonster:
 *	Pick a monster to show up.  The lower the level,
 *	the meaner the monster.
 */

int randmonster(bool wander)
{
    register int d;
    register char *mons;
    int attempts;

    mons = wander ? wand_mons : lvl_mons;
    for (attempts = 0; attempts < 1000; attempts++)
    {
	d = level + (rnd(10) - 5);
	if (d < 1)
	    d = rnd(5) + 1;
	if (d > 26)
	    d = rnd(5) + 22;
	if (mons[--d] != ' ')
	    return mons[d];
    }

    /*
     * If genocide has removed nearly every monster that this table can
     * select, the old do/while loop can spin forever.  Fall back to any
     * remaining level monster.  If the player has somehow genocided every
     * type, return kobold as a harmless last-resort value rather than
     * hanging the game.
     */
    for (d = 0; d < 26; d++)
	if (lvl_mons[d] != ' ')
	    return lvl_mons[d];
    return 'K';
}

/*
 * new_monster:
 *	Pick a new monster and add it to the list
 */

void new_monster(struct linked_list *item, char type, coord *cp)
{
    register struct thing *tp;
    register struct monster *mp;

    attach(mlist, item);
    tp = (struct thing *) ldata(item);
    tp->t_type = type;
    tp->t_pos = *cp;
    tp->t_oldch = rogue_chtype_to_char(mvwinch(cw, cp->y, cp->x));
    mvwaddch(mw, cp->y, cp->x, tp->t_type);
    mp = &monsters[tp->t_type-'A'];
    tp->t_stats.s_hpt = roll(mp->m_stats.s_lvl, 8);
    tp->t_stats.s_lvl = mp->m_stats.s_lvl;
    tp->t_stats.s_arm = mp->m_stats.s_arm;
    tp->t_stats.s_dmg = mp->m_stats.s_dmg;
    tp->t_stats.s_exp = mp->m_stats.s_exp;
    tp->t_stats.s_str.st_str = 10;
    tp->t_flags = mp->m_flags;
    tp->t_turn = TRUE;
    tp->t_pack = NULL;
    if (ISWEARING(R_AGGR))
	runto(cp, &hero);
    if (type == 'M')
    {
	char mch = GOLD;

	if (tp->t_pack != NULL)
	    mch = ((struct object *) ldata(tp->t_pack))->o_type;
	else
	    switch (rnd(level > 25 ? 9 : 8))
	    {
		when 0: mch = GOLD;
		when 1: mch = POTION;
		when 2: mch = SCROLL;
		when 3: mch = STAIRS;
		when 4: mch = WEAPON;
		when 5: mch = ARMOR;
		when 6: mch = RING;
		when 7: mch = STICK;
		when 8: mch = AMULET;
	    }
	tp->t_disguise = mch;
    }
}

/*
 * wanderer:
 *	A wandering monster has awakened and is headed for the player
 */

void wanderer(void)
{
    register int i, ch;
    register struct room *rp, *hr = roomin(&hero);
    register struct linked_list *item;
    register struct thing *tp;
    coord cp;

    item = new_item(sizeof *tp);
    do
    {
	i = rnd_room();
	if ((rp = &rooms[i]) == hr)
	    continue;
	rnd_pos(rp, &cp);
	if ((ch = rogue_chtype_to_char(mvwinch(stdscr, cp.y, cp.x))) == ERR)
	{
	    debug(JSTR("徘徊怪物処理: mvwinch が %d,%d で失敗", "Routine wanderer: mvwinch failed to %d,%d"), cp.y, cp.x);
	    wait_for('\n');
	    return;
	}
    } until(hr != rp && step_ok(ch));
    new_monster(item, randmonster(TRUE), &cp);
    tp = (struct thing *) ldata(item);
    tp->t_flags |= ISRUN;
    tp->t_pos = cp;
    tp->t_dest = &hero;
    if (wizard)
	msg(JSTR("%sがさまよい始めた", "Started a wandering %s"), monsters[tp->t_type-'A'].m_name);
}

/*
 * what to do when the hero steps next to a monster
 */
struct linked_list * wake_monster(int y, int x)
{
    register struct thing *tp;
    register struct linked_list *it;
    register struct room *rp;
    register char ch;

    /*
     * The original code only assigned rp inside the umber-hulk gaze block,
     * but the greedy-monster code below also uses it.  A dragon is greedy but
     * not an umber hulk, so stepping next to a dragon could read an
     * uninitialized rp and crash on modern systems.
     */
    rp = roomin(&hero);

    if ((it = find_mons(y, x)) == NULL)
    {
	msg(JSTR("表示する怪物が見つからない。", "Can't find monster in show"));
	return NULL;
    }
    tp = (struct thing *) ldata(it);
    if (tp == NULL)
	return NULL;
    ch = tp->t_type;
    /*
     * Every time he sees mean monster, it might start chasing him
     */
    if (rnd(100) > 33 && on(*tp, ISMEAN) && off(*tp, ISHELD)
	&& !ISWEARING(R_STEALTH))
    {
	tp->t_dest = &hero;
	tp->t_flags |= ISRUN;
    }
    if (ch == 'U' && off(player, ISBLIND))
    {
	if ((rp != NULL && !(rp->r_flags&ISDARK))
	    || DISTANCE(y, x, hero.y, hero.x) < 3)
	{
	    if (off(*tp, ISFOUND) && !save(VS_MAGIC))
	    {
		msg(JSTR("褐色の大あご虫の視線で混乱した。", "The umber hulk's gaze has confused you."));
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(20)+HUHDURATION);
		else
		    fuse(unconfuse, 0, rnd(20)+HUHDURATION, AFTER);
		player.t_flags |= ISHUH;
	    }
	    tp->t_flags |= ISFOUND;
	}
    }
    /*
     * Hide invisible monsters
     */
    if (on(*tp, ISINVIS) && off(player, CANSEE))
	ch = rogue_chtype_to_char(mvwinch(stdscr, y, x));
    /*
     * Let greedy ones guard gold
     */
    if (on(*tp, ISGREED) && off(*tp, ISRUN))
	if (rp != NULL && rp->r_goldval)
	{
	    tp->t_dest = &rp->r_gold;
	    tp->t_flags |= ISRUN;
	}
    return it;
}

void genocide(void)
{
    register struct linked_list *ip;
    register struct thing *mp;
    register char c;
    register int i;
    register struct linked_list *nip;

    addmsg(JSTR("どの怪物を", "Which monster"));
    if (!terse)
	addmsg(JSTR("消し去りますか", " do you wish to wipe out"));
    msg(JSTR("？", "? "));
    while (!isalpha(c = readchar()))
	if (c == ESCAPE)
	    return;
	else
	{
	    mpos = 0;
	    msg(JSTR("'A'から'Z'までの文字を指定してください。", "Please specifiy a letter between 'A' and 'Z'"));
	}
    if (islower(c))
	c = toupper(c);
    if (c == 'F')
    {
	player.t_flags &= ~ISHELD;
	fung_hit = 0;
	strcpy(fungi_damage, "000d0");
	monsters['F'-'A'].m_stats.s_dmg = fungi_damage;
    }
    for (ip = mlist; ip; ip = nip)
    {
	mp = (struct thing *) ldata(ip);
	nip = next(ip);
	if (mp->t_type == c)
	    remove(&mp->t_pos, ip);
    }
    for (i = 0; i < 26; i++)
	if (lvl_mons[i] == c)
	{
	    lvl_mons[i] = ' ';
	    wand_mons[i] = ' ';
	    break;
	}
}
