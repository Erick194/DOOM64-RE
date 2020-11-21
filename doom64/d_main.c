/* D_main.c  */

#include "i_main.h"
#include "doomdef.h"
#include "p_spec.h"
#include "r_local.h"

int gamevbls;		            // 80063130 /* may not really be vbls in multiplayer */
int gametic;		            // 80063134
int ticsinframe;                // 80063138 /* how many tics since last drawer */
int ticon;			            // 8006313C
int lastticon;                  // 80063140
int vblsinframe[MAXPLAYERS];	// 80063144 /* range from 4 to 8 */
int ticbuttons[MAXPLAYERS];		// 80063148
int oldticbuttons[MAXPLAYERS];	// 8006314C

buttons_t   *BT_DATA[MAXPLAYERS];// 800A559C

extern boolean run_hectic_demo;

void D_DoomMain(void) // 800027C0
{
    int exit;

    I_Init();
    Z_Init();
    W_Init();
    R_Init();
    ST_Init();

    gamevbls = 0;
    gametic = 0;
    ticsinframe = 0;
    ticon = 0;
    ticbuttons[0] = 0;
    oldticbuttons[0] = 0;

    D_SplashScreen();

    while(true)
    {
        exit = D_TitleMap();

        if(exit != ga_exit)
        {
            exit = D_RunDemo("DEMO1LMP", sk_medium, 3);
            if(exit != ga_exit)
            {
                exit = D_RunDemo("DEMO2LMP", sk_medium, 9);
                if(exit != ga_exit)
                {
                    exit = D_RunDemo("DEMO3LMP", sk_medium, 17);
                    if(exit != ga_exit)
                    {
                        if(run_hectic_demo)
                        {
                            run_hectic_demo = false;
                            exit = D_RunDemo("DEMO4LMP", sk_medium, 32);
                        }

                        if(exit != ga_exit)
                        {
                            exit = D_Credits();

                            if(exit != ga_exit)
                            {
                                continue;
                            }
                        }
                    }
                }
            }
        }

        do {
            exit = M_RunTitle();
        } while(exit != ga_timeout);
    }
}

/*
===============
=
= M_Random
=
= Returns a 0-255 number
=
===============
*/

unsigned char rndtable[256] = { // 8005A190
	0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66 ,
	74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36 ,
	95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188 ,
	52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224 ,
	149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242 ,
	145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0 ,
	175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235 ,
	25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113 ,
	94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75 ,
	136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196 ,
	135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113 ,
	80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241 ,
	24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224 ,
	145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95 ,
	28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226 ,
	71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36 ,
	17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106 ,
	197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136 ,
	120, 163, 236, 249
};

int	rndindex = 0;   // 8005A18C
int prndindex = 0;  // 8005A188

int P_Random(void) // 80002928
{
	prndindex = (prndindex + 1) & 0xff;
	return rndtable[prndindex];
}

int M_Random(void) // 80002954
{
	rndindex = (rndindex + 1) & 0xff;
	return rndtable[rndindex];
}

void M_ClearRandom(void) // 80002980
{
	rndindex = prndindex = 0;
}

/*
===============
=
= MiniLoop
=
===============
*/

int MiniLoop(void(*start)(void), void(*stop)(),
             int(*ticker)(void), void(*drawer)(void)) // 80002998
{
	int		exit;
	int		buttons;

	gameaction = ga_nothing;
	gamevbls = 0;
	gametic = 0;
	ticon = 0;
	ticsinframe = 0;

	/* */
	/* setup (cache graphics, etc) */
	/* */
	if(start != NULL)
        start();

	drawsync1 = 0;
	drawsync2 = vsync;

	while (true)
	{
		vblsinframe[0] = drawsync1;

		// get buttons for next tic
		oldticbuttons[0] = ticbuttons[0];

		buttons = I_GetControllerData();
		ticbuttons[0] = buttons;

		//Read|Write demos
		if (demorecording || demoplayback)
        {
            if (demoplayback)
            {
                if (buttons & (ALL_JPAD|ALL_BUTTONS))
                {
                    exit = ga_exit;
                    break;
                }

                buttons = *demobuffer++;
                ticbuttons[0] = buttons;
            }

            if (demorecording)
            {
                *demobuffer++ = buttons;
            }

            if ((buttons & PAD_START) || ((((int)demobuffer - (int)demo_p) >> 2) >= 4000))
            {
                exit = ga_exitdemo;
                break;
            }
        }

		ticon += vblsinframe[0];
		if (ticsinframe < (ticon >> 1))
		{
			gametic += 1;
			ticsinframe = (ticon >> 1);
		}

        if (disabledrawing == false)
        {
            exit = ticker();
            if (exit != ga_nothing)
                break;

            drawer();
        }

		gamevbls = gametic;
	}

	I_GetScreenGrab();

	if(stop != NULL)
        stop(exit);

	oldticbuttons[0] = ticbuttons[0];

	return exit;
}
