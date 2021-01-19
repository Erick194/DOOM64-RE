/* m_main.c -- menu routines */

#include "doomdef.h"
#include "r_local.h"
#include "st_main.h"

//intermission
int DrawerStatus;

#define CT_TXT00	"default: %d"
#define CT_TXT01	"right"
#define CT_TXT02	"left"
#define CT_TXT03	"forward"
#define CT_TXT04	"backward"
#define CT_TXT05	"attack"
#define CT_TXT06	"use"
#define CT_TXT07	"map"
#define CT_TXT08	"speed"
#define CT_TXT09	"strafe on"
#define CT_TXT10	"strafe left"
#define CT_TXT11	"strafe right"
#define CT_TXT12	"weapon backward"
#define CT_TXT13	"weapon forward"

char *ControlText[] =   //8007517C
{
    CT_TXT00, CT_TXT01, CT_TXT02, CT_TXT03, CT_TXT04,
	CT_TXT05, CT_TXT06, CT_TXT07, CT_TXT08, CT_TXT09,
	CT_TXT10, CT_TXT11, CT_TXT12, CT_TXT13
};

#define M_TXT00	"Control Pad"
#define M_TXT01	"Volume"
#define M_TXT02	"Display"
#define M_TXT03	"Password"
#define M_TXT04	"Main Menu"
#define M_TXT05	"Restart Level"
#define M_TXT06	"\x90 Return"
#define M_TXT07 "Music Volume"
#define M_TXT08 "Sound Volume"
#define M_TXT09 "Brightness"
#define M_TXT10 "Resume"
#define M_TXT11 "Options"
#define M_TXT12 "Default"
#define M_TXT13 "Default"
#define M_TXT14 "New Game"
#define M_TXT15 "Be Gentle!"
#define M_TXT16 "Bring it on!"
#define M_TXT17 "I own Doom!"
#define M_TXT18 "Watch me die!"
#define M_TXT19 "Nightmare!"
#define M_TXT20 "Yes"
#define M_TXT21 "No"
#define M_TXT22 "Features"
#define M_TXT23 "WARP TO LEVEL"
#define M_TXT24 "INVULNERABLE"
#define M_TXT25 "HEALTH BOOST"
#define M_TXT26 "SECURITY KEYS"
#define M_TXT27 "WEAPONS"
#define M_TXT28 "Exit"
#define M_TXT29 "DEBUG"
#define M_TXT30 "TEXTURE TEST"
#define M_TXT31 "WALL BLOCKING"
#define M_TXT32 "Center Display"
#define M_TXT33 "Messages:"
#define M_TXT34 "Status Bar:"
#define M_TXT35 "LOCK MONSTERS"
#define M_TXT36 "SCREENSHOT"
#define M_TXT37 "MAP EVERYTHING"
#define M_TXT38 "MACRO PEEK"
#define M_TXT39 "MUSIC TEST"
#define M_TXT40 "WARP TO FUN"
#define M_TXT41 "Control Stick"
#define M_TXT42 "Default"
#define M_TXT43 "Sensitivity"
#define M_TXT44 "Manage Pak"
#define M_TXT45 "Do not use Pak"
#define M_TXT46 "Try again"
#define M_TXT47 "Create game note"

#define M_TXT48 "COLORS"     // [GEC] NEW CHEAT CODE
#define M_TXT49 "FULL BRIGHT"   // [GEC] NEW CHEAT CODE
#define M_TXT50 "FILTER"   // [GEC] NEW CHEAT CODE

char *MenuText[] =   // 8005ABA0
{
    M_TXT00, M_TXT01, M_TXT02, M_TXT03, M_TXT04,
    M_TXT05, M_TXT06, M_TXT07, M_TXT08, M_TXT09,
    M_TXT10, M_TXT11, M_TXT12, M_TXT13, M_TXT14,
    M_TXT15, M_TXT16, M_TXT17, M_TXT18, M_TXT19,
    M_TXT20, M_TXT21, M_TXT22, M_TXT23, M_TXT24,
    M_TXT25, M_TXT26, M_TXT27, M_TXT28, M_TXT29,
    M_TXT30, M_TXT31, M_TXT32, M_TXT33, M_TXT34,
    M_TXT35, M_TXT36, M_TXT37, M_TXT38, M_TXT39,
    M_TXT40, M_TXT41, M_TXT42, M_TXT43, M_TXT44,
    M_TXT45, M_TXT46, M_TXT47,
    M_TXT48, M_TXT49, M_TXT50  // [GEC] NEW
};

menuitem_t Menu_Title[2] = // 8005A978
{
    { 14, 115, 190 },   // New Game
	{ 11, 115, 210 },   // Options
};

#if ENABLE_NIGHTMARE == 1
menuitem_t Menu_Skill[5] = // 8005A990
#else
menuitem_t Menu_Skill[4] = // 8005A990
#endif // ENABLE_NIGHTMARE
{
    { 15, 102, 80 },    // Be Gentle!
    { 16, 102, 100},    // Bring it on!
    { 17, 102, 120},    // I own Doom!
    { 18, 102, 140},    // Watch me die!
    /* Disable on Doom 64 Original */
    #if ENABLE_NIGHTMARE == 1
    { 19, 102, 160},    // Nightmare
    #endif // ENABLE_NIGHTMARE
};

menuitem_t Menu_Options[6] = // 8005A9C0
{
    {  0, 102, 60 },    // Control Pad
    { 41, 102, 80 },    // Control Stick
    {  1, 102, 100},    // Volume
    {  2, 102, 120},    // Display
    {  3, 102, 140},    // Password
    {  6, 102, 160},    // Return
};

menuitem_t Menu_Volume[4] = // 8005AA08
{
    {  7, 102, 60 },    // Music Volume
    {  8, 102, 100},    // Sound Volume
    { 12, 102, 140},    // Default Volume
    {  6, 102, 160},    // Return
};

menuitem_t Menu_ControlStick[3] = // 8005AA38
{
    { 43, 102, 90 },    // Sensitivity
    { 42, 102, 130},    // Default Sensitivity
    {  6, 102, 150},    // Return
};

menuitem_t Menu_Display[6] = // 8005AA5C
{
    {  9, 102, 60 },    // Brightness
    { 32, 102, 100},    // Center Display
    { 33, 102, 120},    // Messages
    { 34, 102, 140},    // Status Bar
    { 13, 102, 160},    // Default Display
    {  6, 102, 180},    // Return
};

menuitem_t Menu_Game[4] = // 8005AAA4
{
    { 11, 122, 80 },    // Options
    {  4, 122, 100},    // Main Menu
    {  5, 122, 120},    // Restart Level
    { 22, 122, 140},    // Features
};

menuitem_t Menu_Quit[2] = // 8005AAD4
{
    { 20, 142, 100},    // Yes
    { 21, 142, 120},    // No
};

menuitem_t Menu_DeleteNote[2] = // 8005AAEC
{
    { 20, 142, 100},    // Yes
    { 21, 142, 120},    // No
};

menuitem_t Menu_ControllerPakBad[2] = // 8005AB04
{
    { 46, 120, 100},    // Try again
    { 45, 120, 120},    // Do not use Pak
};

menuitem_t Menu_ControllerPakFull[3] = // 8005AB1C
{
    { 44, 110, 90 },    // Manage Pak
    { 47, 110, 110},    // Create game note
    { 45, 110, 130},    // Do not use Pak
};

menuitem_t Menu_CreateNote[3] = // 8005AB40
{
    { 20, 110, 90 },    // Yes
    { 45, 110, 110},    // Do not use Pak
    { 44, 110, 130},    // Manage Pak
};

//#define MAXFEATURES 5
//#define MAXFEATURES 9
#define MAXFEATURES 12
menuitem_t Menu_Features[MAXFEATURES] = // 8005AB64
{
    { 23, 40, 50},      // WARP TO LEVEL
    { 24, 40, 60},      // INVULNERABLE
    { 25, 40, 70},      // HEALTH BOOST
    { 27, 40, 80},      // WEAPONS
    { 37, 40, 90},      // MAP EVERYTHING
    //
    { 26, 40, 100},      // SECURITY KEYS
    { 31, 40, 110},      // WALL BLOCKING
    { 35, 40, 120},      // LOCK MONSTERS
    { 39, 40, 130},      // MUSIC TEST
    //
    { 48, 40, 140},      // COLORS [GEC] NEW CHEAT CODE
    { 49, 40, 150},      // FULL BRIGHT [GEC] NEW CHEAT CODE
    { 50, 40, 160},      // FILTER [GEC] NEW CHEAT CODE

    // no usados
//#define M_TXT26 "SECURITY KEYS"
//#define M_TXT28 "Exit"
//#define M_TXT29 "DEBUG"
//#define M_TXT30 "TEXTURE TEST"
//#define M_TXT31 "WALL BLOCKING"
//#define M_TXT35 "LOCK MONSTERS"
//#define M_TXT36 "SCREENSHOT"
//#define M_TXT38 "MACRO PEEK"
//#define M_TXT39 "MUSIC TEST"
//#define M_TXT40 "WARP TO FUN"

};

