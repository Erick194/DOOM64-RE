
/* WESS API INCLUDES */
#include "wessapi.h"	// audio stuff...
#include "seqload.h"
#include "soundhw.h"
#include "wessarc.h"
#include "wessseq.h"

#include "funqueue.h"

#include "graph.h" // debug

#ifndef NOUSEWESSCODE

unsigned char   master_sfx_volume = 0x7F;   // 80075834
unsigned char   master_mus_volume = 0x7F;   // 80075835
unsigned char   pan_status = 1;             // 80075836
int             enabledecay = 0;            // 8005D980

extern pmasterstat *pm_stat;    //0x800B41CC

char wess_master_sfx_volume_get(void) // 80031120
{
	if (!Is_Module_Loaded())
	{
		return 0;
	}

	return (master_sfx_volume);
}

char wess_master_mus_volume_get(void) // 80031158
{
	if (!Is_Module_Loaded())
	{
		return 0;
	}

	return (master_mus_volume);
}

void wess_master_sfx_vol_set(char volume) // 80031190
{
    char _volume;

    _volume = volume;

	wess_disable();
	queue_the_function(QUEUE_MASTER_SFX_VOL_SET);
	queue_the_data(&_volume, sizeof(char));
	wess_enable();
}

void queue_wess_master_sfx_vol_set(char volume);

void run_queue_wess_master_sfx_vol_set(void) // 800311CC
{
	char volume;

	unqueue_the_data(&volume, sizeof(char));
	queue_wess_master_sfx_vol_set(volume);
}

void queue_wess_master_sfx_vol_set(char volume) // 800311FC
{
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	char *ppos;
	int li, lj;
	char tmpppos[16];

	char _volume;

	_volume = volume;

	if (!Is_Module_Loaded())
	{
		return;
	}

	master_sfx_volume = _volume;

	wess_disable();

	/* search for all sequences with this number and turn them off */
	nt = wess_driver_sequences;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;

	if (na)
	{
		while (nt--)
		{
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				li = psq_stat->tracks_active;
				lj = pm_stat->max_trks_perseq;
				/* *lpdest refers to an active track if not 0xFF */
				lpdest = psq_stat->ptrk_indxs;
				while (lj--)
				{
					if (*lpdest != 0xFF)
					{
						ptmp = (pm_stat->ptrkstattbl + (*lpdest));

						if (ptmp->sndclass == SNDFX_CLASS)
						{
							//save
							ppos = ptmp->ppos;

							// set tmp buffer ppos
							ptmp->ppos = tmpppos;
							ptmp->ppos[0] = VolumeMod;
							ptmp->ppos[1] = ptmp->volume_cntrl;
							ptmp->volume_cntrl = 0;

							CmdFuncArr[ptmp->patchtype][VolumeMod](ptmp);

							//restore
							ptmp->ppos = ppos;
						}
						if (!--li) break;
					}
					lpdest++;
				}

				if (!--na) break;
			}
			psq_stat++;
		}
	}

	wess_enable();
}

void wess_master_mus_vol_set(char volume) // 800313DC
{
    char _volume;

    _volume = volume;

	wess_disable();
	queue_the_function(QUEUE_MASTER_MUS_VOL_SET);
	queue_the_data(&_volume, sizeof(char));
	wess_enable();
}


void queue_wess_master_mus_vol_set(char volume);

void run_queue_wess_master_mus_vol_set(void) // 80031418
{
	char volume;

	unqueue_the_data(&volume, sizeof(char));
	queue_wess_master_mus_vol_set(volume);
}

void queue_wess_master_mus_vol_set(char volume) // 80031448
{
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	char *ppos;
	int li, lj;
	char tmpppos[16];

	char _volume;

	_volume = volume;

	if (!Is_Module_Loaded())
	{
		return;
	}

	master_mus_volume = _volume;

	wess_disable();

	/* search for all sequences with this number and turn them off */
	nt = wess_driver_sequences;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;

	if (na)
	{
		while (nt--)
		{
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				li = psq_stat->tracks_active;
				lj = pm_stat->max_trks_perseq;
				/* *lpdest refers to an active track if not 0xFF */
				lpdest = psq_stat->ptrk_indxs;
				while (lj--)
				{
					if (*lpdest != 0xFF)
					{
						ptmp = (pm_stat->ptrkstattbl + (*lpdest));

						if (ptmp->sndclass == MUSIC_CLASS)
						{
							//save
							ppos = ptmp->ppos;

							// set tmp buffer ppos
							ptmp->ppos = tmpppos;
							ptmp->ppos[0] = VolumeMod;
							ptmp->ppos[1] = ptmp->volume_cntrl;
							ptmp->volume_cntrl = 0;

							CmdFuncArr[ptmp->patchtype][VolumeMod](ptmp);

							//restore
							ptmp->ppos = ppos;
						}
						if (!--li) break;
					}
					lpdest++;
				}

				if (!--na) break;
			}
			psq_stat++;
		}
	}

	wess_enable();
}

char wess_pan_mode_get(void) // 80031624
{
	return pan_status;
}

void wess_pan_mode_set(char mode) // 80031634
{
	pan_status = mode;
}
#endif
