/* DoomData.h */

/* all external data is defined here */
/* most of the data is loaded into different structures at run time */

#ifndef __DOOMDATA__
#define __DOOMDATA__

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
typedef enum {false, true} boolean;
typedef unsigned char byte;
#endif

/*
===============================================================================

						map level types

===============================================================================
*/

/* lump order in a map wad */
enum {ML_LABEL, ML_THINGS, ML_LINEDEFS, ML_SIDEDEFS, ML_VERTEXES, ML_SEGS,
ML_SSECTORS, ML_NODES, ML_SECTORS , ML_REJECT, ML_BLOCKMAP, ML_LEAFS, ML_LIGHTS, ML_MACROS, ENDOFWAD
};

typedef struct
{
	int x, y; // (Psx Doom / Doom 64)
} mapvertex_t;

typedef struct
{
	short		textureoffset;
	short		rowoffset;
	short		toptexture, bottomtexture, midtexture;
	short		sector;				/* on viewer's side */
} mapsidedef_t;

typedef struct
{
	short		v1, v2;
	int		    flags;
	short		special, tag;
	short		sidenum[2];			/* sidenum[1] will be -1 if one sided */
} maplinedef_t;

#define	ML_BLOCKING			1
#define	ML_BLOCKMONSTERS	2
#define	ML_TWOSIDED			4		/* backside will not be present at all  */
									/* if not two sided ???:obsolete */

/* if a texture is pegged, the texture will have the end exposed to air held */
/* constant at the top or bottom of the texture (stairs or pulled down things) */
/* and will move with a height change of one of the neighbor sectors */
/* Unpegged textures allways have the first row of the texture at the top */
/* pixel of the line for both top and bottom textures (windows) */
#define	ML_DONTPEGTOP		8
#define	ML_DONTPEGBOTTOM	16

#define ML_SECRET			32	/* don't map as two sided: IT'S A SECRET! */
#define ML_SOUNDBLOCK		64	/* don't let sound cross two of these */
#define	ML_DONTDRAW			128	/* don't draw on the automap */
#define	ML_MAPPED			256	/* set if allready drawn in automap */

// New Doom 64 Line Flags

#define ML_DRAWMASKED           0x200           /* Draw middle texture on sidedef */
#define ML_DONTOCCLUDE          0x400           /* Don't add to occlusion buffer */
#define ML_BLOCKPROJECTILES     0x800           /* blocks projectiles */
#define ML_THINGTRIGGER         0x1000          /* Line is triggered by dead thing (flagged as ondeathtrigger) */
#define ML_SWITCHX02            0x2000          /* Switch flag 1 */
#define ML_SWITCHX04            0x4000          /* Switch flag 2 */
#define ML_SWITCHX08            0x8000          /* Switch flag 3 */
#define ML_CHECKFLOORHEIGHT     0x10000         /* if true then check the switch's floor height, else use the ceiling height */
#define ML_SCROLLRIGHT          0x20000         /* scroll texture to the right */
#define ML_SCROLLLEFT           0x40000         /* scroll texture to the left */
#define ML_SCROLLUP             0x80000         /* scroll texture up */
#define ML_SCROLLDOWN           0x100000        /* scroll texture down */
#define ML_BLENDFULLTOP         0x200000        /* do not extend blending for top texture */
#define ML_BLENDFULLBOTTOM      0x400000        /* do not extend blending for bottom texture */
#define ML_BLENDING             0x800000        /* use sector color blending (top/lower, ceiling, floor colors). */
#define ML_TRIGGERFRONT         0x1000000       /* can only trigger from the front of the line */
#define ML_HIDEAUTOMAPTRIGGER   0x2000000       /* don't display as yellow line special in automap */
#define ML_INVERSEBLEND         0x4000000       /* reverse the blending of the sector colors */
#define ML_UNKNOWN8000000       0x8000000       /* reserved */
#define ML_UNKNOWN10000000      0x10000000      /* reserved */
#define ML_UNKNOWN20000000      0x20000000      /* reserved */
#define ML_HMIRROR              0x40000000      /* horizontal mirror the texture */
#define ML_VMIRROR              0x80000000      /* vertical mirror the texture */

/*
// Psx Doom New Flags
#define ML_MIDMASKED		0x200
#define ML_MIDTRANSLUCENT	0x400
#define ML_BLOCKPRJECTILE	0x800
// Psx Final Doom New Flag
#define ML_MIDCLIPTEXTURE	0x1000
*/

