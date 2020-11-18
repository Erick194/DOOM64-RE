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

int wess_seq_range_sizeof(int seqfirst, int numseqs) // 8003A230
{
	int count;

	count = 0;
	if (seq_loader_enable)
	{
		if (numseqs == 0)
			return 0;

		while (numseqs--)
		{
			count += wess_seq_sizeof(seqfirst);
			seqfirst++;
			//printf("numseqs %d || seqfirst %d || count %d\n",numseqs, seqfirst, count);
		}
	}
	return (count);
}

int wess_seq_range_load(int seqfirst, int numseqs, void *memptr) // 8003A2AC
{
	int count;
	char *pmem;

	pmem = (char *)memptr;

	count = 0;
	if (seq_loader_enable)
	{
		if (!open_sequence_data())
			return 0;

		if (!numseqs)
			return 0;

		while (numseqs--)
		{
			count += wess_seq_load(seqfirst, pmem + count);
			seqfirst += 1;
		}

		close_sequence_data();
	}

	return count;
}

int wess_seq_range_free(int seqfirst, int numseqs) // 8003A35C
{
	if (seq_loader_enable)
	{
		if (numseqs == 0)
			return 0;

		while (numseqs--)
		{
			wess_seq_free(seqfirst);
			seqfirst++;
		}

		return 1;
	}
	return 0;
}

#endif
