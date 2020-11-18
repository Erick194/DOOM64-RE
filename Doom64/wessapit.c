
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

/* used by wess trigger functions */
enum HandleFlag { NoHandle, YesHandle };

/* used by wess_seq_stop and wess_seq_stop_and_voiceramp functions */
enum MuteRelease { NoMuteRelease, YesMuteRelease};

void wess_seq_trigger_type(int seq_num, unsigned long seq_type) // 80032160
{
	sequence_data *psq_info;

	psq_info = pm_stat->pmod_info->pseq_info + seq_num; /* pointer math */

	wess_seq_structrig(psq_info, seq_num, seq_type, NoHandle, NULL);
}

void wess_seq_trigger_type_special(int seq_num, unsigned long seq_type, TriggerPlayAttr *attr) // 800321A8
{
	sequence_data *psq_info;

	psq_info = pm_stat->pmod_info->pseq_info + seq_num; /* pointer math */

	wess_seq_structrig(psq_info, seq_num, seq_type, NoHandle, attr);
}

void updatetrackstat(track_status *ptk_stat, TriggerPlayAttr *attr) // 800321FC
{
	int tempmask;
	char *temp;
	char tmpppos[4];

	if ((attr == NULL) || (!attr->mask))
		return;

	tempmask = attr->mask;

	if (tempmask & TRIGGER_VOLUME) //0x01
	{
		temp = ptk_stat->ppos; //copy

		// set tmp buffer ppos
		ptk_stat->ppos = tmpppos;
		ptk_stat->ppos[0] = VolumeMod;
		ptk_stat->ppos[1] = attr->volume;

		CmdFuncArr[ptk_stat->patchtype][VolumeMod](ptk_stat);
		ptk_stat->ppos = temp; //restore
	}

	if (tempmask & TRIGGER_PAN)
	{
		temp = ptk_stat->ppos; //copy

		// set tmp buffer ppos
		ptk_stat->ppos = tmpppos;
		ptk_stat->ppos[0] = PanMod;
		ptk_stat->ppos[1] = attr->pan;

		CmdFuncArr[ptk_stat->patchtype][PanMod](ptk_stat);
		ptk_stat->ppos = temp; //restore
	}

	if (tempmask & TRIGGER_PATCH) //0x04
	{
		ptk_stat->patchnum = attr->patch;
	}

	if (tempmask & TRIGGER_PITCH) //0x08
	{
		temp = ptk_stat->ppos; //copy

		// set tmp buffer ppos
		ptk_stat->ppos = tmpppos;
		ptk_stat->ppos[0] = PitchMod;
		ptk_stat->ppos[1] = (char)(attr->pitch & 0xff);
		ptk_stat->ppos[2] = (char)(attr->pitch >> 8);

		CmdFuncArr[ptk_stat->patchtype][PitchMod](ptk_stat);
		ptk_stat->ppos = temp; //restore
	}

	if (tempmask & TRIGGER_MUTEMODE) //0x10
	{
		if (ptk_stat->mutemask & (1 << attr->mutemode))
		{
			ptk_stat->flags |= TRK_MUTE;
		}
		else {
			ptk_stat->flags &= ~TRK_MUTE;
		}
	}

	if (tempmask & TRIGGER_TEMPO) //0x20
	{
		ptk_stat->qpm = attr->tempo;
		ptk_stat->ppi = CalcPartsPerInt(GetIntsPerSec(), ptk_stat->ppq, ptk_stat->qpm);
	}

	if (tempmask & TRIGGER_TIMED) //0x40
	{
		ptk_stat->endppi = ptk_stat->totppi + attr->timeppq;
		ptk_stat->flags |= TRK_TIMED;
	}

	if (tempmask&TRIGGER_LOOPED) //0x80
	{
		ptk_stat->flags |= TRK_LOOPED;
	}
}

void wess_seq_update_type_special(unsigned long seq_type, TriggerPlayAttr *attr) // 80032460
{
    unsigned long _seq_type;
    TriggerPlayAttr *_attr;

    _seq_type = seq_type;
    _attr = attr;

	wess_disable();

	queue_the_function(QUEUE_SEQ_UPDATE_TYPE_SPECIAL);
	queue_the_data(&_seq_type, sizeof(unsigned long));
	queue_the_data(&_attr, sizeof(TriggerPlayAttr));

	wess_enable();
}