menudata_t MenuData[8]; // 800A54F0
int MenuAnimationTic;   // 800a5570
int cursorpos;          // 800A5574
int m_vframe1;          // 800A5578
menuitem_t *MenuItem;   // 800A5578
int itemlines;          // 800A5580
menufunc_t MenuCall;    // 800A5584

int linepos;            // 800A5588
int text_alpha_change_value;    // 800A558C
int MusicID;            // 800A5590
int m_actualmap;        // 800A5594
int last_ticon;         // 800A5598

skill_t startskill;     // 800A55A0
int startmap;           // 800A55A4
int EnableExpPak;       // 800A55A8

//-----------------------------------------

int MenuIdx = 0;                // 8005A7A4
int text_alpha = 255;           // 8005A7A8
int ConfgNumb = 0;              // 8005A7AC
int Display_X = 0;              // 8005A7B0
int Display_Y = 0;              // 8005A7B4
boolean enable_messages = true; // 8005A7B8
boolean enable_statusbar = true;// 8005A7BC
int SfxVolume = 80;             // 8005A7C0
int MusVolume = 80;             // 8005A7C4
int brightness = 0;             // 8005A7C8
int M_SENSITIVITY = 0;          // 8005A7CC
boolean FeaturesUnlocked = false; // 8005A7D0

int TempConfiguration[13] = // 8005A80C
{
    PAD_LEFT, PAD_RIGHT, PAD_UP, PAD_DOWN,
    PAD_LEFT_C, PAD_RIGHT_C, PAD_UP_C, PAD_DOWN_C,
    PAD_L_TRIG, PAD_R_TRIG, PAD_A, PAD_B, PAD_Z_TRIG
};

int ActualConfiguration[13] = // 8005A840
{
    PAD_RIGHT, PAD_LEFT, PAD_UP, PAD_DOWN,
    PAD_Z_TRIG,
    PAD_RIGHT_C, PAD_UP_C, PAD_LEFT_C, PAD_DOWN_C,
    PAD_L_TRIG, PAD_R_TRIG, PAD_A, PAD_B
};

int DefaultConfiguration[5][13] = // 8005A840
{
    // Default 1
    {
        PAD_RIGHT, PAD_LEFT, PAD_UP, PAD_DOWN,
        PAD_Z_TRIG,
        PAD_RIGHT_C, PAD_UP_C, PAD_LEFT_C, PAD_DOWN_C,
        PAD_L_TRIG, PAD_R_TRIG, PAD_A, PAD_B
    },

    // Default 2
    {
        PAD_RIGHT, PAD_LEFT, PAD_UP, PAD_DOWN,
        PAD_Z_TRIG,
        PAD_RIGHT_C, PAD_UP_C,
        PAD_R_TRIG, PAD_L_TRIG,
        PAD_A, PAD_DOWN_C,
        PAD_B, PAD_LEFT_C
    },

    // Default 3
    {
        PAD_RIGHT, PAD_LEFT, PAD_UP, PAD_DOWN,
        PAD_Z_TRIG,
        PAD_UP_C, PAD_UP,
        PAD_R_TRIG,
        PAD_DOWN, PAD_LEFT_C, PAD_RIGHT_C,
        PAD_A, PAD_B
    },

    // Default 4
    {
        PAD_RIGHT_C, PAD_LEFT_C,
        PAD_UP, PAD_DOWN,
        PAD_Z_TRIG,
        PAD_UP, PAD_UP_C,
        PAD_L_TRIG,
        PAD_DOWN_C, PAD_LEFT, PAD_RIGHT,
        PAD_A, PAD_B
    },

    // Default 5
    {
        PAD_RIGHT, PAD_LEFT, PAD_UP, PAD_DOWN,
        PAD_A, PAD_RIGHT_C, PAD_UP_C, PAD_DOWN_C,
        PAD_Z_TRIG, PAD_L_TRIG, PAD_R_TRIG,
        PAD_B, PAD_LEFT_C
    }
};

//-----------------------------------------

int M_RunTitle(void) // 80007630
{
    int exit;

    DrawerStatus = 0;
    startskill = sk_easy;
    startmap = 1;
    MenuIdx = 0;
    MenuItem = Menu_Title;
    MenuCall = M_MenuTitleDrawer;
    text_alpha = 0;
    itemlines = 2;
    cursorpos = 0;
    last_ticon = 0;

    S_StartMusic(116);

    exit = MiniLoop(M_FadeInStart, M_MenuClearCall, M_MenuTicker, M_MenuGameDrawer);
    I_WIPE_FadeOutScreen();
    S_StopMusic();

    if (exit == ga_timeout)
        return ga_timeout;

    G_InitNew(startskill, startmap, ga_nothing);
    G_RunGame();

    return 0;
}

int M_ControllerPak(void) // 80007724
{
    int exit;
    int ret;
    boolean PakBad;

    PakBad = false;

    while(1)
    {
        ret = I_CheckControllerPak();

        if ((ret != PFS_ERR_NOPACK) && (ret != PFS_ERR_ID_FATAL))
            PakBad = true;

        if(ret == 0)
        {
            ret = I_ReadPakFile();

            // Free Pak_Data
            if (Pak_Data)
            {
                Z_Free(Pak_Data);
                Pak_Data = NULL;
            }

            if(ret == 0)
            {
                exit = ga_nothing;
                break;
            }

            // Create Controller Pak Note
            MenuItem = Menu_CreateNote;
            itemlines = 3;
            MenuCall = M_MenuTitleDrawer;
            cursorpos = 0;

            MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);
            M_FadeOutStart(8);

            if (cursorpos != 0)
            {
                exit = ga_exit;
                break;
            }

            // Check Memory and Files Used on Controller Pak
            if ((Pak_Memory > 0) && (FilesUsed != 16))
            {
                if (I_CreatePakFile() != 0)
                    goto ControllerPakBad;

                exit = ga_nothing;
                break;
            }

            // Show Controller Pak Full
            MenuItem = Menu_ControllerPakFull;
            itemlines = 3;
            MenuCall = M_MenuTitleDrawer;
            cursorpos = 0;

            MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);
            M_FadeOutStart(8);

            if (cursorpos != 1)
            {
                exit = ga_exit;
                break;
            }
        }
        else
        {
            if (PakBad == false)
            {
                exit = ga_exit;
                break;
            }

            // Show Controller Pak Bad
        ControllerPakBad:
            MenuItem = Menu_ControllerPakBad;
            itemlines = 2;
            MenuCall = M_MenuTitleDrawer;
            cursorpos = 0;

            MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);
            M_FadeOutStart(8);

            if (cursorpos != 0)
            {
                exit = ga_exit;
                break;
            }
        }
    }

    return exit;
}

#define MAXSENSIVITY    20

int M_ButtonResponder(int buttons) // 80007960
{
    int sensitivity;
    int NewButtons;

    /* Copy Default Buttons */
    NewButtons = (buttons);

    /* Analyze Analog Stick (up / down) */
    sensitivity = (int)((buttons) << 24) >> 24;

    if (sensitivity <= -MAXSENSIVITY)
        NewButtons |= PAD_DOWN;
    else if (sensitivity >= MAXSENSIVITY)
        NewButtons |= PAD_UP;

    /* Analyze Analog Stick (left / right) */
    sensitivity = (int)(((buttons & 0xff00) >> 8) << 24) >> 24;

    if (sensitivity <= -MAXSENSIVITY)
        NewButtons |= PAD_LEFT;
    else if (sensitivity >= MAXSENSIVITY)
        NewButtons |= PAD_RIGHT;

    return NewButtons & 0xffff0000;
}

void M_AlphaInStart(void) // 800079E0
{
    text_alpha = 0;
    text_alpha_change_value = 20;
}

void M_AlphaOutStart(void) // 800079F8
{
    text_alpha = 255;
    text_alpha_change_value = -20;
}

int M_AlphaInOutTicker(void) // 80007A14
{
    if ((gamevbls < gametic) && ((gametic & 3) == 0)) {
        MenuAnimationTic = MenuAnimationTic + 1 & 7;
    }

    text_alpha += text_alpha_change_value;
    if (text_alpha_change_value < 0)
    {
        if (text_alpha < 0)
        {
            text_alpha = 0;
            return 8;
        }
    }
    else
    {
        if ((text_alpha_change_value > 0) && (text_alpha >= 256))
        {
            text_alpha = 255;
            return 8;
        }
    }

    return 0;
}

