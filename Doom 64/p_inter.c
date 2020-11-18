/* P_inter.c */


#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"

#define	BONUSADD		4

/* a weapon is found with two clip loads, a big item has five clip loads */
int		maxammo[NUMAMMO] = {200, 50, 300, 50}; // 8005AD40
int		clipammo[NUMAMMO] = {10, 4, 20, 1}; // 8005AD50

/*
===============================================================================

							GET STUFF

===============================================================================
*/

/*
===================
=
= P_GiveAmmo
=
= Num is the number of clip loads, not the individual count (0= 1/2 clip)
= Returns false if the ammo can't be picked up at all
===================
*/

boolean P_GiveAmmo (player_t *player, ammotype_t ammo, int num) // 800143E0
{
	int		oldammo;

	if (ammo == am_noammo)
		return false;

	if (ammo > NUMAMMO)
		I_Error ("P_GiveAmmo: bad type %i", ammo);

	if ( player->ammo[ammo] == player->maxammo[ammo]  )
		return false;

	if (num)
		num *= clipammo[ammo];
	else
    {
        //num = clipammo[ammo]/2;
		num = clipammo[ammo];
		if(!(num >= 0))
        {
            num = num + 1;
        }

        num = num/2;
    }

	if (gameskill == sk_baby)
		num <<= 1;			/* give double ammo in trainer mode */

	oldammo = player->ammo[ammo];
	player->ammo[ammo] += num;
	if (player->ammo[ammo] > player->maxammo[ammo])
		player->ammo[ammo] = player->maxammo[ammo];

	if (oldammo)
		return true;		/* don't change up weapons, player was lower on */
							/* purpose */

	switch (ammo)
	{
	case am_clip:
		if (player->readyweapon == wp_fist)
		{
			if (player->weaponowned[wp_chaingun])
				player->pendingweapon = wp_chaingun;
			else
				player->pendingweapon = wp_pistol;
		}
		break;
	case am_shell:
		if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
		{
			if (player->weaponowned[wp_shotgun])
				player->pendingweapon = wp_shotgun;
		}
		break;
	case am_cell:
		if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
		{
			if (player->weaponowned[wp_plasma])
				player->pendingweapon = wp_plasma;
		}
		break;
	case am_misl:
		if (player->readyweapon == wp_fist)
		{
			if (player->weaponowned[wp_missile])
				player->pendingweapon = wp_missile;
		}
	default:
		break;
	}

	return true;
}


/*
===================
=
= P_GiveWeapon
=
= The weapon name may have a MF_DROPPED flag ored in
===================
*/

boolean P_GiveWeapon (player_t *player, weapontype_t weapon, boolean dropped) // 800145C0
{
	boolean		gaveammo, gaveweapon;

	if (weaponinfo[weapon].ammo != am_noammo)
	{	/* give one clip with a dropped weapon, two clips with a found weapon */
		if (dropped)
			gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, 1);
		else
			gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, 2);
	}
	else
		gaveammo = false;

	if (player->weaponowned[weapon])
		gaveweapon = false;
	else
	{
		gaveweapon = true;
		player->weaponowned[weapon] = true;
		player->pendingweapon = weapon;
	}

	return gaveweapon || gaveammo;
}



/*
===================
=
= P_GiveBody
=
= Returns false if the body isn't needed at all
===================
*/

boolean P_GiveBody (player_t *player, int num) // 80014680
{
	if (player->health >= MAXHEALTH)
		return false;

	player->health += num;
	if (player->health > MAXHEALTH)
		player->health = MAXHEALTH;
	player->mo->health = player->health;

	return true;
}


/*
===================
=
= P_GiveArmor
=
= Returns false if the armor is worse than the current armor
===================
*/

boolean P_GiveArmor (player_t *player, int armortype) // 800146C8
{
	int		hits;

	hits = armortype*100;
	if (player->armorpoints >= hits)
		return false;		/* don't pick up */

	player->armortype = armortype;
	player->armorpoints = hits;

	return true;
}


/*
===================
=
= P_GiveCard
=
===================
*/

void P_GiveCard (player_t *player, card_t card) // 80014704
{
	if (player->cards[card])
		return;
	player->bonuscount = BONUSADD;
	player->cards[card] = true;
}


/*
===================
=
= P_GivePower
=
===================
*/

