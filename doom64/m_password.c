/* m_password.c -- password (encode/decode) and menu password routines */

#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"
#include "r_local.h"

char *passwordChar = "bcdfghjklmnpqrstvwxyz0123456789?";    // 8005AC60
int passwordTable[10] = { 1, 8, 9, 5, 6, 2, 7, 0, 4, 3 };   // 8005AC80

char *hectic_demo = "rvnh3ct1cd3m0???"; // 8005ACA8
boolean run_hectic_demo = false;        // 8005A7A0

byte Passwordbuff[16];  // 800A55B0
int PassCodePos;        // 800A55C0
int PassInvalidTic;     // 800A55C4

boolean doPassword = false; // 8005A7A0
int CurPasswordSlot = 0;    // 8005ACBC

char *passFeatures = "3n4bl3f34tvr3s??"; // New Pass Code By [GEC]

void M_EncodePassword(byte *buff) // 8000BC10
{
    byte encode[10];
    int i;
    int bit;
    short decodebit[3];
    int passBit;
    int xbit1, xbit2, xbit3;
    int maxclip, maxshell, maxcell, maxmisl;
    player_t* player;

    player = &players[0];
    D_memset(encode, 0, sizeof(encode));

    //
    // Map and Skill
    //
    encode[0] = ((((nextmap & 63) << 2) & 0xff) | (gameskill & 3));

    //
    // Weapons
    //
    bit = 0;
    for(i = 0; i < NUMWEAPONS; i++)
    {
        if(i != wp_fist && i != wp_pistol)
        {
            if(player->weaponowned[i])
            {
                encode[1] |= (1 << bit);
                encode[1] = encode[1] & 0xff;
            }

            bit++;
        }
    }

    //
    // Get Maximun Ammo
    //
	maxclip = maxammo[am_clip];
	maxshell = maxammo[am_shell];
	maxcell = maxammo[am_cell];
	maxmisl = maxammo[am_misl];

	//
    // Backpack
    //
    if(player->backpack)
    {
        maxclip <<= 1;
		maxshell <<= 1;
		maxcell <<= 1;
		maxmisl <<= 1;
        encode[5] |= 0x80;
    }

    //
    // Clip
    //
    bit = (player->ammo[am_clip] << 3) / maxclip;
	if ((player->ammo[am_clip] << 3) % maxclip) { bit += 1; }
	encode[2] = bit << 4;

	//
	// Shell
	//
	bit = (player->ammo[am_shell] << 3) / maxshell;
	if ((player->ammo[am_shell] << 3) % maxshell) { bit += 1; }
	encode[2] |= bit;

	//
	// Cell
	//
	bit = (player->ammo[am_cell] << 3) / maxcell;
	if ((player->ammo[am_cell] << 3) % maxcell) { bit += 1; }
	encode[3] = bit << 4;

	//
	// Missile
	//
	bit = (player->ammo[am_misl] << 3) / maxmisl;
	if ((player->ammo[am_misl] << 3) % maxmisl) { bit += 1; }
	encode[3] |= bit;

	//
	// Health
	//
	bit = (player->health << 3) / 200;
	if ((player->health << 3) % 200) { bit += 1; }
	encode[4] = bit << 4;

	//
	// Armor
	//
	bit = (player->armorpoints << 3) / 200;
	if ((player->armorpoints << 3) % 200) { bit += 1; }
	encode[4] |= bit;

	//
    // ArmorType
    //
    encode[5] |= player->armortype;

    //
    // Artifacts
    //
    encode[5] |= (player->artifacts << 2);

    decodebit[0] = (*(short*)&encode[0]);
    decodebit[1] = (*(short*)&encode[2]);
    decodebit[2] = (*(short*)&encode[4]);

    *(short*)&encode[6] = (~(decodebit[0] + decodebit[1] + decodebit[2]));
    *(short*)&encode[8] = (~(decodebit[0] ^ decodebit[1] ^ decodebit[2]));

    for(i = 0; i < 10; i++)
    {
        bit = encode[passwordTable[i]];
        encode[i] = (encode[i] ^ bit);
    }

    bit = 0;

    while(bit < 80)
    {
        passBit = 0;

        if(bit < 0) {
            xbit2 = (bit + 7) >> 3;
        }
        else {
            xbit2 = bit >> 3;
        }

        xbit3 = (bit & 7);

        if(bit < 0)
        {
            if(xbit3 != 0) {
                xbit3 -= 8;
            }
        }

        if((encode[xbit2] & (0x80 >> xbit3)))
        {
            passBit = 16;
        }

        xbit1 = 8;
        bit++;

        for(i = 0; i < 4; i++)
        {
            if(bit < 0) {
                xbit2 = (bit + 7) >> 3;
            }
            else {
                xbit2 = bit >> 3;
            }

            xbit3 = (bit & 7);

            if(bit < 0)
            {
                if(xbit3 != 0) {
                    xbit3 -= 8;
                }
            }

            if((encode[xbit2] & (0x80 >> xbit3)))
            {
                passBit |= xbit1;
            }

            xbit1 >>= 1;
            bit++;
        }

        buff[((bit - 1) / 5)] = passBit;
    }
}

