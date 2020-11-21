/* r_main.c */

#include "doomdef.h"
#include "r_local.h"

/*===================================== */

/* */
/* subsectors */
/* */
//subsector_t		*vissubsectors[MAXVISSSEC], **lastvissubsector;

/* */
/* walls */
/* */
//viswall_t	viswalls[MAXWALLCMDS], *lastwallcmd;

/* */
/* planes */
/* */
//visplane_t	visplanes[MAXVISPLANES], *lastvisplane;

/* */
/* sprites */
/* */
//vissprite_t	vissprites[MAXVISSPRITES], *lastsprite_p, *vissprite_p;

/* */
/* openings / misc refresh memory */
/* */
//unsigned short	openings[MAXOPENINGS], *lastopening;


/*===================================== */

fixed_t		viewx, viewy, viewz;    // 800A6890, 800A6894, 800A6898
angle_t		viewangle;              // 800A689C
fixed_t		viewcos, viewsin;       // 800A68A0,
player_t	*viewplayer;            // 800A688C, 800a68a4

int			validcount;		/* increment every time a check is made */ // 800A6900
//int			framecount;		    /* incremented every frame */

/* */
/* sky mapping */
/* */
boolean     rendersky; // 800A68A8

byte        solidcols[320];                     // 800A6348
subsector_t *solidsubsectors[MAXSUBSECTORS];	// 800A6488  /* List of valid ranges to scan through */
subsector_t **endsubsector;				        // 800A6888    /* Pointer to the first free entry */
int numdrawsubsectors;                          // 800A68AC

vissprite_t	vissprites[MAXVISSPRITES];          // 800A6908
vissprite_t	*visspritehead;                     // 800A8108
int numdrawvissprites;                          // 800A68B0

int globallump;                                 // 800A68f8
int globalcm;                                   // 800A68FC

Mtx R_ProjectionMatrix;                         // 800A68B8
/*Mtx R_ProjectionMatrix =                          // 800A68B8
{
    0x00010000,	0x00000000,
    0x00000001,	0x00000000,
    0x00000000,	0xfffeffff,
    0x00000000,	0xffef0000,
    0x00000000,	0x00000000,
    0x00005555,	0x00000000,
    0x00000000,	0xfeed0000,
    0x00000000,	0xf7610000
};*/

Mtx R_ModelMatrix =                             // 8005b0C8
{
    0x00010000,	0x00000000,
    0x00000001,	0x00000000,
    0x00000000,	0x00010000,
    0x00000000,	0x00000001,
    0x00000000,	0x00000000,
    0x00000000,	0x00000000,
    0x00000000,	0x00000000,
    0x00000000,	0x00000000
};

/* */
/* precalculated math */
/* */
fixed_t*    finecosine = &finesine[FINEANGLES / 4]; // 8005B890

int         infraredFactor; // 800A810C
int         FlashEnvColor;  // 800A8110
fixed_t     quakeviewx;     // 800A8118
fixed_t     quakeviewy;     // 800A8114
mobj_t      *cameratarget;  // 800A5D70
angle_t     camviewpitch;   // 800A811C

fixed_t     scrollfrac;     // 800A812C
sector_t    *frontsector;	// 800A6340

/*============================================================================= */

/*
==============
=
= R_Init
=
==============
*/

void R_Init(void) // 800233E0
{
    R_InitData();
    guFrustum(&R_ProjectionMatrix, -8.0f, 8.0f, -6.0f, 6.0f, 8.0f, 3808.0f, 1.0f);

    /*PRINTF_D2(WHITE, 0, 10, "R_ProjectionMatrix[0][0] %x", R_ProjectionMatrix.m[0][0]);
    PRINTF_D2(WHITE, 0, 11, "R_ProjectionMatrix[0][1] %x", R_ProjectionMatrix.m[0][1]);
    PRINTF_D2(WHITE, 0, 12, "R_ProjectionMatrix[0][2] %x", R_ProjectionMatrix.m[0][2]);
    PRINTF_D2(WHITE, 0, 13, "R_ProjectionMatrix[0][3] %x", R_ProjectionMatrix.m[0][3]);
    PRINTF_D2(WHITE, 0, 14, "R_ProjectionMatrix[1][0] %x", R_ProjectionMatrix.m[1][0]);
    PRINTF_D2(WHITE, 0, 15, "R_ProjectionMatrix[1][1] %x", R_ProjectionMatrix.m[1][1]);
    PRINTF_D2(WHITE, 0, 16, "R_ProjectionMatrix[1][2] %x", R_ProjectionMatrix.m[1][2]);
    PRINTF_D2(WHITE, 0, 17, "R_ProjectionMatrix[1][3] %x", R_ProjectionMatrix.m[1][3]);
    PRINTF_D2(WHITE, 0, 18, "R_ProjectionMatrix[2][0] %x", R_ProjectionMatrix.m[2][0]);
    PRINTF_D2(WHITE, 0, 19, "R_ProjectionMatrix[2][1] %x", R_ProjectionMatrix.m[2][1]);
    PRINTF_D2(WHITE, 0, 20, "R_ProjectionMatrix[2][2] %x", R_ProjectionMatrix.m[2][2]);
    PRINTF_D2(WHITE, 0, 21, "R_ProjectionMatrix[2][3] %x", R_ProjectionMatrix.m[2][3]);
    PRINTF_D2(WHITE, 0, 22, "R_ProjectionMatrix[3][0] %x", R_ProjectionMatrix.m[3][0]);
    PRINTF_D2(WHITE, 0, 23, "R_ProjectionMatrix[3][1] %x", R_ProjectionMatrix.m[3][1]);
    PRINTF_D2(WHITE, 0, 24, "R_ProjectionMatrix[3][2] %x", R_ProjectionMatrix.m[3][2]);
    PRINTF_D2(WHITE, 0, 25, "R_ProjectionMatrix[3][3] %x", R_ProjectionMatrix.m[3][3]);
    while(1){}*/
}


