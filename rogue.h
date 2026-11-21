/*
 * Rogue definitions and variable declarations
 *
 * @(#)rogue.h	3.38 (Berkeley) 6/15/81
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#ifndef _WIN32
#include <sys/wait.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <locale.h>
#include "jec/language.h"
#include "jec/unicode_text.h"
#ifdef _WIN32
#include <io.h>
#include <process.h>
#include "windows/rogue_windows.h"
/* Match POSIX file I/O names in the original game. */
#define read _read
#define close _close
#define unlink _unlink
#define access _access
#else
#define rogue_platform_pid() getpid()
#endif

#include "jec/unicode_output.h"
#include "jec/screen_text.h"

#define daemon rogue_daemon
#define remove rogue_remove
#define unctrl rogue_unctrl
#ifndef crmode
#define crmode() cbreak()
#endif


/*
 * Maximum number of different things
 */
/* Rogue 3.6 uses a fixed 80x24 logical playfield.  curses LINES/COLS
 * describe the physical terminal; do not use them for dungeon geometry. */
#define ROGUE_SCREEN_COLS 80
#define ROGUE_SCREEN_LINES 24
#define MAXROOMS 9
#define MAXTHINGS 9
#define MAXOBJ 9
#define MAXPACK 23
#define MAXTRAPS 10
#define	NUMTHINGS 7	/* number of types of things (scrolls, rings, etc.) */

/*
 * return values for get functions
 */
#define	NORM	0	/* normal exit */
#define	QUIT	1	/* quit option setting */
#define	MINUS	2	/* back up one option */

/*
 * All the fun defines
 */
#define next(ptr) (*ptr).l_next
#define prev(ptr) (*ptr).l_prev
#define ldata(ptr) (*ptr).l_data
#define inroom(rp, cp) (\
    (cp)->x <= (rp)->r_pos.x + ((rp)->r_max.x - 1) && (rp)->r_pos.x <= (cp)->x \
 && (cp)->y <= (rp)->r_pos.y + ((rp)->r_max.y - 1) && (rp)->r_pos.y <= (cp)->y)
int rogue_init_screen(void);
int rogue_chtype_to_char(chtype ch);
char *rogue_strdup(const char *src);
void rogue_init_locale(void);
void rogue_ensure_utf8_locale(void);
int rogue_winat(WINDOW *monster_win, WINDOW *screen_win, int y, int x);
#define winat(y, x) rogue_winat(mw, stdscr, (y), (x))
#define debug if (wizard) msg
#define RN (((seed = seed*11109+13849) & 0x7fff) >> 1)
#define unc(cp) (cp).y, (cp).x
#define cmov(xy) move((xy).y, (xy).x)
#define DISTANCE(y1, x1, y2, x2) ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))
#define when break;case
#define otherwise break;default
#define until(expr) while(!(expr))
#define ce(a, b) ((a).x == (b).x && (a).y == (b).y)
#ifdef _WIN32
#define draw(window) rogue_draw_utf8(window)
#else
#define draw(window) wrefresh(window)
#endif
#ifndef ROGUE_ANIMATION_DELAY_MS
#define ROGUE_ANIMATION_DELAY_MS 24
#endif
#ifndef ROGUE_STARTUP_DELAY_MS
#define ROGUE_STARTUP_DELAY_MS 1000
#endif
#ifndef ROGUE_SAVE_FILENAME_DELAY_MS
#define ROGUE_SAVE_FILENAME_DELAY_MS 250
#endif
#ifndef ROGUE_SAVEFILE
#define ROGUE_SAVEFILE "rogue.save"
#endif
#ifndef ROGUE_SAVEDIR
#define ROGUE_SAVEDIR "."
#endif
#ifndef ROGUE_SCOREDIR
#define ROGUE_SCOREDIR "."
#endif
#ifndef ROGUE_LOCKDIR
#define ROGUE_LOCKDIR "."
#endif
void rogue_delay_ms(int ms);
void rogue_build_file_path(char *dst, size_t dstlen, const char *dir, const char *name);
#define hero player.t_pos
#define pstats player.t_stats
#define pack player.t_pack
#define attach(a,b) _attach(&a,b)
#define detach(a,b) _detach(&a,b)
#define free_list(a) _free_list(&a)
/* Windows headers can define max already; use the game's definition. */
#ifdef max
#undef max
#endif
#define max(a, b) ((a) > (b) ? (a) : (b))
#define on(thing, flag) (((thing).t_flags & flag) != 0)
#define off(thing, flag) (((thing).t_flags & flag) == 0)
#define CTRL(ch) ((ch) & 037)
#define ALLOC(x) malloc((size_t)(x))
#define FREE(x) free((void *)(x))
#define	EQSTR(a, b, c)	(strncmp(a, b, c) == 0)
#define GOLDCALC (rnd(50 + 10 * level) + 2)
#define ISRING(h,r) (cur_ring[h] != NULL && cur_ring[h]->o_which == r)
#define ISWEARING(r) (ISRING(LEFT, r) || ISRING(RIGHT, r))
#define newgrp() ++group
#define o_charges o_ac
#define ISMULT(type) (type == POTION || type == SCROLL || type == FOOD)

