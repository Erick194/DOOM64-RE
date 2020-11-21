/* P_spec.h */

#include "r_local.h"

/*
===============================================================================

							P_SPEC

===============================================================================
*/

/* */
/*	Aim Camera */
/* */
typedef struct
{
	thinker_t	thinker;
	mobj_t      *viewmobj;
} aimcamera_t;

/* */
/*	move Camera */
/* */
typedef struct
{
	thinker_t	thinker;
	fixed_t     x;
	fixed_t     y;
	fixed_t     z;
	fixed_t     slopex;
	fixed_t     slopey;
	fixed_t     slopez;
	int         tic;
	int         current;
} movecamera_t;

/* */
/*	Fade Mobj in/out */
/* */
typedef struct
{
	thinker_t	thinker;
	mobj_t      *mobj;
	int         amount;
	int         destAlpha;
	int         flagReserve;
} fade_t;

/* */
/*	Fade Bright */
/* */
typedef struct
{
	thinker_t	thinker;
	int         factor;
} fadebright_t;

/* */
/*	Mobj Exp */
/* */
typedef struct
{
	thinker_t	thinker;
	int         delay;
	int         lifetime;
	int         delaydefault;
	mobj_t      *mobj;
} mobjexp_t;


/* */
/*	Quake */
/* */
typedef struct
{
	thinker_t	thinker;
	int         tics;
} quake_t;


/* */
/*	Animating textures and planes */
/* */
typedef struct
{
	int		basepic;
	int		picstart;
	int		picend;
	int		current;
	int     frame;
	int		tics;
	int     delay;
	int     delaycnt;
	boolean isreverse;
} anim_t;

/* */
/*	psx doom / doom64 exit delay */
/* */
typedef struct
{
	thinker_t  thinker;
	int tics;
	void(*finishfunc)(void);
}delay_t;

/* */
/*	source animation definition */
/* */
typedef struct
{
    int     delay;
	char	startname[9];
	int     frames;
	int		speed;
	boolean	isreverse;
	boolean	ispalcycle;
} animdef_t;

#define	MAXANIMS		15  //[d64] is 15
extern	anim_t	anims[MAXANIMS], *lastanim;

extern card_t   MapBlueKeyType;
extern card_t   MapRedKeyType;
extern card_t   MapYellowKeyType;

/* */
/*	Animating line specials */
/* */
extern	line_t	**linespeciallist;
extern	int		numlinespecials;

/* */
/*	Animating sector specials */
/* */
extern	sector_t    **sectorspeciallist;
extern	int         numsectorspecials;

/*	Define values for map objects */
#define	MO_TELEPORTMAN		14


/* at game start */
//void	P_InitPicAnims (void);

/* at map load */
void	P_SpawnSpecials (void);

/* every tic */
void 	P_UpdateSpecials (void);

/* when needed */
boolean	P_UseSpecialLine (line_t *line, mobj_t *thing);
//void	P_ShootSpecialLine ( mobj_t *thing, line_t *line);
//void P_CrossSpecialLine (line_t *line,mobj_t *thing);

void 	P_PlayerInSpecialSector (player_t *player, sector_t *sec);

int		twoSided(int sector,int line);
sector_t *getSector(int currentSector,int line,int side);
side_t	*getSide(int currentSector,int line, int side);
fixed_t	P_FindLowestFloorSurrounding(sector_t *sec);
fixed_t	P_FindHighestFloorSurrounding(sector_t *sec);
fixed_t	P_FindNextHighestFloor(sector_t *sec,int currentheight);
fixed_t	P_FindLowestCeilingSurrounding(sector_t *sec);
fixed_t	P_FindHighestCeilingSurrounding(sector_t *sec);
int		P_FindSectorFromLineTag(int tag,int start);
int		P_FindMinSurroundingLight(sector_t *sector,int max);
sector_t *getNextSector(line_t *line,sector_t *sec);

int P_FindLightFromLightTag(int tag,int start);
boolean P_ActivateLineByTag(int tag,mobj_t *thing);

/* */
/*	SPECIAL */
/* */
int EV_DoDonut(line_t *line);

/*
===============================================================================

							P_LIGHTS

===============================================================================
*/

typedef struct
{
	thinker_t	thinker;
	sector_t    *sector;
	int		    count;
	int		    special;
} fireflicker_t;

