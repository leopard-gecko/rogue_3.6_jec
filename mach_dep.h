/*
 * machine dependicies
 *
 * %G% (Berkeley) %W%
 */
/*
 * where scorefile and lockfile should live
 */
#ifndef ROGUE_SCOREFILE
#define ROGUE_SCOREFILE "rogue36.scr"
#endif
#ifndef ROGUE_SCOREDIR
#define ROGUE_SCOREDIR "."
#endif
#ifndef ROGUE_LOCKFILE
#define ROGUE_LOCKFILE "rogue36.lock"
#endif
#ifndef ROGUE_LOCKDIR
#define ROGUE_LOCKDIR "."
#endif
#define SCOREFILE ROGUE_SCOREFILE
#define SCOREDIR ROGUE_SCOREDIR
#define LOCKFILE ROGUE_LOCKFILE
#define LOCKDIR ROGUE_LOCKDIR
#define	NUMTOP		20

/*
 * Variables for checking to make sure the system isn't too loaded
 * for people to play
 */

#undef	MAXUSERS	/* max number of users for this game */
#undef	MAXLOAD		/* 10 * max 15 minute load average */

#if MAXUSERS|MAXLOAD
#define	CHECKTIME	15	/* number of minutes between load checks */
				/* if not defined checks are only on startup */
#endif

#ifdef MAXLOAD
#define	LOADAV			/* defined if rogue should provide loadav() */

#ifdef LOADAV
#define	NAMELIST	"/vmunix"	/* where the system namelist lives */
#endif
#endif

#ifdef MAXUSERS
#define	UCOUNT			/* defined if rogue should provide ucount() */

#ifdef UCOUNT
#define UTMP	"/etc/utmp"	/* where utmp file lives */
#endif
#endif