/*
 * Things that appear on the screens
 */
#define PASSAGE '#'
#define DOOR '+'
#define FLOOR '.'
#define PLAYER '@'
#define TRAP '^'
#define TRAPDOOR '>'
#define ARROWTRAP '{'
#define SLEEPTRAP '$'
#define BEARTRAP '}'
#define TELTRAP '~'
#define DARTTRAP '`'
#define SECRETDOOR '&'
#define STAIRS '%'
#define GOLD '*'
#define POTION '!'
#define SCROLL '?'
#define MAGIC '$'
#define FOOD ':'
#define WEAPON ')'
#define ARMOR ']'
#define AMULET ','
#define RING '='
#define STICK '/'
#define CALLABLE -1

/*
 * Various constants
 */
#define	PASSWD	"mTcTtUJ57exqY"
/* #define PASSWD "mTUPAHIUXEmyQ" */
/* #define	PASSWD "mTmZcVjBEccKk" */
#define BEARTIME 3
#define SLEEPTIME 5
#define HEALTIME 30
#define HOLDTIME 2
#define STPOS 0
#define WANDERTIME 70
#define BEFORE 1
#define AFTER 2
#define HUHDURATION 20
#define HUH_SIZE 256
#define ROGUE_STRING_SIZE 256
#define ROGUE_PATH_SIZE 256
#define ROGUE_INPUT_SIZE 256
#define ROGUE_INPUT_MAX_WIDTH 40
#define SEEDURATION 850
#define HUNGERTIME 1300
#define MORETIME 150
#define STOMACHSIZE 2000
#define ESCAPE 27
#define LEFT 0
#define RIGHT 1
#define BOLT_LENGTH 6

/*
 * Save against things
 */
#define VS_POISON 00
#define VS_PARALYZATION 00
#define VS_DEATH 00
#define VS_PETRIFICATION 01
#define VS_BREATH 02
#define VS_MAGIC  03

/*
 * Various flag bits
 */
#define ISDARK	0000001
#define ISCURSED 000001
#define ISBLIND 0000001
#define ISGONE	0000002
#define ISKNOW  0000002
#define ISRUN	0000004
#define ISFOUND 0000010
#define ISINVIS 0000020
#define ISMEAN  0000040
#define ISGREED 0000100
#define ISBLOCK 0000200
#define ISHELD  0000400
#define ISHUH   0001000
#define ISREGEN 0002000
#define CANHUH  0004000
#define CANSEE  0010000
#define ISMISL  0020000
#define ISCANC	0020000
#define ISMANY  0040000
#define ISSLOW	0040000
#define ISHASTE 0100000

/*
 * Potion types
 */
#define P_CONFUSE 0
#define P_PARALYZE 1
#define P_POISON 2
#define P_STRENGTH 3
#define P_SEEINVIS 4
#define P_HEALING 5
#define P_MFIND 6
#define P_TFIND 7
#define P_RAISE 8
#define P_XHEAL 9
#define P_HASTE 10
#define P_RESTORE 11
#define P_BLIND 12
#define P_NOP 13
#define MAXPOTIONS 14

/*
 * Scroll types
 */
#define S_CONFUSE 0
#define S_MAP 1
#define S_LIGHT 2
#define S_HOLD 3
#define S_SLEEP 4
#define S_ARMOR 5
#define S_IDENT 6
#define S_SCARE 7
#define S_GFIND 8
#define S_TELEP 9
#define S_ENCH 10
#define S_CREATE 11
#define S_REMOVE 12
#define S_AGGR 13
#define S_NOP 14
#define S_GENOCIDE 15
#define MAXSCROLLS 16
#define MAX_SCROLL_SYLLABLES 20

