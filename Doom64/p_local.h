/* P_local.h */

#ifndef __P_LOCAL__
#define __P_LOCAL__

#ifndef __R_LOCAL__
#include "r_local.h"
#endif

#define	FLOATSPEED		(FRACUNIT*4)

#define	GRAVITY			(FRACUNIT*4)    //like JagDoom
#define	MAXMOVE			(16*FRACUNIT)


#define	MAXHEALTH			100
#define	VIEWHEIGHT			(56*FRACUNIT) //  D64 change to 41

/* mapblocks are used to check movement against lines and things */
#define MAPBLOCKUNITS	128
#define	MAPBLOCKSIZE	(MAPBLOCKUNITS*FRACUNIT)
#define	MAPBLOCKSHIFT	(FRACBITS+7)
#define	MAPBMASK		(MAPBLOCKSIZE-1)
#define	MAPBTOFRAC		(MAPBLOCKSHIFT-FRACBITS)


/* player radius for movement checking */
#define	PLAYERRADIUS	16*FRACUNIT

/* MAXRADIUS is for precalculated sector block boxes */
/* the spider demon is larger, but we don't have any moving sectors */
/* nearby */
#define	MAXRADIUS		32*FRACUNIT


#define	USERANGE		(70*FRACUNIT)
#define	MELEERANGE		(80*FRACUNIT)
#define	MISSILERANGE	(32*64*FRACUNIT)


typedef enum
{
	DI_EAST,
	DI_NORTHEAST,
	DI_NORTH,
	DI_NORTHWEST,
	DI_WEST,
	DI_SOUTHWEST,
	DI_SOUTH,
	DI_SOUTHEAST,
	DI_NODIR,
	NUMDIRS
} dirtype_t;

#define	BASETHRESHOLD	90		/* follow a player exlusively for 3 seconds */



/*
===============================================================================

							P_TICK

===============================================================================
*/

extern	thinker_t	thinkercap;	/* both the head and tail of the thinker list */


void P_InitThinkers (void);
void P_AddThinker (thinker_t *thinker);
void P_RemoveThinker (thinker_t *thinker);

/*
===============================================================================

							P_PSPR

===============================================================================
*/

void P_SetupPsprites (int curplayer); //(player_t *curplayer);
void P_MovePsprites (player_t *curplayer);

void P_SetPsprite (player_t *player, int position, statenum_t stnum);
void P_BringUpWeapon (player_t *player);
void P_DropWeapon (player_t *player);

/*
===============================================================================

							P_USER

===============================================================================
*/

void	P_PlayerThink (player_t *player);


/*
===============================================================================

							P_MOBJ

===============================================================================
*/

extern	mobj_t	mobjhead;

extern	int			activethinkers;	/* debug count */
extern	int			activemobjs;	/* debug count */

#define ONFLOORZ	MININT
#define	ONCEILINGZ	MAXINT

mobj_t *P_SpawnMobj (fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);

void 	P_RemoveMobj (mobj_t *th);
boolean	P_SetMobjState (mobj_t *mobj, statenum_t state);
void 	P_MobjThinker (mobj_t *mobj);

void	P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z);
void 	P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage);
//mobj_t *P_SpawnMissile (mobj_t *source, mobj_t *dest, mobjtype_t type);
mobj_t *P_SpawnMissile (mobj_t *source, mobj_t *dest, fixed_t xoffs, fixed_t yoffs, fixed_t heightoffs, mobjtype_t type);

void	P_SpawnPlayerMissile (mobj_t *source, mobjtype_t type);

void	P_RunMobjBase (void);//P_RunMobjBase2 (void);
void	P_RunMobjExtra (void);

void L_SkullBash (mobj_t *mo);
void L_MissileHit (mobj_t *mo);
void L_CrossSpecial (mobj_t *mo);

void P_ExplodeMissile (mobj_t *mo);

/*
===============================================================================

							P_ENEMY

===============================================================================
*/

void A_MissileExplode (mobj_t *mo);
void A_SkullBash (mobj_t *mo);

/*
===============================================================================

							P_MAPUTL

===============================================================================
*/

/*typedef struct
{
	fixed_t	x,y, dx, dy;
} divline_t;*/

typedef struct
{
	fixed_t     frac;
    boolean     isaline;
    union {
		line_t  *line;
		mobj_t  *thing;
	} d;//8
} intercept_t;

typedef boolean(*traverser_t)(intercept_t *in);


fixed_t P_AproxDistance (fixed_t dx, fixed_t dy);
int 	P_PointOnLineSide (fixed_t x, fixed_t y, line_t *line);
int 	P_PointOnDivlineSide (fixed_t x, fixed_t y, divline_t *line);
void 	P_MakeDivline (line_t *li, divline_t *dl);
fixed_t P_InterceptVector (divline_t *v2, divline_t *v1);
int 	P_BoxOnLineSide (fixed_t *tmbox, line_t *ld);
boolean P_CheckUseHeight(line_t *line);

