#include "doomdef.h"
#include "p_local.h"

#define	CLIPRADIUS	23
#define SIDE_ON	0
#define	SIDE_FRONT	1
#define	SIDE_BACK	-1

fixed_t bestslidefrac;  // 800A5728
line_t  *bestslideline; // 800A572C
mobj_t  *slidemo;       // 800A5730

/*
====================
=
= PTR_SlideTraverse
=
====================
*/

boolean PTR_SlideTraverse(intercept_t* in) // 800170AC
{
    line_t* li;

    li = in->d.line;

    if(!(li->flags & ML_TWOSIDED))
    {
        if(P_PointOnLineSide(slidemo->x, slidemo->y, li))
            return true; /* don't hit the back side */

        goto isblocking;
    }

    /* set openrange, opentop, openbottom */
    P_LineOpening(li);

    if(openrange < slidemo->height)
    {
        goto isblocking;    /* doesn't fit */
    }

    if(opentop - slidemo->z < slidemo->height)
    {
        goto isblocking;    /* mobj is too high */
    }

    if(openbottom - slidemo->z > 24*FRACUNIT)
    {
        goto isblocking;    /* too big a step up */
    }

    /* this line doesn't block movement */
    return true;

    /* the line does block movement, */
    /* see if it is closer than best so far */

isblocking:

    if(in->frac < bestslidefrac)
    {
        bestslidefrac = in->frac;
        bestslideline = li;
    }

    return false;       /* stop */
}

/*
===============================================
= P_SlideMove
= The momx / momy move is bad, so try to slide
= along a wall.
= Find the first line hit, move flush to it,
= and slide along it
=
= This is a kludgy mess.
===============================================
*/

