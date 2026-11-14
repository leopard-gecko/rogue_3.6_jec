/*
 * All the fighting gets done here
 *
 * @(#)fight.c	3.28 (Berkeley) 6/15/81
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

long e_levels[] = {
    10L,20L,40L,80L,160L,320L,640L,1280L,2560L,5120L,10240L,20480L,
    40920L, 81920L, 163840L, 327680L, 655360L, 1310720L, 2621440L, 0L };

/*
 * fight:
 *	The player attacks the monster.
 */

int fight(coord *mp, char mn, struct object *weap, bool thrown)
{
    register struct thing *tp;
    register struct linked_list *item;
    register bool did_hit = TRUE;

    /*
     * Find the monster we want to fight
     */
    if ((item = find_mons(mp->y, mp->x)) == NULL)
    {
	debug(JSTR("戦闘対象 @ %d,%d", "Fight what @ %d,%d"), mp->y, mp->x);
	return FALSE;
    }
    tp = (struct thing *) ldata(item);
    /*
     * Since we are fighting, things are not quiet so no healing takes
     * place.
     */
    quiet = 0;
    runto(mp, &hero);
    /*
     * Let him know it was really a mimic (if it was one).
     */
    if (tp->t_type == 'M' && tp->t_disguise != 'M' && off(player, ISBLIND))
    {
	msg(JSTR("待て！ それはミミックだ！", "Wait! That's a mimic!"));
	tp->t_disguise = 'M';
	did_hit = thrown;
    }
    if (did_hit)
    {
	register char *mname;

	did_hit = FALSE;
	if (on(player, ISBLIND))
	    mname = (char *) JSTR("何か", "it");
	else
	    mname = monsters[mn-'A'].m_name;
	if (roll_em(&pstats, &tp->t_stats, weap, thrown))
	{
	    did_hit = TRUE;
	    if (thrown)
		thunk(weap, mname);
	    else
		hit(NULL, mname);
	    if (on(player, CANHUH))
	    {
		msg(JSTR("手の赤い光がおさまった。", "Your hands stop glowing red"));
		msg(JSTR("%sは混乱したようだ。", "The %s appears confused."), mname);
		tp->t_flags |= ISHUH;
		player.t_flags &= ~CANHUH;
	    }
	    if (tp->t_stats.s_hpt <= 0)
		killed(item, TRUE);
	}
	else
	    if (thrown)
		bounce(weap, mname);
	    else
		miss(NULL, mname);
    }
    count = 0;
    return did_hit;
}

/*
 * attack:
 *	The monster attacks the player
 */

