#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"

/*================================================================== */
/*================================================================== */
/* */
/*							VERTICAL DOORS */
/* */
/*================================================================== */
/*================================================================== */

/*================================================================== */
/* */
/*	T_VerticalDoor */
/* */
/*================================================================== */
void T_VerticalDoor (vldoor_t *door) // 800104A0
{
	result_e	res1;
	result_e	res2;

	switch(door->direction)
	{
		case 0:		/* WAITING */
			if (!--door->topcountdown)
            {
				switch(door->type)
				{
					case BlazeRaise:
						door->direction = -1; /* time to go back down */
						S_StartSound((mobj_t *)&door->sector->soundorg,80/*sfx_bdcls*/);
						break;
					case Normal:
						door->direction = -1; /* time to go back down */
						S_StartSound((mobj_t *)&door->sector->soundorg,18/*sfx_dorcls*/);
						break;
					case Close30ThenOpen:
						door->direction = 1;
						S_StartSound((mobj_t *)&door->sector->soundorg,17/*sfx_doropn*/);
						break;
                    //case RaiseIn5Mins: // [D64] moved here?
						//door->direction = 1;
						//door->type = Normal;
						//S_StartSound((mobj_t *)&door->sector->soundorg,17/*sfx_doropn*/);
						//break;
					default:
						break;
				}
            }
			break;
        #if 1 // [D64] no used
		case 2:		/*  INITIAL WAIT */
			if (!--door->topcountdown)
            {
				switch(door->type)
				{
					case RaiseIn5Mins:
						door->direction = 1;
						door->type = Normal;
						S_StartSound((mobj_t *)&door->sector->soundorg,17/*sfx_doropn*/);
						break;
				default:
					break;
				}
            }
			break;
        #endif // 0
		case -1:	/* DOWN */
			res1 = T_MovePlane(door->sector,door->speed, door->initceiling,false,1,-1);
			res2 = T_MovePlane(door->sector,door->speed, door->initceiling,false,0,1);
			if ((res1 == pastdest) && (res2 == pastdest))
            {
				switch(door->type)
				{
					case BlazeRaise:
					case BlazeClose:
						door->sector->specialdata = NULL;
						P_RemoveThinker(&door->thinker);  /* unlink and free */
						S_StartSound((mobj_t *)&door->sector->soundorg, 80/*sfx_bdcls*/);
						break;
					case Normal:
					case DoorClose:
						door->sector->specialdata = NULL;
						P_RemoveThinker (&door->thinker);  /* unlink and free */
						break;
					case Close30ThenOpen:
						door->direction = 0;
						door->topcountdown = TICRATE*30;
						break;
					default:
						break;
				}
            }
			else if ((res1 == crushed) || (res2 == crushed))
			{
				switch (door->type)
				{
				case BlazeClose:
				case DoorClose:		/* DO NOT GO BACK UP! */
					break;

				default:
					door->direction = 1;
					S_StartSound((mobj_t *)&door->sector->soundorg, 17/*sfx_doropn*/);
					break;
				}
			}
			break;
		case 1:		/* UP */
			res1 = T_MovePlane(door->sector,door->speed,door->topheight,false,1,1);
			res2 = T_MovePlane(door->sector,door->speed,door->bottomheight,false,0,-1);
			if ((res1 == pastdest) && (res2 == pastdest))
            {
				switch(door->type)
				{
					case BlazeRaise:
					case Normal:
						door->direction = 0; /* wait at top */
						door->topcountdown = door->topwait;
						break;
					case Close30ThenOpen:
					case BlazeOpen:
					case DoorOpen:
						door->sector->specialdata = NULL;
						P_RemoveThinker (&door->thinker);  /* unlink and free */
						break;
					default:
						break;
				}
            }
			break;
	}
}

