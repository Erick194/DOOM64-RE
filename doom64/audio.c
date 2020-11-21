
#include "audio.h"

#include "graph.h"

//------------
extern void N64_wdd_location(char *wdd_location);
extern void N64_set_output_rate(u32 rate);

extern int wess_driver_num_dma_buffers;       // 8005D948
extern int wess_driver_num_dma_messages;      // 8005D94C
extern int wess_driver_dma_buffer_length;   // 8005D950
extern int wess_driver_extra_samples;         // 8005D954
extern int wess_driver_frame_lag;              // 8005D958
extern int wess_driver_voices;                // 8005D95C
extern int wess_driver_updates;               // 8005D960
extern int wess_driver_sequences;             // 8005D964
extern int wess_driver_tracks;                // 8005D968
extern int wess_driver_gates;                  // 8005D96C
extern int wess_driver_iters;                  // 8005D970
extern int wess_driver_callbacks;              // 8005D974
extern int wess_driver_max_trks_per_seq;      // 8005D978                                                                                        load_sequence_data:80039868(R),
extern int wess_driver_max_subs_per_trk;       // 8005D97C

/* used by wesssys_exit */
enum RestoreFlag {NoRestore,YesRestore};

extern int wesssys_init(void);
extern void wesssys_exit(enum RestoreFlag rflag);

//------------

#ifndef NOUSEWESSCODE
//------------
AMAudioMgr  __am;           //800B4060
ALVoice     *voice;         //800B40E0
char        *reverb_status; //800B40E4
//------------

//------------
AMDMAState      dmaState;			//800B40C0
AMDMABuffer		*dmaBuffs;			//800B40CC
u32             audFrameCt = 0;		//8005D8B0
u32             nextDMA = 0;		//8005D8B4
u32             curAcmdList = 0;	//8005D8B8
u32             minFrameSize;		//800B40D0
u32             frameSize;			//800B40D4
u32             maxFrameSize;		//800b40d8
u32             maxRSPCmds;			//800B40DC

/* Queues and storage for use with audio DMA's */
OSMesgQueue     audDMAMessageQ;		//800B40E8
OSIoMesg        *audDMAIOMesgBuf;	//800B4100
OSMesg          *audDMAMessageBuf;	//800B4104

u32				buf = 0;			//8005D8BC
AudioInfo		*lastInfo = 0;		//8005D8C0

u32				init_completed = 0;	//8005D8C4

#define NUM_DMA_MESSAGES	8	/* The maximum number of DMAs any one frame can have.*/
OSMesgQueue     wess_dmaMessageQ;	//800b4108
OSMesg          wess_dmaMessageBuf[NUM_DMA_MESSAGES];//800b4120
//------------

int wess_memfill(void *dst, unsigned char fill, int count) // 8002E300
{
    char *d;

    d = (char *)dst;
	while (count != 0)
	{
		*d++ = (unsigned char)fill;
		count--;
	}
	return count;
}

int wess_rom_copy(char *src, char *dest, int len) // 8002E334
{
	OSMesg dummyMesg;
	OSIoMesg DMAIoMesgBuf;

	if (init_completed != 0 && len != 0)
	{
		osInvalDCache((void *)dest, (s32)len);
		osPiStartDma(&DMAIoMesgBuf, OS_MESG_PRI_HIGH, OS_READ, (u32)src, (void *)dest, (u32)len, &wess_dmaMessageQ);
		osRecvMesg(&wess_dmaMessageQ, &dummyMesg, OS_MESG_BLOCK);

		return len;
	}

	return 0;
}

/*LEAF(milli_to_param)
lui     $at, 0x447A
mtc1    $a1, $f12
mtc1    $at, $f8
mtc1    $a0, $f4
div.s   $f10, $f12, $f8
cvt.s.w $f6, $f4
mul.s   $f16, $f6, $f10
cfc1    $t6, FCSR
nop
li      $at, $t6, 3
xori    $at, $at, 2
ctc1    $at, FCSR
li      $at, 0xFFFFFFF8
cvt.w.s $f18, $f16
mfc1    $v0, $f18
ctc1    $t6, FCSR
and     $t7, $v0, $at
jr      $ra
move    $v0, $t7
END(milli_to_param)*/