extern	fixed_t opentop, openbottom, openrange;//,,800A5748
extern	fixed_t	lowfloor;
void 	P_LineOpening (line_t *linedef);

boolean P_BlockLinesIterator (int x, int y, boolean(*func)(line_t*) );
boolean P_BlockThingsIterator (int x, int y, boolean(*func)(mobj_t*) );

extern	divline_t 	trace;  // 800A5D58

#define PT_ADDLINES         1
#define PT_ADDTHINGS        2
#define PT_EARLYOUT         4

#define MAXINTERCEPTS       128

boolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, int flags, boolean(*trav)(intercept_t *));
boolean PIT_AddLineIntercepts(line_t* ld); // 80018574
boolean PIT_AddThingIntercepts(mobj_t* thing); // 8001860C
fixed_t P_InterceptLine(line_t *line, divline_t *trace); // 8001872C
boolean P_TraverseIntercepts(traverser_t func, fixed_t maxfrac);

/*
===============================================================================

							P_MAP

===============================================================================
*/

extern	boolean		floatok;				/* if true, move would be ok if */  //80077ea8
extern	fixed_t		tmfloorz, tmceilingz;	/* within tmfloorz - tmceilingz */  //80078010, 80077d30

extern	line_t	*specialline;//80077dc8
extern	mobj_t	*movething;


boolean P_CheckPosition (mobj_t *thing, fixed_t x, fixed_t y);
boolean P_TryMove (mobj_t *thing, fixed_t x, fixed_t y);
boolean P_CheckSight (mobj_t *t1, mobj_t *t2);
void 	P_UseLines (player_t *player);

boolean P_ChangeSector (sector_t *sector, boolean crunch);

extern	mobj_t		*linetarget;			/* who got hit (or NULL) */
fixed_t P_AimLineAttack (mobj_t *t1, angle_t angle, fixed_t zheight, fixed_t distance);

void P_LineAttack (mobj_t *t1, angle_t angle, fixed_t zheight, fixed_t distance, fixed_t slope, int damage);

void P_RadiusAttack (mobj_t *spot, mobj_t *source, int damage);

/*
===============================================================================

							P_SETUP

===============================================================================
*/

extern	byte		*rejectmatrix;			/* for fast sight rejection */
extern	short		*blockmaplump;		/* offsets in blockmap are from here */
extern	short		*blockmap;
extern	int			bmapwidth, bmapheight;	/* in mapblocks */
extern	fixed_t		bmaporgx, bmaporgy;		/* origin of block map */
extern	mobj_t		**blocklinks;			/* for thing chains */

/*
===============================================================================

							P_INTER

===============================================================================
*/

extern	int		maxammo[NUMAMMO];
extern	int		clipammo[NUMAMMO];

void P_TouchSpecialThing (mobj_t *special, mobj_t *toucher);

void P_DamageMobj (mobj_t *target, mobj_t *inflictor, mobj_t *source, int damage);

#include "p_spec.h"

/*
===============================================================================

							P_MOVE

===============================================================================
*/

//PSX NEW
#define MAXTHINGSPEC 8
extern line_t  *thingspec[8];
extern int		numthingspec;//80077ee8

extern mobj_t  *tmthing;
extern fixed_t  tmx, tmy;
extern boolean  checkposonly;

void	P_TryMove2(void);
int     PM_PointOnLineSide(fixed_t x, fixed_t y, line_t *line);
void 	P_UnsetThingPosition (mobj_t *thing);
void	P_SetThingPosition (mobj_t *thing);
void	PM_CheckPosition(void);
boolean PM_BoxCrossLine(line_t *ld);
boolean PIT_CheckLine(line_t *ld);
boolean PIT_CheckThing(mobj_t *thing);
boolean PM_BlockLinesIterator(int x, int y);
boolean PM_BlockThingsIterator(int x, int y);


/*
===============================================================================

							P_SHOOT

===============================================================================
*/

void P_Shoot2(void);
boolean PA_DoIntercept(void *value, boolean isline, int frac);
boolean	PA_ShootLine(line_t *li, fixed_t interceptfrac);
boolean PA_ShootThing(mobj_t *th, fixed_t interceptfrac);
fixed_t PA_SightCrossLine(line_t *line);
boolean PA_CrossSubsector(subsector_t *sub);
int PA_DivlineSide(fixed_t x, fixed_t y, divline_t *line);
boolean PA_CrossBSPNode(int bspnum);

/*
===============================================================================

							P_SIGHT

===============================================================================
*/

void P_CheckSights(void);
boolean P_CheckSight(mobj_t *t1, mobj_t *t2);
boolean PS_CrossBSPNode(int bspnum);
boolean PS_CrossSubsector(subsector_t *sub);
fixed_t PS_SightCrossLine (line_t *line);

#endif	/* __P_LOCAL__ */