int M_DecodePassword(byte *inbuff, int *levelnum, int *skill, player_t *player) // 8000C194
{
    byte data[16];
    byte decode[10];
    int bit;
    int i, j;
    short xbit1, xbit2, xbit3;
    short x, y;
    int passBit;
    int decodeBit;
    byte checkByte;

    D_memcpy(data, inbuff, 16);

    //
    // Decode Password
    //
    bit = 0;

    while(bit < 80)
    {
        passBit = 0;
        decodeBit = 0x80;
        checkByte = 0;

        i = 0;

        while(i != 8)
        {
            i += 4;

            for(j = 0; j < 4; j++)
            {
                checkByte = data[bit / 5];
                if((checkByte & (16 >> (bit % 5))))
                {
                    passBit |= decodeBit;
                }

                bit++;
                decodeBit >>= 1;
            }
        }

        if((bit - 1) >= 0)
        {
            checkByte = ((bit - 1) >> 3);
        }
        else
        {
            checkByte = (((bit - 1) + 7) >> 3);
        }

        decode[checkByte] = passBit;
    }

    for(i = 9; i >= 0; i--)
    {
        bit = decode[passwordTable[i]];
        decode[i] = (decode[i] ^ bit);
    }

    //
    // Verify Decoded Password
    //

    xbit1 = *(short*)&decode[0];
    xbit2 = *(short*)&decode[2];
    xbit3 = *(short*)&decode[4];

    x = ((~((xbit1 + xbit2) + xbit3) << 16) >> 16);
    y = *(short*)&decode[6];

    if(x != y)
    {
        return false;
    }

    x = ((~(xbit1 ^ (xbit2 ^ xbit3)) << 16) >> 16);
    y = *(short*)&decode[8];

    if(x != y)
    {
        return false;
    }

    //
    // Get Map
    //
    *levelnum = (decode[0] >> 2);

    //
    // Verify Map
    //
    if ((*levelnum == 0) || (*levelnum >= TOTALMAPS))
    {
        return false;
    }

    //
    // Get Skill
    //
    *skill = (decode[0] & 3);

    //
    // Verify Skill
    //
    if(*skill > sk_nightmare)
    {
        return false;
    }

    //
    // Verify Ammo (Shell / Clip)
    //
    if((decode[2] & 0xf) >= 9 || (decode[2] >> 4) >= 9)
    {
        return false;
    }

    //
    // Verify Ammo (Missile / Cell)
    //
    if((decode[3] & 0xf) >= 9 || (decode[3] >> 4) >= 9)
    {
        return false;
    }

    //
    // Verify (Armor / Health)
    //
    if((decode[4] & 0xf) >= 9 || (decode[4] >> 4) >= 9)
    {
        return false;
    }

    //
    // Verify Armortype
    //
    if((decode[5] & 3) >= 3)
    {
        return false;
    }

    bit = 0;
    if (player != 0)
    {
        //
        // Get Weapons
        //
        for(i = 0; i < NUMWEAPONS; i++)
        {
            if(i != wp_fist && i != wp_pistol)
            {
                if(decode[1] & (1 << bit))
                {
                    player->weaponowned[i] = true;
                }

                bit++;
            }
        }

        //
        // Get Backpack
        //
        if(decode[5] & 0x80)
        {
            if (!player->backpack)
            {
                player->backpack = true;
                player->maxammo[am_clip]    = (maxammo[am_clip] << 1);
                player->maxammo[am_shell]   = (maxammo[am_shell] << 1);
                player->maxammo[am_cell]    = (maxammo[am_cell] << 1);
                player->maxammo[am_misl]    = (maxammo[am_misl] << 1);
            }
        }

        //
        // Get Clip
        //
        bit = (decode[2] >> 4) * player->maxammo[am_clip];
        if (bit < 0) { bit += 7; }
        player->ammo[am_clip] = bit >> 3;

        //
        // Get Shell
        //
        bit = (decode[2] & 0xf) * player->maxammo[am_shell];
        if (bit < 0) { bit += 7; }
        player->ammo[am_shell] = bit >> 3;

        //
        // Get Cell
        //
        bit = (decode[3] >> 4) * player->maxammo[am_cell];
        if (bit < 0) { bit += 7; }
        player->ammo[am_cell] = bit >> 3;

        //
        // Get Missile
        //
        bit = (decode[3] & 0xf) * player->maxammo[am_misl];
        if (bit < 0) { bit += 7; }
        player->ammo[am_misl] = bit >> 3;

        //
        // Get Health
        //
        bit = (decode[4] >> 4) * 200;
        if (bit < 0) { bit += 7; }
        player->health = bit >> 3;

        //
        // Get Armor
        //
        bit = (decode[4] & 0xf) * 200;
        if (bit < 0) { bit += 7; }
        player->armorpoints = bit >> 3;

        //
        // Get Armor Type
        //
        player->armortype = (decode[5] & 3);

        //
        // Get Artifacts
        //
        player->artifacts = ((decode[5] >> 2) & 7);

        //
        // Apply Health on mobj_t
        //
        player->mo->health = player->health;

        //
        // Set Cheat Menu If Password Leads To Map 01
        //
        if((decode[0] >> 2) == 1)
        {
            FeaturesUnlocked = true;
        }
    }

    return true;
}