int attack(struct thing *mp)
{
    register char *mname;
    bool monster_removed = FALSE;

    /*
     * Since this is an attack, stop running and any healing that was
     * going on at the time.
     */
    running = FALSE;
    quiet = 0;
    if (mp->t_type == 'M' && off(player, ISBLIND))
	mp->t_disguise = 'M';
    if (on(player, ISBLIND))
	mname = (char *) JSTR("何か", "it");
    else
	mname = monsters[mp->t_type-'A'].m_name;
    if (roll_em(&mp->t_stats, &pstats, NULL, FALSE))
    {
	if (mp->t_type != 'E')
	    hit(mname, NULL);
	if (pstats.s_hpt <= 0)
	    death(mp->t_type);	/* Bye bye life ... */
	if (off(*mp, ISCANC))
	    switch (mp->t_type)
	    {
		when 'R':
		    /*
		     * If a rust monster hits, you lose armor
		     */
		    if (cur_armor != NULL && cur_armor->o_ac < 9)
		    {
			if (!terse)
			    msg(JSTR("よろいがもろくなったようだ。なんてこった！", "Your armor appears to be weaker now. Oh my!"));
			else
			    msg(JSTR("よろいがもろくなった。", "Your armor weakens"));
			cur_armor->o_ac++;
		    }
		when 'E':
		    /*
		     * The gaze of the floating eye hypnotizes you
		     */
		    if (on(player, ISBLIND))
			break;
		    if (!no_command)
		    {
			addmsg(JSTR("あなたは身動きできない。", "You are transfixed"));
			if (!terse)
			    addmsg(JSTR("浮遊する目の視線に釘付けになった。", " by the gaze of the floating eye."));
			endmsg();
		    }
		    no_command += rnd(2)+2;
		when 'A':
		    /*
		     * Ants have poisonous bites
		     */
		    if (!save(VS_POISON))
		    {
			if (!ISWEARING(R_SUSTSTR))
			{
			    chg_str(-1);
			    if (!terse)
				msg(JSTR("腕に刺すような痛みを感じ、力が弱くなった。", "You feel a sting in your arm and now feel weaker"));
			    else
				msg(JSTR("刺すような痛みで力が弱くなった。", "A sting has weakened you"));
			}
			else
			{
			    if (!terse)
				msg(JSTR("刺すような痛みで一瞬力が弱くなった。", "A sting momentarily weakens you"));
			    else
				msg(JSTR("刺すような痛みは効果がなかった。", "Sting has no effect"));
			}
		    }
		when 'W':
		    /*
		     * Wraiths might drain energy levels
		     */
		    if (rnd(100) < 15)
		    {
			int fewer;

			if (pstats.s_exp == 0)
			    death('W');		/* All levels gone */
			msg(JSTR("急に弱くなったようだ。", "You suddenly feel weaker."));
			if (--pstats.s_lvl == 0)
			{
			    pstats.s_exp = 0;
			    pstats.s_lvl = 1;
			}
			else
			    pstats.s_exp = e_levels[pstats.s_lvl-1]+1;
			fewer = roll(1, 10);
			pstats.s_hpt -= fewer;
			max_hp -= fewer;
			if (pstats.s_hpt < 1)
			    pstats.s_hpt = 1;
			if (max_hp < 1)
			    death('W');
		    }
		when 'F':
		    /*
		     * Violet fungi stops the poor guy from moving
		     */
		    player.t_flags |= ISHELD;
		    snprintf(fungi_damage, sizeof fungi_damage, "%dd1", ++fung_hit);
		    mp->t_stats.s_dmg = fungi_damage;
		when 'L':
		{
		    /*
		     * Leperachaun steals some gold
		     */
		    register long lastpurse;

		    lastpurse = purse;
		    purse -= GOLDCALC;
		    if (!save(VS_MAGIC))
			purse -= GOLDCALC + GOLDCALC + GOLDCALC + GOLDCALC;
		    if (purse < 0)
			purse = 0;
		    if (purse != lastpurse)
			msg(JSTR("財布が軽くなった気がするぞ。", "Your purse feels lighter"));
		    {
			struct linked_list *gone;

			gone = find_mons(mp->t_pos.y, mp->t_pos.x);
			if (gone != NULL)
			{
			    remove(&mp->t_pos, gone);
			    monster_removed = TRUE;
			}
		    }
		}
		when 'N':
		{
		    register struct linked_list *list, *steal;
		    register struct object *obj;
		    register int nobj;

		    /*
		     * Nymph's steal a magic item, look through the pack
		     * and pick out one we like.
		     */
		    steal = NULL;
		    for (nobj = 0, list = pack; list != NULL; list = next(list))
		    {
			obj = (struct object *) ldata(list);
			if (obj != cur_armor && obj != cur_weapon &&
			    is_magic(obj) && rnd(++nobj) == 0)
				steal = list;
		    }
		    if (steal != NULL)
		    {
			register struct object *obj;

			obj = (struct object *) ldata(steal);
			{
			    struct linked_list *gone;

			    gone = find_mons(mp->t_pos.y, mp->t_pos.x);
			    if (gone != NULL)
			    {
				remove(&mp->t_pos, gone);
				monster_removed = TRUE;
			    }
			}
			if (obj->o_count > 1 && obj->o_group == 0)
			{
			    register int oc;

			    oc = obj->o_count--;
			    obj->o_count = 1;
			    msg(JSTR("彼女は%sを盗んでいった！", "She stole %s!"), inv_name(obj, TRUE));
			    obj->o_count = oc;
			}
			else
			{
			    msg(JSTR("彼女は%sを盗んでいった！", "She stole %s!"), inv_name(obj, TRUE));
			    detach(pack, steal);
			    discard(steal);
			}
			inpack--;
		    }
		}
		otherwise:
		    break;
	    }
    }
    else if (mp->t_type != 'E')
    {
	if (mp->t_type == 'F')
	{
	    pstats.s_hpt -= fung_hit;
	    if (pstats.s_hpt <= 0)
		death(mp->t_type);	/* Bye bye life ... */
	}
	miss(mname, NULL);
    }
    /*
     * Check to see if this is a regenerating monster and let it heal if
     * it is.
     */
    if (!monster_removed && on(*mp, ISREGEN) && rnd(100) < 33)
	mp->t_stats.s_hpt++;
    if (fight_flush)
    {
	rogue_flush_typeahead();
    }
    count = 0;
    status();
    return !monster_removed;
}