void P_SlideMove(mobj_t* mo) // 800171B0
{
    fixed_t tmxmove;
    fixed_t tmymove;
    fixed_t leadx;
    fixed_t leady;
    fixed_t trailx;
    fixed_t traily;
    fixed_t newx;
    fixed_t newy;
    int     hitcount;
    line_t* ld;
    int     an1;
    int     an2;

    slidemo = mo;
    hitcount = 0;

retry:
    hitcount++;

    if(hitcount == 3)
    {
        goto stairstep;    // don't loop forever
    }

    // trace along the three leading corners
    if(mo->momx > 0)
    {
        leadx = mo->x + mo->radius;
        trailx = mo->x - mo->radius;
    }
    else
    {
        leadx = mo->x - mo->radius;
        trailx = mo->x + mo->radius;
    }

    if(mo->momy > 0)
    {
        leady = mo->y + mo->radius;
        traily = mo->y - mo->radius;
    }
    else
    {
        leady = mo->y - mo->radius;
        traily = mo->y + mo->radius;
    }

    bestslidefrac = FRACUNIT+1;

    P_PathTraverse(leadx, leady, leadx + mo->momx, leady + mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(trailx, leady, trailx + mo->momx, leady + mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(leadx, traily, leadx + mo->momx, traily + mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);

    // move up to the wall
    if(bestslidefrac == FRACUNIT+1)
    {
        // the move most have hit the middle, so stairstep
stairstep:
        if(!P_TryMove(mo, mo->x, mo->y + mo->momy))
        {
            if(!P_TryMove(mo, mo->x + mo->momx, mo->y))
            {
                // [d64] set momx and momy to 0
                mo->momx = 0;
                mo->momy = 0;
            }
        }
        return;
    }

    // fudge a bit to make sure it doesn't hit
    bestslidefrac -= 0x800;
    if(bestslidefrac > 0)
    {
        newx = FixedMul(mo->momx, bestslidefrac);
        newy = FixedMul(mo->momy, bestslidefrac);

        if(!P_TryMove(mo, mo->x + newx, mo->y + newy))
        {
            bestslidefrac = FRACUNIT;

            // [d64] jump to hitslideline instead of stairstep
            goto hitslideline;
        }
    }

    // Now continue along the wall.
    // First calculate remainder.
    bestslidefrac = 0xf800 - bestslidefrac;
    //bestslidefrac = FRACUNIT - (bestslidefrac + 0x800);

    if(bestslidefrac > FRACUNIT)
    {
        bestslidefrac = FRACUNIT;
    }

    if(bestslidefrac <= 0)
    {
        return;
    }

    //
    // [d64] code below is loosely based on P_HitSlideLine
    //
hitslideline:

    ld = bestslideline;

    if(ld->slopetype == ST_HORIZONTAL)
    {
        tmymove = 0;
    }
    else
    {
        tmymove = FixedMul(mo->momy, bestslidefrac);
    }

    if(ld->slopetype == ST_VERTICAL)
    {
        tmxmove = 0;
    }
    else
    {
        tmxmove = FixedMul(mo->momx, bestslidefrac);
    }

    //
    // [d64] this new algorithm seems to reduce the chances
    // of boosting the player's speed when wall running
    //

    an1 = finecosine[ld->fineangle];
    an2 = finesine[ld->fineangle];

    if(P_PointOnLineSide(mo->x, mo->y, bestslideline))
    {
        //
        // [d64] same as deltaangle += ANG180 ?
        //
        an1 = -an1;
        an2 = -an2;
    }

    newx = FixedMul(tmxmove, an1);
    newy = FixedMul(tmymove, an2);

    mo->momx = FixedMul(newx + newy, an1);
    mo->momy = FixedMul(newx + newy, an2);

    if(!P_TryMove(mo, mo->x + mo->momx, mo->y + mo->momy))
    {
        goto retry;
    }
}

//----------------------

#if 0

fixed_t slidex, slidey;		// the final position       //80077DBC|fGp000009ac, 80077DC0|fGp000009b0
line_t *specialline;		//80077DC8, uGp000009b8

fixed_t		slidedx, slidedy;		// current move for completablefrac //80077E9C|fGp00000a8c, 80077EA0|fGp00000a90

fixed_t		endbox[4];				// final proposed position //800979d0

fixed_t blockfrac;			// the fraction of move that gets completed //8007804C|iGp00000c3c
fixed_t blocknvx, blocknvy;	// the vector of the line that blocks move //80077FD0|fGp00000bc0, 80077FD8|fGp00000bc8

// p1, p2 are line endpoints
// p3, p4 are move endpoints
/*iGp00000ae4, iGp00000aec*///p1x, p1y
/*iGp00000ae8, iGp00000af8*///p2x, p2y
/*iGp00000af4, iGp00000b0c*///p3x, p3y
/*iGp00000b08, iGp00000b14*///p4x, p4y

int p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y;
fixed_t	nvx, nvy;				// normalized line vector //fGp00000b70, fGp00000b74

extern mobj_t *slidething;//80077D04


fixed_t P_CompletableFrac(fixed_t dx, fixed_t dy);
int SL_PointOnSide(fixed_t x, fixed_t y);
fixed_t SL_CrossFrac (void);
boolean CheckLineEnds (void);
void SL_ClipToLine( void );
boolean SL_CheckLine(line_t *ld);
int	SL_PointOnSide2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, fixed_t x3, fixed_t y3);
void SL_CheckSpecialLines (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);

/*
===================
=
= P_SlideMove
=
===================
*/

void P_SlideMove(void)//L8002553C()
{
    fixed_t dx, dy, rx, ry;
    fixed_t frac, slide;
    int i;

	dx = slidething->momx;
	dy = slidething->momy;
	slidex = slidething->x;
	slidey = slidething->y;

	if (slidething->flags & MF_NOCLIP)//Psx Doom
    {
        frac = FRACUNIT;
        goto Skip_P_CompletableFrac;
    }

    // perform a maximum of three bumps
    for (i = 0; i < 3; i++)
    {
        frac = P_CompletableFrac(dx, dy);

        if (frac != FRACUNIT)
            frac -= 0x1000;

        if (frac < 0)
            frac = 0;

    Skip_P_CompletableFrac:
        rx = FixedMul(frac, dx);
        ry = FixedMul(frac, dy);

        slidex += rx;
        slidey += ry;

        // made it the entire way
        if (frac == FRACUNIT)
        {
            slidething->momx = dx;
            slidething->momy = dy;
            SL_CheckSpecialLines(slidething->x, slidething->y, slidex, slidey);
            return;
        }

        // project the remaining move along the line that blocked movement
        dx -= rx;
        dy -= ry;
        slide = FixedMul(dx, blocknvx) + FixedMul(dy, blocknvy);

        dx = FixedMul(slide, blocknvx);
        dy = FixedMul(slide, blocknvy);
    }

	// some hideous situation has happened that won't let the player slide
	slidex = slidething->x;
	slidey = slidething->y;
	slidething->momx = slidething->momy = 0;
}


/*
===================
=
= P_CompletableFrac
=
= Returns the fraction of the move that is completable
===================
*/

fixed_t P_CompletableFrac(fixed_t dx, fixed_t dy)//L800256CC()
{
	int	xl,xh,yl,yh,bx,by;
	int			offset;
    short		*list;
    line_t		*ld;

	blockfrac = FRACUNIT;		// the entire dist until shown otherwise
	slidedx = dx;
	slidedy = dy;

	endbox[BOXTOP   ] = slidey + CLIPRADIUS*FRACUNIT;
	endbox[BOXBOTTOM] = slidey - CLIPRADIUS*FRACUNIT;
	endbox[BOXRIGHT ] = slidex + CLIPRADIUS*FRACUNIT;
	endbox[BOXLEFT  ] = slidex - CLIPRADIUS*FRACUNIT;

	if (dx > 0)
		endbox[BOXRIGHT ] += dx;
	else
		endbox[BOXLEFT  ] += dx;

	if (dy > 0)
		endbox[BOXTOP   ] += dy;
	else
		endbox[BOXBOTTOM] += dy;

	++validcount;

	//
	// check lines
	//
	xl = (endbox[BOXLEFT  ] - bmaporgx) >> MAPBLOCKSHIFT;
	xh = (endbox[BOXRIGHT ] - bmaporgx) >> MAPBLOCKSHIFT;
	yl = (endbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
	yh = (endbox[BOXTOP   ] - bmaporgy) >> MAPBLOCKSHIFT;

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
		    /*inline ??*/
			//P_BlockLinesIterator(bx, by, SL_CheckLine);

            offset = by*bmapwidth+bx;

            offset = *(blockmap+offset);

            for ( list = blockmaplump+offset ; *list != -1 ; list++)
            {
                ld = &lines[*list];

                if (ld->validcount != validcount)
                {
                    ld->validcount = validcount;
                    SL_CheckLine(ld);
                }
            }
		}
	}

	//
	// examine results
	//
	if (blockfrac < 0x1000)
	{
		blockfrac = 0;
		specialline = 0;	// can't cross anything on a bad move
		return 0;			// solid wall or thing
	}

	return blockfrac;
}

