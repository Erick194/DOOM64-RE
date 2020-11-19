
/* ULTRA64 LIBRARIES */
#include <ultra64.h>
#include "ultratypes.h"
#include <libaudio.h>

/* WESS API INCLUDES */
#include "wessapi.h"	// audio stuff...
#include "seqload.h"
#include "soundhw.h"
#include "wessarc.h"
#include "wessseq.h"

#include "funqueue.h"

#include "wessedit.h"
#include "wesstrak.h"
#include "wesshand.h"

#ifndef NOUSEWESSCODE
extern pmasterstat *pm_stat;			//0x800B41CC
extern unsigned char CmdLength[36];
extern unsigned char CmdSort[44];
extern void(*DrvFunctions[36])(track_status *);

/* used by wess trigger functions */
enum HandleFlag { NoHandle, YesHandle };

enum HandleStatus { HANDLE_INVALID, HANDLE_STOPPED, HANDLE_PLAYING };


char scratch_area[32];//800B41E0

void tracksetspecial(track_status *ptmp, TriggerPlayAttr *attr); // 8003429C
void trackgetspecial(track_status *ptmp, TriggerPlayAttr *attr); // 80034508

sequence_status *gethandleseq(int handle) // 80032E80
{
	char hindx;
	sequence_status *psq;

	if ((handle > 0) && (handle <= wess_driver_sequences))
	{
		hindx = handle - 1;
		if ((psq = (pm_stat->pseqstattbl + hindx))->flags & SEQ_HANDLE)
		{
			return(psq);
		}
	}
	return(NULL);
}

track_status *gethandletrk(int handle, int track) // 80032EE0
{
	char hindx;
	sequence_status *psq;
	track_status *ptrk;

	if (handle > 0 && handle <= wess_driver_sequences)
	{
		hindx = handle - 1;
		if ((psq = (pm_stat->pseqstattbl + hindx))->flags & SEQ_HANDLE)
		{
			if (*(psq->ptrk_indxs + track) != 0xFF)
			{
				if ((ptrk = pm_stat->ptrkstattbl + *(psq->ptrk_indxs + track))->flags & TRK_HANDLED)
				{
					return(ptrk);
				}
			}
		}
	}
	return(NULL);
}

int wess_handle_get(int seq_num) // 80032F80
{
	sequence_data *psq_info;

	if (!Is_Seq_Num_Valid(seq_num))
		return(NULL);

	psq_info = (pm_stat->pmod_info->pseq_info + seq_num);
	return wess_seq_structrig(psq_info, seq_num, 0, YesHandle, NULL);
}

int wess_handle_status(int handle) // 80032FD8
{
	sequence_status *psq_stat;
	int status;

	if(!Is_Module_Loaded())
		return HANDLE_INVALID;

	psq_stat = gethandleseq(handle);

	status = HANDLE_INVALID;
	if (psq_stat)
	{
		if (psq_stat->playmode)
		{
			if (psq_stat->playmode == SEQ_STATE_STOPPED)
				status = HANDLE_STOPPED;
			else if (psq_stat->playmode == SEQ_STATE_PLAYING)
				status = HANDLE_PLAYING;
		}
	}

	return status;
}

void wess_handle_return(int handle) // 80033048
{
	sequence_status *psq_stat;
	track_status *ptmp;

	int li, lj;
	char *lpdest;

	int _handle;

	_handle = handle;

	if (!Is_Module_Loaded())
		return;

	wess_disable();

	psq_stat = gethandleseq(_handle);

	if (psq_stat)
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
				ptmp->flags &= ~TRK_HANDLED;
				CmdFuncArr[ptmp->patchtype][TrkOff](ptmp);
				if (!--li) break;
			}
			lpdest++;
		}

		psq_stat->flags &= ~SEQ_HANDLE;
	}

	wess_enable();

	return;
}

void wess_handle_play_special(int handle, TriggerPlayAttr *attr) // 80033180
{
	sequence_status *psq_stat;
	track_status *ptmp;

	int li, lj;
	char *lpdest;

	int _handle;
	TriggerPlayAttr *_attr;

	_handle = handle;
	_attr = attr;

	if (!Is_Module_Loaded())
		return;

	wess_disable();

	psq_stat = gethandleseq(_handle);

	if (psq_stat)
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
				trackstart(ptmp, psq_stat);
				tracksetspecial(ptmp, _attr);
				if (!--li) break;
			}
			lpdest++;
		}

		psq_stat->playmode = SEQ_STATE_PLAYING;
	}

	wess_enable();

	return;
}