/*
 * swing:
 *	returns true if the swing hits
 */

int swing(int at_lvl, int op_arm, int wplus)
{
    register int res = rnd(20)+1;
    register int need = (21-at_lvl)-op_arm;

    return (res+wplus >= need);
}

/*
 * check_level:
 *	Check to see if the guy has gone up a level.
 */

void check_level(void)
{
    register int i, add;

    for (i = 0; e_levels[i] != 0; i++)
	if (e_levels[i] > pstats.s_exp)
	    break;
    i++;
    if (i > pstats.s_lvl)
    {
	add = roll(i-pstats.s_lvl,10);
	max_hp += add;
	if ((pstats.s_hpt += add) > max_hp)
	    pstats.s_hpt = max_hp;
	msg(JSTR("レベル%dへようこそ。", "Welcome to level %d"), i);
    }
    pstats.s_lvl = i;
}

/*
 * roll_em:
 *	Roll several attacks
 */

int roll_em(struct stats *att, struct stats *def, struct object *weap, bool hurl)
{
    register char *cp;
    register int ndice, nsides, def_arm;
    register bool did_hit = FALSE;
    register int prop_hplus, prop_dplus;

    prop_hplus = prop_dplus = 0;
    if (weap == NULL)
	cp = att->s_dmg;
    else if (hurl)
	if ((weap->o_flags&ISMISL) && cur_weapon != NULL &&
	  cur_weapon->o_which == weap->o_launch)
	{
	    cp = weap->o_hurldmg;
	    prop_hplus = cur_weapon->o_hplus;
	    prop_dplus = cur_weapon->o_dplus;
	}
	else
	    cp = (weap->o_flags&ISMISL ? weap->o_damage : weap->o_hurldmg);
    else
    {
	cp = weap->o_damage;
	/*
	 * Drain a staff of striking
	 */
	if (weap->o_type == STICK && weap->o_which == WS_HIT
	    && weap->o_charges == 0)
		{
		    weap->o_damage = "0d0";
		    weap->o_hplus = weap->o_dplus = 0;
		}
    }
    for (;;)
    {
	int damage;
	int hplus = prop_hplus + (weap == NULL ? 0 : weap->o_hplus);
	int dplus = prop_dplus + (weap == NULL ? 0 : weap->o_dplus);

	if (weap == cur_weapon)
	{
	    if (ISRING(LEFT, R_ADDDAM))
		dplus += cur_ring[LEFT]->o_ac;
	    else if (ISRING(LEFT, R_ADDHIT))
		hplus += cur_ring[LEFT]->o_ac;
	    if (ISRING(RIGHT, R_ADDDAM))
		dplus += cur_ring[RIGHT]->o_ac;
	    else if (ISRING(RIGHT, R_ADDHIT))
		hplus += cur_ring[RIGHT]->o_ac;
	}
	ndice = atoi(cp);
	if ((cp = strchr(cp, 'd')) == NULL)
	    break;
	nsides = atoi(++cp);
	if (def == &pstats)
	{
	    if (cur_armor != NULL)
		def_arm = cur_armor->o_ac;
	    else
		def_arm = def->s_arm;
	    if (ISRING(LEFT, R_PROTECT))
		def_arm -= cur_ring[LEFT]->o_ac;
	    else if (ISRING(RIGHT, R_PROTECT))
		def_arm -= cur_ring[RIGHT]->o_ac;
	}
	else
	    def_arm = def->s_arm;
	if (swing(att->s_lvl, def_arm, hplus+str_plus(&att->s_str)))
	{
	    register int proll;

	    proll = roll(ndice, nsides);
	    if (ndice + nsides > 0 && proll < 1)
		debug(JSTR("%dd%d のダメージは %d になった。", "Damage for %dd%d came out %d."), ndice, nsides, proll);
	    damage = dplus + proll + add_dam(&att->s_str);
	    def->s_hpt -= max(0, damage);
	    did_hit = TRUE;
	}
	if ((cp = strchr(cp, '/')) == NULL)
	    break;
	cp++;
    }
    return did_hit;
}

/*
 * prname:
 *	The print name of a combatant
 */

