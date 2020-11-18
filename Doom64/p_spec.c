/* P_Spec.c */
#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"
#include "st_main.h"

extern mapthing_t  *spawnlist;     // 800A5D74
extern int         spawncount;     // 800A5D78

line_t	    **linespeciallist;  // 800A5F60
int		    numlinespecials;    // 800A5F64

sector_t    **sectorspeciallist;// 800A5F68
int         numsectorspecials;  // 800A5F6C

animdef_t		animdefs[MAXANIMS] = // 8005AE80
{
	{ 15, "SMONAA", 4, 7, false, false },
	{  0, "SMONBA", 4, 1, false, false },
	{  0, "SMONCA", 4, 7, false, false },
	{ 90, "CFACEA", 3, 3,  true, false },
	{  0, "SMONDA", 4, 3, false, false },
	{ 10, "SMONEA", 4, 7, false, false },
	{  0, "SPORTA", 9, 3, false,  true },
	{ 10,  "SMONF", 5, 1,  true,  true },
	{ 10, "STRAKR", 5, 1,  true,  true },
	{ 10, "STRAKB", 5, 1,  true,  true },
	{ 10, "STRAKY", 5, 1,  true,  true },
	{ 50,  "C307B", 5, 1,  true,  true },
	{  0,   "CTEL", 8, 3, false,  true },
	{  0,"CASFL98", 5, 7,  true,  true },
	{  0,  "HTELA", 4, 1,  true, false }
};

/*----------
/ anims[8] -> anims[MAXANIMS = 15]
/ For some reason Doom 64 is 8,
/ I will leave it at 15 to avoid problems loading data into this pointer.
/---------*/
anim_t	anims[MAXANIMS], *lastanim; // 800A5F70, 800A6090

card_t      MapBlueKeyType;     //0x80077E9C
card_t      MapRedKeyType;      //0x8007821C
card_t      MapYellowKeyType;   //0x800780A0

void P_AddSectorSpecial(sector_t *sec);

/*
=================
=
= P_Init
=
=================
*/

void P_Init (void) // 8001F340
{
	int i;
	sector_t    *sector;
	side_t      *side;

	side = sides;
	for (i = 0; i < numsides; i++, side++)
	{
        W_CacheLumpNum(side->toptexture + firsttex    , PU_LEVEL, dec_d64);
        W_CacheLumpNum(side->bottomtexture + firsttex , PU_LEVEL, dec_d64);
        W_CacheLumpNum(side->midtexture + firsttex    , PU_LEVEL, dec_d64);
	}

	sector = sectors;
	for (i = 0; i < numsectors; i++, sector++)
	{
		if (sector->ceilingpic >= 0)
            W_CacheLumpNum(sector->ceilingpic + firsttex, PU_LEVEL, dec_d64);

        if (sector->floorpic >= 0)
            W_CacheLumpNum(sector->floorpic + firsttex, PU_LEVEL, dec_d64);

        if (sector->flags & MS_LIQUIDFLOOR)
            W_CacheLumpNum(sector->floorpic + firsttex + 1, PU_LEVEL,dec_d64);
	}
}

/*
==============================================================================

							SPECIAL SPAWNING

==============================================================================
*/
/*
================================================================================
= P_SpawnSpecials
=
= After the map has been loaded, scan for specials that
= spawn thinkers
=
===============================================================================
*/

