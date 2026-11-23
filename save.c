/*
 * save and restore routines
 *
 * Modern macOS port note:
 * The original Rogue 3.6 save code depended on sbrk()/brk() and saved
 * process memory.  That is not reliable on modern 64-bit macOS, so this
 * file implements a one-use, structured suspend/resume save instead.
 */

#include <curses.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "rogue.h"

#ifdef _WIN32
#define rogue_save_fopen(path, mode) rogue_windows_fopen_utf8((path), (mode))
#define rogue_save_unlink(path) rogue_windows_unlink_utf8((path))
#else
#define rogue_save_fopen(path, mode) fopen((path), (mode))
#define rogue_save_unlink(path) unlink((path))
#endif

#define RS_MAGIC "ROGUE36SAVE-PORTABLE-5"
#define RS_MAXSTR 256

typedef struct stat STAT;
extern char version[], encstr[];
STAT sbuf;

struct save_stats {
    str_t s_str;
    long s_exp;
    int s_lvl;
    int s_arm;
    int s_hpt;
};

struct save_object {
    int o_type;
    coord o_pos;
    char o_launch;
    int o_count;
    int o_which;
    int o_hplus;
    int o_dplus;
    int o_ac;
    int o_flags;
    int o_group;
};

struct save_thing {
    coord t_pos;
    bool t_turn;
    char t_type;
    char t_disguise;
    char t_oldch;
    short t_flags;
    struct save_stats t_stats;
    int dest_kind;
    int dest_room;
};

static int write_bytes(FILE *fp, const void *buf, size_t len)
{
    return fwrite(buf, 1, len, fp) == len;
}

static int read_bytes(FILE *fp, void *buf, size_t len)
{
    return fread(buf, 1, len, fp) == len;
}

static int write_int(FILE *fp, int v)
{
    return write_bytes(fp, &v, sizeof v);
}

static int read_int(FILE *fp, int *v)
{
    return read_bytes(fp, v, sizeof *v);
}

static int write_string(FILE *fp, char *s)
{
    int len;

    if (s == NULL)
    {
	len = -1;
	return write_int(fp, len);
    }
    len = (int) strlen(s);
    if (!write_int(fp, len))
	return FALSE;
    return write_bytes(fp, s, (size_t) len);
}

static char *read_string(FILE *fp)
{
    int len;
    char *s;

    if (!read_int(fp, &len))
	return NULL;
    if (len < 0)
	return NULL;
    if (len > RS_MAXSTR)
	return NULL;
    s = new(len + 1);
    if (!read_bytes(fp, s, (size_t) len))
    {
	FREE(s);
	return NULL;
    }
    s[len] = '\0';
    return s;
}

static int list_count(struct linked_list *list)
{
    int n;

    n = 0;
    while (list != NULL)
    {
	n++;
	list = next(list);
    }
    return n;
}


static void append_item(struct linked_list **list, struct linked_list *item)
{
    struct linked_list *tail;

    item->l_next = NULL;
    item->l_prev = NULL;
    if (*list == NULL)
    {
	*list = item;
	return;
    }
    tail = *list;
    while (next(tail) != NULL)
	tail = next(tail);
    next(tail) = item;
    prev(item) = tail;
}

static int object_index(struct linked_list *list, struct object *obj)
{
    int n;

    n = 0;
    while (list != NULL)
    {
	if ((struct object *) ldata(list) == obj)
	    return n;
	n++;
	list = next(list);
    }
    return -1;
}

static struct object *object_at(struct linked_list *list, int index)
{
    int n;

    n = 0;
    while (list != NULL)
    {
	if (n == index)
	    return (struct object *) ldata(list);
	n++;
	list = next(list);
    }
    return NULL;
}

static void save_stats_from_stats(struct save_stats *dst, struct stats *src)
{
    dst->s_str = src->s_str;
    dst->s_exp = src->s_exp;
    dst->s_lvl = src->s_lvl;
    dst->s_arm = src->s_arm;
    dst->s_hpt = src->s_hpt;
}

static void stats_from_save_stats(struct stats *dst, struct save_stats *src, char *dmg)
{
    dst->s_str = src->s_str;
    dst->s_exp = src->s_exp;
    dst->s_lvl = src->s_lvl;
    dst->s_arm = src->s_arm;
    dst->s_hpt = src->s_hpt;
    dst->s_dmg = dmg;
}

static void save_object_from_object(struct save_object *dst, struct object *src)
{
    dst->o_type = src->o_type;
    dst->o_pos = src->o_pos;
    dst->o_launch = src->o_launch;
    dst->o_count = src->o_count;
    dst->o_which = src->o_which;
    dst->o_hplus = src->o_hplus;
    dst->o_dplus = src->o_dplus;
    dst->o_ac = src->o_ac;
    dst->o_flags = src->o_flags;
    dst->o_group = src->o_group;
}