char * prname(char *who, bool upper)
{
    static char tbuf[80];

    *tbuf = '\0';
    if (jrogue_lang_is_japanese)
    {
	if (who == 0)
	    snprintf(tbuf, sizeof tbuf, "あなた");
	else if (strcmp(who, "it") == 0 || on(player, ISBLIND))
	    snprintf(tbuf, sizeof tbuf, "何か");
	else
	    snprintf(tbuf, sizeof tbuf, "%s", who);
    }
    else
    {
	if (who == 0)
	    snprintf(tbuf, sizeof tbuf, "you"); 
	else if (on(player, ISBLIND))
	    snprintf(tbuf, sizeof tbuf, "it");
	else
	    snprintf(tbuf, sizeof tbuf, "the %s", who);
	if (upper)
	    *tbuf = toupper((unsigned char)*tbuf);
    }
    return tbuf;
}

/*
 * hit:
 *	Print a message to indicate a succesful hit
 */

void hit(char *er, char *ee)
{
    register char *s = " hit ";

    if (jrogue_lang_is_japanese)
    {
	addmsg("%s", prname(er, TRUE));
	if (terse)
	    addmsg("の攻撃は命中した。");
	else
	{
	    addmsg("は%sに", prname(ee, FALSE));
	    switch (rnd(4))
	    {
		when 0: s = "激しい一撃を食らわせた。";
		when 1: s = "攻撃し命中した。";
		when 2: s = "傷を負わせた。";
		when 3: s = "殴りかかり命中した。";
	    }
	    addmsg("%s", s);
	}
	endmsg();
	return;
    }

    addmsg(prname(er, TRUE));
    if (terse)
	s = " hit.";
    else
	switch (rnd(4))
	{
	    when 0: s = " scored an excellent hit on ";
	    when 1: s = " hit ";
	    when 2: s = (er == 0 ? " have injured " : " has injured ");
	    when 3: s = (er == 0 ? " swing and hit " : " swings and hits ");
	}
    addmsg(s);
    if (!terse)
	addmsg(prname(ee, FALSE));
    endmsg();
}

/*
 * miss:
 *	Print a message to indicate a poor swing
 */

void miss(char *er, char *ee)
{
    register char *s = (er == 0 ? " miss" : " misses");

    if (jrogue_lang_is_japanese)
    {
	addmsg("%sは", prname(er, TRUE));
	if (!terse)
	    addmsg("%sに", prname(ee, FALSE));
	switch (terse ? 0 : rnd(4))
	{
	    when 0: s = "攻撃したが外れた。";
	    when 1: s = "殴りかかったが外れた。";
	    when 2: s = "攻撃したがぎりぎりでそれた。";
	    when 3: s = "攻撃したが当たらなかった。";
	}
	addmsg("%s", s);
	endmsg();
	return;
    }

    addmsg(prname(er, TRUE));
    switch (terse ? 0 : rnd(4))
    {
	when 0: s = (er == 0 ? " miss" : " misses");
	when 1: s = (er == 0 ? " swing and miss" : " swings and misses");
	when 2: s = (er == 0 ? " barely miss" : " barely misses");
	when 3: s = (er == 0 ? " don't hit" : " doesn't hit");
    }
    addmsg(s);
    if (!terse)
	addmsg(" %s", prname(ee, FALSE));
    endmsg();
}

/*
 * save_throw:
 *	See if a creature save against something
 */
int save_throw(int which, struct thing *tp)
{
    register int need;

    if (tp == NULL)
	return TRUE;
    need = 14 + which - tp->t_stats.s_lvl / 2;
    return (roll(1, 20) >= need);
}
/*
 * save:
 *	See if he saves against various nasty things
 */

int save(int which)
{
    return save_throw(which, &player);
}

/*
 * str_plus:
 *	compute bonus/penalties for strength on the "to hit" roll
 */

int str_plus(str_t *str)
{
    if (str->st_str == 18)
    {
	if (str->st_add == 100)
	    return 3;
	if (str->st_add > 50)
	    return 2;
    }
    if (str->st_str >= 17)
	return 1;
    if (str->st_str > 6)
	return 0;
    return str->st_str - 7;
}

/*
 * add_dam:
 *	compute additional damage done for exceptionally high or low strength
 */

int add_dam(str_t *str)
 {
    if (str->st_str == 18)
    {
	if (str->st_add == 100)
	    return 6;
	if (str->st_add > 90)
	    return 5;
	if (str->st_add > 75)
	    return 4;
	if (str->st_add != 0)
	    return 3;
	return 2;
    }
    if (str->st_str > 15)
	return 1;
    if (str->st_str > 6)
	return 0;
    return str->st_str - 7;
}

/*
 * raise_level:
 *	The guy just magically went up a level.
 */