typedef struct
{
	thinker_t	thinker;
	sector_t	*sector;
	int			count;
	int		    special;
} lightflash_t;

typedef struct
{
	thinker_t	thinker;
	sector_t	*sector;
	int			count;
	int			maxlight;
	int			darktime;
	int			brighttime;
	int		    special;
} strobe_t;

typedef enum
{
	PULSENORMAL,
	PULSESLOW,
	PULSERANDOM
} glowtype_e;

/*#define GLOWSPEED           2
#define STROBEBRIGHT        1
#define SUPERFAST           10
#define FASTDARK            15
#define SLOWDARK            30*/

typedef struct
{
	thinker_t	thinker;
	sector_t	*sector;
	glowtype_e	type;
	int			count;
	int			direction;
	int			minlight;
	int			maxlight;
	int			special;
} glow_t;

typedef struct
{
	thinker_t	thinker;
	sector_t	*sector;
	sector_t	*headsector;
	int			count;
	int			start;
	int			index;
	int		    special;
} sequenceglow_t;

#define GLOWSPEED       2
#define	STROBEBRIGHT    3
#define	STROBEBRIGHT2   1
#define	TURBODARK       4
#define	FASTDARK        15
#define	SLOWDARK        30

void	T_FireFlicker(fireflicker_t *flick);
void	P_SpawnFireFlicker(sector_t *sector);
void	T_LightFlash (lightflash_t *flash);
void	P_SpawnLightFlash (sector_t *sector);
void	T_StrobeFlash (strobe_t *flash);
void 	P_SpawnStrobeFlash (sector_t *sector, int fastOrSlow);
void	P_SpawnStrobeAltFlash(sector_t *sector, int fastOrSlow);
int     EV_StartLightStrobing(line_t *line);
void	T_Glow(glow_t *g);
void	P_SpawnGlowingLight(sector_t *sector, glowtype_e type);


typedef enum
{
    mods_flags,
    mods_special,
    mods_flats,
    mods_lights,
} modifysector_t;

#define LIGHT_FLOOR     0
#define LIGHT_CEILING   1
#define LIGHT_THING     2
#define LIGHT_UPRWALL   3
#define LIGHT_LWRWALL   4

int P_ModifySectorColor(line_t* line, int index, int type);

#define SEQUENCELIGHTMAX 48

void T_SequenceGlow(sequenceglow_t *seq_g);
void P_SpawnSequenceLight(sector_t* sector, boolean first);

typedef struct
{
	thinker_t	thinker;
	sector_t	*sector;
	int         dest;
	int         src;
	int			r;
	int			g;
	int			b;
	int		    inc;
} lightmorph_t;

void P_UpdateLightThinker(int destlight, int srclight);
void T_LightMorph(lightmorph_t *lt);
int P_ChangeLightByTag(int tag1, int tag2);
int P_DoSectorLightChange(int tag1,int tag2);

typedef struct
{
	thinker_t	thinker;
	sector_t	*sector;
	sector_t    *combiner;
	int		    special;
} combine_t;
void P_CombineLightSpecials(sector_t *sector);

/*
===============================================================================

							P_SWITCH

===============================================================================
*/
typedef struct
{
	char	name1[9];
	char	name2[9];
} switchlist_t;

typedef enum
{
	top,
	middle,
	bottom
} bwhere_e;

typedef struct
{
	side_t      *side;  //old line_t		*line;
	bwhere_e	where;
	int			btexture;
	int			btimer;
	mobj_t		*soundorg;
} button_t;

#define	MAXSWITCHES	50		/* max # of wall switches in a level */
#define	MAXBUTTONS	16		/* 4 players, 4 buttons each at once, max. */
#define BUTTONTIME	30		/* 1 second */

extern	button_t	buttonlist[MAXBUTTONS];

void	P_ChangeSwitchTexture(line_t *line,int useAgain);
void 	P_InitSwitchList(void);

/*
===============================================================================

							P_PLATS

===============================================================================
*/
typedef enum
{
	down = -1,
	waiting = 0,
	up = 1,
	in_stasis = 2
} plat_e;

typedef enum
{
	perpetualRaise,
	raiseAndChange,
	raiseToNearestAndChange,
	downWaitUpStay,
	blazeDWUS,
	upWaitDownStay,
    blazeUWDS,
    customDownUp,
    customDownUpFast,
    customUpDown,
    customUpDownFast
} plattype_e;

