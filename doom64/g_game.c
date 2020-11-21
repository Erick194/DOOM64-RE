/* G_game.c  */

#include "doomdef.h"
#include "p_local.h"

void G_PlayerReborn (int player);

void G_DoReborn (int playernum);

void G_DoLoadLevel (void);


gameaction_t    gameaction;                 // 80063230
skill_t         gameskill;                  // 80063234
int             gamemap;                    // 80063238
int				nextmap;				    // 8006323C /* the map to go to after the stats */

//boolean         playeringame[MAXPLAYERS]; //
player_t        players[MAXPLAYERS];        // 80063240

int             consoleplayer;          /* player taking events and displaying  */
int             displayplayer;          /* view being displayed  */
int             gametic;
int             totalkills, totalitems, totalsecret;    /* for intermission  */

//char            demoname[32];
boolean         demorecording;          // 800633A4
boolean         demoplayback;           // 800633A8
int		        *demo_p = NULL, *demobuffer = NULL;   // 8005A180, 8005a184

//mapthing_t	deathmatchstarts[10], *deathmatch_p;    // 80097e4c, 80077E8C
mapthing_t	playerstarts[MAXPLAYERS];   // 800a8c60

/*
==============
=
= G_DoLoadLevel
=
==============
*/

void G_DoLoadLevel (void) // 80004530
{
    if (((gameaction == 7) || (gameaction == 4)) || (players[0].playerstate == PST_DEAD))
        players[0].playerstate = PST_REBORN;

	P_SetupLevel(gamemap, gameskill);
	gameaction = ga_nothing;
}


/*
==============================================================================

						PLAYER STRUCTURE FUNCTIONS

also see P_SpawnPlayer in P_Mobj
==============================================================================
*/

/*
====================
=
= G_PlayerFinishLevel
=
= Can when a player completes a level
====================
*/

void G_PlayerFinishLevel (int player) // 80004598
{
	player_t *p;

	p = &players[player];

	D_memset (p->powers, 0, sizeof (p->powers));
	D_memset (p->cards, 0, sizeof (p->cards));
	p->mo->flags &= ~MF_SHADOW; /* cancel invisibility  */
	p->extralight = 0;                      /* cancel gun flashes  */
	p->damagecount = 0;                     /* no palette changes  */
	p->bonuscount = 0;
    p->bfgcount = 0;
    p->automapflags = 0;
    p->messagetic = 0;
}

/*
====================
=
= G_PlayerReborn
=
= Called after a player dies
= almost everything is cleared and initialized
====================
*/

int gobalcheats = 0; // [GEC]

void G_PlayerReborn (int player) // 80004630
{
	player_t *p;

	p = &players[player];
	D_memset(p, 0, sizeof(*p));

	p->usedown = p->attackdown = true; // don't do anything immediately
	p->playerstate = PST_LIVE;
	p->health = MAXHEALTH;
	p->readyweapon = p->pendingweapon = wp_pistol;
	p->weaponowned[wp_fist] = true;
	p->weaponowned[wp_pistol] = true;
	p->ammo[am_clip] = 50;
    p->maxammo[am_clip] = maxammo[am_clip];
    p->maxammo[am_shell] = maxammo[am_shell];
    p->maxammo[am_cell] = maxammo[am_cell];
    p->maxammo[am_misl] = maxammo[am_misl];

    p->cheats |= gobalcheats; // [GEC] Apply global cheat codes
}

/*============================================================================  */

/*
====================
=
= G_CompleteLevel
=
====================
*/

void G_CompleteLevel (void) // 800046E4
{
	gameaction = ga_completed;
}

/*
====================
=
= G_InitNew
=
====================
*/

extern int ActualConfiguration[13];
mobj_t emptymobj; // 80063158

void G_InitNew (skill_t skill, int map, gametype_t gametype) // 800046F4
{
	//printf ("G_InitNew, skill %d, map %d\n", skill, map);

	/* free all tags except the PU_STATIC tag */
	Z_FreeTags(mainzone, ~PU_STATIC); // (PU_LEVEL | PU_LEVSPEC | PU_CACHE)

	M_ClearRandom ();

/* force players to be initialized upon first level load          */
    players[0].playerstate = PST_REBORN;

/* these may be reset by I_NetSetup */
	gameskill = skill;
	gamemap = map;

	D_memset(&emptymobj, 0, sizeof(emptymobj));
	players[0].mo = &emptymobj;	/* for net consistancy checks */

	demorecording = false;
	demoplayback = false;

	BT_DATA[0] = (buttons_t *)ActualConfiguration;

	/*if (skill == sk_nightmare)
	{
		states[S_SARG_ATK1].tics = 2;
		states[S_SARG_ATK2].tics = 2;
		states[S_SARG_ATK3].tics = 2;
		mobjinfo[MT_SERGEANT].speed = 15;

		mobjinfo[MT_BRUISERSHOT].speed = 40*FRACUNIT;
		mobjinfo[MT_HEADSHOT].speed = 40*FRACUNIT;
		mobjinfo[MT_TROOPSHOT].speed = 40*FRACUNIT;
	}
	else
	{
		states[S_SARG_ATK1].tics = 4;
		states[S_SARG_ATK2].tics = 4;
		states[S_SARG_ATK3].tics = 4;
		mobjinfo[MT_SERGEANT].speed = 10;

		mobjinfo[MT_BRUISERSHOT].speed = 30*FRACUNIT;
		mobjinfo[MT_HEADSHOT].speed = 20*FRACUNIT;
		mobjinfo[MT_TROOPSHOT].speed = 20*FRACUNIT;
	}*/
}

