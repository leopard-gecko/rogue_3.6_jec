/*
 * Functions for dealing with linked lists of goodies
 *
 * @(#)list.c	3.3 (Berkeley) 6/15/81
 */

#include <curses.h>
#include "rogue.h"

/*
 * detach:
 *	Takes an item out of whatever linked list it might be in
 */

void _detach(struct linked_list **list, struct linked_list *item)
{
    if (list == NULL || item == NULL)
	return;
    if (*list == item)
	*list = next(item);
    if (prev(item) != NULL) item->l_prev->l_next = next(item);
    if (next(item) != NULL) item->l_next->l_prev = prev(item);
    item->l_next = NULL;
    item->l_prev = NULL;
}

/*
 * _attach:
 *	add an item to the head of a list
 */

void _attach(struct linked_list **list, struct linked_list *item)
{
    if (*list != NULL)
    {
	item->l_next = *list;
	(*list)->l_prev = item;
	item->l_prev = NULL;
    }
    else
    {
	item->l_next = NULL;
	item->l_prev = NULL;
    }

    *list = item;
}

/*
 * _free_list:
 *	Throw the whole blamed thing away
 */

void _free_list(struct linked_list **ptr)
{
    register struct linked_list *item;

    while (*ptr != NULL)
    {
	item = *ptr;
	*ptr = next(item);
	discard(item);
    }
}

/*
 * discard:
 *	free up an item
 */

void discard(struct linked_list *item)
{
    if (item == NULL)
	return;
    total -= 2;
    if (item->l_data != NULL)
	FREE(item->l_data);
    FREE(item);
}

/*
 * new_item
 *	get a new item with a specified size
 */

struct linked_list * new_item(int size)
{
    register struct linked_list *item;

    if ((item = (struct linked_list *) new(sizeof *item)) == NULL)
	msg(JSTR("%d個のヘッダを確保した後、メモリが足りなくなった。", "Ran out of memory for header after %d items"), total);
    if ((item->l_data = new(size)) == NULL)
	msg(JSTR("%d個のデータを確保した後、メモリが足りなくなった。", "Ran out of memory for data after %d items"), total);
    memset(item->l_data, 0, (size_t) size);
    item->l_next = item->l_prev = NULL;
    return item;
}

char * new(int size)
{
    register char *space = ALLOC(size);

    if (space == NULL)
    {
	sprintf(prbuf, "Rogue ran out of memory after %d allocations.  Fatal error!", total);
	fatal(prbuf);
    }
    total++;
    return space;
}
