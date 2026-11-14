/*
 * global variable initializaton
 *
 * @(#)init.c	3.33 (Berkeley) 6/15/81
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

#define ___ 1
#define _x {1,1}
struct monster monsters[26] = {
	/* Name		 CARRY	FLAG    str, exp, lvl, amr, hpt, dmg */
	{ JSTR_STATIC("巨大アリ", "giant ant"),	 0,	ISMEAN,	{ _x, 10,   2,   3, ___, "1d6" } },
	{ JSTR_STATIC("こうもり", "bat"),	 0,	0,	{ _x,  1,   1,   3, ___, "1d2" } },
	{ JSTR_STATIC("ケンタウロス", "centaur"),	 15,	0,	{ _x, 15,   4,   4, ___, "1d6/1d6" } },
	{ JSTR_STATIC("ドラゴン", "dragon"),	 100,	ISGREED,{ _x,9000, 10,  -1, ___, "1d8/1d8/3d10" } },
	{ JSTR_STATIC("浮遊する目", "floating eye"),0,	0,	{ _x,  5,   1,   9, ___, "0d0" } },
	{ JSTR_STATIC("菫の大キノコ", "violet fungi"),0,	ISMEAN,	{ _x, 85,   8,   3, ___, fungi_damage } },
	{ JSTR_STATIC("ノーム", "gnome"),	 10,	0,	{ _x,  8,   1,   5, ___, "1d6" } },
	{ JSTR_STATIC("ホブゴブリン", "hobgoblin"),	 0,	ISMEAN,	{ _x,  3,   1,   5, ___, "1d8" } },
	{ JSTR_STATIC("見えざる追手", "invisible stalker"),0,ISINVIS,{ _x,120,   8,   3, ___, "4d4" } },
	{ JSTR_STATIC("ジャッカル", "jackal"),	 0,	ISMEAN,	{ _x,  2,   1,   7, ___, "1d2" } },
	{ JSTR_STATIC("コボルド", "kobold"),	 0,	ISMEAN,	{ _x,  1,   1,   7, ___, "1d4" } },
	{ JSTR_STATIC("レプラコーン", "leprechaun"),	 0,	0,	{ _x, 10,   3,   8, ___, "1d1" } },
	{ JSTR_STATIC("ミミック", "mimic"),	 30,	0,	{ _x,140,   7,   7, ___, "3d4" } },
	{ JSTR_STATIC("ニンフ", "nymph"),	 100,	0,	{ _x, 40,   3,   9, ___, "0d0" } },
	{ JSTR_STATIC("オーク", "orc"),	 15,	ISBLOCK,{ _x,  5,   1,   6, ___, "1d8" } },
	{ JSTR_STATIC("紫の巨虫", "purple worm"), 70,	0,	{ _x,7000, 15,   6, ___, "2d12/2d4" } },
	{ JSTR_STATIC("小悪魔", "quasit"),	 30,	ISMEAN,	{ _x, 35,   3,   2, ___, "1d2/1d2/1d4" } },
	{ JSTR_STATIC("さびの怪物", "rust monster"),0,	ISMEAN,	{ _x, 25,   5,   2, ___, "0d0/0d0" } },
	{ JSTR_STATIC("へび", "snake"),	 0,	ISMEAN,	{ _x,  3,   1,   5, ___, "1d3" } },
	{ JSTR_STATIC("トロル", "troll"),	 50,	ISREGEN|ISMEAN,{ _x, 55,   6,   4, ___, "1d8/1d8/2d6" } },
	{ JSTR_STATIC("褐色の大あご虫", "umber hulk"),	 40,	ISMEAN,	{ _x,130,   8,   2, ___, "3d4/3d4/2d5" } },
	{ JSTR_STATIC("ヴァンパイア", "vampire"),	 20,	ISREGEN|ISMEAN,{ _x,380,   8,   1, ___, "1d10" } },
	{ JSTR_STATIC("死霊", "wraith"),	 0,	0,	{ _x, 55,   5,   4, ___, "1d6" } },
	{ JSTR_STATIC("ゾーン", "xorn"),	 0,	ISMEAN,	{ _x,120,   7,  -2, ___, "1d3/1d3/1d3/4d6" } },
	{ JSTR_STATIC("イエティ", "yeti"),	 30,	0,	{ _x, 50,   4,   6, ___, "1d6/1d6" } },
	{ JSTR_STATIC("ゾンビ", "zombie"),	 0,	ISMEAN,	{ _x,  7,   2,   8, ___, "1d8" } }
};
#undef ___


/*
 * init_player:
 *	roll up the rogue
 */

void init_player(void)
{
    pstats.s_lvl = 1;
    pstats.s_exp = 0L;
    max_hp = pstats.s_hpt = 12;
    if (rnd(100) == 7)
    {
	pstats.s_str.st_str = 18;
	pstats.s_str.st_add = rnd(100) + 1;
    }
    else
    {
	pstats.s_str.st_str = 16;
	pstats.s_str.st_add = 0;
    }
    pstats.s_dmg = "1d4";
    pstats.s_arm = 10;
    max_stats = pstats;
    pack = NULL;
}

/*
 * Contains defintions and functions for dealing with things like
 * potions and scrolls
 */

