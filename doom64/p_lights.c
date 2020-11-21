#include "doomdef.h"
#include "p_local.h"

/*================================================================== */
/*================================================================== */
/* */
/*							BROKEN LIGHT FLASHING */
/* */
/*================================================================== */
/*================================================================== */

/*================================================================== */
/* */
/*	T_FireFlicker */
/* */
/*	Exclusive Psx Doom From PC Doom */
/* */
/*================================================================== */

void T_FireFlicker(fireflicker_t *flick) // 80015740
{
	int	amount;

	if (--flick->count)
		return;

    if(flick->sector->special != flick->special)
    {
        P_RemoveThinker(&flick->thinker);
        return;
    }

    amount = (P_Random() & 31);
    flick->sector->lightlevel = amount;
    flick->count = 3;
}

/*================================================================== */
/* */
/*	P_SpawnFireFlicker */
/* */
/*================================================================== */

void P_SpawnFireFlicker(sector_t *sector) // 800157B4
{
	fireflicker_t *flick;

	flick = Z_Malloc(sizeof(*flick), PU_LEVSPEC, 0);
	P_AddThinker(&flick->thinker);
	flick->thinker.function = T_FireFlicker;
	flick->sector = sector;
	flick->special = sector->special;
	flick->count = 3;
}

/*================================================================== */
/* */
/*	Spawn glowing light */
/* */
/*================================================================== */

void T_Glow(glow_t *g) // 80015820
{
    if(--g->count)
        return;

    if(g->sector->special != g->special)
    {
        P_RemoveThinker(&g->thinker);
        return;
    }

    g->count = 2;

	switch(g->direction)
	{
		case -1:		/* DOWN */
			g->sector->lightlevel -= GLOWSPEED;
			if (g->sector->lightlevel < g->minlight)
			{
				g->sector->lightlevel = g->minlight;

				if(g->type == PULSERANDOM)
                    g->maxlight = (P_Random() & 31) + 17;

				g->direction = 1;
			}
			break;
		case 1:			/* UP */
			g->sector->lightlevel += GLOWSPEED;
			if (g->maxlight < g->sector->lightlevel)
			{
				g->sector->lightlevel = g->maxlight;

				if(g->type == PULSERANDOM)
                    g->minlight = (P_Random() & 15);

				g->direction = -1;
			}
			break;
	}
}

/*================================================================== */
/* */
/*	P_SpawnGlowingLight */
/* */
/*================================================================== */

void P_SpawnGlowingLight(sector_t *sector, glowtype_e type) // 80015968
{
	glow_t *g;

	g = Z_Malloc(sizeof(*g), PU_LEVSPEC, 0);
	P_AddThinker(&g->thinker);
	g->thinker.function = T_Glow;
	g->sector = sector;
	g->count = 2;
	g->direction = 1;
	g->minlight = 0;
	g->type = type;
	g->special = sector->special;

	switch (type)
	{
	case PULSENORMAL:
		g->maxlight = 32;
		break;
	case PULSESLOW:
	case PULSERANDOM:
		g->maxlight = 48;
		break;
	}
}

/*================================================================== */
/* */
/*	T_LightFlash */
/* */
/*	After the map has been loaded, scan each sector for specials */
/*	that spawn thinkers */
/* */
/*================================================================== */
void T_LightFlash (lightflash_t *flash) // 80015A14
{
	if (--flash->count)
		return;

    if(flash->sector->special != flash->special)
    {
        P_RemoveThinker(&flash->thinker);
        return;
    }

	if (flash->sector->lightlevel == 32)
	{
		flash->sector->lightlevel = 0;
		flash->count = (P_Random()&7)+1;
	}
	else
	{
		flash->sector->lightlevel = 32;
		flash->count = (P_Random()&32)+1;
	}
}

/*================================================================== */
/* */
/*	P_SpawnLightFlash */
/* */
/*	After the map has been loaded, scan each sector for specials that spawn thinkers */
/* */
/*================================================================== */
void P_SpawnLightFlash (sector_t *sector) // 80015AB4
{
	lightflash_t	*flash;

	sector->special = 0;		/* nothing special about it during gameplay */

	flash = Z_Malloc ( sizeof(*flash), PU_LEVSPEC, 0);
	P_AddThinker (&flash->thinker);
	flash->thinker.function = T_LightFlash;
	flash->sector = sector;
	flash->special = sector->special;
	flash->count = (P_Random()&63)+1;
}

