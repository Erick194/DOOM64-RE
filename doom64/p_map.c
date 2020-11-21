/* P_map.c */

#include "doomdef.h"
#include "p_local.h"

//int DSPRead (void volatile *adr);

/*============================================================================= */

/*================== */
/* */
/* in */
/* */
/*================== */

mobj_t  *tmthing;		// 800A56B0
fixed_t  tmx, tmy;		// 800A56B4, 800A56B8
boolean  checkposonly;	// 800A56C8

/*================== */
/* */
/* out */
/* */
/*================== */
extern	boolean		trymove2;           // 800A5D80

extern	boolean		floatok;				/* if true, move would be ok if */
											/* within tmfloorz - tmceilingz */

extern	fixed_t		tmfloorz, tmceilingz, tmdropoffz;

extern	mobj_t	*movething;             // 800A5D98

/*============================================================================= */

/*
===================
=
= P_TryMove
=
in:
tmthing		a mobj_t (can be valid or invalid)
tmx,tmy		a position to be checked (doesn't need relate to the mobj_t->x,y)

out:

newsubsec
floatok			if true, move would be ok if within tmfloorz - tmceilingz
floorz
ceilingz
tmdropoffz		the lowest point contacted (monsters won't move to a dropoff)

movething

==================
*/

void P_TryMove2 (void);

//int checkpostics;

boolean P_CheckPosition (mobj_t *thing, fixed_t x, fixed_t y) // 800166e0
{
    checkposonly = true;

	tmthing = thing;
	tmx = x;
	tmy = y;

	P_TryMove2 ();

	return trymove2;
}

boolean P_TryMove (mobj_t *thing, fixed_t x, fixed_t y) // 80016724
{
	int		damage;
	mobj_t	*latchedmovething;

	tmthing = thing;
	tmx = x;
	tmy = y;

	P_TryMove2 ();

	/* */
	/* pick up the specials */
	/* */
	latchedmovething = movething;

	if (latchedmovething)
	{
		if (thing->flags & MF_MISSILE)
		{	/* missile bash into a monster */
			damage = ((P_Random()&7)+1)*thing->info->damage;
			P_DamageMobj (latchedmovething, thing, thing->target, damage);
		}
		else if (thing->flags & MF_SKULLFLY)
		{	/* skull bash into a monster */
			damage = ((P_Random()&7)+1)*thing->info->damage;
			P_DamageMobj (latchedmovething, thing, thing, damage);
			thing->flags &= ~MF_SKULLFLY;
			thing->momx = thing->momy = thing->momz = 0;
			P_SetMobjState (thing, thing->info->spawnstate);
		}
		else	/* pick up  */
        {
			P_TouchSpecialThing (latchedmovething, thing);
        }

        // [d64]: clear movething
        movething = NULL;
	}

	return trymove2;
}


/*
==============================================================================

							USE LINES

==============================================================================
*/

int			usebbox[4]; // 800A56D0
divline_t	useline;    // 800A56E0

line_t		*closeline; // 800A56F0
fixed_t		closedist;  // 800A56F4

/*
================
=
= P_CheckUseHeight
=
================
*/

boolean P_CheckUseHeight(line_t *line) // 80016858
{
    int flags;
    fixed_t rowoffset;
    fixed_t check;

    rowoffset = sides[line->sidenum[0]].rowoffset;
    flags = line->flags & (ML_CHECKFLOORHEIGHT|ML_SWITCHX08);

    if (flags == ML_SWITCHX08)
    {
        check = (line->backsector->ceilingheight + rowoffset) + (32*FRACUNIT);
    }
    else if (flags == ML_CHECKFLOORHEIGHT)
    {
        check = (line->backsector->floorheight + rowoffset) -(32*FRACUNIT);
    }
    else if (flags == (ML_CHECKFLOORHEIGHT|ML_SWITCHX08))
    {
        check = (line->frontsector->floorheight + rowoffset) + (32*FRACUNIT);
    }
    else
        return true;

    if((check < players[0].mo->z))
        return false;

    if((players[0].mo->z + (64*FRACUNIT)) < check)
        return false;

    return true;
}

/*
===============
=
= P_InterceptVector
=
= Returns the fractional intercept point along the first divline
=
===============
*/

fixed_t P_InterceptVector (divline_t *v2, divline_t *v1) // 80016954
{
	fixed_t	frac, num, den;

	den = (v1->dy>>16)*(v2->dx>>16) - (v1->dx>>16)*(v2->dy>>16);

	if (den == 0)
		return -1;

	num = ((v1->x-v2->x)>>16) *(v1->dy>>16) + ((v2->y-v1->y)>>16) * (v1->dx>>16);

	frac = (num<<16) / den;

	return frac;
}


/*
================
=
= PIT_UseLines
=
================
*/

