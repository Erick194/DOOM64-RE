/* P_enemy.c */

#include "doomdef.h"
#include "p_local.h"

void A_Fall (mobj_t *actor);

typedef enum
{
	DP_STRAIGHT,
    DP_LEFT,
    DP_RIGHT
} dirproj_e;

mobj_t* P_MissileAttack(mobj_t *actor, dirproj_e direction);

/*
===============================================================================

							ENEMY THINKING

enemies are allways spawned with targetplayer = -1, threshold = 0
Most monsters are spawned unaware of all players, but some can be made preaware

===============================================================================
*/

/*
================
=
= P_CheckMeleeRange
=
================
*/

boolean P_CheckMeleeRange (mobj_t *actor) // 80010B90
{
	mobj_t		*pl;
	fixed_t		dist;

	if (!(actor->flags&MF_SEETARGET))
		return false;

	if (!actor->target)
		return false;

	pl = actor->target;
	dist = P_AproxDistance (pl->x-actor->x, pl->y-actor->y);
	if (dist >= MELEERANGE)
		return false;

	return true;
}

/*
================
=
= P_CheckMissileRange
=
================
*/

boolean P_CheckMissileRange (mobj_t *actor) // 80010C10
{
	fixed_t		dist;

	if (!(actor->flags & MF_SEETARGET))
		return false;

	if (actor->flags & MF_JUSTHIT)
	{	/* the target just hit the enemy, so fight back! */
		actor->flags &= ~MF_JUSTHIT;
		return true;
	}

	if (actor->reactiontime)
		return false;		/* don't attack yet */

	dist = P_AproxDistance ( actor->x-actor->target->x, actor->y-actor->target->y) - 64*FRACUNIT;
	if (!actor->info->meleestate)
		dist -= 128*FRACUNIT;		/* no melee attack, so fire more */

	dist >>= 16;

	if (actor->type == MT_SKULL)
		dist >>= 1;

	if (dist > 200)
		dist = 200;

	if (P_Random() < dist)
		return false;

	return true;
}


/*
================
=
= P_Move
=
= Move in the current direction
= returns false if the move is blocked
================
*/

fixed_t	xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};// 8005ACC0
fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};// 8005ACE0

extern	line_t *blockline; // 800A5D9C

boolean P_Move (mobj_t *actor) // 80010D08
{
	fixed_t	tryx, tryy;
	boolean		good;
	line_t		*blkline;

	if (actor->movedir == DI_NODIR)
		return false;

    if((actor->flags & MF_GRAVITY) != 0)
    {
        if(actor->floorz != actor->z)
        {
            return false;
        }
    }

	tryx = actor->x + actor->info->speed * xspeed[actor->movedir];
	tryy = actor->y + actor->info->speed * yspeed[actor->movedir];

	if (!P_TryMove (actor, tryx, tryy) )
	{	/* open any specials */
		if (actor->flags & MF_FLOAT && floatok)
		{	/* must adjust height */
			if (actor->z < tmfloorz)
				actor->z += FLOATSPEED;
			else
				actor->z -= FLOATSPEED;

			//actor->flags |= MF_INFLOAT;
			return true;
		}

		blkline = blockline;//(line_t *)DSPRead (&blockline);
		if (!blkline || !blkline->special)
			return false;

		actor->movedir = DI_NODIR;
		good = false;

        if(blockline->special & MLU_USE)
            good = P_UseSpecialLine (blkline, actor);

		return good;
	}
	//else
	//	actor->flags &= ~MF_INFLOAT;

	//if (!(actor->flags & MF_FLOAT))
	//	actor->z = actor->floorz;

	return true;
}


/*
==================================
=
= TryWalk
=
= Attempts to move actoron in its current (ob->moveangle) direction.
=
= If blocked by either a wall or an actor returns FALSE
= If move is either clear or blocked only by a door, returns TRUE and sets
= If a door is in the way, an OpenDoor call is made to start it opening.
=
==================================
*/

boolean P_TryWalk (mobj_t *actor) // 80010E88
{
	if (!P_Move (actor))
		return false;

	actor->movecount = P_Random()&7;
	return true;
}


/*
================
=
= P_NewChaseDir
=
================
*/

dirtype_t opposite[] = // 8005AD00
{DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST, DI_EAST, DI_NORTHEAST,
DI_NORTH, DI_NORTHWEST, DI_NODIR};

dirtype_t diags[] = {DI_NORTHWEST,DI_NORTHEAST,DI_SOUTHWEST,DI_SOUTHEAST}; // 8005AD24