void M_FadeInStart(void) // 80007AB4
{
    MiniLoop(M_AlphaInStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void M_FadeOutStart(int exitmode) // 80007AEC
{
    if (exitmode == 8)
        MiniLoop(M_AlphaOutStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void M_SaveMenuData(void) // 80007B2C
{
    menudata_t *mdat;

    // Save Actual Menu Page
    mdat = &MenuData[MenuIdx];
    MenuIdx += 1;

    mdat->menu_item  = MenuItem;
    mdat->item_lines = itemlines;
    mdat->menu_call  = MenuCall;
    mdat->cursor_pos = cursorpos;

    // Start Menu Fade Out
    MiniLoop(M_AlphaOutStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void M_RestoreMenuData(boolean alpha_in) // 80007BB8
{
    menudata_t *mdat;

    // Restore Previous Save Menu Page
    MenuIdx -= 1;
    mdat = &MenuData[MenuIdx];

    MenuItem  = mdat->menu_item;
    itemlines = mdat->item_lines;
    MenuCall  = mdat->menu_call;
    cursorpos = mdat->cursor_pos;

    // Start Menu Fade In
    if (alpha_in)
        MiniLoop(M_AlphaInStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void M_MenuGameDrawer(void) // 80007C48
{
    if (DrawerStatus == 1) {
        P_Drawer();
    }
    else if (DrawerStatus == 2) {
        F_DrawerIntermission();
    }
    else if (DrawerStatus == 3) {
        F_Drawer();
    }
    else
    {
        I_ClearFrame();

        gDPPipeSync(GFX1++);
        gDPSetCycleType(GFX1++, G_CYC_FILL);
        gDPSetRenderMode(GFX1++,G_RM_NOOP,G_RM_NOOP2);
        gDPSetColorImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_32b, SCREEN_WD, OS_K0_TO_PHYSICAL(cfb[vid_side]));
        gDPSetFillColor(GFX1++, GPACK_RGBA5551(0,0,0,0) << 16 | GPACK_RGBA5551(0,0,0,0));
        gDPFillRectangle(GFX1++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);

        M_DrawBackground(56, 57, 80, "TITLE");

        if (MenuItem != Menu_Title) {
            M_DrawOverlay(0, 0, 320, 240, 96);
        }

        MenuCall();
        I_DrawFrame();
    }
}

extern mobj_t mobjhead;
extern mapthing_t *spawnlist;   // 800A5D74
extern int spawncount;          // 800A5D78
extern int gobalcheats; // [GEC]

int M_MenuTicker(void) // 80007E0C
{
    unsigned int buttons, oldbuttons;
    int exit;
    int truebuttons;
    int ret;
    int i;
    mobj_t *m;

    /* animate skull */
    if ((gamevbls < gametic) && ((gametic & 3) == 0))
        MenuAnimationTic = MenuAnimationTic + 1 & 7;

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = oldticbuttons[0] & 0xffff0000;

    /* exit menu if button press */
    if (buttons != 0)
        last_ticon = ticon;

    /* exit menu if time out */
    if ((MenuItem == Menu_Title) && ((ticon - last_ticon) >= 900)) // 30 * TICRATE
    {
        exit = ga_timeout;
    }
    else
    {
        /* check for movement */
        if (!(buttons & (PAD_Z_TRIG|ALL_JPAD)))
            m_vframe1 = 0;
        else
        {
            m_vframe1 = m_vframe1 - vblsinframe[0];
            if (m_vframe1 <= 0)
            {
                m_vframe1 = 0xf; // TICRATE / 2

                if (buttons & PAD_DOWN)
                {
                    cursorpos += 1;

                    if (cursorpos >= itemlines)
                        cursorpos = 0;

                    S_StartSound(NULL, sfx_switch1);
                }
                else if (buttons & PAD_UP)
                {
                    cursorpos -= 1;

                    if (cursorpos < 0)
                        cursorpos = itemlines-1;

                    S_StartSound(NULL, sfx_switch1);
                }
            }
        }

        if ((buttons & PAD_START) && !(oldticbuttons[0] & PAD_START))
        {
            if ((MenuItem == Menu_Title) ||
                (MenuItem == Menu_ControllerPakBad) ||
                (MenuItem == Menu_CreateNote) ||
                (MenuItem == Menu_ControllerPakFull))
            {
                return ga_nothing;
            }
            else
            {
                if (MenuIdx != 0)
                    S_StartSound(NULL, sfx_pistol);

                return ga_exit;
            }
        }
        else
        {
                truebuttons = (0 < (buttons ^ oldbuttons));

                if (truebuttons)
                    truebuttons = (0 < (buttons & ALL_BUTTONS));

                switch(MenuItem[cursorpos].casepos)
                {

                case 0: // Control Pad
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        MenuCall = M_ControlPadDrawer;
                        cursorpos = 0;
                        linepos = 0;

                        MiniLoop(M_FadeInStart,M_FadeOutStart,M_ControlPadTicker,M_MenuGameDrawer);
                        M_RestoreMenuData(true);
                        return ga_nothing;
                    }
                    break;

                case 1: // Volume
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        MenuItem = Menu_Volume;
                        itemlines = 4;
                        MenuCall = M_VolumeDrawer;
                        cursorpos = 0;

                        MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                        M_RestoreMenuData(true);
                        return ga_nothing;
                    }
                    break;

                case 2: // Display
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        MenuItem = Menu_Display;
                        itemlines = 6;
                        MenuCall = M_DisplayDrawer;
                        cursorpos = 0;

                        MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                        M_RestoreMenuData(true);
                        return ga_nothing;
                    }
                    break;

                case 3: // Password
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        ret = I_CheckControllerPak();
                        exit = ga_exit;

                        if (ret == 0)
                        {
                            if (I_ReadPakFile() == 0)
                            {
                                EnableExpPak = 1;
                                MenuCall = M_LoadPakDrawer;
                                exit = MiniLoop(M_LoadPakStart,M_LoadPakStop,M_LoadPakTicker,M_MenuGameDrawer);
                            }
                            else
                                exit = ga_exit;
                        }

                        if (exit == ga_exit)
                        {
                            MenuCall = M_PasswordDrawer;
                            exit = MiniLoop(M_PasswordStart,M_PasswordStop,M_PasswordTicker,M_MenuGameDrawer);
                        }

                        if (exit == ga_exit)
                        {
                            M_RestoreMenuData(true);
                            return ga_nothing;
                        }

                        if (EnableExpPak != 0)
                        {
                            return exit;
                        }

                        EnableExpPak = (M_ControllerPak() == 0);
                        return exit;
                    }
                    break;

                case 4: // Main Menu
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        MenuItem = Menu_Quit;
                        itemlines = 2;
                        MenuCall = M_MenuTitleDrawer;
                        cursorpos = 1;

                        exit = MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                        M_RestoreMenuData((exit == ga_exit));
                        if (exit == ga_exit) {
                            return ga_nothing;
                        }

                        return 5;//ga_exitdemo;
                    }
                    break;

                case 5: // Restart Level
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        MenuItem = Menu_Quit;
                        itemlines = 2;
                        MenuCall = M_MenuTitleDrawer;
                        cursorpos = 1;

                        exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
                        M_RestoreMenuData((exit == ga_exit));

                        if (exit == ga_exit)
                            return ga_nothing;

                        return ga_restart;
                    }
                    break;

                case 6: // Return
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        return ga_exit;
                    }
                    break;

                case 7: // Music Volume
                    if (buttons & PAD_RIGHT)
                    {
                        MusVolume += 1;
                        if (MusVolume <= 100)
                        {
                            S_SetMusicVolume(MusVolume);
                            if (MusVolume & 1)
                            {
                                S_StartSound(NULL, sfx_secmove);
                                return ga_nothing;
                            }
                        }
                        else
                        {
                            MusVolume = 100;
                        }
                    }
                    else if (buttons & PAD_LEFT)
                    {
                        MusVolume -= 1;
                        if (MusVolume < 0)
                        {
                            MusVolume = 0;
                        }
                        else
                        {
                            S_SetMusicVolume(MusVolume);
                            if (MusVolume & 1)
                            {
                                S_StartSound(NULL, sfx_secmove);
                                return ga_nothing;
                            }
                        }
                    }
                    break;

                case 8: // Sound Volume
                    if (buttons & PAD_RIGHT)
                    {
                        SfxVolume += 1;
                        if (SfxVolume <= 100)
                        {
                            S_SetSoundVolume(SfxVolume);
                            if (SfxVolume & 1)
                            {
                                S_StartSound(NULL, sfx_secmove);
                                return ga_nothing;
                            }
                        }
                        else
                        {
                            SfxVolume = 100;
                        }
                    }
                    else if (buttons & PAD_LEFT)
                    {
                        SfxVolume -= 1;
                        if (SfxVolume < 0)
                        {
                            SfxVolume = 0;
                        }
                        else
                        {
                            S_SetSoundVolume(SfxVolume);
                            if (SfxVolume & 1)
                            {
                                S_StartSound(NULL, sfx_secmove);
                                return ga_nothing;
                            }
                        }
                    }
                    break;

                case 9: // Brightness
                    if (buttons & PAD_RIGHT)
                    {
                        brightness += 1;
                        if (brightness <= 100)
                        {
                            P_RefreshBrightness();
                            if (brightness & 1)
                            {
                                S_StartSound(NULL, sfx_secmove);
                                return ga_nothing;
                            }
                        }
                        else
                        {
                            brightness = 100;
                        }
                    }
                    else if (buttons & PAD_LEFT)
                    {
                        brightness -= 1;
                        if (brightness < 0)
                        {
                            brightness = 0;
                        }
                        else
                        {
                            P_RefreshBrightness();
                            if (brightness & 1)
                            {
                                S_StartSound(NULL, sfx_secmove);
                                return ga_nothing;
                            }
                        }
                    }
                    break;

                case 11: // Options
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        MenuItem = Menu_Options;
                        itemlines = 6;
                        MenuCall = M_MenuTitleDrawer;
                        cursorpos = 0;

                        exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
                        M_RestoreMenuData((exit == ga_exit));

                        if (exit == ga_exit)
                            return ga_nothing;

                        return exit;
                    }
                    break;

                case 12: // Default Volume
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_switch2);

                        SfxVolume = 0x50;
                        MusVolume = 0x50;

                        S_SetMusicVolume(MusVolume);
                        S_SetSoundVolume(SfxVolume);

                        return ga_nothing;
                    }
                    break;

                case 13: // Default Display
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_switch2);

                        Display_X = 0;
                        Display_Y = 0;

                        enable_messages = true;
                        enable_statusbar = true;

                        brightness = 0;
                        I_MoveDisplay(0,0);
                        P_RefreshBrightness();
                        return ga_nothing;
                    }
                    break;

                case 14: // New Game
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_FadeOutStart(8);

                        // Check ControllerPak
                        EnableExpPak = (M_ControllerPak() == 0);

                        MenuItem = Menu_Skill;
                        #if ENABLE_NIGHTMARE == 1
                        itemlines = 5;
                        #else
                        itemlines = 4;
                        #endif // ENABLE_NIGHTMARE
                        MenuCall = M_MenuTitleDrawer;
                        cursorpos = 1;  // Set Default Bring it on!

                        MiniLoop(M_FadeInStart, M_MenuClearCall, M_MenuTicker, M_MenuGameDrawer);
                        startskill = MenuItem[cursorpos].casepos - 15;

                        return ga_exit;
                    }
                    break;

                case 15: // Be Gentle!
                case 16: // Bring it on!
                case 17: // I own Doom!
                case 18: // Watch me die!
				case 19: // Nightmare!
                
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        return ga_exit;
                    }
                    break;

                case 20: // Yes
                case 46: // Try again
                case 47: // Create game note
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        return 5; //ga_exitdemo;
                    }
                    break;

                case 21: // No
                case 45: // Do not use Pak
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        return ga_exit;
                    }
                    break;

                case 22: // Features
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        players[0].cheats &= 0xffff1fff;

                        MenuItem = Menu_Features;
                        itemlines = MAXFEATURES;
                        MenuCall = M_FeaturesDrawer;
                        cursorpos = 0;
                        m_actualmap = gamemap;

                        exit = MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                        M_RestoreMenuData((exit == 8));

                        if (exit == 8)
                            return ga_nothing;

                        return exit;
                    }
                    break;

                case 23: // WARP TO LEVEL
                    if (buttons ^ oldbuttons)
                    {
                        if (buttons & PAD_LEFT)
                        {
                            m_actualmap -= 1;
                            if (m_actualmap > 27)
                                m_actualmap = 27;

                            if (m_actualmap > 0)
                            {
                                S_StartSound(NULL, sfx_switch2);
                                return ga_nothing;
                            }
                            m_actualmap = 1;
                        }
                        else if (buttons & PAD_RIGHT)
                        {
                            m_actualmap += 1;
                            if (m_actualmap < 28)
                            {
                                S_StartSound(NULL, sfx_switch2);
                                return ga_nothing;
                            }
                            m_actualmap = 27;
                        }
                        else if (buttons & ALL_CBUTTONS)
                        {
                            gamemap = m_actualmap;
                            startmap = m_actualmap;
                            return ga_warped;
                        }
                    }
                    break;

                case 24: // INVULNERABLE
                    if (((gamemap != 32) & truebuttons))
                    {
                        players[0].cheats ^= CF_GODMODE;
                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;

                case 25: // HEALTH BOOST
                    if (truebuttons)
                    {
                        players[0].cheats |= CF_HEALTH;
                        players[0].health = 100;
                        players[0].mo->health = 100;
                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;

                case 26: // SECURITY KEYS
                    /* Not available in the release code */
                    /*
                    Reconstructed code based on Psx Doom
                    */
                    if (truebuttons)
                    {
                        players[0].cheats |= CF_ALLKEYS;

                        for (m = mobjhead.next; m != &mobjhead; m = m->next)
                        {
                            switch (m->type)
                            {
                            case MT_ITEM_BLUECARDKEY:
                                players[0].cards[it_bluecard] = true;
                                break;
                            case MT_ITEM_REDCARDKEY:
                                players[0].cards[it_redcard] = true;
                                break;
                            case MT_ITEM_YELLOWCARDKEY:
                                players[0].cards[it_yellowcard] = true;
                                break;
                            case MT_ITEM_YELLOWSKULLKEY:
                                players[0].cards[it_yellowskull] = true;
                                break;
                            case MT_ITEM_REDSKULLKEY:
                                players[0].cards[it_redskull] = true;
                                break;
                            case MT_ITEM_BLUESKULLKEY:
                                players[0].cards[it_blueskull] = true;
                                break;
                            default:
                                break;
                            }
                        }

                        for (i = 0; i < spawncount; i++)
                        {
                            switch (spawnlist[i].type)
                            {
                            case 5:
                                players[0].cards[it_bluecard] = true;
                                break;
                            case 13:
                                players[0].cards[it_redcard] = true;
                                break;
                            case 6:
                                players[0].cards[it_yellowcard] = true;
                                break;
                            case 39:
                                players[0].cards[it_yellowskull] = true;
                                break;
                            case 38:
                                players[0].cards[it_redskull] = true;
                                break;
                            case 40:
                                players[0].cards[it_blueskull] = true;
                                break;
                            default:
                                break;
                            }
                        }

                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;

                case 27: // WEAPONS
                    if (truebuttons)
                    {
                        players[0].cheats |= CF_WEAPONS;

                        for(i = 0; i < NUMWEAPONS; i++) {
                            players[0].weaponowned[i] = true;
                        }

                        for(i = 0; i < NUMAMMO; i++) {
                            players[0].ammo[i] = players[0].maxammo[i];
                        }

                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;

                case 28: // Exit
                    /* nothing special */
                    break;

                case 29: // DEBUG
                    /* Not available in the release code */
                    if (truebuttons)
                    {
                        players[0].cheats ^= CF_DEBUG;
                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;

                case 30: // TEXTURE TEST
                    /* Not available in the release code */
                    if (truebuttons)
                    {
                        players[0].cheats ^= CF_TEX_TEST;
                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;

                case 31: // WALL BLOCKING
                    /* Not available in the release code */
                    /*
                    In my opinion it must have been the NOCLIP cheat code
                    */
                    if (truebuttons)
                    {
                        players[0].cheats ^= CF_WALLBLOCKING;
                        players[0].mo->flags ^= MF_NOCLIP;
                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;

                case 32: // Center Display
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        MenuCall = M_CenterDisplayDrawer;

                        MiniLoop(M_FadeInStart,M_FadeOutStart,M_CenterDisplayTicker,M_MenuGameDrawer);
                        M_RestoreMenuData(true);

                        return ga_nothing;
                    }
                    break;

                case 33: // Messages
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_switch2);
                        enable_messages ^= true;
                    }
                    break;

                case 34: // Status Bar
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_switch2);
                        enable_statusbar ^= true;
                    }
                    break;

                case 35: // LOCK MONSTERS
                    /* Not available in the release code */
                    /*
                    Reconstructed code based on Doom 64 Ex
                    */
                    if (truebuttons)
                    {
                        players[0].cheats ^= CF_LOCKMOSTERS;
                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;

                case 36: // SCREENSHOT
                    /* Not available in the release code */
                    if (truebuttons)
                    {
                        players[0].cheats ^= CF_SCREENSHOT;
                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;

                case 37: // MAP EVERYTHING
                    if (truebuttons)
                    {
                        players[0].cheats ^= CF_ALLMAP;
                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;

                case 38: // MACRO PEEK
                    /* Not available in the release code */
                    if (truebuttons)
                    {
                        players[0].cheats ^= CF_MACROPEEK;
                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;

                case 39: // MUSIC TEST
                    /* Not available in the release code */
                    /*
                    Reconstructed code in my interpretation
                    */
                    if (buttons ^ oldbuttons)
                    {
                        if (buttons & PAD_LEFT)
                        {
                            MusicID -= 1;
                            if (MusicID > 0)
                            {
                                S_StartSound(NULL, sfx_switch2);
                                return ga_nothing;
                            }
                            MusicID = 1;
                        }
                        else if (buttons & PAD_RIGHT)
                        {
                            MusicID += 1;
                            if (MusicID < 25)
                            {
                                S_StartSound(NULL, sfx_switch2);
                                return ga_nothing;
                            }
                            MusicID = 24;
                        }
                        else if (buttons & ALL_CBUTTONS)
                        {
                            S_StopMusic();
                            S_StartMusic(MusicID+92);
                            return ga_nothing;
                        }
                    }
                    break;

                case 41: // Control Stick
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        MenuItem = Menu_ControlStick;
                        itemlines = 3;
                        MenuCall = M_ControlStickDrawer;
                        cursorpos = 0;

                        MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
                        M_RestoreMenuData(true);

                        return ga_nothing;
                    }
                    break;

                case 42: // Default Sensitivity
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_switch2);
                        M_SENSITIVITY = 0;
                    }
                    break;

                case 43: // Sensitivity
                    if (buttons & PAD_RIGHT)
                    {
                        M_SENSITIVITY += 1;
                        if (M_SENSITIVITY <= 100)
                        {
                            if (M_SENSITIVITY & 1)
                            {
                                S_StartSound(NULL, sfx_secmove);
                                return ga_nothing;
                            }
                        }
                        else
                        {
                            M_SENSITIVITY = 100;
                        }
                    }
                    else if (buttons & PAD_LEFT)
                    {
                        M_SENSITIVITY -= 1;
                        if (M_SENSITIVITY < 0)
                        {
                            M_SENSITIVITY = 0;
                        }
                        else
                        {
                            if (M_SENSITIVITY & 1)
                            {
                                S_StartSound(NULL, sfx_secmove);
                                return ga_nothing;
                            }
                        }
                    }
                    break;

                case 44: // Manage Pak
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        MenuCall = M_ControllerPakDrawer;
                        linepos = 0;
                        cursorpos = 0;

                        exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_ScreenTicker, M_MenuGameDrawer);
                        M_RestoreMenuData((exit == 8));

                        if (exit == 8)
                            return ga_nothing;

                        return exit;
                    }
                    break;

                case 48: // COLORS [GEC] NEW CHEAT CODE
                    if (truebuttons)
                    {
                    players[0].cheats ^= CF_NOCOLORS;
                    gobalcheats ^= CF_NOCOLORS;
                    P_RefreshBrightness();
                    S_StartSound(NULL, sfx_switch2);
                    return ga_nothing;
                    }
                    break;

                case 49: // FULL BRIGHT [GEC] NEW CHEAT CODE
                    if (truebuttons)
                    {
                        players[0].cheats ^= CF_FULLBRIGHT;
                        gobalcheats ^= CF_FULLBRIGHT;
                        P_RefreshBrightness();
                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;

                case 50: // FILTER [GEC] NEW CHEAT CODE
                    if (truebuttons)
                    {
                        players[0].cheats ^= CF_FILTER;
                        gobalcheats ^= CF_FILTER;
                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;
                }

            exit = ga_nothing;
        }
    }

    return exit;
}