void queue_wess_seq_update_type_special(unsigned long seq_type, TriggerPlayAttr *attr);

void run_queue_wess_seq_update_type_special(void) // 800324AC
{
	unsigned long seq_type;
	TriggerPlayAttr attr;

	unqueue_the_data(&seq_type, sizeof(unsigned long));
	unqueue_the_data(&attr, sizeof(TriggerPlayAttr));
	queue_wess_seq_update_type_special(seq_type, &attr);
}

void queue_wess_seq_update_type_special(unsigned long seq_type, TriggerPlayAttr *attr) // 800324E8
{
	/* immediate stop of sequence */
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;

	unsigned long _seq_type;
	TriggerPlayAttr *_attr;

	_seq_type = seq_type;
	_attr = attr;

	if (!Is_Module_Loaded())
	{
		return;
	}

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
				if (psq_stat->seq_type == _seq_type)
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
							if (_attr != 0)
							{
								updatetrackstat(ptmp, _attr);
							}
							if (!--li) break;
						}
						lpdest++;
					}
				}
				if (!--na) break;
			}
			psq_stat++;
		}
	}

	wess_enable();
}

int wess_seq_type_status(unsigned long sequence_type) // 8003266C
{
	char nt, na;
	sequence_status *psq_stat;
	int status;

	unsigned long _sequence_type;

	_sequence_type = sequence_type;

	if (!Is_Module_Loaded())
	{
		return(SEQUENCE_INACTIVE);
	}

	wess_disable();

	status = SEQUENCE_INACTIVE;

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
				if (psq_stat->seq_type == _sequence_type)
				{
					if (psq_stat->playmode == SEQ_STATE_STOPPED)
					{
						status = SEQUENCE_STOPPED;
					}
					else if (psq_stat->playmode == SEQ_STATE_PLAYING) {
						status = SEQUENCE_PLAYING;
					}
				}

				if (!--na) break;
			}
			psq_stat++;
		}
	}

	wess_enable();

	return(status);
}

void __wess_seq_stoptype(unsigned long sequence_type, enum MuteRelease mrelease, int millisec) // 80032758
{
	char nt, na;
	sequence_status *psq_stat;

	unsigned long _sequence_type;
	enum MuteRelease _mrelease;
	int _millisec;

	_sequence_type = sequence_type;
	_mrelease = mrelease;
	_millisec = millisec;

	if (!Is_Module_Loaded())
	{
		return;
	}


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
				if (psq_stat->seq_type == _sequence_type)
				{
					psq_stat->flags = (psq_stat->flags | SEQ_STOP) & ~(SEQ_PAUSE|SEQ_RESTART);
				}

				if (!--na) break;
			}
			psq_stat++;
		}
	}

	if (_mrelease)
	{
		queue_the_function(QUEUE_SEQ_STOPTYPE_AND_VOICERAMP);
		queue_the_data(&_millisec, sizeof(int));
	}
	else
	{
		queue_the_function(QUEUE_SEQ_STOPTYPE);
	}
	queue_the_data(&_sequence_type, sizeof(int));
	wess_enable();
}

void wess_seq_stoptype(unsigned long sequence_type) // 80032868
{
	__wess_seq_stoptype(sequence_type, NoMuteRelease, 0);
}

void wess_seq_stoptype_and_voiceramp(unsigned long sequence_type, int millisec) // 8003288C
{
	__wess_seq_stoptype(sequence_type, YesMuteRelease, millisec);
}

void queue_wess_seq_stoptype(unsigned long sequence_type, enum MuteRelease mrelease, int millisec);

void run_queue_wess_seq_stoptype(void) // 800328B0
{
	unsigned long sequence_type;

	unqueue_the_data(&sequence_type, sizeof(unsigned long));
	queue_wess_seq_stoptype(sequence_type, NoMuteRelease, 0);
}

void run_queue_wess_seq_stoptype_and_voiceramp(void) // 800328E4
{
	int millisec;
	unsigned long sequence_type;

	unqueue_the_data(&millisec, sizeof(int));
	unqueue_the_data(&sequence_type, sizeof(unsigned long));
	queue_wess_seq_stoptype(sequence_type, YesMuteRelease, millisec);
}