s32 milli_to_param(register s32 paramvalue, register s32 rate) // 8002E3D0
{
	register u32 fpstat, fpstatset, out;

#ifdef N64ASM
	asm("lui		$at, 	0x447A\n");
	asm("mtc1		%0, 	$f12\n" ::"r"(rate));//mtc1    $a1, $f12
	asm("mtc1		$at, 	$f8\n");
	asm("mtc1		%0, 	$f4\n" ::"r"(paramvalue));//mtc1    $a0, $f4
	asm("div.s		$f10, 	$f12, 	$f8\n");
	asm("cvt.s.w 	$f6, 	$f4\n");
	asm("mul.s   	$f16, 	$f6, 	$f10\n");
	// fetch the current floating-point control/status register
	asm("cfc1   	%0, 	$f31\n" :"=r"(fpstat));
	asm("nop\n");
	// enable round to negative infinity for floating point
	//"li			$at, 	$t6, 3\n"
	asm("ori      	$at, 	%0, 3\n" ::"r"(fpstat));//# fpstat |= FPCSR_RM_RM;
	asm("xori   	$at, 	$at, 2\n");
	asm("ctc1   	$at, 	$f31\n");
	//asm("li      	$at, 	0xFFFFFFF8\n");
	asm("cvt.w.s 	$f18, 	$f16\n");
	asm("mfc1    	%0, 	$f18\n" :"=r"(out));
	// _Disable_ unimplemented operation exception for floating point.
	asm("ctc1   	%0, 	$f31\n" ::"r"(fpstat));

	return (s32)(out &~0x7);
#else
	//No asm mode

	/*
	// fetch the current floating-point control/status register
	fpstat = __osGetFpcCsr();
	// enable round to negative infinity for floating point
	fpstatset = (fpstat | FPCSR_RM_RM) ^ 2;
	// _Disable_ unimplemented operation exception for floating point.
	__osSetFpcCsr(fpstatset);
	*/

	return (s32)((f32)paramvalue * ((f32)rate / 1000.0f)) &~0x7;
#endif
}

void wess_init(WessConfig *wessconfig) // 8002E41C
{
	ALSynConfig config;
	f32 samplerate;
	s32 *params;//Custom reverb
	int sections, section_pos;

	// Create audio DMA thread...
	osCreateMesgQueue(&wess_dmaMessageQ, wess_dmaMessageBuf, NUM_DMA_MESSAGES);

	buf = 0;
	lastInfo = NULL;

	N64_wdd_location(wessconfig->wdd_location);

	config.dmaproc = __amDmaNew;
	config.maxPVoices = config.maxVVoices = wess_driver_voices;
	config.maxUpdates = wess_driver_updates;
	config.fxType = (ALFxId)wessconfig->reverb_id;

	N64_set_output_rate(wessconfig->outputsamplerate);
	config.outputRate = osAiSetFrequency(wessconfig->outputsamplerate);

	config.heap = wessconfig->heap_ptr;

	if (config.fxType == WESS_REVERB_CUSTOM)
	{
		/* set address reverb table*/
		params = wessconfig->revtbl_ptr;

		samplerate = wessconfig->outputsamplerate;
		if ((u32)samplerate < 0) { samplerate += 4.2949673e9; }

		/* total allocated memory */
		params[1] = milli_to_param(params[1], (u32)samplerate);

		/* total allocated memory -> params[0]*/
		section_pos = 0;
		for (sections = 0; sections < params[0]; sections++)
		{
			samplerate = wessconfig->outputsamplerate;
			if ((u32)samplerate < 0) { samplerate += 4.2949673e9; }

			/* input */
			params[2 + section_pos] = milli_to_param(params[2 + section_pos], (u32)samplerate);

			samplerate = wessconfig->outputsamplerate;
			if ((u32)samplerate < 0) { samplerate += 4.2949673e9; }

			/* output */
			params[3 + section_pos] = milli_to_param(params[3 + section_pos], (u32)samplerate);

			section_pos += 8;
		}

		config.params = params;
	}

	amCreateAudioMgr(&config, wessconfig);
	SSP_SeqpNew();
	wesssys_init();
	init_completed = 1;
}