void M_MenuClearCall(void) // 80008E6C
{
    MenuCall = NULL;
}

void M_MenuTitleDrawer(void) // 80008E7C
{
    menuitem_t *item;
    int i;

    if (MenuItem == Menu_Game)
    {
        ST_DrawString(-1, 20, "Pause", text_alpha | 0xc0000000);
        ST_DrawString(-1, 200, "press \x8d to resume", text_alpha | 0xffffff00);
    }
    else if (MenuItem == Menu_Skill)
    {
        ST_DrawString(-1, 20, "Choose Your Skill...", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_Options)
    {
        ST_DrawString(-1, 20, "Options", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_Quit)
    {
        ST_DrawString(-1, 20, "Quit Game?", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_DeleteNote)
    {
        ST_DrawString(-1, 20, "Delete Game Note?", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_ControllerPakBad)
    {
        ST_DrawString(-1, 20, "Controller Pak Bad", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_ControllerPakFull)
    {
        ST_DrawString(-1, 20, "Controller Pak Full", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_CreateNote)
    {
        ST_DrawString(-1, 20, "Create Game Note?", text_alpha | 0xc0000000);
    }

    item = MenuItem;
    for(i = 0; i < itemlines; i++)
    {
        ST_DrawString(item->x, item->y, MenuText[item->casepos], text_alpha | 0xc0000000);
        item++;
    }

    ST_DrawSymbol(MenuItem[0].x -37, MenuItem[cursorpos].y -9, MenuAnimationTic + 70, text_alpha | 0xffffff00);
}

void M_FeaturesDrawer(void) // 800091C0
{
    char *text, textbuff[16];
    menuitem_t *item;
    int i;

    ST_DrawString(-1, 20, "Features", text_alpha | 0xc0000000);
    item = MenuItem;

    for(i = 0; i < itemlines; i++)
    {
        if ((item->casepos == 23) && ((m_actualmap >= 25) && (m_actualmap <= 27)))
        {
            /* Show "WARP TO FUN" text */
            ST_Message(item->x, item->y, MenuText[40], text_alpha | 0xffffff00);
        }
        else
        {
            /* Show "WARP TO LEVEL" text */
            ST_Message(item->x, item->y, MenuText[item->casepos], text_alpha | 0xffffff00);
        }

        text = textbuff;
        switch(item->casepos)
        {
            case 23: /* WARP TO LEVEL */
                sprintf(textbuff, "%s", MapInfo[m_actualmap].name);
                break;
            case 24: /* INVULNERABLE */
                text = (!(players[0].cheats & CF_GODMODE)) ? "OFF": "ON";
                break;
            case 25: /* HEALTH BOOST */
                text = (!(players[0].cheats & CF_HEALTH)) ? "-" : "100%";
                break;
            case 26: /* SECURITY KEYS */
                text = (!(players[0].cheats & CF_ALLKEYS)) ? "-" : "100%";
                break;
            case 27: /* WEAPONS */
                text = (!(players[0].cheats & CF_WEAPONS)) ? "-" : "100%";
                break;
            case 28: /* Exit */
                break;
            case 29: /* DEBUG */
                text = (!(players[0].cheats & CF_DEBUG)) ? "OFF": "ON";
                break;
            case 30: /* TEXTURE TEST */
                text = (!(players[0].cheats & CF_TEX_TEST)) ? "OFF": "ON";
                break;
            case 31: /* WALL BLOCKING */
                text = (!(players[0].cheats & CF_WALLBLOCKING)) ? "ON": "OFF";
                break;
            case 35: /* LOCK MONSTERS */
                text = (!(players[0].cheats & CF_LOCKMOSTERS)) ? "OFF": "ON";
                break;
            case 36: /* SCREENSHOT */
                text = (!(players[0].cheats & CF_SCREENSHOT)) ? "OFF": "ON";
                break;
            case 37: /* MAP EVERYTHING */
                text = (!(players[0].cheats & CF_ALLMAP)) ? "OFF": "ON";
                break;
            case 38: /* MACRO PEEK */
                text = ((players[0].cheats & CF_MACROPEEK)) ? "ON": "OFF";
                break;
            case 39: /* MUSIC TEST */
                sprintf(textbuff, "%d", MusicID);
                break;

            // [GEC] NEW CHEAT CODES
            case 48: /* COLORS */
                text = (!(players[0].cheats & CF_NOCOLORS)) ? "ON": "OFF";
                break;

            case 49: /* FULL BRIGHT */
                text = (!(players[0].cheats & CF_FULLBRIGHT)) ? "OFF": "ON";
                break;

            case 50: /* FILTER */
                text = (!(players[0].cheats & CF_FILTER)) ? "LINEAR": "NEAREST";
                break;

            default:
                text = "NOT IMPLEMENTED";
                break;
        }

        ST_Message(item->x + 130, item->y, text, text_alpha | 0xffffff00);
        item++;
    }

    ST_DrawSymbol(MenuItem->x -10, MenuItem[cursorpos].y -1, 78, text_alpha | 0xffffff00);
}

void M_VolumeDrawer(void) // 800095B4
{
    menuitem_t *item;
    int i;

    ST_DrawString(-1, 20, "Volume", text_alpha | 0xc0000000);
    item = Menu_Volume;

    for(i = 0; i < itemlines; i++)
    {
        ST_DrawString(item->x, item->y, MenuText[item->casepos], text_alpha | 0xc0000000);
        item++;
    }

    ST_DrawSymbol(MenuItem->x - 37, MenuItem[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00);

    ST_DrawSymbol(102, 80, 68, text_alpha | 0xffffff00);
    ST_DrawSymbol(MusVolume + 103, 80, 69, text_alpha | 0xffffff00);

    ST_DrawSymbol(102, 120, 68, text_alpha | 0xffffff00);
    ST_DrawSymbol(SfxVolume + 103, 120, 69, text_alpha | 0xffffff00);
}

void M_ControlStickDrawer(void) // 80009738
{
    menuitem_t *item;
    int i;

    ST_DrawString(-1, 20, "Control Stick", text_alpha | 0xc0000000);

    item = Menu_ControlStick;

    for(i = 0; i < itemlines; i++)
    {
        ST_DrawString(item->x, item->y, MenuText[item->casepos], text_alpha | 0xc0000000);
        item++;
    }

    ST_DrawSymbol(MenuItem->x - 37, MenuItem[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00);

    ST_DrawSymbol(102,110,68,text_alpha | 0xffffff00);
    ST_DrawSymbol(M_SENSITIVITY + 103, 110, 69, text_alpha | 0xffffff00);
}

void M_DisplayDrawer(void) // 80009884
{
    char *text;
    menuitem_t *item;
    int i, casepos;

    ST_DrawString(-1, 20, "Display", text_alpha | 0xc0000000);

    item = Menu_Display;

    for(i = 0; i < 6; i++)
    {
        casepos = item->casepos;

        if (casepos == 33) // Messages:
        {
            if (enable_messages)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == 34) // Status Bar:
        {
            if (enable_statusbar)
                text = "On";
            else
                text = "Off";
        }
        else
        {
            text = NULL;
        }

        if (text)
            ST_DrawString(item->x + 140, item->y, text, text_alpha | 0xc0000000);

        ST_DrawString(item->x, item->y, MenuText[casepos], text_alpha | 0xc0000000);

        item++;
    }

    ST_DrawSymbol(102, 80, 68, text_alpha | 0xffffff00);
    ST_DrawSymbol(brightness + 103, 80, 69, text_alpha | 0xffffff00);

    ST_DrawSymbol(Menu_Display[0].x - 37, Menu_Display[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00);
}

void M_DrawBackground(int x, int y, int color, char *name) // 80009A68
{
    int width, height;
    int yh, xh, t;
    int offset;
    byte *data;

    data = (byte *)W_CacheLumpName(name, PU_CACHE, dec_jag);

    gDPPipeSync(GFX1++);
    gDPSetCycleType(GFX1++, G_CYC_1CYCLE);

    gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
    gDPSetTexturePersp(GFX1++, G_TP_NONE);

    gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);

    gDPSetBlendColor(GFX1++, 0, 0, 0, 0);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB03, G_CC_D64COMB03);

    if (color == 0xff)
    {
        gDPSetRenderMode(GFX1++, G_RM_TEX_EDGE, G_RM_TEX_EDGE2);
    }
    else
    {
        gDPSetRenderMode(GFX1++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    }

    gDPSetPrimColorD64(GFX1++, 0, 0, color);

    width = ((gfxN64_t*)data)->width;
    height = ((gfxN64_t*)data)->height;

    // Load Palette Data
    offset = (width * height);
    offset = (offset + 7) & ~7;
    gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b ,
                        1, data + offset + sizeof(gfxN64_t));

    gDPTileSync(GFX1++);
    gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

    gDPLoadSync(GFX1++);
    gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 255);

    gDPPipeSync(GFX1++);

    xh = (width + 7) & ~7;

    t = 0;
    while (height != 0)
    {
        if ((2048 / xh) < height)
            yh = (2048 / xh);
        else
            yh = height;

        // Load Image Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b ,
                        width, data + sizeof(gfxN64_t));

         // Clip Rectangle From Image
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b,
                        (width + 7) / 8, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTile(GFX1++, G_TX_LOADTILE,
                    (0 << 2), (t << 2),
                    ((width - 1) << 2), (((t + yh) - 1) << 2));

        gDPPipeSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b,
                    (width + 7) / 8, 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPSetTileSize(GFX1++, G_TX_RENDERTILE,
                       (0 << 2), (t << 2),
                       ((width - 1) << 2), (((t + yh) - 1) << 2));

        gSPTextureRectangle(GFX1++,
            (x << 2), (y << 2),
            ((width + x) << 2), ((yh + y) << 2),
            G_TX_RENDERTILE,
            (0 << 5), (t << 5),
            (1 << 10), (1 << 10));

        height -= yh;
        t += yh;
        y += yh;
    }

    globallump = -1;
}

void M_DrawOverlay(int x, int y, int w, int h, int color) // 80009F58
{
    I_CheckGFX();

    gDPPipeSync(GFX1++);

    gDPSetCycleType(GFX1++, G_CYC_1CYCLE);

    gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
    gDPSetTexturePersp(GFX1++, G_TP_NONE);

    gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);

    gDPSetBlendColor(GFX1++, 0, 0, 0, 0);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB05, G_CC_D64COMB05);
    gDPSetRenderMode(GFX1++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

    gDPSetPrimColorD64(GFX1++, 0, 0, color);

    gDPFillRectangle(GFX1++, x, y, w, h);
    globallump = -1;
}

int M_ScreenTicker(void) // 8000A0F8
{
    int exit;
    unsigned int buttons;
    unsigned int oldbuttons;
    OSPfsState *fState;

    if ((FilesUsed == -1) && (I_CheckControllerPak() == 0))
    {
        cursorpos = 0;
        linepos = 0;
    }

    if ((gamevbls < gametic) && ((gametic & 3) == 0))
        MenuAnimationTic = MenuAnimationTic + 1 & 7;

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = oldticbuttons[0] & 0xffff0000;

    if (!(buttons & ALL_JPAD))
    {
        m_vframe1 = 0;
    }
    else
    {
        m_vframe1 -= vblsinframe[0];

        if (m_vframe1 <= 0)
        {
            m_vframe1 = 0xf; // TICRATE/2

            if (buttons & PAD_DOWN)
            {
                cursorpos += 1;

                if (cursorpos < 16)
                    S_StartSound(NULL, sfx_switch1);
                else
                    cursorpos = 15;

                if ((linepos + 5) < cursorpos)
                    linepos += 1;
            }
            else if (buttons & PAD_UP)
            {
                cursorpos -= 1;

                if (cursorpos < 0)
                    cursorpos = 0;
                else
                    S_StartSound(NULL, sfx_switch1);

                if(cursorpos < linepos)
                    linepos -= 1;
            }
        }
    }

    if (!(buttons ^ oldbuttons) || !(buttons & PAD_START))
    {
        if (buttons ^ oldbuttons)
        {
            if(buttons == (PAD_LEFT_C|PAD_RIGHT_C))
            {
                fState = &FileState[cursorpos];

                if(fState->file_size != 0)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    MenuItem = Menu_DeleteNote;
                    itemlines = 2;
                    MenuCall = M_MenuTitleDrawer;
                    cursorpos = 1;
                    MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);

                    M_FadeOutStart(8);
                    if (cursorpos == 0)
                    {
                        if (I_DeletePakFile(cursorpos) == 0)
                        {
                            fState->file_size = 0;
                        }
                        else
                        {
                            FilesUsed = -1;
                        }
                    }
                    M_RestoreMenuData(true);
                }
            }
        }
        exit = 0;
    }
    else
    {
        S_StartSound(NULL, sfx_pistol);
        exit = 8;
    }
    return exit;
}

void M_ControllerPakDrawer(void) // 8000A3E4
{
    byte idx;
    int i,j;
    OSPfsState *fState;
    char buffer [32];
    char *tmpbuf;

    ST_DrawString(-1, 20, "Controller Pak", text_alpha | 0xc0000000);

    if (FilesUsed == -1)
    {
        if ((MenuAnimationTic & 2) != 0)
            ST_DrawString(-1, 114, "Controller Pak removed!", text_alpha | 0xc0000000);

        ST_DrawString(-1, 210, "press \x8d to exit", text_alpha | 0xffffff00);
    }
    else
    {
        fState = &FileState[linepos];

        for(i = linepos; i < (linepos + 6); i++)
        {
            if (fState->file_size == 0)
            {
                D_memmove(buffer, "empty");
            }
            else
            {
                tmpbuf = buffer;

                for(j = 0; j < 16; j++)
                {
                    idx = (byte) fState->game_name[j];
                    if(idx == 0)
                        break;

                    tmpbuf[0] = Pak_Table[idx];
                    tmpbuf++;
                }

                idx = (byte) fState->ext_name[0];
                if (idx != 0)
                {
                    tmpbuf[0] = '.';
                    tmpbuf[1] = Pak_Table[idx];
                    tmpbuf += 2;
                }

                *tmpbuf = '\0';
            }

            ST_DrawString(60, (i - linepos) * 15 + 60, buffer, text_alpha | 0xc0000000);

            fState++;
        }

        if (linepos != 0)
        {
            ST_DrawString(60, 45, "\x8F more...", text_alpha | 0xffffff00);
        }

        if ((linepos + 6) < 16)
        {
            ST_DrawString(60, 150, "\x8E more...", text_alpha | 0xffffff00);
        }

        sprintf(buffer, "pages used: %d   free: %d", FileState[cursorpos].file_size >> 8, Pak_Memory);

        ST_DrawString(-1, 170, buffer, text_alpha | 0xc0000000);
        ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 51, MenuAnimationTic + 70, text_alpha | 0xffffff00);

        ST_DrawString(-1, 200, "press \x8d to exit", text_alpha | 0xffffff00);
        ST_DrawString(-1, 215, "press \x84\x85 to delete", text_alpha | 0xffffff00);
    }
}

void M_SavePakStart(void) // 8000A6E8
{
    int i;
    int ret;
    int size;

    cursorpos = 0;
    linepos = 0;
    last_ticon = 0;

    ret = I_CheckControllerPak();
    if (ret == 0)
    {
        if (I_ReadPakFile() == 0)
        {
            size = Pak_Size / 32;

            i = 0;
            if (size != 0)
            {
                do
                {
                    if (Pak_Data[i * 32] == 0)
                        break;

                    i++;
                } while (i != size);
            }

            if (i < size)
            {
                cursorpos = i;

                if (!(size < (i+6)))
                    linepos = i;
                else
                    linepos = (size-6);
            }
        }
    }
    else
    {
        FilesUsed = -1;
    }
}

void M_SavePakStop(void) // 8000A7B4
{
    S_StartSound(NULL, sfx_pistol);

    if (Pak_Data)
    {
        Z_Free(Pak_Data);
        Pak_Data = NULL;
    }
}

int M_SavePakTicker(void) // 8000A804
{
    unsigned int buttons;
    unsigned int oldbuttons;
    int size;

    if ((gamevbls < gametic) && ((gametic & 3) == 0)) {
        MenuAnimationTic = MenuAnimationTic + 1 & 7;
    }

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = oldticbuttons[0] & 0xffff0000;

    if ((buttons != oldbuttons) && (buttons & PAD_START)) {
        return ga_exit;
    }

    if (FilesUsed == -1)
    {
        if (I_CheckControllerPak()) {
            return ga_nothing;
        }

        if (I_ReadPakFile()) {
            FilesUsed = -1;
            return ga_nothing;
        }

        cursorpos = 0;
        linepos = 0;
    }

    if (!(buttons & ALL_JPAD)) {
        m_vframe1 = 0;
    }
    else
    {
        m_vframe1 -= vblsinframe[0];

        if (m_vframe1 <= 0)
        {
            m_vframe1 = 0xf; // TICRATE/2

            if (buttons & PAD_DOWN)
            {
                cursorpos += 1;

                size = (Pak_Size / 32) - 1;

                if (size < cursorpos)
                    cursorpos = size;
                else
                    S_StartSound(NULL, sfx_switch1);


                if ((linepos + 5) < cursorpos)
                    linepos += 1;
            }
            else if (buttons & PAD_UP)
            {
                cursorpos -= 1;

                if (cursorpos < 0)
                    cursorpos = 0;
                else
                    S_StartSound(NULL, sfx_switch1);

                if(cursorpos < linepos)
                    linepos -= 1;
            }
        }
    }

    if (last_ticon == 0)
    {
        if ((buttons != oldbuttons) && (buttons == (PAD_RIGHT_C|PAD_LEFT_C)))
        {
            // save the next level number and password data in text format
            sprintf(&Pak_Data[cursorpos * 32], "level %2.2d", nextmap);
            D_memcpy(&Pak_Data[(cursorpos * 32) + 16], &Passwordbuff, 16);

            if (I_SavePakFile(File_Num, PFS_WRITE, Pak_Data, Pak_Size) == 0) {
                last_ticon = ticon;
            }
            else
            {
                FilesUsed = -1;
                if (Pak_Data)
                {
                    Z_Free(Pak_Data);
                    Pak_Data = NULL;
                }
            }
        }
    }
    else if ((ticon - last_ticon) >= 60) // 2 * TICRATE
    {
        return ga_exit;
    }

    return ga_nothing;
}

void M_SavePakDrawer(void) // 8000AB44
{
    int i;
    char buffer[36];

    I_ClearFrame();

    gDPPipeSync(GFX1++);
    gDPSetCycleType(GFX1++, G_CYC_FILL);
    gDPSetRenderMode(GFX1++,G_RM_NOOP,G_RM_NOOP2);
    gDPSetColorImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_32b, SCREEN_WD, OS_K0_TO_PHYSICAL(cfb[vid_side]));
    // Fill borders with black
    gDPSetFillColor(GFX1++, GPACK_RGBA5551(0,0,0,0) << 16 | GPACK_RGBA5551(0,0,0,0)) ;
    gDPFillRectangle(GFX1++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);

    M_DrawBackground(63, 25, 128, "EVIL");

    ST_DrawString(-1, 20, "Controller Pak", text_alpha | 0xc0000000);

    if (FilesUsed == -1)
    {
        if (MenuAnimationTic & 2)
        {
            ST_DrawString(-1, 100, "Controller Pak removed!", 0xc00000ff);
            ST_DrawString(-1, 120, "Game cannot be saved.", 0xc00000ff);
        }

        ST_DrawString(-1, 210, "press \x8d to exit", text_alpha | 0xffffff00);
    }
    else
    {
        for(i = linepos; i < (linepos + 6); i++)
        {
            if (Pak_Data[i * 32] == 0) {
                D_memmove(buffer, "empty");
            }
            else {
                D_memmove(buffer, &Pak_Data[i * 32]);
            }

            ST_DrawString(60, (i - linepos) * 15 + 65, buffer, text_alpha | 0xc0000000);
        }

        if (linepos != 0) {
            ST_DrawString(60, 50, "\x8f more...", text_alpha | 0xffffff00);
        }

        if ((linepos + 6) <= ((Pak_Size >> 5) - 1)) {
            ST_DrawString(60, 155, "\x8e more...", text_alpha | 0xffffff00);
        }

        ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 56, MenuAnimationTic + 70, text_alpha | 0xffffff00);

        ST_DrawString(-1, 195, "press \x8d to exit", text_alpha | 0xffffff00);
        ST_DrawString(-1, 210, "press \x84\x85 to save", text_alpha | 0xffffff00);
    }

    I_DrawFrame();
}