void P_NewChaseDir (mobj_t *actor) // 80010ED0
{
	fixed_t		deltax,deltay;
	dirtype_t	d[3];
	dirtype_t	tdir, olddir, turnaround;

	if (!actor->target)
		I_Error ("P_NewChaseDir: called with no target");

	olddir = actor->movedir;
	turnaround=opposite[olddir];

	deltax = actor->target->x - actor->x;
	deltay = actor->target->y - actor->y;

	if (deltax>10*FRACUNIT)
		d[1]= DI_EAST;
	else if (deltax<-10*FRACUNIT)
		d[1]= DI_WEST;
	else
		d[1]=DI_NODIR;

	if (deltay<-10*FRACUNIT)
		d[2]= DI_SOUTH;
	else if (deltay>10*FRACUNIT)
		d[2]= DI_NORTH;
	else
		d[2]=DI_NODIR;

/* try direct route */
	if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	{
		actor->movedir = diags[((deltay<0)<<1)+(deltax>0)];
		if (actor->movedir != turnaround && P_TryWalk(actor))
			return;
	}

/* try other directions */
	if (P_Random() > 200 ||  abs(deltay)>abs(deltax))
	{
		tdir=d[1];
		d[1]=d[2];
		d[2]=tdir;
	}

	if (d[1]==turnaround)
		d[1]=DI_NODIR;

	if (d[2]==turnaround)
		d[2]=DI_NODIR;

	if (d[1]!=DI_NODIR)
	{
		actor->movedir = d[1];
		if (P_TryWalk(actor))
			return;     /*either moved forward or attacked*/
	}

	if (d[2]!=DI_NODIR)
	{
		actor->movedir =d[2];
		if (P_TryWalk(actor))
			return;
	}

/* there is no direct path to the player, so pick another direction */

	if (olddir!=DI_NODIR)
	{
		actor->movedir =olddir;
		if (P_TryWalk(actor))
			return;
	}

	if (P_Random()&1) 	/*randomly determine direction of search*/
	{
		for (tdir=DI_EAST ; tdir<=DI_SOUTHEAST ; tdir++)
		{
			if (tdir!=turnaround)
			{
				actor->movedir =tdir;
				if ( P_TryWalk(actor) )
					return;
			}
		}
	}
	else
	{
		for (tdir=DI_SOUTHEAST ; (int)tdir >= (int)DI_EAST;tdir--)
		{
			if (tdir!=turnaround)
			{
				actor->movedir =tdir;
				if ( P_TryWalk(actor) )
				return;
			}
		}
	}

	if (turnaround !=  DI_NODIR)
	{
		actor->movedir =turnaround;
		if ( P_TryWalk(actor) )
			return;
	}

	actor->movedir = DI_NODIR;		/* can't move */
}


/*
================
=
= P_LookForPlayers
=
= If allaround is false, only look 180 degrees in front
= returns true if a player is targeted
================
*/

boolean P_LookForPlayers (mobj_t *actor, boolean allaround) // 8001115C
{
	angle_t		an;
	fixed_t		dist, dist2;
	mobj_t		*mo;
	mobj_t		*mobj;

	mo = actor->target;
	if (!mo || mo->health <= 0 || !(actor->flags & MF_SEETARGET))
	{
		if(actor->type > MT_PLAYERBOT3)   /* for monsters */
        {
            actor->target = players[0].mo;
        }
        else /* special case for player bots */
        {
            dist2 = MAXINT;

            for(mobj = mobjhead.next; mobj != &mobjhead; mobj = mobj->next)
            {

                if((mobj->flags & MF_COUNTKILL) == 0 ||
                     mobj->type <= MT_PLAYERBOT3 ||
                     mobj->health <= 0           ||
                     mobj == actor)
                {
                    continue;
                }

                /* find a killable target as close as possible */
                dist = P_AproxDistance(mobj->x - actor->x, mobj->y - actor->y);
                if(!(dist < dist2))
                    continue;

                actor->target = mobj;
                dist2 = dist;
            }
        }

		return false;
	}

	if (!actor->subsector->sector->soundtarget)
    {
        if (!allaround)
        {
            an = R_PointToAngle2 (actor->x, actor->y, mo->x, mo->y) - actor->angle;
            if (an > ANG90 && an < ANG270)
            {
                dist = P_AproxDistance (mo->x - actor->x, mo->y - actor->y);
                /* if real close, react anyway */
                if (dist > MELEERANGE)
                    return false;		/* behind back */
            }
        }
    }

	return true;
}


/*
===============================================================================

						ACTION ROUTINES

===============================================================================
*/

/*
==============
=
= A_Look
=
= Stay in state until a player is sighted
=
==============
*/

void A_Look (mobj_t *actor) // 80011340
{
	mobj_t	*targ;
	int		sound;

	/* if current target is visible, start attacking */
	if (!P_LookForPlayers(actor, false))
	{
		/* if the sector has a living soundtarget, make that the new target */
		actor->threshold = 0;		/* any shot will wake up */
		targ = actor->subsector->sector->soundtarget;
		if (targ == NULL || !(targ->flags & MF_SHOOTABLE) || (actor->flags & MF_AMBUSH))
			return;
		actor->target = targ;
	}

	/* go into chase state */
	if (actor->info->seesound)
	{
		switch (actor->info->seesound)
		{
		case sfx_possit1://sfx_posit1:
		case sfx_possit2://sfx_posit2:
		case sfx_possit3://sfx_posit3:
			sound = sfx_possit1+(P_Random()&1);
			break;
		case sfx_impsit1://sfx_bgsit1:
		case sfx_impsit2://sfx_bgsit2:
			sound = sfx_impsit1+(P_Random()&1);
			break;
		default:
			sound = actor->info->seesound;
			break;
		}

		if (actor->type == MT_RESURRECTOR || actor->type == MT_CYBORG)
			S_StartSound(NULL, sound);	// full volume
		else
			S_StartSound(actor, sound);
	}

	P_SetMobjState (actor, actor->info->seestate);
}


/*
==============
=
= A_Chase
=
= Actor has a melee attack, so it tries to close as fast as possible
=
==============
*/

