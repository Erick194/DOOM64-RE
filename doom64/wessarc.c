
#include "wessapi.h"
#include "wessarc.h"
#include "wessint_s.h"
#ifndef NOUSEWESSCODE
extern void(*DrvFunctions[36])(track_status *);
extern void(*drv_cmds[19])(track_status *);

void (**CmdFuncArr[10])(track_status *) = {
    DrvFunctions,
    drv_cmds,
    DrvFunctions,
    DrvFunctions,
    DrvFunctions,
    DrvFunctions,
    DrvFunctions,
    DrvFunctions,
    DrvFunctions,
    DrvFunctions
    };

int wess_driver_num_dma_buffers = 24;       // 8005D948
int wess_driver_num_dma_messages = 32;      // 8005D94C
int wess_driver_dma_buffer_length = 0x800;   // 8005D950
int wess_driver_extra_samples = 80;         // 8005D954
int wess_driver_frame_lag = 1;              // 8005D958
int wess_driver_voices = 24;                // 8005D95C
int wess_driver_updates = 48;               // 8005D960
int wess_driver_sequences = 26;             // 8005D964
int wess_driver_tracks = 25;                // 8005D968
int wess_driver_gates = 0;                  // 8005D96C
int wess_driver_iters = 0;                  // 8005D970
int wess_driver_callbacks = 0;              // 8005D974
int wess_driver_max_trks_per_seq = 16;      // 8005D978                                                                                        load_sequence_data:80039868(R),
int wess_driver_max_subs_per_trk = 0;       // 8005D97C


int WessTimerActive = 0;        // 8005D984
int T2counter = 0;              // 8005D988
unsigned long millicount = 0;   // 8005D98C
int SeqOn = 0;                  // 8005D990
unsigned long accmpi = 0;       // 8005D994
int disabledeep = 0;            // 8005D998

WessErrorCallbackProc wesserr;  // 800B4210
WessDecompCallbackProc wessdecomp; // 800B4214
long imask; // 800B4218

void wess_set_error_callback(WessErrorCallbackProc errcall) // 80035290
{
	wesserr = errcall;
}

void wess_error_callback(char *errstring, int errnum1, int errnum2) // 8003529C
{
	if (wesserr)
	{
		wesserr(errstring, errnum1, errnum2);
	}
}

void wess_set_decomp_callback(WessDecompCallbackProc decompcall) // 800352C8
{
	wessdecomp = decompcall;
}

int wess_decomp(unsigned char decomp_type,
	char          *fileref,
	unsigned long file_offset,
	char          *ramdest,
	unsigned long uncompressed_size) // 800352D4
{
	if (wessdecomp != 0)
	{
		return wessdecomp(decomp_type, fileref, file_offset, ramdest, uncompressed_size);
	}

	return(-1);
}

void wess_low_level_init(void) // 8003531C
{
	//Nothing
}

void wess_low_level_exit(void) // 80035324
{
	//Nothing
}

char *wess_malloc(char *mem) // 8003532C
{
	return NULL;
}

void wess_free(char *mem) // 80035338
{
	//Nothing
}

void wess_engine_off(void) // 80035340
{
	SeqOn = 0;
}

void wess_engine_on(void) // 8003534C
{
	SeqOn = 1;
}

void wess_disable(void) // 8003535C
{
	if (disabledeep == 0)
	{
		imask = wesssys_disable_ints();
	}
	disabledeep += 1;
}

void wess_enable(void) // 8003539C
{
	disabledeep += -1;
	if (disabledeep == 0)
	{
		wesssys_restore_ints(imask);
	}
}

short GetIntsPerSec(void) // 800353DC
{
	return(120);
}


unsigned long CalcPartsPerInt(short ips, short ppq, short qpm) // 800353E4
{
	register unsigned long arg0;
	register unsigned long ppi;

    __ll_mul((s32) qpm >> 31, qpm, 0, (1<<16));
    asm("move	%0,$2":"=r"(arg0) : );
	asm("move	%0,$3":"=r"(ppi) : );
    __ll_mul(arg0, (u32) ppi, (s32) ppq >> 31, ppq);
    asm("move	%0,$2":"=r"(arg0) : );
	asm("move	%0,$3":"=r"(ppi) : );
    __ull_div(arg0, (u32) ppi, 0, 60);
    asm("move	%0,$2":"=r"(arg0) : );
	asm("move	%0,$3":"=r"(ppi) : );
	__ull_div(arg0, (u32) ppi, (s32) ips >> 31, ips);
	asm("move	%0,$2":"=r"(arg0) : );
	asm("move	%0,$3":"=r"(ppi) : );
    return (u32) ppi;
}