void M_LoadPakStart(void) // 8000AEEC
{
    int i;
    int size;

    cursorpos = 0;
    linepos = 0;

    size = Pak_Size / 32;

    i = 0;
    if (size != 0)
    {
        do
        {
            if (Pak_Data[i * 32])
                break;

            i++;
        } while (i != size);
    }

    if (i < size)
    {
        cursorpos = i;

        if (!(size < (i+6)))
            linepos = i;
        else
            linepos = (size-6);
    }

    M_FadeInStart();
}

void M_LoadPakStop(void) // 8000AF8C
{
    S_StartSound(NULL, sfx_pistol);
    M_FadeOutStart(ga_exit);

    if (Pak_Data)
    {
        Z_Free(Pak_Data);
        Pak_Data = NULL;
    }
}

int M_LoadPakTicker(void) // 8000AFE4
{
    unsigned int buttons;
    unsigned int oldbuttons;
    int size;
    int skill;
    int levelnum;
    int exit;

    if ((gamevbls < gametic) && ((gametic & 3U) == 0)) {
        MenuAnimationTic = MenuAnimationTic + 1 & 7;
    }

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = oldticbuttons[0] & 0xffff0000;

    if (!(buttons & ALL_JPAD))
    {
        m_vframe1 = 0;
    }
    else
    {
        m_vframe1 -= vblsinframe[0];

        if (m_vframe1 <= 0)
        {
            m_vframe1 = 0xf; // TICRATE/2

            if (buttons & PAD_DOWN)
            {
                cursorpos += 1;

                size = (Pak_Size / 32) - 1;

                if (size < cursorpos)
                    cursorpos = size;
                else
                    S_StartSound(NULL, sfx_switch1);

                if ((linepos + 5) < cursorpos)
                    linepos += 1;

            }
            else if (buttons & PAD_UP)
            {
                cursorpos -= 1;

                if (cursorpos < 0)
                    cursorpos = 0;
                else
                    S_StartSound(NULL, sfx_switch1);

                if(cursorpos < linepos)
                    linepos -= 1;
            }
        }
    }

    if (!(buttons ^ oldbuttons) || !(buttons & PAD_START))
    {
        if (!(buttons ^ oldbuttons) || buttons != (PAD_RIGHT_C|PAD_LEFT_C) ||
            (Pak_Data[cursorpos * 32] == 0))
        {
            exit = ga_nothing;
        }
        else
        {
            // load the password data in text format
            D_memcpy(&Passwordbuff, &Pak_Data[((cursorpos * 32) + 16)], 16);

            if (M_DecodePassword(Passwordbuff, &levelnum, &skill, 0) == 0)
            {
                CurPasswordSlot = 0;
                exit = ga_exit;
            }
            else
            {
                doPassword = true;
                CurPasswordSlot = 16;

                startmap = gamemap = levelnum;
                startskill = gameskill = skill;

                exit = ga_warped;
            }
        }
    }
    else
    {
        exit = ga_exit;
    }

    return exit;
}

