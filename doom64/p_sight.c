#include "doomdef.h"
#include "p_local.h"

fixed_t sightzstart;            //800A5F20 // eye z of looker
fixed_t topslope, bottomslope;	//800A5F24 ,800A5F28 // slopes to top and bottom of target

divline_t strace;				//800A5F30 // from t1 to t2
fixed_t t2x, t2y;				//800A5F40, 800A5F44

int t1xs,t1ys,t2xs,t2ys;        //800A5F48,800A5F4C,800A5F50,800A5F54


/*
===============
=
= P_CheckSights
=
= Check sights of all mobj thinkers that are going to change state this
= tic and have MF_COUNTKILL set
===============
*/

void P_CheckSights(void) // 8001EB00
{
	mobj_t *mobj;

	for (mobj = mobjhead.next; mobj != &mobjhead; mobj = mobj->next)
	{
		// must be killable
		if (!(mobj->flags & MF_COUNTKILL))
			continue;

		// must be about to change states
		if (mobj->tics != 1)
			continue;

		mobj->flags &= ~MF_SEETARGET;

		// must have a target
		if (!mobj->target)
			continue;

		if (P_CheckSight(mobj, mobj->target))
			mobj->flags |= MF_SEETARGET;
	}
}


/**********************************

Returns true if a straight line between t1 and t2 is unobstructed

**********************************/

boolean P_CheckSight(mobj_t *t1, mobj_t *t2) // 8001EBCC
{
	int	s1, s2;
	int	pnum, bytenum, bitnum;

	//
	// check for trivial rejection
	//
	s1 = (t1->subsector->sector - sectors);
	s2 = (t2->subsector->sector - sectors);
	pnum = s1*numsectors + s2;
	bytenum = pnum >> 3;
	bitnum = 1 << (pnum & 7);

	if (rejectmatrix[bytenum] & bitnum) {
		return false;	// can't possibly be connected
	}

	// look from eyes of t1 to any part of t2

	++validcount;

	// make sure it never lies exactly on a vertex coordinate

	strace.x = (t1->x & ~0x1ffff) | 0x10000;
	strace.y = (t1->y & ~0x1ffff) | 0x10000;
	t2x = (t2->x & ~0x1ffff) | 0x10000;
	t2y = (t2->y & ~0x1ffff) | 0x10000;
	strace.dx = t2x - strace.x;
	strace.dy = t2y - strace.y;

	t1xs = strace.x >> FRACBITS;
	t1ys = strace.y >> FRACBITS;
	t2xs = t2x >> FRACBITS;
	t2ys = t2y >> FRACBITS;

	sightzstart = (t1->z + t1->height) - (t1->height >> 2);
	topslope = (t2->z + t2->height) - sightzstart;
	bottomslope = (t2->z) - sightzstart;

	return PS_CrossBSPNode(numnodes - 1);
}

/*
=================
=
= PS_SightCrossLine
=
= First checks the endpoints of the line to make sure that they cross the
= sight trace treated as an infinite line.
=
= If so, it calculates the fractional distance along the sight trace that
= the intersection occurs at.  If 0 < intercept < 1.0, the line will block
= the sight.
=================
*/

fixed_t PS_SightCrossLine (line_t *line) // 8001EDD8
{
	int			s1, s2;
	int			p1x,p1y,p2x,p2y,p3x,p3y,p4x,p4y,dx,dy,ndx,ndy;

	// p1, p2 are line endpoints
	p1x = line->v1->x >> FRACBITS;
	p1y = line->v1->y >> FRACBITS;
	p2x = line->v2->x >> FRACBITS;
	p2y = line->v2->y >> FRACBITS;

	// p3, p4 are sight endpoints
	p3x = t1xs;
	p3y = t1ys;
	p4x = t2xs;
	p4y = t2ys;

	dx = p2x - p3x;
	dy = p2y - p3y;

	ndx = p4x - p3x;		// this can be precomputed if worthwhile
	ndy = p4y - p3y;

	s1 =  (ndy * dx) <  (dy * ndx);

	dx = p1x - p3x;
	dy = p1y - p3y;

	s2 =  (ndy * dx) <  (dy * ndx);

	if (s1 == s2)
		return -1;			// line isn't crossed

	ndx = p1y - p2y;		// vector normal to world line
	ndy = p2x - p1x;

	s1 = ndx*dx + ndy*dy;	// distance projected onto normal

	dx = p4x - p1x;
	dy = p4y - p1y;

	s2 = ndx*dx + ndy*dy;	// distance projected onto normal

	s2 = FixedDiv(s1,(s1+s2));

	return s2;
}