void P_SpawnSpecials (void) // 8001F490
{
    mobj_t      *mo;
	sector_t    *sector;
	line_t      *line;
	int         i, j;
	int         lump;

	/* */
	/*	Init animation aka (P_InitPicAnims) */
	/* */
	lastanim = anims;
	for (i=0 ; i < MAXANIMS ; i++)
	{
	    lump = W_GetNumForName(animdefs[i].startname);

        lastanim->basepic   = (lump - firsttex);
        lastanim->tics      = animdefs[i].speed;
        lastanim->delay     = animdefs[i].delay;
        lastanim->delaycnt  = lastanim->delay;
        lastanim->isreverse = animdefs[i].isreverse;

	    if (animdefs[i].ispalcycle == false)
	    {
            lastanim->current   = (lump << 4);
            lastanim->picstart  = (lump << 4);
            lastanim->picend    = (animdefs[i].frames + lump - 1) << 4;
            lastanim->frame     = 16;

            /* Load the following graphics for animation */
            for (j=0 ; j < animdefs[i].frames ; j++)
            {
                W_CacheLumpNum(lump, PU_LEVEL, dec_d64);
                lump++;
            }
	    }
	    else
        {
            lastanim->current   = (lump << 4);
            lastanim->picstart  = (lump << 4);
            lastanim->picend    = (lump << 4) | (animdefs[i].frames - 1);
            lastanim->frame     = 1;
        }

		lastanim++;
	}

	/* */
	/*	Init Macro Variables */
	/* */
    activemacro = NULL;
    macrocounter = 0;
    macroidx1 = 0;
    macroidx2 = 0;

	/* */
	/*	Init special SECTORs */
	/* */
	scrollfrac = 0;
	numsectorspecials = 0; // Restart count
	sector = sectors;
	for(i = 0; i < numsectors; i++, sector++)
    {
        P_AddSectorSpecial(sector);
        if(sector->flags & MS_SECRET)
        {
            totalsecret++;
        }

        if(sector->flags & (MS_SCROLLCEILING | MS_SCROLLFLOOR|
        MS_SCROLLLEFT | MS_SCROLLRIGHT| MS_SCROLLUP| MS_SCROLLDOWN))
        {
            numsectorspecials++;
        }
    }

    sectorspeciallist = (sector_t**)Z_Malloc(numsectorspecials*sizeof(void*), PU_LEVEL, NULL);
    sector = sectors;
	for(i = 0, j = 0; i < numsectors; i++, sector++)
    {
        if(sector->flags & (MS_SCROLLCEILING | MS_SCROLLFLOOR|
        MS_SCROLLLEFT | MS_SCROLLRIGHT| MS_SCROLLUP| MS_SCROLLDOWN))
        {
            sectorspeciallist[j] = sector;
            j++;
        }
    }

	/* */
	/*	Init line EFFECTs */
	/* */
	numlinespecials = 0;
	line = lines;
	for (i = 0;i < numlines; i++, line++)
    {
        if(line->flags & (ML_SCROLLRIGHT|ML_SCROLLLEFT|ML_SCROLLUP|ML_SCROLLDOWN))
        {
            numlinespecials++;
        }
    }

    linespeciallist = (line_t**)Z_Malloc(numlinespecials*sizeof(void*), PU_LEVEL, NULL);
    line = lines;
	for (i = 0, j = 0; i < numlines; i++, line++)
    {
        if(line->flags & (ML_SCROLLRIGHT|ML_SCROLLLEFT|ML_SCROLLUP|ML_SCROLLDOWN))
        {
            linespeciallist[j] = line;
            j++;
        }
    }

    /* */
	/*	Init Keys */
	/* */

    MapBlueKeyType   = it_bluecard;
    MapYellowKeyType = it_yellowcard;
    MapRedKeyType    = it_redcard;
    for (mo = mobjhead.next ; mo != &mobjhead ; mo = mo->next)
    {
        if((mo->type == MT_ITEM_BLUESKULLKEY) ||
           (mo->type == MT_ITEM_YELLOWSKULLKEY) ||
           (mo->type == MT_ITEM_REDSKULLKEY))
        {
            MapBlueKeyType   = it_blueskull;
            MapYellowKeyType = it_yellowskull;
            MapRedKeyType    = it_redskull;
            break;
        }

        /*switch (mo->type)
        {
            case MT_ITEM_BLUESKULLKEY:   MapBlueKeyType   = it_blueskull;     break;
            case MT_ITEM_YELLOWSKULLKEY: MapYellowKeyType = it_yellowskull;   break;
            case MT_ITEM_REDSKULLKEY:    MapRedKeyType    = it_redskull;      break;
        }*/
    }

    for (i = 0; i < spawncount; i++)
    {
        if((spawnlist[i].type == 40) ||
           (spawnlist[i].type == 39) ||
           (spawnlist[i].type == 38))
        {
            MapBlueKeyType   = it_blueskull;
            MapYellowKeyType = it_yellowskull;
            MapRedKeyType    = it_redskull;
            break;
        }

        /*if(spawnlist[i].type == 40)//mobjinfo[MT_ITEM_BLUESKULLKEY].doomednum
            MapBlueKeyType   = it_blueskull;
        if(spawnlist[i].type == 39)//mobjinfo[MT_ITEM_YELLOWSKULLKEY].doomednum
            MapYellowKeyType   = it_yellowskull;
        if(spawnlist[i].type == 38)//mobjinfo[MT_ITEM_REDSKULLKEY].doomednum
            MapRedKeyType   = it_redskull;*/
    }

	/* */
	/*	Init other misc stuff */
	/* */
	D_memset(activeceilings, 0, MAXCEILINGS * sizeof(ceiling_t*));
	D_memset(activeplats, 0, MAXPLATS * sizeof(plat_t*));
	D_memset(buttonlist, 0, MAXBUTTONS * sizeof(button_t));
}

/*
==============================================================================

							UTILITIES

==============================================================================
*/

/*================================================================== */
/* */
/*	Return sector_t * of sector next to current. NULL if not two-sided line */
/* */
/*================================================================== */
sector_t *getNextSector(line_t *line,sector_t *sec) // 8001F96C
{
	if (!(line->flags & ML_TWOSIDED))
		return NULL;

	if (line->frontsector == sec)
		return line->backsector;

	return line->frontsector;
}