void A_Chase (mobj_t *actor) // 8001146C
{
	int		delta;

	if (actor->reactiontime)
		actor->reactiontime--;

	/* */
	/* modify target threshold */
	/* */
	if (actor->threshold)
		actor->threshold--;

	/* */
	/* turn towards movement direction if not there yet */
	/* */
	if (actor->movedir < 8)
	{
		actor->angle &= (7<<29);
		delta = actor->angle - (actor->movedir << 29);
		if (delta > 0)
			actor->angle -= ANG90/2;
		else if (delta < 0)
			actor->angle += ANG90/2;
	}

	if (!actor->target || !(actor->target->flags&MF_SHOOTABLE))
	{	/* look for a new target */
		if (P_LookForPlayers(actor,true))
			return;		/* got a new target */
		P_SetMobjState (actor, actor->info->spawnstate);
		return;
	}

	/* */
	/* don't attack twice in a row */
	/* */
	if (actor->flags & MF_JUSTATTACKED)
	{
		actor->flags &= ~MF_JUSTATTACKED;
		P_NewChaseDir (actor);
		return;
	}

	/* */
	/* check for melee attack */
	/* */
	if (actor->info->meleestate && P_CheckMeleeRange (actor))
	{
		if (actor->info->attacksound)
			S_StartSound (actor, actor->info->attacksound);
		P_SetMobjState (actor, actor->info->meleestate);
		return;
	}

	/* */
	/* check for missile attack */
	/* */
	if ( (/*gameskill == sk_nightmare || */!actor->movecount) && actor->info->missilestate
	&& P_CheckMissileRange (actor))
	{
		P_SetMobjState (actor, actor->info->missilestate);
		//if (gameskill != sk_nightmare)
			actor->flags |= MF_JUSTATTACKED;
		return;
	}

	/* */
	/* chase towards player */
	/* */
	if (--actor->movecount<0 || !P_Move (actor))
		P_NewChaseDir (actor);

	/* */
	/* make active sound */
	/* */
	if (actor->info->activesound && P_Random () < 3)
		S_StartSound (actor, actor->info->activesound);
}

/*============================================================================= */

/*
==============
=
= A_FaceTarget
=
==============
*/

void A_FaceTarget (mobj_t *actor) // 800116A8
{
    int rnd1, rnd2;
	if (!actor->target)
		return;

	actor->flags &= ~MF_AMBUSH;
	actor->angle = R_PointToAngle2 (actor->x, actor->y , actor->target->x, actor->target->y);

	if (actor->target->flags & MF_SHADOW)
    {
        rnd1 = P_Random();
        rnd2 = P_Random();
		actor->angle += (rnd2 - rnd1) << 21;
    }
}

/*
==============
=
= A_Scream
=
==============
*/

void A_Scream (mobj_t *actor) // 80011740
{
	int		sound;

	switch (actor->info->deathsound)
	{
	case 0:
		return;

	case sfx_posdie1://sfx_podth1:
	case sfx_posdie2://sfx_podth2:
	case sfx_posdie3://sfx_podth3:
		sound = sfx_posdie1 + (P_Random ()&1);
		break;

	case sfx_impdth1://sfx_bgdth1:
	case sfx_impdth2://sfx_bgdth2:
		sound = sfx_impdth1 + (P_Random ()&1);
		break;

	default:
		sound = actor->info->deathsound;
		break;
	}

    S_StartSound(actor, sound);
}

/*
==============
=
= A_XScream
=
==============
*/

void A_XScream (mobj_t *actor) // 800117E4
{
	S_StartSound (actor, sfx_slop);
}

/*
==============
=
= A_Pain
=
==============
*/

void A_Pain (mobj_t *actor) // 80011804
{
    if(actor->info->painsound)
    {
        if(actor->type == MT_RESURRECTOR)
            S_StartSound(NULL, actor->info->painsound);
        else
            S_StartSound(actor, actor->info->painsound);
    }
}

/*
==============
=
= A_Fall
=
==============
*/

void A_Fall (mobj_t *actor) // 8001185C
{
/* actor is on ground, it can be walked over */
	actor->flags &= ~MF_SOLID;
}


/*
================
=
= A_Explode
=
================
*/

void A_Explode (mobj_t *thingy) // 80011870
{
	P_RadiusAttack(thingy, thingy->target, 128);
}


/*
================
=
= A_OnDeathTrigger(A_BossDeath)
=
= Possibly trigger special effects
================
*/

void A_OnDeathTrigger (mobj_t *mo) // 80011894
{
	mobj_t		*mo2;

    if(!(mo->flags & MF_TRIGDEATH))
        return;

    for(mo2 = mobjhead.next; mo2 != &mobjhead; mo2 = mo2->next)
    {
        if((mo2->tid == mo->tid) && (mo2->health > 0))
            return;
    }

    if(!P_ActivateLineByTag(mo->tid, mo))
    {
        macroqueue[macroidx1].activator = mo;
        macroqueue[macroidx1].tag = mo->tid;
        macroidx1 = (macroidx1 + 1) & 3;
    }
}

/*
==============
=
= A_PosAttack
=
==============
*/

void A_PosAttack (mobj_t *actor) // 80011954
{
	int		angle, damage, rnd1, rnd2;

	if (!actor->target)
		return;

	A_FaceTarget (actor);
	angle = actor->angle;

	S_StartSound (actor, sfx_pistol);

    rnd1 = P_Random();
    rnd2 = P_Random();
	angle += (rnd2-rnd1)<<20;

	damage = ((P_Random() & 7) * 3) + 3;
	P_LineAttack (actor, angle, 0, MISSILERANGE, MAXINT, damage);
}

/*
==============
=
= A_SPosAttack
=
==============
*/

void A_SPosAttack (mobj_t *actor) // 800119FC
{
	int		i;
	int		angle, bangle, damage;

	if (!actor->target)
		return;

	S_StartSound (actor, sfx_shotgun);
	A_FaceTarget (actor);
	bangle = actor->angle;

	for (i=0 ; i<3 ; i++)
	{
		angle = bangle + ((P_Random()-P_Random())<<20);
		damage = ((P_Random() % 5) * 3) + 3;
		P_LineAttack (actor, angle, 0, MISSILERANGE, MAXINT, damage);
	}
}

/*
==============
=
= A_PlayAttack(A_CPosAttack)
=
==============
*/

