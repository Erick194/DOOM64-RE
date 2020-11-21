/* P_user.c */

#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"

#define MAXMOCKTIME     1800
int deathmocktics; // 800A56A0

#define MK_TXT01	"HAHAHAHA!"
#define MK_TXT02	"YOU SHOULDN'T HAVE DONE THAT."
#define MK_TXT03	"TRY AN EASIER LEVEL..."
#define MK_TXT04	"WOW! LOOK AT THOSE DEMON FEET."
#define MK_TXT05	"I PLAY DOOM AND I CAN'T GET UP."
#define MK_TXT06	"OUCH! THAT HAD TO HURT."
#define MK_TXT07	"LOOK AT ME! I'M FLAT!"
#define MK_TXT08	"THANKS FOR PLAYING!"
#define MK_TXT09	"YOU LAZY @&$#!"
#define MK_TXT10	"HAVE YOU HAD ENOUGH?"
#define MK_TXT11	"THE DEMONS GAVE YOU THE BOOT!"
#define MK_TXT12	"THE LEAD DEMON VANQUISHED YOU!"

char *mockstrings[] =   // 8005A290
{
    MK_TXT01, MK_TXT02, MK_TXT03, MK_TXT04,
	MK_TXT05, MK_TXT06, MK_TXT07, MK_TXT08,
	MK_TXT09, MK_TXT10, MK_TXT11, MK_TXT12,
};

fixed_t 		forwardmove[2] = {0xE000, 0x16000}; // 8005B060
fixed_t 		sidemove[2] = {0xE000, 0x16000};    // 8005B068

#define SLOWTURNTICS    10
fixed_t			angleturn[] =       // 8005B070
	{50,50,83,83,100,116,133,150,150,166,
	133,133,150,166,166,200,200,216,216,233}; // fastangleturn

/*============================================================================= */

mobj_t          *slidething;    //80077D04, pmGp000008f4
extern	fixed_t	slidex, slidey; //80077dbc || 80077dc0
extern	line_t	*specialline;   //80077dc8

void P_SlideMove (mobj_t *mo);

#if 0
void P_PlayerMove (mobj_t *mo)//L80029CB8()
{
	fixed_t		momx, momy;
	line_t		*latchedline;
	fixed_t		latchedx, latchedy;

	//momx = vblsinframe[playernum] * (mo->momx>>2);
	//momy = vblsinframe[playernum] * (mo->momy>>2);

	// Change on Final Doom
	momx = mo->momx;
	momy = mo->momy;

	slidething = mo;

	P_SlideMove ();

	latchedline = (line_t *)specialline;
	latchedx = slidex;
	latchedy = slidey;

	if ((latchedx == mo->x) && (latchedy == mo->y))
		goto stairstep;

	if (P_TryMove (mo, latchedx, latchedy))
		goto dospecial;

stairstep:

	if (momx > MAXMOVE)
		momx = MAXMOVE;
	else if (momx < -MAXMOVE)
		momx = -MAXMOVE;

	if (momy > MAXMOVE)
		momy = MAXMOVE;
	else if (momy < -MAXMOVE)
		momy = -MAXMOVE;

	/* something fucked up in slidemove, so stairstep */

	if (P_TryMove (mo, mo->x, mo->y + momy))
	{
		mo->momx = 0;
		mo->momy = momy;
		goto dospecial;
	}

	if (P_TryMove (mo, mo->x + momx, mo->y))
	{
		mo->momx = momx;
		mo->momy = 0;
		goto dospecial;
	}

	mo->momx = mo->momy = 0;

dospecial:
	if (latchedline)
		P_CrossSpecialLine (latchedline, mo);
}
#endif // 0

/*
===================
=
= P_PlayerXYMovement
=
===================
*/

#define	STOPSPEED		0x1000
#define	FRICTION		0xd200  //Jag 0xd240