/*================================================================== */
/* */
/*	FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t	P_FindLowestFloorSurrounding(sector_t *sec) // 8001F9AC
{
	int			i;
	line_t		*check;
	sector_t	*other;
	fixed_t		floor = sec->floorheight;

	for (i=0 ;i < sec->linecount ; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check,sec);
		if (!other)
			continue;
		if (other->floorheight < floor)
			floor = other->floorheight;
	}
	return floor;
}

/*================================================================== */
/* */
/*	FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t	P_FindHighestFloorSurrounding(sector_t *sec) // 8001FA48
{
	int			i;
	line_t		*check;
	sector_t	*other;
	fixed_t		floor = -500*FRACUNIT;

	for (i=0 ;i < sec->linecount ; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check,sec);
		if (!other)
			continue;
		if (other->floorheight > floor)
			floor = other->floorheight;
	}
	return floor;
}

/*================================================================== */
/* */
/*	FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t	P_FindNextHighestFloor(sector_t *sec,int currentheight) // 8001FAE4
{
	int			i;
	int			h;
	int			min;
	line_t		*check;
	sector_t	*other;
	fixed_t		height = currentheight;
	fixed_t		heightlist[20];		/* 20 adjoining sectors max! */

	for (i =0,h = 0 ;i < sec->linecount ; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check,sec);
		if (!other)
			continue;
		if (other->floorheight > height)
			heightlist[h++] = other->floorheight;
	}

	/* */
	/* Find lowest height in list */
	/* */
	min = heightlist[0];
	for (i = 1;i < h;i++)
		if (heightlist[i] < min)
			min = heightlist[i];

	return min;
}

/*================================================================== */
/* */
/*	FIND LOWEST CEILING IN THE SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t	P_FindLowestCeilingSurrounding(sector_t *sec) // 8001FC68
{
	int			i;
	line_t		*check;
	sector_t	*other;
	fixed_t		height = MAXINT;

	for (i=0 ;i < sec->linecount ; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check,sec);
		if (!other)
			continue;
		if (other->ceilingheight < height)
			height = other->ceilingheight;
	}
	return height;
}

/*================================================================== */
/* */
/*	FIND HIGHEST CEILING IN THE SURROUNDING SECTORS */
/* */
/*================================================================== */
fixed_t	P_FindHighestCeilingSurrounding(sector_t *sec) // 8001FD08
{
	int	i;
	line_t	*check;
	sector_t	*other;
	fixed_t	height = 0;

	for (i=0 ;i < sec->linecount ; i++)
	{
		check = sec->lines[i];
		other = getNextSector(check,sec);
		if (!other)
			continue;
		if (other->ceilingheight > height)
			height = other->ceilingheight;
	}
	return height;
}

/*================================================================== */
/* */
/*	RETURN NEXT SECTOR # THAT LINE TAG REFERS TO */
/* */
/*================================================================== */
int	P_FindSectorFromLineTag(int tag,int start) // 8001FDA4
{
	int	i;

	for (i=start+1;i<numsectors;i++)
		if (sectors[i].tag == tag)
			return i;
	return -1;
}

/*================================================================== */
/* */
/*	RETURN NEXT LIGHT # THAT LINE TAG REFERS TO */
/*	Exclusive Doom 64 */
/* */
/*================================================================== */
int P_FindLightFromLightTag(int tag,int start) // 8001FE08
{
	int	i;

	for (i=(start+256+1);i<numlights;i++)
		if (lights[i].tag == tag)
			return i;
	return -1;
}

/*================================================================== */
/* */
/*	RETURN TRUE OR FALSE */
/*	Exclusive Doom 64 */
/* */
/*================================================================== */
boolean P_ActivateLineByTag(int tag,mobj_t *thing) // 8001FE64
{
	int	i;
	line_t *li;

	li = lines;
	for (i=0;i<numlines;i++,li++)
    {
		if (li->tag == tag)
			return P_UseSpecialLine(li, thing);
    }
	return false;
}

#if 0
/*================================================================== */
/* */
/*	Find minimum light from an adjacent sector */
/* */
/*================================================================== */
int	P_FindMinSurroundingLight(sector_t *sector,int max)//L80026C10()
{
	int			i;
	int			min;
	line_t		*line;
	sector_t	*check;

	min = max;
	for (i=0 ; i < sector->linecount ; i++)
	{
		line = sector->lines[i];
		check = getNextSector(line,sector);
		if (!check)
			continue;
		if (check->lightlevel < min)
			min = check->lightlevel;
	}
	return min;
}
#endif // 0

/*
==============================================================================

							EVENTS

Events are operations triggered by using, crossing, or shooting special lines, or by timed thinkers

==============================================================================
*/

/*
===============================================================================
=
= P_UpdateSpecials
=
= Animate planes, scroll walls, etc
===============================================================================
*/

#define SCROLLLIMIT (FRACUNIT*127)

