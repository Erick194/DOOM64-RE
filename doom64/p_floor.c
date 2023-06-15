#include "doomdef.h"
#include "p_local.h"

/*================================================================== */
/*================================================================== */
/* */
/*								FLOORS */
/* */
/*================================================================== */
/*================================================================== */



/*================================================================== */
/* */
/*	Move a plane (floor or ceiling) and check for crushing */
/* */
/*================================================================== */
result_e	T_MovePlane(sector_t *sector,fixed_t speed,
			fixed_t dest,boolean crush,int floorOrCeiling,int direction) // 800136C0
{
	boolean	flag;
	fixed_t	lastpos;
	result_e result;

	result = ok;

	switch(floorOrCeiling)
	{
		case 0:		/* FLOOR */
			switch(direction)
			{
				case -1:	/* DOWN */
                    lastpos = sector->floorheight;
                    sector->floorheight -= speed;
                    if (sector->floorheight <= dest)
                    {
                        sector->floorheight = dest;
                        result = pastdest;
                    }

                    flag = P_ChangeSector(sector,crush);
                    if (flag != 0)
                    {
                        sector->floorheight = lastpos;
                        P_ChangeSector(sector,crush);
                        if (!crush)
                            result = stop;
                        else
                            result = crushed;
                    }
					break;

				case 1:		/* UP */
                    lastpos = sector->floorheight;
                    sector->floorheight += speed;
                    if (dest <= sector->floorheight)
                    {
                        sector->floorheight = dest;
                        result = pastdest;
                    }

                    flag = P_ChangeSector(sector,crush);
                    if (flag != 0)
                    {
                        sector->floorheight = lastpos;
                        P_ChangeSector(sector,crush);
                        if(!crush)
                            result = stop;
                        else
                            result = crushed;
                    }
					break;
			}
			break;

		case 1:		/* CEILING */
			switch(direction)
			{
				case -1:	/* DOWN */

                    lastpos = sector->ceilingheight;
                    sector->ceilingheight -= speed;
                    if (sector->ceilingheight <= dest)
                    {
                        sector->ceilingheight = dest;
                        result = pastdest;
                    }

                    flag  = P_ChangeSector(sector,crush);
                    if (flag != 0)
                    {
                        if(crush == true && result == ok)
                        {
                            result = crushed;
                        }
                        else
                        {
                            sector->ceilingheight = lastpos;
                            P_ChangeSector(sector,crush);
                            if (result == ok)
                                result = crushed;
                        }
                    }
					break;

				case 1:		/* UP */

                    lastpos = sector->ceilingheight;
                    sector->ceilingheight += speed;
                    if (dest <= sector->ceilingheight)
                    {
                        sector->ceilingheight = dest;
                        result = pastdest;
                    }

                    flag = P_ChangeSector(sector,false);
                    if ((flag != 0) && (result != 0))
                    {
                        sector->ceilingheight = lastpos;
                        P_ChangeSector(sector,false);
                    }
					break;
			}
			break;

	}

	return result;
}

/*================================================================== */
/* */
/*	MOVE A FLOOR TO IT'S DESTINATION (UP OR DOWN) */
/* */
/*================================================================== */
void T_MoveFloor(floormove_t *floor) // 80013920
{
	result_e	res;

	res = T_MovePlane(floor->sector,floor->speed,
			floor->floordestheight,floor->crush,0,floor->direction);

	if (!floor->instant)
    {
        if (!(gametic&3))
        {
            S_StartSound((mobj_t *)&floor->sector->soundorg,sfx_secmove);//sfx_stnmov
        }
    }

	if (res == pastdest)
	{
		floor->sector->specialdata = NULL;
		if (floor->direction == -1)
        {
			switch(floor->type)
			{
				case lowerAndChange:
					floor->sector->special = floor->newspecial;
					floor->sector->floorpic = floor->texture;
				default:
					break;
			}
        }
		P_RemoveThinker(&floor->thinker);
	}

}