/*================================================================== */
/* */
/*		EV_DoDoor */
/*		Move a door up/down and all around! */
/* */
/*================================================================== */
int EV_DoDoor (line_t *line, vldoor_e  type) // 80010750
{
	int			secnum,rtn;
	sector_t		*sec;
	vldoor_t		*door;

	secnum = -1;
	rtn = 0;
	while ((secnum = P_FindSectorFromLineTag(line->tag,secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (sec->specialdata)
			continue;

		/* */
		/* new door thinker */
		/* */
		rtn = 1;
		door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
		P_AddThinker (&door->thinker);
		sec->specialdata = door;
		door->thinker.function = T_VerticalDoor;
		door->topwait = VDOORWAIT;
		door->speed = VDOORSPEED;
		door->sector = sec;
		door->type = type;
        door->bottomheight = sec->floorheight;
        door->initceiling = sec->floorheight;

		switch(type)
		{
			case BlazeClose:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->direction = -1;
				door->speed = VDOORSPEED * 4;
				S_StartSound((mobj_t *)&door->sector->soundorg, 80/*sfx_bdcls*/);
				break;
			case DoorClose:
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4*FRACUNIT;
				door->direction = -1;
				S_StartSound((mobj_t *)&door->sector->soundorg,18/*sfx_dorcls*/);
				break;
			case Close30ThenOpen:
				door->topheight = sec->ceilingheight;
				door->direction = -1;
				S_StartSound((mobj_t *)&door->sector->soundorg,18/*sfx_dorcls*/);
				break;
			case BlazeRaise:
			case BlazeOpen:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4 * FRACUNIT;
				door->speed = VDOORSPEED * 4;
				if (door->topheight != sec->ceilingheight)
					S_StartSound((mobj_t *)&door->sector->soundorg, 79/*sfx_bdopn*/);
				break;
			case Normal:
			case DoorOpen:
				door->direction = 1;
				door->topheight = P_FindLowestCeilingSurrounding(sec);
				door->topheight -= 4*FRACUNIT;
				if (door->topheight != sec->ceilingheight)
					S_StartSound((mobj_t *)&door->sector->soundorg,17/*sfx_doropn*/);
				break;
			default:
				break;
		}
	}
	return rtn;
}


/*================================================================== */
/* */
/*	EV_VerticalDoor : open a door manually, no tag value */
/* */
/*================================================================== */
void EV_VerticalDoor (line_t *line, mobj_t *thing) // 80010998
{
	player_t		*player;
	sector_t		*sec;
	vldoor_t		*door;
	int				side;

	side = 0;			/* only front sides can be used */

	/* if the sector has an active thinker, use it */
	sec = sides[ line->sidenum[side^1]] .sector;
	if (sec->specialdata)
	{
		door = sec->specialdata;
		switch(SPECIALMASK(line->special))
		{
			case 1:		/* ONLY FOR "RAISE" DOORS, NOT "OPEN"s */
			case 117:
				if (door->direction == -1)
					door->direction = 1;	/* go back up */
				else
				{
					if (!thing->player)
						return;				/* JDC: bad guys never close doors */
					door->direction = -1;	/* start going down immediately */
				}
				return;
		}
	}

	/* for proper sound */
	switch(SPECIALMASK(line->special))
	{
		case 117:	/* BLAZING DOOR RAISE */
		case 118:	/* BLAZING DOOR OPEN */
			S_StartSound((mobj_t *)&sec->soundorg,79/*sfx_bdopn*/);
			break;
		default:	/* LOCKED DOOR SOUND */
			S_StartSound((mobj_t *)&sec->soundorg,17/*sfx_doropn*/);
			break;
	}

	/* */
	/* new door thinker */
	/* */
	door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
	P_AddThinker (&door->thinker);
	sec->specialdata = door;
	door->thinker.function = T_VerticalDoor;
	door->speed = VDOORSPEED;
	door->sector = sec;
	door->direction = 1;
	door->topwait = VDOORWAIT;

	switch(SPECIALMASK(line->special))
	{
		case 1:
			door->type = Normal;
			break;
		case 31:
			door->type = DoorOpen;
			line->special = 0;
			break;
		case 117:	/* blazing door raise */
			door->type = BlazeRaise;
			door->speed = VDOORSPEED * 4;
			break;
		case 118:	/* blazing door open */
			door->type = BlazeOpen;
			door->speed = VDOORSPEED * 4;
			line->special = 0;
			break;
	}

	/* */
	/* find the top and bottom of the movement range */
	/* */
	door->topheight = P_FindLowestCeilingSurrounding(sec);
	door->topheight -= 4*FRACUNIT;
    door->bottomheight = P_FindLowestFloorSurrounding(sec);
    door->initceiling = sec->floorheight;

    if(door->bottomheight != sec->floorheight)
        door->bottomheight += 4*FRACUNIT;
}