void P_UpdateSpecials (void) // 8001FEC0
{
	anim_t		*anim;
	line_t		*line;
	sector_t	*sector;
	fixed_t     speed;
	int			i;
	int         neg;

	/* */
	/*	ANIMATE FLATS AND TEXTURES GLOBALY */
	/* */
	for (anim = anims ; anim < lastanim ; anim++)
	{
	    anim->delaycnt--;
		if ((anim->delaycnt <= 0) && !(gametic & anim->tics))
		{
		    anim->current += anim->frame;

		    if ((anim->current < anim->picstart) || (anim->picend < anim->current))
            {
                neg = -anim->frame;

                if (anim->isreverse)
                {
                    anim->frame = neg;
                    anim->current += neg;

                    if (anim->delay == 0)
                        anim->current += neg + neg;
                }
                else
                {
                    anim->current = anim->picstart;
                }

                anim->delaycnt = anim->delay;
            }

            textures[anim->basepic] = anim->current;
		}
	}

	/* */
	/*	ANIMATE LINE SPECIALS */
	/* */
	for (i = 0; i < numlinespecials; i++)
	{
		line = linespeciallist[i];

		if(line->flags & ML_SCROLLRIGHT)
        {
            sides[line->sidenum[0]].textureoffset += FRACUNIT;
            sides[line->sidenum[0]].textureoffset &= SCROLLLIMIT;
        }
        else if(line->flags & ML_SCROLLLEFT)
        {
            sides[line->sidenum[0]].textureoffset -= FRACUNIT;
            sides[line->sidenum[0]].textureoffset &= SCROLLLIMIT;
        }

        if(line->flags & ML_SCROLLUP)
        {
            sides[line->sidenum[0]].rowoffset += FRACUNIT;
            sides[line->sidenum[0]].rowoffset &= SCROLLLIMIT;
        }
        else if(line->flags & ML_SCROLLDOWN)
        {
            sides[line->sidenum[0]].rowoffset -= FRACUNIT;
            sides[line->sidenum[0]].rowoffset &= SCROLLLIMIT;
        }
	}

	/* */
	/*	ANIMATE SECTOR SPECIALS */
	/* */

    scrollfrac = (scrollfrac + (FRACUNIT / 2));

    for (i = 0; i < numsectorspecials; i++)
	{
		sector = sectorspeciallist[i];

		if(sector->flags & MS_SCROLLFAST)
            speed = 3*FRACUNIT;
        else
            speed = FRACUNIT;

        if(sector->flags & MS_SCROLLLEFT)
        {
            sector->xoffset += speed;
        }
        else if(sector->flags & MS_SCROLLRIGHT)
        {
            sector->xoffset -= speed;
        }

        if(sector->flags & MS_SCROLLUP)
        {
            sector->yoffset -= speed;
        }
        else if(sector->flags & MS_SCROLLDOWN)
        {
            sector->yoffset += speed;
        }
	}

	/* */
	/*	DO BUTTONS */
	/* */
	for (i = 0; i < MAXBUTTONS; i++)
	{
		if (buttonlist[i].btimer > 0)
		{
			buttonlist[i].btimer -= vblsinframe[0];

			if (buttonlist[i].btimer <= 0)
			{
				switch (buttonlist[i].where)
				{
				case top:
					buttonlist[i].side->toptexture = buttonlist[i].btexture;
					break;
				case middle:
					buttonlist[i].side->midtexture = buttonlist[i].btexture;
					break;
				case bottom:
					buttonlist[i].side->bottomtexture = buttonlist[i].btexture;
					break;
				}
				S_StartSound((mobj_t *)buttonlist[i].soundorg, sfx_switch1);
				D_memset(&buttonlist[i], 0, sizeof(button_t)); // ? Doom 64 elimina esta linea
			}
		}
	}
}

/*
==============================================================================

							UTILITIES

==============================================================================
*/

/* */
/*	Will return a side_t* given the number of the current sector, */
/*		the line number, and the side (0/1) that you want. */
/* */
side_t *getSide(int currentSector,int line, int side) // 8002026C
{
	return &sides[ (sectors[currentSector].lines[line])->sidenum[side] ];
}

/* */
/*	Will return a sector_t* given the number of the current sector, */
/*		the line number and the side (0/1) that you want. */
/* */
sector_t *getSector(int currentSector,int line,int side) // 800202BC
{
	return sides[ (sectors[currentSector].lines[line])->sidenum[side] ].sector;
}

/* */
/*	Given the sector number and the line number, will tell you whether */
/*		the line is two-sided or not. */
/* */
int	twoSided(int sector,int line) // 80020314
{
	return (sectors[sector].lines[line])->flags & ML_TWOSIDED;
}

/*
==============================================================================

							EVENTS

Events are operations triggered by using, crossing, or shooting special lines, or by timed thinkers

==============================================================================
*/

