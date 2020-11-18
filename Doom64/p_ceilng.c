#include "doomdef.h"
#include "p_local.h"

/*================================================================== */
/*================================================================== */
/* */
/*							CEILINGS */
/* */
/*================================================================== */
/*================================================================== */

ceiling_t	*activeceilings[MAXCEILINGS]; //800A5610

/*================================================================== */
/* */
/*	T_MoveCeiling */
/* */
/*================================================================== */
void T_MoveCeiling (ceiling_t *ceiling) // 8000F880
{
	result_e	res;

	switch(ceiling->direction)
	{
		case 0:		/* IN STASIS */
			break;
		case 1:		/* UP */
			res = T_MovePlane(ceiling->sector,ceiling->speed,
					ceiling->topheight,false,1,ceiling->direction);

			if (!ceiling->instant)
			{
			    if (!(gametic & 7))
                {
                    S_StartSound((mobj_t *)&ceiling->sector->soundorg, sfx_secmove);
                }
			}

			if (res == pastdest)
            {
				switch(ceiling->type)
				{
				    //case lowerToFloor://---
					case raiseToHighest:
                    case customCeiling:
                    case crushAndRaiseOnce:
                    case customCeilingToHeight:
						P_RemoveActiveCeiling(ceiling);
						break;
                    //case lowerAndCrush://---
					case crushAndRaise:
                    case silentCrushAndRaise:
                    case fastCrushAndRaise:
						ceiling->direction = -1;
						break;
					default:
						break;
				}
            }
			break;
		case -1:	/* DOWN */
			res = T_MovePlane(ceiling->sector,ceiling->speed,
				ceiling->bottomheight,ceiling->crush,1,ceiling->direction);

            if (!ceiling->instant)
			{
			    if (!(gametic & 7))
                {
                    S_StartSound((mobj_t *)&ceiling->sector->soundorg, sfx_secmove);
                }
			}

			if (res == pastdest)
            {
				switch(ceiling->type)
				{
					case silentCrushAndRaise:
					case crushAndRaise:
						ceiling->speed = CEILSPEED;
					case fastCrushAndRaise:
                    case crushAndRaiseOnce:
						ceiling->direction = 1;
						break;
					case lowerAndCrush:
					case lowerToFloor:
                    case customCeiling:
                    case customCeilingToHeight:
						P_RemoveActiveCeiling(ceiling);
						break;
					default:
						break;
				}
            }
			else
            {
                if (res == crushed)
                {
                    switch(ceiling->type)
                    {
                        case crushAndRaise:
                        case lowerAndCrush:
                            ceiling->speed = CEILSPEED / 8;
                            break;
                        default:
                            break;
                    }
                }
            }
			break;
	}
}