void M_LoadPakDrawer(void) // 8000B270
{
    int i;
    char buffer[32];

    ST_DrawString(-1, 20, "Controller Pak", text_alpha | 0xc0000000);

    for(i = linepos; i < (linepos + 6); i++)
    {
        if (FilesUsed == -1) {
            D_memmove(buffer, "-");
        }
        else if (Pak_Data[i * 32] == 0) {
            D_memmove(buffer, "no save");
        }
        else {
            D_memmove(buffer, &Pak_Data[i * 32]);
        }

        ST_DrawString(60, (i - linepos) * 15 + 65, buffer, text_alpha | 0xc0000000);
    }

    if (linepos != 0) {
        ST_DrawString(60, 50, "\x8f more...", text_alpha | 0xffffff00);
    }

    if ((linepos + 6) <= ((Pak_Size >> 5) - 1)) {
        ST_DrawString(60, 155, "\x8e more...", text_alpha | 0xffffff00);
    }

    ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 56, MenuAnimationTic + 70, text_alpha | 0xffffff00);

    ST_DrawString(-1, 195, "press \x8D to exit", text_alpha | 0xffffff00);
    ST_DrawString(-1, 210, "press \x84\x85 to load", text_alpha | 0xffffff00);
}

int M_CenterDisplayTicker(void) // 8000B4C4
{
    unsigned int buttons, oldbuttons;
    int exit;

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = oldticbuttons[0] & 0xffff0000;

    if ((buttons == oldbuttons) || !(buttons & PAD_START))
    {
        if (buttons & PAD_LEFT)
        {
            Display_X -= 1;
            if (Display_X < -16)
                Display_X = -16;
        }
        else if (buttons & PAD_RIGHT)
        {
            Display_X += 1;
            if (Display_X > 24)
                Display_X = 24;
        }

        if (buttons & PAD_UP)
        {
            Display_Y -= 1;
            if (Display_Y < -20)
                Display_Y = -20;
        }
        else if (buttons & PAD_DOWN)
        {
            Display_Y += 1;
            if (Display_Y > 12)
                Display_Y = 12;
        }

        if (buttons & ALL_JPAD)
            I_MoveDisplay(Display_X, Display_Y);

        exit = 0;
    }
    else
    {
        S_StartSound(NULL, sfx_pistol);
        exit = 8;
    }

    return exit;
}