/*================================================================== */
/* */
/*							STROBE LIGHT FLASHING */
/* */
/*================================================================== */

/*================================================================== */
/* */
/*	T_StrobeFlash */
/* */
/*	After the map has been loaded, scan each sector for specials that spawn thinkers */
/* */
/*================================================================== */
void T_StrobeFlash (strobe_t *flash) // 80015B28
{
	if (--flash->count)
		return;

    if(flash->sector->special != flash->special)
    {
        P_RemoveThinker(&flash->thinker);
        return;
    }

	if (flash->sector->lightlevel == 0)
	{
		flash->sector->lightlevel = flash->maxlight;
		flash->count = flash->brighttime;
	}
	else
	{
		flash->sector->lightlevel = 0;
		flash->count = flash->darktime;
	}
}

/*================================================================== */
/* */
/*	P_SpawnLightFlash */
/* */
/*	After the map has been loaded, scan each sector for specials that spawn thinkers */
/* */
/*================================================================== */

void P_SpawnStrobeFlash (sector_t *sector,int fastOrSlow) // 80015BB4
{
	strobe_t	*flash;

	flash = Z_Malloc ( sizeof(*flash), PU_LEVSPEC, 0);
	P_AddThinker (&flash->thinker);
	flash->thinker.function = T_StrobeFlash;
	flash->sector = sector;
	flash->special = sector->special;
	flash->brighttime = STROBEBRIGHT;
	flash->maxlight = 16;
	flash->darktime = fastOrSlow;
    flash->count = (P_Random()&7)+1;
}

/*================================================================== */
/* */
/*	P_SpawnStrobeAltFlash */
/*	Alternate variation of P_SpawnStrobeFlash */
/* */
/*================================================================== */
void P_SpawnStrobeAltFlash (sector_t *sector,int fastOrSlow) // 80015C44
{
	strobe_t	*flash;

	flash = Z_Malloc ( sizeof(*flash), PU_LEVSPEC, 0);
	P_AddThinker (&flash->thinker);
	flash->thinker.function = T_StrobeFlash;
	flash->sector = sector;
	flash->special = sector->special;
	flash->brighttime = STROBEBRIGHT2;
	flash->maxlight = 127;
	flash->darktime = fastOrSlow;
    flash->count = 1;
}

/*================================================================== */
/* */
/*	Start strobing lights (usually from a trigger) */
/* */
/*================================================================== */
int EV_StartLightStrobing(line_t *line) // 80015CC4
{
	int	secnum;
	sector_t	*sec;
	int ok;

	ok = 0;
	secnum = -1;
	while ((secnum = P_FindSectorFromLineTag(line->tag,secnum)) >= 0)
	{
		sec = &sectors[secnum];
		if (sec->specialdata)
			continue;

        ok = 1;
		P_SpawnStrobeFlash (sec,SLOWDARK);
	}

	return ok;
}

/*================================================================== */
/* */
/*							SPECIALS FUNCTIONS */
/* */
/*================================================================== */

/*================================================================== */
/* */
/*	Changes a sector light index */
/*	This doesn't appear to be used at all */
/* */
/*================================================================== */
int P_ModifySectorColor(line_t* line, int index, int type) // 80015D6C
{
    int secnum;
    int rtn;
    sector_t* sec;

    secnum = -1;
    rtn = 0;

    while((secnum = P_FindSectorFromLineTag(line->tag, secnum)) >= 0)
    {
        sec = &sectors[secnum];
        rtn = 1;

        switch(type)
        {
        case 0:
            sec->colors[1] = index;
            break;
        case 1:
            sec->colors[0] = index;
            break;
        case 2:
            sec->colors[2] = index;
            break;
        case 3:
            sec->colors[3] = index;
            break;
        case 4:
            sec->colors[4] = index;
            break;
        }
    }

    return rtn;
}

