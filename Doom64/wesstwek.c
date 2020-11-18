
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
#include "wesstrak.h"
#include "wesshand.h"


#include "graph.h"

#ifndef NOUSEWESSCODE

void wess_set_tweaks(WessTweakAttr *attr) // 8002F180
{
	if (attr->mask & TWEAK_DMA_BUFFERS)
		wess_driver_num_dma_buffers = attr->dma_buffers;

	if (attr->mask & TWEAK_DMA_MESSAGES)
		wess_driver_num_dma_messages = attr->dma_messages;

	if (attr->mask & TWEAK_DMA_BUFFER_LENGTH)
		wess_driver_dma_buffer_length = attr->dma_buffer_length;

	if (attr->mask & TWEAK_EXTRA_SAMPLES)
		wess_driver_extra_samples = attr->extra_samples;

	if (attr->mask & TWEAK_FRAME_LAG)
		wess_driver_frame_lag = attr->frame_lag;

	if (attr->mask & TWEAK_VOICES)
		wess_driver_voices = attr->voices;

	if (attr->mask & TWEAK_UPDATES)
		wess_driver_updates = attr->updates;

	if (attr->mask & TWEAK_SEQUENCES)
		wess_driver_sequences = attr->sequences;

	if (attr->mask & TWEAK_TRACKS)
		wess_driver_tracks = attr->tracks;

	if (attr->mask & TWEAK_GATES)
		wess_driver_gates = attr->gates;

	if (attr->mask & TWEAK_ITERS)
		wess_driver_iters = attr->iters;

	if (attr->mask & TWEAK_CALLBACKS)
		wess_driver_callbacks = attr->callbacks;

	if (attr->mask & TWEAK_MAX_TRKS_PER_SEQ)
		wess_driver_max_trks_per_seq = attr->max_trks_per_seq;

	if (attr->mask & TWEAK_MAX_SUBS_PER_TRK)
		wess_driver_max_subs_per_trk = attr->max_subs_per_trk;
}

void wess_get_tweaks(WessTweakAttr *attr) // 8002F348
{
	attr->mask = 0;
	attr->dma_buffers = wess_driver_num_dma_buffers;
	attr->dma_messages = wess_driver_num_dma_messages;
	attr->dma_buffer_length = wess_driver_dma_buffer_length;
	attr->extra_samples = wess_driver_extra_samples;
	attr->frame_lag = wess_driver_frame_lag;
	attr->voices = wess_driver_voices;
	attr->updates = wess_driver_updates;
	attr->sequences = wess_driver_sequences;
	attr->tracks = wess_driver_tracks;
	attr->gates = wess_driver_gates;
	attr->iters = wess_driver_iters;
	attr->callbacks = wess_driver_callbacks;
	attr->max_trks_per_seq = wess_driver_max_trks_per_seq;
	attr->max_subs_per_trk = wess_driver_max_subs_per_trk;
}
#endif