void P_AddSectorSpecial(sector_t* sector) // 80020354
{

    if((sector->flags & MS_SYNCSPECIALS) && (sector->special))
    {
        P_CombineLightSpecials(sector);
        return;
    }

    switch(sector->special)
    {
    case 0:
        sector->lightlevel = 0;
        break;

    case 1:
        /* FLICKERING LIGHTS */
        P_SpawnLightFlash(sector);
        break;

    case 2:
        /* STROBE FAST */
        P_SpawnStrobeFlash(sector, FASTDARK);
        break;

    case 3:
        /* STROBE SLOW */
        P_SpawnStrobeFlash(sector, SLOWDARK);
        break;

    case 8:
        /* GLOWING LIGHT */
        P_SpawnGlowingLight(sector, PULSENORMAL);
        break;

    case 9:
        P_SpawnGlowingLight(sector, PULSESLOW);
        break;

    case 11:
        P_SpawnGlowingLight(sector, PULSERANDOM);
        break;

    case 17:
        P_SpawnFireFlicker(sector);
        break;

    case 202:
        P_SpawnStrobeAltFlash(sector, 3);
        break;

    case 204:
        P_SpawnStrobeFlash(sector, 7);
        break;

    case 205:
        P_SpawnSequenceLight(sector, true);
        break;

    case 206:
        P_SpawnStrobeFlash(sector, 90);
        break;

    case 208:
        P_SpawnStrobeAltFlash(sector, 6);
        break;

    case 666:
        break;
    }
}

/*
==============================================================================
=
= P_UseSpecialLine
=
= Called when a thing uses a special line
= Only the front sides of lines are usable
===============================================================================
*/