static void object_from_save_object(struct object *dst, struct save_object *src)
{
    dst->o_type = src->o_type;
    dst->o_pos = src->o_pos;
    dst->o_text = NULL;
    dst->o_launch = src->o_launch;
    dst->o_damage = NULL;
    dst->o_hurldmg = NULL;
    dst->o_count = src->o_count;
    dst->o_which = src->o_which;
    dst->o_hplus = src->o_hplus;
    dst->o_dplus = src->o_dplus;
    dst->o_ac = src->o_ac;
    dst->o_flags = src->o_flags;
    dst->o_group = src->o_group;
}

static void save_thing_from_thing(struct save_thing *dst, struct thing *src)
{
    dst->t_pos = src->t_pos;
    dst->t_turn = src->t_turn;
    dst->t_type = src->t_type;
    dst->t_disguise = src->t_disguise;
    dst->t_oldch = src->t_oldch;
    dst->t_flags = src->t_flags;
    save_stats_from_stats(&dst->t_stats, &src->t_stats);
}

static void thing_from_save_thing(struct thing *dst, struct save_thing *src, char *dmg)
{
    dst->t_pos = src->t_pos;
    dst->t_turn = src->t_turn;
    dst->t_type = src->t_type;
    dst->t_disguise = src->t_disguise;
    dst->t_oldch = src->t_oldch;
    dst->t_dest = NULL;
    dst->t_flags = src->t_flags;
    stats_from_save_stats(&dst->t_stats, &src->t_stats, dmg);
    dst->t_pack = NULL;
}

static void fix_object_pointers(struct object *obj)
{
    obj->o_text = NULL;
    switch (obj->o_type)
    {
	case WEAPON:
	    if (obj->o_which >= 0 && obj->o_which < MAXWEAPONS)
		init_weapon(obj, obj->o_which);
	    break;
	case STICK:
	    if (obj->o_which >= 0 && obj->o_which < MAXSTICKS)
		fix_stick(obj);
	    break;
	default:
	    obj->o_damage = "0d0";
	    obj->o_hurldmg = "0d0";
	    obj->o_launch = 0;
	    break;
    }
}

static int write_object(FILE *fp, struct object *obj)
{
    struct save_object so;

    save_object_from_object(&so, obj);
    return write_bytes(fp, &so, sizeof so);
}

static int write_player(FILE *fp)
{
    struct save_thing st;

    save_thing_from_thing(&st, &player);
    st.dest_kind = 0;
    st.dest_room = -1;
    return write_bytes(fp, &st, sizeof st);
}

static int read_player(FILE *fp)
{
    struct save_thing st;

    if (!read_bytes(fp, &st, sizeof st))
	return FALSE;
    thing_from_save_thing(&player, &st, "1d4");
    return TRUE;
}

static int write_stats(FILE *fp, struct stats *st)
{
    struct save_stats ss;

    save_stats_from_stats(&ss, st);
    return write_bytes(fp, &ss, sizeof ss);
}

static int read_stats(FILE *fp, struct stats *st, char *dmg)
{
    struct save_stats ss;

    if (!read_bytes(fp, &ss, sizeof ss))
	return FALSE;
    stats_from_save_stats(st, &ss, dmg);
    return TRUE;
}

static struct linked_list *read_object_item(FILE *fp)
{
    struct save_object so;
    struct linked_list *item;
    struct object *obj;
    int saved_hplus, saved_dplus, saved_ac, saved_count, saved_flags, saved_group;
    char saved_launch;

    if (!read_bytes(fp, &so, sizeof so))
	return NULL;
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    object_from_save_object(obj, &so);
    saved_hplus = obj->o_hplus;
    saved_dplus = obj->o_dplus;
    saved_ac = obj->o_ac;
    saved_count = obj->o_count;
    saved_flags = obj->o_flags;
    saved_group = obj->o_group;
    saved_launch = obj->o_launch;
    fix_object_pointers(obj);
    obj->o_hplus = saved_hplus;
    obj->o_dplus = saved_dplus;
    obj->o_ac = saved_ac;
    obj->o_count = saved_count;
    obj->o_flags = saved_flags;
    obj->o_group = saved_group;
    obj->o_launch = saved_launch;
    return item;
}

static int write_object_list(FILE *fp, struct linked_list *list)
{
    int n;

    n = list_count(list);
    if (!write_int(fp, n))
	return FALSE;
    while (list != NULL)
    {
	if (!write_object(fp, (struct object *) ldata(list)))
	    return FALSE;
	list = next(list);
    }
    return TRUE;
}

static int read_object_list(FILE *fp, struct linked_list **list)
{
    int n, i;
    struct linked_list *item;

    *list = NULL;
    if (!read_int(fp, &n))
	return FALSE;
    if (n < 0 || n > 2048)
	return FALSE;
    for (i = 0; i < n; i++)
    {
	item = read_object_item(fp);
	if (item == NULL)
	    return FALSE;
	append_item(list, item);
    }
    return TRUE;
}