void A_PlayAttack(mobj_t* actor) // 80011b1C
{
	int		angle;
	int		bangle;
	int		damage;
	int		slope;

	if (!actor->target)
		return;

	S_StartSound(actor, sfx_pistol);
	A_FaceTarget(actor);
	bangle = actor->angle;

	slope = P_AimLineAttack(actor, bangle, 0, MISSILERANGE);

	angle = bangle + ((P_Random() - P_Random()) << 20);
	damage = ((P_Random() % 5) * 3) + 3;
	P_LineAttack(actor, angle, 0, MISSILERANGE, slope, damage);
}

/*
==============
=
= A_CPosRefire
=
==============
*/

void A_CPosRefire(mobj_t* actor) // 80011BD4
{
	A_FaceTarget(actor);

	if (P_Random() < 40)
		return;

	if (!actor->target || actor->target->health <= 0
		|| !P_CheckSight(actor, actor->target))
	{
		P_SetMobjState(actor, actor->info->seestate);
	}
}

/*
==============
=
= A_BspiFaceTarget
=
==============
*/

void A_BspiFaceTarget(mobj_t *actor) // 80011C50
{
	A_FaceTarget(actor);
    actor->extradata = (int *)5;
}

/*
==============
=
= A_BspiAttack
=
==============
*/

void A_BspiAttack(mobj_t *actor) // 80011C74
{
	if (!actor->target)
		return;

	A_FaceTarget(actor);

/* */
/* launch a missile */
/* */
	P_MissileAttack(actor, DP_LEFT);
    P_MissileAttack(actor, DP_RIGHT);
}

/*
==============
=
= A_SpidRefire
=
==============
*/

void A_SpidRefire (mobj_t *actor) // 80011CBC
{
	A_FaceTarget (actor);

	if (P_Random () < 10)
		return;

	if (!actor->target || actor->target->health <= 0 || !(actor->flags&MF_SEETARGET) )
    {
		P_SetMobjState (actor, actor->info->seestate);
		actor->extradata = (int *)5;
		return;
    }

    if(--actor->extradata <= 0)
    {
        P_SetMobjState(actor, actor->info->missilestate);
        actor->extradata = (int *)5;
    }
}

/*
==============
=
= A_TroopMelee
=
==============
*/

void A_TroopMelee(mobj_t* actor) // 80011D78
{
    int    damage;

    if(!actor->target)
        return;

    A_FaceTarget (actor);
    if(P_CheckMeleeRange(actor))
    {
        S_StartSound(actor, sfx_scratch);
        damage = ((P_Random() & 7) * 3) + 3;
        P_DamageMobj(actor->target, actor, actor, damage);
    }
}

/*
==============
=
= A_TroopAttack
=
==============
*/

void A_TroopAttack (mobj_t *actor) // 80011DEC
{
	int		damage;

	if (!actor->target)
		return;

    A_FaceTarget (actor);

/* */
/* launch a missile */
/* */
	P_MissileAttack(actor, DP_STRAIGHT);
}

/*
==============
=
= A_SargAttack
=
==============
*/

void A_SargAttack (mobj_t *actor) // 80011E28
{
	int		damage;

	if (!actor->target)
		return;

	A_FaceTarget (actor);
	if(P_CheckMeleeRange(actor))
    {
        damage = ((P_Random() & 7) * 4) + 4;
        P_DamageMobj(actor->target, actor, actor, damage);
    }
}

/*
==============
=
= A_HeadAttack
=
==============
*/

void A_HeadAttack (mobj_t *actor) // 80011E90
{
	int		damage;

	if (!actor->target)
		return;

	A_FaceTarget (actor);
	if (P_CheckMeleeRange (actor))
	{
		damage = ((P_Random() & 7) * 8) + 8;
		P_DamageMobj (actor->target, actor, actor, damage);
		return;
	}
/* */
/* launch a missile */
/* */
	P_MissileAttack(actor, DP_STRAIGHT);
}

/*
==============
=
= A_CyberAttack
=
==============
*/

void A_CyberAttack (mobj_t *actor) // 80011F08
{
	if (!actor->target)
		return;

	A_FaceTarget (actor);
	P_MissileAttack(actor, DP_LEFT);
}

/*
==============
=
= A_CyberDeathEvent
=
==============
*/

void A_CyberDeathEvent(mobj_t* actor) // 80011F44
{
    mobjexp_t *exp;

    //P_BossExplode(actor, 4, 12);
    exp = Z_Malloc (sizeof(*exp), PU_LEVSPEC, 0);
    P_AddThinker (&exp->thinker);
    exp->thinker.function = T_MobjExplode;
    exp->mobj = actor;
    exp->delay = 0;
    exp->delaydefault = 4;
    exp->lifetime = 12;
    S_StartSound(NULL, actor->info->deathsound);
}

/*
==============
=
= A_BruisAttack
=
==============
*/

void A_BruisAttack (mobj_t *actor) // 80011FC4
{
	int		damage;

	if (!actor->target)
		return;

	if (P_CheckMeleeRange (actor))
	{
		S_StartSound (actor, sfx_scratch);
		damage = ((P_Random() & 7) * 11) + 11;
		P_DamageMobj (actor->target, actor, actor, damage);
		return;
	}
/* */
/* launch a missile */
/* */
	P_MissileAttack(actor, DP_STRAIGHT);
}

/*
==============
=
= A_SpawnSmoke
=
==============
*/

void A_SpawnSmoke(mobj_t *actor) // 8001204C
{
    mobj_t *smoke;

    smoke = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SMOKE_GRAY);
    smoke->momz = FRACUNIT/2;
}