/*-------------------*/
/* Audio Manager API */
/*-------------------*/

void amCreateAudioMgr(ALSynConfig *config, WessConfig *wessconfig) // 8002E610
{
	f32 fsize;
	f32 fsize2;
	int frameSize1;
	int i;

	/*
     * Calculate the frame sample parameters from the
     * video field rate and the output rate
     */
	fsize = (f32)(config->outputRate) / wessconfig->audioframerate;
	frameSize1 = (s32)fsize;

	if (frameSize1 < 0) {
		frameSize1 = -1;
	}

	fsize2 = (float)frameSize1;
	if (frameSize1 < 0) {
		fsize2 += 4294967296.0;
	}

	frameSize = frameSize1 + 1;
	if (fsize <= fsize2) {
		frameSize = frameSize1;
	}

	if (frameSize & 15)
		frameSize = (frameSize & ~0xf) + 16;

	minFrameSize = frameSize - 16;
	maxFrameSize = frameSize + wess_driver_extra_samples + 16;

	voice = (ALVoice *)alHeapAlloc(config->heap, 1, wess_driver_voices * sizeof(ALVoice));
	wess_memfill(voice, 0, wess_driver_voices * sizeof(ALVoice));

	reverb_status = (char *)alHeapAlloc(config->heap, 1, wess_driver_voices);
	wess_memfill(reverb_status, 0, wess_driver_voices);

	dmaBuffs = (AMDMABuffer *)alHeapAlloc(config->heap, 1, wess_driver_num_dma_buffers * sizeof(AMDMABuffer));
	wess_memfill(dmaBuffs, 0, wess_driver_num_dma_buffers * sizeof(AMDMABuffer));

	/* allocate buffers for voices */
	dmaBuffs[0].node.prev = 0;
	dmaBuffs[0].node.next = 0;

	for (i = 0; i < (wess_driver_num_dma_buffers - 1); i++)
	{
		alLink((ALLink *)&dmaBuffs[i + 1], (ALLink *)&dmaBuffs[i]);
		dmaBuffs[i].ptr = (char *)alHeapAlloc(config->heap, 1, wess_driver_dma_buffer_length);
		wess_memfill(dmaBuffs[i].ptr, 0, wess_driver_dma_buffer_length);
	}

	/* last buffer already linked, but still needs buffer */
	dmaBuffs[i].ptr = alHeapAlloc(config->heap, 1, wess_driver_dma_buffer_length);
	wess_memfill(dmaBuffs[i].ptr, 0, wess_driver_dma_buffer_length);

	for (i = 0; i < NUM_ACMD_LISTS; i++)
	{
		__am.ACMDList[i] = (Acmd*)alHeapAlloc(config->heap, 1, wessconfig->maxACMDSize * sizeof(Acmd));
		wess_memfill(__am.ACMDList[i], 0, wessconfig->maxACMDSize * sizeof(Acmd));
	}

	maxRSPCmds = wessconfig->maxACMDSize;

	/* initialize the done messages */
	for (i = 0; i < NUM_OUTPUT_BUFFERS; i++)
	{
		__am.audioInfo[i] = (AudioInfo *)alHeapAlloc(config->heap, 1, sizeof(AudioInfo));
		wess_memfill(__am.audioInfo[i], 0, sizeof(AudioInfo));

		/* allocate output buffer */
		__am.audioInfo[i]->data = alHeapAlloc(config->heap, 1, maxFrameSize << 2);
		wess_memfill(__am.audioInfo[i]->data, 0, maxFrameSize << 2);
	}

	audDMAIOMesgBuf = alHeapAlloc(config->heap, 1, wess_driver_num_dma_messages * sizeof(OSIoMesg));
	wess_memfill(audDMAIOMesgBuf, 0, wess_driver_num_dma_messages * sizeof(OSIoMesg));

	audDMAMessageBuf = alHeapAlloc(config->heap, 1, wess_driver_num_dma_messages * sizeof(OSMesg));
	wess_memfill(audDMAMessageBuf, 0, wess_driver_num_dma_messages * sizeof(OSMesg));

	osCreateMesgQueue(&audDMAMessageQ, audDMAMessageBuf, wess_driver_num_dma_messages);

	alInit(&__am.g, config);
}