boolean P_GivePower (player_t *player, powertype_t power) // 8001472C
{
	switch (power)
	{
	case pw_invulnerability:
		player->powers[power] = INVULNTICS;
		return true;
	case pw_invisibility:
		player->powers[power] = INVISTICS;
		player->mo->flags |= MF_SHADOW;
		return true;
	case pw_infrared:
		player->powers[power] = INFRATICS;
        infraredFactor = 300;
        P_RefreshBrightness();
		return true;
	case pw_ironfeet:
		player->powers[power] = IRONTICS;
		return true;
	case pw_strength:
		P_GiveBody(player, 100);
		player->powers[power] = STRTICS;
		return true;
	default:
		break;
	}
    //pw_allmap
	if (player->powers[power])
		return false;		/* already got it */

	player->powers[power] = 1;
	return true;
}

/*
==================
=
= P_TouchSpecialThing
=
==================
*/

int ArtifactLookupTable[8] = {0, 1, 1, 2, 1, 2, 2, 3}; // 8005AD60

void P_TouchSpecialThing (mobj_t *special, mobj_t *toucher) // 80014810
{
	player_t	*player;
	int			i;
	fixed_t		delta;
	int			sound;
	char        *message;
	int         artflag;

	delta = special->z - toucher->z;
	if (delta > toucher->height || delta < -8*FRACUNIT)
		return;			/* out of reach */

	sound = sfx_itemup;
	player = toucher->player;
	if (toucher->health <= 0)
		return;						/* can happen with a sliding player corpse */

    message = NULL;

	switch (special->type)
	{

    /* */
    /* bonus items */
    /* */
	case MT_ITEM_BONUSHEALTH:
		player->health+=2;		/* can go over 100% */
		if (player->health > 200)
			player->health = 200;
		player->mo->health = player->health;
		message = "You pick up a health bonus.";
		break;
	case MT_ITEM_BONUSARMOR:
		player->armorpoints+=2;		/* can go over 100% */
		if (player->armorpoints > 200)
			player->armorpoints = 200;
		if (!player->armortype)
			player->armortype = 1;
		message = "You pick up an armor bonus.";
		break;
	case MT_ITEM_SOULSPHERE:
		player->health += 100;
		if (player->health > 200)
			player->health = 200;
		player->mo->health = player->health;
		message = "Supercharge!";
		sound = sfx_powerup;
		break;
	case MT_ITEM_MEGASPHERE:
		player->health = 200;
		player->mo->health = 200;
		P_GiveArmor(player, 2);
		message = "Mega Sphere!";
		sound = sfx_powerup;
		break;

    /* */
    /* ammo */
    /* */
	case MT_AMMO_CLIP:
		if (special->flags & MF_DROPPED)
		{
			if (!P_GiveAmmo (player,am_clip,0))
				return;
		}
		else
		{
			if (!P_GiveAmmo (player,am_clip,1))
				return;
		}
		message = "Picked up a clip.";
		break;
	case MT_AMMO_CLIPBOX:
		if (!P_GiveAmmo (player, am_clip,5))
			return;
		message = "Picked up a box of bullets.";
		break;
	case MT_AMMO_ROCKET:
		if (!P_GiveAmmo (player, am_misl,1))
			return;
		message = "Picked up a rocket.";
		break;
	case MT_AMMO_ROCKETBOX:
		if (!P_GiveAmmo (player, am_misl,5))
			return;
		message = "Picked up a box of rockets.";
		break;
	case MT_AMMO_CELL:
		if (!P_GiveAmmo (player, am_cell,1))
			return;
		message = "Picked up an energy cell.";
		break;
	case MT_AMMO_CELLPACK:
		if (!P_GiveAmmo (player, am_cell,5))
			return;
		message = "Picked up an energy cell pack.";
		break;
	case MT_AMMO_SHELL:
		if (!P_GiveAmmo (player, am_shell,1))
			return;
        if (gameskill == sk_baby)
            message = "Picked up 8 shotgun shells.";
        else
            message = "Picked up 4 shotgun shells.";
		break;
	case MT_AMMO_SHELLBOX:
		if (!P_GiveAmmo (player, am_shell,5))
			return;
		message = "Picked up a box of shotgun shells.";
		break;
	case MT_AMMO_BACKPACK:
		if (!player->backpack)
		{
			for (i=0 ; i<NUMAMMO ; i++)
				player->maxammo[i] *= 2;
			player->backpack = true;
		}
		for (i=0 ; i<NUMAMMO ; i++)
			P_GiveAmmo (player, i, 1);
		message = "You got the backpack!";
		break;


	/* */
	/* weapons */
	/* */
	case MT_WEAP_BFG:
		if (!P_GiveWeapon (player, wp_bfg, false) )
			return;
		message = "You got the BFG9000!  Oh, yes.";
		sound = sfx_sgcock;
		break;
	case MT_WEAP_CHAINGUN:
		if (!P_GiveWeapon (player, wp_chaingun, special->flags&MF_DROPPED) )
			return;
		message = "You got the chaingun!";
		sound = sfx_sgcock;
		break;
	case MT_WEAP_CHAINSAW:
		if (!P_GiveWeapon (player, wp_chainsaw, false) )
			return;
		message = "A chainsaw!  Find some meat!";
		sound = sfx_sgcock;
		break;
	case MT_WEAP_LAUNCHER:
		if (!P_GiveWeapon (player, wp_missile, false) )
			return;
		message = "You got the rocket launcher!";
		sound = sfx_sgcock;
		break;
	case MT_WEAP_PLASMA:
		if (!P_GiveWeapon (player, wp_plasma, false) )
			return;
		message = "You got the plasma gun!";
		sound = sfx_sgcock;
		break;
	case MT_WEAP_SHOTGUN:
		if (!P_GiveWeapon (player, wp_shotgun, special->flags&MF_DROPPED ) )
			return;
		message = "You got the shotgun!";
		sound = sfx_sgcock;
		break;
	case MT_WEAP_SSHOTGUN:
		if (!P_GiveWeapon(player, wp_supershotgun, special->flags&MF_DROPPED))
			return;
		message = "You got the super shotgun!";
		sound = sfx_sgcock;
		break;
    case MT_WEAP_LCARBINE:
		if (!P_GiveWeapon(player, wp_laser, false))
			return;
		message = "What the !@#%* is this!";
		sound = sfx_sgcock;
		break;

		/* */
		/* armor */
		/* */
	case MT_ITEM_ARMOR1:
		if (!P_GiveArmor(player, 1))
			return;
		message = "You pick up the armor.";
		break;

	case MT_ITEM_ARMOR2:
		if (!P_GiveArmor(player, 2))
			return;
		message = "You got the MegaArmor!";
		break;

    /* */
    /* cards */
    /* leave cards for everyone */
    /* */
	case MT_ITEM_BLUECARDKEY:
		if (!player->cards[it_bluecard])
			message = "You pick up a blue keycard.";
		P_GiveCard(player, it_bluecard);
		break;
    case MT_ITEM_REDCARDKEY:
		if (!player->cards[it_redcard])
			message = "You pick up a red keycard.";
		P_GiveCard(player, it_redcard);
		break;
	case MT_ITEM_YELLOWCARDKEY:
		if (!player->cards[it_yellowcard])
			message = "You pick up a yellow keycard.";
		P_GiveCard(player, it_yellowcard);
		break;
	case MT_ITEM_BLUESKULLKEY:
		if (!player->cards[it_blueskull])
			message = "You pick up a blue skull key.";
		P_GiveCard(player, it_blueskull);
		break;
    case MT_ITEM_REDSKULLKEY:
		if (!player->cards[it_redskull])
			message = "You pick up a red skull key.";
		P_GiveCard(player, it_redskull);
		break;
	case MT_ITEM_YELLOWSKULLKEY:
		if (!player->cards[it_yellowskull])
			message = "You pick up a yellow skull key.";
		P_GiveCard(player, it_yellowskull);
		break;

    /* */
    /* heals */
    /* */
	case MT_ITEM_STIMPACK:
		if (!P_GiveBody(player, 10))
			return;
		message = "You pick up a stimpack.";
		break;
	case MT_ITEM_MEDKIT:
		if (!P_GiveBody(player, 25))
			return;
		if (player->health < 25)
			message = "You pick up a medikit that you REALLY need!";
		else
			message = "You pick up a medikit.";
		break;

    /* */
    /* power ups */
    /* */
	case MT_ITEM_INVULSPHERE:
		P_GivePower(player, pw_invulnerability);
		message = "Invulnerability!";
		sound = sfx_powerup;
		break;
	case MT_ITEM_BERSERK:
		P_GivePower(player, pw_strength);
		message = "Berserk!";
		if (player->readyweapon != wp_fist)
			player->pendingweapon = wp_fist;
		sound = sfx_powerup;
		break;
	case MT_ITEM_INVISSPHERE:
		P_GivePower(player, pw_invisibility);
		message = "Partial Invisibility!";
		sound = sfx_powerup;
		break;
	case MT_ITEM_RADSPHERE:
		P_GivePower(player, pw_ironfeet);
		message = "Radiation Shielding Suit";
		sound = sfx_powerup;
		break;
	case MT_ITEM_AUTOMAP:
		if (!P_GivePower(player, pw_allmap))
			return;
		message = "Computer Area Map";
		sound = sfx_powerup;
		break;
	case MT_ITEM_PVIS:
		P_GivePower(player, pw_infrared);
		message = "Light Amplification Goggles";
		sound = sfx_powerup;
		break;

    /* */
    /* artifacts */
    /* */
    case MT_ITEM_ARTIFACT1:
    case MT_ITEM_ARTIFACT2:
    case MT_ITEM_ARTIFACT3:

        artflag = 1 << ((special->type + 7) & 0x1f);

        if ((player->artifacts & artflag))
            return;

        player->artifacts |= artflag;

        if (ArtifactLookupTable[player->artifacts] == 1) /* ART_FAST */
            message = "You have a feeling that\nit wasn't to be touched...";
        else if (ArtifactLookupTable[player->artifacts] == 2) /* ART_TRIPLE */
            message = "Whatever it is, it doesn't\nbelong in this world...";
        else /* ART_DOUBLE */
            message = "It must do something...";

        sound = sfx_powerup;
		break;

    /* */
    /* fake item */
    /* */
    case MT_FAKEITEM:
        goto runtrigger;
        break;

	default:
		I_Error("P_SpecialThing: Unknown gettable thing"); // Restored
		break;
	}

    if (message)
    {
        player->message = message;
        player->messagetic = MSGTICS;
    }

	if (special->flags & MF_COUNTITEM)
		player->itemcount++;

    if (special->flags & MF_COUNTSECRET)
        player->secretcount++;

	P_RemoveMobj (special);
	player->bonuscount += BONUSADD;

	if (player == &players[0])
		S_StartSound(NULL, sound);

	if(special->flags & MF_TRIGTOUCH)
    {
    runtrigger:
        if(!P_ActivateLineByTag(special->tid, toucher))
        {
            macroqueue[macroidx1].activator = toucher;
            macroqueue[macroidx1].tag = special->tid;
            macroidx1 = (macroidx1 + 1) & 3;
        }
    }
}