static char *rainbow[] = {
    "red",
    "blue",
    "green",
    "yellow",
    "black",
    "brown",
    "orange",
    "pink",
    "purple",
    "grey",
    "white",
    "silver",
    "gold",
    "violet",
    "clear",
    "vermilion",
    "ecru",
    "turquoise",
    "magenta",
    "amber",
    "topaz",
    "plaid",
    "tan",
    "tangerine"
};

static char *rainbow_ja[] = {
    "赤色",
    "青色",
    "緑色",
    "黄色",
    "黒色",
    "茶色",
    "オレンジ色",
    "ピンク色",
    "紫色",
    "灰色",
    "白色",
    "銀色",
    "金色",
    "すみれ色",
    "透明",
    "朱色",
    "ベージュ色",
    "青緑色",
    "赤紫色",
    "こはく色",
    "淡褐色",
    "格子柄",
    "淡い茶色",
    "みかん色"
};

#define NCOLORS (sizeof rainbow / sizeof (char *))

static char *sylls_en[] = {
    "a", "ab", "ag", "aks", "ala", "an", "ankh", "app",
    "arg", "arze", "ash", "ban", "bar", "bat", "bek", "bie",
    "bin", "bit", "bjor", "blu", "bot", "bu", "byt", "comp",
    "con", "cos", "cre", "dalf", "dan", "den", "do", "e",
    "eep", "el", "eng", "er", "ere", "erk", "esh", "evs",
    "fa", "fid", "for", "fri", "fu", "gan", "gar", "glen",
    "gop", "gre", "ha", "he", "hyd", "i", "ing", "ion",
    "ip", "ish", "it", "ite", "iv", "jo", "kho", "kli",
    "klis", "la", "lech", "man", "mar", "me", "mi", "mic",
    "mik", "mon", "mung", "mur", "nej", "nelg", "nep", "ner",
    "nes", "nes", "nih", "nin", "o", "od", "ood", "org",
    "orn", "ox", "oxy", "pay", "pet", "ple", "plu", "po",
    "pot", "prok", "re", "rea", "rhov", "ri", "ro", "rog",
    "rok", "rol", "sa", "san", "sat", "see", "sef", "seh",
    "shu", "ski", "sna", "sne", "snik", "sno", "so", "sol",
    "sri", "sta", "sun", "ta", "tab", "tem", "ther", "ti",
    "tox", "trol", "tue", "turs", "u", "ulk", "um", "un",
    "uni", "ur", "val", "viv", "vly", "vom", "wah", "wed",
    "werg", "wex", "whon", "wun", "xo", "y", "yot", "yu",
    "zant", "zap", "zeb", "zim", "zok", "zon", "zum"
};

static char *sylls_ja[] = {
    "ア", "アブ", "アグ", "アクス", "アラ", "アン", "アンク", "アプ",
    "アーグ", "アーゼ", "アッシュ", "バン", "バー", "バット", "ベク", "ビー",
    "ビン", "ビット", "ビジョー", "ブル", "ボット", "ブ", "バイト", "コンプ",
    "コン", "コス", "クレ", "ダルフ", "ダン", "デン", "ド", "エ",
    "イープ", "エル", "エング", "エー", "エレ", "イーク", "エッシュ", "エブス",
    "ファ", "フィッド", "フォー", "フライ", "フー", "ガン", "ガー", "グレン",
    "ゴップ", "グレ", "ハー", "ヘ", "ヒド", "イ", "イング", "イオン",
    "イップ", "イッシュ", "イット", "アイト", "イブ", "ジョー", "コー", "クリ",
    "クリス", "ラー", "レック", "マン", "マー", "メ", "マイ", "マイク",
    "ミク", "モン", "ムング", "マー", "ネジュ", "ネルグ", "ネップ", "ナー",
    "ネス", "ネス", "ニー", "ニン", "オ", "オッド", "オード", "オーグ",
    "オルン", "オックス", "オキシー", "ペイ", "ペット", "プル", "プルー", "ポ",
    "ポット", "プロック", "リ", "レー", "ロブ", "リー", "ロ", "ログ",
    "ロク", "ロル", "サ", "サン", "サット", "シー", "セフ", "ゼー",
    "シュ", "スキ", "スネイ", "スネ", "スニック", "スノ", "ソ", "ソル",
    "スリ", "スタ", "サン", "タ", "タブ", "テム", "ザー", "ティ",
    "トックス", "トロール", "チュー", "ターズ", "ウ", "ウルク", "ウム", "ウン",
    "ウニ", "アー", "ヴァル", "ヴィヴ", "ヴライ", "ヴォム", "ワー", "ウェド",
    "ワーグ", "ウェックス", "ホン", "ウン", "クソー", "イ", "ヨット", "ユ",
    "ザント", "ザップ", "ゼブ", "ジム", "ゾック", "ゾン", "ズム"
};

#define NSYLLS (sizeof sylls_en / sizeof (char *))

static char *stones[] = {
    "Agate",
    "Alexandrite",
    "Amethyst",
    "Carnelian",
    "Diamond",
    "Emerald",
    "Granite",
    "Jade",
    "Kryptonite",
    "Lapis lazuli",
    "Moonstone",
    "Obsidian",
    "Onyx",
    "Opal",
    "Pearl",
    "Ruby",
    "Sapphire",
    "Tiger eye",
    "Topaz",
    "Turquoise",
};