void P_PlayerXYMovement (mobj_t *mo) // 80021E20
{
    /* */
	/* try to slide along a blocked move */
	/* */
    if (!P_TryMove(mo, mo->x + mo->momx, mo->y + mo->momy))
        P_SlideMove(mo);

	/* */
	/* slow down */
	/* */
	if (mo->z > mo->floorz)
		return;		/* no friction when airborne */

	if (mo->flags & MF_CORPSE)
		if (mo->floorz != mo->subsector->sector->floorheight)
			return;			/* don't stop halfway off a step */

	if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED &&
        mo->momy > -STOPSPEED && mo->momy < STOPSPEED)
	{
		mo->momx = 0;
		mo->momy = 0;
	}
	else
	{
		mo->momx = (mo->momx>>8)*(FRICTION>>8);
		mo->momy = (mo->momy>>8)*(FRICTION>>8);
	}
}


/*
===============
=
= P_PlayerZMovement
=
===============
*/

void P_PlayerZMovement (mobj_t *mo) // 80021f38
{
	/* */
	/* check for smooth step up */
	/* */
	if (mo->z < mo->floorz)
	{
		mo->player->viewheight -= (mo->floorz - mo->z);
		mo->player->deltaviewheight = (VIEWHEIGHT - mo->player->viewheight) >> 2;
	}

	/* */
	/* adjust height */
	/* */
	mo->z += mo->momz;

	/* */
	/* clip movement */
	/* */
	if (mo->z <= mo->floorz)
	{	/* hit the floor */
		if (mo->momz < 0)
		{
			if (mo->momz < -(GRAVITY*2))	/* squat down */
			{
				mo->player->deltaviewheight = mo->momz>>3;
				S_StartSound (mo, sfx_oof);
			}
			mo->momz = 0;
		}
		mo->z = mo->floorz;
	}
	else
	{
		if (mo->momz == 0)
			mo->momz = -(GRAVITY/2);
		else
			mo->momz -= (GRAVITY/4);
	}

	if (mo->z + mo->height > mo->ceilingz)
	{	/* hit the ceiling */
		if (mo->momz > 0)
			mo->momz = 0;
		mo->z = mo->ceilingz - mo->height;
	}
}


/*
================
=
= P_PlayerMobjThink
=
================
*/

void P_PlayerMobjThink (mobj_t *mobj) // 80022060
{
	state_t	*st;
	int		state;

	/* */
	/* momentum movement */
	/* */
	if (mobj->momx || mobj->momy)
		P_PlayerXYMovement (mobj);

    mobj->player->onground = (mobj->z <= mobj->floorz);

	if ( (mobj->z != mobj->floorz) || mobj->momz)
		P_PlayerZMovement (mobj);

	/* */
	/* cycle through states, calling action functions at transitions */
	/* */
	if (mobj->tics == -1)
		return;				/* never cycle */

	mobj->tics--;

	if (mobj->tics > 0)
		return;				/* not time to cycle yet */

	state = mobj->state->nextstate;
	st = &states[state];

	mobj->state = st;
	mobj->tics = st->tics;
	mobj->sprite = st->sprite;
	mobj->frame = st->frame;
}

/*============================================================================= */


/*
====================
=
= P_BuildMove
=
====================
*/

#define MAXSENSIVITY    10