void queue_wess_seq_stoptype(unsigned long sequence_type, enum MuteRelease mrelease, int millisec) // 80032924
{
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;
	int get_millisec;

	unsigned long _sequence_type;
	enum MuteRelease _mrelease;
	int _millisec;

	_sequence_type = sequence_type;
	_mrelease = mrelease;
	_millisec = millisec;

	if (!Is_Module_Loaded())
	{
		return;
	}

	wess_disable();


	/* search for all sequences with this number and turn them off */
	nt = wess_driver_sequences;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;
	if (na)
	{
		if (_mrelease)
		{
			get_millisec = wess_get_mute_release();
			wess_set_mute_release(_millisec);
		}

		while (nt--)
		{
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				if (psq_stat->seq_type == _sequence_type)
				{
					if (psq_stat->flags & SEQ_STOP)
					{
						psq_stat->flags &= ~SEQ_STOP;

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
				}

				if (!--na) break;
			}
			psq_stat++;
		}
	}

	if (_mrelease)
	{
		wess_set_mute_release(get_millisec);
	}

	wess_enable();
}

int wess_get_seq_num_active(int seq_num) // 80032B24
{
	char nt, na;
	sequence_status *psq_stat;
	int seq_count;

	int _seq_num;

	_seq_num = seq_num;

	if (!Is_Seq_Num_Valid(_seq_num))
	{
		return(0);
	}

	seq_count = 0;

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
				if (psq_stat->seq_num == _seq_num)
				{
					seq_count++;
				}

				if (!--na) break;
			}
			psq_stat++;
		}
	}

	wess_enable();

	return (seq_count);
}

int wess_copy_all_seq_num_active(int *seq_num_list) // 80032BE8
{
	char nt, na;
	sequence_status *psq_stat;
	int i, seq_count, sikip_set_seq;
	int *list_temp;

	int *_seq_num_list;

	_seq_num_list = seq_num_list;

	if (!Is_Module_Loaded())
	{
		return(0);
	}

	seq_count = 0;

	wess_disable();

	/* search for all sequences with this number and turn them off */
	nt = wess_driver_sequences;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;
	if (na)
	{
		while (nt--)
		{
			sikip_set_seq = 0;
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				i = 0;
				if(seq_count > 0)
				{
					list_temp = _seq_num_list;
					do
					{
						if (psq_stat->seq_num == *list_temp)
						{
							sikip_set_seq = 1;
							break;
						}
						list_temp++;
						i++;
					}while (i != seq_count);
				}

				if (!sikip_set_seq)
				{
					list_temp = (_seq_num_list + seq_count);
					*list_temp = psq_stat->seq_num;
					seq_count++;
				}

				if (!--na) break;
			}
			psq_stat++;
		}
	}

	wess_enable();

	return (seq_count);
}

int wess_get_seq_type_active(int seq_type) // 80032CD4
{
	char nt, na;
	sequence_status *psq_stat;
	int seq_count;

	int _seq_type;

	_seq_type = seq_type;

	if (!Is_Module_Loaded())
	{
		return(0);
	}

	seq_count = 0;

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
				if (psq_stat->seq_type == _seq_type)
				{
					seq_count++;
				}

				if (!--na) break;
			}
			psq_stat++;
		}
	}

	wess_enable();

	return (seq_count);
}

int wess_copy_all_seq_type_active(int *seq_type_list) // 80032D8C
{
	char nt, na;
	sequence_status *psq_stat;
	int i, seq_count, sikip_set_seq;
	int *list_temp;

	int *_seq_type_list;

	_seq_type_list = seq_type_list;

	if (!Is_Module_Loaded())
	{
		return(0);
	}

	wess_disable();

	seq_count = 0;

	/* search for all sequences with this number and turn them off */
	nt = wess_driver_sequences;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;
	if (na)
	{
		while (nt--)
		{
			sikip_set_seq = 0;
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				i = 0;
				if (seq_count > 0)
				{
					list_temp = _seq_type_list;
					do
					{
						if (psq_stat->seq_type == *list_temp)
						{
							sikip_set_seq = 1;
							break;
						}
						list_temp++;
						i++;
					} while (i != seq_count);
				}

				if (!sikip_set_seq)
				{
					list_temp = (_seq_type_list + seq_count);
					*list_temp = psq_stat->seq_type;
					seq_count++;
				}

				if (!--na) break;
			}
			psq_stat++;
		}
	}

	wess_enable();

	return (seq_count);
}
#endif