static void fix_monster_template_pointers(void)
{
    /*
     * Monster templates are rebuilt from the program image rather than read
     * from the save file.  Reattach their damage-string pointers after the
     * saved violet-fungi damage string has been restored.
     */
    static char *monster_damage[26] = {
	"1d6", "1d2", "1d6/1d6", "1d8/1d8/3d10", "0d0",
	NULL, "1d6", "1d8", "4d4", "1d2", "1d4", "1d1",
	"3d4", "0d0", "1d8", "2d12/2d4", "1d2/1d2/1d4",
	"0d0/0d0", "1d3", "1d8/1d8/2d6", "3d4/3d4/2d5",
	"1d10", "1d6", "1d3/1d3/1d3/4d6", "1d6/1d6", "1d8"
    };
    int i;

    for (i = 0; i < 26; i++)
    {
	if (i == 'F' - 'A')
	    monsters[i].m_stats.s_dmg = fungi_damage;
	else
	    monsters[i].m_stats.s_dmg = monster_damage[i];
    }
}

static int monster_dest_kind(struct thing *tp, int *room_index)
{
    int i;

    *room_index = -1;
    if (tp->t_dest == &hero)
	return 1;
    for (i = 0; i < MAXROOMS; i++)
	if (tp->t_dest == &rooms[i].r_gold)
	{
	    *room_index = i;
	    return 2;
	}
    return 0;
}

static void fix_monster_pointers(struct thing *tp, int kind, int room_index)
{
    int idx;

    idx = tp->t_type - 'A';
    if (idx >= 0 && idx < 26)
	tp->t_stats.s_dmg = monsters[idx].m_stats.s_dmg;
    else
	tp->t_stats.s_dmg = "0d0";
    if (kind == 1)
	tp->t_dest = &hero;
    else if (kind == 2 && room_index >= 0 && room_index < MAXROOMS)
	tp->t_dest = &rooms[room_index].r_gold;
    else
	tp->t_dest = &hero;
}

static int write_thing(FILE *fp, struct thing *tp)
{
    struct save_thing st;

    save_thing_from_thing(&st, tp);
    st.dest_kind = monster_dest_kind(tp, &st.dest_room);
    if (!write_bytes(fp, &st, sizeof st))
	return FALSE;
    return write_object_list(fp, tp->t_pack);
}

static struct linked_list *read_thing_item(FILE *fp)
{
    struct save_thing st;
    struct linked_list *item;
    struct thing *tp;

    if (!read_bytes(fp, &st, sizeof st))
	return NULL;
    item = new_item(sizeof *tp);
    tp = (struct thing *) ldata(item);
    thing_from_save_thing(tp, &st, "0d0");
    if (!read_object_list(fp, &tp->t_pack))
	return NULL;
    fix_monster_pointers(tp, st.dest_kind, st.dest_room);
    return item;
}

static int write_thing_list(FILE *fp, struct linked_list *list)
{
    int n;

    n = list_count(list);
    if (!write_int(fp, n))
	return FALSE;
    while (list != NULL)
    {
	if (!write_thing(fp, (struct thing *) ldata(list)))
	    return FALSE;
	list = next(list);
    }
    return TRUE;
}

static int read_thing_list(FILE *fp, struct linked_list **list)
{
    int n, i;
    struct linked_list *item;

    *list = NULL;
    if (!read_int(fp, &n))
	return FALSE;
    if (n < 0 || n > 512)
	return FALSE;
    for (i = 0; i < n; i++)
    {
	item = read_thing_item(fp);
	if (item == NULL)
	    return FALSE;
	append_item(list, item);
    }
    return TRUE;
}

static int write_string_array(FILE *fp, char **arr, int n)
{
    int i;

    for (i = 0; i < n; i++)
	if (!write_string(fp, arr[i]))
	    return FALSE;
    return TRUE;
}

static int read_string_array(FILE *fp, char **arr, int n)
{
    int i;

    for (i = 0; i < n; i++)
	arr[i] = read_string(fp);
    return TRUE;
}

static int write_window(FILE *fp, WINDOW *win)
{
    int y, x, ch;

    for (y = 0; y < ROGUE_SCREEN_LINES; y++)
	for (x = 0; x < ROGUE_SCREEN_COLS; x++)
	{
	    ch = rogue_chtype_to_char(mvwinch(win, y, x));
	    if (!write_int(fp, ch))
		return FALSE;
	}
    return TRUE;
}

static int read_window(FILE *fp, WINDOW *win, int saved_lines, int saved_cols)
{
    int y, x, ch;

    rogue_vt_clear_window(win);
    werase(win);
    for (y = 0; y < saved_lines; y++)
	for (x = 0; x < saved_cols; x++)
	{
	    if (!read_int(fp, &ch))
		return FALSE;
	    if (y < ROGUE_SCREEN_LINES && x < ROGUE_SCREEN_COLS)
		mvwaddch(win, y, x, ch);
	}
    return TRUE;
}