static char *stones_ja[] = {
    "めのう",
    "アレキサンドライト",
    "紫水晶",
    "カーネリアン",
    "ダイヤ",
    "エメラルド",
    "御影石",
    "ひすい",
    "クリプトナイト",
    "るり",
    "月長石",
    "黒曜石",
    "オニキス",
    "オパール",
    "真珠",
    "ルビー",
    "サファイア",
    "虎目石",
    "トパーズ",
    "トルコ石"
};

#define NSTONES (sizeof stones / sizeof (char *))

static char *wood[] = {
    "Avocado wood",
    "Balsa",
    "Banyan",
    "Birch",
    "Cedar",
    "Cherry",
    "Cinnibar",
    "Driftwood",
    "Ebony",
    "Eucalyptus",
    "Hemlock",
    "Ironwood",
    "Mahogany",
    "Manzanita",
    "Maple",
    "Oak",
    "Persimmon wood",
    "Redwood",
    "Rosewood",
    "Teak",
    "Walnut",
    "Zebra wood",
};

static char *wood_ja[] = {
    "アボカド",
    "バルサ",
    "バニヤン",
    "白樺",
    "杉",
    "桜",
    "辰砂",
    "流木",
    "黒檀",
    "ユーカリ",
    "ツガ",
    "鉄樹",
    "マホガニー",
    "マンザニータ",
    "楓",
    "樫",
    "柿",
    "アメリカ杉",
    "紫檀",
    "チーク",
    "胡桃",
    "ゼブラウッド"
};

#define NWOOD (sizeof wood / sizeof (char *))

static char *metal[] = {
    "Aluminium",
    "Bone",
    "Brass",
    "Bronze",
    "Copper",
    "Iron",
    "Lead",
    "Pewter",
    "Steel",
    "Tin",
    "Zinc",
};

static char *metal_ja[] = {
    "アルミニウム",
    "骨",
    "真ちゅう",
    "青銅",
    "銅",
    "鉄",
    "鉛",
    "ピューター",
    "鋼鉄",
    "すず",
    "亜鉛"
};

#define NMETAL (sizeof metal / sizeof (char *))

struct magic_item things[NUMTHINGS] = {
    { 0,			27, 0 },	/* potion */
    { 0,			27, 0 },	/* scroll */
    { 0,			18, 0 },	/* food */
    { 0,			 9, 0 },	/* weapon */
    { 0,			 9, 0 },	/* armor */
    { 0,			 5, 0 },	/* ring */
    { 0,			 5, 0 },	/* stick */
};
struct magic_item s_magic[MAXSCROLLS] = {
    { JSTR_STATIC("怪物を混乱させる", "monster confusion"),	 8, 170 },
    { JSTR_STATIC("魔法の地図の", "magic mapping"),		 5, 180 },
    { JSTR_STATIC("灯りの", "light"),			10, 100 },
    { JSTR_STATIC("怪物を封じこめる", "hold monster"),		 2, 200 },
    { JSTR_STATIC("眠りにおちる", "sleep"),			 5,  50 },
    { JSTR_STATIC("よろいに魔法をかける", "enchant armor"),		 8, 130 },
    { JSTR_STATIC("持ちものを識別する", "identify"),		21, 100 },
    { JSTR_STATIC("怪物を怖がらせる", "scare monster"),		 4, 180 },
    { JSTR_STATIC("金塊を見つける", "gold detection"),		 4, 110 },
    { JSTR_STATIC("テレポートする", "teleportation"),		 7, 175 },
    { JSTR_STATIC("武器に魔法をかける", "enchant weapon"),		10, 150 },
    { JSTR_STATIC("怪物を作りだす", "create monster"),		 5,  75 },
    { JSTR_STATIC("のろいを解く", "remove curse"),		 8, 105 },
    { JSTR_STATIC("怪物を怒らせる", "aggravate monsters"),	 1,  60 },
    { JSTR_STATIC("白紙の", "blank paper"),		 1,  50 },
    { JSTR_STATIC("怪物を根絶やしにする", "genocide"),		 1, 200 },
};

struct magic_item p_magic[MAXPOTIONS] = {
    { JSTR_STATIC("頭が混乱する", "confusion"),		 8,  50 },
    { JSTR_STATIC("体がしびれる", "paralysis"),		10,  50 },
    { JSTR_STATIC("毒の", "poison"),			 8,  50 },
    { JSTR_STATIC("強さが増す", "gain strength"),		15, 150 },
    { JSTR_STATIC("見えないものが見える", "see invisible"),		 2, 170 },
    { JSTR_STATIC("傷を治す", "healing"),		15, 130 },
    { JSTR_STATIC("怪物探知の", "monster detection"),	 6, 120 },
    { JSTR_STATIC("魔法探知の", "magic detection"),	 6, 105 },
    { JSTR_STATIC("レベルが上がる", "raise level"),		 2, 220 },
    { JSTR_STATIC("とてもよく傷を治す", "extra healing"),		 5, 180 },
    { JSTR_STATIC("素早くなる", "haste self"),		 4, 200 },
    { JSTR_STATIC("強さが元にもどる", "restore strength"),	14, 120 },
    { JSTR_STATIC("目が見えなくなる", "blindness"),		 4,  50 },
    { JSTR_STATIC("のどを潤す", "thirst quenching"),	 1,  50 },
};

