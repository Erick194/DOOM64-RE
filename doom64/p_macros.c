/* P_Macros.c */
#include "doomdef.h"
#include "r_local.h"
#include "p_local.h"
#include "st_main.h"

/* MACRO Variables */
macro_t     *activemacro;       // 800A6094
mobj_t      *macroactivator;    // 800A6098
line_t      macrotempline;      // 800A60A0
line_t      *macroline;         // 800A60EC
thinker_t   *macrothinker;      // 800A60F0
int         macrointeger;       // 800A60F4
macro_t     *restartmacro;      // 800A60F8
int         macrocounter;       // 800A60FC
macroactivator_t macroqueue[4]; // 800A6100
int         macroidx1;          // 800A6120
int         macroidx2;          // 800A6124

int P_StartMacro(int macroindex, line_t *line, mobj_t *thing) // 80021088
{
    macro_t *macro;

    if (activemacro)
        return 0;

    macro = macros[macroindex-256];
    if (macro->id <= 0)
    {
        if (macro->id == 0)
            line->special = 0;

        return 0;
    }

    activemacro = macro;
    macroactivator = thing;
    macrothinker = NULL;
    macroline = line;

    D_memcpy(&macrotempline, line, sizeof(line_t));
    P_ChangeSwitchTexture(line, line->special & MLU_REPEAT);

    return 1;
}

int P_SuspendMacro(void) // 80021148
{
    macroactivator_t *activatorInfo;

    if(!activemacro)
        return 0;

    macrothinker = NULL;
    macrocounter = 0;
    activemacro = NULL;

    if (!(macroline->special & MLU_REPEAT))
    {
        macros[SPECIALMASK(macroline->special)-256]->id = 0;
        macroline->special = 0;
    }

    if (macroidx2 != macroidx1)
    {
        activatorInfo = &macroqueue[macroidx2];
        macroidx2 = (macroidx2 + 1) & 3;

        P_ActivateLineByTag(activatorInfo->tag, activatorInfo->activator);
    }

    return 1;
}

void P_ToggleMacros(int tag, boolean toggleon) // 80021214
{
    macro_t *macro = macros[tag-256];

    if(toggleon)
    {
        if(macro->id >= 0)
            return;
    }
    else
    {
        if(macro->id <= 0)
            return;
    }

    macro->id = -macro->id;
}

void P_RunMacros(void) // 8002126C
{
    thinker_t *thinker;
    int id;

    if(!activemacro)
        return;

    if(macrothinker)
    {
        //ST_DebugPrint("macrothinker %x wait", macrothinker);
        return; /* must wait for this thinker to finish */
    }

    //ST_DebugPrint("activemacro->id %d",activemacro->id);


    /* keep track of the current thinker */
    thinker = thinkercap.prev;

    while(activemacro->id != 0)
    {
        id = activemacro->id;

        macrotempline.special = activemacro->special;
        macrotempline.tag = activemacro->tag;

        activemacro++;

        /* invoke a line special from this macro */
        P_UseSpecialLine(&macrotempline, macroactivator);

        /* keep executing macros until reaching a new batch ID */
        if (id != activemacro->id)
        {
            if (activemacro->id == 0)
            {
                //ST_DebugPrint("P_SuspendMacro act->id %d",activemacro->id);
                P_SuspendMacro();
                return;
            }

            /* if the last macro produced a new thinker then keep track of it */
            if (thinker != thinkercap.prev)
            {
                /* don't execute any more macros until this thinker is done */
                macrothinker = thinkercap.prev;
                return;
            }
        }

        thinker = thinkercap.prev;
    }
}

void P_RestartMacro(line_t *line, int id) // 80021384
{
    macro_t *macro;

    if(!activemacro)
        return;

    if (macrocounter == 0)
    {
        macro = macros[SPECIALMASK(macroline->special)-256];

        /* find the first macro in the batch to restart on */
        while((macro->id != 0) && (macro->id != id))
        {
            macro++;
        }

        if ((macro->id != 0) && (line->tag != 0))
        {
            /* its now set */
            activemacro = macro;
            restartmacro = macro;
            macrocounter = line->tag;

        }
    }
    else
    {
        if(macrocounter > 0)
        {
            if(--macrocounter == 0)
                return;
        }

        // restart the macro
        activemacro = restartmacro;
    }
}
