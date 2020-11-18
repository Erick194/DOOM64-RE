#include "doomdef.h"
#include "p_local.h"

/*================== */
/* */
/* out */
/* */
/*================== */

extern	mobj_t  *tmthing;		// 800A56B0
extern	fixed_t  tmx, tmy;		// 800A56B4, 800A56B8
extern	boolean  checkposonly;	// 800A56C8

/*================== */
/* */
/* in */
/* */
/*================== */

boolean trymove2;		// 800A5D80	/* Result from P_TryMove2 */
boolean floatok;		// 800A5D84	/* if true, move would be ok if within tmfloorz - tmceilingz */
fixed_t tmfloorz;		// 800A5D88	/* Current floor z for P_TryMove2 */
fixed_t tmceilingz;		// 800A5D8C	/* Current ceiling z for P_TryMove2 */
mobj_t *movething;		// 800A5D98  /* Either a skull/missile target or a special pickup */
line_t *blockline;		// 800A5D9C	/* Might be a door that can be opened */

fixed_t		oldx, oldy;	// 800A5DA0, 800A5DA4
fixed_t		tmbbox[4];  //
int			tmflags;	// 800A5DB8
fixed_t		tmdropoffz;	// 800A5D90 /* Lowest point contacted */
subsector_t	*newsubsec;	// 800A5D94 /* Dest subsector */

//PSX NEW
line_t *thingspec[8];		// 800A5DE0
int		numthingspec;		// 800A5DE0

/*
===================
=
= P_TryMove2
=
= Attempt to move to a new position, crossing special lines unless MF_TELEPORT
= is set
=
===================
*/

void P_TryMove2(void) // 80019980
{
	int		side;
	int		oldside;
	line_t	*line;

	trymove2 = false;		// until proven otherwise
	floatok = false;

	oldx = tmthing->x;
	oldy = tmthing->y;

	PM_CheckPosition();

	if (checkposonly)
    {
		checkposonly = false;
		return;
	}

	if (!trymove2)
		return;

	if (!(tmthing->flags & MF_NOCLIP))
    {
		trymove2 = false;

		if (tmceilingz - tmfloorz < tmthing->height)
			return;			// doesn't fit
		floatok = true;
		if ( !(tmthing->flags&MF_TELEPORT) && tmceilingz - tmthing->z < tmthing->height)
			return;			// mobj must lower itself to fit
		if ( !(tmthing->flags&MF_TELEPORT) && tmfloorz - tmthing->z > 24*FRACUNIT )
			return;			// too big a step up
		if ( !(tmthing->flags&(MF_DROPOFF|MF_FLOAT)) && tmfloorz - tmdropoffz > 24*FRACUNIT )
			return;			// don't stand over a dropoff
	}

	//
	// the move is ok, so link the thing into its new position
	//
	P_UnsetThingPosition(tmthing);

	tmthing->floorz = tmfloorz;
	tmthing->ceilingz = tmceilingz;
	tmthing->x = tmx;
	tmthing->y = tmy;

	P_SetThingPosition(tmthing);

	if (!(tmthing->flags & (MF_NOCLIP | MF_TELEPORT)))
	{
		while (numthingspec > 0)
		{
			numthingspec--;
			line = thingspec[numthingspec];

			side = P_PointOnLineSide(tmthing->x, tmthing->y, line);
			oldside = P_PointOnLineSide(oldx, oldy, line);

			if (side != oldside)
            {
                if (!(line->flags & ML_TRIGGERFRONT) || (side))
                {
                    P_UseSpecialLine(line, tmthing);
                }
            }
		}
	}

	trymove2 = true;

	return;
}

/*
==================
=
= P_PointOnLineSide
=
= Returns 0 or 1
==================
*/

int P_PointOnLineSide (fixed_t x, fixed_t y, line_t *line) // 80019C24
{
	fixed_t	dx,dy;
	fixed_t	left, right;

	if (!line->dx)
	{
		if (x <= line->v1->x)
			return line->dy > 0;
		return line->dy < 0;
	}
	if (!line->dy)
	{
		if (y <= line->v1->y)
			return line->dx < 0;
		return line->dx > 0;
	}

	dx = (x - line->v1->x);
	dy = (y - line->v1->y);

	left = (line->dy>>16) * (dx>>16);
	right = (dy>>16) * (line->dx>>16);

	if (right < left)
		return 0;		/* front side */
	return 1;			/* back side */
}

#if 0
static boolean PM_CrossCheck(line_t *ld)
{
	if (PM_BoxCrossLine (ld))	{
		if (!PIT_CheckLine(ld)) {
			return true;
		}
	}
	return false;
}

