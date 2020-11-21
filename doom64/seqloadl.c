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

#ifndef NOUSEWESSCODE

extern module_header	sfile_hdr;				//800b6a60
extern master_status_structure *ref_pm_stat;	//800B6A80
extern char *loaderfile;						//800B6A84
extern int ref_max_seq_num;					    //800B6A88
extern int opencount;							//800B6A8C
extern int(*Error_func)(int, int);				//800B6A90
extern int Error_module;						//800B6A94
extern Wess_File_IO_Struct *fp_seq_file;		//800B6A98
extern int seq_loader_offset;					//800B6A9C
extern int seq_loader_enable;					//800B6AA0

#define END_SEQ_LIST -1

int wess_seq_list_sizeof(short *seqlist) // 8003A090
{
	int seqnum;
	int count;

	count = 0;
	if (seq_loader_enable)
	{
		if (*seqlist == END_SEQ_LIST)
			return 0;

		do
		{
			seqnum = *seqlist++;
			count += wess_seq_sizeof(seqnum);
		} while (seqnum != END_SEQ_LIST);
	}
	return (count);
}

int wess_seq_list_load(short *seqlist, void *memptr) // 8003A108
{
	int seqnum;
	int count;
	char *pmem;

	pmem = (char *)memptr;

	count = 0;
	if (seq_loader_enable)
	{
		if (!open_sequence_data())
			return 0;

		if (*seqlist != END_SEQ_LIST)
		{
			do
			{
				seqnum = *seqlist++;
				count += wess_seq_load(seqnum, pmem + count);
			} while (seqnum != END_SEQ_LIST);
		}

		close_sequence_data();
	}

	return count;
}

int wess_seq_list_free(short *seqlist) // 8003A1B4
{
	int seqnum;
	if (seq_loader_enable)
	{
		if (*seqlist != END_SEQ_LIST)
		{
			do
			{
				seqnum = *seqlist++;
				wess_seq_free(seqnum);
			} while (seqnum != END_SEQ_LIST);
		}
		return 1;
	}
	return 0;
}

#endif
