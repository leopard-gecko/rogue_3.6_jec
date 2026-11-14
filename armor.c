/*
 * This file contains misc functions for dealing with armor
 * @(#)armor.c	3.9 (Berkeley) 6/15/81
 */

#include <curses.h>
#include "rogue.h"

/*
 * wear:
 *	The player wants to wear something, so let him/her put it on.
 */

void wear(void)
{
    register struct linked_list *item;
    register struct object *obj;

    if (cur_armor != NULL)
    {
	if (terse)
	    msg(JSTR("他のよろいを着ている。", "You are already wearing some"));
	else
	    msg(JSTR("すでに何かよろいを着ている。まず脱がなければならない。", "You are already wearing some.  You'll have to take it off first"));
	after = FALSE;
	return;
    }
    if ((item = get_item("wear", ARMOR)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != ARMOR)
    {
	msg(JSTR("それはよろいではない。", "You can't wear that."));
	return;
    }
    waste_time();
    if (!terse)
	msg(JSTR("%sを身につけた。", "You are now wearing %s."), a_names[obj->o_which]);
    else
	msg(JSTR("%sを身につけた。", "Wearing %s."), a_names[obj->o_which]);
    cur_armor = obj;
    obj->o_flags |= ISKNOW;
}

/*
 * take_off:
 *	Get the armor off of the players back
 */

void take_off(void)
{
    register struct object *obj;

    if ((obj = cur_armor) == NULL)
    {
	if (terse)
		msg(JSTR("よろいを着ていない。", "Not wearing armor"));
	else
		msg(JSTR("あなたはよろいを着ていない。", "You aren't wearing any armor"));
	return;
    }
    if (!dropcheck(cur_armor))
	return;
    cur_armor = NULL;
    if (terse)
	msg(JSTR("%c) %sを脱いだ。", "Was wearing %c) %s"),
	    pack_char(obj), inv_name(obj, TRUE));
    else
	msg(JSTR("今まで着ていた%c) %sを脱いだ。",
	    "You used to be wearing %c) %s"),
	    pack_char(obj), inv_name(obj, TRUE));
}

/*
 * waste_time:
 *	Do nothing but let other things happen
 */

void waste_time(void)
{
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    do_daemons(AFTER);
    do_fuses(AFTER);
}
