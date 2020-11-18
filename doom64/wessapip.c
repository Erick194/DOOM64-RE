
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

void trackstart(track_status *ptmp, sequence_status *psq_stat) // 80031650
{
	if (ptmp->flags & TRK_STOPPED)
	{
		ptmp->flags &= ~(TRK_MUTE|TRK_STOPPED);
		if (++psq_stat->tracks_playing)
		{
			psq_stat->playmode = SEQ_STATE_PLAYING;
		}
	}
}

void trackstop(track_status *ptmp, sequence_status *psq_stat) // 800316A0
{
	if (!(ptmp->flags & TRK_STOPPED))
	{
		ptmp->flags |= (TRK_MUTE|TRK_STOPPED);
		if (!--psq_stat->tracks_playing)
		{
			psq_stat->playmode = SEQ_STATE_STOPPED;
		}
	}
}

void wess_seq_pause(int sequence_number, enum MuteFlag mflag) // 800316F0
{
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;

	int _sequence_number;
	enum MuteFlag _mflag;

	_sequence_number = sequence_number;
	_mflag = mflag;

	if (!Is_Seq_Num_Valid(_sequence_number))
	{
		return;
	}

	/* immediate pause of sequence */
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
				if (psq_stat->seq_num == _sequence_number)
				{
					psq_stat->flags |= SEQ_PAUSE;
				}
				if (!--na) break;
			}
			psq_stat++;
		}
	}

	queue_the_function(QUEUE_SEQ_PAUSE);
	queue_the_data(&_sequence_number, sizeof(int));
	queue_the_data(&_mflag, sizeof(int));

	wess_enable();
}

void queue_wess_seq_pause(int sequence_number, enum MuteFlag mflag);

void run_queue_wess_seq_pause(void) // 800317B8
{
	enum MuteFlag mflag;
	int sequence_number;

	unqueue_the_data(&sequence_number, sizeof(int));
	unqueue_the_data(&mflag, sizeof(int));
	queue_wess_seq_pause(sequence_number, mflag);
}

void queue_wess_seq_pause(int sequence_number, enum MuteFlag mflag) // 800317F8
{
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;

	int _sequence_number;
	enum MuteFlag _mflag;

	_sequence_number = sequence_number;
	_mflag = mflag;

	if (!Is_Seq_Num_Valid(_sequence_number))
	{
		return;
	}

	/* immediate pause of sequence */
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
				if (psq_stat->seq_num == _sequence_number)
				{
					if (psq_stat->flags & SEQ_PAUSE)
					{
						psq_stat->flags &= ~SEQ_PAUSE;

						li = psq_stat->tracks_active;
						lj = pm_stat->max_trks_perseq;
						/* *lpdest refers to an active track if not 0xFF */
						lpdest = psq_stat->ptrk_indxs;
						while (lj--)
						{
							if (*lpdest != 0xFF)
							{
								ptmp = (pm_stat->ptrkstattbl + (*lpdest));
								trackstop(ptmp, psq_stat);
								if (_mflag == YesMute)
								{
									CmdFuncArr[ptmp->patchtype][TrkMute](ptmp);
								}
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

	wess_enable();
}

void wess_seq_restart(int sequence_number) // 800319C4
{
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;

	int _sequence_number;

	_sequence_number = sequence_number;

	if (!Is_Seq_Num_Valid(_sequence_number))
	{
		return;
	}

	/* immediate restart of sequence */
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
				if (psq_stat->seq_num == _sequence_number)
				{
					psq_stat->flags |= SEQ_RESTART;
				}
				if (!--na) break;
			}
			psq_stat++;
		}
	}

	queue_the_function(QUEUE_SEQ_RESTART);
	queue_the_data(&_sequence_number, sizeof(int));

	wess_enable();
}

void queue_wess_seq_restart(int sequence_number);

void run_queue_wess_seq_restart(void) // 80031A7C
{
	int sequence_number;

	unqueue_the_data(&sequence_number, sizeof(int));
	queue_wess_seq_restart(sequence_number);
}

void queue_wess_seq_restart(int sequence_number) // 80031AAC
{
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;

	int _sequence_number;

	_sequence_number = sequence_number;

	if (!Is_Seq_Num_Valid(_sequence_number))
	{
		return;
	}

	/* immediate restart of sequence */
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
				if (psq_stat->seq_num == _sequence_number)
				{
					if (psq_stat->flags & SEQ_RESTART)
					{
						psq_stat->flags &= ~SEQ_RESTART;
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

	wess_enable();
}

void wess_seq_pauseall(enum MuteFlag mflag, int remember) // 80031C14
{
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;

	enum MuteFlag _mflag;
	int _remember;

	_mflag = mflag;
	_remember = remember;

	if (!Is_Module_Loaded())
	{
		return;
	}

	/* immediate pause of all sequences */
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
				psq_stat->flags |= SEQ_PAUSE;
				if (!--na) break;
			}
			psq_stat++;
		}
	}

	queue_the_function(QUEUE_SEQ_PAUSEALL);
	queue_the_data(&_mflag, sizeof(int));
	queue_the_data(&_remember, sizeof(int));

	wess_enable();
}