//inline
int SL_PointOnSide(fixed_t x, fixed_t y)//L80025970()
{
    //checked
    fixed_t	dx, dy, dist;

    dx = x - p1x;
    dy = y - p1y;
    dist = FixedMul(dx,nvx) + FixedMul(dy,nvy);

    if(dist > FRACUNIT)
        return SIDE_FRONT;
    else if(dist < -FRACUNIT)
        return SIDE_BACK;
    else
        return SIDE_ON;
}

//inline
fixed_t SL_CrossFrac (void)//L800259E0()
{
    //checked
    fixed_t	dx, dy, dist1, dist2, frac;

    // project move start and end points onto line normal
    dx = p3x - p1x;
    dy = p3y - p1y;

    dist1 = FixedMul(dx,nvx) + FixedMul(dy,nvy);

    dx = p4x - p1x;
    dy = p4y - p1y;
    dist2 = FixedMul(dx,nvx) + FixedMul(dy,nvy);

    if ((dist1 < 0) == (dist2 < 0))
		return FRACUNIT;		// doesn't cross

	frac = FixedDiv(dist1, dist1 - dist2 );

	return frac;
}


boolean CheckLineEnds (void)//L80025A98()
{
    //checked
    fixed_t		snx, sny;		// sight normals
    fixed_t		dist1, dist2;
    fixed_t		dx, dy;

    snx = p4y - p3y;
    sny = p3x - p4x;

    dx = p1x - p3x;
    dy = p1y - p3y;

    dist1 = FixedMul(dx,snx) + FixedMul(dy,sny);

    dx = p2x - p3x;
    dy = p2y - p3y;

    dist2 = FixedMul(dx,snx) + FixedMul(dy,sny);

    return ((dist1 < 0)^(dist2 < 0));

    /*
    if ( (dist1<0) == (dist2<0) )
		return false;

	return true;
    */
}


