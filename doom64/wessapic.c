
/* WESS API INCLUDES */
#include "wessapi.h"	// audio stuff...
#include "seqload.h"
#include "soundhw.h"
#include "wessarc.h"
#include "wessseq.h"

#include "funqueue.h"

#include "graph.h" // debug

#ifndef NOUSEWESSCODE
extern pmasterstat *pm_stat;    //0x800B41CC

void wess_register_callback(char marker_ID, void(*function_pointer)(char, short)) // 80030EF0
{
	static char si, sn;				//800B41D0, 800B41D1
	static callback_status *cbs;	//800B41D4

	char _marker_ID;
	void(*_function_pointer)(char, short);

	_marker_ID = marker_ID;
	_function_pointer = function_pointer;

	if (!Is_Module_Loaded())
	{
		return;
	}

	wess_disable();

	sn = pm_stat->callbacks_active;
	si = wess_driver_callbacks;
	if (sn != si) /* check if we are full of callbacks */
	{
		cbs = pm_stat->pcalltable;
		while (si--)
		{
			if (!cbs->active)
			{
			    cbs->curval = 0;
				cbs->type = _marker_ID;
				cbs->callfunc = _function_pointer;
				pm_stat->callbacks_active++;
				break;
			}
			cbs++;
		}
	}

	wess_enable();
}

void wess_delete_callback(char marker_ID) // 80030FF0
{
	static char si, sn;				//800B41D8, 800B41D9
	static callback_status *cbs;	//800B41DC

	char _marker_ID;

	_marker_ID = marker_ID;

	if (!Is_Module_Loaded())
	{
		return;
	}

	wess_disable();

	sn = pm_stat->callbacks_active;
	if (sn)
	{
		cbs = pm_stat->pcalltable;
		si = wess_driver_callbacks;
		while (si--)
		{
			if (cbs->active)
			{
				if (cbs->type == _marker_ID)
				{
					cbs->active = 0;
					--pm_stat->callbacks_active;
					break;
				}
				if (!--sn) break;
			}
			cbs++;
		}
	}

	wess_enable();
}

#endif
