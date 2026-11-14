#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * Routines to deal with the pack
 *
 * @(#)pack.c	3.6 (Berkeley) 6/15/81
 */

/*
 * add_pack:
 *	Pick up an object and add it to the pack.  If the argument is non-null
 * use it as the linked_list pointer instead of gettting it off the ground.
 */
void add_pack(struct linked_list *item, bool silent)
{
    register struct linked_list *ip, *lp;
    register struct object *obj, *op;
    register bool exact, from_floor;

    if (item == NULL)
    {
	from_floor = TRUE;
	if ((item = find_obj(hero.y, hero.x)) == NULL)
	    return;
    }
    else
	from_floor = FALSE;
    obj = (struct object *) ldata(item);
    /*
     * Link it into the pack.  Search the pack for a object of similar type
     * if there isn't one, stuff it at the beginning, if there is, look for one
     * that is exactly the same and just increment the count if there is.
     * it  that.  Food is always put at the beginning for ease of access, but
     * is not ordered so that you can't tell good food from bad.  First check
     * to see if there is something in thr same group and if there is then
     * increment the count.
     */
    if (obj->o_group)
    {
	for (ip = pack; ip != NULL; ip = next(ip))
	{
	    op = (struct object *) ldata(ip);
	    if (op->o_group == obj->o_group)
	    {
		/*
		 * Put it in the pack and notify the user
		 */
		op->o_count++;
		if (from_floor)
		{
		    detach(lvl_obj, item);
		    mvaddch(hero.y, hero.x,
			(roomin(&hero) == NULL ? PASSAGE : FLOOR));
		}
		discard(item);
		item = ip;
		goto picked_up;
	    }
	}
    }
    /*
     * Check if there is room
     */
    if (inpack == MAXPACK-1)
    {
	msg(JSTR("もうこれ以上、物は持てない。", "You can't carry anything else."));
	return;
    }
    /*
     * Check for and deal with scare monster scrolls
     */
    if (obj->o_type == SCROLL && obj->o_which == S_SCARE)
    {
	if (obj->o_flags & ISFOUND)
	{
	    msg(JSTR("拾いあげたとたん、巻き物はちりになってしまった。", "The scroll turns to dust as you pick it up."));
	    detach(lvl_obj, item);
	    mvaddch(hero.y, hero.x, FLOOR);
	    return;
	}
	else
	    obj->o_flags |= ISFOUND;
    }

    inpack++;
    if (from_floor)
    {
	detach(lvl_obj, item);
	mvaddch(hero.y, hero.x, (roomin(&hero) == NULL ? PASSAGE : FLOOR));
    }
    /*
     * Search for an object of the same type
     */
    exact = FALSE;
    for (ip = pack; ip != NULL; ip = next(ip))
    {
	op = (struct object *) ldata(ip);
	if (obj->o_type == op->o_type)
	    break;
    }
    if (ip == NULL)
    {
	/*
	 * Put it at the end of the pack since it is a new type
	 */
	for (ip = pack; ip != NULL; ip = next(ip))
	{
	    op = (struct object *) ldata(ip);
	    if (op->o_type != FOOD)
		break;
	    lp = ip;
	}
    }
    else
    {
	/*
	 * Search for an object which is exactly the same
	 */
	while (ip != NULL && op->o_type == obj->o_type)
	{
	    if (op->o_which == obj->o_which)
	    {
		exact = TRUE;
		break;
	    }
	    lp = ip;
	    if ((ip = next(ip)) == NULL)
		break;
	    op = (struct object *) ldata(ip);
	}
    }
    if (ip == NULL)
    {
	/*
	 * Didn't find an exact match, just stick it here
	 */
	if (pack == NULL)
	    pack = item;
	else
	{
	    lp->l_next = item;
	    item->l_prev = lp;
	    item->l_next = NULL;
	}
    }
    else
    {
	/*
	 * If we found an exact match.  If it is a potion, food, or a 
	 * scroll, increase the count, otherwise put it with its clones.
	 */
	if (exact && ISMULT(obj->o_type))
	{
	    op->o_count++;
	    discard(item);
	    item = ip;
	    goto picked_up;
	}
	if ((item->l_prev = prev(ip)) != NULL)
	    item->l_prev->l_next = item;
	else
	    pack = item;
	item->l_next = ip;
	ip->l_prev = item;
    }
picked_up:
    /*
     * Notify the user
     */
    obj = (struct object *) ldata(item);
    if (notify && !silent)
    {
	if (!terse)
	    addmsg(JSTR("あなたは", "You now have "));
	msg(JSTR("%s (%c)を手に入れた。", "%s (%c)"),
	    inv_name(obj, !terse), pack_char(obj));
    }
    if (obj->o_type == AMULET)
	amulet = TRUE;
}

/*
 * inventory:
 *	list what is in the pack
 */
int inventory(struct linked_list *list, int type)
{
    register struct linked_list *item;
    register struct object *obj;
    register int n_objs;
    register char ch;
    char inv_temp[256];

    n_objs = 0;
    for (item = list; item != NULL; item = next(item))
    {
	obj = (struct object *) ldata(item);
	if (type && type != obj->o_type && !(type == CALLABLE &&
	    (obj->o_type == SCROLL || obj->o_type == POTION ||
	     obj->o_type == RING || obj->o_type == STICK)))
		continue;
	n_objs++;
    }

    if (n_objs == 0)
    {
	if (terse)
	    msg(type == 0 ? JSTR("何も持っていない。", "Empty handed.") :
			    JSTR("それは１つも持っていない。", "Nothing appropriate"));
	else
	    msg(type == 0 ? JSTR("何も持っていない。", "You are empty handed.") :
			    JSTR("それは１つも持っていない。", "You don't have anything appropriate"));
	return FALSE;
    }

    if (n_objs == 1)
    {
	for (ch = 'a', item = list; item != NULL; item = next(item), ch++)
	{
	    obj = (struct object *) ldata(item);
	    if (type && type != obj->o_type && !(type == CALLABLE &&
		(obj->o_type == SCROLL || obj->o_type == POTION ||
		 obj->o_type == RING || obj->o_type == STICK)))
		continue;
	    snprintf(inv_temp, sizeof inv_temp, "%c) %s", ch, inv_name(obj, FALSE));
	    msg("%s", inv_temp);
	    return TRUE;
	}
    }

    if (slow_invent)
    {
	for (ch = 'a', item = list; item != NULL; item = next(item), ch++)
	{
	    obj = (struct object *) ldata(item);
	    if (type && type != obj->o_type && !(type == CALLABLE &&
		(obj->o_type == SCROLL || obj->o_type == POTION ||
		 obj->o_type == RING || obj->o_type == STICK)))
		continue;
	    msg("%c) %s", ch, inv_name(obj, FALSE));
	}
	return TRUE;
    }

    /*
     * Clear any pending message before drawing the overlapping inventory
     * window, then build the list in one pass.
     */
    msg("");
    clearok(cw, TRUE);
    touchwin(cw);
    draw(cw);

    rogue_vt_clear_window(hw);
    wclear(hw);
    clearok(hw, TRUE);
    n_objs = 0;
    for (ch = 'a', item = list; item != NULL; item = next(item), ch++)
    {
	obj = (struct object *) ldata(item);
	if (type && type != obj->o_type && !(type == CALLABLE &&
	    (obj->o_type == SCROLL || obj->o_type == POTION ||
	     obj->o_type == RING || obj->o_type == STICK)))
		continue;
	wmove(hw, n_objs++, 0);
	wprintw(hw, "%c) ", ch);
	rogue_waddstr_utf8(hw, inv_name(obj, FALSE));
    }
    mvwaddstr(hw, ROGUE_SCREEN_LINES-1, 0, JSTR("--スペースを押してください--", "--Press space to continue--"));
    touchwin(hw);
    draw(hw);
    wait_for_window(hw, ' ');
    rogue_vt_clear_window(hw);
    wclear(hw);
    clearok(cw, TRUE);
    touchwin(cw);
    draw(cw);
    return TRUE;
}

/*
 * pick_up:
 *	Add something to characters pack.
 */
void pick_up(char ch)
{
    switch(ch)
    {
	case GOLD:
	    money();
	    break;
	default:
	    debug(JSTR("どこでそれを拾った？？？", "Where did you pick that up???"));
	    goto pick_up_item;
	case ARMOR:
	case POTION:
	case FOOD:
	case WEAPON:
	case SCROLL:	
	case AMULET:
	case RING:
	case STICK:
pick_up_item:
	    add_pack(NULL, FALSE);
	    break;
    }
}

/*
 * picky_inven:
 *	Allow player to inventory a single item
 */
void picky_inven(void)
{
    register struct linked_list *item;
    register char ch, mch;

    if (pack == NULL)
	msg(JSTR("何も持っていない。", "You aren't carrying anything"));
    else if (next(pack) == NULL)
	msg("a) %s", inv_name((struct object *) ldata(pack), FALSE));
    else
    {
	if (terse)
	    msg(JSTR("調べる持ちものは？", "Item: "));
	else
	    msg(JSTR("調べる持ちものの種類は？", "Which item do you wish to inventory: "));
	mpos = 0;
	if ((mch = readchar()) == ESCAPE)
	{
	    msg("");
	    return;
	}
	for (ch = 'a', item = pack; item != NULL; item = next(item), ch++)
	    if (ch == mch)
	    {
		msg("%c) %s",ch,inv_name((struct object *) ldata(item), FALSE));
		return;
	    }
	if (!terse)
	    msg(JSTR("'%s'は持っていない。", "'%s' not in pack"), unctrl(mch));
	msg(JSTR("範囲は 'a' から '%c' です。", "Range is 'a' to '%c'"), --ch);
    }
}

/*
 * get_item:
 *	pick something out of a pack for a purpose
 */
static const char *
get_item_purpose(const char *purpose)
{
    if (!jrogue_lang_is_japanese)
	return purpose;
    if (strcmp(purpose, "wear") == 0)
	return "よろいを身につける";
    if (strcmp(purpose, "wield") == 0)
	return "武器を装備する";
    if (strcmp(purpose, "throw") == 0)
	return "武器を投げる";
    if (strcmp(purpose, "drop") == 0)
	return "持ちものを落とす";
    if (strcmp(purpose, "quaff") == 0)
	return "水薬を飲む";
    if (strcmp(purpose, "read") == 0)
	return "巻き物を読む";
    if (strcmp(purpose, "eat") == 0)
	return "食べ物をたべる";
    if (strcmp(purpose, "put on") == 0)
	return "指輪をはめる";
    if (strcmp(purpose, "take off") == 0)
	return "外す";
    if (strcmp(purpose, "zap with") == 0)
	return "つえを振りかざす";
    if (strcmp(purpose, "charge") == 0)
	return "つえをチャージする";
    if (strcmp(purpose, "call") == 0)
	return "持ちものに呼び名をつける";
    if (strcmp(purpose, "identify") == 0)
	return "持ちものを識別する";
    return purpose;
}

struct linked_list * get_item(const char *purpose, int type)
{
    register struct linked_list *obj;
    register char ch, och;
    const char *localized_purpose;

    if (pack == NULL)
	msg(JSTR("何も持っていない。", "You aren't carrying anything."));
    else
    {
	for (;;)
	{
	    localized_purpose = get_item_purpose(purpose);
	    if (terse)
		msg(JSTR("どの%s？ ('*'でリスト表示) ",
		    "%s what? (* for list): "),
		    jrogue_lang_is_japanese ? localized_purpose : purpose);
	    else
		msg(JSTR("どの%s？ ('*'でリスト表示、ESCでキャンセル) ",
		    "Which object do you want to %s? (* for list): "),
		    jrogue_lang_is_japanese ? localized_purpose : purpose);
	    ch = readchar();
	    mpos = 0;
	    /*
	     * Give the poor player a chance to abort the command
	     */
	    if (ch == ESCAPE || ch == CTRL('G'))
	    {
		after = FALSE;
		msg("");
		return NULL;
	    }
	    if (ch == '*')
	    {
		mpos = 0;
		if (inventory(pack, type) == 0)
		{
		    after = FALSE;
		    return NULL;
		}
		continue;
	    }
	    for (obj = pack, och = 'a'; obj != NULL; obj = next(obj), och++)
		if (ch == och)
		    break;
	    if (obj == NULL)
	    {
		msg(JSTR("'a'から'%c'までの文字を指定してください。", "Please specify a letter between 'a' and '%c'"), och-1);
		continue;
	    }
	    else 
		return obj;
	}
    }
    return NULL;
}

int pack_char(struct object *obj)
{
    register struct linked_list *item;
    register char c;

    c = 'a';
    for (item = pack; item != NULL; item = next(item))
	if ((struct object *) ldata(item) == obj)
	    return c;
	else
	    c++;
    return 'z';
}