/*
=================
=
= PS_CrossSubsector
=
= Returns true if strace crosses the given subsector successfuly
=================
*/

boolean PS_CrossSubsector(subsector_t *sub) // 8001EF10
{
	seg_t		*seg;
	line_t		*line;
	int			count;
	sector_t	*front, *back;
	fixed_t		opentop, openbottom;
	fixed_t		frac, slope;

	//
	// check lines
	//
	count = sub->numlines;
	seg = &segs[sub->firstline];

	for ( ; count ; seg++, count--)
	{
		line = seg->linedef;

		if (line->validcount == validcount)
			continue;		// allready checked other side
		line->validcount = validcount;

		frac = PS_SightCrossLine (line);

		if (frac < 4 || frac >= (FRACUNIT+1))
			continue;

		//
		// crosses line
		//
		back = line->backsector;
		if (!back)
			return false;	// one sided line
		front = line->frontsector;

		if (front->floorheight == back->floorheight
		&& front->ceilingheight == back->ceilingheight)
			continue;		// no wall to block sight with

		if (front->ceilingheight < back->ceilingheight)
			opentop = front->ceilingheight;
		else
			opentop = back->ceilingheight;
		if (front->floorheight > back->floorheight)
			openbottom = front->floorheight;
		else
			openbottom = back->floorheight;

		if (openbottom >= opentop)	// quick test for totally closed doors
			return false;	// stop

		frac >>= 2;

		if (front->floorheight != back->floorheight)
		{
			slope =  (((openbottom - sightzstart)<<6) / frac) << 8;
			if (slope > bottomslope)
				bottomslope = slope;
		}

		if (front->ceilingheight != back->ceilingheight)
		{
			slope = (((opentop - sightzstart)<<6) / frac) << 8;
			if (slope < topslope)
				topslope = slope;
		}

		if (topslope <= bottomslope)
			return false;	// stop
	}

	return true;			// passed the subsector ok
}

/*
=================
=
= PS_CrossBSPNode
=
= Returns true if strace crosses the given node successfuly
=================
*/

boolean PS_CrossBSPNode(int bspnum) // 8001F15C
{
	node_t  *bsp;
	int     side1, side2;
	int     bsp_num;
	fixed_t dx, dy;
    fixed_t left, right;

	if (bspnum & NF_SUBSECTOR)
	{
		bsp_num = (bspnum & ~NF_SUBSECTOR);
		if (bsp_num >= numsubsectors)
		{
			I_Error("PS_CrossSubsector: ss %i with numss = %i", bsp_num, numsubsectors);
		}

		return PS_CrossSubsector(&subsectors[bsp_num]);
	}

	bsp = &nodes[bspnum];

	// decide which side the start point is on
    side1 = 1;

    dx = (strace.x - bsp->line.x);
    dy = (strace.y - bsp->line.y);

    left  = (bsp->line.dy>>FRACBITS) * (dx>>FRACBITS);
    right = (dy>>FRACBITS) * (bsp->line.dx>>FRACBITS);

    if(right < left)
        side1 = 0;    // front side

	// cross the starting side
	if (!PS_CrossBSPNode(bsp->children[side1]))
		return false;

	// the partition plane is crossed here
	side2 = 1;

    dx = (t2x - bsp->line.x);
    dy = (t2y - bsp->line.y);

    left  = (bsp->line.dy>>FRACBITS) * (dx>>FRACBITS);
    right = (dy>>FRACBITS) * (bsp->line.dx>>FRACBITS);

    if(right < left)
        side2 = 0;    // front side

	if (side1 == side2)
		return true; // the line doesn't touch the other side

	// cross the ending side
	return PS_CrossBSPNode(bsp->children[side1 ^ 1]);
}