/*================================================================== */
/* */
/*	HANDLE FLOOR TYPES */
/* */
/*================================================================== */
int EV_DoFloor(line_t *line,floor_e floortype,fixed_t speed) // 800139FC
{
	int			secnum;
	int			rtn;
	int			i;
	sector_t	*sec;
	floormove_t	*floor;

	secnum = -1;
	rtn = 0;
	while ((secnum = P_FindSectorFromLineTag(line->tag,secnum)) >= 0)
	{
		sec = &sectors[secnum];

		/*	ALREADY MOVING?  IF SO, KEEP GOING... */
		if (sec->specialdata)
			continue;

		/* */
		/*	new floor thinker */
		/* */
		rtn = 1;
		floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
		P_AddThinker (&floor->thinker);
		sec->specialdata = floor;
		floor->thinker.function = T_MoveFloor;
		floor->type = floortype;
		floor->crush = false;
		floor->sector = sec;

		if (speed == (4096 * FRACUNIT))
            floor->instant = true;
        else
            floor->instant = false;

		switch(floortype)
		{
			case lowerFloor:
				floor->direction = -1;
				floor->speed = speed;
				floor->floordestheight = P_FindHighestFloorSurrounding(sec);
				break;
			case lowerFloorToLowest:
				floor->direction = -1;
				floor->speed = speed;
				floor->floordestheight = P_FindLowestFloorSurrounding(sec);
				break;
			case turboLower:
				floor->direction = -1;
				floor->speed = speed;
				floor->floordestheight = P_FindHighestFloorSurrounding(sec);
				if (floor->floordestheight != sec->floorheight)
					floor->floordestheight += 8 * FRACUNIT;
				break;
			case raiseFloorCrush:
				floor->crush = true;
			case raiseFloor:
				floor->direction = 1;
				floor->speed = speed;
				floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
				if (floor->floordestheight > sec->ceilingheight)
					floor->floordestheight = sec->ceilingheight;

				if (floortype == raiseFloorCrush)
					floor->floordestheight -= (8 * FRACUNIT);
				break;
			case raiseFloorToNearest:
				floor->direction = 1;
				floor->speed = speed;
				floor->floordestheight = P_FindNextHighestFloor(sec,sec->floorheight);
				break;
			case raiseFloor24:
				floor->direction = 1;
				floor->speed = speed;
				floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
				break;
			case raiseFloor24AndChange:
				floor->direction = 1;
				floor->speed = speed;
				floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
				sec->floorpic = line->frontsector->floorpic;
				sec->special = line->frontsector->special;
				break;
            case customFloor:
                floor->direction = (macrointeger > 0) ? 1 : -1;
                floor->speed = speed;
                floor->floordestheight = floor->sector->floorheight + (macrointeger * FRACUNIT);
                break;
            case customFloorToHeight:
                floor->direction = ((macrointeger*FRACUNIT) > (floor->sector->floorheight)) ? 1 : -1;
                floor->speed = speed;
                floor->floordestheight = (macrointeger * FRACUNIT);
                break;
#if 0 //Missing on D64
			case raiseToTexture:
				{
					int	minsize = MAXINT;
					side_t	*side;

					floor->direction = 1;
					floor->speed = speed;
					for (i = 0; i < sec->linecount; i++)
                    {
						if (twoSided (secnum, i) )
						{
							side = getSide(secnum,i,0);
							if (side->bottomtexture >= 0)
							{
								if ((textures[side->bottomtexture].h << FRACBITS) < minsize)
									minsize = (textures[side->bottomtexture].h << FRACBITS);
							}
							side = getSide(secnum,i,1);
							if (side->bottomtexture >= 0)
							{
								if ((textures[side->bottomtexture].h << FRACBITS) < minsize)
									minsize = (textures[side->bottomtexture].h << FRACBITS);
							}
						}
                    }
					floor->floordestheight = floor->sector->floorheight + minsize;
				}
				break;
#endif // 0
			case lowerAndChange:
				floor->direction = -1;
				floor->speed = speed;
				floor->floordestheight = P_FindLowestFloorSurrounding(sec);
				floor->texture = sec->floorpic;
				for (i = 0; i < sec->linecount; i++)
                {
					if ( twoSided(secnum, i) )
					{
						if (getSide(secnum,i,0)->sector-sectors == secnum)
						{
							sec = getSector(secnum,i,1);
							floor->texture = sec->floorpic;
							floor->newspecial = sec->special;
							break;
						}
						else
						{
							sec = getSector(secnum,i,0);
							floor->texture = sec->floorpic;
							floor->newspecial = sec->special;
							break;
						}
					}
                }
			default:
				break;
		}
	}
	return rtn;
}