/*
 * Weapon types
 */
#define MACE 0
#define SWORD 1
#define BOW 2
#define ARROW 3
#define DAGGER 4
#define ROCK 5
#define TWOSWORD 6
#define SLING 7
#define DART 8
#define CROSSBOW 9
#define BOLT 10
#define SPEAR 11
#define MAXWEAPONS 12

/*
 * Armor types
 */
#define LEATHER 0
#define RING_MAIL 1
#define STUDDED_LEATHER 2
#define SCALE_MAIL 3
#define CHAIN_MAIL 4
#define SPLINT_MAIL 5
#define BANDED_MAIL 6
#define PLATE_MAIL 7
#define MAXARMORS 8

/*
 * Ring types
 */
#define R_PROTECT 0
#define R_ADDSTR 1
#define R_SUSTSTR 2
#define R_SEARCH 3
#define R_SEEINVIS 4
#define R_NOP 5
#define R_AGGR 6
#define R_ADDHIT 7
#define R_ADDDAM 8
#define R_REGEN 9
#define R_DIGEST 10
#define R_TELEPORT 11
#define R_STEALTH 12
#define MAXRINGS 13

/*
 * Rod/Wand/Staff types
 */

#define WS_LIGHT 0
#define WS_HIT 1
#define WS_ELECT 2
#define WS_FIRE 3
#define WS_COLD 4
#define WS_POLYMORPH 5
#define WS_MISSILE 6
#define WS_HASTE_M 7
#define WS_SLOW_M 8
#define WS_DRAIN 9
#define WS_NOP 10
#define WS_TELAWAY 11
#define WS_TELTO 12
#define WS_CANCEL 13
#define MAXSTICKS 14

/*
 * Now we define the structures and types
 */

/*
 * Help list
 */

struct h_list {
    char h_ch;
    char *h_desc;
};
extern struct h_list helpstr[];

/*
 * Coordinate data type
 */
typedef struct {
    int x;
    int y;
} coord;

typedef struct {
    short st_str;
    short st_add;
} str_t;

/*
 * Linked list data type
 */
struct linked_list {
    struct linked_list *l_next;
    struct linked_list *l_prev;
    char *l_data;			/* Various structure pointers */
};

/*
 * Stuff about magic items
 */

struct magic_item {
    char *mi_name;
    int mi_prob;
    int mi_worth;
};

/*
 * Room structure
 */
struct room {
    coord r_pos;			/* Upper left corner */
    coord r_max;			/* Size of room */
    coord r_gold;			/* Where the gold is */
    int r_goldval;			/* How much the gold is worth */
    int r_flags;			/* Info about the room */
    int r_nexits;			/* Number of exits */
    coord r_exit[4];			/* Where the exits are */
};

/*
 * Array of all traps on this level
 */
struct trap {
    coord tr_pos;			/* Where trap is */
    char tr_type;			/* What kind of trap */
    int tr_flags;			/* Info about trap (i.e. ISFOUND) */
};
extern struct trap traps[MAXTRAPS];

/*
 * Structure describing a fighting being
 */
struct stats {
    str_t s_str;			/* Strength */
    long s_exp;				/* Experience */
    int s_lvl;				/* Level of mastery */
    int s_arm;				/* Armor class */
    int s_hpt;				/* Hit points */
    char *s_dmg;			/* String describing damage done */
};

/*
 * Structure for monsters and player
 */
struct thing {
    coord t_pos;			/* Position */
    bool t_turn;			/* If slowed, is it a turn to move */
    char t_type;			/* What it is */
    char t_disguise;			/* What mimic looks like */
    char t_oldch;			/* Character that was where it was */
    coord *t_dest;			/* Where it is running to */
    short t_flags;			/* State word */
    struct stats t_stats;		/* Physical description */
    struct linked_list *t_pack;		/* What the thing is carrying */
};

/*
 * Array containing information on all the various types of mosnters
 */
struct monster {
    char *m_name;			/* What to call the monster */
    short m_carry;			/* Probability of carrying something */
    short m_flags;			/* Things about the monster */
    struct stats m_stats;		/* Initial stats */
};