void M_CenterDisplayDrawer(void) // 8000B604
{
    ST_DrawString(-1, 20, "Center Display", text_alpha | 0xc0000000);
    ST_DrawString(-1, 114, "use control pad to adjust", text_alpha | 0xffffff00);
    ST_DrawString(-1, 210, "press \x8d to exit", text_alpha | 0xffffff00);
}

int M_ControlPadTicker(void) // 8000B694
{
    unsigned int buttons;
    unsigned int oldbuttons;
    int exit;
    int *tmpcfg, code;

    if ((gamevbls < gametic) && ((gametic & 3U) == 0)) {
        MenuAnimationTic = MenuAnimationTic + 1 & 7;
    }

    buttons = M_ButtonResponder(ticbuttons[0] & 0xffff);

    if (!(buttons & ALL_JPAD))
    {
        m_vframe1 = 0;
    }
    else
    {
        m_vframe1 = m_vframe1 - vblsinframe[0];
        if (m_vframe1 <= 0)
        {
            m_vframe1 = 0xf; // TICRATE / 2

            if (buttons & PAD_DOWN)
            {
                cursorpos += 1;
                if (cursorpos < 14)
                    S_StartSound(NULL, sfx_switch1);
                else
                    cursorpos = 13;

                if (cursorpos > (linepos + 5))
                    linepos += 1;
            }
            else
            {
                if (buttons & PAD_UP)
                {
                    cursorpos -= 1;
                    if (cursorpos < 0)
                        cursorpos = 0;
                    else
                        S_StartSound(NULL, sfx_switch1);

                    if (cursorpos < linepos)
                        linepos -= 1;
                }
            }
        }
    }

    buttons = ticbuttons[0] & 0xffff0000;
    oldbuttons = oldticbuttons[0] & 0xffff0000;

    if (buttons & PAD_START)
    {
        S_StartSound(NULL, sfx_pistol);
        exit = 8;
    }
    else
    {
        if (buttons == oldbuttons)
            exit = 0;
        else
        {
            tmpcfg = TempConfiguration;

            if (cursorpos == 0) // Set Default Configuration
            {
                if (!(buttons & (PAD_UP|PAD_LEFT)))
                {
                    if (buttons & (PAD_DOWN|PAD_RIGHT))
                    {
                        ConfgNumb += 1;
                        if(ConfgNumb > 4)
                            ConfgNumb = 0;
                    }
                }
                else
                {
                    ConfgNumb -= 1;
                    if (ConfgNumb < 0)
                        ConfgNumb = 4;
                }

                if ((buttons & (ALL_BUTTONS|ALL_JPAD)) != 0)
                {
                    D_memcpy(ActualConfiguration, DefaultConfiguration[ConfgNumb], (13 * sizeof(int)));
                    S_StartSound(NULL, sfx_switch2);
                    return 0;
                }
            }
            else // Set Custom Configuration
            {
                do
                {
                    code = *tmpcfg++;
                    if ((code & buttons) != 0)
                    {
                        TempConfiguration[cursorpos + 12] = code;
                        S_StartSound(NULL,sfx_switch2);
                        return 0;
                    }
                } while (tmpcfg != (int*)(TempConfiguration+13));
            }
            exit = 0;
        }
    }
    return exit;
}