typedef struct
{
	thinker_t	thinker;
	sector_t	*sector;
	fixed_t		speed;
	fixed_t		low;
	fixed_t		high;
	int			wait;
	int			count;
	plat_e		status;
	plat_e		oldstatus;
	boolean		crush;
	int			tag;
	plattype_e	type;
} plat_t;

#define	PLATWAIT	3			/* seconds */
#define	PLATSPEED	(FRACUNIT*2)
#define	MAXPLATS	30

extern	plat_t	*activeplats[MAXPLATS];

void	T_PlatRaise(plat_t	*plat);
int		EV_DoPlat(line_t *line,plattype_e type,int amount);
void	P_AddActivePlat(plat_t *plat);
void	P_RemoveActivePlat(plat_t *plat);
int     EV_StopPlat(line_t *line);
void	P_ActivateInStasis(int tag);

/*
===============================================================================

							P_DOORS

===============================================================================
*/
typedef enum
{
	Normal,
	Close30ThenOpen,
	DoorClose,
	DoorOpen,
	RaiseIn5Mins,
	BlazeRaise,
	BlazeOpen,
	BlazeClose
} vldoor_e;

typedef struct
{
	thinker_t	thinker;
	vldoor_e	type;
	sector_t	*sector;
	fixed_t		topheight;
	fixed_t		bottomheight;   // D64 new
	boolean     initceiling;    // D64 new
	fixed_t		speed;
	int			direction;		/* 1 = up, 0 = waiting at top, -1 = down */
	int			topwait;		/* tics to wait at the top */
								/* (keep in case a door going down is reset) */
	int			topcountdown;	/* when it reaches 0, start going down */
} vldoor_t;

#define	VDOORSPEED	FRACUNIT*2
#define	VDOORWAIT	120

void	EV_VerticalDoor (line_t *line, mobj_t *thing);
boolean P_CheckKeyLock(line_t *line, mobj_t *thing);  // Psx Doom New
int		EV_DoDoor (line_t *line, vldoor_e  type);
void	T_VerticalDoor (vldoor_t *door);
void	P_SpawnDoorCloseIn30 (sector_t *sec);
void	P_SpawnDoorRaiseIn5Mins (sector_t *sec, int secnum);

/*
===============================================================================

							P_CEILNG

===============================================================================
*/
typedef enum
{
	lowerToFloor,
	raiseToHighest,
	lowerAndCrush,
	crushAndRaise,
	fastCrushAndRaise,
	silentCrushAndRaise,
	customCeiling,
	crushAndRaiseOnce,
	customCeilingToHeight,
} ceiling_e;

typedef struct
{
	thinker_t	thinker;
	ceiling_e	type;
	sector_t	*sector;
	fixed_t		bottomheight, topheight;
	fixed_t		speed;
	boolean		crush;
	int			direction;		/* 1 = up, 0 = waiting, -1 = down */
	int			tag;			/* ID */
	int			olddirection;
	boolean     instant;
} ceiling_t;

#define	CEILSPEED		FRACUNIT*2
#define MAXCEILINGS		30

extern	ceiling_t	*activeceilings[MAXCEILINGS];

int		EV_DoCeiling (line_t *line, ceiling_e  type, fixed_t speed);
void	T_MoveCeiling (ceiling_t *ceiling);
void	P_AddActiveCeiling(ceiling_t *c);
void	P_RemoveActiveCeiling(ceiling_t *c);
int		EV_CeilingCrushStop(line_t	*line);
void	P_ActivateInStasisCeiling(line_t *line);

/*
===============================================================================

							P_FLOOR

===============================================================================
*/
typedef enum
{
	lowerFloor,			/* lower floor to highest surrounding floor */
	lowerFloorToLowest,	/* lower floor to lowest surrounding floor */
	turboLower,			/* lower floor to highest surrounding floor VERY FAST */
	raiseFloor,			/* raise floor to lowest surrounding CEILING */
	raiseFloorToNearest,	/* raise floor to next highest surrounding floor */
	raiseToTexture,		/* raise floor to shortest height texture around it */
	lowerAndChange,		/* lower floor to lowest surrounding floor and change */
						/* floorpic */
	raiseFloor24,
	raiseFloor24AndChange,
	raiseFloorCrush,
	raiseFloorTurbo,        // [d64]: unused
    customFloor,
    customFloorToHeight
} floor_e;