void P_BuildMove (player_t *player) // 80022154
{
	int         speed, sensitivity;
	int			buttons, oldbuttons;
	mobj_t		*mo;
	buttons_t	*cbutton;

	cbutton = BT_DATA[0];
	buttons = ticbuttons[0];
	oldbuttons = oldticbuttons[0];

    player->forwardmove = player->sidemove = player->angleturn = 0;

	speed = (buttons & cbutton->BT_SPEED) > 0;
	sensitivity = 0;

	/*  */
	/* forward and backward movement  */
	/*  */
	if (cbutton->BT_FORWARD & buttons)
    {
        player->forwardmove = forwardmove[speed];
    }
    else if (cbutton->BT_BACK & buttons)
    {
        player->forwardmove = -forwardmove[speed];
    }
    else
    {
        /* Analyze analog stick movement (up / down) */
        sensitivity = (int)((buttons) << 24) >> 24;

        if(sensitivity >= MAXSENSIVITY || sensitivity <= -MAXSENSIVITY)
        {
            player->forwardmove += (forwardmove[1] * sensitivity) / 80;
        }
    }

	/*  */
	/* use two stage accelerative turning on the joypad  */
	/*  */
	if (((buttons & cbutton->BT_LEFT) && (oldbuttons & cbutton->BT_LEFT)))
		player->turnheld++;
    else if (((buttons & cbutton->BT_RIGHT) && (oldbuttons & cbutton->BT_RIGHT)))
		player->turnheld++;
	else
		player->turnheld = 0;

	if (player->turnheld >= SLOWTURNTICS)
		player->turnheld = SLOWTURNTICS-1;

    /*  */
	/* strafe movement  */
	/*  */
	if (buttons & cbutton->BT_STRAFELEFT)
	{
	    player->sidemove -= sidemove[speed];
	}
	if (buttons & cbutton->BT_STRAFERIGHT)
	{
		player->sidemove += sidemove[speed];
	}

    if (buttons & cbutton->BT_STRAFE)
	{
		if (buttons & cbutton->BT_LEFT)
		{
            player->sidemove = -sidemove[speed];
		}
		else if (buttons & cbutton->BT_RIGHT)
		{
            player->sidemove = sidemove[speed];
		}
		else
        {
            /* Analyze analog stick movement (left / right) */
            sensitivity = (int)(((buttons & 0xff00) >> 8) << 24) >> 24;

            if(sensitivity >= MAXSENSIVITY || sensitivity <= -MAXSENSIVITY)
            {
                player->sidemove += (sidemove[1] * sensitivity) / 80;
            }
        }
	}
	else
	{
        if (sensitivity == 0)
            speed = 0;

        if (cbutton->BT_LEFT & buttons)
        {
            player->angleturn =  angleturn[player->turnheld + (speed * SLOWTURNTICS)] << 17;
        }
        else if (cbutton->BT_RIGHT & buttons)
        {
            player->angleturn = -angleturn[player->turnheld + (speed * SLOWTURNTICS)] << 17;
        }
        else
        {
            /* Analyze analog stick movement (left / right) */
            sensitivity = (int)(((buttons & 0xff00) >> 8) << 24) >> 24;
            sensitivity = -sensitivity;

            if(sensitivity >= MAXSENSIVITY || sensitivity <= -MAXSENSIVITY)
            {
                sensitivity = (((M_SENSITIVITY * 800) / 100) + 233) * sensitivity;
                player->angleturn += (sensitivity / 80) << 17;
            }
        }
	}

	/* */
	/* if slowed down to a stop, change to a standing frame */
	/* */
	mo = player->mo;

	if (!mo->momx && !mo->momy && player->forwardmove == 0 && player->sidemove == 0 )
	{	/* if in a walking frame, stop moving */
		if (mo->state == &states[S_002] //S_PLAY_RUN1
		|| mo->state == &states[S_003]  //S_PLAY_RUN2
		|| mo->state == &states[S_004]  //S_PLAY_RUN3
		|| mo->state == &states[S_005]) //S_PLAY_RUN4
			P_SetMobjState (mo, S_001); //S_PLAY
	}
}

/*
===============================================================================

						movement

===============================================================================
*/

#define MAXBOB			0x100000		/* 16 pixels of bob */

/*
==================
=
= P_Thrust
=
= moves the given origin along a given angle
=
==================
*/

void P_Thrust (player_t *player, angle_t angle, fixed_t move) // 800225BC
{
    angle >>= ANGLETOFINESHIFT;
	player->mo->momx += FixedMul(vblsinframe[0] * move, finecosine[angle]);
	player->mo->momy += FixedMul(vblsinframe[0] * move, finesine[angle]);
}



