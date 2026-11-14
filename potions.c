/*
 * 	@(#)potions.c	3.1	3.1	5/7/81
 * Function(s) for dealing with potions
 */

#include <curses.h>
#include "rogue.h"

void quaff(void)
{
    register struct object *obj;
    register struct linked_list *item, *titem;
    register struct thing *th;
    char buf[ROGUE_INPUT_SIZE];

    item = get_item("quaff", POTION);
    /*
     * Make certain that it is somethings that we want to drink
     */
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != POTION)
    {
	if (!terse)
	    msg(JSTR("おえっ。なんでそんなものを飲もうとするの？", "Yuk! Why would you want to drink that?"));
	else
	    msg(JSTR("それは飲めない。", "That's undrinkable"));
	return;
    }
    if (obj == cur_weapon)
	cur_weapon = NULL;

    /*
     * Calculate the effect it has on the poor guy.
     */
    switch(obj->o_which)
    {
	when P_CONFUSE:
	    if (off(player, ISHUH))
	    {
		msg(JSTR("ちょっと待った、何が起こったんだ？え？なに？だれ？", "Wait, what's going on here. Huh? What? Who?"));
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(8)+HUHDURATION);
		else
		    fuse(unconfuse, 0, rnd(8)+HUHDURATION, AFTER);
		player.t_flags |= ISHUH;
	    }
	    p_know[P_CONFUSE] = TRUE;
	when P_POISON:
	    if (!ISWEARING(R_SUSTSTR))
	    {
		chg_str(-(rnd(3)+1));
		msg(JSTR("すごく具合が悪くなった。", "You feel very sick now."));
	    }
	    else
		msg(JSTR("具合が悪くなったが、すぐに治った。", "You feel momentarily sick"));
	    p_know[P_POISON] = TRUE;
	when P_HEALING:
	    if ((pstats.s_hpt += roll(pstats.s_lvl, 4)) > max_hp)
		pstats.s_hpt = ++max_hp;
	    msg(JSTR("気分がよくなってきた。", "You begin to feel better."));
	    sight(0);
	    p_know[P_HEALING] = TRUE;
	when P_STRENGTH:
	    msg(JSTR("強くなった気がするぞ。このはちきれそうな筋肉！", "You feel stronger, now.  What bulging muscles!"));
	    chg_str(1);
	    p_know[P_STRENGTH] = TRUE;
	when P_MFIND:
	    /*
	     * Potion of monster detection, if there are monters, detect them
	     */
	    if (mlist != NULL)
	    {
		rogue_vt_clear_window(hw);
		wclear(hw);
		overwrite(mw, hw);
		show_win(hw,
		    (char *) JSTR("怪物の存在を感じ始めた。--続く--",
		         "You begin to sense the presence of monsters.--More--"));
		p_know[P_MFIND] = TRUE;
	    }
	    else
		msg(JSTR("変な気分におそわれたが、すぐに消えていった。", "You have a strange feeling for a moment, then it passes."));
	when P_TFIND:
	    /*
	     * Potion of magic detection.  Show the potions and scrolls
	     */
	    if (lvl_obj != NULL)
	    {
		struct linked_list *mobj;
		struct object *tp;
		bool show;

		show = FALSE;
		rogue_vt_clear_window(hw);
		wclear(hw);
		for (mobj = lvl_obj; mobj != NULL; mobj = next(mobj))
		{
		    tp = (struct object *) ldata(mobj);
		    if (is_magic(tp))
		    {
			show = TRUE;
			mvwaddch(hw, tp->o_pos.y, tp->o_pos.x, MAGIC);
		    }
		    p_know[P_TFIND] = TRUE;
		}
		for (titem = mlist; titem != NULL; titem = next(titem))
		{
		    register struct linked_list *pitem;

		    th = (struct thing *) ldata(titem);
		    for (pitem = th->t_pack; pitem != NULL; pitem = next(pitem))
		    {
			if (is_magic((struct object *) ldata(pitem)))
			{
			    show = TRUE;
			    mvwaddch(hw, th->t_pos.y, th->t_pos.x, MAGIC);
			}
			p_know[P_TFIND] = TRUE;
		    }
		}
		if (show)
		{
		    show_win(hw, 
			(char *) JSTR("この階で魔法の存在を感じた。--続く--",
			     "You sense the presence of magic on this level.--More--"));
		    break;
		}
	    }
	    msg(JSTR("変な気分におそわれたが、すぐに消えていった。", "You have a strange feeling for a moment, then it passes."));
	when P_PARALYZE:
	    msg(JSTR("動けない。", "You can't move."));
	    no_command = HOLDTIME;
	    p_know[P_PARALYZE] = TRUE;
	when P_SEEINVIS:
	    msg(JSTR("この水薬は、%sジュースのような味がする。", "This potion tastes like %s juice."), fruit);
	    if (off(player, CANSEE))
	    {
		player.t_flags |= CANSEE;
		fuse(unsee, 0, SEEDURATION, AFTER);
		light(&hero);
	    }
	    sight(0);
	when P_RAISE:
	    msg(JSTR("突然に腕前がすごく上がったようだ。", "You suddenly feel much more skillful"));
	    p_know[P_RAISE] = TRUE;
	    raise_level();
	when P_XHEAL:
	    if ((pstats.s_hpt += roll(pstats.s_lvl, 8)) > max_hp)
		pstats.s_hpt = ++max_hp;
	    msg(JSTR("とても気分がよくなってきた。", "You begin to feel much better."));
	    p_know[P_XHEAL] = TRUE;
	    sight(0);
	when P_HASTE:
	    add_haste(TRUE);
	    msg(JSTR("動きが素早くなったようだ。", "You feel yourself moving much faster."));
	    p_know[P_HASTE] = TRUE;
	when P_RESTORE:
	    msg(JSTR("ああ、これはうまい。身体中があたたまる。", "Hey, this tastes great.  It make you feel warm all over."));
	    if (pstats.s_str.st_str < max_stats.s_str.st_str ||
		(pstats.s_str.st_str == 18 &&
		 pstats.s_str.st_add < max_stats.s_str.st_add))
	    pstats.s_str = max_stats.s_str;
	when P_BLIND:
	    msg(JSTR("深い暗闇のとばりがあたりをおおってゆく。", "A cloak of darkness falls around you."));
	    if (off(player, ISBLIND))
	    {
		player.t_flags |= ISBLIND;
		fuse(sight, 0, SEEDURATION, AFTER);
		look(FALSE);
	    }
	    p_know[P_BLIND] = TRUE;
	when P_NOP:
	    msg(JSTR("この水薬はひどく退屈な味がする。", "This potion tastes extremely dull."));
	otherwise:
	    msg(JSTR("なんてひどい味の水薬だ！", "What an odd tasting potion!"));
	    return;
    }
    status();
    if (p_know[obj->o_which] && p_guess[obj->o_which])
    {
	free(p_guess[obj->o_which]);
	p_guess[obj->o_which] = NULL;
    }
    else if (!p_know[obj->o_which] && askme && p_guess[obj->o_which] == NULL)
    {
	if (terse)
	    msg(JSTR("呼び名は？", "Call it: "));
	else
	    msg(JSTR("どんな呼び名をつける？", "What do you want to call it? "));
	if (get_str(buf, cw) == NORM)
	{
	    p_guess[obj->o_which] = rogue_strdup(buf);
	}
    }
    /*
     * Throw the item away
     */
    inpack--;
    if (obj->o_count > 1)
	obj->o_count--;
    else
    {
	detach(pack, item);
        discard(item);
    }
}