void M_PasswordStart(void) // 8000C710
{
    PassInvalidTic = 0;
    PassCodePos = 0;
    last_ticon = 0;
    M_FadeInStart();
}

void M_PasswordStop(void) // 8000C744
{
    S_StartSound(NULL, sfx_pistol);
    M_FadeOutStart(8);
}

int M_PasswordTicker(void) // 8000C774
{
    byte *passbuf;
    char *hpassbuf;
    unsigned int buttons;
    unsigned int oldbuttons;
    boolean playsound;
    int exit;
    int skill;
    int levelnum;

    if (last_ticon)
    {
        if ((ticon - last_ticon) < 16)
            exit = ga_nothing;
        else
            exit = ga_warped;

        return exit;
    }

    if (PassInvalidTic)
    {
        if((gametic & 1U) == 0)
        {
            PassInvalidTic -= 1;
            if((PassInvalidTic & 7) == 4) {
                S_StartSound(NULL, sfx_itemup);
            }
        }
    }

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = oldticbuttons[0] & 0xffff0000;

    if (!(buttons & (ALL_TRIG|PAD_A|PAD_B|ALL_JPAD)))
    {
        m_vframe1 = 0;
    }
    else
    {
        m_vframe1 -= vblsinframe[0];

        if (m_vframe1 <= 0)
        {
            m_vframe1 = 0xf; // TICRATE / 2

            playsound = false;

            if (buttons & PAD_UP)
            {
                if (PassCodePos > 7)
                {
                    playsound = true;
                    PassCodePos -= 8;
                }
            }
            if ((buttons & PAD_DOWN) && (PassCodePos < 24))
            {
                playsound = true;
                PassCodePos += 8;
            }

            if (buttons & PAD_LEFT)
            {
                if (PassCodePos > 0)
                {
                    playsound = true;
                    PassCodePos -= 1;
                }
            }
            else if ((buttons & PAD_RIGHT) && (PassCodePos < 31))
            {
                playsound = true;
                PassCodePos += 1;
            }

            if (playsound)
            {
                S_StartSound(NULL, sfx_switch1);
            }
        }
    }

    if (buttons == oldbuttons)
    {
        exit = ga_nothing;
    }
    else
    {
        if (buttons & PAD_START)
        {
            exit = ga_exit;
        }
        else
        {
            if (!(buttons & (ALL_TRIG|PAD_A|PAD_B|PAD_UP_C|PAD_DOWN_C|PAD_RIGHT_C)))
            {
                if (buttons & PAD_LEFT_C)
                {
                    S_StartSound(0, sfx_switch2);

                    CurPasswordSlot -= 1;
                    if (CurPasswordSlot < 0)
                        CurPasswordSlot = 0;

                    Passwordbuff[CurPasswordSlot] = 0;
                }
            }
            else
            {
                S_StartSound(0, sfx_switch2);

                if (CurPasswordSlot < 16)
                {
                    Passwordbuff[CurPasswordSlot] = (byte)PassCodePos;
                    CurPasswordSlot += 1;
                }

                if (CurPasswordSlot > 15)
                {
                    hpassbuf = hectic_demo;
                    passbuf = Passwordbuff;
                    do
                    {
                        if (passwordChar[*passbuf++] != *hpassbuf++)
                            break;

                    } while (hpassbuf != (hectic_demo + 16));

                    if ((hectic_demo + 15) < hpassbuf)
                    {
                        run_hectic_demo = true;
                        return ga_exit;
                    }

                    // [GEC] New Password Code Enable Features Menu.
                    fpassbuf = passFeatures;
                    passbuf = Passwordbuff;
                    do
                    {
                        if (passwordChar[*passbuf++] != *fpassbuf++)
                            break;

                    } while (fpassbuf != (passFeatures + 16));

                    if ((passFeatures + 15) < fpassbuf)
                    {
                        FeaturesUnlocked = true;
                        return ga_exit;
                    }

                    if (M_DecodePassword(Passwordbuff, &levelnum, &skill, NULL) == 0)
                    {
                        PassInvalidTic = 16;
                    }
                    else
                    {
                        doPassword = true;
                        startmap = gamemap = levelnum;
                        startskill = gameskill = skill;
                        last_ticon = ticon;
                    }
                }
            }

            exit = ga_nothing;
        }
    }

    return exit;
}