/*================================================================== */
/* */
/*	T_SequenceGlow */
/* */
/*================================================================== */
void T_SequenceGlow(sequenceglow_t *seq) // 80015E5C
{
    sector_t *next;
    int i;

    if(--seq->count)
        return;

    if(seq->sector->special != seq->special)
    {
        P_RemoveThinker(&seq->thinker);
        return;
    }

    seq->count = 1;

    switch(seq->start)
	{
		case -1:		/* DOWN */
			seq->sector->lightlevel -= GLOWSPEED;

			if(seq->sector->lightlevel > 0)
                return;

            seq->sector->lightlevel = 0;

            if (seq->headsector == NULL)
            {
                seq->sector->special = 0;
                P_RemoveThinker(&seq->thinker);
                return;
            }

            seq->start = 0;
			break;
        case 0:		    /* CHECK */
            if(!seq->headsector->lightlevel)
                return;

            seq->start = 1;
            break;
		case 1:			/* UP */
			seq->sector->lightlevel += GLOWSPEED;
			if (seq->sector->lightlevel < (SEQUENCELIGHTMAX + 1))
			{
                if(seq->sector->lightlevel != 8)
                    return;

                if(seq->sector->linecount <= 0)
                    return;

                for(i = 0; i < seq->sector->linecount; i++)
                {
                    next = seq->sector->lines[i]->backsector;

                    if(next && (next->special == 0))
                    {
                        if(next->tag == (seq->sector->tag + 1))
                        {
                            next->special = seq->sector->special;
                            P_SpawnSequenceLight(next, false);
                        }
                    }
                }
			}
			else
			{
                seq->sector->lightlevel = SEQUENCELIGHTMAX;
                seq->start = -1;
			}
			break;
	}
}

/*================================================================== */
/* */
/*	P_SpawnSequenceLight */
/* */
/*================================================================== */
void P_SpawnSequenceLight(sector_t* sector, boolean first) // 80016038
{
    sequenceglow_t *seq;
    sector_t *headsector;
    int i;

    headsector = NULL;

    if(first)
    {
        for(i = 0; i < sector->linecount; i++)
        {
            headsector = sector->lines[i]->frontsector;

            if ((headsector != sector) && (sector->tag == headsector->tag))
                break;
        }

        if(headsector == NULL)
            return;
    }

    seq = Z_Malloc ( sizeof(*seq), PU_LEVSPEC, 0);
	P_AddThinker (&seq->thinker);
	seq->thinker.function = T_SequenceGlow;
	seq->sector = sector;
	seq->special = sector->special;
    seq->count = 1;
    seq->index = sector->tag;
    seq->start = (headsector == NULL ? 1 : 0);
    seq->headsector = headsector;
}

/*================================================================== */
/* */
/*	P_UpdateLightThinker */
/* */
/*================================================================== */

extern maplights_t *maplights;     // 800A5EA4

void P_UpdateLightThinker(int destlight, int srclight) // 80016118
{
    lightmorph_t *lt;
    byte r, g, b;
    int rgb;

    r = g = b = (byte)destlight;

    if (destlight > 255)
    {
        r = maplights[destlight-256].r;
        g = maplights[destlight-256].b;
        b = maplights[destlight-256].g;
    }

    maplights[srclight-256].r = r;
    maplights[srclight-256].b = g;
    maplights[srclight-256].g = b;

    rgb = lights[srclight].rgba;

    lt = Z_Malloc ( sizeof(*lt), PU_LEVSPEC, 0);
    P_AddThinker(&lt->thinker);
    lt->thinker.function = T_LightMorph;
    lt->inc = 0;
    lt->dest = destlight;
    lt->src = srclight;
    lt->r = (rgb >> 24) & 0xff;
    lt->g = (rgb >> 16) & 0xff;
    lt->b = (rgb >>  8) & 0xff;
}