void wess_handle_play(int handle) // 80033298()
{
	wess_handle_play_special(handle, 0);
}

void wess_handle_play_track_special(int handle, int track, TriggerPlayAttr *attr) // 800332B8
{
	sequence_status *psq_stat;
	track_status *ptmp;

	int _handle;
	int _track;
	TriggerPlayAttr *_attr;

	_handle = handle;
	_track = track;
	_attr = attr;

	if (!Is_Module_Loaded())
		return;

	wess_disable();

	psq_stat = gethandleseq(_handle);

	if (psq_stat)
	{
		ptmp = gethandletrk(_handle, _track);

		if (ptmp)
		{
			trackstart(ptmp, psq_stat);
			tracksetspecial(ptmp, _attr);
		}
	}

	wess_enable();

	return;
}

void wess_handle_play_track(int handle, int track) // 80033344
{
	wess_handle_play_track_special(handle, track, 0);
	return;
}

void wess_handle_get_special(int handle, TriggerPlayAttr *attr) // 80033364
{
	sequence_status *psq_stat;
	track_status *ptmp;

	int li, lj;
	char *lpdest;

	int _handle;
	TriggerPlayAttr *_attr;

	_handle = handle;
	_attr = attr;

	if (!Is_Module_Loaded())
		return;

	wess_disable();

	psq_stat = gethandleseq(_handle);

	if (psq_stat)
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
				trackgetspecial(ptmp, _attr);
				if (!--li) break;
			}
			lpdest++;
		}
	}

	wess_enable();

	return;
}

void wess_handle_set_special(int handle, TriggerPlayAttr *attr) // 80033454
{
	sequence_status *psq_stat;
	track_status *ptmp;

	int li, lj;
	char *lpdest;

	int _handle;
	TriggerPlayAttr *_attr;

	_handle = handle;
	_attr = attr;

	if (!Is_Module_Loaded())
		return;

	wess_disable();

	psq_stat = gethandleseq(_handle);

	if (psq_stat)
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
				tracksetspecial(ptmp, _attr);
				if (!--li) break;
			}
			lpdest++;
		}
	}

	wess_enable();

	return;
}

void wess_handle_stop(int handle) // 80033544
{
	sequence_status *psq_stat;
	track_status *ptmp;

	int li, lj;
	char *lpdest;

	int _handle;

	_handle = handle;

	if (!Is_Module_Loaded())
		return;

	wess_disable();

	psq_stat = gethandleseq(_handle);

	if (psq_stat)
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
				CmdFuncArr[ptmp->patchtype][TrkOff](ptmp);
				if (!--li) break;
			}
			lpdest++;
		}
	}

	wess_enable();

	return;
}

void wess_handle_fastsettempo(int handle, short tempo) // 80033658
{
	sequence_status *psq_stat;
	track_status *ptmp;

	int li, lj;
	char *lpdest;
	unsigned long ppi;

	int _handle;
	short _tempo;

	_handle = handle;
	_tempo = tempo;

	if (!Is_Module_Loaded())
		return;

	wess_disable();

	psq_stat = gethandleseq(_handle);
	ppi = 0;

	if (psq_stat)
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
				ptmp->qpm = _tempo;
				if (ppi == 0)
					ppi = CalcPartsPerInt(GetIntsPerSec(), ptmp->ppq, ptmp->qpm);
				ptmp->ppi = ppi;

				if (!--li) break;
			}
			lpdest++;
		}
	}

	wess_enable();

	return;
}

int wess_handle_fade(void) // 80033788
{
	return Is_Module_Loaded();
}