long WessInterruptHandler(void) // 80035458
{
	accmpi += 0x85555;
	millicount += (accmpi >> 16);
	accmpi &= 65535;

	T2counter++;
	if (SeqOn)
	{
		process_function_queue();
		SeqEngine();
	}
	return(0);
}

void init_WessTimer(void) // 800354DC
{
	SeqOn = 0;
	WessTimerActive = 1;
}

void exit_WessTimer(void) // 800354F4
{
	WessTimerActive = 0;
}

int Wess_init_for_LoadFileData(char *filename) // 80035500
{
	return(1);
}

char alignbuf[28]; // 800B41E4 unused
Wess_File_IO_Struct module_fileref; // 800B4200
Wess_Data_IO_Struct data_fileref; // 800B4208

Wess_File_IO_Struct *module_open(char *filename) // 8003550C
{
	module_fileref.start = filename;
	module_fileref.src = filename;
	return &module_fileref;
}

int module_read(void *destptr, int readbytes, Wess_File_IO_Struct *fileptr) // 80035520
{
	wess_rom_copy(fileptr->src, (char *)destptr, readbytes);
	fileptr->src += readbytes;
	return readbytes;
}

int module_seek(Wess_File_IO_Struct *fileptr, int seekpos, int seekmode) // 80035574
{
	if (seekmode == SEEK_SET)
	{
		fileptr->src = fileptr->start + seekpos;
	}
	else if (seekmode == SEEK_CUR)
	{
		fileptr->src += seekpos;
	}
	return 0;
}

unsigned long module_tell(Wess_File_IO_Struct *fileptr) // 800355B0
{
	return (unsigned long)fileptr->src;
}

void module_close(Wess_File_IO_Struct *fileptr) // 800355BC
{
	return;
}

int get_num_Wess_Sound_Drivers(int **settings_tag_lists) // 800355C4
{
	return(1);
}

Wess_File_IO_Struct *data_open(char *filename) // 800355D0
{
	return &data_fileref;
}

int data_read(Wess_Data_IO_Struct *fileptr, void *destptr, int readbytes, int filepos) //800355E0
{
	return 0;
}

void data_close(Wess_Data_IO_Struct *fileptr) // 800355F8
{
	return;
}

static unsigned char sequence_table[128]; // 800B4220

char *get_sequence_table(void) // 80035600
{
	master_status_structure *pm_stat_ptr;
	int sequences;
	int i;

	pm_stat_ptr = (master_status_structure *)wess_get_master_status();

	sequences = wess_driver_sequences;
	if (sequences > 127)
		sequences = 127;

	if (pm_stat_ptr)
	{
		i = 0;
		if (sequences > 0)
		{
			if (sequences & 3)
			{
				do
				{
					if ((pm_stat_ptr->pseqstattbl + i)->flags & SEQ_ACTIVE)
						sequence_table[i] = '1';
					else
						sequence_table[i] = '0';

					i++;
				} while (sequences & 3 != i);
			}
		}

		if (i != sequences)
		{
			do
			{
				if ((pm_stat_ptr->pseqstattbl + i)->flags & SEQ_ACTIVE)
					sequence_table[i] = '1';
				else
					sequence_table[i] = '0';

				if ((pm_stat_ptr->pseqstattbl + (i + 1))->flags & SEQ_ACTIVE)
					sequence_table[i + 1] = '1';
				else
					sequence_table[i + 1] = '0';

				if ((pm_stat_ptr->pseqstattbl + (i + 2))->flags & SEQ_ACTIVE)
					sequence_table[i + 2] = '1';
				else
					sequence_table[i + 2] = '0';

				if ((pm_stat_ptr->pseqstattbl + (i + 3))->flags & SEQ_ACTIVE)
					sequence_table[i + 3] = '1';
				else
					sequence_table[i + 3] = '0';

				i += 4;
			} while (sequences != i);
		}

		sequence_table[sequences] = '\0';
	}

	return (char *)&sequence_table;
}

static unsigned char tracks_table[128]; // 800B42A0