boolean P_UseSpecialLine (line_t *line, mobj_t *thing) // 800204BC
{
    player_t    *player;
    boolean     ok;
    int         actionType;

    actionType = SPECIALMASK(line->special);

    if(actionType == 0)
        return false;


    player = thing->player;

	/* */
	/*	Switches that other things can activate */
	/* */
	if (!player)
	{
	    /* Missiles should NOT trigger specials... */
        if(thing->flags & MF_MISSILE)
            return false;

        if(!(line->flags & ML_THINGTRIGGER))
        {
            /* never open secret doors */
            if (line->flags & ML_SECRET)
                return false;

            /* never allow a non-player mobj to use lines with these useflags */
            if (line->special & (MLU_BLUE|MLU_YELLOW|MLU_RED))
                return false;

            /*
                actionType == 1 // MANUAL DOOR RAISE
                actionType == 2 // OPEN DOOR IMPACT
                actionType == 4 // RAISE DOOR
                actionType == 10 // PLAT DOWN-WAIT-UP-STAY TRIGGER
                actionType == 39 // TELEPORT TRIGGER
                actionType == 125 // TELEPORT MONSTERONLY TRIGGER
            */

            if (!((line->special & MLU_USE   && actionType == 1) ||
                  (line->special & MLU_CROSS &&(actionType == 4 || actionType == 10 || actionType == 39 || actionType == 125)) ||
                  (line->special & MLU_SHOOT && actionType == 2)))
                return false;
        }
	}
    else
    {
        if(line->special & MLU_BLUE) /* Blue Card Lock */
        {
            if(!player->cards[it_bluecard] && !player->cards[it_blueskull])
            {
                player->message = "You need a blue key.";
                player->messagetic = MSGTICS;
                S_StartSound(thing, sfx_oof);

                if (player == &players[0])
                    tryopen[MapBlueKeyType] = true;

                return true;
            }
        }

        if(line->special & MLU_YELLOW) /* Yellow Card Lock */
        {
            if(!player->cards[it_yellowcard] && !player->cards[it_yellowskull])
            {
                player->message = "You need a yellow key.";
                player->messagetic = MSGTICS;
                S_StartSound(thing, sfx_oof);

                if (player == &players[0])
                    tryopen[MapYellowKeyType] = true;

                return true;
            }
        }

        if(line->special & MLU_RED) /* Red Card Lock */
        {
            if(!player->cards[it_redcard] && !player->cards[it_redskull])
            {
                player->message = "You need a red key.";
                player->messagetic = MSGTICS;
                S_StartSound(thing, sfx_oof);   // ?? line missing on Doom64

                if (player == &players[0])
                    tryopen[MapRedKeyType] = true;

                return true;
            }
        }

        /*
            actionType == 90 // ARTIFACT SWITCH 1
            actionType == 91 // ARTIFACT SWITCH 2
            actionType == 92 // ARTIFACT SWITCH 3
        */

        if ((actionType == 90 || actionType == 91 || actionType == 92) &&
           !((player->artifacts & 1) << ((actionType + 6) & 0x1f)))
        {
            player->message = "You lack the ability to activate it.";
            player->messagetic = MSGTICS;
            S_StartSound(thing, sfx_oof);

            return false;
        }
    }

    if(actionType >= 256)
        return P_StartMacro(actionType, line, thing);

    ok = false;

	/* */
	/* do something */
	/*	*/
	switch(SPECIALMASK(line->special))
	{
        case 1:			/* Vertical Door */
		case 31:		/* Manual Door Open */
		case 117:		/* Blazing Door Raise */
		case 118:		/* Blazing Door Open */
            EV_VerticalDoor(line, thing);
            ok = true;
			break;
        case 2:			/* Open Door */
			ok = EV_DoDoor(line, DoorOpen);
			break;
		case 3:			/* Close Door */
			ok = EV_DoDoor(line, DoorClose);
			break;
		case 4:			/* Raise Door */
			ok = EV_DoDoor(line, Normal);
			break;
		case 5:			/* Raise Floor */
			ok = EV_DoFloor(line, raiseFloor, FLOORSPEED);
			break;
		case 6:			/* Fast Ceiling Crush & Raise */
			ok = EV_DoCeiling(line, fastCrushAndRaise, CEILSPEED*2);
			break;
		case 8:			/* Build Stairs */
			ok = EV_BuildStairs(line, build8);
			break;
		case 10:		/* PlatDownWaitUp */
			ok = EV_DoPlat(line, downWaitUpStay, 0);
			break;
        case 16:		/* Close Door 30 */
			ok = EV_DoDoor(line, Close30ThenOpen);
			break;
		case 17:		/* Start Light Strobing */
			ok = EV_StartLightStrobing(line);
			break;
		case 19:		/* Lower Floor */
			ok = EV_DoFloor(line, lowerFloor, FLOORSPEED);
			break;
		case 22:		/* Raise floor to nearest height and change texture */
			ok = EV_DoPlat(line, raiseToNearestAndChange, 0);
			break;
		case 25:		/* Ceiling Crush and Raise */
			ok = EV_DoCeiling(line, crushAndRaise, CEILSPEED);
			break;
		case 30:		/* Raise floor to shortest texture height on either side of lines */
			ok = EV_DoFloor(line, raiseToTexture, FLOORSPEED);
			break;
		case 36:		/* Lower Floor (TURBO) */
			ok = EV_DoFloor(line, turboLower, FLOORSPEED * 4);
			break;
		case 37:		/* LowerAndChange */
			ok = EV_DoFloor(line, lowerAndChange, FLOORSPEED);
			break;
		case 38:		/* Lower Floor To Lowest */
			ok = EV_DoFloor(line, lowerFloorToLowest, FLOORSPEED);
			break;
		case 39:		/* TELEPORT! */
			EV_Teleport(line, thing);
			ok = false;
			break;
        case 43:		/* Lower Ceiling to Floor */
			ok = EV_DoCeiling(line, lowerToFloor, CEILSPEED);
			break;
        case 44:		/* Ceiling Crush */
			ok = EV_DoCeiling(line, lowerAndCrush, CEILSPEED);
			break;
        case 52:		/* EXIT! */
			P_ExitLevel();//G_ExitLevel
			ok = true;
			break;
        case 53:		/* Perpetual Platform Raise */
			ok = EV_DoPlat(line, perpetualRaise, 0);
			break;
        case 54:		/* Platform Stop */
			ok = EV_StopPlat(line);
			break;
        case 56:		/* Raise Floor Crush */
			ok = EV_DoFloor(line, raiseFloorCrush, FLOORSPEED);
			break;
        case 57:		/* Ceiling Crush Stop */
			ok = EV_CeilingCrushStop(line);
			break;
        case 58:		/* Raise Floor 24 */
			ok = EV_DoFloor(line, raiseFloor24, FLOORSPEED);
			break;
		case 59:		/* Raise Floor 24 And Change */
			ok = EV_DoFloor(line, raiseFloor24AndChange, FLOORSPEED);
			break;
        case 66:		/* Raise Floor 24 and change texture */
			ok = EV_DoPlat(line, raiseAndChange, 24);
			break;
        case 67:		/* Raise Floor 32 and change texture */
			ok = EV_DoPlat(line, raiseAndChange, 32);
			break;
        case 90:        /* Artifact Switch 1 */
        case 91:        /* Artifact Switch 2 */
        case 92:        /* Artifact Switch 3 */
            ok = P_ActivateLineByTag(line->tag + 1, thing);
            break;
        case 93:        /* Modify mobj flags */
            ok = P_ModifyMobjFlags(line->tag, MF_NOINFIGHTING);
            break;
        case 94:        /* Noise Alert */
            ok = P_AlertTaggedMobj(line->tag, thing);
            break;
        case 100:		/* Build Stairs Turbo 16 */
			ok = EV_BuildStairs(line, turbo16);
			break;
        case 108:		/* Blazing Door Raise (faster than TURBO!) */
			ok = EV_DoDoor(line, BlazeRaise);
			break;
        case 109:		/* Blazing Door Open (faster than TURBO!) */
			ok = EV_DoDoor(line, BlazeOpen);
			break;
        case 110:		/* Blazing Door Close (faster than TURBO!) */
			ok = EV_DoDoor(line, BlazeClose);
			break;
        case 119:		/* Raise floor to nearest surr. floor */
			ok = EV_DoFloor(line, raiseFloorToNearest, FLOORSPEED);
			break;
        case 121:		/* Blazing PlatDownWaitUpStay */
			ok = EV_DoPlat(line, blazeDWUS, 0);
			break;
        case 122:		/* PlatDownWaitUpStay */
			ok = EV_DoPlat(line, upWaitDownStay, 0);
			break;
        case 123:		/* Blazing PlatDownWaitUpStay */
			ok = EV_DoPlat(line, blazeDWUS, 0);
			break;
        case 124:		/* Secret EXIT */
			P_SecretExitLevel(line->tag);//(G_SecretExitLevel)
			ok = true;
			break;
        case 125:		/* TELEPORT MonsterONLY */
			if (!thing->player)
			{
				EV_Teleport(line, thing);
				ok = false;
			}
			break;
        case 141:		/* Silent Ceiling Crush & Raise (Demon Disposer)*/
			ok = EV_DoCeiling(line, silentCrushAndRaise, CEILSPEED*2);
			break;
        case 200:       /* Set Lookat Camera */
            ok = P_SetAimCamera(line, true);
            break;
        case 201:       /* Set Camera */
            ok = P_SetAimCamera(line, false);
            break;
        case 202:       /* Invoke Dart */
            ok = EV_SpawnTrapMissile(line, thing, MT_PROJ_DART);
            break;
        case 203:       /* Delay Thinker */
            P_SpawnDelayTimer(line->tag, NULL);
            ok = true;
            break;
        case 204:       /* Set global integer */
            macrointeger = line->tag;
            ok = true;
            break;
        case 205:       /* Modify sector color */
            P_ModifySectorColor(line, LIGHT_FLOOR, macrointeger);
            ok = true;
            break;
        case 206:       /* Modify sector color */
            ok = P_ModifySectorColor(line, LIGHT_CEILING, macrointeger);
            break;
        case 207:       /* Modify sector color */
            ok = P_ModifySectorColor(line, LIGHT_THING, macrointeger);
            break;
        case 208:       /* Modify sector color */
            ok = P_ModifySectorColor(line, LIGHT_UPRWALL, macrointeger);
            break;
        case 209:       /* Modify sector color */
            ok = P_ModifySectorColor(line, LIGHT_LWRWALL, macrointeger);
            break;
        case 210:       /* Modify sector ceiling height */
            ok = EV_DoCeiling(line, customCeiling, CEILSPEED);
            break;
        case 212:       /* Modify sector floor height */
            ok = EV_DoFloor(line, customFloor, FLOORSPEED);
            break;
        case 214:       /* Elevator Sector */
            ok = EV_SplitSector(line, true);
            break;
        case 218:       /* Modify Line Flags */
            ok = P_ModifyLineFlags(line, macrointeger);
            break;
        case 219:       /* Modify Line Texture */
            ok = P_ModifyLineTexture(line, macrointeger);
            break;
        case 220:       /* Modify Sector Flags */
            ok = P_ModifySector(line, macrointeger, mods_flags);
            break;
        case 221:       /* Modify Sector Specials */
            ok = P_ModifySector(line, macrointeger, mods_special);
            break;
        case 222:       /* Modify Sector Lights */
            ok = P_ModifySector(line, macrointeger, mods_lights);
            break;
        case 223:       /* Modify Sector Flats */
            ok = P_ModifySector(line, macrointeger, mods_flats);
            break;
        case 224:       /* Spawn Thing */
            ok = EV_SpawnMobjTemplate(line->tag);
            break;
        case 225:       /* Quake Effect */
            P_SpawnQuake(line->tag);
            ok = true;
            break;
        case 226:       /* Modify sector ceiling height */
            ok = EV_DoCeiling(line, customCeiling, CEILSPEED * 4);
            break;
        case 227:       /* Modify sector ceiling height */
            ok = EV_DoCeiling(line, customCeiling, 4096 * FRACUNIT);
            break;
        case 228:       /* Modify sector floor height */
            ok = EV_DoFloor(line, customFloor, FLOORSPEED * 4);
            break;
        case 229:       /* Modify sector floor height */
            ok = EV_DoFloor(line, customFloor, 4096 * FRACUNIT);
            break;
        case 230:       /* Modify Line Special */
            ok = P_ModifyLineData(line, macrointeger);
            break;
        case 231:       /* Invoke Revenant Missile */
            ok = EV_SpawnTrapMissile(line, thing, MT_PROJ_TRACER);
            break;
        case 232:       /* Fast Ceiling Crush & Raise */
            ok = EV_DoCeiling(line, crushAndRaiseOnce, CEILSPEED * 4);
            break;
        case 233:       /* Freeze Player */
            thing->reactiontime = line->tag;
            ok = true;
            break;
        case 234:      /* Change light by light tag */
            ok = P_ChangeLightByTag(macrointeger, line->tag);
            break;
        case 235:       /* Modify Light Data */
            ok = P_DoSectorLightChange(macrointeger, line->tag);
            break;
        case 236:       /* Modify platform */
            ok = EV_DoPlat(line, customDownUp, 0);
            break;
        case 237:       /* Modify platform */
            ok = EV_DoPlat(line, customDownUpFast,0);
            break;
        case 238:       /* Modify platform */
            ok = EV_DoPlat(line,customUpDown,0);
            break;
        case 239:       /* Modify platform */
            ok = EV_DoPlat(line,customUpDownFast,0);
            break;
        case 240:       /* Execute random line trigger */
            ok = P_RandomLineTrigger(line, thing);
            break;
        case 241:       /* Split Open Sector */
            ok = EV_SplitSector(line, false);
            break;
        case 242:       /* Fade Out Thing */
            ok = EV_FadeOutMobj(line->tag);
            break;
        case 243:       /* Move and Aim Camera */
            P_SetMovingCamera(line);
            ok = false;
            break;
        case 244:       /* Modify Sector Floor */
            ok = EV_DoFloor(line, customFloorToHeight, 4096 * FRACUNIT);
            break;
        case 245:       /* Modify Sector Ceiling */
            ok = EV_DoCeiling(line, customCeilingToHeight, 4096 * FRACUNIT);
            break;
        case 246:       /* Restart Macro at ID */
            P_RestartMacro(line, macrointeger);
            ok = false;
            break;
        case 247:       /* Modify Sector Floor */
            ok = EV_DoFloor(line, customFloorToHeight, FLOORSPEED);
            break;
        case 248:       /* Suspend a macro script */
            ok = P_SuspendMacro();
            break;
        case 249:       /* Silent Teleport */
            ok = EV_SilentTeleport(line, thing);
            break;
        case 250:       /* Toggle macros on */
            P_ToggleMacros(line->tag, true);
            ok = true;
            break;
        case 251:       /* Toggle macros off */
            P_ToggleMacros(line->tag, false);
            ok = true;
            break;
        case 252:       /* Modify Sector Ceiling */
            ok = EV_DoCeiling(line, customCeilingToHeight, CEILSPEED);
            break;
        case 253:       /* Unlock Cheat Menu */
            if(!demoplayback) {
                FeaturesUnlocked = true;
            }
            ok = true;
            break;
        case 254:       /* D64 Map33 Logo */
            Skyfadeback = true;
            break;

        default:
            return false;
	}

	if (ok)
    {
        if (line == &macrotempline)
            return true;

        P_ChangeSwitchTexture(line, line->special & MLU_REPEAT);

        if (line->special & MLU_REPEAT)
            return true;

        line->special = 0;
    }

    return true;
}