struct magic_item r_magic[MAXRINGS] = {
    { JSTR_STATIC("守りの", "protection"),		 9, 200 },
    { JSTR_STATIC("強さが増す", "add strength"),		 9, 200 },
    { JSTR_STATIC("強さが減らない", "sustain strength"),	 5, 180 },
    { JSTR_STATIC("探索の", "searching"),		10, 200 },
    { JSTR_STATIC("見えないものが見える", "see invisible"),		10, 175 },
    { JSTR_STATIC("飾りものの", "adornment"),		 1, 100 },
    { JSTR_STATIC("怪物を怒らせる", "aggravate monster"),	11, 100 },
    { JSTR_STATIC("機敏になる", "dexterity"),		 8, 220 },
    { JSTR_STATIC("ダメージを増やす", "increase damage"),	 8, 220 },
    { JSTR_STATIC("再生の", "regeneration"),		 4, 260 },
    { JSTR_STATIC("消化が遅くなる", "slow digestion"),		 9, 240 },
    { JSTR_STATIC("テレポートする", "telportation"),		 9, 100 },
    { JSTR_STATIC("身を隠す", "stealth"),		 7, 100 },
};

struct magic_item ws_magic[MAXSTICKS] = {
    { JSTR_STATIC("灯りの", "light"),			12, 120 },
    { JSTR_STATIC("打撃の", "striking"),		 9, 115 },
    { JSTR_STATIC("いなずまの", "lightning"),		 3, 200 },
    { JSTR_STATIC("火の", "fire"),			 3, 200 },
    { JSTR_STATIC("冷気の", "cold"),			 3, 200 },
    { JSTR_STATIC("変身させる", "polymorph"),		15, 210 },
    { JSTR_STATIC("魔法のミサイルの", "magic missile"),		10, 170 },
    { JSTR_STATIC("怪物を素早くする", "haste monster"),		 9,  50 },
    { JSTR_STATIC("怪物を遅くする", "slow monster"),		11, 220 },
    { JSTR_STATIC("いのちを削る", "drain life"),		 9, 210 },
    { JSTR_STATIC("役に立たない", "nothing"),		 1,  70 },
    { JSTR_STATIC("怪物を遠くに飛ばす", "teleport away"),		 5, 140 },
    { JSTR_STATIC("怪物を引き寄せる", "teleport to"),		 5,  60 },
    { JSTR_STATIC("魔力を封じる", "cancellation"),		 5, 130 },
};

int a_class[MAXARMORS] = {
    8,
    7,
    7,
    6,
    5,
    4,
    4,
    3,
};

char *a_names[MAXARMORS] = {
    JSTR_STATIC("革のよろい", "leather armor"),
    JSTR_STATIC("鉄環のよろい", "ring mail"),
    JSTR_STATIC("鋲つき革のよろい", "studded leather armor"),
    JSTR_STATIC("鱗のよろい", "scale mail"),
    JSTR_STATIC("鎖のよろい", "chain mail"),
    JSTR_STATIC("鉄片のよろい", "splint mail"),
    JSTR_STATIC("帯金のよろい", "banded mail"),
    JSTR_STATIC("全身のよろい", "plate mail"),
};

int a_chances[MAXARMORS] = {
    20,
    35,
    50,
    63,
    75,
    85,
    95,
    100
};

/*
 * init_things
 *	Initialize the probabilities for types of things
 */
void init_things(void)
{
    register struct magic_item *mp;

    for (mp = &things[1]; mp < &things[NUMTHINGS]; mp++)
	mp->mi_prob += (mp-1)->mi_prob;
    badcheck(JSTR("品物", "things"), things, NUMTHINGS);
}


static void
replace_string(char **dst, const char *src)
{
    if (*dst != NULL)
	FREE(*dst);
    *dst = new(strlen(src) + 1);
    strcpy(*dst, src);
}

static int
ascii_streq_ci(const char *a, const char *b)
{
    unsigned char ca, cb;

    if (a == NULL || b == NULL)
	return FALSE;
    while (*a && *b)
    {
	ca = (unsigned char) *a++;
	cb = (unsigned char) *b++;
	if (ca < 128)
	    ca = (unsigned char) tolower(ca);
	if (cb < 128)
	    cb = (unsigned char) tolower(cb);
	if (ca != cb)
	    return FALSE;
    }
    return *a == '\0' && *b == '\0';
}

static int
find_pair_name(const char *name, char **en, char **ja, int n)
{
    int i;

    if (name == NULL)
	return -1;
    for (i = 0; i < n; i++)
	if (ascii_streq_ci(name, en[i]) || strcmp(name, ja[i]) == 0)
	    return i;
    return -1;
}

static const char *
potion_color(int which)
{
    return jrogue_lang_is_japanese ? rainbow_ja[which] : rainbow[which];
}

static const char *
ring_stone(int which)
{
    return jrogue_lang_is_japanese ? stones_ja[which] : stones[which];
}

static const char *
stick_material(int woodp, int which)
{
    return woodp ? (jrogue_lang_is_japanese ? wood_ja[which] : wood[which])
		 : (jrogue_lang_is_japanese ? metal_ja[which] : metal[which]);
}