OSTask * wess_work(void) // 8002EB2C
{
	OSTask *validTask;

	if (init_completed == 0)
		return (OSTask *)NULL;

	validTask = __amHandleFrameMsg(__am.audioInfo[buf]);

	lastInfo = __am.audioInfo[buf];

	buf++;
	if (buf == NUM_OUTPUT_BUFFERS)
		buf = 0;

	if (validTask)
		curAcmdList ^= 1; /* swap which acmd list you use each frame */

	return validTask;
}

OSTask *__amHandleFrameMsg(AudioInfo *info) // 8002EBD8
{
	s16		*audioPtr;
	Acmd	*cmdp;
	s32		cmdLen;
	int		samplesLeft;
	int		check;

	/* audFrameCnt updated here */
	__clearAudioDMA(); /* call once a frame, before doing alAudioFrame */

	audioPtr = (s16 *)osVirtualToPhysical(info->data); /* info->data addr of current buffer */

	/* set up the next DMA transfer from DRAM to the audio interface buffer. */
	/* lastInfo->data is the buffer used in the last audio frame. It should be full. */
	if (lastInfo)
	{
		osAiSetNextBuffer(lastInfo->data, lastInfo->frameSamples << 2);
	}

	/* calculate how many samples needed for this frame to keep the DAC full */
	/* this will vary slightly frame to frame, must recalculate every frame */
	samplesLeft = osAiGetLength() >> 2; /* divide by four, to convert bytes */

	/* to stereo 16 bit samples */
	info->frameSamples = ((frameSize - samplesLeft) + wess_driver_extra_samples & ~0xf) + 16;

	/* no longer necessary to have extra samples, because the buffers */
	/* will be swapped exactly when the buffer runs out */
	/* info->frameSamples = frameSize; */

	if (info->frameSamples < minFrameSize)
		info->frameSamples = minFrameSize;

	cmdp = alAudioFrame(__am.ACMDList[curAcmdList], &cmdLen, audioPtr, info->frameSamples);

	if (maxRSPCmds < cmdLen)
	{
		wess_error_callback("MAXRSPCMDS", 0, 0);
	}

	/* this is the task for the next audio frame */
	info->task.t.data_ptr = (u64 *)__am.ACMDList[curAcmdList];
	info->task.t.data_size = (u32)((int)(((int)cmdp - (int)__am.ACMDList[curAcmdList]) >> 3) << 3);
	info->task.t.type = M_AUDTASK;
	info->task.t.ucode_boot = (u64 *)rspbootTextStart;
	info->task.t.ucode_boot_size = ((int)rspbootTextEnd - (int)rspbootTextStart);
	info->task.t.flags = 0;
	info->task.t.ucode = (u64 *)aspMainTextStart;
	info->task.t.ucode_data = (u64 *)aspMainDataStart;
	info->task.t.ucode_data_size = SP_UCODE_DATA_SIZE;
	info->task.t.yield_data_ptr = NULL;
	info->task.t.yield_data_size = 0;

	return (OSTask *)&info->task;
}