/*
==================
=
= PM_PointOnLineSide
= Exclusive Psx Doom
=
= Returns 0 or 1
=
==================
*/

int PM_PointOnLineSide(fixed_t x, fixed_t y, line_t *line)//L8001EB8C()
{
	fixed_t dx, dy;
	fixed_t left, right;

	dx = (x - line->v1->x);
	dy = (y - line->v1->y);

	left  = (line->dy >> 16) * (dx >> 16);
	right = (dy >> 16) *(line->dx >> 16);

	if (right < left)
		return 0; /* front side */
	return 1;    /* back side */
}
#endif

/*
===============================================================================

						THING POSITION SETTING

===============================================================================
*/

/*
===================
=
= P_UnsetThingPosition
=
= Unlinks a thing from block map and sectors
=
===================
*/

void P_UnsetThingPosition (mobj_t *thing)//L8001C768()
{
	int blockx, blocky;

	if (!(thing->flags & MF_NOSECTOR))
	{	/* inert things don't need to be in blockmap */
		/* unlink from subsector */
		if (thing->snext)
			thing->snext->sprev = thing->sprev;
		if (thing->sprev)
			thing->sprev->snext = thing->snext;
		else
			thing->subsector->sector->thinglist = thing->snext;
	}

	if (!(thing->flags & MF_NOBLOCKMAP))
	{	/* inert things don't need to be in blockmap */
		/* unlink from block map */
		if (thing->bnext)
			thing->bnext->bprev = thing->bprev;
		if (thing->bprev)
			thing->bprev->bnext = thing->bnext;
		else
		{
			blockx = (thing->x - bmaporgx)>>MAPBLOCKSHIFT;
			blocky = (thing->y - bmaporgy)>>MAPBLOCKSHIFT;

			// Prevent buffer overflow if the map object is out of bounds.
            // This is part of the fix for the famous 'linedef deletion' bug.
            // From PsyDoom (StationDoom) by BodbDearg
			#if FIX_LINEDEFS_DELETION == 1
            if (blockx>=0 && blockx <bmapwidth
             && blocky>=0 && blocky <bmapheight)
            {
                blocklinks[blocky*bmapwidth+blockx] = thing->bnext;
            }
            #else
                blocklinks[blocky*bmapwidth+blockx] = thing->bnext;
			#endif
		}
	}
}


/*
===================
=
= P_SetThingPosition
=
= Links a thing into both a block and a subsector based on it's x y
= Sets thing->subsector properly
=
===================
*/

void P_SetThingPosition (mobj_t *thing) // 80019E20
{
	subsector_t  *ss;
    sector_t     *sec;
    int           blockx, blocky;
    mobj_t      **link;

    /* */
    /* link into subsector */
    /* */
    ss = R_PointInSubsector (thing->x,thing->y);
    thing->subsector = ss;
    if(!(thing->flags & MF_NOSECTOR))
    {
        /* invisible things don't go into the sector links */
        sec = ss->sector;

        thing->sprev = NULL;
        thing->snext = sec->thinglist;
        if(sec->thinglist)
         sec->thinglist->sprev = thing;
        sec->thinglist = thing;
    }

    /* */
    /* link into blockmap */
    /* */
    if(!(thing->flags & MF_NOBLOCKMAP))
    {
        /* inert things don't need to be in blockmap */
        blockx = (thing->x - bmaporgx)>>MAPBLOCKSHIFT;
        blocky = (thing->y - bmaporgy)>>MAPBLOCKSHIFT;
        if(blockx >= 0 && blockx < bmapwidth && blocky >= 0 && blocky < bmapheight)
        {
            link = &blocklinks[blocky*bmapwidth+blockx];
            thing->bprev = NULL;
            thing->bnext = *link;
            if (*link)
                (*link)->bprev = thing;
            *link = thing;
        }
        else
        {
            /* thing is off the map */
            thing->bnext = thing->bprev = NULL;
        }
    }
}

/*
==================
=
= PM_CheckPosition
=
= This is purely informative, nothing is modified (except things picked up)

in:
tmthing		a mobj_t (can be valid or invalid)
tmx,tmy		a position to be checked (doesn't need relate to the mobj_t->x,y)

out:

newsubsec
floorz
ceilingz
tmdropoffz		the lowest point contacted (monsters won't move to a dropoff)
movething

==================
*/