/*
 * Structure for a thing that the rogue can carry
 */

struct object {
    int o_type;				/* What kind of object it is */
    coord o_pos;			/* Where it lives on the screen */
    char *o_text;			/* What it says if you read it */
    char o_launch;			/* What you need to launch it */
    char *o_damage;			/* Damage if used like sword */
    char *o_hurldmg;			/* Damage if thrown */
    int o_count;			/* Count for plural objects */
    int o_which;			/* Which object of a type it is */
    int o_hplus;			/* Plusses to hit */
    int o_dplus;			/* Plusses to damage */
    int o_ac;				/* Armor class */
    int o_flags;			/* Information about objects */
    int o_group;			/* Group number for this object */
};

/*
 * Now all the global variables
 */

extern struct room rooms[MAXROOMS];		/* One for each room -- A level */
extern struct room *oldrp;			/* Roomin(&oldpos) */
extern struct linked_list *mlist;		/* List of monsters on the level */
extern struct thing player;			/* The rogue */
extern struct stats max_stats;			/* The maximum for the player */
extern struct monster monsters[26];		/* The initial monster states */
extern struct linked_list *lvl_obj;		/* List of objects on this level */
extern struct object *cur_weapon;		/* Which weapon he is weilding */
extern struct object *cur_armor;		/* What a well dresssed rogue wears */
extern struct object *cur_ring[2];		/* Which rings are being worn */
extern struct magic_item things[NUMTHINGS];	/* Chances for each type of item */
extern struct magic_item s_magic[MAXSCROLLS];	/* Names and chances for scrolls */
extern struct magic_item p_magic[MAXPOTIONS];	/* Names and chances for potions */
extern struct magic_item r_magic[MAXRINGS];	/* Names and chances for rings */
extern struct magic_item ws_magic[MAXSTICKS];	/* Names and chances for sticks */

extern int level;				/* What level rogue is on */
extern int purse;				/* How much gold the rogue has */
extern int mpos;				/* Where cursor is on top line */
extern int ntraps;				/* Number of traps on this level */
extern int no_move;				/* Number of turns held in place */
extern int no_command;				/* Number of turns asleep */
extern int inpack;				/* Number of things in pack */
extern int max_hp;				/* Player's max hit points */
extern int total;				/* Total dynamic memory bytes */
extern int a_chances[MAXARMORS];		/* Probabilities for armor */
extern int a_class[MAXARMORS];			/* Armor class for various armors */
extern int lastscore;				/* Score before this turn */
extern int no_food;				/* Number of levels without food */
extern int seed;				/* Random number seed */
extern int count;				/* Number of times to repeat command */
extern int dnum;				/* Dungeon number */
extern int fung_hit;				/* Number of time fungi has hit */
extern int quiet;				/* Number of quiet turns */
extern int max_level;				/* Deepest player has gone */
extern int food_left;				/* Amount of food in hero's stomach */
extern int group;				/* Current group number */
extern int hungry_state;			/* How hungry is he */
extern char fungi_damage[16];			/* Writable violet fungi damage string */

extern char take;				/* Thing the rogue is taking */
extern char prbuf[256];				/* Buffer for sprintfs */
extern char outbuf[BUFSIZ];			/* Output buffer for stdout */
extern char runch;				/* Direction player is running */
extern char *s_names[MAXSCROLLS];		/* Names of the scrolls */
extern char *p_colors[MAXPOTIONS];		/* Colors of the potions */
extern char *r_stones[MAXRINGS];		/* Stone settings of the rings */
extern int p_color_index[MAXPOTIONS];	/* Color index for each potion */
extern int r_stone_index[MAXRINGS];	/* Stone index for each ring */
extern int ws_material_index[MAXSTICKS];	/* Material index for each stick */
extern bool ws_material_is_wood[MAXSTICKS]; /* TRUE for staff/wood */
extern int scroll_syll_count[MAXSCROLLS];
extern int scroll_syll_ids[MAXSCROLLS][MAX_SCROLL_SYLLABLES];
extern bool scroll_syll_breaks[MAXSCROLLS][MAX_SCROLL_SYLLABLES];
extern char *w_names[MAXWEAPONS];		/* Names of the various weapons */
extern char *a_names[MAXARMORS];		/* Names of armor types */
extern char *ws_made[MAXSTICKS];		/* What sticks are made of */
extern char *release;				/* Release number of rogue */
extern char whoami[ROGUE_STRING_SIZE];			/* Name of player */
extern char fruit[ROGUE_STRING_SIZE];				/* Favorite fruit */
extern char huh[HUH_SIZE];			/* The last message printed */
extern char *s_guess[MAXSCROLLS];		/* Players guess at what scroll is */
extern char *p_guess[MAXPOTIONS];		/* Players guess at what potion is */
extern char *r_guess[MAXRINGS];		/* Players guess at what ring is */
extern char *ws_guess[MAXSTICKS];		/* Players guess at what wand is */
extern char *ws_type[MAXSTICKS];		/* Is it a wand or a staff */
extern char file_name[ROGUE_PATH_SIZE];			/* Save file name */
extern char home[ROGUE_PATH_SIZE];				/* User's home directory */