/*
====================
=
= SL_ClipToLine
=
= Call with p1 and p2 set to the endpoints
= and nvx, nvy set to normalized vector
= Assumes the start point is definately on the front side of the line
= returns the fraction of the current move that crosses the line segment
====================
*/

void SL_ClipToLine( void )//L80025B58()
{
    fixed_t frac;
    int     side2, side3;

    // adjust start so it will be the first point contacted on the player circle
    // p3, p4 are move endpoints

    p3x = slidex - CLIPRADIUS * nvx;
    p3y = slidey - CLIPRADIUS * nvy;
    p4x = p3x + slidedx;
    p4y = p3y + slidedy;

    // if the adjusted point is on the other side of the line, the endpoint must
    // be checked.
    side2 = SL_PointOnSide(p3x, p3y);
    if(side2 == SIDE_BACK)
        return; // ClipToPoint and slide along normal to line

    side3 = SL_PointOnSide(p4x, p4y);
    if(side3 == SIDE_ON)
        return; // the move goes flush with the wall
    else if(side3 == SIDE_FRONT)
        return; // move doesn't cross line

    if(side2 == SIDE_ON)
    {
        frac = 0; // moves toward the line
        goto blockmove;
    }

    // the line endpoints must be on opposite sides of the move trace

    // find the fractional intercept
    frac = SL_CrossFrac();

    if(frac < blockfrac)
    {
        blockmove:
        blockfrac =  frac;
        blocknvx  = -nvy;
        blocknvy  =  nvx;
    }
}

/*
==================
=
= SL_CheckLine
=
==================
*/


boolean SL_CheckLine(line_t *ld)//L80025D50()
{
	fixed_t		opentop, openbottom;
	sector_t	*front, *back;
	int			side1, temp;

	// check bbox first
	if (endbox[BOXRIGHT ] < ld->bbox[BOXLEFT  ]
	||	endbox[BOXLEFT  ] > ld->bbox[BOXRIGHT ]
	||	endbox[BOXTOP   ] < ld->bbox[BOXBOTTOM]
	||	endbox[BOXBOTTOM] > ld->bbox[BOXTOP   ] )
		return true;

	// see if it can possibly block movement
	if (!ld->backsector || ld->flags & ML_BLOCKING)
		goto findfrac;		// explicitly blocking

	front = ld->frontsector;
	back = ld->backsector;

	if (front->floorheight > back->floorheight)
		openbottom = front->floorheight;
	else
		openbottom = back->floorheight;

	if (openbottom - slidething->z > 24*FRACUNIT)
		goto findfrac;		// too big of a step up

	if (front->ceilingheight < back->ceilingheight)
		opentop = front->ceilingheight;
	else
		opentop = back->ceilingheight;

	if (opentop - openbottom >= 56*FRACUNIT)
		return true;		// the line doesn't block movement

	// the line definately blocks movement
findfrac:
	// p1, p2 are line endpoints
	p1x = ld->v1->x;
	p1y = ld->v1->y;
	p2x = ld->v2->x;
	p2y = ld->v2->y;

	nvx = finesine[ld->fineangle];
	nvy = -finecosine[ld->fineangle];

	side1 = SL_PointOnSide (slidex, slidey);
	if (side1 == SIDE_ON)
		return true;
	if (side1 == SIDE_BACK)
	{
		if (!ld->backsector)
			return true;			// don't clip to backs of one sided lines
		temp = p1x;
		p1x = p2x;
		p2x = temp;
		temp = p1y;
		p1y = p2y;
		p2y = temp;
		nvx = -nvx;
		nvy = -nvy;
	}

	SL_ClipToLine();
	return true;
}