void PM_CheckPosition (void) // 80019F50
{
	int			xl,xh,yl,yh,bx,by;

	tmflags = tmthing->flags;

	tmbbox[BOXTOP] = tmy + tmthing->radius;
	tmbbox[BOXBOTTOM] = tmy - tmthing->radius;
	tmbbox[BOXRIGHT] = tmx + tmthing->radius;
	tmbbox[BOXLEFT] = tmx - tmthing->radius;

	newsubsec = R_PointInSubsector(tmx,tmy);

	//
	// the base floor / ceiling is from the subsector that contains the
	// point.  Any contacted lines the step closer together will adjust them
	//
	tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = newsubsec->sector->ceilingheight;

	++validcount;

	numthingspec = 0;//PSX
	movething = NULL;
	blockline = NULL;

	if (tmflags & MF_NOCLIP)
	{
		trymove2 = true;
		return;
	}

	//
	// check things first, possibly picking things up
	// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
	// into mapblocks based on their origin point, and can overlap into adjacent
	// blocks by up to MAXRADIUS units
	//
	// [D64] no use MAXRADIUS
	//
	xl = (tmbbox[BOXLEFT] - bmaporgx/* - MAXRADIUS*/)>>MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx/* + MAXRADIUS*/)>>MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy/* - MAXRADIUS*/)>>MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy/* + MAXRADIUS*/)>>MAPBLOCKSHIFT;

	if (xl<0)
		xl = 0;
	if (yl<0)
		yl = 0;
	if (xh>= bmapwidth)
		xh = bmapwidth -1;
	if (yh>= bmapheight)
		yh = bmapheight -1;

	for (bx = xl; bx <= xh; bx++)
	{
		for (by = yl; by <= yh; by++)
		{
			if (!PM_BlockThingsIterator(bx, by))
			{
				trymove2 = false;
				return;
			}
		}
	}

	//
	// check lines
	//
	xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

	if (xl<0)
		xl = 0;
	if (yl<0)
		yl = 0;
	if (xh>= bmapwidth)
		xh = bmapwidth -1;
	if (yh>= bmapheight)
		yh = bmapheight -1;

	for (bx = xl; bx <= xh; bx++)
	{
		for (by = yl; by <= yh; by++)
		{
			if (!PM_BlockLinesIterator(bx, by))
			{
				trymove2 = false;
				return;
			}
		}
	}

	trymove2 = true;
	return;
}

//=============================================================================


/*
=================
=
= PM_BoxCrossLine
=
=================
*/
boolean PM_BoxCrossLine (line_t *ld) // 8001A280
{
	boolean		side1, side2;

	if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
	||	tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
	||	tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
	||	tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP] )
		return false;

    switch(ld->slopetype)
    {
    case ST_HORIZONTAL:
        side1 = (ld->bbox[BOXTOP] < tmbbox[BOXTOP]);
        side2 = (ld->bbox[BOXTOP] < tmbbox[BOXBOTTOM]);
        break;

    case ST_VERTICAL:
        side1 = (ld->bbox[BOXLEFT] < tmbbox[BOXRIGHT]);
        side2 = (ld->bbox[BOXLEFT] < tmbbox[BOXLEFT]);
        break;

    case ST_POSITIVE:
        side1 = P_PointOnLineSide(tmbbox[BOXLEFT], tmbbox[BOXTOP], ld);
        side2 = P_PointOnLineSide(tmbbox[BOXRIGHT], tmbbox[BOXBOTTOM], ld);
        break;

    case ST_NEGATIVE:
        side1 = P_PointOnLineSide(tmbbox[BOXRIGHT], tmbbox[BOXTOP], ld);
        side2 = P_PointOnLineSide(tmbbox[BOXLEFT], tmbbox[BOXBOTTOM], ld);
        break;

    default:
        break;
    }

    return (0 < (side1 ^ side2));
}

//=============================================================================


/*
==================
=
= PIT_CheckLine
=
= Adjusts tmfloorz and tmceilingz as lines are contacted
==================
*/

boolean PIT_CheckLine (line_t *ld) // 8001A3DC
{
	fixed_t		pm_opentop, pm_openbottom;
	fixed_t		pm_lowfloor;
	sector_t	*front, *back;

	// a line has been hit

	/*
	=
	= The moving thing's destination position will cross the given line.
	= If this should not be allowed, return false.
	*/
	if (!ld->backsector)
		return false;		// one sided line

	if (!(tmthing->flags & MF_MISSILE) )
	{
		if ( ld->flags & ML_BLOCKING )
			return false;		// explicitly blocking everything
		if ( !tmthing->player && ld->flags & ML_BLOCKMONSTERS )
			return false;		// block monsters only
	}

	front = ld->frontsector;
	back = ld->backsector;

	if (front->ceilingheight == front->floorheight
	|| back->ceilingheight == back->floorheight)
	{
		blockline = ld;
		return false;			// probably a closed door
	}

	if (front->ceilingheight < back->ceilingheight)
		pm_opentop = front->ceilingheight;
	else
		pm_opentop = back->ceilingheight;

	if (front->floorheight > back->floorheight)
	{
		pm_openbottom = front->floorheight;
		pm_lowfloor = back->floorheight;
	}
	else
	{
		pm_openbottom = back->floorheight;
		pm_lowfloor = front->floorheight;
	}

	// adjust floor / ceiling heights
	if (pm_opentop < tmceilingz)
		tmceilingz = pm_opentop;
	if (pm_openbottom > tmfloorz)
		tmfloorz = pm_openbottom;
	if (pm_lowfloor < tmdropoffz)
		tmdropoffz = pm_lowfloor;

	// if contacted a special line, add it to the list
    if(ld->special & MLU_CROSS)
	{
	    //New Psx Doom
		if (numthingspec < MAXTHINGSPEC)
		{
			thingspec[numthingspec] = ld;
			numthingspec++;
		}
	}

	return true;
}