/*
==============
=
= R_RenderView
=
==============
*/

void R_RenderPlayerView(void) // 80023448
{
	fixed_t pitch;
	fixed_t Fnear, FnearA, FnearB;
	fixed_t sin, cos;

	viewplayer = &players[0];

    if (cameratarget == players[0].mo)
    {
        viewz = players[0].viewz;
        pitch = players[0].recoilpitch >> ANGLETOFINESHIFT;
    }
    else
    {
        viewz = cameratarget->z;
        pitch = camviewpitch >> ANGLETOFINESHIFT;
    }

    viewx = cameratarget->x;
    viewy = cameratarget->y;
    viewz += quakeviewy;

	viewangle = cameratarget->angle + quakeviewx;
	viewcos = finecosine[viewangle >> ANGLETOFINESHIFT];
	viewsin = finesine[viewangle >> ANGLETOFINESHIFT];

	// Phase 1
	R_BSP();

	gDPSetEnvColorD64(GFX1++, FlashEnvColor);

	// Phase 2
	if (rendersky)
    {
        R_RenderSKY();
        gDPPipeSync(GFX1++);
    }

    gDPSetCycleType(GFX1++, G_CYC_2CYCLE);
    gDPSetTextureLOD(GFX1++, G_TL_TILE);
    gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
    gDPSetTexturePersp(GFX1++, G_TP_PERSP);
    gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);
    gDPSetBlendColor(GFX1++, 0, 0, 0, 0);

    gDPSetCombineMode(GFX1++, G_CC_D64COMB07, G_CC_D64COMB08);

    gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_TEX_EDGE2);

    FnearA = (1000 - FogNear);
    FnearB = ((0-FogNear) << 8) + 128000;
    Fnear  = (((128000 / FnearA) << 16) | ((FnearB / FnearA) & 0xffff));
    gMoveWd(GFX1++, G_MW_FOG, G_MWO_FOG, Fnear);

    // Apply Fog Color
    gDPSetFogColorD64(GFX1++, FogColor);

    sin = finesine[pitch];
    cos = finecosine[pitch];

    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(MTX1), G_MTX_MODELVIEW| G_MTX_LOAD | G_MTX_NOPUSH);
    MTX1->m[0][0] = 0x10000;
    MTX1->m[0][1] = 0;
    MTX1->m[0][2] = ((cos & 0xffff0000) >> 16);
    MTX1->m[0][3] = ((-sin) & 0xffff0000);
    MTX1->m[1][0] = ((sin & 0xffff0000) >> 16);
    MTX1->m[1][1] = (cos & 0xffff0000);
    MTX1->m[1][2] = 0;
    MTX1->m[1][3] = 1;
    MTX1->m[2][0] = 0;
    MTX1->m[2][1] = 0;
    MTX1->m[2][2] = (cos & 0xffff);
    MTX1->m[2][3] = (((-sin) << 16) & 0xffff0000);
    MTX1->m[3][0] = (sin & 0xffff);
    MTX1->m[3][1] = ((cos << 16) & 0xffff0000);
    MTX1->m[3][2] = 0;
    MTX1->m[3][3] = 0;
    MTX1++;

    sin = viewsin;
    cos = viewcos;

    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(MTX1), G_MTX_MODELVIEW| G_MTX_MUL | G_MTX_NOPUSH);
    MTX1->m[0][0] = (sin & 0xffff0000);
    MTX1->m[0][1] = ((-cos) & 0xffff0000);
    MTX1->m[0][2] = 1;
    MTX1->m[0][3] = 0;
    MTX1->m[1][0] = (cos & 0xffff0000);
    MTX1->m[1][1] = (sin & 0xffff0000);
    MTX1->m[1][2] = 0;
    MTX1->m[1][3] = 1;
    MTX1->m[2][0] = ((sin << 16) & 0xffff0000);
    MTX1->m[2][1] = (((-cos) << 16) & 0xffff0000);
    MTX1->m[2][2] = 0;
    MTX1->m[2][3] = 0;
    MTX1->m[3][0] = ((cos << 16) & 0xffff0000);
    MTX1->m[3][1] = ((sin << 16) & 0xffff0000);
    MTX1->m[3][2] = 0;
    MTX1->m[3][3] = 0;
    MTX1++;

    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(MTX1), G_MTX_MODELVIEW| G_MTX_MUL | G_MTX_NOPUSH);
    MTX1->m[0][0] = 0x10000;
    MTX1->m[0][1] = 0;
    MTX1->m[0][2] = 1;
    MTX1->m[0][3] = 0;
    MTX1->m[1][0] = 0;
    MTX1->m[1][1] = 0x10000;
    MTX1->m[1][2] = ((-viewx) & 0xffff0000) | (((-viewz) >> 16) & 0xffff);
    MTX1->m[1][3] = (viewy & 0xffff0000) | 1;
    MTX1->m[2][0] = 0;
    MTX1->m[2][1] = 0;
    MTX1->m[2][2] = 0;
    MTX1->m[2][3] = 0;
    MTX1->m[3][0] = 0;
    MTX1->m[3][1] = 0;
    MTX1->m[3][2] = (((-viewx) << 16) & 0xffff0000) | ((-viewz) & 0xffff);
    MTX1->m[3][3] = ((viewy << 16) & 0xffff0000);
    MTX1++;

    // Phase 3
    R_RenderAll();

    if (cameratarget == viewplayer->mo)
        R_RenderPSprites();
}

