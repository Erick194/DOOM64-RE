/* R_local.h */

#ifndef __R_LOCAL__
#define __R_LOCAL__

/* proper screen size would be 160*100, stretched to 224 is 2.2 scale */
#define	STRETCH				(22*FRACUNIT/10)

#define	CENTERX				(SCREENWIDTH/2)
#define	CENTERY				(SCREENHEIGHT/2)
#define	CENTERXFRAC			(SCREENWIDTH/2*FRACUNIT)
#define	CENTERYFRAC			(SCREENHEIGHT/2*FRACUNIT)
#define	PROJECTION			CENTERXFRAC

#define	ANGLETOSKYSHIFT		22		/* sky map is 256*128*4 maps */

#define	BASEYCENTER			100

#define	CENTERY				(SCREENHEIGHT/2)
#define	WINDOWHEIGHT		(SCREENHEIGHT-SBARHEIGHT)

#define	MINZ				8
#define	MAXZ				256

#define	FIELDOFVIEW			2048   /* fineangles in the SCREENWIDTH wide window */

/* */
/* Seg flags */
/* */
#define SGF_VISIBLE_COLS    1       /* The seg has at least 1 visible (non fully occluded column) */


/* */
/* lighting constants */
/* */
#define	LIGHTLEVELS			256		/* number of diminishing */
#define	INVERSECOLORMAP		255

/*
==============================================================================

					INTERNAL MAP TYPES

==============================================================================
*/

/*================ used by play and refresh */

typedef struct
{
	fixed_t	x, y, dx, dy;
} divline_t;

typedef struct
{
	fixed_t x, y;
	fixed_t vx, vy;
	int     validcount;
} vertex_t;

struct line_s;
struct subsector_s;

typedef	struct
{
	fixed_t		floorheight, ceilingheight;
	VINT		floorpic, ceilingpic;	/* if ceilingpic == -1,draw sky */
	int		    colors[5];			// Doom 64 New
	int		    lightlevel;
	VINT		special, tag;

	VINT        xoffset, yoffset;   // Doom 64 New

	VINT		soundtraversed;		/* 0 = untraversed, 1,2 = sndlines -1 */
	mobj_t		*soundtarget;		/* thing that made a sound (or null) */

	VINT	    flags;	            // Psx Doom / Doom 64 New
	VINT		blockbox[4];		/* mapblock bounding box for height changes */
	degenmobj_t	soundorg;			/* for any sounds played by the sector */

	int			validcount;			/* if == validcount, already checked */
	mobj_t		*thinglist;			/* list of mobjs in sector */
	void		*specialdata;		/* thinker_t for reversable actions */
	VINT		linecount;
	struct line_s	**lines;			/* [linecount] size */
} sector_t;

typedef struct
{
	fixed_t		textureoffset;		/* add this to the calculated texture col */
	fixed_t		rowoffset;			/* add this to the calculated texture top */
	VINT		toptexture, bottomtexture, midtexture;
	sector_t	*sector;
} side_t;

typedef enum {ST_HORIZONTAL, ST_VERTICAL, ST_POSITIVE, ST_NEGATIVE} slopetype_t;

typedef struct line_s
{
	vertex_t	*v1, *v2;
	fixed_t		dx,dy;				/* v2 - v1 for side checking */
	VINT		flags;
	VINT		special, tag;
	VINT		sidenum[2];			/* sidenum[1] will be -1 if one sided */
	fixed_t		bbox[4];
	slopetype_t	slopetype;			/* to aid move clipping */
	sector_t	*frontsector, *backsector;
	int			validcount;			/* if == validcount, already checked */
	void		*specialdata;		/* thinker_t for reversable actions */
	int			fineangle;			/* to get sine / cosine for sliding */
} line_t;

typedef struct vissprite_s
{
    int         zdistance;  //*
    mobj_t      *thing;     //*4
    int         lump;       //*8
    boolean     flip;       //*12
    sector_t    *sector;    //*16
    struct		vissprite_s *next;//*20
} vissprite_t;

typedef struct subsector_s
{
	sector_t	*sector;	//*
	vissprite_t *vissprite; //*4
	short		numlines;	//*8
	short		firstline;	//*10
	short       numverts;	//*12
	short       leaf;	    //*14
    short       drawindex;	//*16
	short       padding;	//*18
} subsector_t;

typedef struct seg_s
{
	vertex_t	*v1, *v2;
	fixed_t		offset;
	angle_t		angle;				/* this is not used (keep for padding) */
	side_t		*sidedef;
	line_t		*linedef;
	sector_t	*frontsector;
	sector_t	*backsector;		/* NULL for one sided lines */
	short       flags;
	short       length;
} seg_t;

typedef struct
{
	//fixed_t		x,y,dx,dy;		//old
	divline_t	line;				/* partition line */
	fixed_t		bbox[2][4];			/* bounding box for each child */
	int			children[2];		/* if NF_SUBSECTOR its a subsector */
} node_t;

typedef struct {
	vertex_t    *vertex;
	seg_t       *seg;//*(A24 + 4)
} leaf_t;

//
// Light Data Doom64
//
typedef struct {
	int rgba;
	int tag;
} light_t;

//
// Macros Doom64
//
typedef struct
{
    int id, tag, special;
} macro_t;

/*
==============================================================================

						OTHER TYPES

==============================================================================
*/

/* Sprites are patches with a special naming convention so they can be  */
/* recognized by R_InitSprites.  The sprite and frame specified by a  */
/* thing_t is range checked at run time. */
/* a sprite is a patch_t that is assumed to represent a three dimensional */
/* object and may have multiple rotations pre drawn.  Horizontal flipping  */
/* is used to save space. Some sprites will only have one picture used */
/* for all views.   */