void wess_handle_resetposition(int handle) // 800337B0
{
	sequence_status *psq_stat;
	track_status *ptmp;

	int li, lj;
	char *lpdest;

	int _handle;

	_handle = handle;

	if (!Is_Module_Loaded())
		return;

	wess_disable();

	psq_stat = gethandleseq(_handle);

	if (psq_stat)
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
				CmdFuncArr[ptmp->patchtype][TrkMute](ptmp);

				ptmp->starppi = 0;
				ptmp->totppi = 0;
				ptmp->ppos = Read_Vlq(ptmp->pstart, &ptmp->deltatime);

				if (!--li) break;
			}
			lpdest++;
		}
	}

	wess_enable();
	return;
}

int wess_track_gotoposition(track_status *ptmp, int position, char *ppos) // 800338F0
{
	int deltatime;
	int status;
	int accppi;


	deltatime = ptmp->deltatime;
	accppi = ptmp->totppi + (ptmp->starppi >> 16);

	while (1)
	{
		accppi += deltatime;

		if (position <= accppi)
		{
			status = 1;
			break;
		}

		if (*ptmp->ppos == TrkEnd)
		{
			status = 0;
			break;
		}

		if (*ptmp->ppos == NoteOn || *ptmp->ppos == NoteOff)
		{
			ptmp->ppos += CmdLength[*ptmp->ppos];
			ptmp->ppos = Read_Vlq(ptmp->ppos, &deltatime);
			continue;
		}
		else
		{
			if ((*ptmp->ppos < PatchChg) || (*ptmp->ppos > NoteOff))
			{
				if ((*ptmp->ppos < StatusMark) || (*ptmp->ppos > NullEvent))
				{
					Eng_SeqEnd(ptmp);
				}
				else
				{
					DrvFunctions[*ptmp->ppos](ptmp);

					if (ptmp->flags & TRK_SKIP)
					{
						ptmp->flags &= ~TRK_SKIP;
					}
					else
					{
						ptmp->ppos += CmdLength[*ptmp->ppos];
						ptmp->ppos = Read_Vlq(ptmp->ppos, &deltatime);
					}
				}
			}
			else
			{
				CmdFuncArr[ptmp->patchtype][*ptmp->ppos](ptmp);
				ptmp->ppos += CmdLength[*ptmp->ppos];
				ptmp->ppos = Read_Vlq(ptmp->ppos, &deltatime);
			}
		}
	}

	if (status)
	{
		ptmp->deltatime = accppi - position;
		ptmp->starppi = 0;
		ptmp->totppi = position;
	}
	else
	{
		ptmp->deltatime = 0;
		ptmp->starppi = 0;
		ptmp->totppi = accppi;
	}

	ppos = ptmp->ppos;

	return ptmp->totppi;
}

int wess_track_setposition(track_status *ptmp, int position, char *ppos) // 80033B24
{
	int deltatime;
	int val;
	int accppi;
	int status;

	deltatime = 0;
	accppi = 0;
	ptmp->ppos = ptmp->pstart;
	ptmp->ppos = Read_Vlq(ptmp->ppos, &deltatime);

	while (1)
	{
		accppi += deltatime;

		if (position <= accppi)
		{
			status = 1;
			break;
		}

		if (*ptmp->ppos == TrkEnd)
		{
			status = 0;
			break;
		}

		if (*ptmp->ppos == NoteOn || *ptmp->ppos == NoteOff)
		{
			ptmp->ppos += CmdLength[*ptmp->ppos];
			ptmp->ppos = Read_Vlq(ptmp->ppos, &deltatime);
			continue;
		}
		else
		{
			if ((*ptmp->ppos < PatchChg) || (*ptmp->ppos > NoteOff))
			{
				if ((*ptmp->ppos < StatusMark) || (*ptmp->ppos > NullEvent))
				{
					Eng_SeqEnd(ptmp);
				}
				else
				{
					DrvFunctions[*ptmp->ppos](ptmp);

					if (ptmp->flags & TRK_SKIP)
					{
						ptmp->flags &= ~TRK_SKIP;
					}
					else
					{
						ptmp->ppos += CmdLength[*ptmp->ppos];
						ptmp->ppos = Read_Vlq(ptmp->ppos, &deltatime);
					}
				}
			}
			else
			{
				CmdFuncArr[ptmp->patchtype][*ptmp->ppos](ptmp);
				ptmp->ppos += CmdLength[*ptmp->ppos];
				ptmp->ppos = Read_Vlq(ptmp->ppos, &deltatime);
			}
		}
	}

	if (status)
	{
		ptmp->deltatime = (accppi - position);
		ptmp->starppi = 0;
		ptmp->totppi = position;
	}
	else
	{
		ptmp->deltatime = 0;
		ptmp->starppi = 0;
		ptmp->totppi = accppi;
	}

	ppos = ptmp->ppos;

	return ptmp->totppi;
}