/*
==============
=
= A_Tracer
=
==============
*/

#define TRACEANGLE 0x10000000

void A_Tracer(mobj_t *actor) // 80012088
{
	angle_t	exact;
	fixed_t	dist;
	fixed_t	slope;
	mobj_t *dest;
	mobj_t *th;

    th = P_SpawnMobj(actor->x - actor->momx, actor->y - actor->momy, actor->z, MT_SMOKE_RED);

    th->momz = FRACUNIT;
    th->tics -= P_Random() & 3;

    if (th->tics < 1)
        th->tics = 1;

    if(actor->threshold-- < -100)
        return;

    // adjust direction
    dest = actor->tracer;

    if (!dest || dest->health <= 0)
        return;

    // change angle
    exact = R_PointToAngle2(actor->x, actor->y, dest->x, dest->y);

    if (exact != actor->angle)
    {
        if (exact - actor->angle > 0x80000000)
        {
            actor->angle -= TRACEANGLE;
            if (exact - actor->angle < 0x80000000)
                actor->angle = exact;
        }
        else
        {
            actor->angle += TRACEANGLE;
            if (exact - actor->angle > 0x80000000)
                actor->angle = exact;
        }
    }

    exact = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = (actor->info->speed * finecosine[exact]);
    actor->momy = (actor->info->speed * finesine[exact]);

    // change slope
    dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
    dist = dist / (actor->info->speed << FRACBITS);

    if (dist < 1)
        dist = 1;

    slope = (dest->height * 3);
    if(slope < 0) {
        slope = slope + 3;
    }

    slope = (dest->z + (slope >> 2) - actor->z) / dist;

    if (slope < actor->momz)
        actor->momz -= FRACUNIT / 8;
    else
        actor->momz += FRACUNIT / 4;
}

/*
==============
=
= A_FatRaise
=
==============
*/

#define	FATSPREAD	(ANG90/4)

void A_FatRaise(mobj_t *actor) // 800122F4
{
	A_FaceTarget(actor);
	S_StartSound(actor, sfx_fattatk);
}

/*
==============
=
= A_FatAttack1
=
==============
*/

void A_FatAttack1(mobj_t *actor) // 80012320
{
	mobj_t  *mo;
	int	    an;

	A_FaceTarget(actor);

	// Change direction  to ...
	P_MissileAttack(actor, DP_RIGHT);
	mo = P_MissileAttack(actor, DP_LEFT);

	mo->angle += FATSPREAD;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = (mo->info->speed * finecosine[an]);
	mo->momy = (mo->info->speed * finesine[an]);
}

/*
==============
=
= A_FatAttack2
=
==============
*/

void A_FatAttack2(mobj_t *actor) // 800123B0
{
    mobj_t  *mo;
	mobj_t  *target;
	int	    an;

	A_FaceTarget(actor);

	// Now here choose opposite deviation.
	P_MissileAttack(actor, DP_LEFT);
	mo = P_MissileAttack(actor, DP_RIGHT);

	mo->angle -= FATSPREAD;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = (mo->info->speed * finecosine[an]);
	mo->momy = (mo->info->speed * finesine[an]);
}

/*
==============
=
= A_FatAttack3
=
==============
*/

void A_FatAttack3(mobj_t *actor) // 80012440
{
    mobj_t  *mo;
	mobj_t  *target;
	int	    an;

	A_FaceTarget(actor);

	mo = P_MissileAttack(actor, DP_RIGHT);
	mo->angle -= FATSPREAD / 4;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = (mo->info->speed * finecosine[an]);
	mo->momy = (mo->info->speed * finesine[an]);

	mo = P_MissileAttack(actor, DP_LEFT);
	mo->angle += FATSPREAD / 4;
	an = mo->angle >> ANGLETOFINESHIFT;
	mo->momx = (mo->info->speed * finecosine[an]);
	mo->momy = (mo->info->speed * finesine[an]);
}


/*
==================
=
= SkullAttack
=
= Fly at the player like a missile
=
==================
*/

#define	SKULLSPEED		(40*FRACUNIT)

void A_SkullAttack (mobj_t *actor) // 80012528
{
	mobj_t			*dest;
	angle_t			an;
	int				dist;

	if (!actor->target)
		return;

	dest = actor->target;
	actor->flags |= MF_SKULLFLY;

	S_StartSound (actor, actor->info->attacksound);
	A_FaceTarget (actor);
	an = actor->angle >> ANGLETOFINESHIFT;
	actor->momx = (finecosine[an] * (SKULLSPEED/FRACUNIT));
	actor->momy = (finesine[an] * (SKULLSPEED/FRACUNIT));
	dist = P_AproxDistance (dest->x - actor->x, dest->y - actor->y);
	dist = dist / SKULLSPEED;
	if (dist < 1)
		dist = 1;
	actor->momz = (dest->z+(dest->height>>1) - actor->z) / dist;
}


/*
==============
=
= PIT_PainCheckLine
=
==============
*/
boolean PIT_PainCheckLine(intercept_t *in) // 80012654
{
  if (!(in->d.line->backsector))
    return false;

  return true;
}

/*
==============
=
= A_PainShootSkull
=
==============
*/