/*
==============
=
= KillMobj
=
==============
*/
extern int deathmocktics; // 800A56A0

void P_KillMobj (mobj_t *source, mobj_t *target) // 80015080
{
	mobjtype_t		item;
	mobj_t			*mo;
	boolean         forceXdeath;

	target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY);

	if (target->type != MT_SKULL)
		target->flags |= MF_GRAVITY;

	target->flags |= MF_CORPSE|MF_DROPOFF;
	target->height >>= 2;

	forceXdeath = false;    //New PsxDoom / Doom64

	if (target->player)
	{	/* a frag of one sort or another */
		if (!source || !source->player || source->player == target->player)
		{	/* killed self somehow */
			target->player->frags--;
			//if (target->player->frags < 0)
				//target->player->frags = 0;
		}
		else
		{	/* killed by other player */
			source->player->frags++;
		}

		/* else just killed by a monster */
	}
	else if (source && source->player && (target->flags & MF_COUNTKILL) )
	{	/* a deliberate kill by a player */
		source->player->killcount++;		/* count for intermission */
	}
	else if ((target->flags & MF_COUNTKILL) )
		players[0].killcount++;			/* count all monster deaths, even */
										/* those caused by other monsters */

	if (target->player)
	{
		target->flags &= ~MF_SOLID;
		target->player->playerstate = PST_DEAD;
		P_DropWeapon (target->player);
		if (target->health < -50)
		{
		    forceXdeath = true; //Force the player to the state of Xdeath

			S_StartSound (target, sfx_slop);
		}
		else
			S_StartSound (target, sfx_plrdie);

        deathmocktics = ticon;
	}

	if (forceXdeath || (target->health < -target->info->spawnhealth) && target->info->xdeathstate)
		P_SetMobjState (target, target->info->xdeathstate);
	else
		P_SetMobjState (target, target->info->deathstate);

	target->tics -= P_Random()&1;
	if (target->tics < 1)
		target->tics = 1;

	/* */
	/* drop stuff */
	/* */
	switch (target->type)
	{
	case MT_POSSESSED1: //MT_POSSESSED:
		item = MT_AMMO_CLIP; //MT_CLIP;
		break;
	case MT_POSSESSED2: //MT_SHOTGUY:
		item = MT_WEAP_SHOTGUN; //MT_SHOTGUN;
		break;
	/*case MT_CHAINGUY:
		item = MT_WEAP_CHAINGUN; //MT_CHAINGUN;
		break;*/
	default:
		return;
	}

	mo = P_SpawnMobj (target->x,target->y,ONFLOORZ, item);
	mo->flags |= MF_DROPPED;		/* special versions of items */
}