/*============================================================================= */

/*
===============================================================================
=
= R_PointOnSide
=
= Returns side 0 (front) or 1 (back)
===============================================================================
*/
int	R_PointOnSide(int x, int y, node_t *node) // 80023B6C
{
	fixed_t	dx, dy;
	fixed_t	left, right;

	if (!node->line.dx)
	{
		if (x <= node->line.x)
			return (node->line.dy > 0);
		return (node->line.dy < 0);
	}
	if (!node->line.dy)
	{
		if (y <= node->line.y)
			return (node->line.dx < 0);
		return (node->line.dx > 0);
	}

	dx = (x - node->line.x);
	dy = (y - node->line.y);

	left = (node->line.dy >> 16) * (dx >> 16);
	right = (dy >> 16) * (node->line.dx >> 16);

	if (right < left)
		return 0;		/* front side */
	return 1;			/* back side */
}

/*
==============
=
= R_PointInSubsector
=
==============
*/

struct subsector_s *R_PointInSubsector(fixed_t x, fixed_t y) // 80023C44
{
	node_t	*node;
	int		side, nodenum;

	if (!numnodes)				/* single subsector is a special case */
		return subsectors;

	nodenum = numnodes - 1;

	while (!(nodenum & NF_SUBSECTOR))
	{
		node = &nodes[nodenum];
		side = R_PointOnSide(x, y, node);
		nodenum = node->children[side];
	}

	return &subsectors[nodenum & ~NF_SUBSECTOR];
}

/*
===============================================================================
=
= R_PointToAngle
=
===============================================================================
*/

extern	angle_t	tantoangle[SLOPERANGE + 1];

int SlopeDiv(unsigned num, unsigned den) // 80023D10
{
	unsigned ans;
	if (den < 512)
		return SLOPERANGE;
	ans = (num << 3) / (den >> 8);
	return ans <= SLOPERANGE ? ans : SLOPERANGE;
}

angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2) // 80023D60
{
	int		x;
	int		y;

	x = x2 - x1;
	y = y2 - y1;

	if ((!x) && (!y))
		return 0;

	if (x >= 0)
	{	/* x >=0 */
		if (y >= 0)
		{	/* y>= 0 */
			if (x>y)
				return tantoangle[SlopeDiv(y, x)];     /* octant 0 */
			else
				return ANG90 - 1 - tantoangle[SlopeDiv(x, y)];  /* octant 1 */
		}
		else
		{	/* y<0 */
			y = -y;
			if (x>y)
				return -tantoangle[SlopeDiv(y, x)];  /* octant 8 */
			else
				return ANG270 + tantoangle[SlopeDiv(x, y)];  /* octant 7 */
		}
	}
	else
	{	/* x<0 */
		x = -x;
		if (y >= 0)
		{	/* y>= 0 */
			if (x>y)
				return ANG180 - 1 - tantoangle[SlopeDiv(y, x)]; /* octant 3 */
			else
				return ANG90 + tantoangle[SlopeDiv(x, y)];  /* octant 2 */
		}
		else
		{	/* y<0 */
			y = -y;
			if (x>y)
				return ANG180 + tantoangle[SlopeDiv(y, x)];  /* octant 4 */
			else
				return ANG270 - 1 - tantoangle[SlopeDiv(x, y)];  /* octant 5 */
		}
	}
}