void A_PainShootSkull(mobj_t *actor, angle_t angle) // 8001267C
{
	fixed_t	x;
	fixed_t	y;
	fixed_t	z;

	mobj_t*	newmobj;
	angle_t	an;
	int		prestep;
	int		count;

	mobj_t	*mo;

	// count total number of skull currently on the level
	count = 0;
	for (mo=mobjhead.next ; mo != &mobjhead ; mo=mo->next)
	{
		if ((mo->type == MT_SKULL))
        {
            count++;

            // if there are allready 17 skulls on the level,
            // don't spit another one
            if (count >= 17)
                return;
        }
	}

	// okay, there's playe for another one
	an = angle >> ANGLETOFINESHIFT;

	prestep = (mobjinfo[MT_SKULL].radius + (4 * FRACUNIT) + actor->info->radius) >> FRACBITS;

	x = actor->x + (finecosine[an] * prestep);
	y = actor->y + (finesine[an] * prestep);
	z = actor->z + 16 * FRACUNIT;

	newmobj = P_SpawnMobj(x, y, z, MT_SKULL);

	// Check for movements.
	if (!P_PathTraverse(actor->x, actor->y, newmobj->x, newmobj->y, PT_ADDLINES, PIT_PainCheckLine) ||
        !P_TryMove(newmobj, newmobj->x, newmobj->y))
	{
		// kill it immediately
		P_DamageMobj(newmobj, actor, actor, 10000);
        P_RadiusAttack(newmobj, newmobj, 128);
		return;
	}

	newmobj->target = actor->target;
	P_SetMobjState(newmobj,newmobj->info->missilestate);
	A_SkullAttack(newmobj);
}

/*
==============
=
= A_PainAttack
=
==============
*/

void A_PainAttack(mobj_t *actor) // 80012804
{
	if (!actor->target)
		return;

	A_FaceTarget(actor);
	A_PainShootSkull(actor, actor->angle-0x15550000);
	A_PainShootSkull(actor, actor->angle+0x15550000);
}

/*
==============
=
= A_PainDie
=
==============
*/

void A_PainDie(mobj_t *actor) // 8001285C
{
	A_Fall(actor);
	A_PainShootSkull(actor, actor->angle + ANG90);
	A_PainShootSkull(actor, actor->angle + ANG180);
	A_PainShootSkull(actor, actor->angle + ANG270);

	A_OnDeathTrigger(actor);
}


/*
==============
=
= A_RectChase
=
==============
*/

void A_RectChase(mobj_t* actor) // 800128C4
{
    if(!(actor->target) || (actor->target->health <= 0) ||
       !(P_AproxDistance(actor->target->x-actor->x, actor->target->y-actor->y) < (600*FRACUNIT)))
    {
        A_Chase(actor);
        return;
    }

    A_FaceTarget(actor);
    S_StartSound(NULL, actor->info->attacksound);
    P_SetMobjState(actor, actor->info->meleestate);

}

/*
==============
=
= A_RectGroundFire
=
==============
*/

void A_RectGroundFire(mobj_t* actor) // 8001296C
{
    mobj_t* mo;
    angle_t an;

    A_FaceTarget(actor);

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_PROJ_RECTFIRE);
    mo->target = actor;
    an = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);

    mo->angle = an;
    mo->momx = finecosine[mo->angle >> ANGLETOFINESHIFT] * mo->info->speed;
    mo->momy = finesine[mo->angle >> ANGLETOFINESHIFT] * mo->info->speed;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_PROJ_RECTFIRE);
    mo->target = actor;
    mo->angle = an - ANG45;
    mo->momx = finecosine[mo->angle >> ANGLETOFINESHIFT] * mo->info->speed;
    mo->momy = finesine[mo->angle >> ANGLETOFINESHIFT] * mo->info->speed;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_PROJ_RECTFIRE);
    mo->target = actor;
    mo->angle = an + ANG45;
    mo->momx = finecosine[mo->angle >> ANGLETOFINESHIFT] * mo->info->speed;
    mo->momy = finesine[mo->angle >> ANGLETOFINESHIFT] * mo->info->speed;

    S_StartSound(mo, mo->info->seesound);
}

/*
==============
=
= A_RectMissile
=
==============
*/

void A_RectMissile(mobj_t* actor) // 80012B1C
{
    mobj_t* mo;
    int count;
    angle_t an;
    fixed_t x;
    fixed_t y;
    fixed_t speed;

    if(!actor->target)
        return;

    A_FaceTarget(actor);
    for(mo = mobjhead.next; mo != &mobjhead; mo = mo->next)
    {
        // not a rect projectile
        if(mo->type == MT_PROJ_RECT)
        {
            if((++count >= 9))
                return;
        }
    }

    // Arm 1

    an = (actor->angle/*-ANG90*/) >> ANGLETOFINESHIFT;
    x = (finesine[an] * 68);
    y = (finecosine[an] * 68);
    mo = P_SpawnMobj(actor->x + x, actor->y - y, actor->z + (68*FRACUNIT), MT_PROJ_RECT);
    mo->target = actor;
    mo->tracer = actor->target;
    mo->threshold = 5;
    an = (actor->angle + ANG270);
    mo->angle = an;
    an >>= ANGLETOFINESHIFT;
    speed = mo->info->speed >> 1;
    if(!(speed >= 0))
    {
        speed = (mo->info->speed + 1) >> 1;
    }
    mo->momx = (finecosine[an] * speed);
    mo->momy = (finesine[an] * speed);

    // Arm2

    an = (actor->angle/*-ANG90*/) >> ANGLETOFINESHIFT;
    x = (finesine[an] * 50);
    y = (finecosine[an] * 50);
    mo = P_SpawnMobj(actor->x + x, actor->y - y, actor->z + (139*FRACUNIT), MT_PROJ_RECT);
    mo->target = actor;
    mo->tracer = actor->target;
    mo->threshold = 1;
    an = (actor->angle + ANG270);
    mo->angle = an;
    an >>= ANGLETOFINESHIFT;
    speed = mo->info->speed >> 1;
    if(!(speed >= 0))
    {
        speed = (mo->info->speed + 1) >> 1;
    }
    mo->momx = (finecosine[an] * speed);
    mo->momy = (finesine[an] * speed);

    // Arm3

    an = (actor->angle/*+ANG90*/) >> ANGLETOFINESHIFT;
    x = (finesine[an] * 68);
    y = (finecosine[an] * 68);
    mo = P_SpawnMobj(actor->x - x, actor->y + y, actor->z + (68*FRACUNIT), MT_PROJ_RECT);
    mo->target = actor;
    mo->tracer = actor->target;
    mo->threshold = 5;
    an = (actor->angle - ANG270);
    mo->angle = an;
    an >>= ANGLETOFINESHIFT;
    speed = mo->info->speed >> 1;
    if(!(speed >= 0))
    {
        speed = (mo->info->speed + 1) >> 1;
    }
    mo->momx = (finecosine[an] * speed);
    mo->momy = (finesine[an] * speed);

    // Arm4

    an = (actor->angle/*+ANG90*/) >> ANGLETOFINESHIFT;
    x = (finesine[an] * 50);
    y = (finecosine[an] * 50);
    mo = P_SpawnMobj(actor->x - x, actor->y + y, actor->z + (139*FRACUNIT), MT_PROJ_RECT);
    mo->target = actor;
    mo->tracer = actor->target;
    mo->threshold = 1;
    an = (actor->angle - ANG270);
    mo->angle = an;
    an >>= ANGLETOFINESHIFT;
    speed = mo->info->speed >> 1;
    if(!(speed >= 0))
    {
        speed = (mo->info->speed + 1) >> 1;
    }
    mo->momx = (finecosine[an] * speed);
    mo->momy = (finesine[an] * speed);
}

