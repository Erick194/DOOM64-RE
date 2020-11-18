/* WESS API INCLUDES */
#include "wessapi.h"	// audio stuff...
#include "seqload.h"
#include "soundhw.h"
#include "wessarc.h"
#include "wessseq.h"

#include "funqueue.h"

#include "graph.h"

#ifndef NOUSEWESSCODE

extern void run_queue_wess_seq_stopall(void);
extern void run_queue_wess_seq_stop(void);

extern void run_queue_wess_master_sfx_vol_set(void);
extern void run_queue_wess_master_mus_vol_set(void);
extern void run_queue_wess_handle_parm_mod(void);
extern void run_queue_wess_handle_noteon(void);

extern void run_queue_wess_seq_pause(void);
extern void run_queue_wess_seq_restart(void);
extern void run_queue_wess_seq_pauseall(void);
extern void run_queue_wess_seq_restartall(void);

extern void run_queue_wess_seq_stoptype(void);
extern void run_queue_wess_seq_update_type_special(void);
extern void run_queue_wess_seq_stopall_and_voiceramp(void);
extern void run_queue_wess_seq_stop_and_voiceramp(void);
extern void run_queue_wess_seq_stoptype_and_voiceramp(void);

static void(*FuncQueueArr[15])(void) =	//8005D9A0
{
	run_queue_wess_seq_stopall,					//QUEUE_SEQ_STOPALL
	run_queue_wess_seq_stop,					//QUEUE_SEQ_STOP
	run_queue_wess_master_sfx_vol_set,			//QUEUE_MASTER_SFX_VOL_SET
	run_queue_wess_master_mus_vol_set,			//QUEUE_MASTER_MUS_VOL_SET
	run_queue_wess_handle_parm_mod,				//QUEUE_HANDLE_PARM_MOD
	run_queue_wess_handle_noteon,				//QUEUE_HANDLE_NOTEON
	run_queue_wess_seq_pause,					//QUEUE_SEQ_PAUSE
	run_queue_wess_seq_restart,					//QUEUE_SEQ_RESTART
	run_queue_wess_seq_pauseall,				//QUEUE_SEQ_PAUSEALL
	run_queue_wess_seq_restartall,				//QUEUE_SEQ_RESTARTALL
	run_queue_wess_seq_stoptype,				//QUEUE_SEQ_STOPTYPE
	run_queue_wess_seq_update_type_special,		//QUEUE_SEQ_UPDATE_TYPE_SPECIAL
	run_queue_wess_seq_stopall_and_voiceramp,	//QUEUE_SEQ_STOPALL_AND_VOICERAMP
	run_queue_wess_seq_stop_and_voiceramp,		//QUEUE_SEQ_STOP_AND_VOICERAMP
	run_queue_wess_seq_stoptype_and_voiceramp	//QUEUE_SEQ_STOPTYPE_AND_VOICERAMP
};

static int func_num = 0;							//8005D9DC
static int func_num_arr = 0;						//8005D9E0
static QUEUE_DATA_MEMORY queue_data_mem;			//800B4420
static char *queue_write_ptr = queue_data_mem.data; //8005D9E4
static char *queue_read_ptr;						//800B6620

void queue_memcpy(void *dest, void *src, int size) // 80035CC0
{
	char *d, *s;

	d = (char *)dest;
	s = (char *)src;
	while (size--)
	{
		*d++ = *s++;
	}
}

void queue_the_function(char mode) // 80035CF0
{
	if (func_num_arr == 0)
	{
		if (func_num < MAX_QUEUE_FUNCTION_SIZE)
		{
			queue_data_mem.function[func_num] = mode;
			func_num += 1;
			return;
		}
		func_num_arr = 1;
	}
	return;
}

void queue_the_data(void *src, int size) // 80035D48
{
	int memsize;

	if (func_num_arr == 0)
	{
		memsize = queue_data_mem.data - queue_write_ptr + MAX_QUEUE_DATA_SIZE;

		if (memsize < size)
		{
			if (func_num > 0)
				func_num -= 1;

			func_num_arr = 1;
		}
		else
		{
			queue_memcpy(queue_write_ptr, src, size);
			queue_write_ptr += size;
		}
	}
	return;
}

void unqueue_the_data(void *dest, int size) // 80035DF4
{
	queue_memcpy(dest, queue_read_ptr, size);
	queue_read_ptr += size;
}

void process_function_queue(void) // 80035E38
{
	char *func;
	int i;

	//PRINTF_D2(WHITE,0,10,"process_function_queue");
	//PRINTF_D2(WHITE,0,11,"func_num %d",func_num);

	func_num_arr = 0;

	//init queue_read_ptr
	queue_read_ptr = (char *)&queue_data_mem.data;

	func = queue_data_mem.function;
	for (i = 0; i < func_num; i++)
	{
	    //PRINTF_D(WHITE,"*func %d",*func);
		FuncQueueArr[*func]();
		func++;
	}

	//restore queue_write_ptr
	wess_disable();
	func_num = 0;
	queue_write_ptr = (char *)&queue_data_mem.data;
	wess_enable();
	return;
}

#endif