int wess_handle_advposition(int handle, int position) // 80033D58
{
	sequence_status *psq_stat;
	track_status *ptmp;
	char ppos[12];

	int li, lj;
	char *lpdest;

	int _handle, _position;

	_handle = handle;
	_position = position;

	if (!Is_Module_Loaded())
		return 0;

	wess_disable();

	psq_stat = gethandleseq(_handle);

	if (psq_stat)
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
				CmdFuncArr[ptmp->patchtype][TrkMute](ptmp);
				wess_track_gotoposition(ptmp, ptmp->totppi + _position, ppos);
				if (!--li) break;
			}
			lpdest++;
		}
	}

	wess_enable();
	return wess_handle_getposition(_handle);
}

int wess_handle_setposition(int handle, int position) // 80033EB8
{
	sequence_status *psq_stat;
	track_status *ptmp;
	char ppos[12];

	int li, lj;
	char *lpdest;

	int _handle, _position;

	_handle = handle;
	_position = position;

	if (!Is_Module_Loaded())
		return 0;

	wess_disable();

	psq_stat = gethandleseq(_handle);

	if (psq_stat)
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
				CmdFuncArr[ptmp->patchtype][TrkMute](ptmp);
				wess_track_setposition(ptmp, _position, ppos);
				if (!--li) break;
			}
			lpdest++;
		}
	}

	wess_enable();
	return wess_handle_getposition(_handle);
}

int wess_handle_getposition(int handle) // 80034010
{
	sequence_status *psq_stat;
	track_status *ptmp;

	int li, lj;
	char *lpdest;
	int position;

	int _handle;

	_handle = handle;

	position = 0;

	if (!Is_Module_Loaded())
		return 0;

	wess_disable();

	psq_stat = gethandleseq(_handle);

	if (psq_stat)
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
				if(position < ptmp->totppi)
					position = ptmp->totppi;

				if (!--li) break;
			}
			lpdest++;
		}
	}

	wess_enable();

	return position;
}

void patch_chg_action(track_status *ptmp, int patch_num) // 800340E0
{
	ptmp->ppos[0] = PatchChg;
	ptmp->ppos[1] = patch_num;
	ptmp->ppos[2] = patch_num >> 8;
	CmdFuncArr[ptmp->patchtype][PatchChg](ptmp);
}

void pitch_mod_action(track_status *ptmp, int pitch_cntr) // 80034144
{
	ptmp->ppos[0] = PitchMod;
	ptmp->ppos[1] = pitch_cntr;
	ptmp->ppos[2] = pitch_cntr >> 8;
	CmdFuncArr[ptmp->patchtype][PitchMod](ptmp);
}

void volume_mod_action(track_status *ptmp, int volume_cntr) // 800341A8
{
	ptmp->ppos[0] = VolumeMod;
	ptmp->ppos[1] = volume_cntr;
	CmdFuncArr[ptmp->patchtype][VolumeMod](ptmp);
}

void pan_mod_action(track_status *ptmp, int pan_cntr) // 80034200
{
	ptmp->ppos[0] = PanMod;
	ptmp->ppos[1] = pan_cntr;
	CmdFuncArr[ptmp->patchtype][PanMod](ptmp);
}

void wess_track_parm_mod(track_status *ptmp, int value, WessAction funcion) // 80034258
{
	char *ppos;

	//save
	ppos = ptmp->ppos;
	ptmp->ppos = scratch_area;
	funcion(ptmp, value);
	//restore
	ptmp->ppos = ppos;
}