extern WINDOW *cw;				/* Window that the player sees */
extern WINDOW *hw;				/* Used for the help command */
extern WINDOW *mw;				/* Used to store mosnters */

extern bool running;				/* True if player is running */
extern bool playing;				/* True until he quits */
extern bool wizard;				/* True if allows wizard commands */
extern bool after;				/* True if we want after daemons */
extern bool notify;				/* True if player wants to know */
extern bool fight_flush;			/* True if toilet input */
extern bool terse;				/* True if we should be short */
extern bool door_stop;				/* Stop running when we pass a door */
extern bool jump;				/* Show running as series of jumps */
extern bool slow_invent;			/* Inventory one line at a time */
extern bool firstmove;				/* First move after setting door_stop */
extern bool waswizard;				/* Was a wizard sometime */
extern bool askme;				/* Ask about unidentified things */
extern bool s_know[MAXSCROLLS];		/* Does he know what a scroll does */
extern bool p_know[MAXPOTIONS];		/* Does he know what a potion does */
extern bool r_know[MAXRINGS];			/* Does he know what a ring does */
extern bool ws_know[MAXSTICKS];		/* Does he know what a stick does */
extern bool amulet;				/* He found the amulet */
extern bool in_shell;				/* True if executing a shell */

extern coord oldpos;				/* Position before last look() call */
extern coord delta;				/* Change indicated to get_dir() */











