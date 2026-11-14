#include <curses.h>
#include "mach_dep.h"
#include "rogue.h"

struct room rooms[MAXROOMS];
struct room *oldrp;
struct trap traps[MAXTRAPS];
struct linked_list *mlist = NULL;
struct thing player;
struct stats max_stats;
struct linked_list *lvl_obj = NULL;
struct object *cur_weapon = NULL;
struct object *cur_armor = NULL;
struct object *cur_ring[2] = { NULL, NULL };

int level = 1;
int purse = 0;
int mpos = 0;
int ntraps = 0;
int no_move = 0;
int no_command = 0;
int inpack = 0;
int max_hp = 0;
int total = 0;
int lastscore = -1;
int no_food = 0;
int seed = 0;
int count = 0;
int dnum = 0;
int fung_hit = 0;
int quiet = 0;
int max_level = 0;
int food_left = HUNGERTIME;
int group = 1;
int hungry_state = 0;

char fungi_damage[16] = "000d0";

char take = 0;
char prbuf[256];
char outbuf[BUFSIZ];
char runch = 0;
char *s_names[MAXSCROLLS];
char *p_colors[MAXPOTIONS];
char *r_stones[MAXRINGS];
char *ws_made[MAXSTICKS];
int p_color_index[MAXPOTIONS];
int r_stone_index[MAXRINGS];
int ws_material_index[MAXSTICKS];
bool ws_material_is_wood[MAXSTICKS];
int scroll_syll_count[MAXSCROLLS];
int scroll_syll_ids[MAXSCROLLS][MAX_SCROLL_SYLLABLES];
bool scroll_syll_breaks[MAXSCROLLS][MAX_SCROLL_SYLLABLES];
char whoami[ROGUE_STRING_SIZE];
char fruit[ROGUE_STRING_SIZE];
char huh[HUH_SIZE];
char *s_guess[MAXSCROLLS];
char *p_guess[MAXPOTIONS];
char *r_guess[MAXRINGS];
char *ws_guess[MAXSTICKS];
char *ws_type[MAXSTICKS];
char file_name[ROGUE_PATH_SIZE];
char home[ROGUE_PATH_SIZE];

WINDOW *cw = NULL;
WINDOW *hw = NULL;
WINDOW *mw = NULL;

bool running = FALSE;
bool playing = TRUE;
bool wizard = FALSE;
bool after = FALSE;
bool notify = TRUE;
bool fight_flush = FALSE;
bool terse = FALSE;
bool door_stop = FALSE;
bool jump = FALSE;
bool slow_invent = FALSE;
bool firstmove = FALSE;
bool waswizard = FALSE;
bool askme = FALSE;
bool s_know[MAXSCROLLS];
bool p_know[MAXPOTIONS];
bool r_know[MAXRINGS];
bool ws_know[MAXSTICKS];
bool amulet = FALSE;
bool in_shell = FALSE;

coord oldpos;
coord delta;


char *rogue_strdup(const char *src)
{
    size_t len;
    char *dst;

    if (src == NULL)
        src = "";
    len = strlen(src);
    dst = ALLOC(len + 1);
    if (dst == NULL)
        fatal("Rogue ran out of memory while copying a string.  Fatal error!");
    memcpy(dst, src, len + 1);
    return dst;
}

int rogue_chtype_to_char(chtype ch)
{
    if (ch == (chtype) ERR)
        return ' ';
#ifdef A_CHARTEXT
    return (int) (ch & A_CHARTEXT);
#else
    return (int) ch;
#endif
}

int rogue_winat(WINDOW *monster_win, WINDOW *screen_win, int y, int x)
{
    int mch;

    mch = rogue_chtype_to_char(mvwinch(monster_win, y, x));
    if (mch == ' ')
        return rogue_chtype_to_char(mvwinch(screen_win, y, x));
    return rogue_chtype_to_char(winch(monster_win));
}

void rogue_build_file_path(char *dst, size_t dstlen, const char *dir, const char *name)
{
    char expanded_dir[2048];
    char local_home[1024];
    const char *home_dir;
    const char *slash;

    home_dir = home;
    if (home_dir == NULL || home_dir[0] == '\0')
    {
        home_dir = getenv("HOME");
        if (home_dir == NULL)
            home_dir = "";
    }

    snprintf(local_home, sizeof local_home, "%s", home_dir);
    if (local_home[0] != '\0' && local_home[strlen(local_home) - 1] == '/')
        local_home[strlen(local_home) - 1] = '\0';

    if (name == NULL || name[0] == '\0')
        name = "rogue.save";

    /*
     * If the file name already contains a directory component, use it as a
     * complete path.  A ~/... path is expanded using HOME.
     */
    if (strchr(name, '/') != NULL)
    {
        if (name[0] == '~' && (name[1] == '\0' || name[1] == '/'))
        {
            if (name[1] == '/')
                snprintf(dst, dstlen, "%s/%s", local_home, name + 2);
            else
                snprintf(dst, dstlen, "%s", local_home);
        }
        else
            snprintf(dst, dstlen, "%s", name);
        return;
    }

    if (dir == NULL || dir[0] == '\0')
        dir = ".";

    if (dir[0] == '~' && (dir[1] == '\0' || dir[1] == '/'))
    {
        if (dir[1] == '/')
            snprintf(expanded_dir, sizeof expanded_dir, "%s/%s", local_home, dir + 2);
        else
            snprintf(expanded_dir, sizeof expanded_dir, "%s", local_home);
        dir = expanded_dir;
    }

    slash = (dir[0] != '\0' && dir[strlen(dir) - 1] == '/') ? "" : "/";
    snprintf(dst, dstlen, "%s%s%s", dir, slash, name);
}