typedef enum
{
	build8,	// slowly build by 8
	turbo16	// quickly build by 16
} stair_e;

typedef struct
{
	thinker_t	thinker;
	floor_e		type;
	boolean		crush;
	sector_t	*sector;
	int			direction;
	int			newspecial;
	short		texture;
	fixed_t		floordestheight;
	fixed_t		speed;
	boolean     instant;
} floormove_t;

typedef struct
{
	thinker_t	thinker;
	sector_t	*sector;
	fixed_t		ceildest;
	fixed_t		flrdest;
	int         ceildir;
	int         flrdir;
} splitmove_t;

#define	FLOORSPEED	FRACUNIT*3

typedef enum
{
	ok,
	crushed,
	pastdest,
	stop
} result_e;

result_e	T_MovePlane(sector_t *sector,fixed_t speed,
			fixed_t dest,boolean crush,int floorOrCeiling,int direction);

int		EV_BuildStairs(line_t *line, stair_e type);
int		EV_DoFloor(line_t *line,floor_e floortype,fixed_t speed);
int     EV_SplitSector(line_t *line, boolean sync);// New D64
void	T_MoveFloor(floormove_t *floor);
void    T_MoveSplitPlane(splitmove_t *split);// New D64

/*
===============================================================================

							P_TELEPT

===============================================================================
*/
int		EV_Teleport( line_t *line,mobj_t *thing );
int		EV_SilentTeleport( line_t *line,mobj_t *thing );

/*
===============================================================================

                            P_MISC

===============================================================================
*/

void T_AimCamera(aimcamera_t *camera); // 8000DE60
int P_SetAimCamera(line_t *line, boolean aim); // 8000DF20
int EV_SpawnTrapMissile(line_t *line, mobj_t *target, mobjtype_t type); // 8000E02C
void P_SpawnDelayTimer(int tics, void(*action)()); // 8000E1CC
void T_CountdownTimer(delay_t *timer); // 8000E1CC
void P_ExitLevel(void); // 8000E220
void P_SecretExitLevel(int map); // 8000E25C
int P_ModifyLineFlags(line_t *line, int tag); // 8000E6BC
int P_ModifyLineData(line_t *line, int tag); // 8000E780
int P_ModifyLineTexture(line_t *line, int tag); // 8000E82C
int P_ModifySector(line_t *line, int tag, int type); // 8000E928
void T_FadeThinker(fade_t *fade); // 8000EACC
int EV_SpawnMobjTemplate(int tag); // 8000EB8C
int EV_FadeOutMobj(int tag); // 8000ED08
void T_Quake(quake_t *quake); // 8000EDE8
void P_SpawnQuake(int tics); // 8000EE7C
int P_RandomLineTrigger(line_t *line,mobj_t *thing); // 8000EEE0
void T_MoveCamera(movecamera_t *camera); // 8000F014
void P_SetMovingCamera(line_t *line); // 8000f2f8
void P_RefreshBrightness(void); // 8000f410
void P_SetLightFactor(int lightfactor); // 8000F458
void T_FadeInBrightness(fadebright_t *fb); // 8000f610
int P_ModifyMobjFlags(int tid, int flags); // 8000F674
int P_AlertTaggedMobj(int tid, mobj_t *activator); // 8000F6C4
void T_MobjExplode(mobjexp_t *exp); // 8000F76C

/*
===============================================================================

                            P_MACROS

===============================================================================
*/

typedef struct
{
    int tag;
    mobj_t *activator;
} macroactivator_t;

/* MACRO Variables */
extern macro_t     *activemacro;       // 800A6094
extern mobj_t      *macroactivator;    // 800A6098
extern line_t      macrotempline;      // 800A60A0
extern line_t      *macroline;         // 800A60EC
extern thinker_t   *macrothinker;      // 800A60F0
extern int         macrointeger;       // 800A60F4
extern macro_t     *restartmacro;      // 800A60F8
extern int         macrocounter;       // 800A60FC
extern macroactivator_t macroqueue[4]; // 800A6100
extern int         macroidx1;          // 800A6120
extern int         macroidx2;          // 800A6124

int P_StartMacro(int macroindex, line_t *line, mobj_t *thing); // 80021088
int P_SuspendMacro(void); // 80021148
void P_ToggleMacros(int tag, boolean toggleon); // 80021214
void P_RunMacros(void); // 8002126C
void P_RestartMacro(line_t *line, int id); // 80021384