/*
==================
=
= PIT_CheckThing
=
==================
*/

boolean PIT_CheckThing(mobj_t *thing) // 8001A560
{
	fixed_t blockdist;
	fixed_t x, y;
    fixed_t rx, ry;

    if (thing == tmthing)
        return true;        // don't clip against self

	if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE) ))
		return true;

	blockdist = thing->radius + tmthing->radius;

	/*delta = thing->x - tmx;
	if (delta < 0)
		delta = -delta;
	if (delta >= blockdist)
		return true;		// didn't hit it

	delta = thing->y - tmy;
	if (delta < 0)
		delta = -delta;
	if (delta >= blockdist)
		return true;		// didn't hit it

	if (thing == tmthing)
		return true;		// don't clip against self*/

    // [d64]: different logic versus Jaguar Doom
    x = abs(thing->x - tmx);
    y = abs(thing->y - tmy);

    rx = blockdist - x;
    ry = blockdist - x;

    if(!(x < y))
    {
        if(((rx - y) + (y >> 1)) <= 0)
            return true; // didn't hit it
    }
    else
    {
        if(((ry - y) + (x >> 1)) <= 0)
            return true; // didn't hit it
    }

	//
	// check for skulls slamming into things
	//
	if (tmthing->flags & MF_SKULLFLY)
	{
		movething = thing;
		return false;		// stop moving
	}

	//
	// missiles can hit other things
	//
	if (tmthing->flags & MF_MISSILE)
	{
		// see if it went over / under
		if (tmthing->z > thing->z + thing->height)
			return true;		// overhead
		if (tmthing->z+tmthing->height < thing->z)
			return true;		// underneath
		if (tmthing->target->type == thing->type) // don't hit same species as originator
		{
			if (thing == tmthing->target)
				return true;
			if (thing->type != MT_PLAYER) // let players missile other players
				return false;	// explode, but do no damage
		}
		if (! (thing->flags & MF_SHOOTABLE) )
			return !(thing->flags & MF_SOLID);		// didn't do any damage

		// damage / explode
		movething = thing;
		return false;			// don't traverse any more
	}

	//
	// check for special pickup
	//
	if ((thing->flags&MF_SPECIAL) && (tmflags&MF_PICKUP) )
	{
		movething = thing;
		return true;
	}

	return !(thing->flags & MF_SOLID);
}

/*
===============================================================================

BLOCK MAP ITERATORS

For each line/thing in the given mapblock, call the passed function.
If the function returns false, exit with false without checking anything else.

===============================================================================
*/

/*
==================
=
= PM_BlockLinesIterator
= Exclusive Psx Doom / Doom 64
=
= The validcount flags are used to avoid checking lines
= that are marked in multiple mapblocks, so increment validcount before
= the first call to PM_BlockLinesIterator, then make one or more calls to it
=
==================
*/

boolean PM_BlockLinesIterator(int x, int y) // 8001A710
{
	int     offset;
	short  *list;
	line_t *ld;

	offset = (y*bmapwidth)+x;
	offset = *(blockmap + offset);

	for (list = blockmaplump + offset; *list != -1; list++)
	{
		ld = &lines[*list];
		if (ld->validcount == validcount)
			continue; /* line has already been checked */
		ld->validcount = validcount;

		if (PM_BoxCrossLine(ld))
		{
			if (!PIT_CheckLine(ld))
				return false;
		}
	}

	return true; /* everything was checked */
}

/*
==================
=
= PM_BlockThingsIterator
= Exclusive Psx Doom / Doom 64
=
==================
*/

boolean PM_BlockThingsIterator(int x, int y) // 8001A810
{
	mobj_t *mobj;

	for (mobj = blocklinks[y * bmapwidth + x]; mobj; mobj = mobj->bnext)
	{
		if (!PIT_CheckThing(mobj))
			return false;
	}

	return true;
}