static int save_snapshot(FILE *fp)
{
    int saved_lines, saved_cols;
    int idx_weapon, idx_armor, idx_left, idx_right;
    char magic[sizeof RS_MAGIC];

    memset(magic, 0, sizeof magic);
    strcpy(magic, RS_MAGIC);
    saved_lines = ROGUE_SCREEN_LINES;
    saved_cols = ROGUE_SCREEN_COLS;
    idx_weapon = object_index(pack, cur_weapon);
    idx_armor = object_index(pack, cur_armor);
    idx_left = object_index(pack, cur_ring[LEFT]);
    idx_right = object_index(pack, cur_ring[RIGHT]);

    if (!write_bytes(fp, magic, sizeof magic)) return FALSE;
    if (!write_int(fp, (int) sizeof(struct save_object))) return FALSE;
    if (!write_int(fp, (int) sizeof(struct save_thing))) return FALSE;
    if (!write_int(fp, saved_lines) || !write_int(fp, saved_cols)) return FALSE;

    if (!write_bytes(fp, rooms, sizeof rooms)) return FALSE;
    if (!write_bytes(fp, traps, sizeof traps)) return FALSE;
    if (!write_player(fp)) return FALSE;
    if (!write_stats(fp, &max_stats)) return FALSE;

    if (!write_int(fp, level) || !write_int(fp, purse) || !write_int(fp, mpos)) return FALSE;
    if (!write_int(fp, ntraps) || !write_int(fp, no_move) || !write_int(fp, no_command)) return FALSE;
    if (!write_int(fp, inpack) || !write_int(fp, max_hp) || !write_int(fp, lastscore)) return FALSE;
    if (!write_int(fp, no_food) || !write_int(fp, seed) || !write_int(fp, count)) return FALSE;
    if (!write_int(fp, dnum) || !write_int(fp, fung_hit) || !write_int(fp, quiet)) return FALSE;
    if (!write_int(fp, max_level) || !write_int(fp, food_left) || !write_int(fp, group)) return FALSE;
    if (!write_int(fp, hungry_state)) return FALSE;

    if (!write_bytes(fp, &take, sizeof take)) return FALSE;
    if (!write_bytes(fp, &runch, sizeof runch)) return FALSE;
    if (!write_bytes(fp, whoami, sizeof whoami)) return FALSE;
    if (!write_bytes(fp, fruit, sizeof fruit)) return FALSE;
    if (!write_bytes(fp, huh, sizeof huh)) return FALSE;
    if (!write_bytes(fp, fungi_damage, sizeof fungi_damage)) return FALSE;

    if (!write_bytes(fp, &running, sizeof running)) return FALSE;
    if (!write_bytes(fp, &playing, sizeof playing)) return FALSE;
    if (!write_bytes(fp, &wizard, sizeof wizard)) return FALSE;
    if (!write_bytes(fp, &after, sizeof after)) return FALSE;
    if (!write_bytes(fp, &notify, sizeof notify)) return FALSE;
    if (!write_bytes(fp, &fight_flush, sizeof fight_flush)) return FALSE;
    if (!write_bytes(fp, &terse, sizeof terse)) return FALSE;
    if (!write_bytes(fp, &door_stop, sizeof door_stop)) return FALSE;
    if (!write_bytes(fp, &jump, sizeof jump)) return FALSE;
    if (!write_bytes(fp, &slow_invent, sizeof slow_invent)) return FALSE;
    if (!write_bytes(fp, &firstmove, sizeof firstmove)) return FALSE;
    if (!write_bytes(fp, &waswizard, sizeof waswizard)) return FALSE;
    if (!write_bytes(fp, &askme, sizeof askme)) return FALSE;
    if (!write_bytes(fp, s_know, sizeof s_know)) return FALSE;
    if (!write_bytes(fp, p_know, sizeof p_know)) return FALSE;
    if (!write_bytes(fp, r_know, sizeof r_know)) return FALSE;
    if (!write_bytes(fp, ws_know, sizeof ws_know)) return FALSE;
    if (!write_bytes(fp, &amulet, sizeof amulet)) return FALSE;
    if (!write_bytes(fp, &oldpos, sizeof oldpos)) return FALSE;
    if (!write_bytes(fp, &delta, sizeof delta)) return FALSE;

    if (!write_bytes(fp, p_color_index, sizeof p_color_index)) return FALSE;
    if (!write_bytes(fp, r_stone_index, sizeof r_stone_index)) return FALSE;
    if (!write_bytes(fp, ws_material_index, sizeof ws_material_index)) return FALSE;
    if (!write_bytes(fp, ws_material_is_wood, sizeof ws_material_is_wood)) return FALSE;
    if (!write_bytes(fp, scroll_syll_count, sizeof scroll_syll_count)) return FALSE;
    if (!write_bytes(fp, scroll_syll_ids, sizeof scroll_syll_ids)) return FALSE;
    if (!write_bytes(fp, scroll_syll_breaks, sizeof scroll_syll_breaks)) return FALSE;

    if (!write_string_array(fp, s_names, MAXSCROLLS)) return FALSE;
    if (!write_string_array(fp, p_colors, MAXPOTIONS)) return FALSE;
    if (!write_string_array(fp, r_stones, MAXRINGS)) return FALSE;
    if (!write_string_array(fp, ws_made, MAXSTICKS)) return FALSE;
    if (!write_string_array(fp, s_guess, MAXSCROLLS)) return FALSE;
    if (!write_string_array(fp, p_guess, MAXPOTIONS)) return FALSE;
    if (!write_string_array(fp, r_guess, MAXRINGS)) return FALSE;
    if (!write_string_array(fp, ws_guess, MAXSTICKS)) return FALSE;
    if (!write_string_array(fp, ws_type, MAXSTICKS)) return FALSE;

    if (!write_object_list(fp, pack)) return FALSE;
    if (!write_object_list(fp, lvl_obj)) return FALSE;
    if (!write_thing_list(fp, mlist)) return FALSE;
    if (!write_int(fp, idx_weapon) || !write_int(fp, idx_armor)) return FALSE;
    if (!write_int(fp, idx_left) || !write_int(fp, idx_right)) return FALSE;

    if (!rogue_save_daemons(fp)) return FALSE;
    if (!write_window(fp, stdscr)) return FALSE;
    if (!write_window(fp, cw)) return FALSE;
    if (!write_window(fp, mw)) return FALSE;
    /* This final int was zero in earlier PORTABLE-5 saves.  Reuse it for
     * the A-Z genocide bitmask without changing the save file layout. */
    return write_int(fp, (int) rogue_genocide_mask());
}