/*
==================
=
= P_CalcHeight
=
= Calculate the walking / running height adjustment
=
==================
*/

void P_CalcHeight (player_t *player) // 80022670
{
	int			angle;
	fixed_t		bob;
	fixed_t		val;

	/* */
	/* regular movement bobbing (needs to be calculated for gun swing even */
	/* if not on ground) */
	/* OPTIMIZE: tablify angle  */
	/* */
	val = player->mo->momx;
	player->bob = FixedMul(val, val);
	val = player->mo->momy;
	player->bob += FixedMul(val, val);

	player->bob >>= 2;
	if (player->bob > MAXBOB)
	{
		player->bob = MAXBOB;
	}

	if (!player->onground)
	{
		player->viewz = player->mo->z + VIEWHEIGHT;
		if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
			player->viewz = player->mo->ceilingz-4*FRACUNIT;
		return;
	}

	angle = (FINEANGLES/40*ticon)&(FINEANGLES-1);
	bob = FixedMul((player->bob / 2), finesine[angle]);

	//ST_DebugPrint("bob %x",FixedMul((player->bob / 2), finesine[angle]));
	//ST_DebugPrint("bob2 %x",FixedMul2((player->bob / 2), finesine[angle]));

	//ST_DebugPrint("bobdiv %x",FixedDiv2(0x49003, 0x2));
	//ST_DebugPrint("bobdiv2 %x",FixedMul3(0x49003, 0x2));

	/* */
	/* move viewheight */
	/* */
	if (player->playerstate == PST_LIVE)
	{
		player->viewheight += player->deltaviewheight;
		if (player->viewheight > VIEWHEIGHT)
		{
			player->viewheight = VIEWHEIGHT;
			player->deltaviewheight = 0;
		}
		if (player->viewheight < VIEWHEIGHT/2)
		{
			player->viewheight = VIEWHEIGHT/2;
			if (player->deltaviewheight <= 0)
				player->deltaviewheight = 1;
		}

		if (player->deltaviewheight)
		{
			player->deltaviewheight += FRACUNIT/2;
			if (!player->deltaviewheight)
				player->deltaviewheight = 1;
		}
	}
	player->viewz = player->mo->z + player->viewheight + bob;
	if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
		player->viewz = player->mo->ceilingz-4*FRACUNIT;
}

/*
=================
=
= P_MovePlayer
=
=================
*/

void P_MovePlayer (player_t *player) // 8002282C
{
	player->mo->angle += vblsinframe[0] * player->angleturn;

	if(player->onground)
    {
        if (player->forwardmove)
            P_Thrust (player, player->mo->angle, player->forwardmove);
        if (player->sidemove)
            P_Thrust (player, player->mo->angle-ANG90, player->sidemove);
    }

	if ((player->forwardmove || player->sidemove) && player->mo->state == &states[S_001])//S_PLAY
		P_SetMobjState (player->mo, S_002);//S_PLAY_RUN1
}


/*
=================
=
= P_DeathThink
=
=================
*/

