
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
#ifndef NOUSEWESSCODE
extern pmasterstat *pm_stat;    //0x800B41CC

static int max_edit_labels = 2;		//0x8005D900
static int soundfx_edit_tracks = 8;	//0x8005D904
static int music_edit_tracks = 15;	//0x8005D908
static int drums_edit_tracks = 1;	//0x8005D90C
static int edt_mem_is_mine = 0;//0x8005D910
static char *edt_mem = 0;//0x8005D914
static int  edt_size = 0;//0x8005D918

void wess_handle_free_edit_space(void) // 80034D20
{
	if (edt_mem_is_mine)
	{
		if (edt_mem)
		{
			wess_free(edt_mem);
			edt_mem = 0;
		}
		edt_mem_is_mine = 0;
	}
}

char *wess_get_edt_start(void) // 80034D70
{
	return(edt_mem);
}

int wess_get_edt_size(void) // 80034D80
{
	return(edt_size);
}

void wess_handle_set_edit_tracks(int labels, int soundfx, int music, int drums) // 80034D90
{
	max_edit_labels = labels;
	soundfx_edit_tracks = soundfx;
	music_edit_tracks = music;
	drums_edit_tracks = drums;
}

int wess_handle_create_edit_space(char *memory_pointer, int data_size, int memory_allowance) // 80034DB4
{
	int i, j, k, soundfx_cnt, music_cnt, drums_cnt;
	sequence_status *psq_stat;
	track_status *ptk_stat;
	char *pdest;
	char *pmem;
	int ptrk_indxs_pos;
	int set_psq_stat;

	char *_memory_pointer;
	int _data_size;
	int _memory_allowance;

	_memory_pointer = memory_pointer;
	_data_size = data_size;
	_memory_allowance = memory_allowance;

	if (!Is_Module_Loaded())
		return(0);

	soundfx_cnt = 0;
	music_cnt = 0;
	drums_cnt = 0;

	set_psq_stat = 0;

	if (_memory_pointer == NULL)
	{
		edt_mem_is_mine = 1;
		edt_mem = wess_malloc((char *)_memory_allowance);
		if (edt_mem == NULL)
		{
			return(0);
		}
	}
	else
	{
		edt_mem_is_mine = 0;
		edt_mem = _memory_pointer;
	}

	edt_size = _memory_allowance;

	wess_disable();

	for (i = 0; i < wess_driver_tracks; i++)
	{
		if (!((pm_stat->pseqstattbl + i)->flags & SEQ_ACTIVE))
			break;
	}

	if (wess_driver_tracks == i)
	{
		wess_enable();
		return 0;
	}

	psq_stat = (pm_stat->pseqstattbl + i);
	pdest = psq_stat->ptrk_indxs;
	for (j = 0; j < wess_driver_tracks; j++)
	{
		ptk_stat = (pm_stat->ptrkstattbl + i);

		if (!(ptk_stat->flags & TRK_ACTIVE))
		{
			if (soundfx_cnt < soundfx_edit_tracks)
			{
				ptk_stat->sndclass = SNDFX_CLASS;
				ptk_stat->patchtype = 1;
				ptk_stat->data_space = _data_size - (max_edit_labels << 2);
				ptrk_indxs_pos = soundfx_cnt;
				soundfx_cnt++;
			}
			else if (music_cnt < music_edit_tracks)
			{
				ptk_stat->sndclass = MUSIC_CLASS;
				ptk_stat->patchtype = 1;
				ptk_stat->data_space = _data_size - (max_edit_labels << 2);
				ptrk_indxs_pos = (music_cnt + soundfx_edit_tracks);
				music_cnt++;
			}
			else if (drums_cnt < drums_edit_tracks)
			{
				ptk_stat->sndclass = DRUMS_CLASS;
				ptk_stat->patchtype = 1;
				ptk_stat->data_space = (_memory_allowance - (soundfx_edit_tracks + music_edit_tracks) * _data_size) - (max_edit_labels << 2);
				ptrk_indxs_pos = (drums_cnt + soundfx_edit_tracks + music_edit_tracks);
				drums_cnt++;
			}
			else
			{
				break;
			}

			//----------------------
			set_psq_stat = 1;

			*(pdest + ptrk_indxs_pos) = j; /* update ptrk_indxs for the sequence */

			ptk_stat->seq_owner = i;
			ptk_stat->patchnum = 0;
			ptk_stat->volume_cntrl = 0x7f;
			ptk_stat->pitch_cntrl = 0;
			ptk_stat->pan_cntrl = 0x40;
			ptk_stat->voices_active = 0;
			ptk_stat->ppq = 120;
			ptk_stat->qpm = 120;
			ptk_stat->ppi = CalcPartsPerInt(GetIntsPerSec(), ptk_stat->ppq, ptk_stat->qpm);
			ptk_stat->starppi = 0;
			ptk_stat->totppi = 0;
			ptk_stat->labellist_count = 0;
			ptk_stat->data_size = 2;
			ptk_stat->psp = (unsigned char*)ptk_stat->psubstack;
			ptk_stat->labellist_max = max_edit_labels;

			//----------------------
			pmem = (edt_mem + (ptrk_indxs_pos * _data_size));
			ptk_stat->plabellist = (unsigned long *)pmem;

			for (k = 0; k < ptk_stat->labellist_max; k++)
			{
				*pmem++ = 0;
			}

			//----------------------
			pmem = (edt_mem + (max_edit_labels << 2) + (ptrk_indxs_pos * _data_size));
			ptk_stat->pstart = (char *)pmem;
			pmem[0] = 0;
			pmem[1] = TrkEnd;

			ptk_stat->flags = (ptk_stat->flags|TRK_ACTIVE|TRK_STOPPED|TRK_HANDLED|TRK_OFF) & ~(TRK_MUTE|TRK_TIMED|TRK_LOOPED|TRK_SKIP);
			ptk_stat->ppos = Read_Vlq(ptk_stat->pstart, &ptk_stat->deltatime);
			ptk_stat->mutemask = 0;

			psq_stat->tracks_active++;
			pm_stat->trks_active++;
		}
	}

	if (set_psq_stat)
	{
		psq_stat->flags |= (SEQ_HANDLE|SEQ_ACTIVE);
		psq_stat->seq_num = -1;
		psq_stat->seq_type = 0;
		psq_stat->playmode = 0;
		psq_stat->volume = 0x7f;
		psq_stat->pan = 0x40;
		pm_stat->seqs_active++;
	}

	wess_enable();

	if (set_psq_stat)
	{
		return (i + 1);
	}

	return 0;
}
#endif