static int restore_snapshot(FILE *fp)
{
    int obj_size, thing_size, saved_lines, saved_cols;
    int idx_weapon, idx_armor, idx_left, idx_right;
    int genocide_mask;
    char magic[sizeof RS_MAGIC];

    if (!read_bytes(fp, magic, sizeof magic)) return FALSE;
    if (memcmp(magic, RS_MAGIC, sizeof magic) != 0)
	return FALSE;
    if (!read_int(fp, &obj_size) || !read_int(fp, &thing_size)) return FALSE;
    if (obj_size != (int) sizeof(struct save_object) || thing_size != (int) sizeof(struct save_thing)) return FALSE;
    if (!read_int(fp, &saved_lines) || !read_int(fp, &saved_cols)) return FALSE;
    /* Saves from an enlarged dungeon cannot safely fit the fixed map.
     * Accept earlier 80x24 saves, but reject other playfield sizes. */
    if (saved_lines != ROGUE_SCREEN_LINES ||
        saved_cols != ROGUE_SCREEN_COLS) return FALSE;

    init_player();
    init_things();
    init_names();
    init_colors();
    init_stones();
    init_materials();

    if (!read_bytes(fp, rooms, sizeof rooms)) return FALSE;
    if (!read_bytes(fp, traps, sizeof traps)) return FALSE;
    if (!read_player(fp)) return FALSE;
    if (!read_stats(fp, &max_stats, "1d4")) return FALSE;
    fix_monster_template_pointers();

    if (!read_int(fp, &level) || !read_int(fp, &purse) || !read_int(fp, &mpos)) return FALSE;
    if (!read_int(fp, &ntraps) || !read_int(fp, &no_move) || !read_int(fp, &no_command)) return FALSE;
    if (!read_int(fp, &inpack) || !read_int(fp, &max_hp) || !read_int(fp, &lastscore)) return FALSE;
    if (!read_int(fp, &no_food) || !read_int(fp, &seed) || !read_int(fp, &count)) return FALSE;
    if (!read_int(fp, &dnum) || !read_int(fp, &fung_hit) || !read_int(fp, &quiet)) return FALSE;
    if (!read_int(fp, &max_level) || !read_int(fp, &food_left) || !read_int(fp, &group)) return FALSE;
    if (!read_int(fp, &hungry_state)) return FALSE;

    if (!read_bytes(fp, &take, sizeof take)) return FALSE;
    if (!read_bytes(fp, &runch, sizeof runch)) return FALSE;
    if (!read_bytes(fp, whoami, sizeof whoami)) return FALSE;
    if (!read_bytes(fp, fruit, sizeof fruit)) return FALSE;
    if (!read_bytes(fp, huh, sizeof huh)) return FALSE;
    whoami[sizeof whoami - 1] = '\0';
    fruit[sizeof fruit - 1] = '\0';
    huh[sizeof huh - 1] = '\0';
    if (!read_bytes(fp, fungi_damage, sizeof fungi_damage)) return FALSE;
    fungi_damage[sizeof fungi_damage - 1] = '\0';
    fix_monster_template_pointers();

    if (!read_bytes(fp, &running, sizeof running)) return FALSE;
    if (!read_bytes(fp, &playing, sizeof playing)) return FALSE;
    if (!read_bytes(fp, &wizard, sizeof wizard)) return FALSE;
    if (!read_bytes(fp, &after, sizeof after)) return FALSE;
    if (!read_bytes(fp, &notify, sizeof notify)) return FALSE;
    if (!read_bytes(fp, &fight_flush, sizeof fight_flush)) return FALSE;
    if (!read_bytes(fp, &terse, sizeof terse)) return FALSE;
    if (!read_bytes(fp, &door_stop, sizeof door_stop)) return FALSE;
    if (!read_bytes(fp, &jump, sizeof jump)) return FALSE;
    if (!read_bytes(fp, &slow_invent, sizeof slow_invent)) return FALSE;
    if (!read_bytes(fp, &firstmove, sizeof firstmove)) return FALSE;
    if (!read_bytes(fp, &waswizard, sizeof waswizard)) return FALSE;
    if (!read_bytes(fp, &askme, sizeof askme)) return FALSE;
    if (!read_bytes(fp, s_know, sizeof s_know)) return FALSE;
    if (!read_bytes(fp, p_know, sizeof p_know)) return FALSE;
    if (!read_bytes(fp, r_know, sizeof r_know)) return FALSE;
    if (!read_bytes(fp, ws_know, sizeof ws_know)) return FALSE;
    if (!read_bytes(fp, &amulet, sizeof amulet)) return FALSE;
    if (!read_bytes(fp, &oldpos, sizeof oldpos)) return FALSE;
    if (!read_bytes(fp, &delta, sizeof delta)) return FALSE;

    if (!read_bytes(fp, p_color_index, sizeof p_color_index)) return FALSE;
    if (!read_bytes(fp, r_stone_index, sizeof r_stone_index)) return FALSE;
    if (!read_bytes(fp, ws_material_index, sizeof ws_material_index)) return FALSE;
    if (!read_bytes(fp, ws_material_is_wood, sizeof ws_material_is_wood)) return FALSE;
    if (!read_bytes(fp, scroll_syll_count, sizeof scroll_syll_count)) return FALSE;
    if (!read_bytes(fp, scroll_syll_ids, sizeof scroll_syll_ids)) return FALSE;
    if (!read_bytes(fp, scroll_syll_breaks, sizeof scroll_syll_breaks)) return FALSE;

    if (!read_string_array(fp, s_names, MAXSCROLLS)) return FALSE;
    if (!read_string_array(fp, p_colors, MAXPOTIONS)) return FALSE;
    if (!read_string_array(fp, r_stones, MAXRINGS)) return FALSE;
    if (!read_string_array(fp, ws_made, MAXSTICKS)) return FALSE;
    if (!read_string_array(fp, s_guess, MAXSCROLLS)) return FALSE;
    if (!read_string_array(fp, p_guess, MAXPOTIONS)) return FALSE;
    if (!read_string_array(fp, r_guess, MAXRINGS)) return FALSE;
    if (!read_string_array(fp, ws_guess, MAXSTICKS)) return FALSE;
    if (!read_string_array(fp, ws_type, MAXSTICKS)) return FALSE;
    jrogue_apply_language();
    rogue_ensure_utf8_locale();
    fix_monster_template_pointers();

    if (!read_object_list(fp, &pack)) return FALSE;
    player.t_pack = pack;
    if (!read_object_list(fp, &lvl_obj)) return FALSE;
    if (!read_thing_list(fp, &mlist)) return FALSE;
    if (!read_int(fp, &idx_weapon) || !read_int(fp, &idx_armor)) return FALSE;
    if (!read_int(fp, &idx_left) || !read_int(fp, &idx_right)) return FALSE;
    cur_weapon = object_at(pack, idx_weapon);
    cur_armor = object_at(pack, idx_armor);
    cur_ring[LEFT] = object_at(pack, idx_left);
    cur_ring[RIGHT] = object_at(pack, idx_right);

    if (!rogue_init_screen())
        return FALSE;

    if (!rogue_restore_daemons(fp)) return FALSE;
    if (!read_window(fp, stdscr, saved_lines, saved_cols)) return FALSE;
    if (!read_window(fp, cw, saved_lines, saved_cols)) return FALSE;
    if (!read_window(fp, mw, saved_lines, saved_cols)) return FALSE;
    /* Earlier PORTABLE-5 saves wrote zero here (no genocides).  Reject
     * damaged masks rather than silently reviving or excluding monsters. */
    if (!read_int(fp, &genocide_mask) ||
        ((unsigned int)genocide_mask & ~((1U << 26) - 1U)) != 0)
        return FALSE;
    rogue_restore_genocide_mask((unsigned int)genocide_mask);
    clearok(cw, TRUE);
    touchwin(cw);
    draw(cw);
    status();
    return TRUE;
}

