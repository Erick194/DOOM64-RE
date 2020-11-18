/*================================================================== */
/*================================================================== */
/* */
/*							PLATFORM RAISING */
/* */
/*================================================================== */
/*================================================================== */
#include "doomdef.h"
#include "p_local.h"

plat_t	*activeplats[MAXPLATS];//80097a24

/*================================================================== */
/* */
/*	Move a plat up and down */
/* */
/*================================================================== */
void	T_PlatRaise(plat_t	*plat) // 8001A890
{
	result_e	res;

	switch(plat->status)
	{
		case	up:
			res = T_MovePlane(plat->sector,plat->speed, plat->high,plat->crush,0,1);

			if (plat->type == raiseAndChange || plat->type == raiseToNearestAndChange)
            {
				if (!(gametic&7))
					S_StartSound((mobj_t *)&plat->sector->soundorg,sfx_secmove);
            }

			//if (res == crushed && (!plat->crush))
			//
            // [d64] crushed and crush is no longer used.
            // more likely to avoid confusion with ceilings
            //
            if(res == stop)
			{
				plat->count = plat->wait;
				plat->status = down;
				S_StartSound((mobj_t *)&plat->sector->soundorg,sfx_pstart);
			}
			else
            {
                if (res == pastdest)
                {
                    plat->count = plat->wait;
                    plat->status = waiting;
                    S_StartSound((mobj_t *)&plat->sector->soundorg,sfx_pstop);
                    switch(plat->type)
                    {
                        case raiseAndChange:
                        case raiseToNearestAndChange:
                        case downWaitUpStay:
                        case blazeDWUS:
                        case customDownUp:
                        case customDownUpFast:
                            P_RemoveActivePlat(plat);
                            break;
                        default:
                            break;
                    }
                }
            }
			break;
		case	down:
			res = T_MovePlane(plat->sector,plat->speed,plat->low,false,0,-1);
			if (res == pastdest)
			{
			    plat->count = plat->wait;
                plat->status = waiting;
                S_StartSound((mobj_t *)&plat->sector->soundorg,sfx_pstop);
			    switch(plat->type)
                {
                    case upWaitDownStay:
                    case blazeUWDS:
                    case customUpDown:
                    case customUpDownFast:
                        P_RemoveActivePlat(plat);
                        break;
                    default:
                        break;
                }
			}
			break;
		case	waiting:
			if (!--plat->count)
			{
				if (plat->sector->floorheight == plat->low)
					plat->status = up;
				else
					plat->status = down;
				S_StartSound((mobj_t *)&plat->sector->soundorg,sfx_pstart);
			}
		case	in_stasis:
			break;
	}
}

