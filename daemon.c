/*
 * Contains functions for dealing with things that happen in the
 * future.
 *
 * @(#)daemon.c	3.3 (Berkeley) 6/15/81
 */

#include <curses.h>
#include "rogue.h"

#define EMPTY 0
#define DAEMON -1
#define MAXDAEMONS 20

#define _X_ { EMPTY }

struct delayed_action {
    int d_type;
    void (*d_func)(int);
    int d_arg;
    int d_time;
} d_list[MAXDAEMONS] = {
    _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
    _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, 
};

/*
 * d_slot:
 *	Find an empty slot in the daemon/fuse list
 */
struct delayed_action * d_slot(void)
{
    register int i;
    register struct delayed_action *dev;

    for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
	if (dev->d_type == EMPTY)
	    return dev;
    debug(JSTR("フューズスロットが足りない。", "Ran out of fuse slots"));
    return NULL;
}

/*
 * find_slot:
 *	Find a particular slot in the table
 */

struct delayed_action * find_slot(void (*func)(int))
{
    register int i;
    register struct delayed_action *dev;

    for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
	if (dev->d_type != EMPTY && func == dev->d_func)
	    return dev;
    return NULL;
}

/*
 * daemon:
 *	Start a daemon, takes a function.
 */

void daemon(void (*func)(int), int arg, int type)
{
    register struct delayed_action *dev;

    dev = d_slot();
    dev->d_type = type;
    dev->d_func = func;
    dev->d_arg = arg;
    dev->d_time = DAEMON;
}

/*
 * kill_daemon:
 *	Remove a daemon from the list
 */

void kill_daemon(void (*func)(int))
{
    register struct delayed_action *dev;

    if ((dev = find_slot(func)) == NULL)
	return;
    /*
     * Take it out of the list
     */
    dev->d_type = EMPTY;
}

/*
 * do_daemons:
 *	Run all the daemons that are active with the current flag,
 *	passing the argument to the function.
 */

void do_daemons(int flag)
{
    register struct delayed_action *dev;

    /*
     * Loop through the devil list
     */
    for (dev = d_list; dev < &d_list[MAXDAEMONS]; dev++)
	/*
	 * Executing each one, giving it the proper arguments
	 */
	if (dev->d_type == flag && dev->d_time == DAEMON)
	    (*dev->d_func)(dev->d_arg);
}

/*
 * fuse:
 *	Start a fuse to go off in a certain number of turns
 */

void fuse(void (*func)(int), int arg, int time, int type)
{
    register struct delayed_action *wire;

    wire = d_slot();
    wire->d_type = type;
    wire->d_func = func;
    wire->d_arg = arg;
    wire->d_time = time;
}

/*
 * lengthen:
 *	Increase the time until a fuse goes off
 */

void lengthen(void (*func)(int), int xtime)
{
     register struct delayed_action *wire;

    if ((wire = find_slot(func)) == NULL)
	return;
    wire->d_time += xtime;
}

/*
 * extinguish:
 *	Put out a fuse
 */

void extinguish(void (*func)(int))
{
    register struct delayed_action *wire;

    if ((wire = find_slot(func)) == NULL)
	return;
    wire->d_type = EMPTY;
}

/*
 * do_fuses:
 *	Decrement counters and start needed fuses
 */

void do_fuses(int flag)
{
    register struct delayed_action *wire;

    /*
     * Step though the list
     */
    for (wire = d_list; wire < &d_list[MAXDAEMONS]; wire++)
    {
	/*
	 * Decrementing counters and starting things we want.  We also need
	 * to remove the fuse from the list once it has gone off.
	 */
	if (flag == wire->d_type && wire->d_time > 0 && --wire->d_time == 0)
	{
	    wire->d_type = EMPTY;
	    (*wire->d_func)(wire->d_arg);
	}
     }
}

static int daemon_func_id(void (*func)(int))
{
    if (func == doctor) return 1;
    if (func == swander) return 2;
    if (func == rollwand) return 3;
    if (func == stomach) return 4;
    if (func == runners) return 5;
    if (func == unconfuse) return 6;
    if (func == unsee) return 7;
    if (func == sight) return 8;
    if (func == nohaste) return 9;
    return 0;
}

static void (*daemon_id_func(int id))(int)
{
    switch (id)
    {
        case 1: return doctor;
        case 2: return swander;
        case 3: return rollwand;
        case 4: return stomach;
        case 5: return runners;
        case 6: return unconfuse;
        case 7: return unsee;
        case 8: return sight;
        case 9: return nohaste;
        default: return NULL;
    }
}

int rogue_save_daemons(FILE *fp)
{
    int i, id;

    for (i = 0; i < MAXDAEMONS; i++)
    {
        id = daemon_func_id(d_list[i].d_func);
        if (fwrite(&d_list[i].d_type, sizeof d_list[i].d_type, 1, fp) != 1) return FALSE;
        if (fwrite(&id, sizeof id, 1, fp) != 1) return FALSE;
        if (fwrite(&d_list[i].d_arg, sizeof d_list[i].d_arg, 1, fp) != 1) return FALSE;
        if (fwrite(&d_list[i].d_time, sizeof d_list[i].d_time, 1, fp) != 1) return FALSE;
    }
    return TRUE;
}

int rogue_restore_daemons(FILE *fp)
{
    int i, id;

    for (i = 0; i < MAXDAEMONS; i++)
    {
        if (fread(&d_list[i].d_type, sizeof d_list[i].d_type, 1, fp) != 1) return FALSE;
        if (fread(&id, sizeof id, 1, fp) != 1) return FALSE;
        if (fread(&d_list[i].d_arg, sizeof d_list[i].d_arg, 1, fp) != 1) return FALSE;
        if (fread(&d_list[i].d_time, sizeof d_list[i].d_time, 1, fp) != 1) return FALSE;
        d_list[i].d_func = daemon_id_func(id);
        if (d_list[i].d_type != EMPTY && d_list[i].d_func == NULL)
            d_list[i].d_type = EMPTY;
    }
    return TRUE;
}