/*
 * Path shortening now belongs to the OS-neutral unicode_text module.
 * Keep only the current message line's available-width calculation here;
 * the actual save path is never changed for display purposes.
 */
/* The entire message must fit on row zero.  Allow one unused column to
 * avoid a terminal's automatic wrap at the rightmost cell. */
static void save_prompt_path(char *dst, size_t cap, const char *path,
                             const char *empty_format)
{
    int available;

    available = ROGUE_SCREEN_COLS - 1 - rogue_display_width(empty_format);
    rogue_utf8_path_display(dst, cap, path, available);
}

int save_game(void)
{
    FILE *savef;
    int c;
    char display[ROGUE_SCREEN_COLS * 4 + 1];
    char entered[ROGUE_PATH_SIZE];

    save_prompt_path(display, sizeof display, file_name,
        JSTR("ファイル名（）でセーブしますか？", "Save file ()?"));
    msg(JSTR("ファイル名（%s）でセーブしますか？", "Save file (%s)?"), display);
    c = readchar();
    mpos = 0;

    if (c == ESCAPE || c == '\007')
    {
	msg("");
	return FALSE;
    }

    if (c != 'y' && c != 'Y')
    {
	/* Use a blank editing buffer.  Reprinting a long existing full path
         * from get_str() could otherwise wrap and overwrite dungeon row 1
         * before the visible screen is written into the save snapshot. */
        entered[0] = '\0';
	msg(JSTR("ファイル名：", "File name: "));
	if (get_str(entered, cw) == QUIT || entered[0] == '\0')
	{
	    msg("");
	    return FALSE;
	}
        rogue_utf8_copy(file_name, sizeof file_name, entered);
    }
    else
    {
	save_prompt_path(display, sizeof display, file_name,
            JSTR("ファイル名： ", "File name: "));
	msg(JSTR("ファイル名： %s", "File name: %s"), display);
	draw(cw);
	/* Keep the save filename briefly visible on modern fast terminals. */
	rogue_delay_ms(ROGUE_SAVE_FILENAME_DELAY_MS);
    }

    if (save_file_is_present())
    {
	msg(JSTR("セーブファイルはすでに存在します。", "Save file already exists."));
	return FALSE;
    }
    savef = rogue_save_fopen(file_name, "wb");
    if (savef == NULL)
    {
	save_prompt_path(display, sizeof display, file_name,
            JSTR("セーブファイルを作成できません：", "Cannot create save file: "));
	msg(JSTR("セーブファイルを作成できません：%s", "Cannot create save file: %s"), display);
	return FALSE;
    }
    /*
     * Do not store the transient save prompt in the restored screen.
     * With Japanese text, saving the prompt line in a narrow curses
     * snapshot can also make the restored line appear as stray glyphs
     * such as "^^^ ./rogue.save".  Clear only the visible message line;
     * the previous Ctrl-R message history is left untouched.
     */
    msg("");

    if (!save_snapshot(savef))
    {
	fclose(savef);
	rogue_save_unlink(file_name);
	msg(JSTR("ゲームをセーブできません。", "Cannot save game."));
	return FALSE;
    }
    fclose(savef);
    return TRUE;
}