void
jrogue_rebuild_scroll_names(void)
{
    int i, n, id;
    size_t len;
    char *name, *cp, *sp;
    char **table;

    table = jrogue_lang_is_japanese ? sylls_ja : sylls_en;
    for (i = 0; i < MAXSCROLLS; i++)
    {
	if (scroll_syll_count[i] <= 0 || scroll_syll_count[i] > MAX_SCROLL_SYLLABLES)
	    continue;
	len = 0;
	for (n = 0; n < scroll_syll_count[i]; n++)
	{
	    id = scroll_syll_ids[i][n];
	    if (id < 0 || id >= (int) NSYLLS)
		continue;
	    len += strlen(table[id]);
	    if (scroll_syll_breaks[i][n] && n + 1 < scroll_syll_count[i])
		len++;
	}
	name = new((int) len + 1);
	cp = name;
	for (n = 0; n < scroll_syll_count[i]; n++)
	{
	    id = scroll_syll_ids[i][n];
	    if (id < 0 || id >= (int) NSYLLS)
		continue;
	    sp = table[id];
	    while (*sp)
		*cp++ = *sp++;
	    if (scroll_syll_breaks[i][n] && n + 1 < scroll_syll_count[i])
		*cp++ = ' ';
	}
	*cp = '\0';
	replace_string(&s_names[i], name);
	FREE(name);
    }
}

void
jrogue_rebuild_random_names(void)
{
    int i, which;

    for (i = 0; i < MAXPOTIONS; i++)
    {
	if (p_colors[i] == NULL)
	    continue;
	which = find_pair_name(p_colors[i], rainbow, rainbow_ja, (int) NCOLORS);
	if (which >= 0)
	    p_color_index[i] = which;
	replace_string(&p_colors[i], potion_color(p_color_index[i]));
    }
    for (i = 0; i < MAXRINGS; i++)
    {
	if (r_stones[i] == NULL)
	    continue;
	which = find_pair_name(r_stones[i], stones, stones_ja, (int) NSTONES);
	if (which >= 0)
	    r_stone_index[i] = which;
	replace_string(&r_stones[i], ring_stone(r_stone_index[i]));
    }
    for (i = 0; i < MAXSTICKS; i++)
    {
	if (ws_made[i] == NULL)
	    continue;
	which = find_pair_name(ws_made[i], wood, wood_ja, (int) NWOOD);
	if (which >= 0)
	{
	    ws_material_is_wood[i] = TRUE;
	    ws_material_index[i] = which;
	}
	else
	{
	    which = find_pair_name(ws_made[i], metal, metal_ja, (int) NMETAL);
	    if (which >= 0)
	    {
		ws_material_is_wood[i] = FALSE;
		ws_material_index[i] = which;
	    }
	}
	ws_type[i] = (char *) (ws_material_is_wood[i] ? JSTR("つえ", "staff") : JSTR("魔法の棒", "wand"));
	replace_string(&ws_made[i], stick_material(ws_material_is_wood[i], ws_material_index[i]));
    }
    if (s_names[0] != NULL)
	jrogue_rebuild_scroll_names();
}

/*
 * init_colors:
 *	Initialize the potion color scheme for this time
 */

void init_colors(void)
{
    register int i;
    register int which;
    bool used[NCOLORS];

    for (i = 0; i < (int) NCOLORS; i++)
	used[i] = FALSE;

    for (i = 0; i < MAXPOTIONS; i++)
    {
	do
	    which = rnd(NCOLORS);
	until (!used[which]);
	used[which] = TRUE;
	p_color_index[i] = which;
	replace_string(&p_colors[i], potion_color(which));
	p_know[i] = FALSE;
	p_guess[i] = NULL;
	if (i > 0)
		p_magic[i].mi_prob += p_magic[i-1].mi_prob;
    }
    badcheck(JSTR("水薬", "potions"), p_magic, MAXPOTIONS);
}

/*
 * init_names:
 *	Generate the names of the various scrolls
 */

#define MAXNAME 40	/* Max bytes in a scroll title, following 5.4 */

static size_t
scroll_syll_len(int id)
{
    size_t en_len, ja_len;

    if (id < 0 || id >= (int) NSYLLS)
	return 0;
    en_len = strlen(sylls_en[id]);
    ja_len = strlen(sylls_ja[id]);
    return en_len > ja_len ? en_len : ja_len;
}

void init_names(void)
{
    register int nsyl;
    register int i, nwords, n, id;
    size_t len, add;
    bool broke;

    for (i = 0; i < MAXSCROLLS; i++)
    {
	nwords = rnd(4)+2;
	n = 0;
	len = 0;
	broke = FALSE;
	while(nwords-- && n < MAX_SCROLL_SYLLABLES && !broke)
	{
	    nsyl = rnd(3)+1;
	    while(nsyl-- && n < MAX_SCROLL_SYLLABLES)
	    {
		id = rnd((int) NSYLLS);
		add = scroll_syll_len(id);
		if (len + add > MAXNAME)
		{
		    broke = TRUE;
		    break;
		}
		scroll_syll_ids[i][n] = id;
		scroll_syll_breaks[i][n] = FALSE;
		len += add;
		n++;
	    }
	    if (n > 0 && !broke && nwords > 0 && len + 1 <= MAXNAME)
	    {
		scroll_syll_breaks[i][n - 1] = TRUE;
		len++;
	    }
	}
	scroll_syll_count[i] = n;
	s_know[i] = FALSE;
	s_guess[i] = NULL;
	if (i > 0)
		s_magic[i].mi_prob += s_magic[i-1].mi_prob;
    }
    jrogue_rebuild_scroll_names();
    badcheck(JSTR("巻き物", "scrolls"), s_magic, MAXSCROLLS);
}