void queue_wess_seq_pauseall(enum MuteFlag mflag, int remember);

void run_queue_wess_seq_pauseall(void) // 80031CCC
{
	enum MuteFlag mflag;
	int remember;

	unqueue_the_data(&mflag, sizeof(int));
	unqueue_the_data(&remember, sizeof(int));
	queue_wess_seq_pauseall(mflag, remember);
}

void queue_wess_seq_pauseall(enum MuteFlag mflag, int remember) // 80031D0C
{
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;

	enum MuteFlag _mflag;
	int _remember;

	_mflag = mflag;
	_remember = remember;

	if (!Is_Module_Loaded())
	{
		return;
	}

	/* immediate pause of all sequences */
	wess_disable();

	if (_mflag == YesMute)
	{
		start_record_music_mute(_remember);
	}

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
				if (psq_stat->flags & SEQ_PAUSE)
				{
					psq_stat->flags &= ~SEQ_PAUSE;

					li = psq_stat->tracks_active;
					lj = pm_stat->max_trks_perseq;
					/* *lpdest refers to an active track if not 0xFF */
					lpdest = psq_stat->ptrk_indxs;
					while (lj--)
					{
						if (*lpdest != 0xFF)
						{
							ptmp = (pm_stat->ptrkstattbl + (*lpdest));
							trackstop(ptmp, psq_stat);
							if (_mflag == YesMute)
							{
								CmdFuncArr[ptmp->patchtype][TrkMute](ptmp);
							}
							if (!--li) break;
						}
						lpdest++;
					}

					if (!--na) break;
				}
			}
			psq_stat++;
		}
	}

	if (_mflag == YesMute)
	{
		end_record_music_mute();
	}

	wess_enable();
}

void wess_seq_restartall(enum VoiceRestartFlag restart_remembered_voices) // 80031EF4
{
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj, ncnt, nc;

	enum VoiceRestartFlag _restart_remembered_voices;

	_restart_remembered_voices = restart_remembered_voices;

	if (!Is_Module_Loaded())
	{
		return;
	}

	/* immediate restart of all sequences */
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
				psq_stat->flags |= SEQ_RESTART;
				if (!--na) break;
			}
			psq_stat++;
		}
	}

	queue_the_function(QUEUE_SEQ_RESTARTALL);
	queue_the_data(&_restart_remembered_voices, sizeof(int));

	wess_enable();
}

void queue_wess_seq_restartall(enum VoiceRestartFlag restart_remembered_voices);

void run_queue_wess_seq_restartall(void) // 80031F9C
{
	enum VoiceRestartFlag restart_remembered_voices;

	unqueue_the_data(&restart_remembered_voices, sizeof(int));
	queue_wess_seq_restartall(restart_remembered_voices);
}

extern void do_record_music_unmute(int seq_num, int track, track_status *ptk_stat);

void queue_wess_seq_restartall(enum VoiceRestartFlag restart_remembered_voices) // 80031FCC
{
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj, ncnt, nc;

	enum VoiceRestartFlag _restart_remembered_voices;

	_restart_remembered_voices = restart_remembered_voices;

	if (!Is_Module_Loaded())
	{
		return;
	}

	/* immediate restart of all sequences */
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
				if (psq_stat->flags & SEQ_RESTART)
				{
					psq_stat->flags &= ~SEQ_RESTART;

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

							if (_restart_remembered_voices)
							{
								do_record_music_unmute(psq_stat->seq_num, *lpdest, ptmp);
							}

							if (!--li) break;
						}
						lpdest++;
					}

					if (!--na) break;
				}
			}
			psq_stat++;
		}
	}

	wess_enable();
}
#endif