int save_file_is_present(void)
{
#ifdef _WIN32
    return rogue_windows_file_exists_utf8(file_name);
#else
    return access(file_name, F_OK) == 0;
#endif
}

void auto_save(int sig)
{
    FILE *savef;
    int i;

    (void) sig;

    /*
     * Original Rogue auto-saves on fatal signals without asking.  Calling
     * save_game() here is unsafe and can drop the normal interactive
     * ``Save file (...) ?'' prompt into the middle of play if a signal is
     * raised during combat or drawing.
     */
#ifdef NSIG
    for (i = 0; i < NSIG; i++)
	signal(i, SIG_IGN);
#else
    (void) i;
    signal(sig, SIG_IGN);
#endif

    if (cw != NULL && file_name[0] != '\0')
    {
	savef = rogue_save_fopen(file_name, "wb");
	if (savef != NULL)
	{
	    if (!save_snapshot(savef))
		rogue_save_unlink(file_name);
	    fclose(savef);
	}
    }
    rogue_endwin();
    exit(1);
}

/*
 * Keep the original Rogue 3.6 ctime() text in English mode.  Japanese
 * mode formats the same local modification time with Japanese date and
 * weekday notation, without requiring a Japanese system locale.
 * This is shared by macOS, Linux and Windows; only file stat differs.
 */