/*================================================================== */
/* */
/*	T_LightMorph */
/* */
/*================================================================== */
void T_LightMorph(lightmorph_t *lt) // 80016244
{
    int rgb;

    lt->inc += 4;

    if(lt->inc > 256)
    {
        P_RemoveThinker(&lt->thinker);
        return;
    }

    rgb = lights[lt->dest].rgba;
    lights[lt->src].rgba =
    (lt->r + ((lt->inc * (((rgb >> 24) & 0xff) - lt->r)) >> 8)) << 24 |
    (lt->g + ((lt->inc * (((rgb >> 16) & 0xff) - lt->g)) >> 8)) << 16 |
    (lt->b + ((lt->inc * (((rgb >>  8) & 0xff) - lt->b)) >> 8)) <<  8 | 0xff;
}

/*================================================================== */
/* */
/*	P_ChangeLightByTag */
/*	This has to be early stuff because this is only */
/*	used once in the entire game and the only place where */
/*	internal light tags are used.. */
/* */
/*================================================================== */
int P_ChangeLightByTag(int tag1, int tag2) // 80016320
{
    int destlight;
    int srclight;
    int rtn;

    destlight = P_FindLightFromLightTag(tag1,-1);
    if (destlight < 0)
        return 0;

    srclight = -1;
    rtn = 0;

    while((srclight = P_FindLightFromLightTag(tag2, srclight)) >= 256)
    {
        rtn = 1;
        P_UpdateLightThinker(destlight, srclight);
    }

    return rtn;
}

/*================================================================== */
/* */
/*	P_DoSectorLightChange */
/* */
/*================================================================== */
int P_DoSectorLightChange(int tag1,int tag2) // 800163B8
{
    sector_t *sec1;
    sector_t *sec2;
    int secnum;
    int rtn;

    secnum = P_FindSectorFromLineTag(tag1, -1);
    if (secnum < 0)
        return 0;

    sec2 = &sectors[secnum];

    secnum = -1;
    rtn = 0;

    while((secnum = P_FindSectorFromLineTag(tag2, secnum)) >= 0)
    {
        sec1 = &sectors[secnum];
        rtn = 1;

        if (sec1->colors[0] >= 256)
            P_UpdateLightThinker(sec2->colors[0], sec1->colors[0]);

        if (sec1->colors[1] >= 256)
            P_UpdateLightThinker(sec2->colors[1], sec1->colors[1]);

        if (sec1->colors[2] >= 256)
            P_UpdateLightThinker(sec2->colors[2], sec1->colors[2]);

        if (sec1->colors[3] >= 256)
            P_UpdateLightThinker(sec2->colors[3], sec1->colors[3]);

        if (sec1->colors[4] >= 256)
            P_UpdateLightThinker(sec2->colors[4], sec1->colors[4]);
    }

    return rtn;
}

/*================================================================== */
/* */
/*	T_Combine */
/* */
/*================================================================== */
void T_Combine(combine_t *combine) // 80016524
{
    sector_t *sector;

    sector = combine->sector;
    if (sector->special != combine->special)
    {
        P_RemoveThinker(&combine->thinker);
        return;
    }

    sector->lightlevel = combine->combiner->lightlevel;
}

/*================================================================== */
/* */
/*	P_CombineLightSpecials */
/* */
/*================================================================== */
void P_CombineLightSpecials(sector_t *sector) // 80016578
{
    think_t func;
	thinker_t *thinker;
	combine_t *combine;

	switch(sector->special)
	{
		case 1:
			func = T_LightFlash;
			break;
		case 2:
        case 3:
        case 202:
        case 204:
        //case 205:
        case 206:
        case 208:
			func = T_StrobeFlash;
			break;
		case 8:
        case 9:
        case 11:
			func = T_Glow;
			break;
		case 17:
			func = T_FireFlicker;
			break;
		default:
			return;
	}

	for(thinker = thinkercap.next; thinker != &thinkercap; thinker = thinker->next)
	{
		if(func != thinker->function)
			continue;

        combine = Z_Malloc ( sizeof(*combine), PU_LEVSPEC, 0);
        P_AddThinker(&combine->thinker);
        combine->thinker.function = T_Combine;
        combine->sector = sector;
        combine->combiner = ((combine_t *)thinker)->sector;
        combine->special = sector->special;
		return;
	}
}