void M_PasswordDrawer(void) // 8000CAF0
{
    byte    pass[2];
    byte    c;
    int     texid, cnt;
    int     xpos, ypos, pos1;

    ST_DrawString(-1, 20, "Password", text_alpha | 0xc0000000);

    for(cnt = 0; cnt < 32; cnt++)
    {
        pos1 = cnt;
        if (cnt < 0) {
            pos1 = cnt + 7;
        }
        pos1 >>= 3;
        ypos = (pos1 * 20) + 60;

        if ((cnt == PassCodePos) && (ticon & 8))
            continue;

        c = passwordChar[cnt];
        if ((byte)(c - 'a') < 26)
        {
            texid = (byte)(c - 55);
            ypos = (pos1 * 20) + 63;
        }
        else if ((byte)(c - '0') < 10)
        {
            texid = (byte)(c - '0');
        }
        else if (c == '?')
        {
            texid = 14;
        }

        pos1 = cnt & 7;
        if ((cnt < 0) && (pos1 != 0)) {
            pos1 -= 8;
        }
        xpos = (pos1 * 20) + 84;

        ST_DrawSymbol(xpos, ypos, texid, text_alpha | 0xc0000000);
    }

    cnt = PassCodePos;

    pos1 = cnt & 7;
    if ((cnt < 0) && (pos1 != 0)) {
        pos1 -= 8;
    }
    xpos = (pos1 * 20) + 80;

    pos1 = cnt;
    if (pos1 < 0) {
        pos1 = cnt + 7;
    }
    pos1 >>= 3;
    ypos = (pos1 * 20) + 59;

    ST_DrawSymbol(xpos, ypos, 79, text_alpha | 0xffffff00);

    xpos = 47;
    cnt = 0;
    if ((PassInvalidTic & 4) == 0)
    {
        pass[1] = '\0';
        do
        {
            if ((cnt & 3) == 0) {
                xpos += 6;
            }

            if (cnt < CurPasswordSlot) {
                pass[0] = passwordChar[Passwordbuff[cnt]];
            }
            else {
                pass[0] = '.';
            }

            ST_DrawString(xpos, 160, pass, text_alpha | 0xc0000000);

            xpos += 13;
            cnt += 1;
        } while (cnt != 16);
    }
    else
    {
        ST_DrawString(-1, 160, "Invalid Password", text_alpha | 0xffffff00);
    }

    ST_DrawString(-1,195, "press \x8d to exit", text_alpha | 0xffffff00);
    ST_DrawString(-1,210, "press \x84 to change", text_alpha | 0xffffff00);
}