void M_ControlPadDrawer(void) // 8000B988
{
    int i, lpos;
    int *tmpcfg;
    char **text;
    char buffer [44];

    ST_DrawString(-1, 20, "Control Pad", text_alpha | 0xc0000000);

    if (linepos < (linepos + 6))
    {
        text = &ControlText[linepos];
        lpos = linepos;
        do
        {
            if (lpos != 0)
            {
                i = 0;
                if(lpos != cursorpos || ((ticon & 8U) == 0))
                {
                    tmpcfg = TempConfiguration;
                    do
                    {
                        if ((*tmpcfg & TempConfiguration[lpos + 12]) != 0) break;

                        i += 1;
                        tmpcfg++;
                    } while (i != 13);

                    ST_DrawSymbol(60, ((lpos - linepos) * 18) + 68, i + 80, text_alpha | 0xffffff00);
                }
            }

            sprintf(buffer, *text, ConfgNumb + 1);
            ST_DrawString(80, ((lpos - linepos) * 18) + 68, buffer, text_alpha | 0xc0000000);

            lpos += 1;
            text += 1;
        } while (lpos < (linepos + 6));
    }

    if (linepos != 0) {
        ST_DrawString(80, 50, "\x8f more...", text_alpha | 0xffffff00);
    }

    if ((linepos + 6) < 14) {
        ST_DrawString(80, 176, "\x8e more...", text_alpha | 0xffffff00);
    }

    ST_DrawSymbol(23,(cursorpos - linepos) * 0x12 + 0x3b, MenuAnimationTic + 0x46, text_alpha | 0xffffff00);

    ST_DrawString(-1, 210, "press \x8d to exit", text_alpha | 0xffffff00);
}