boolean	PIT_UseLines (line_t *li) // 80016A28
{
	divline_t	dl;
	fixed_t		frac;

	/* */
	/* check bounding box first */
	/* */
	if (usebbox[BOXRIGHT] <= li->bbox[BOXLEFT]
	||	usebbox[BOXLEFT] >= li->bbox[BOXRIGHT]
	||	usebbox[BOXTOP] <= li->bbox[BOXBOTTOM]
	||	usebbox[BOXBOTTOM] >= li->bbox[BOXTOP] )
		return true;

    if(!li->special)
    {
        // [d64]: new logic
        if(closeline != NULL)
        {
            if(closeline->special)
                return true; // keep going
        }
    }
    else
    {
        // [d64]: new logic
        if(P_PointOnLineSide(useline.x, useline.y, li) != 0)
            return true; // must be in front of the line
    }

	/* */
	/* find distance along usetrace */
	/* */
	//P_MakeDivline (li, &dl);
	dl.x = li->v1->x;
	dl.y = li->v1->y;
	dl.dx = li->dx;
	dl.dy = li->dy;

	frac = P_InterceptVector (&useline, &dl);
	if (frac < 0)
		return true;		/* behind source */

	/* */
	/* the line is actually hit, find the distance  */
	/* */
	if (!li->special)
	{
	    if (frac > closedist)
            return true;	/* too far away */

		P_LineOpening (li);
		if (openrange > 0)
			return true;	/* keep going */
	}
	else if(closeline && closeline->special != 0)
    {
        if(frac > closedist)
            return true;    /* too far away */
    }

	closeline = li;
	closedist = frac;

	return true;			/* can't use for than one special line in a row */
}


/*
================
=
= P_UseLines
=
= Looks for special lines in front of the player to activate
================
*/

void P_UseLines (player_t *player) // 80016C10
{
	int			angle;
	fixed_t		x1, y1, x2, y2;
	int			x,y, xl, xh, yl, yh;

	angle = player->mo->angle >> ANGLETOFINESHIFT;
	x1 = player->mo->x;
	y1 = player->mo->y;
	x2 = x1 + (USERANGE>>FRACBITS)*finecosine[angle];
	y2 = y1 + (USERANGE>>FRACBITS)*finesine[angle];

	useline.x = x1;
	useline.y = y1;
	useline.dx = x2-x1;
	useline.dy = y2-y1;

	if (useline.dx > 0)
	{
		usebbox[BOXRIGHT ] = x2;
		usebbox[BOXLEFT  ] = x1;
	}
	else
	{
		usebbox[BOXRIGHT ] = x1;
		usebbox[BOXLEFT  ] = x2;
	}

	if (useline.dy > 0)
	{
		usebbox[BOXTOP   ] = y2;
		usebbox[BOXBOTTOM] = y1;
	}
	else
	{
		usebbox[BOXTOP   ] = y1;
		usebbox[BOXBOTTOM] = y2;
	}

	yh = (usebbox[BOXTOP   ] - bmaporgy) >> MAPBLOCKSHIFT;
	yl = (usebbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
	xh = (usebbox[BOXRIGHT ] - bmaporgx) >> MAPBLOCKSHIFT;
	xl = (usebbox[BOXLEFT  ] - bmaporgx) >> MAPBLOCKSHIFT;

	closeline = NULL;
	closedist = FRACUNIT;
	validcount++;

	for (y = yl; y <= yh; y++)
	{
		for (x = xl; x <= xh; x++)
        {
			P_BlockLinesIterator(x, y, PIT_UseLines);
        }
	}

	/* */
	/* check closest line */
	/* */
	if (!closeline)
		return;

	if (!(closeline->special & MLU_USE) ||
        !P_CheckUseHeight(closeline))
		S_StartSound (player->mo, sfx_noway);
	else
		P_UseSpecialLine (closeline, player->mo);
}



/*
==============================================================================

							RADIUS ATTACK

==============================================================================
*/

mobj_t		*bombsource;    //pmGp0000090c
mobj_t		*bombspot;      //pmGp00000bb8
int			bombdamage;     //iGp000008b4

/*
=================
=
= PIT_RadiusAttack
=
= Source is the creature that casued the explosion at spot
=================
*/

boolean PIT_RadiusAttack (mobj_t *thing) // 80016E3C
{
	fixed_t		dx, dy, dist;

	if (!(thing->flags & MF_SHOOTABLE))
		return true;

	// Boss cyborg take no damage from concussion.
    if(thing->type == MT_CYBORG)
        return true;

    if(thing->type == MT_SKULL || thing->type == MT_PAIN)
    {
        if(bombsource && bombsource->type == MT_SKULL)
            return true;
    }

	dx = abs(thing->x - bombspot->x);
	dy = abs(thing->y - bombspot->y);

	dist = dx>dy ? dx : dy;
	dist = dist - thing->radius >> FRACBITS;

	if (dist < 0)
		dist = 0;

	if (dist >= bombdamage)
		return true;		/* out of range */

	if (P_CheckSight(thing, bombspot) != 0) // must be in direct path */
    {
		P_DamageMobj(thing, bombspot, bombsource, bombdamage - dist);
    }

	return true;
}


/*
=================
=
= P_RadiusAttack
=
= Source is the creature that casued the explosion at spot
=================
*/

void P_RadiusAttack (mobj_t *spot, mobj_t *source, int damage)// 80016F9C
{
	int			x,y, xl, xh, yl, yh;
	fixed_t		dist;

	dist = (damage+MAXRADIUS)<<FRACBITS;

	yh = (spot->y + dist - bmaporgy)>>MAPBLOCKSHIFT;
	yl = (spot->y - dist - bmaporgy)>>MAPBLOCKSHIFT;
	xh = (spot->x + dist - bmaporgx)>>MAPBLOCKSHIFT;
	xl = (spot->x - dist - bmaporgx)>>MAPBLOCKSHIFT;

	bombspot = spot;
	bombsource = source;
	bombdamage = damage;

	for (y = yl; y <= yh; y++)
	{
		for (x = xl; x <= xh; x++)
        {
			P_BlockThingsIterator(x, y, PIT_RadiusAttack);
        }
	}
}