int	SL_PointOnSide2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, fixed_t x3, fixed_t y3)//L80025f9C()
{
    //checked
	fixed_t	nx, ny;
	fixed_t dist;

	x1 = (x1 - x2);
	y1 = (y1 - y2);

	nx = (y3 - y2);
	ny = (x2 - x3);

	dist = FixedMul(x1, nx) + FixedMul(y1, ny);

	if (dist < 0)
		return SIDE_BACK;
	return SIDE_FRONT;
}

static short    *list_;     //80078074|psGp00000c64
static line_t   *ld_;       //80077E48|plGp00000a38
static int      offset_;    //80077DB0|iGp000009a0

void SL_CheckSpecialLines (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)//L8002600C()
{
	fixed_t			bx, by, xl, xh, yl, yh, bxl, bxh, byl, byh;
	fixed_t			x3,y3,x4,y4;
	int			    side1, side2;

	if (x1<x2) {
		xl = x1;
		xh = x2;
	} else {
		xl = x2;
		xh = x1;
	}
	if (y1<y2) {
		yl = y1;
		yh = y2;
	} else {
		yl = y2;
		yh = y1;
	}

	bxl = (xl - bmaporgx)>>MAPBLOCKSHIFT;
	bxh = (xh - bmaporgx)>>MAPBLOCKSHIFT;
	byl = (yl - bmaporgy)>>MAPBLOCKSHIFT;
	byh = (yh - bmaporgy)>>MAPBLOCKSHIFT;

	if (bxl<0)
		bxl = 0;
	if (byl<0)
		byl = 0;
	if (bxh>= bmapwidth)
		bxh = bmapwidth -1;
	if (byh>= bmapheight)
		byh = bmapheight -1;

	specialline = 0;
	++validcount;

	for (bx = bxl; bx <= bxh; bx++)
	{
		for (by = byl; by <= byh; by++)
		{
			offset_ = (by*bmapwidth) + bx;
			offset_ = *(blockmap + offset_);

			for (list_ = blockmaplump + offset_; *list_ != -1; list_++)
			{
				ld_ = &lines[*list_];

				if (!ld_->special)
					continue;
				if (ld_->validcount == validcount)
					continue;		// line has already been checked

				ld_->validcount = validcount;

				if (xh < ld_->bbox[BOXLEFT]
					|| xl > ld_->bbox[BOXRIGHT]
					|| yh < ld_->bbox[BOXBOTTOM]
					|| yl > ld_->bbox[BOXTOP])
					continue;

				x3 = ld_->v1->x;
				y3 = ld_->v1->y;
				x4 = ld_->v2->x;
				y4 = ld_->v2->y;

				side1 = SL_PointOnSide2(x1, y1, x3, y3, x4, y4);
				side2 = SL_PointOnSide2(x2, y2, x3, y3, x4, y4);

				if (side1 == side2)
					continue;		// move doesn't cross line

				side1 = SL_PointOnSide2(x3, y3, x1, y1, x2, y2);
				side2 = SL_PointOnSide2(x4, y4, x1, y1, x2, y2);

				if (side1 == side2)
					continue;		// line doesn't cross move

				specialline = ld_;
				return;
			}
		}
	}
}
#endif // 0