/*================================================================== */
/* */
/*		EV_DoCeiling */
/*		Move a ceiling up/down and all around! */
/* */
/*================================================================== */
int EV_DoCeiling (line_t *line, ceiling_e  type, fixed_t speed) // 8000FA4C
{
	int			secnum,rtn;
	sector_t		*sec;
	ceiling_t		*ceiling;

	secnum = -1;
	rtn = 0;

	/* */
	/*	Reactivate in-stasis ceilings...for certain types. */
	/* */
	switch(type)
	{
		case fastCrushAndRaise:
		case silentCrushAndRaise:
		case crushAndRaise:
			P_ActivateInStasisCeiling(line);
		default:
			break;
	}

	while ((secnum = P_FindSectorFromLineTag(line->tag,secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (sec->specialdata)
			continue;

		/* */
		/* new door thinker */
		/* */
		rtn = 1;
		ceiling = Z_Malloc (sizeof(*ceiling), PU_LEVSPEC, 0);
		P_AddThinker (&ceiling->thinker);
		sec->specialdata = ceiling;
		ceiling->thinker.function = T_MoveCeiling;
		ceiling->sector = sec;
		ceiling->crush = false;

        if (speed == (4096 * FRACUNIT))
            ceiling->instant = true;
        else
            ceiling->instant = false;

		switch(type)
		{
		    case silentCrushAndRaise:
                ceiling->instant = true;
			case fastCrushAndRaise:
            case crushAndRaiseOnce:
				ceiling->crush = true;
				ceiling->topheight = sec->ceilingheight;
				ceiling->bottomheight = sec->floorheight + (8*FRACUNIT);
				ceiling->direction = -1;
				ceiling->speed = speed;
				break;
			case crushAndRaise:
				ceiling->crush = true;
				ceiling->topheight = sec->ceilingheight;
			case lowerAndCrush:
			case lowerToFloor:
				ceiling->bottomheight = sec->floorheight;
				if (type != lowerToFloor)
                    ceiling->bottomheight += 8*FRACUNIT;
				ceiling->direction = -1;
				ceiling->speed = speed;
				break;
			case raiseToHighest:
				ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
				ceiling->direction = 1;
				ceiling->speed = speed;
				break;
            case customCeiling:
                ceiling->speed = speed;
                if(macrointeger >= 0)
                {
                    ceiling->direction = 1;
                    ceiling->topheight = ceiling->sector->ceilingheight + (macrointeger * FRACUNIT);
                }
                else
                {
                    ceiling->direction = -1;
                    ceiling->bottomheight = ceiling->sector->ceilingheight + (macrointeger * FRACUNIT);
                }
                break;
            case customCeilingToHeight:
                ceiling->speed = speed;
                if((macrointeger * FRACUNIT) < ceiling->sector->ceilingheight)
                {
                    ceiling->bottomheight = (macrointeger * FRACUNIT);
                    ceiling->direction = -1;
                }
                else
                {
                    ceiling->topheight = (macrointeger * FRACUNIT);
                    ceiling->direction = 1;
                }
                break;
		}

		ceiling->tag = sec->tag;
		ceiling->type = type;
		P_AddActiveCeiling(ceiling);
	}
	return rtn;
}

/*================================================================== */
/* */
/*		Add an active ceiling */
/* */
/*================================================================== */
void P_AddActiveCeiling(ceiling_t *c) // 8000FCC0
{
	int		i;
	for (i = 0; i < MAXCEILINGS;i++)
    {
		if (activeceilings[i] == NULL)
		{
			activeceilings[i] = c;
			return;
		}
    }

    // [d64] added error message
    I_Error("P_AddActiveCeiling: no more ceiling slots");
}

/*================================================================== */
/* */
/*		Remove a ceiling's thinker */
/* */
/*================================================================== */
void P_RemoveActiveCeiling(ceiling_t *c) // 8000FD18
{
	int		i;

	for (i = 0;i < MAXCEILINGS;i++)
    {
		if (activeceilings[i] == c)
		{
			activeceilings[i]->sector->specialdata = NULL;
			P_RemoveThinker (&activeceilings[i]->thinker);
			activeceilings[i] = NULL;
			return;
		}
    }

    // [d64] added error message
    I_Error("P_RemoveActiveCeiling: ceiling not found");
}

/*================================================================== */
/* */
/*		Restart a ceiling that's in-stasis */
/* */
/*================================================================== */
void P_ActivateInStasisCeiling(line_t *line) // 8000FD88
{
	int	i;

	for (i = 0;i < MAXCEILINGS;i++)
    {
		if (activeceilings[i] && (activeceilings[i]->tag == line->tag) &&
			(activeceilings[i]->direction == 0))
		{
			activeceilings[i]->direction = activeceilings[i]->olddirection;
			activeceilings[i]->thinker.function = T_MoveCeiling;
		}
    }
}

/*================================================================== */
/* */
/*		EV_CeilingCrushStop */
/*		Stop a ceiling from crushing! */
/* */
/*================================================================== */
int	EV_CeilingCrushStop(line_t	*line) // 8000FF74
{
	int		i;
	int		rtn;

	rtn = 0;
	for (i = 0;i < MAXCEILINGS;i++)
    {
		if (activeceilings[i] && (activeceilings[i]->tag == line->tag) &&
			(activeceilings[i]->direction != 0))
		{
			activeceilings[i]->olddirection = activeceilings[i]->direction;
			activeceilings[i]->thinker.function = NULL;
			activeceilings[i]->direction = 0;		/* in-stasis */
			rtn = 1;
		}
    }

	return rtn;
}