void tracksetspecial(track_status *ptmp, TriggerPlayAttr *attr) // 8003429C
{
	int mask;

	ptmp->flags &= ~(TRK_TIMED | TRK_LOOPED);

	if (attr != NULL)
	{
		mask = attr->mask;
		if (mask)
		{
			if (mask & TRIGGER_VOLUME)
			{
				ptmp->volume_cntrl = attr->volume;
				wess_track_parm_mod(ptmp, attr->volume, volume_mod_action);

				mask &= ~TRIGGER_VOLUME;
				if (!(mask))
					return;
			}

			if (mask & TRIGGER_PAN)
			{
				ptmp->pan_cntrl = attr->pan;
				wess_track_parm_mod(ptmp, attr->pan, pan_mod_action);

				mask &= ~TRIGGER_PAN;
				if (!(mask))
					return;
			}

			if (mask & TRIGGER_PATCH)
			{
				ptmp->patchnum = attr->patch;
				wess_track_parm_mod(ptmp, attr->patch, patch_chg_action);

				mask &= ~TRIGGER_PATCH;
				if (!(mask))
					return;
			}

			if (mask & TRIGGER_PITCH)
			{
				ptmp->pitch_cntrl = attr->pitch;
				wess_track_parm_mod(ptmp, attr->pitch, pitch_mod_action);

				mask &= ~TRIGGER_PITCH;
				if (!(mask))
					return;
			}

			if (mask & TRIGGER_MUTEMODE)
			{
				if (ptmp->mutemask & (1 << attr->mutemode))
				{
					ptmp->flags |= TRK_MUTE;
					CmdFuncArr[ptmp->patchtype][TrkMute](ptmp);
				}
				ptmp->flags &= ~TRK_MUTE;

				mask &= ~TRIGGER_MUTEMODE;
				if (!(mask))
					return;
			}

			if (mask & TRIGGER_TEMPO)
			{
				ptmp->qpm = attr->tempo;
				ptmp->ppi = CalcPartsPerInt(GetIntsPerSec(), ptmp->ppq, ptmp->qpm);

				mask &= ~TRIGGER_TEMPO;
				if (!(mask))
					return;
			}

			if (mask & TRIGGER_TIMED)
			{
				ptmp->endppi = ptmp->totppi + attr->timeppq;
				ptmp->flags |= TRK_TIMED;

				mask &= ~TRIGGER_TIMED;
				if (!(mask))
					return;
			}

			if (mask & TRIGGER_LOOPED)
			{
				ptmp->flags |= TRK_LOOPED;
			}
		}
	}
}

void trackgetspecial(track_status *ptmp, TriggerPlayAttr *attr) // 80034508
{
	int mask;

	mask = attr->mask;

	if (mask)
	{
		if (mask & TRIGGER_VOLUME)
		{
			attr->volume = ptmp->volume_cntrl;
			mask &= ~TRIGGER_VOLUME;
			if (!(mask))
				return;
		}

		if (mask & TRIGGER_PAN)
		{
			attr->pan = ptmp->pan_cntrl;
			mask &= ~TRIGGER_PAN;
			if (!(mask))
				return;
		}

		if (mask & TRIGGER_PATCH)
		{
			attr->patch = ptmp->patchnum;
			mask &= ~TRIGGER_PATCH;
			if (!(mask))
				return;
		}

		if (mask & TRIGGER_PITCH)
		{
			attr->pitch = ptmp->pitch_cntrl;
			mask &= ~TRIGGER_PITCH;
			if (!(mask))
				return;
		}

		if (mask & TRIGGER_MUTEMODE)
		{
			attr->mutemode = ptmp->mutemask;
			mask &= ~TRIGGER_MUTEMODE;
			if (!(mask))
				return;
		}

		if (mask & TRIGGER_TEMPO)
		{
			attr->tempo = ptmp->qpm;
			mask &= ~TRIGGER_TEMPO;
			if (!(mask))
				return;
		}

		if (mask & TRIGGER_TIMED)
		{
			if (!(ptmp->flags & TRK_TIMED))
				attr->mask &= ~TRIGGER_TIMED;
			else
				attr->timeppq = ptmp->endppi - ptmp->totppi;

			mask &= ~TRIGGER_TIMED;
			if (!(mask))
				return;
		}

		if (mask & TRIGGER_LOOPED)
		{
			if (!(ptmp->flags & TRK_LOOPED))
				attr->mask &= ~TRIGGER_LOOPED;
		}
	}
}
#endif // 0