static int save_mtime_display(char *dst, size_t cap, const time_t *mtime)
{
    char *tstr;

    if (cap == 0)
        return FALSE;
    dst[0] = '\0';

    if (jrogue_lang_is_japanese)
    {
        static const char *const weekdays[] = {
            "日", "月", "火", "水", "木", "金", "土"
        };
        struct tm *local = localtime(mtime);

        if (local != NULL && local->tm_wday >= 0 && local->tm_wday < 7)
        {
            int n = snprintf(dst, cap,
                "%d年%d月%d日（%s）%02d:%02d:%02d",
                local->tm_year + 1900, local->tm_mon + 1,
                local->tm_mday, weekdays[local->tm_wday],
                local->tm_hour, local->tm_min, local->tm_sec);
            if (n >= 0 && (size_t)n < cap)
                return TRUE;
        }
    }

    /* ctime() retains the historical English display and is a fallback
     * if a platform cannot convert the timestamp into a local date. */
    tstr = ctime(mtime);
    if (tstr == NULL)
        return FALSE;
    strncpy(dst, tstr, cap - 1);
    dst[cap - 1] = '\0';
    dst[strcspn(dst, "\n")] = '\0';
    return TRUE;
}

int restore(char *file, char **envp)
{
    FILE *savef;
    STAT sbuf2;
    char tbuf[64];
    char display[ROGUE_SCREEN_COLS * 4 + 1];
    (void) envp;

    /* Original Rogue 3.6 treats -r as the configured default save. */
    if (file != NULL && strcmp(file, "-r") == 0)
        file = file_name;

    /* A restored pathname becomes the next default save name, as in
     * the original game.  Never truncate a real filesystem path: a
     * truncated path could create a save in the wrong place. */
    if (file == NULL || strlen(file) >= sizeof file_name)
    {
        printf("%s", JSTR(
            "セーブファイルのパスが長すぎます。\n",
            "Save file path is too long.\n"));
        return FALSE;
    }

#ifdef _WIN32
    memset(&sbuf2, 0, sizeof sbuf2);
    sbuf2.st_mtime = rogue_windows_file_mtime_utf8(file);
#else
    if (stat(file, &sbuf2) < 0)
        memset(&sbuf2, 0, sizeof sbuf2);
#endif
    savef = rogue_save_fopen(file, "rb");
    if (savef == NULL)
    {
#ifdef _WIN32
        rogue_windows_clear_score_screen();
#endif
        printf(JSTR("セーブファイル %s を開けません\n", "Cannot open save file %s\n"), file);
        return FALSE;
    }
    if (!restore_snapshot(savef))
    {
        fclose(savef);
#ifdef _WIN32
        /* A damaged save can fail after the curses screen starts. Return
         * to the normal console before printing the failure message. */
        if (cw != NULL)
            rogue_endwin();
        rogue_windows_clear_score_screen();
#endif
        printf("%s", JSTR("セーブファイルに互換性がないか、壊れています。\n", "Save file is incompatible or damaged.\n"));
        return FALSE;
    }
    fclose(savef);

    /* Preserve the original 3.6 filename-and-date message, localizing
     * only its timestamp and keeping it within the top screen row. */
    if (save_mtime_display(tbuf, sizeof tbuf, &sbuf2.st_mtime))
    {
        rogue_utf8_path_display(display, sizeof display, file,
                          ROGUE_SCREEN_COLS - 1 - 2 - rogue_display_width(tbuf));
        mpos = 0;
        rogue_screen_prepare_text_row(cw, 0);
        /* The display path has already been shortened to fit row zero.
         * Write its pieces directly: concatenating UTF-8 bytes into a
         * fixed-size snprintf buffer can trigger -Wformat-truncation. */
        rogue_waddstr_utf8(cw, display);
        rogue_waddstr_utf8(cw, ": ");
        rogue_waddstr_utf8(cw, tbuf);
        draw(cw);
    }

    /* The snapshot is now valid; keep the path used for this restore as
     * the next default save name.  Avoid overlapping copies with -r. */
    if (file != file_name)
        memcpy(file_name, file, strlen(file) + 1);

    rogue_save_unlink(file);
    playing = TRUE;
    waswizard = waswizard || wizard;
    playit();
    return TRUE;
}

void encwrite(char *start, unsigned int size, FILE *outf)
{
    register char *ep;
    ep = encstr;
    while (size--)
    {
	putc(*start++ ^ *ep++, outf);
	if (*ep == '\0') ep = encstr;
    }
}

int encread(char *start, unsigned int size, int inf)
{
    register char *ep;
    register int read_size;
    if ((read_size = read(inf, start, size)) == -1 || read_size == 0)
	return read_size;
    ep = encstr;
    while (size--)
    {
	*start++ ^= *ep++;
	if (*ep == '\0') ep = encstr;
    }
    return read_size;
}