/*============================================================================  */

/*
=================
=
= G_RunGame
=
= The game should allready have been initialized or laoded
=================
*/

void G_RunGame (void) // 80004794
{

	while (1)
	{
        /* load a level */
        G_DoLoadLevel ();

        //printf("RUN P_Start\n");
        //PRINTF_D2(WHITE, 0, 28, "RUN P_Start\n");
		/* run a level until death or completion */
		MiniLoop (P_Start, P_Stop, P_Ticker, P_Drawer);

        //if (gameaction == ga_recorddemo)
            //G_RecordDemo();

        if(gameaction == ga_warped)
			continue; /* skip intermission */

        if ((gameaction == ga_died) || (gameaction == ga_restart))
			continue;			/* died, so restart the level */

        if (gameaction == ga_exitdemo)
            return;

        /* run a stats intermission */
        if (nextmap != 32) {
            MiniLoop(IN_Start, IN_Stop, IN_Ticker, IN_Drawer);
        }

        if(((gamemap ==  8) && (nextmap ==  9)) ||
           ((gamemap ==  4) && (nextmap == 29)) ||
           ((gamemap == 12) && (nextmap == 30)) ||
           ((gamemap == 18) && (nextmap == 31)) ||
           ((gamemap ==  1) && (nextmap == 32)))
        {
            /* run the intermission if needed */
            MiniLoop(F_StartIntermission, F_StopIntermission, F_TickerIntermission, F_DrawerIntermission);

            if(gameaction == ga_warped)
                continue; /* skip intermission */

            if(gameaction == ga_restart)
                continue;

            if (gameaction == ga_exitdemo)
                return;
        }
        else
        {
            if (nextmap >= LASTLEVEL)
            {
                /* run the finale if needed */
                MiniLoop(F_Start, F_Stop, F_Ticker, F_Drawer);

                if(gameaction == ga_warped)
                    continue; /* skip intermission */

                if(gameaction == ga_restart)
                    continue;
                else
                    return;
            }
        }

        /* Set Next Level */
        gamemap = nextmap;
	}
}

int G_PlayDemoPtr (int skill, int map) // 800049D0
{
	int		exit;
	int		config[13];
	int     sensitivity;

	demobuffer = demo_p;

	/* copy key configuration */
	D_memcpy(config, ActualConfiguration, sizeof(config));

	/* set new key configuration */
	D_memcpy(ActualConfiguration, demobuffer, sizeof(config));

	/* copy analog m_sensitivity */
	sensitivity = M_SENSITIVITY;

	/* set new analog m_sensitivity */
	M_SENSITIVITY = demobuffer[13];

	/* skip analog and key configuration */
	demobuffer += 14;

	/* play demo game */
	G_InitNew (skill, map, gt_single);
	G_DoLoadLevel ();
	demoplayback = true;
	exit = MiniLoop (P_Start, P_Stop, P_Ticker, P_Drawer);
	demoplayback = false;

	/* restore key configuration */
	D_memcpy(ActualConfiguration, config, sizeof(config));

	/* restore analog m_sensitivity */
	M_SENSITIVITY = sensitivity;

	/* free all tags except the PU_STATIC tag */
	Z_FreeTags(mainzone, ~PU_STATIC); // (PU_LEVEL | PU_LEVSPEC | PU_CACHE)

	return exit;
}

/*
=================
=
= G_RecordDemo
=
=================
*/

void G_RecordDemo (void)//80013D0C
{
    #if 0
	demo_p = demobuffer = Z_Malloc (0x8000, PU_STATIC, NULL);

	*demo_p++ = startskill;
	*demo_p++ = startmap;

	G_InitNew (startskill, startmap, gt_single);
	G_DoLoadLevel ();
	demorecording = true;
	MiniLoop (P_Start, P_Stop, P_Ticker, P_Drawer);
	demorecording = false;

	D_printf ("w %x,%x",demobuffer,demo_p);

	while (1)
	{
		G_PlayDemoPtr (demobuffer);
	D_printf ("w %x,%x",demobuffer,demo_p);
	}
    #endif
}