/*
 * init_stones:
 *	Initialize the ring stone setting scheme for this time
 */

void init_stones(void)
{
    register int i;
    register int which;
    bool used[NSTONES];

    for (i = 0; i < (int) NSTONES; i++)
	used[i] = FALSE;

    for (i = 0; i < MAXRINGS; i++)
    {
	do
	    which = rnd(NSTONES);
	until (!used[which]);
	used[which] = TRUE;
	r_stone_index[i] = which;
	replace_string(&r_stones[i], ring_stone(which));
	r_know[i] = FALSE;
	r_guess[i] = NULL;
	if (i > 0)
		r_magic[i].mi_prob += r_magic[i-1].mi_prob;
    }
    badcheck(JSTR("指輪", "rings"), r_magic, MAXRINGS);
}

/*
 * init_materials:
 *	Initialize the construction materials for wands and staffs
 */

void init_materials(void)
{
    register int i;
    register int which;
    bool used_metal[NMETAL];
    bool used_wood[NWOOD];

    for (i = 0; i < (int) NMETAL; i++)
	used_metal[i] = FALSE;
    for (i = 0; i < (int) NWOOD; i++)
	used_wood[i] = FALSE;

    for (i = 0; i < MAXSTICKS; i++)
    {
	if (rnd(100) > 50)
	{
	    do
		which = rnd(NMETAL);
	    until (!used_metal[which]);
	    used_metal[which] = TRUE;
	    ws_material_is_wood[i] = FALSE;
	    ws_material_index[i] = which;
	    ws_type[i] = (char *) JSTR("魔法の棒", "wand");
	}
	else
	{
	    do
		which = rnd(NWOOD);
	    until (!used_wood[which]);
	    used_wood[which] = TRUE;
	    ws_material_is_wood[i] = TRUE;
	    ws_material_index[i] = which;
	    ws_type[i] = (char *) JSTR("つえ", "staff");
	}
	replace_string(&ws_made[i], stick_material(ws_material_is_wood[i], which));
	ws_know[i] = FALSE;
	ws_guess[i] = NULL;
	if (i > 0)
		ws_magic[i].mi_prob += ws_magic[i-1].mi_prob;
    }
    badcheck(JSTR("つえ／魔法の棒", "sticks"), ws_magic, MAXSTICKS);
}

void badcheck(const char *name, struct magic_item *magic, int bound)
{
    register struct magic_item *end;

    if (magic[bound - 1].mi_prob == 100)
	return;
    printf(JSTR("\n%sの出現率が正しくありません:\n",
		"\nBad percentages for %s:\n"), name);
    for (end = &magic[bound]; magic < end; magic++)
	printf("%3d%% %s\n", magic->mi_prob, magic->mi_name);
    fputs(JSTR("［リターンを押してください］", "[hit RETURN to continue]"), stdout);
    fflush(stdout);
    while (getchar() != '\n')
	continue;
}

struct h_list helpstr[] = {
    { '?',	JSTR_STATIC("	ヘルプを表示する", "	prints help") },
    { '/',	JSTR_STATIC("	画面の文字が表すものを表示", "	identify object") },
    { 'h',	JSTR_STATIC("	左に進む", "	left") },
    { 'j',	JSTR_STATIC("	下に進む", "	down") },
    { 'k',	JSTR_STATIC("	上に進む", "	up") },
    { 'l',	JSTR_STATIC("	右に進む", "	right") },
    { 'y',	JSTR_STATIC("	左上に進む", "	up & left") },
    { 'u',	JSTR_STATIC("	右上に進む", "	up & right") },
    { 'b',	JSTR_STATIC("	左下に進む", "	down & left") },
    { 'n',	JSTR_STATIC("	右下に進む", "	down & right") },
    { 'H',	JSTR_STATIC("	左に走る", "	run left") },
    { 'J',	JSTR_STATIC("	下に走る", "	run down") },
    { 'K',	JSTR_STATIC("	上に走る", "	run up") },
    { 'L',	JSTR_STATIC("	右に走る", "	run right") },
    { 'Y',	JSTR_STATIC("	左上に走る", "	run up & left") },
    { 'U',	JSTR_STATIC("	右上に走る", "	run up & right") },
    { 'B',	JSTR_STATIC("	左下に走る", "	run down & left") },
    { 'N',	JSTR_STATIC("	右下に走る", "	run down & right") },
    { 't',	JSTR_STATIC("<方角>	物を投げる", "<dir>	throw something") },
    { 'f',	JSTR_STATIC("<方角>	何かを見つけるまで進む", "<dir>	forward until find something") },
    { 'p',	JSTR_STATIC("<方角>	つえを振りかざす", "<dir>	zap a wand in a direction") },
    { 'z',	JSTR_STATIC("	つえか魔法の棒を振りかざす", "	zap a wand or staff") },
    { '>',	JSTR_STATIC("	階段を降りる", "	go down a staircase") },
    { 's',	JSTR_STATIC("	わなと隠しとびらを探す", "	search for trap/secret door") },
    { ' ',	JSTR_STATIC("	しばらく休む", "	(space) rest for a while") },
    { 'i',	JSTR_STATIC("	持ちものを見る", "	inventory") },
    { 'I',	JSTR_STATIC("	１つの持ちものを見る", "	inventory single item") },
    { 'q',	JSTR_STATIC("	水薬を飲む", "	quaff potion") },
    { 'r',	JSTR_STATIC("	巻き物を読む", "	read paper") },
    { 'e',	JSTR_STATIC("	食べ物を食べる", "	eat food") },
    { 'w',	JSTR_STATIC("	武器を手に持つ", "	wield a weapon") },
    { 'W',	JSTR_STATIC("	よろいを着る", "	wear armor") },
    { 'T',	JSTR_STATIC("	よろいを脱ぐ", "	take armor off") },
    { 'P',	JSTR_STATIC("	指輪をはめる", "	put on ring") },
    { 'R',	JSTR_STATIC("	指輪をはずす", "	remove ring") },
    { 'd',	JSTR_STATIC("	持ちものを落とす", "	drop object") },
    { 'c',	JSTR_STATIC("	持ちものに呼び名をつける", "	call object") },
    { 'o',	JSTR_STATIC("	オプションの表示・設定", "	examine/set options") },
    { CTRL('L'),	JSTR_STATIC("	画面を描き直す", "	redraw screen") },
    { CTRL('R'),	JSTR_STATIC("	最後のメッセージを再表示する", "	repeat last message") },
    { ESCAPE,	JSTR_STATIC("	コマンドを中止する", "	cancel command") },
    { 'v',	JSTR_STATIC("	バージョン情報を表示する", "	print program version number") },
    { '!',	JSTR_STATIC("	シェルに抜ける", "	shell escape") },
    { 'S',	JSTR_STATIC("	ゲームをセーブして終了する", "	save game") },
    { 'Q',	JSTR_STATIC("	ゲームを終了する", "	quit") },
    { 0, 0 }
};