/*---------------------*/
/* Special attributes. */
/*---------------------*/

#define MLU_MACRO               0x100           /* line is set to be used as a macro */
#define MLU_RED                 0x200           /* requires red key */
#define MLU_BLUE                0x400           /* requires blue key */
#define MLU_YELLOW              0x800           /* requires yellow key */
#define MLU_CROSS               0x1000          /* must cross to trigger */
#define MLU_SHOOT               0x2000          /* must shoot the line to trigger */
#define MLU_USE                 0x4000          /* must press use on the line to trigger */
#define MLU_REPEAT              0x8000          /* line can be reactivated again */

/*------------*/
/* Line masks */
/*------------*/

#define SWITCHMASK(x)           (x & 0x6000)
#define SPECIALMASK(x)          (x & 0x1FF)
#define MACROMASK(x)            (SPECIALMASK(x) - (x & MLU_MACRO))

typedef	struct
{
	short		floorheight, ceilingheight;
	short       floorpic, ceilingpic;
	short		colors[5];
	short		special, tag;
	short		flags;
} mapsector_t;

/*--------------*/
/* Sector Flags */
/*--------------*/

#define MS_REVERB               1       /* sounds are echoed in this sector */
#define MS_REVERBHEAVY          2       /* heavier echo effect */
#define MS_LIQUIDFLOOR          4       /* water effect (blitting two flats together) */
#define MS_SYNCSPECIALS         8       /* sync light special with multiple sectors */
#define MS_SCROLLFAST           16      /* faster ceiling/floor scrolling */
#define MS_SECRET               32      /* count secret when entering and display message */
#define MS_DAMAGEX5             64      /* damage player x5 */
#define MS_DAMAGEX10            128     /* damage player x10 */
#define MS_DAMAGEX20            256     /* damage player x20 */
#define MS_HIDESSECTOR          512     /* hide subsectors in automap (textured mode) */
#define MS_SCROLLCEILING        1024    /* enable ceiling scrolling */
#define MS_SCROLLFLOOR          2048    /* enable floor scrolling */
#define MS_SCROLLLEFT           4096    /* scroll flat to the left */
#define MS_SCROLLRIGHT          8192    /* scroll flat to the right */
#define MS_SCROLLUP             16384   /* scroll flat to the north */
#define MS_SCROLLDOWN           32768   /* scroll flat to the south */


typedef struct
{
	short		numsegs;
	short		firstseg;			/* segs are stored sequentially */
} mapsubsector_t;

typedef struct
{
	short		v1, v2;
	short		angle;			/* ???: make this a sidedef? */
	short		linedef, side;
	short		offset;
} mapseg_t;

enum {BOXTOP,BOXBOTTOM,BOXLEFT,BOXRIGHT};	/* bbox coordinates */

#define	NF_SUBSECTOR	0x8000
typedef struct
{
	short		x,y,dx,dy;			/* partition line */
	short		bbox[2][4];			/* bounding box for each child */
	unsigned short	children[2];		/* if NF_SUBSECTOR its a subsector */
} mapnode_t;

typedef struct
{
	short		x,y,z;
	short		angle;
	short		type;
	short		options;
	short		tid;
} mapthing_t;

#define	MTF_EASY		1
#define	MTF_NORMAL		2
#define	MTF_HARD		4
#define	MTF_AMBUSH		8

#define MTF_MULTI           16     /* Multiplayer specific */
#define MTF_SPAWN           32     /* Don't spawn until triggered in level */
#define MTF_ONTOUCH         64     /* Trigger something when picked up */
#define MTF_ONDEATH         128    /* Trigger something when killed */
#define MTF_SECRET          256    /* Count as secret for intermission when picked up */
#define MTF_NOINFIGHTING    512    /* Ignore other attackers */
#define MTF_NODEATHMATCH    1024   /* Don't spawn in deathmatch games */
#define MTF_NONETGAME       2048   /* Don't spawn in standard netgame mode */
#define MTF_NIGHTMARE       4096   /* [kex] Nightmare thing */

/*
//Psx Doom
#define MTF_BLENDMASK1	0x20
#define MTF_BLENDMASK2	0x40
#define MTF_BLENDMASK3	0x80
*/

// New Doom 64
typedef struct
{
    byte    r;
    byte    g;
    byte    b;
    byte    a;
    short   tag;
} maplights_t;

#endif			/* __DOOMDATA__ */