#ifdef MARS

int spritelump[NUMSPRITES];	/* no rotations, so just add frame num... */

#else

typedef struct
{
	boolean		rotate;		/* if false use 0 for any position */
	int			lump[8];	/* lump to use for view angles 0-7 */
	byte		flip[8];	/* flip (1 = flip) to use for view angles 0-7 */
} spriteframe_t;

typedef struct
{
	spriteframe_t	*spriteframes;
	int				numframes;
} spritedef_t;

extern	spritedef_t		sprites[NUMSPRITES];

#endif

/*
===============================================================================

							MAP DATA

===============================================================================
*/

extern	int			numvertexes;
extern	vertex_t	*vertexes;

extern	int			numsegs;
extern	seg_t		*segs;

extern	int			numsectors;
extern	sector_t	*sectors;

extern	int			numsubsectors;
extern	subsector_t	*subsectors;

extern	int			numnodes;
extern	node_t		*nodes;

extern	int			numlines;
extern	line_t		*lines;

extern	int			numsides;
extern	side_t		*sides;

extern	int			numleafs;
extern	leaf_t		*leafs;

extern light_t      *lights;
extern int          numlights;

extern macro_t      **macros;
extern int          nummacros;

//extern int          skyflatnum;

/*============================================================================= */

/*------*/
/*R_main*/
/*------*/
extern mobj_t   *cameratarget;  // 800A5D70
extern angle_t  camviewpitch;   // 800A811C
extern fixed_t  scrollfrac;     // 800A812C
extern sector_t *frontsector;   // 800A6340
extern int      globallump;     // 800A68f8
extern int      globalcm;       // 800A68fC
extern int      infraredFactor; // 800A810C
extern int      FlashEnvColor;  // 800A8110
extern fixed_t  quakeviewx;     // 800A8118
extern fixed_t  quakeviewy;     // 800A8114

/*------*/
/*R_data*/
/*------*/
void	R_InitData (void);

/*--------*/
/*r_phase1*/
/*--------*/
void	R_BSP (void);
void	R_RenderBSPNode (int bspnum);

/*--------*/
/*r_phase2*/
/*--------*/
typedef void(*skyfunc_t)();
extern fixed_t  FogNear;            // 800A8120
extern int      FogColor;           // 800A8124
skyfunc_t       R_RenderSKY;        // 800A8130
extern int      Skyfadeback;        // 800A814C

void R_SetupSky(void);

/*--------*/
/*r_phase3*/
/*--------*/
void R_RenderAll(void);
void R_RenderPSprites(void);


/* to get a global angle from cartesian coordinates, the coordinates are */
/* flipped until they are in the first octant of the coordinate system, then */
/* the y (<=x) is scaled and divided by x to get a tangent (slope) value */
/* which is looked up in the tantoangle[] table.  The +1 size is to handle */
/* the case when x==y without additional checking. */
#define	SLOPERANGE	2048
#define	SLOPEBITS	11
#define	DBITS		(FRACBITS-SLOPEBITS)

extern	angle_t	tantoangle[SLOPERANGE+1];


#define	VIEW_3D_H 200
//extern	fixed_t	yslope[VIEW_3D_H];

#define	HEIGHTBITS			6
#define	SCALEBITS			9

#define	FIXEDTOSCALE		(FRACBITS-SCALEBITS)
#define	FIXEDTOHEIGHT		(FRACBITS-HEIGHTBITS)


#define	HALF_SCREEN_W       (SCREENWIDTH/2)

extern	fixed_t		viewx, viewy, viewz;    //80077D0C, 80077D10, 80077D18
extern	angle_t		viewangle;              //800780B8
extern	fixed_t		viewcos, viewsin;       //80077EC8, 80077EE0

extern	player_t	*viewplayer;            //80077D60

extern	fixed_t		finetangent[FINEANGLES/2];

extern	int			validcount; //800779F4
//extern	int			framecount;




/* */
/* R_data.c */
/* */
extern boolean rendersky;                           // 800A68A8
extern byte solidcols[320];				            // 800A6348

#define	MAXSUBSECTORS	256		/* Maximum number of subsectors to scan */
extern subsector_t *solidsubsectors[MAXSUBSECTORS];	// 800A6488 /* List of valid ranges to scan through */
extern subsector_t **endsubsector;				    // 800A6888 /* Pointer to the first free entry */
extern int numdrawsubsectors;                       // 800A68AC

#define	MAXVISSPRITES       256
extern	vissprite_t	vissprites[MAXVISSPRITES];  // 800A6908
extern	vissprite_t	*visspritehead;             // 800A8108
extern int numdrawvissprites;                   // 800A68B0


//extern	short		skypalette;
//extern	psxobj_t	*skytexturep;

extern	int firsttex, lasttex, numtextures,firstswx;
extern	int *textures;
extern	int skytexture;

//extern	int			*flattranslation;		/* for global animation */
//extern	int			*texturetranslation;	/* for global animation */

//extern	int			firstflat, lastflat,  numflats;
//extern	psxobj_t	*texflats;

extern	int firstsprite, lastsprite, numsprites;
//extern	int *texsprites;

//#define MAX_PALETTES 26 //Final Doom 20 to 26
//extern	short palette[MAX_PALETTES];
//extern	short palettebase;
//extern	light_t		*lights;

#endif		/* __R_LOCAL__ */