void P_DeathThink (player_t *player) // 80022914
{
	angle_t		angle, delta;

	P_MovePsprites (player);

	/* fall to the ground */
	if (player->viewheight > 8*FRACUNIT)
		player->viewheight -= FRACUNIT;

	player->onground = (player->mo->z <= player->mo->floorz);

	P_CalcHeight (player);

	if (player->attacker && player->attacker != player->mo)
	{
		angle = R_PointToAngle2 (player->mo->x, player->mo->y, player->attacker->x, player->attacker->y);
		delta = angle - player->mo->angle;
		if (delta < ANG5 || delta > (unsigned)-ANG5)
		{	/* looking at killer, so fade damage flash down */
			player->mo->angle = angle;
			if (player->damagecount)
				player->damagecount--;
		}
		else if (delta < ANG180)
			player->mo->angle += ANG5;
		else
			player->mo->angle -= ANG5;
	}
	else if (player->damagecount)
		player->damagecount--;

	/* mocking text */
    if ((ticon - deathmocktics) > MAXMOCKTIME)
    {
        player->messagetic = MSGTICS;
        player->message = mockstrings[P_Random() % 12];
        deathmocktics = ticon;
    }

	if (((ticbuttons[0] & (PAD_A|PAD_B|ALL_TRIG|ALL_CBUTTONS)) != 0) &&
        (player->viewheight <= 8*FRACUNIT))
    {
		player->playerstate = PST_REBORN;
    }

    if (player->bonuscount)
        player->bonuscount -= 1;

    // [d64] - recoil pitch from weapons
    if (player->recoilpitch)
        player->recoilpitch = (((player->recoilpitch << 2) - player->recoilpitch) >> 2);

    if(player->bfgcount)
    {
        player->bfgcount -= 6;

        if(player->bfgcount < 0)
            player->bfgcount = 0;
    }
}

/*
===============================================================================
=
= P_PlayerInSpecialSector
=
= Called every tic frame that the player origin is in a special sector
=
===============================================================================
*/

void P_PlayerInSpecialSector (player_t *player, sector_t *sec) // 80022B1C
{
    fixed_t speed;

	if (player->mo->z != sec->floorheight)
		return;		/* not all the way down yet */

    if(sec->flags & MS_SECRET)
    {
        player->secretcount++;
        player->message = "You found a secret area!";
        player->messagetic = MSGTICS;
        sec->flags &= ~MS_SECRET;
    }

    if(sec->flags & MS_DAMAGEX5)    /* NUKAGE DAMAGE */
    {
        if(!player->powers[pw_ironfeet])
        {
            if ((gamevbls < (int)gametic) && !(gametic & 31))
                  P_DamageMobj(player->mo, NULL, NULL, 5);
        }
    }

    if(sec->flags & MS_DAMAGEX10)    /* HELLSLIME DAMAGE */
    {
        if(!player->powers[pw_ironfeet])
        {
            if ((gamevbls < (int)gametic) && !(gametic & 31))
                  P_DamageMobj(player->mo, NULL, NULL, 10);
        }
    }

    if(sec->flags & MS_DAMAGEX20)    /* SUPER HELLSLIME DAMAGE */
    {
        if(!player->powers[pw_ironfeet] || (P_Random() < 5))
        {
            if ((gamevbls < (int)gametic) && !(gametic & 31))
                  P_DamageMobj(player->mo, NULL, NULL, 20);
        }
    }

    if(sec->flags & MS_SCROLLFLOOR)
    {
        if(sec->flags & MS_SCROLLFAST)
            speed = 0x3000;
        else
            speed = 0x1000;

        if(sec->flags & MS_SCROLLLEFT)
        {
            P_Thrust(player, ANG180, speed);
        }
        else if(sec->flags & MS_SCROLLRIGHT)
        {
            P_Thrust(player, 0, speed);
        }

        if(sec->flags & MS_SCROLLUP)
        {
            P_Thrust(player, ANG90, speed);
        }
        else if(sec->flags & MS_SCROLLDOWN)
        {
            P_Thrust(player, ANG270, speed);
        }
    }
}

/*
=================
=
= P_PlayerThink
=
=================
*/