/*
==============
=
= A_MoveGroundFire
=
==============
*/

void A_MoveGroundFire(mobj_t* fire) // 80012EA4
{
    mobj_t* mo;
    fade_t *fade;

    mo = P_SpawnMobj(fire->x, fire->y, fire->floorz, MT_PROP_FIRE);

    //P_FadeMobj(mo, -8, 0, mobjflag_t::MF_NONE);
    fade = Z_Malloc (sizeof(*fade), PU_LEVSPEC, 0);
    P_AddThinker (&fade->thinker);
    fade->thinker.function = T_FadeThinker;
    fade->amount = -8;
    fade->destAlpha = 0;
    fade->flagReserve = 0;
    fade->mobj = mo;
}

/*
==============
=
= A_RectTracer
=
==============
*/

void A_RectTracer(mobj_t* actor) // 80012F34
{
    if(actor->threshold < 0)
        A_Tracer(actor);
    else
        actor->threshold--;
}


/*
==============
=
= A_RectDeathEvent
=
==============
*/

void A_RectDeathEvent(mobj_t* actor) // 80012F6C
{
    mobjexp_t *exp;

    //P_BossExplode(actor, 3, 32);
    exp = Z_Malloc (sizeof(*exp), PU_LEVSPEC, 0);
    P_AddThinker (&exp->thinker);
    exp->thinker.function = T_MobjExplode;
    exp->mobj = actor;
    exp->delay = 0;
    exp->delaydefault = 3;
    exp->lifetime = 32;
    S_StartSound(NULL, actor->info->deathsound);
}


/*
==================
=
= A_TargetCamera
=
==================
*/

void A_TargetCamera(mobj_t* actor) // 80012FEC
{
    mobj_t* mo;

    actor->threshold = MAXINT;

    for(mo = mobjhead.next; mo != &mobjhead; mo = mo->next)
    {
        if(actor->tid+1 == mo->tid)
        {
            actor->target = mo;
            P_SetMobjState(actor, actor->info->missilestate);
            return;
        }
    }
}

/*
==================
=
= A_BarrelExplode
=
==================
*/

void A_BarrelExplode(mobj_t* actor) // 80013070
{

    S_StartSound(actor, actor->info->deathsound);
    P_SpawnMobj(actor->x, actor->y, actor->z + (actor->info->height >> 1), MT_EXPLOSION1);
    P_RadiusAttack(actor, actor->target, 128);//A_Explode(actor);

    A_OnDeathTrigger(actor);
}


/*
================
=
= A_Hoof
=
================
*/

void A_Hoof (mobj_t *mo) // 800130E0
{
	S_StartSound(mo, sfx_cybhoof);
	A_Chase(mo);
}

/*
================
=
= A_Metal
=
================
*/

void A_Metal (mobj_t *mo) // 80013110
{
	S_StartSound(mo, sfx_metal);
	A_Chase(mo);
}

/*
================
=
= A_BabyMetal
=
================
*/

void A_BabyMetal(mobj_t* mo) // 80013140
{
	S_StartSound(mo, sfx_bspistomp);
	A_Chase(mo);
}

/*============================================================================= */

/* a move in p_base.c crossed a special line */
#if 0
void L_CrossSpecial (mobj_t *mo)
{
	line_t	*line;

	line = (line_t *)(mo->extradata & ~1);

	P_CrossSpecialLine (line, mo);
}
#endif

/*
================
=
= L_MissileHit
=
================
*/

/* a move in p_base.c caused a missile to hit another thing or wall */
void L_MissileHit (mobj_t *mo) // 80013170
{
	int	damage;
	mobj_t	*missilething;

	missilething = (mobj_t *)mo->extradata;
	damage = 0;

	if (missilething)
	{
		damage = ((P_Random()&7)+1)*mo->info->damage;
		P_DamageMobj (missilething, mo, mo->target, damage);

        if ((mo->type == MT_PROJ_RECTFIRE) && (missilething->player))
        {
            missilething->momz = (1500*FRACUNIT) / missilething->info->mass;
        }
	}

    if (mo->type == MT_PROJ_DART)
    {
        if (missilething && !(missilething->flags & MF_NOBLOOD))
        {
            P_SpawnBlood(mo->x, mo->y, mo->z, damage);
        }
        else
        {
            S_StartSound(mo, sfx_darthit);
            P_SpawnPuff(mo->x, mo->y, mo->z);
        }
    }
	P_ExplodeMissile (mo);
}