/*
=================
=
= P_DamageMobj
=
= Damages both enemies and players
= inflictor is the thing that caused the damage
= 		creature or missile, can be NULL (slime, etc)
= source is the thing to target after taking damage
=		creature or NULL
= Source and inflictor are the same for melee attacks
= source can be null for barrel explosions and other environmental stuff
==================
*/

void P_DamageMobj (mobj_t *target, mobj_t *inflictor, mobj_t *source, int damage) // 800152DC
{
	unsigned	ang, an;
	int			saved;
	player_t	*player;
	fixed_t		thrust;

	if (!(target->flags & MF_SHOOTABLE))
		return;						/* shouldn't happen... */

	if (target->health <= 0)
		return;

	if (target->flags & MF_SKULLFLY)
	{
		target->momx = target->momy = target->momz = 0;
	}

	player = target->player;
	if (player && gameskill == sk_baby)
		damage >>= 1;				/* take half damage in trainer mode */

	/* */
	/* kick away unless using the chainsaw */
	/* */
	if (inflictor && (!source || !source->player || source->player->readyweapon != wp_chainsaw))
	{
		ang = R_PointToAngle2 (inflictor->x, inflictor->y, target->x, target->y);

		thrust = (damage * ((FRACUNIT >> 2) * 100)) / target->info->mass;

		/* make fall forwards sometimes */
		if ( (damage < 40) && (damage > target->health) && (target->z - inflictor->z > (64*FRACUNIT)) && (P_Random ()&1))
		{
			ang += ANG180;
			thrust *= 4;
		}

		an = ang >> ANGLETOFINESHIFT;
		thrust >>= 16;
		target->momx += thrust * finecosine[an];
		target->momy += thrust * finesine[an];

		// [psx/d64]: clamp thrust for players only
		if (target->player)
		{
            if (target->momx > MAXMOVE)
                target->momx = MAXMOVE;
            else if (target->momx < -MAXMOVE)
                target->momx = -MAXMOVE;

            if (target->momy > MAXMOVE)
                target->momy = MAXMOVE;
            else if (target->momy < -MAXMOVE)
                target->momy = -MAXMOVE;
		}
	}

	/* */
	/* player specific */
	/* */
	if (player)
	{
		if ((player->cheats&CF_GODMODE) || player->powers[pw_invulnerability])
			return;

		if (player->armortype)
		{
			if (player->armortype == 1)
				saved = damage/3;
			else
				saved = damage/2;
			if (player->armorpoints <= saved)
			{	/* armor is used up */
				saved = player->armorpoints;
				player->armortype = 0;
			}
			player->armorpoints -= saved;
			damage -= saved;
		}
		S_StartSound (target,sfx_plrpain);
		player->health -= damage;		/* mirror mobj health here for Dave */
		if (player->health < 0)
			player->health = 0;
		player->attacker = source;

        player->damagecount += (damage/*>>1*/);	/* add damage after armor / invuln */
	}

	/* */
	/* do the damage */
	/* */
	target->health -= damage;
	if (target->health <= 0)
	{
		P_KillMobj (source, target);
		return;
	}

	if ( (P_Random () < target->info->painchance) && !(target->flags&MF_SKULLFLY) )
	{
		target->flags |= MF_JUSTHIT;		/* fight back! */
		if(target->info->painstate)
            P_SetMobjState (target, target->info->painstate);
	}

	target->reactiontime = 0;		/* we're awake now...	 */
	if (!target->threshold && source && (source->flags & MF_SHOOTABLE) && !(target->flags & MF_NOINFIGHTING))
	{	/* if not intent on another player, chase after this one */
		target->target = source;
		target->threshold = BASETHRESHOLD;
		if (target->state == &states[target->info->spawnstate] && target->info->seestate != S_000)
        {
			P_SetMobjState (target, target->info->seestate);
        }
	}
}