s32 __amDMA(s32 addr, s32 len, void *state) // 8002ED74
{
	char            *foundBuffer;
	s32             delta, addrEnd, buffEnd;
	AMDMABuffer     *dmaPtr, *lastDmaPtr;

	lastDmaPtr = 0;
	dmaPtr = dmaState.firstUsed;
	addrEnd = addr+len;

	/* first check to see if a currently existing buffer contains the
	sample that you need.  */
	while (dmaPtr)
	{
		buffEnd = dmaPtr->startAddr + wess_driver_dma_buffer_length;
		if (dmaPtr->startAddr > addr) /* since buffers are ordered */
			break;                   /* abort if past possible */

		else if (addrEnd <= buffEnd) /* yes, found a buffer with samples */
		{
			dmaPtr->lastFrame = audFrameCt; /* mark it used */
			foundBuffer = dmaPtr->ptr + addr - dmaPtr->startAddr;
			return (int)osVirtualToPhysical(foundBuffer);
		}
		lastDmaPtr = dmaPtr;
		dmaPtr = (AMDMABuffer*)dmaPtr->node.next;
	}

	/* get here, and you didn't find a buffer, so dma a new one */

	/* get a buffer from the free list */
	dmaPtr = dmaState.firstFree;

	/* be sure you have a buffer, */
	/* if you don't have one, you're fucked */
	if (!(dmaPtr))
	{
		lastDmaPtr = 0;
		wess_error_callback("DMAPTRNULL", 0, 0);
		return (int)osVirtualToPhysical(dmaState.firstUsed);
	}

	dmaState.firstFree = (AMDMABuffer*)dmaPtr->node.next;
	alUnlink((ALLink*)dmaPtr);

	/* add it to the used list */
	if (lastDmaPtr) /* if you have other dmabuffers used, add this one */
	{              /* to the list, after the last one checked above */
		alLink((ALLink*)dmaPtr, (ALLink*)lastDmaPtr);
	}
	else if (dmaState.firstUsed) /* if this buffer is before any others */
	{                           /* jam at begining of list */
		lastDmaPtr = dmaState.firstUsed;
		dmaState.firstUsed = dmaPtr;
		dmaPtr->node.next = (ALLink*)lastDmaPtr;
		dmaPtr->node.prev = 0;
		lastDmaPtr->node.prev = (ALLink*)dmaPtr;
	}
	else /* no buffers in list, this is the first one */
	{
		dmaState.firstUsed = dmaPtr;
		dmaPtr->node.next = 0;
		dmaPtr->node.prev = 0;
	}

	foundBuffer = dmaPtr->ptr;
	delta = addr & 0x1;
	addr -= delta;
	dmaPtr->startAddr = addr;
	dmaPtr->lastFrame = audFrameCt;  /* mark it */

	osPiStartDma(&audDMAIOMesgBuf[nextDMA++], OS_MESG_PRI_HIGH, OS_READ,
		(u32)addr, foundBuffer, wess_driver_dma_buffer_length, &audDMAMessageQ);

	return (int)osVirtualToPhysical(foundBuffer) + delta;
}

ALDMAproc __amDmaNew(AMDMAState **state) // 8002EF48
{

	if (!dmaState.initialized)  /* only do this once */
	{
		dmaState.firstUsed = 0;
		dmaState.initialized = 1;
		dmaState.firstFree = &dmaBuffs[0];
	}

	return __amDMA;
}

void __clearAudioDMA(void) // 8002EF7C
{
	u32          i;
	OSIoMesg     *iomsg;
	AMDMABuffer  *dmaPtr, *nextPtr;

	/* Don't block here. If dma's aren't complete, you've had an audio */
	/* overrun. (Bad news, but go for it anyway, and try and recover. */
	for (i = 0; i<nextDMA; i++)
	{
		if (osRecvMesg(&audDMAMessageQ, (OSMesg *)&iomsg, OS_MESG_NOBLOCK) == -1)
		{
			wess_error_callback("DMANOTDONE", 0, 0);
		}
	}

	dmaPtr = dmaState.firstUsed;

	while (dmaPtr)
	{

		nextPtr = (AMDMABuffer*)dmaPtr->node.next;

		/* remove old dma's from list */
		/* Can change FRAME_LAG value if we want.  Should be at least one.  */
		/* Larger values mean more buffers needed, but fewer DMA's */
		if (dmaPtr->lastFrame + wess_driver_frame_lag  < audFrameCt)
		{
			if (dmaState.firstUsed == dmaPtr)
				dmaState.firstUsed = (AMDMABuffer*)dmaPtr->node.next;

			alUnlink((ALLink*)dmaPtr);

			if (dmaState.firstFree)
			{
				alLink((ALLink*)dmaPtr, (ALLink*)dmaState.firstFree);
			}
			else
			{
				dmaState.firstFree = dmaPtr;
				dmaPtr->node.next = 0;
				dmaPtr->node.prev = 0;
			}
		}

		dmaPtr = nextPtr;
	}

	nextDMA = 0;  /* Reset number of DMAs */
	audFrameCt++;
}

void wess_exit(void) // 8002F0CC
{
	wesssys_exit(YesRestore);
	alClose(&__am.g);
}

#endif // 0
