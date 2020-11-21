/* R_data.c */

#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"

int			firsttex;				// 800A632C
int			lasttex;				// 800A6330
int			numtextures;			// 800A6334
int			firstswx;				// 800A6338
int    	    *textures;				// 800A633C

int			firstsprite;			// 800A6320
int			lastsprite;				// 800A6324
int			numsprites;				// 800A6328

int	        skytexture;             // 800A5f14

void R_InitTextures(void);
void R_InitSprites(void);
/*============================================================================ */

#define PI_VAL 3.141592653589793

/*
================
=
= R_InitData
=
= Locates all the lumps that will be used by all views
= Must be called after W_Init
=================
*/

void R_InitData (void) // 80023180
{
    int i;
    int val;

    for(i = 0; i < (5*FINEANGLES/4); i++)
    {
        finesine[i] = (fixed_t) (sinf((((f64) val * (f64) PI_VAL) / 8192.0)) * 65536.0);
        val += 2;
    }

	R_InitTextures();
    R_InitSprites();
}

/*
==================
=
= R_InitTextures
=
= Initializes the texture list with the textures from the world map
=
==================
*/

void R_InitTextures(void) // 8002327C
{
	int lump, swx, i;

	firsttex = W_GetNumForName("T_START") + 1;
	lasttex = W_GetNumForName("T_END") - 1;
	numtextures = (lasttex - firsttex) + 1;

	textures = Z_Malloc(numtextures * sizeof(int), PU_STATIC, NULL);

	for (i = 0; i < numtextures; i++)
	{
	    textures[i] = (i + firsttex) << 4;
	}

    swx = W_CheckNumForName("SWX", 0x7fffff00, 0);
    firstswx = (swx - firsttex);
}

/*
================
=
= R_InitSprites
=
=================
*/

void R_InitSprites(void) // 80023378
{
	firstsprite = W_GetNumForName("S_START") + 1;
	lastsprite = W_GetNumForName("S_END") - 1;
	numsprites = (lastsprite - firstsprite) + 1;
}