/*================================================================== */
/* */
/*	BUILD A STAIRCASE! */
/* */
/*================================================================== */
int EV_BuildStairs(line_t *line, stair_e type) // 80013DB0
{
	int		secnum;
	int		height;
	int		i;
	int		newsecnum;
	int		texture;
	int		ok;
	int		rtn;
	fixed_t	stairsize;
    fixed_t	speed;
	sector_t	*sec, *tsec;
	floormove_t	*floor;


	secnum = -1;
	rtn = 0;
	while ((secnum = P_FindSectorFromLineTag(line->tag,secnum)) >= 0)
	{
		sec = &sectors[secnum];

		/* ALREADY MOVING?  IF SO, KEEP GOING... */
		if (sec->specialdata)
			continue;

		/* */
		/* new floor thinker */
		/* */
		rtn = 1;
		floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
		P_AddThinker (&floor->thinker);
		sec->specialdata = floor;
		floor->thinker.function = T_MoveFloor;
		floor->direction = 1;
		floor->sector = sec;
		floor->instant = false;

		switch (type)
		{
		case build8:
			speed = FLOORSPEED / 2;
			stairsize = 8 * FRACUNIT;
			break;
		case turbo16:
			speed = FLOORSPEED * 2;
			stairsize = 16 * FRACUNIT;
			break;
		}

		floor->speed = speed;
		height = sec->floorheight + stairsize;
		floor->floordestheight = height;

		texture = sec->floorpic;

		/* */
		/* Find next sector to raise */
		/* 1.	Find 2-sided line with same sector side[0] */
		/* 2.	Other side is the next sector to raise */
		/* */
		do
		{
			ok = 0;
			for (i = 0;i < sec->linecount;i++)
			{
				if ( !((sec->lines[i])->flags & ML_TWOSIDED) )
					continue;

				tsec = (sec->lines[i])->frontsector;
				newsecnum = tsec-sectors;
				if (secnum != newsecnum)
					continue;

				tsec = (sec->lines[i])->backsector;
				newsecnum = tsec - sectors;
				if (tsec->floorpic != texture)
					continue;

				height += stairsize;
				if (tsec->specialdata)
					continue;

				sec = tsec;
				secnum = newsecnum;
				floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
				P_AddThinker (&floor->thinker);
				sec->specialdata = floor;
				floor->thinker.function = T_MoveFloor;
				floor->direction = 1;
				floor->sector = sec;
				floor->speed = speed;
				floor->floordestheight = height;
                floor->instant = false;
				ok = 1;
				break;
			}
		} while(ok);
		secnum = -1;
	}
	return rtn;
}

/*================================================================== */
/* */
/* T_MoveSplitPlane */
/* */
/*================================================================== */

void T_MoveSplitPlane(splitmove_t *split) // 80014098
{
    sector_t *sector;
    fixed_t lastceilpos;
    fixed_t lastflrpos;
    boolean cdone;
    boolean fdone;

    sector = split->sector;
    lastceilpos = sector->ceilingheight;
    lastflrpos = sector->floorheight;
    cdone = false;
    fdone = false;

    if (split->ceildir == -1)
    {
        sector->ceilingheight -= (2*FRACUNIT);
        if (sector->ceilingheight <= split->ceildest)
        {
            sector->ceilingheight = split->ceildest;
            cdone = true;
        }
    }
    else if (split->ceildir == 1)
    {
        sector->ceilingheight += (2*FRACUNIT);
        if(sector->ceilingheight >= split->ceildest)
        {
            sector->ceilingheight = split->ceildest;
            cdone = true;
        }
    }

    if (split->flrdir == -1)
    {
        sector->floorheight -= (2*FRACUNIT);
        if(sector->floorheight <= split->flrdest)
        {
            sector->floorheight = split->flrdest;
            fdone = true;
        }
    }
    else if (split->flrdir == 1)
    {
        sector->floorheight += (2*FRACUNIT);
        if(sector->floorheight >= split->flrdest)
        {
            sector->floorheight = split->flrdest;
            fdone = true;
        }
    }

    if(!P_ChangeSector(sector, false))
    {
        if(!cdone || !fdone)
            return;

        P_RemoveThinker(&split->thinker);
        split->sector->specialdata = NULL;
    }
    else
    {
        sector->floorheight = lastflrpos;
        sector->ceilingheight = lastceilpos;
        P_ChangeSector(sector,false);
    }
}

/*================================================================== */
/* */
/* EV_SplitSector */
/* */
/*================================================================== */

int EV_SplitSector(line_t *line, boolean sync) // 80014234
{
    int secnum;
    int rtn;
    sector_t *sec;
    splitmove_t *split;

    secnum = -1;
    rtn = 0;

    while((secnum = P_FindSectorFromLineTag(line->tag, secnum)) >= 0)
    {
        sec = &sectors[secnum];

        /* ALREADY MOVING?  IF SO, KEEP GOING... */
        if (sec->specialdata)
			continue;

        /* */
		/* new split thinker */
		/* */
		rtn = 1;
		split = Z_Malloc (sizeof(*split), PU_LEVSPEC, 0);
		P_AddThinker (&split->thinker);
		sec->specialdata = split;
		split->thinker.function = T_MoveSplitPlane;
        split->sector = sec;

        split->ceildest = sec->ceilingheight + (macrointeger * FRACUNIT);

        if(sync)
        {
            split->flrdest = sec->floorheight + (macrointeger * FRACUNIT);

            if (macrointeger >= 0)
            {
                split->ceildir = 1;
                split->flrdir = 1;
            }
            else
            {
                split->ceildir = -1;
                split->flrdir = -1;
            }
        }
        else
        {
            split->flrdest = sec->floorheight - (macrointeger * FRACUNIT);

            if (macrointeger >= 0)
            {
                split->ceildir = 1;
                split->flrdir = -1;
            }
            else
            {
                split->ceildir = -1;
                split->flrdir = 1;
            }
        }
    }

    return rtn;
}