char *get_tracks_table(void) // 800357A0
{
	master_status_structure *pm_stat_ptr;
	int tracks;
	int i;

	pm_stat_ptr = (master_status_structure *)wess_get_master_status();

	tracks = wess_driver_tracks;
	if (tracks > 127)
		tracks = 127;

	if (pm_stat_ptr)
	{
		i = 0;
		if (tracks > 0)
		{
			if (tracks & 3)
			{
				do
				{
					if ((pm_stat_ptr->ptrkstattbl + i)->flags & TRK_ACTIVE)
						tracks_table[i] = '1';
					else
						tracks_table[i] = '0';

					i++;
				} while (tracks & 3 != i);
			}
		}

		if (i != tracks)
		{
			do
			{
				if ((pm_stat_ptr->ptrkstattbl + i)->flags & TRK_ACTIVE)
					tracks_table[i] = '1';
				else
					tracks_table[i] = '0';

				if ((pm_stat_ptr->ptrkstattbl + (i + 1))->flags & TRK_ACTIVE)
					tracks_table[i + 1] = '1';
				else
					tracks_table[i + 1] = '0';

				if ((pm_stat_ptr->ptrkstattbl + (i + 2))->flags & TRK_ACTIVE)
					tracks_table[i + 2] = '1';
				else
					tracks_table[i + 2] = '0';

				if ((pm_stat_ptr->ptrkstattbl + (i + 3))->flags & TRK_ACTIVE)
					tracks_table[i + 3] = '1';
				else
					tracks_table[i + 3] = '0';

				i += 4;
			} while (tracks != i);
		}

		tracks_table[tracks] = '\0';
	}

	return (char *)&tracks_table;
}

int get_track_volume_cntrl(int track) // 8003593C
{
	master_status_structure *pm_stat_ptr;

	pm_stat_ptr = (master_status_structure *)wess_get_master_status();

	if (pm_stat_ptr)
	{
		return (pm_stat_ptr->ptrkstattbl + track)->volume_cntrl;
	}

	return -1;
}

static unsigned char voices_active_table[128]; // 800B4320

char *get_voices_active_table(void) // L80035988
{
	master_status_structure *pm_stat_ptr;
	int voices;
	int i;

	pm_stat_ptr = (master_status_structure *)wess_get_master_status();

	voices = wess_driver_voices;

	if (!(voices < 128))
		voices = 127;

	if (pm_stat_ptr)
	{
		i = 0;
		if (voices > 0)
		{
			if (voices & 3)
			{
				do
				{
					if ((pm_stat_ptr->pvoicestattbl + i)->flags & VOICE_ACTIVE)
						voices_active_table[i] = '1';
					else
						voices_active_table[i] = '0';

					i++;
				} while (voices & 3 != i);
			}
		}

		if (i != voices)
		{
			do
			{
				if ((pm_stat_ptr->pvoicestattbl + i)->flags & VOICE_ACTIVE)
					voices_active_table[i] = '1';
				else
					voices_active_table[i] = '0';

				if ((pm_stat_ptr->pvoicestattbl + (i + 1))->flags & VOICE_ACTIVE)
					voices_active_table[i + 1] = '1';
				else
					voices_active_table[i + 1] = '0';

				if ((pm_stat_ptr->pvoicestattbl + (i + 2))->flags & VOICE_ACTIVE)
					voices_active_table[i + 2] = '1';
				else
					voices_active_table[i + 2] = '0';

				if ((pm_stat_ptr->pvoicestattbl + (i + 3))->flags & VOICE_ACTIVE)
					voices_active_table[i + 3] = '1';
				else
					voices_active_table[i + 3] = '0';

				i += 4;
			} while (voices != i);
		}

		voices_active_table[voices] = '\0';
	}

	return (char *)&voices_active_table;
}

static unsigned char voices_handle_table[128]; // 800B43A0

char *get_voices_handle_table(void) // 80035B24
{
	master_status_structure *pm_stat_ptr;
	int voices;
	int i;

	pm_stat_ptr = (master_status_structure *)wess_get_master_status();

	voices = wess_driver_voices;

	if (!(voices < 128))
		voices = 127;

	if (pm_stat_ptr)
	{
		i = 0;
		if (voices > 0)
		{
			if (voices & 3)
			{
				do
				{
					if ((pm_stat_ptr->pvoicestattbl + i)->flags & VOICE_RELEASE)
						voices_handle_table[i] = '1';
					else
						voices_handle_table[i] = '0';

					i++;
				} while (voices & 3 != i);
			}
		}

		if (i != voices)
		{
			do
			{
				if ((pm_stat_ptr->pvoicestattbl + i)->flags & VOICE_RELEASE)
					voices_handle_table[i] = '1';
				else
					voices_handle_table[i] = '0';

				if ((pm_stat_ptr->pvoicestattbl + (i + 1))->flags & VOICE_RELEASE)
					voices_handle_table[i + 1] = '1';
				else
					voices_handle_table[i + 1] = '0';

				if ((pm_stat_ptr->pvoicestattbl + (i + 2))->flags & VOICE_RELEASE)
					voices_handle_table[i + 2] = '1';
				else
					voices_handle_table[i + 2] = '0';

				if ((pm_stat_ptr->pvoicestattbl + (i + 3))->flags & VOICE_RELEASE)
					voices_handle_table[i + 3] = '1';
				else
					voices_handle_table[i + 3] = '0';

				i += 4;
			} while (voices != i);
		}

		voices_handle_table[voices] = '\0';
	}

	return (char *)&voices_handle_table;
}
#endif