/*================================================================== */
/* */
/*	Do Platforms */
/*	"amount" is only used for SOME platforms. */
/* */
/*================================================================== */
int	EV_DoPlat(line_t *line,plattype_e type,int amount) // 8001AA94
{
	plat_t		*plat;
	int			secnum;
	int			rtn;
	sector_t	*sec;

	secnum = -1;
	rtn = 0;

	/* */
	/*	Activate all <type> plats that are in_stasis */
	/* */
	switch(type)
	{
		case perpetualRaise:
			P_ActivateInStasis(line->tag);
			break;
		default:
			break;
	}

	while ((secnum = P_FindSectorFromLineTag(line->tag,secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (sec->specialdata)
			continue;

		/* */
		/* Find lowest & highest floors around sector */
		/* */
		rtn = 1;
		plat = Z_Malloc( sizeof(*plat), PU_LEVSPEC, 0);
		P_AddThinker(&plat->thinker);

		plat->type = type;
		plat->sector = sec;
		plat->sector->specialdata = plat;
		plat->thinker.function = T_PlatRaise;
		plat->crush = false;
		plat->tag = line->tag;
		switch(type)
		{
			case raiseToNearestAndChange:
				plat->speed = PLATSPEED/2;
				sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
				plat->high = P_FindNextHighestFloor(sec,sec->floorheight);
				plat->wait = 0;
				plat->status = up;
				sec->special = 0;		/* NO MORE DAMAGE, IF APPLICABLE */
				S_StartSound((mobj_t *)&sec->soundorg,sfx_secmove);
				break;
			case raiseAndChange:
				plat->speed = PLATSPEED/2;
				sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
				plat->high = sec->floorheight + amount*FRACUNIT;
				plat->wait = 0;
				plat->status = up;
				S_StartSound((mobj_t *)&sec->soundorg,sfx_secmove);
				break;
            case downWaitUpStay:
			case blazeDWUS:
                if (type == blazeDWUS)
                    plat->speed = PLATSPEED*8;
                else
                    plat->speed = PLATSPEED*4;
				plat->low = P_FindLowestFloorSurrounding(sec);
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->high = sec->floorheight;
				plat->wait = 30*PLATWAIT;
				plat->status = down;
				S_StartSound((mobj_t *)&sec->soundorg,sfx_pstart);
				break;
            case upWaitDownStay:
			case blazeUWDS:
                if (type == blazeUWDS)
                    plat->speed = PLATSPEED*8;
                else
                    plat->speed = PLATSPEED*4;
				plat->low = sec->floorheight;
				plat->high = P_FindHighestFloorSurrounding(sec);
				plat->wait = 30*PLATWAIT;
				plat->status = up;
				S_StartSound((mobj_t *)&sec->soundorg,sfx_pstart);
				break;
            case customDownUp:
			case customDownUpFast:
                if (type == customDownUpFast)
                    plat->speed = PLATSPEED*8;
                else
                    plat->speed = PLATSPEED*4;
				plat->low = sec->floorheight - (macrointeger * FRACUNIT);
				plat->high = sec->floorheight;
				plat->wait = 30*PLATWAIT;
				plat->status = down;
				S_StartSound((mobj_t *)&sec->soundorg,sfx_pstart);
				break;
            case customUpDown:
			case customUpDownFast:
                if (type == customUpDownFast)
                    plat->speed = PLATSPEED*8;
                else
                    plat->speed = PLATSPEED*4;
				plat->low = sec->floorheight;
				plat->high = sec->floorheight + (macrointeger * FRACUNIT);
				plat->wait = 30*PLATWAIT;
				plat->status = up;
				S_StartSound((mobj_t *)&sec->soundorg,sfx_pstart);
				break;
			case perpetualRaise:
				plat->speed = PLATSPEED;
				plat->low = P_FindLowestFloorSurrounding(sec);
				if (plat->low > sec->floorheight)
					plat->low = sec->floorheight;
				plat->high = P_FindHighestFloorSurrounding(sec);
				if (plat->high < sec->floorheight)
					plat->high = sec->floorheight;
				plat->wait = 30*PLATWAIT;
				plat->status = ((P_Random()&1) << 1) + down;
				S_StartSound((mobj_t *)&sec->soundorg,sfx_pstart);
				break;
		}
		P_AddActivePlat(plat);
	}
	return rtn;
}

void P_ActivateInStasis(int tag) // 8001AE6C
{
	int		i;

	for (i = 0;i < MAXPLATS;i++)
		if (activeplats[i] &&
			(activeplats[i])->tag == tag &&
			(activeplats[i])->status == in_stasis)
		{
			(activeplats[i])->status = (activeplats[i])->oldstatus;
			(activeplats[i])->thinker.function = T_PlatRaise;
		}
}

int EV_StopPlat(line_t *line) // 8001AF2C
{
	int		j;
	int     ok;

	ok = 0;
	for (j = 0; j < MAXPLATS; j++)
	{
		if (activeplats[j] && ((activeplats[j])->status != in_stasis) &&
			((activeplats[j])->tag == line->tag))
		{
		    ok = 1;
			(activeplats[j])->oldstatus = (activeplats[j])->status;
			(activeplats[j])->status = in_stasis;
			(activeplats[j])->thinker.function = NULL;
		}
	}

	return ok;
}

void P_AddActivePlat(plat_t *plat) // 8001AFF8
{
	int		i;
	for (i = 0;i < MAXPLATS;i++)
    {
		if (activeplats[i] == NULL)
		{
			activeplats[i] = plat;
			return;
		}
    }
	I_Error ("P_AddActivePlat: no more plats!");
}

void P_RemoveActivePlat(plat_t *plat) // 8001B050
{
	int		i;
	for (i = 0;i < MAXPLATS;i++)
    {
		if (plat == activeplats[i])
		{
			(activeplats[i])->sector->specialdata = NULL;
			P_RemoveThinker(&(activeplats[i])->thinker);
			activeplats[i] = NULL;
			return;
		}
    }
	I_Error ("P_RemoveActivePlat: can't find plat!");
}