#if 0

/*============================================================ */
/* */
/*	Special Stuff that can't be categorized */
/* */
/*============================================================ */
int EV_DoDonut(line_t *line)//L8002796C()
{
	sector_t	*s1;
	sector_t	*s2;
	sector_t	*s3;
	int			secnum;
	int			rtn;
	int			i;
	floormove_t		*floor;

	secnum = -1;
	rtn = 0;
	while ((secnum = P_FindSectorFromLineTag(line->tag,secnum)) >= 0)
	{
		s1 = &sectors[secnum];

		/*	ALREADY MOVING?  IF SO, KEEP GOING... */
		if (s1->specialdata)
			continue;

		rtn = 1;
		s2 = getNextSector(s1->lines[0],s1);
		for (i = 0;i < s2->linecount;i++)
		{
			if (//(!s2->lines[i]->flags & ML_TWOSIDED) ||
				(s2->lines[i]->backsector == s1))
				continue;
			s3 = s2->lines[i]->backsector;

			/* */
			/*	Spawn rising slime */
			/* */
			floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
			P_AddThinker (&floor->thinker);
			s2->specialdata = floor;
			floor->thinker.function = T_MoveFloor;
			floor->type = donutRaise;
			floor->crush = false;
			floor->direction = 1;
			floor->sector = s2;
			floor->speed = FLOORSPEED / 2;
			floor->texture = s3->floorpic;
			floor->newspecial = 0;
			floor->floordestheight = s3->floorheight;

			/* */
			/*	Spawn lowering donut-hole */
			/* */
			floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
			P_AddThinker (&floor->thinker);
			s1->specialdata = floor;
			floor->thinker.function = T_MoveFloor;
			floor->type = lowerFloor;
			floor->crush = false;
			floor->direction = -1;
			floor->sector = s1;
			floor->speed = FLOORSPEED / 2;
			floor->floordestheight = s3->floorheight;
			break;
		}
	}
	return rtn;
}

#endif // 0