/* Project function declarations for modern compilers. */
void _attach(struct linked_list **list, struct linked_list *item); 
void _detach(struct linked_list **list, struct linked_list *item); 
void _free_list(struct linked_list **ptr); int add_dam(str_t *str); void add_haste(bool potion); 
void add_pack(struct linked_list *item, bool silent); void add_pass(void); 
void addmsg(const char *fmt, ...); void aggravate(void); int attack(struct thing *mp); int author(void); 
void auto_save(int sig); void badcheck(const char *name, struct magic_item *magic, int bound); 
int be_trapped(coord *tc); void bounce(struct object *weap, char *mname); void call(void); 
int cansee(int y, int x); char * charge_str(struct object *obj); 
int chase(struct thing *tp, coord *ee); void check_level(void); void checkout(void); 
void chg_str(int amt); void chmsg(const char *fmt, int arg); void command(void); 
void conn(int r1, int r2); void create_obj(void); void d_level(void); 
struct delayed_action * d_slot(void); void daemon(void (*func)(int), int arg, int type); int rogue_save_daemons(FILE *fp); int rogue_restore_daemons(FILE *fp); 
void death(char monst); int diag_ok(coord *sp, coord *ep); void discard(struct linked_list *item); 
int do_chase(struct thing *th); void do_daemons(int flag); void do_fuses(int flag); 
void do_motion(struct object *obj, int ydelta, int xdelta); void do_move(int dy, int dx); 
void do_passages(void); void do_rooms(void); void do_run(char ch); void do_zap(bool gotdir); 
void doadd(const char *fmt, va_list ap); void doctor(int arg); void door(struct room *rm, coord *cp); 
void drain(int ymin, int ymax, int xmin, int xmax); void draw_room(struct room *rp); 
void drop(void); int dropcheck(struct object *op); void eat(void); 
int encread(char *start, unsigned int size, int inf); 
void encwrite(char *start, unsigned int size, FILE *outf); void endit(int sig); void endmsg(void); 
void extinguish(void (*func)(int)); void fall(struct linked_list *item, bool pr); 
int fallpos(coord *pos, coord *newpos, bool passages); void fatal(const char *s); 
int fight(coord *mp, char mn, struct object *weap, bool thrown); 
struct linked_list * find_mons(int y, int x); struct linked_list * find_obj(int y, int x); 
struct delayed_action * find_slot(void (*func)(int)); void fix_stick(struct object *cur); 
void fuse(void (*func)(int), int arg, int time, int type); void genocide(void); 
int get_bool(void *vbp, WINDOW *win); int get_dir(void); 
struct linked_list * get_item(const char *purpose, int type); int get_str(void *vopt, WINDOW *win); 
int gethand(void); void help(void); void hit(char *er, char *ee); 
int hit_monster(int y, int x, struct object *obj); void horiz(int cnt); void identify(void); 
void init_colors(void); void init_materials(void); void init_names(void); void init_player(void); 
void init_stones(void); void init_things(void); void init_weapon(struct object *weap, int type); 
void jrogue_rebuild_scroll_names(void); void jrogue_rebuild_random_names(void); 
char * inv_name(struct object *obj, bool drop); int inventory(struct linked_list *list, int type); 
int is_current(struct object *obj); int is_magic(struct object *obj); 
void kill_daemon(void (*func)(int)); void killed(struct linked_list *item, bool pr); 
char * killname(char monst); void lengthen(void (*func)(int), int xtime); void light(coord *cp); 
void loadav(double *avg); void look(bool wakeup); void miss(char *er, char *ee); 
void missile(int ydelta, int xdelta); void money(void); void msg(const char *fmt, ...); 
char * new(int size); struct linked_list * new_item(int size); void new_level(void); 
void new_monster(struct linked_list *item, char type, coord *cp); 
struct linked_list * new_thing(void); void nohaste(int arg); char * num(int n1, int n2); 
void option(void); int pack_char(struct object *obj); void parse_opts(char *str); int passwd(void); int wizard_password_ok(char *plain); 
int pick_one(struct magic_item *magic, int nitems); void pick_up(char ch); void picky_inven(void); 
void playit(void); char * prname(char *who, bool upper); void put_bool(void *b); 
void put_str(void *str); void put_things(void); void quaff(void); void quit(int sig); 
void raise_level(void); int randmonster(bool wander); void read_scroll(void); int readchar(void); void rogue_flush_typeahead(void); void rogue_animation_delay(void); 
void remove(coord *mp, struct linked_list *item); int restore(char *file, char **envp); 
int ring_eat(int hand); char * ring_num(struct object *obj); void ring_off(void); 
void ring_on(void); int rnd(int range); void rnd_pos(struct room *rp, coord *cp); 
int rnd_room(void); coord * rndmove(struct thing *who); int roll(int number, int sides); 
int roll_em(struct stats *att, struct stats *def, struct object *weap, bool hurl); 
void rollwand(int arg); struct room * roomin(coord *cp); void runners(int arg); 
void runto(coord *runner, coord *spot); int save(int which);
int save_game(void); int save_file_is_present(void); int save_throw(int which, struct thing *tp); 
void score(int amount, int flags, char monst); void search(void); int secretdoor(int y, int x); 
void setup(void); void shell(void); int show(int y, int x); 
void show_win(WINDOW *scr, char *message); void sight(int arg); void status(void); 
int step_ok(int ch); void stomach(int arg); int str_plus(str_t *str); 
void swander(int arg);
int swing(int at_lvl, int op_arm, int wplus); void take_off(void); int teleport(void); 
void thunk(struct object *weap, char *mname); int too_much(void); void total_winner(void); 
const char * tr_name(char ch); struct trap * trap_at(int y, int x); void tstp(int sig); 
void u_level(void); int ucount(void); void unconfuse(int arg); char * unctrl(char ch); 
void unsee(int arg); void vert(int cnt); char * vowelstr(char *str); void wait_for(char ch); void wait_for_window(WINDOW *win, char ch); 
struct linked_list * wake_monster(int y, int x); void wanderer(void); void waste_time(void); 
void wear(void); void whatis(void); void wield(void); 









/* Portable replacement for obsolete getpass() prototypes missing on macOS. */
char *rogue_getpass(const char *prompt);