void raise_level(void)
{
    pstats.s_exp = e_levels[pstats.s_lvl-1] + 1L;
    check_level();
}

/*
 * thunk:
 *	A missile hits a monster
 */

void thunk(struct object *weap, char *mname)
{
    if (weap->o_type == WEAPON)
	msg(JSTR("%sは%sに命中した。", "The %s hits the %s"),
	    w_names[weap->o_which], mname);
    else
	msg(JSTR("%sに命中した。", "You hit the %s."), mname);
}

/*
 * bounce:
 *	A missile misses a monster
 */

void bounce(struct object *weap, char *mname)
{
    if (weap->o_type == WEAPON)
	msg(JSTR("%sはそれて、%sには当たらなかった。",
	    "The %s misses the %s"), w_names[weap->o_which], mname);
    else
	msg(JSTR("%sには当たらなかった。", "You missed the %s."), mname);
}

/*
 * remove a monster from the screen
 */
void remove(coord *mp, struct linked_list *item)
{
    struct thing *tp;

    if (mp == NULL || item == NULL)
	return;
    tp = (struct thing *) ldata(item);
    mvwaddch(mw, mp->y, mp->x, ' ');
    if (tp != NULL)
	mvwaddch(cw, mp->y, mp->x, tp->t_oldch);
    detach(mlist, item);
    discard(item);
}

/*
 * is_magic:
 *	Returns true if an object radiates magic
 */

int is_magic(struct object *obj)
{
    switch (obj->o_type)
    {
	case ARMOR:
	    return obj->o_ac != a_class[obj->o_which];
	when WEAPON:
	    return obj->o_hplus != 0 || obj->o_dplus != 0;
	when POTION:
	case SCROLL:
	case STICK:
	case RING:
	case AMULET:
	    return TRUE;
    }
    return FALSE;
}

/*
 * killed:
 *	Called to put a monster to death
 */

void killed(struct linked_list *item, bool pr)
{
    register struct thing *tp;
    register struct linked_list *pitem, *nexti;
    coord monster_pos;

    tp = (struct thing *) ldata(item);
    if (pr)
    {
	if (on(player, ISBLIND))
	    msg(JSTR("何かを倒した。",
		terse ? "Defeated it." : "You have defeated it."));
	else
	    msg(JSTR("%sを倒した。",
		terse ? "Defeated %s." : "You have defeated the %s."),
		monsters[tp->t_type-'A'].m_name);
    }
    pstats.s_exp += tp->t_stats.s_exp;
    /*
     * Do adjustments if he went up a level
     */
    check_level();
    /*
     * If the monster was a violet fungi, un-hold him
     */
    switch (tp->t_type)
    {
	when 'F':
	    player.t_flags &= ~ISHELD;
	    fung_hit = 0;
	    strcpy(fungi_damage, "000d0");
	    monsters['F'-'A'].m_stats.s_dmg = fungi_damage;
	when 'L':
	{
	    register struct room *rp;

	    if ((rp = roomin(&tp->t_pos)) == NULL)
		break;
	    if (rp->r_goldval != 0 || fallpos(&tp->t_pos,&rp->r_gold,FALSE))
	    {
		rp->r_goldval += GOLDCALC;
		if (save(VS_MAGIC))
		    rp->r_goldval += GOLDCALC + GOLDCALC
				   + GOLDCALC + GOLDCALC;
		mvwaddch(stdscr, rp->r_gold.y, rp->r_gold.x, GOLD);
		if (!(rp->r_flags & ISDARK))
		{
		    light(&hero);
		    mvwaddch(cw, hero.y, hero.x, PLAYER);
		}
	    }
	}
    }
    /*
     * Empty the monster's pack.
     *
     * The original code removed and freed the monster first, then used
     * tp->t_pack and tp->t_pos while dropping carried treasure.  That is a
     * use-after-free on modern malloc implementations and can crash when a
     * carrying monster, such as a centaur, is killed.  Save the position and
     * detach the pack before freeing the monster record.
     */
    monster_pos = tp->t_pos;
    pitem = tp->t_pack;
    tp->t_pack = NULL;
    /*
     * Get rid of the monster.
     */
    remove(&monster_pos, item);
    while (pitem != NULL)
    {
	register struct object *obj;

	nexti = next(pitem);
	pitem->l_next = NULL;
	pitem->l_prev = NULL;
	obj = (struct object *) ldata(pitem);
	obj->o_pos = monster_pos;
	fall(pitem, FALSE);
	pitem = nexti;
    }
}