/*
================
=
= L_SkullBash
=
================
*/

/* a move in p_base.c caused a flying skull to hit another thing or a wall */
void L_SkullBash (mobj_t *mo) // 800132AC
{
	int	damage;
	mobj_t	*skullthing;

	skullthing = (mobj_t *)mo->extradata;

	if (skullthing)
	{
		damage = ((P_Random()&7)+1)*mo->info->damage;
		P_DamageMobj (skullthing, mo, mo, damage);
	}

	mo->flags &= ~MF_SKULLFLY;
	mo->momx = mo->momy = mo->momz = 0;
	P_SetMobjState (mo, mo->info->spawnstate);
}

/*
==================
=
= A_FadeAlpha
=
==================
*/

void A_FadeAlpha(mobj_t *mobj) // 8001333C
{
    int fade;

    fade = mobj->alpha * 3;
    if(!(fade >= 0))
    {
        fade = fade + 3;
    }

    mobj->alpha = (fade >> 2);
}

/*
==================
=
= A_PainDeathEvent
=
==================
*/

void A_PainDeathEvent(mobj_t* actor) // 80013364
{
    actor->alpha -= 0x40;
}

/*
==================
=
= A_SkullSetAlpha
=
==================
*/

void A_SkullSetAlpha(mobj_t* actor) // 80013378
{
    actor->alpha >>= 2;
}

/*
==================
=
= A_MissileSetAlpha
=
==================
*/

void A_MissileSetAlpha(mobj_t* actor) // 8001338C
{
    actor->alpha >>= 1;
}


/*
==================
=
= A_FadeOut
=
==================
*/

void A_FadeOut(mobj_t* actor) // 800133A0
{
    fade_t *fade;

    if (actor->alpha >= 0xff)
    {
        actor->flags |= MF_SHADOW;
        //P_FadeMobj(actor, -8, 0x30, mobjflag_t::MF_NONE);
        fade = Z_Malloc (sizeof(*fade), PU_LEVSPEC, 0);
        P_AddThinker (&fade->thinker);
        fade->thinker.function = T_FadeThinker;
        fade->amount = -8;
        fade->destAlpha = 0x30;
        fade->flagReserve = 0;
        fade->mobj = actor;
    }
}


/*
==================
=
= A_FadeIn
=
==================
*/

void A_FadeIn(mobj_t* actor) // 80013428
{
    fade_t *fade;

    if (actor->alpha < 0xff)
    {
        actor->alpha = 0x30;
        actor->flags &= ~MF_SHADOW;

        //P_FadeMobj(actor, 8, 0xff, mobjflag_t::MF_NONE);
        fade = Z_Malloc (sizeof(*fade), PU_LEVSPEC, 0);
        P_AddThinker (&fade->thinker);
        fade->thinker.function = T_FadeThinker;
        fade->mobj = actor;
        fade->amount = 8;
        fade->destAlpha = 0xff;
        fade->flagReserve = 0;
    }
}


/*
================
=
P_MissileAttack
=
================
*/

mobj_t* P_MissileAttack(mobj_t *actor, dirproj_e direction) // 800134BC
{
    angle_t angle;
    fixed_t deltax, deltay, deltaz;
    fixed_t x, y;
    mobjtype_t type;
    mobj_t *mo;

    if(direction == DP_LEFT)
    {
        angle = actor->angle + ANG45;
    }
    else if(direction == DP_RIGHT)
    {
        angle = actor->angle - ANG45;
    }
    else
    {
        angle = actor->angle;
    }

    angle >>= ANGLETOFINESHIFT;
    x = finecosine[angle];
    y = finesine[angle];

    switch(actor->type)
    {
    case MT_MANCUBUS:
        deltay = (y * 50);
        deltax = (x * 50);
        deltaz = (69*FRACUNIT);
        type = MT_PROJ_FATSO;
        break;
    case MT_IMP1:
        deltay = 0;
        deltax = 0;
        deltaz = (64*FRACUNIT);
        type = MT_PROJ_IMP1;
        break;
    case MT_IMP2:
        deltay = 0;
        deltax = 0;
        deltaz = (64*FRACUNIT);
        type = MT_PROJ_IMP2;
        break;
    case MT_CACODEMON:
        deltay = 0;
        deltax = 0;
        deltaz = (46*FRACUNIT);
        type = MT_PROJ_HEAD;
        break;
    case MT_BRUISER1:
        deltay = 0;
        deltax = 0;
        deltaz = (48*FRACUNIT);
        type = MT_PROJ_BRUISER2;
        break;
    case MT_BRUISER2:
        deltay = 0;
        deltax = 0;
        deltaz = (48*FRACUNIT);
        type = MT_PROJ_BRUISER1;
        break;
    case MT_BABY:
        deltay = (y * 20);
        deltax = (x * 20);
        deltaz = (28*FRACUNIT);
        type = MT_PROJ_BABY;
        break;
    case MT_CYBORG:
    case MT_CYBORG_TITLE:
        deltay = (y * 45);
        deltax = (x * 45);
        deltaz = (88*FRACUNIT);
        type = MT_PROJ_ROCKET;
        break;

    default:
        break;
    }

    mo = P_SpawnMissile(actor, actor->target, deltax, deltay, deltaz, type);
    return mo;
}