void P_PlayerThink (player_t *player) // 80022D60
{
	int		     buttons, oldbuttons;
	buttons_t    *cbutton;
	weapontype_t weapon;
	sector_t     *sec;

	buttons = ticbuttons[0];
	oldbuttons = oldticbuttons[0];
	cbutton = BT_DATA[0];

	/* */
	/* check for weapon change */
	/* */
	if (player->playerstate == PST_LIVE)
	{
		weapon = player->pendingweapon;
		if (weapon == wp_nochange)
			weapon = player->readyweapon;

		if ((buttons & cbutton->BT_WEAPONBACKWARD) && !(oldbuttons & cbutton->BT_WEAPONBACKWARD))
		{
            if (weapon == wp_chainsaw)
            {
                player->pendingweapon = wp_fist;
            }
            else
            {
                if((int)(weapon-1) >= wp_chainsaw)
                {
                    while(--weapon >= wp_chainsaw && !player->weaponowned[weapon]);
                }

                if((int)weapon >= wp_chainsaw)
                    player->pendingweapon = weapon;
            }
		}
		else if ((buttons & cbutton->BT_WEAPONFORWARD) && !(oldbuttons & cbutton->BT_WEAPONFORWARD))
		{
		    if((int)(weapon+1) < NUMWEAPONS)
            {
                while(++weapon < NUMWEAPONS && !player->weaponowned[weapon]);
            }

            if((int)weapon < NUMWEAPONS)
                player->pendingweapon = weapon;
		}
	}

	if (!gamepaused)
	{
		P_PlayerMobjThink(player->mo);
		P_BuildMove(player);

        sec = player->mo->subsector->sector;
        if (sec->flags & (MS_SECRET | MS_DAMAGEX5 | MS_DAMAGEX10 | MS_DAMAGEX20 | MS_SCROLLFLOOR))
            P_PlayerInSpecialSector(player, sec);

		if (player->playerstate == PST_DEAD)
		{
			P_DeathThink(player);
			return;
		}

		/* */
		/* chain saw run forward */
		/* */
		if (player->mo->flags & MF_JUSTATTACKED)
		{
			player->angleturn = 0;
			player->forwardmove = 0xc800;
			player->sidemove = 0;
			player->mo->flags &= ~MF_JUSTATTACKED;
		}

		/* */
		/* move around */
		/* reactiontime is used to prevent movement for a bit after a teleport */
		/* */

		if (player->mo->reactiontime)
			player->mo->reactiontime--;
		else
			P_MovePlayer(player);

		P_CalcHeight(player);

		/* */
		/* check for use */
		/* */

		if ((buttons & cbutton->BT_USE))
		{
			if (player->usedown == false)
			{
				P_UseLines(player);
				player->usedown = true;
			}
		}
		else
        {
			player->usedown = false;
        }

		if (buttons & cbutton->BT_ATTACK)
		{
			P_SetMobjState(player->mo, S_006);//S_PLAY_ATK1
			player->attackdown++;
		}
		else
        {
			player->attackdown = 0;
        }

		/* */
		/* cycle psprites */
		/* */

		P_MovePsprites(player);

		/* */
		/* counters */
		/* */

		if (gamevbls < gametic)
		{
			if (player->powers[pw_strength] > 1)
				player->powers[pw_strength]--;	/* strength counts down to diminish fade */

			if (player->powers[pw_invulnerability])
				player->powers[pw_invulnerability]--;

			if (player->powers[pw_invisibility])
			{
				player->powers[pw_invisibility]--;
				if (!player->powers[pw_invisibility])
				{
					player->mo->flags &= ~MF_SHADOW;
				}
				else if ((player->powers[pw_invisibility] < 61) && !(player->powers[pw_invisibility] & 7))
                {
                    player->mo->flags ^= MF_SHADOW;
                }
			}

            if (player->powers[pw_infrared])
                player->powers[pw_infrared]--;

            if (player->powers[pw_ironfeet])
                player->powers[pw_ironfeet]--;

            if (player->damagecount)
                player->damagecount--;

            if (player->bonuscount)
                player->bonuscount--;

            // [d64] - recoil pitch from weapons
            if (player->recoilpitch)
                player->recoilpitch = (((player->recoilpitch << 2) - player->recoilpitch) >> 2);

            if(player->bfgcount)
            {
                player->bfgcount -= 6;

                if(player->bfgcount < 0)
                    player->bfgcount = 0;
            }
		}
	}
}
