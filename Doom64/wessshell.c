/* ULTRA64 LIBRARIES */
#include <ultra64.h>
#include "ultratypes.h"
#include <libaudio.h>

#include "wessarc.h"
#include "wessshell.h"
#ifndef NOUSEWESSCODE
ALPlayer wessnode;	//800B4140
ALPlayer *wessstate;//800B4154

void SSP_SeqpNew(void) // 8002F100
{
	wessnode.next = NULL;
	wessnode.handler = __wessVoiceHandler;
	wessnode.callTime = 0;
	wessnode.samplesLeft = 0;
	wessnode.clientData = wessstate;
	alSynAddPlayer(&alGlobals->drvr, &wessnode);
}

ALMicroTime __wessVoiceHandler(void *node) // 8002F154
{
	WessInterruptHandler();
	return 8333;   /* call back in 8.333 millisecond */
}
#endif // 0
