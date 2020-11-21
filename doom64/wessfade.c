
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

#include "wessfade.h"
#ifndef NOUSEWESSCODE

int wess_seq_fade(int seq_num) // 80035F00
{
	return Is_Seq_Num_Valid(seq_num);
}

int wess_seq_fadeall(int seq_num) // 80035F24
{
	return Is_Module_Loaded();
}

int wess_seq_segue(int seq_num, int seq_num2) // 80035F48
{
	int val;

	val = Is_Seq_Num_Valid(seq_num);
	if (val)
		val = Is_Seq_Num_Valid(seq_num2);

	return (val);
}
#endif
