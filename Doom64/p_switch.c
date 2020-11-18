#include "doomdef.h"
#include "p_local.h"

/*================================================================== */
/* */
/*	CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE */
/* */
/*================================================================== */

button_t    buttonlist[MAXBUTTONS];//800975B0

void P_StartButton(line_t *line,bwhere_e w,int texture,int time);

/*================================================================== */
/* */
/*	Function that changes wall texture. */
/*	Tell it if switch is ok to use again (1=yes, it's a button). */
/* */
/*================================================================== */
void P_ChangeSwitchTexture(line_t *line,int useAgain) // 80021460
{
	int	sound;
	int	swx;

	/* 52  EXIT! */
	/* 124 Secret EXIT */
	if(SPECIALMASK(line->special) == 52 || SPECIALMASK(line->special) == 124)
        sound = sfx_switch2;//sfx_swtchx
    else
        sound = sfx_switch1;//sfx_swtchn

    if(SWITCHMASK(line->flags) == (ML_SWITCHX02 | ML_SWITCHX04)) /* Mid */
    {
        S_StartSound((mobj_t *)&line->frontsector->soundorg, sound);

        swx = sides[line->sidenum[0]].midtexture;
        sides[line->sidenum[0]].midtexture = ((swx - firstswx) ^ 1) + firstswx;

        if (useAgain)
            P_StartButton(line, middle, swx, BUTTONTIME);
    }
    else if(SWITCHMASK(line->flags) == (ML_SWITCHX02)) /* Top */
    {
        S_StartSound((mobj_t *)&line->frontsector->soundorg, sound);

        swx = sides[line->sidenum[0]].toptexture;
        sides[line->sidenum[0]].toptexture = ((swx - firstswx) ^ 1) + firstswx;

        if (useAgain)
            P_StartButton(line, top ,swx, BUTTONTIME);
    }
    else if(SWITCHMASK(line->flags) == (ML_SWITCHX04)) /* Bot */
    {
        S_StartSound((mobj_t *)&line->frontsector->soundorg, sound);

        swx = sides[line->sidenum[0]].bottomtexture;
        sides[line->sidenum[0]].bottomtexture = ((swx - firstswx) ^ 1) + firstswx;

        if (useAgain)
            P_StartButton(line, bottom, swx, BUTTONTIME);
    }
}

/*================================================================== */
/* */
/*	Start a button counting down till it turns off. */
/* */
/*================================================================== */
void P_StartButton(line_t *line,bwhere_e w,int texture,int time) // 80021608
{
	int i;

	for (i = 0;i < MAXBUTTONS;i++)
    {
		if (buttonlist[i].btimer <= 0)
		{
			buttonlist[i].side = &sides[line->sidenum[0]];
			buttonlist[i].where = w;
			buttonlist[i].btexture = texture;
			buttonlist[i].btimer = time;
			buttonlist[i].soundorg = (mobj_t *)&line->frontsector->soundorg;
			return;
		}
    }

	//I_Error("P_StartButton: no button slots left!");
}