void
jrogue_apply_language(void)
{
    int i;
    static const char *mon_ja[26] = {
	"巨大アリ", "こうもり", "ケンタウロス", "ドラゴン", "浮遊する目",
	"菫の大キノコ", "ノーム", "ホブゴブリン", "見えざる追手",
	"ジャッカル", "コボルド", "レプラコーン", "ミミック", "ニンフ",
	"オーク", "紫の巨虫", "小悪魔", "さびの怪物", "へび",
	"トロル", "褐色の大あご虫", "ヴァンパイア", "死霊", "ゾーン",
	"イエティ", "ゾンビ"
    };
    static const char *mon_en[26] = {
	"giant ant", "bat", "centaur", "dragon", "floating eye",
	"violet fungi", "gnome", "hobgoblin", "invisible stalker",
	"jackal", "kobold", "leprechaun", "mimic", "nymph",
	"orc", "purple worm", "quasit", "rust monster", "snake",
	"troll", "umber hulk", "vampire", "wraith", "xorn",
	"yeti", "zombie"
    };
    static const char *scroll_ja[MAXSCROLLS] = {
	"怪物を混乱させる", "魔法の地図の", "灯りの", "怪物を封じこめる",
	"眠りにおちる", "よろいに魔法をかける", "持ちものを識別する",
	"怪物を怖がらせる", "金塊を見つける", "テレポートする",
	"武器に魔法をかける", "怪物を作りだす", "のろいを解く",
	"怪物を怒らせる", "白紙の", "怪物を根絶やしにする"
    };
    static const char *scroll_en[MAXSCROLLS] = {
	"monster confusion", "magic mapping", "light", "hold monster",
	"sleep", "enchant armor", "identify", "scare monster",
	"gold detection", "teleportation", "enchant weapon", "create monster",
	"remove curse", "aggravate monsters", "blank paper", "genocide"
    };
    static const char *potion_ja[MAXPOTIONS] = {
	"頭が混乱する", "体がしびれる", "毒の", "強さが増す",
	"見えないものが見える", "傷を治す", "怪物探知の",
	"魔法探知の", "レベルが上がる", "とてもよく傷を治す",
	"素早くなる", "強さが元にもどる", "目が見えなくなる", "のどを潤す"
    };
    static const char *potion_en[MAXPOTIONS] = {
	"confusion", "paralysis", "poison", "gain strength",
	"see invisible", "healing", "monster detection", "magic detection",
	"raise level", "extra healing", "haste self", "restore strength",
	"blindness", "thirst quenching"
    };
    static const char *ring_ja[MAXRINGS] = {
	"守りの", "強さが増す", "強さが減らない", "探索の",
	"見えないものが見える", "飾りものの", "怪物を怒らせる",
	"機敏になる", "ダメージを増やす", "再生の", "消化が遅くなる",
	"テレポートする", "身を隠す"
    };
    static const char *ring_en[MAXRINGS] = {
	"protection", "add strength", "sustain strength", "searching",
	"see invisible", "adornment", "aggravate monster", "dexterity",
	"increase damage", "regeneration", "slow digestion", "telportation",
	"stealth"
    };
    static const char *stick_ja[MAXSTICKS] = {
	"灯りの", "打撃の", "いなずまの", "火の", "冷気の",
	"変身させる", "魔法のミサイルの", "怪物を素早くする",
	"怪物を遅くする", "いのちを削る", "役に立たない",
	"怪物を遠くに飛ばす", "怪物を引き寄せる", "魔力を封じる"
    };
    static const char *stick_en[MAXSTICKS] = {
	"light", "striking", "lightning", "fire", "cold", "polymorph",
	"magic missile", "haste monster", "slow monster", "drain life",
	"nothing", "teleport away", "teleport to", "cancellation"
    };
    static const char *armor_ja[MAXARMORS] = {
	"革のよろい", "鉄環のよろい", "鋲つき革のよろい", "鱗のよろい",
	"鎖のよろい", "鉄片のよろい", "帯金のよろい", "全身のよろい"
    };
    static const char *armor_en[MAXARMORS] = {
	"leather armor", "ring mail", "studded leather armor", "scale mail",
	"chain mail", "splint mail", "banded mail", "plate mail"
    };
    static const char *weapon_ja[MAXWEAPONS] = {
	"つちほこ", "長い剣", "短弓", "矢", "短剣", "石",
	"両手持ちの剣", "投石器", "投げ矢", "クロスボウ",
	"クロスボウの矢", "やり"
    };
    static const char *weapon_en[MAXWEAPONS] = {
	"mace", "long sword", "short bow", "arrow", "dagger", "rock",
	"two handed sword", "sling", "dart", "crossbow",
	"crossbow bolt", "spear"
    };
    static const char *help_ja[] = {
	"	ヘルプを表示する", "	画面の文字が表すものを表示", "	左に進む",
	"	下に進む", "	上に進む", "	右に進む", "	左上に進む",
	"	右上に進む", "	左下に進む", "	右下に進む", "	左に走る",
	"	下に走る", "	上に走る", "	右に走る", "	左上に走る",
	"	右上に走る", "	左下に走る", "	右下に走る", "<方角>	物を投げる",
	"<方角>	何かを見つけるまで進む", "<方角>	つえを振りかざす",
	"	つえか魔法の棒を振りかざす", "	階段を降りる", "	わなと隠しとびらを探す",
	"	しばらく休む", "	持ちものを見る", "	１つの持ちものを見る",
	"	水薬を飲む", "	巻き物を読む", "	食べ物を食べる", "	武器を手に持つ",
	"	よろいを着る", "	よろいを脱ぐ", "	指輪をはめる", "	指輪をはずす",
	"	持ちものを落とす", "	持ちものに呼び名をつける", "	オプションの表示・設定",
	"	画面を描き直す", "	最後のメッセージを再表示する", "	コマンドを中止する",
	"	バージョン情報を表示する", "	シェルに抜ける", "	ゲームをセーブして終了する", "	ゲームを終了する"
    };
    static const char *help_en[] = {
	"\tprints help", "\tidentify object", "\tleft", "\tdown", "\tup",
	"\tright", "\tup & left", "\tup & right", "\tdown & left",
	"\tdown & right", "\trun left", "\trun down", "\trun up", "\trun right",
	"\trun up & left", "\trun up & right", "\trun down & left", "\trun down & right",
	"<dir>\tthrow something", "<dir>\tforward until find something",
	"<dir>\tzap a wand in a direction", "\tzap a wand or staff",
	"\tgo down a staircase", "\tsearch for trap/secret door",
	"\t(space) rest for a while", "\tinventory", "\tinventory single item",
	"\tquaff potion", "\tread paper", "\teat food", "\twield a weapon",
	"\twear armor", "\ttake armor off", "\tput on ring", "\tremove ring",
	"\tdrop object", "\tcall object", "\texamine/set options", "\tredraw screen",
	"\trepeat last message", "\tcancel command", "\tprint program version number",
	"\tshell escape", "\tsave game", "\tquit"
    };

    for (i = 0; i < 26; i++)
	monsters[i].m_name = (char *) jrogue_str(mon_ja[i], mon_en[i]);
    for (i = 0; i < MAXSCROLLS; i++)
	s_magic[i].mi_name = (char *) jrogue_str(scroll_ja[i], scroll_en[i]);
    for (i = 0; i < MAXPOTIONS; i++)
	p_magic[i].mi_name = (char *) jrogue_str(potion_ja[i], potion_en[i]);
    for (i = 0; i < MAXRINGS; i++)
	r_magic[i].mi_name = (char *) jrogue_str(ring_ja[i], ring_en[i]);
    for (i = 0; i < MAXSTICKS; i++)
	ws_magic[i].mi_name = (char *) jrogue_str(stick_ja[i], stick_en[i]);
    for (i = 0; i < MAXARMORS; i++)
	a_names[i] = (char *) jrogue_str(armor_ja[i], armor_en[i]);
    for (i = 0; i < MAXWEAPONS; i++)
	w_names[i] = (char *) jrogue_str(weapon_ja[i], weapon_en[i]);
    for (i = 0; i < (int)(sizeof help_ja / sizeof help_ja[0]); i++)
	helpstr[i].h_desc = (char *) jrogue_str(help_ja[i], help_en[i]);
    if (fruit[0] == '\0' || strcmp(fruit, "slime-mold") == 0 ||
	strcmp(fruit, "ネバネバかび") == 0)
	rogue_utf8_copy(fruit, sizeof fruit, JSTR("ネバネバかび", "slime-mold"));

    jrogue_apply_options_language();
    jrogue_apply_rip_language();
    jrogue_rebuild_random_names();
}
