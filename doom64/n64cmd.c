
/* ULTRA64 LIBRARIES */
#include <ultra64.h>
#include "ultratypes.h"
#include <libaudio.h>

#include "wessseq.h"

#include "graph.h"//debug

#define _ALIGN8_ 1

#ifndef NOUSEWESSCODE

extern ALVoice     *voice;         //800B40E0

void wess_set_mute_release(int millisec);
f32 WessCents2Ratio(s32 cents);
void TriggerN64Voice(voice_status *voice_stat/*, unsigned char keynum, unsigned char velnum*/);

void N64_DriverInit (track_status *ptk_stat);//(master_status_structure *pm_stat);
void N64_DriverExit (track_status *ptk_stat);
void N64_DriverEntry1 (track_status *ptk_stat);
void N64_DriverEntry2(track_status *ptk_stat);
void N64_DriverEntry3(track_status *ptk_stat);
void N64_TrkOff(track_status *ptk_stat);
void N64_TrkMute(track_status *ptk_stat);
void N64_PatchChg(track_status *ptk_stat);
void N64_PatchMod(track_status *ptk_stat);
void N64_PitchMod(track_status *ptk_stat);
void N64_ZeroMod(track_status *ptk_stat);
void N64_ModuMod(track_status *ptk_stat);
void N64_VolumeMod(track_status *ptk_stat);
void N64_PanMod(track_status *ptk_stat);
void N64_PedalMod(track_status *ptk_stat);
void N64_ReverbMod(track_status *ptk_stat);
void N64_ChorusMod(track_status *ptk_stat);
void N64_voiceon(voice_status *voice_stat, track_status *ptk_stat,
	           patchmaps_header *patchmaps, patchinfo_header *patchinfo,
	           unsigned char keynum, unsigned char velnum);
void N64_voiceparmoff(voice_status *voice_stat);
void N64_voicemuterelease(voice_status *voice_stat, int muterelease);
void N64_voicerelease(voice_status *voice_stat);
void N64_voicedecay(voice_status *voice_stat);
void N64_voicenote(track_status *ptk_stat,
	           patchmaps_header *patchmap, patchinfo_header *patchinfo,
	           unsigned char keynum, unsigned char velnum);
void N64_NoteOn(track_status *ptk_stat);
void N64_NoteOff(track_status *ptk_stat);

void(*drv_cmds[19])(track_status *) =
{
	N64_DriverInit,         //0x0
	N64_DriverExit,         //0x1
	N64_DriverEntry1,       //0x2
	N64_DriverEntry2,       //0x3
	N64_DriverEntry3,       //0x4
	N64_TrkOff,             //0x5
	N64_TrkMute,            //0x6
	N64_PatchChg,           //0x7
	N64_PatchMod,           //0x8
	N64_PitchMod,           //0x9
	N64_ZeroMod,            //0xA
	N64_ModuMod,            //0xB
	N64_VolumeMod,          //0xC
	N64_PanMod,             //0xD
	N64_PedalMod,           //0xE
	N64_ReverbMod,          //0xF
	N64_ChorusMod,          //0x10
	N64_NoteOn,             //0x11
	N64_NoteOff             //0x12
};

extern unsigned char	master_sfx_volume;
extern unsigned char	master_mus_volume;
extern unsigned char	pan_status;
extern int	            enabledecay;

typedef struct NoteData
{
	short	seq_num;				//*8
	short	track;					//*10
	char	keynum;					//*12
	char	velnum;					//*13
	patchmaps_header *patchmap;		//*16 mapptr
	patchinfo_header *patchinfo;	//*20 sampptr
}NoteData;

typedef struct NoteState
{
	int			numnotes;	//*
	int			remember;	//*4
	NoteData	nd[32];		//*8
}NoteState;//Final Size 520
int i = sizeof(NoteState);

int							N64_mute_release = 250;	// 8005DB48
master_status_structure		*pmsbase;				// 800B66F0
sequence_status				*pssbase;				// 800B66F4
track_status				*ptsbase;				// 800B66F8
voice_status				*pvsbase;				// 800B66FC
unsigned long				nvss;					// 800B6700
patch_group_data			*ppgd;					// 800B6704
unsigned long				*pcurabstime;			// 800B6708
patches_header				*patchesbase;			// 800B670C
patchmaps_header			*patchmapsbase;			// 800B6710
patchinfo_header			*samplesbase;			// 800B6714
char						*drummapsbase;			// 800B6718
loopinfo_header				*samplesinfochunk;		// 800B671C
ALRawLoop2					*samplesrawloopbase;	// 800B6720
ALADPCMloop2				*samplescmploopbase;	// 800B6724
ALADPCMBook2				*samplescmphdrbase;		// 800B6728
ALRawLoop2					rawloop;				// 800B6730
ALADPCMloop2				cmploop;				// 800B6740
NoteState					*pns = 0;				// 8005DB4C
NoteState					pnotestate;				// 800B6770
s32                         g_wddloc;				// 800B6978
f32							wess_output_ratio = 1.0;// 8005DB50

enum Snd_Type {Snd_Music, Snd_Sfx};

extern void (**CmdFuncArr[10])(track_status *);
extern void(*DrvFunctions[36])(track_status *);

//-----------------------------------------------------------
// Sound System
//-----------------------------------------------------------

f32 sample_rate = 22050.0;  // 80061A70
void N64_set_output_rate(u32 rate) // 80037750
{
	f32 out_rate;

	out_rate = (f32)rate;
	if ((s32)rate < 0)
	{
		out_rate += 4.2949673e9;
	}
	wess_output_ratio = (f32)(sample_rate / out_rate);
}

void N64_wdd_location(char *wdd_location) // 80037788
{
	g_wddloc = (s32)wdd_location;
}

void start_record_music_mute(int remember) // 80037794
{
    if ((int)&pnotestate)
	{
		pnotestate.numnotes = 0;
		pnotestate.remember = remember;
	}
	pns = (NoteState *)&pnotestate;
}

void end_record_music_mute(void) // 800377C4
{
	pns = (NoteState *)0;
}

#define REMEMBER_MUSIC   (0x1L<< 0)//1
#define REMEMBER_SNDFX   (0x1L<< 1)//2

void do_record_music_unmute(int seq_num, int track, track_status *ptk_stat) // 800377D0
{
	int i;
	NoteData *notestate;

	for(i = 0; i < pnotestate.numnotes; i++)
	{
		notestate = &pnotestate.nd[i];

		if ((notestate->track == track) && (notestate->seq_num == seq_num) && (notestate->patchmap != 0))
		{
			N64_voicenote(ptk_stat, notestate->patchmap, notestate->patchinfo, notestate->keynum, notestate->velnum);
			notestate->patchmap = (patchmaps_header *)0;
		}
	}
}

void add_music_mute_note(track_status *ptk_stat,
                         unsigned short seq_num,  unsigned short track,
                         unsigned char keynum,  unsigned char velnum,
                         patchmaps_header *patchmap,
                         patchinfo_header *patchinfo) // 800378A8
{

    seq_num = (seq_num << 16) >> 16;
    track = (track << 16) >> 16;

	if (pns != 0)
	{
		if (((ptk_stat->sndclass == MUSIC_CLASS) && (pns->remember & REMEMBER_MUSIC)) ||
			((ptk_stat->sndclass == SNDFX_CLASS) && (pns->remember & REMEMBER_SNDFX)))
		{
			if (pns->numnotes < 32)
			{
				pns->nd[pns->numnotes].seq_num = seq_num;
				pns->nd[pns->numnotes].track = track;
				pns->nd[pns->numnotes].keynum = keynum;
				pns->nd[pns->numnotes].velnum = velnum;
				pns->nd[pns->numnotes].patchmap = patchmap;
				pns->nd[pns->numnotes].patchinfo = patchinfo;
				pns->numnotes++;
			}
		}
	}
}

void wess_set_mute_release(int millisec) // 800379E4
{
	N64_mute_release = millisec;
}

int wess_get_mute_release(void) // 800379F0
{
	return N64_mute_release;
}

f32 WessCents2Ratio(s32 cents) // 80037A00
{
	f32 x;
	f32 ratio = 1.0f;

	if (cents >= 0) {
		x = 1.00057779f;         /* 2^(1/1200) */ //rate1
	}
	else {
		x = 0.9994225441f;       /* 2^(-1/1200) */ //rate2
		cents = -cents;
	}

	while (cents) {
		if (cents & 1)
			ratio *= x;
		x *= x;
		cents >>= 1;
	}

	return ratio * wess_output_ratio;
}

/*extern unsigned long				drv_pitch1;				//L8007EEB8
extern unsigned long				drv_pitch2;				//L8007EEBC
extern unsigned long				drv_pitch3;				//L8007EEC0
extern unsigned long				drv_volume_1;			//L8007EEC4
extern track_status				    *drv_ptrk_stat;			//L8007EEC8
extern short				        drv_pan_1;				//L8007EECC*/

void TriggerN64Voice(voice_status *voice_stat) // 80037A64
{
	static track_status		*pts;	  //800B697C
	static long				adjvol;	  //800B6980
	static short			adjpan;	  //800B6984
	static long				adjpitch; //800B6988
	static ALVoiceConfig	config;	  //800b698c
	static float			pitch;	  //800B6994
	static long				deltatime;//800B6998

	u32 volume;
	s32 priority;
	double fpitch;

	pts = (ptsbase + voice_stat->track);

	if (pan_status != 0)
	{
		adjpan = (s32)(((pts->pan_cntrl + voice_stat->patchmaps->pan) - 0x40) << 16) >> 16;

		if (adjpan < 0)     adjpan = 0;
		if (adjpan > 0x7F)	adjpan = 0x7f;
	}
	else
	{
		adjpan = 0x40;
	}

	//Volume
	if (pts->sndclass == SNDFX_CLASS)
	{
		volume = voice_stat->velnum * voice_stat->patchmaps->volume * pts->volume_cntrl * master_sfx_volume;
	}
	else
	{
		volume = voice_stat->velnum * voice_stat->patchmaps->volume * pts->volume_cntrl * master_mus_volume;
	}

	adjvol = voice_stat->patchmaps->attack_level * (volume >> 0xd) >> 7;

	//Pitch
	if (pts->pitch_cntrl == 0)
	{
		adjpitch = 0;
	}
	else
	{
		if (pts->pitch_cntrl < 1)
		{
			adjpitch = (s32)((f64)(voice_stat->patchmaps->pitchstep_min * pts->pitch_cntrl) * (f64)0.0122);
		}
		else
		{
			adjpitch = (s32)((f64)(voice_stat->patchmaps->pitchstep_max * pts->pitch_cntrl) * (f64)0.0122);
		}
	}

	//Voice Priority
	priority = voice_stat->priority & 255;

	if((priority >= 0) && (priority < 128))
    {
        config.priority = priority;
    }
    else
	{
		config.priority = 0x7f;
	}

	config.fxBus = (u16)0;
	config.unityPitch = (u8)0;

	//PRINTF_D2(WHITE,0,10,"voice_stat->refindx %d   ", voice_stat->refindx);

	alSynAllocVoice(&alGlobals->drvr, &voice[voice_stat->refindx], &config);

	pitch = (f32)WessCents2Ratio((voice_stat->patchinfo->pitch + adjpitch + (voice_stat->keynum - voice_stat->patchmaps->root_key) * 100) - voice_stat->patchmaps->fine_adj);
	deltatime = voice_stat->patchmaps->attack_time * 1000;

	alSynStartVoiceParams(&alGlobals->drvr, &voice[voice_stat->refindx], (ALWaveTable *)&voice_stat->patchinfo->wave, (f32)pitch, (s16)adjvol, (ALPan)adjpan, (u8)pts->reverb, (ALMicroTime)deltatime);
}
//-----------------------------------------------------------
// Driver System
//-----------------------------------------------------------

void N64_DriverInit (track_status *ptk_stat) // 80037DA8
{
    static int vt, mi; //800B699C

	char *pmem;
	patchinfo_header *sample;
	master_status_structure *pm_stat;

	pm_stat = (master_status_structure*)ptk_stat;

	//PRINTF_D2(WHITE,0,7,"N64_DriverInit");
	pmsbase = pm_stat;

	pcurabstime = pm_stat->pabstime;
	pssbase = pm_stat->pseqstattbl;
	ptsbase = pm_stat->ptrkstattbl;
	pvsbase = pm_stat->pvoicestattbl;
	ppgd = pm_stat->ppat_info;
	nvss = wess_driver_voices;

    //PRINTF_D(WHITE,"pmsbase %x",pmsbase);
	//PRINTF_D(WHITE,"pcurabstime %d",*pcurabstime);
	//PRINTF_D(WHITE,"pssbase %x",pssbase);
	//PRINTF_D(WHITE,"ptsbase %x",ptsbase);
	//PRINTF_D(WHITE,"pvsbase %x",pvsbase);
	//PRINTF_D(WHITE,"ppgd %x",ppgd);
	//PRINTF_D(WHITE,"nvss %d",nvss);

	pmem = ppgd->ppat_data; /* pointer math temp */

	patchesbase = (patches_header *)pmem;
	pmem += (unsigned int)(ppgd->pat_grp_hdr.patches * sizeof(patches_header));
	//pmem = ((unsigned int)pmem + 7) & ~7;	/* phrase align everything */
#if _ALIGN8_ == 1
	//force align to word boundary because previous size adjust
	//may wind up with odd address
	pmem += (unsigned int)pmem & 1;
	pmem += (unsigned int)pmem & 2;
	pmem += (unsigned int)pmem & 4;
#endif
    //PRINTF_D(WHITE,"patchesbase %d",(ppgd->pat_grp_hdr.patches));//588

	patchmapsbase = (patchmaps_header *)pmem;
	pmem += (unsigned int)(ppgd->pat_grp_hdr.patchmaps * sizeof(patchmaps_header));
	//pmem = ((unsigned int)pmem + 7) & ~7;	/* phrase align everything */
#if _ALIGN8_ == 1
	//force align to word boundary because previous size adjust
	//may wind up with odd address
	pmem += (unsigned int)pmem & 1;
	pmem += (unsigned int)pmem & 2;
	pmem += (unsigned int)pmem & 4;
#endif
    //PRINTF_D(WHITE,"patchmapsbase %d",(ppgd->pat_grp_hdr.patchmaps));//10100

	samplesbase = (patchinfo_header *)pmem;
	pmem += (unsigned int)(ppgd->pat_grp_hdr.patchinfo * sizeof(patchinfo_header));
	//pmem = ((unsigned int)pmem + 7) & ~7;	/* phrase align everything */
#if _ALIGN8_ == 1
	//force align to word boundary because previous size adjust
	//may wind up with odd address
	pmem += (unsigned int)pmem & 1;
	pmem += (unsigned int)pmem & 2;
	pmem += (unsigned int)pmem & 4;
#endif
    //PRINTF_D(WHITE,"samplesbase %d",(ppgd->pat_grp_hdr.patchinfo));//2976

	drummapsbase = (char *)pmem;
	pmem += (unsigned int)(ppgd->pat_grp_hdr.drummaps * sizeof(char *));
	//pmem = ((unsigned int)pmem + 7) & ~7;	/* phrase align everything */
#if _ALIGN8_ == 1
	//force align to word boundary because previous size adjust
	//may wind up with odd address
	pmem += (unsigned int)pmem & 1;
	pmem += (unsigned int)pmem & 2;
	pmem += (unsigned int)pmem & 4;
#endif
    //PRINTF_D(WHITE,"drummapsbase %d",(ppgd->pat_grp_hdr.drummaps * sizeof(char *)));

	samplesinfochunk = (loopinfo_header *)pmem;
	pmem += sizeof(loopinfo_header);

	samplesrawloopbase = (ALRawLoop2 *)pmem;
	pmem += (samplesinfochunk->rawcount * sizeof(ALRawLoop2));

	samplescmploopbase = (ALADPCMloop2 *)pmem;
	pmem += (samplesinfochunk->adpcmcount * sizeof(ALADPCMloop2));

	samplescmphdrbase = (ALADPCMBook2 *)pmem;//264

	//PRINTF_D(WHITE,"ppgd->pat_grp_hdr.patchinfo %d",ppgd->pat_grp_hdr.patchinfo);

	//PRINTF_D(WHITE,"ALADPCMBook %d",sizeof(ALADPCMBook));
	//PRINTF_D(WHITE,"ALADPCMBook2 %d",sizeof(ALADPCMBook2));
	//PRINTF_D(WHITE,"ALADPCMloop %d",sizeof(ALADPCMloop));
	//PRINTF_D(WHITE,"ALADPCMloop2 %d",sizeof(ALADPCMloop2));

	//PRINTF_D(WHITE,"ALRawLoop %d",sizeof(ALRawLoop));
	//PRINTF_D(WHITE,"ALRawLoop2 %d",sizeof(ALRawLoop2));

	for (vt = 0; vt < ppgd->pat_grp_hdr.patchinfo; vt++)
	{
		sample = (samplesbase + vt); /* pointer math */

		sample->wave.base += g_wddloc; //Offset of (or pointer to) the start of the raw or ADPCM compressed.
		sample->wave.flags = 1;        //If the base field contains an offset, [flags = 0]. If ifcontains an pointer, [flags = 1].

		sample->pitch = (s32)(sample->wave.loop);//move pitch value

		if (sample->wave.type == AL_RAW16_WAVE)
		{
			if ((s32)(sample->wave.book) != -1) // index id for loop table
			{
				sample->wave.loop = &samplesrawloopbase[(u32)(sample->wave.book)];
			}
			else
			{
				sample->wave.loop = &rawloop;
			}
		}
		else//AL_ADPCM_WAVE
		{
			if ((s32)(sample->wave.book) != -1) // index id for loop table
			{
				sample->wave.loop = &samplescmploopbase[(u32)(sample->wave.book)];
			}
			else
			{
				sample->wave.loop = &cmploop;
			}

			sample->wave.book = &samplescmphdrbase[vt];
		}
	}
}

void N64_DriverExit (track_status *ptk_stat) // 8003806C
{
    //PRINTF_D2(WHITE,0,7,"N64_DriverExit");
}

void N64_DriverEntry1 (track_status *ptk_stat) // 80038074
{
    static unsigned long	vn;		//800B69A0
	static unsigned long	vi;		//800B69A4
	static voice_status		*pvs;	//800B69A8

	//printf("N64_DriverEntry1\n");

	//PRINTF_D2(WHITE,0,7,"N64_DriverEntry1");

	vn = pmsbase->voices_active;
	//PRINTF_D2(WHITE,0,8,"vn %d",vn);

	if (vn != 0)
	{
		pvs = pvsbase;
		vi = nvss;

		while (vi--)
		{
			if (pvs->flags & VOICE_ACTIVE)
			{
				if ((pvs->flags & VOICE_RELEASE) && (pvs->pabstime < *pcurabstime))
				{
					N64_voiceparmoff(pvs);
				}
				else if ((pvs->flags & VOICE_DECAY) && ((pvs->pabstime + pvs->patchmaps->attack_time) < *pcurabstime))
				{
					N64_voicedecay(pvs);
				}

				if (!--vn) break;
			}

			pvs++;
		}
	}
}

void N64_DriverEntry2(track_status *ptk_stat) // 800381AC
{
    //PRINTF_D2(WHITE,0,7,"N64_DriverEntry2");
}

void N64_DriverEntry3(track_status *ptk_stat) // 800381B4
{
    //PRINTF_D2(WHITE,0,7,"N64_DriverEntry3");
}

void N64_TrkOff(track_status *ptk_stat) // 800381BC
{
    static sequence_status	*pss;	//800B69AC

    //PRINTF_D2(WHITE,0,7,"N64_TrkOff");

	pss = pssbase + ptk_stat->seq_owner;
	N64_TrkMute(ptk_stat);

	if (ptk_stat->voices_active)
	{
		ptk_stat->flags |= TRK_STOPPED;

		if (!--pss->tracks_playing)
			pss->playmode = SEQ_STATE_STOPPED;
	}
	else
	{
		Eng_TrkOff(ptk_stat);
	}
}

void N64_TrkMute(track_status *ptk_stat) // 80038254
{
	static unsigned long	vn;		//800B69B0
	static unsigned long	vi;		//800B69B4
	static voice_status	    *pvs;	//800B69B8
	static sequence_status	*pss;	//800B69BC

	//PRINTF_D2(WHITE,0,7,"N64_TrkMute");
	vn = ptk_stat->voices_active;

	if (vn != 0)
	{
		pvs = pvsbase;
		vi = nvss;

		while (vi--)
		{
			if ((pvs->flags & VOICE_ACTIVE) && (pvs->track == ptk_stat->refindx))
			{
				if (pns != 0 && (!(pvs->flags & VOICE_RELEASE) && (pvs->sndtype == Snd_Music)))
				{
					pss = (pssbase + ptk_stat->seq_owner);

					add_music_mute_note(ptk_stat,
						pss->seq_num,
						pvs->track,
						pvs->keynum,
						pvs->velnum,
						pvs->patchmaps,
						pvs->patchinfo);
				}

				N64_voicemuterelease(pvs, N64_mute_release);
				if (!--vn) break;
			}

			pvs++;
		}
	}
}

void N64_PatchChg(track_status *ptk_stat) // 800383F8
{
	static unsigned short	thepatch;	//800B69C0

	//PRINTF_D2(WHITE,0,7,"N64_PatchChg");
	thepatch = (*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));
	ptk_stat->patchnum = thepatch;
}

void N64_PatchMod(track_status *ptk_stat) // 8003841C
{
    //PRINTF_D2(WHITE,0,7,"N64_PatchMod");
}

void N64_PitchMod(track_status *ptk_stat) // 80038424
{
	static unsigned long	vn;				//800B69C4
	static unsigned long	vi;				//800B69C8
	static voice_status		*pvs;			//800B69CC
	static short			thepitchmod;	//800B69D0
	static f32				pitch;			//800B69D4
	static unsigned long	adjpitch;		//800B69D8

	//PRINTF_D2(WHITE,0,7,"N64_PitchMod");

	thepitchmod = (*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));
	if (ptk_stat->pitch_cntrl != thepitchmod)
	{
		ptk_stat->pitch_cntrl = thepitchmod;
		vn = ptk_stat->voices_active;

		if (vn != 0)
		{
			pvs = pvsbase;
			vi = nvss;

			while (vi--)
			{
				if ((pvs->flags & VOICE_ACTIVE) && (pvs->track == ptk_stat->refindx))
				{
                    //Pitch
                    if (ptk_stat->pitch_cntrl != 0)
                    {
                        if (ptk_stat->pitch_cntrl < 1)
                        {
                            adjpitch = (s32)((f64)(pvs->patchmaps->pitchstep_min * ptk_stat->pitch_cntrl) * (f64)0.0122);
                        }
                        else
                        {
                            adjpitch = (s32)((f64)(pvs->patchmaps->pitchstep_max * ptk_stat->pitch_cntrl) * (f64)0.0122);
                        }
                    }
                    else
                    {
                        adjpitch = 0;
                    }

                    pitch = (f32)WessCents2Ratio((pvs->patchinfo->pitch + adjpitch + (pvs->keynum - pvs->patchmaps->root_key) * 100) - pvs->patchmaps->fine_adj);
                    alSynSetPitch(&alGlobals->drvr, &voice[pvs->refindx], (f32)pitch);

                    if (!--vn) break;
				}

				pvs++;
			}
		}
	}
}

void N64_ZeroMod(track_status *ptk_stat) // 800386C8
{
    //PRINTF_D2(WHITE,0,7,"N64_ZeroMod");
}

void N64_ModuMod(track_status *ptk_stat) // 800386D0
{
    //PRINTF_D2(WHITE,0,7,"N64_ModuMod");
}

void N64_VolumeMod(track_status *ptk_stat) // 800386D8
{
	static unsigned long	vn;		//800B69DC
	static unsigned long	vi;		//800B69E0
	static unsigned long	adjvol;				//800B69E4
	static voice_status		*pvs;			//800B69E8
	static unsigned char	thevolmod;			//800B69EC

	u32 volume;
	ALMicroTime deltaTime;

	//PRINTF_D2(WHITE,0,7,"N64_VolumeMod");

	thevolmod = *(ptk_stat->ppos + 1);
	if (thevolmod != ptk_stat->volume_cntrl)
	{
		ptk_stat->volume_cntrl = thevolmod;
		vn = ptk_stat->voices_active;

		if (vn != 0)
		{
			pvs = pvsbase;
			vi = nvss;

			while (vi--)
			{
                if ((pvs->flags & VOICE_ACTIVE) && (pvs->track == ptk_stat->refindx))
				{
					if (ptk_stat->sndclass == SNDFX_CLASS)
					{
						volume = pvs->velnum * pvs->patchmaps->volume * ptk_stat->volume_cntrl * master_sfx_volume;
					}
					else//MUSIC_CLASS
					{
						volume = pvs->velnum * pvs->patchmaps->volume * ptk_stat->volume_cntrl * master_mus_volume;
					}

					adjvol = volume >> 0xd;
					deltaTime = 1000;

					alSynSetVol(&alGlobals->drvr, &voice[pvs->refindx], (s16)adjvol, (ALMicroTime)deltaTime);

					if (!--vn) break;
				}

				pvs++;
			}
		}
	}
}

void N64_PanMod(track_status *ptk_stat) // 800388FC
{
	static unsigned long	vn;			//800B69F0
	static unsigned long	vi;			//800B69F4
	static voice_status		*pvs;		//800B69F8
	static short			adjpan;		//800B69FC
	static unsigned char	thepanmod;	//800B69FE

	//PRINTF_D2(WHITE,0,7,"N64_PanMod");

	thepanmod = *(ptk_stat->ppos + 1);
	if (thepanmod != ptk_stat->pan_cntrl)
	{
		ptk_stat->pan_cntrl = thepanmod;

		if (pan_status != 0)
		{
			vn = ptk_stat->voices_active;

			if (vn != 0)
			{
				pvs = pvsbase;
				vi = nvss;

				while (vi--)
				{
					if ((pvs->flags & VOICE_ACTIVE) && (pvs->track == ptk_stat->refindx))
					{
						adjpan = (s32)(((ptk_stat->pan_cntrl + pvs->patchmaps->pan) - 0x40) << 16) >> 16;
						if (adjpan < 0)    adjpan = 0;
						if (adjpan > 0x7f)	adjpan = 0x7f;

						alSynSetPan(&alGlobals->drvr, &voice[pvs->refindx], (ALPan)adjpan);

						if (!--vn) break;
					}

					pvs++;
				}
			}
		}
	}
}

//N64_PedalMod
void N64_PedalMod(track_status *ptk_stat) // 80038AA0
{
    static unsigned char	vn;		//800B69FF
	static unsigned char	vi;		//800B6A00
	static voice_status		*pvs;	//800B6A04

	//PRINTF_D2(WHITE,0,7,"N64_PedalMod");

	if (*(ptk_stat->ppos + 1) == 0)
	{
		ptk_stat->flags |= TRK_OFF;

		vn = ptk_stat->voices_active;

		if (vn != 0)
		{
			pvs = pvsbase;
			vi = nvss;

			while (vi--)
			{
				if ((pvs->flags & VOICE_ACTIVE) && !(pvs->flags & VOICE_RELEASE) && (pvs->track == ptk_stat->refindx))
				{
					if (pvs->sndtype != Snd_Music)
					{
						pvs->sndtype = Snd_Music;
						N64_voicerelease(pvs);
						if (!--vn) break;
					}
				}

				pvs++;
			}
		}
	}
	else
	{
		ptk_stat->flags &= ~TRK_OFF;
	}
}

//N64_ReverbMod
void N64_ReverbMod(track_status *ptk_stat) // 80038BD8
{
    static unsigned char	vn;			    //800B6A08
	static unsigned char	vi;			    //800B6A0C
	static voice_status		*pvs;		    //800B6A10
	static unsigned char	thereverbmod;   //800B6A14

	//PRINTF_D2(WHITE,0,7,"N64_ReverbMod");

	thereverbmod = *(ptk_stat->ppos + 1);
	if (thereverbmod != ptk_stat->reverb)
	{
		ptk_stat->reverb = thereverbmod;
		vn = ptk_stat->voices_active;

		if (vn != 0)
		{
			pvs = pvsbase;
			vi = nvss;

			while (vi--)
			{
				if ((pvs->flags & VOICE_ACTIVE) && (pvs->track == ptk_stat->refindx))
				{
					alSynSetPan(&alGlobals->drvr, &voice[pvs->refindx], (ALPan)thereverbmod);

					if (!--vn) break;
				}

				pvs++;
			}
		}
	}
}

//N64_ChorusMod
void N64_ChorusMod(track_status *ptk_stat) // 80038D2C
{
    //PRINTF_D2(WHITE,0,7,"N64_ChorusMod");
}

void N64_voiceon(voice_status *voice_stat, track_status *ptk_stat,
	           patchmaps_header *patchmaps, patchinfo_header *patchinfo,
	           unsigned char keynum, unsigned char velnum) // 80038D34
{
	//PRINTF_D2(WHITE,0,17,"N64_voiceon");
	voice_stat->flags = (voice_stat->flags | VOICE_ACTIVE | VOICE_DECAY) & ~VOICE_RELEASE;
	voice_stat->track = ptk_stat->refindx;
	voice_stat->priority = patchmaps->priority;
	voice_stat->keynum = keynum;
	voice_stat->sndtype = Snd_Music;
	voice_stat->patchmaps = patchmaps;
	voice_stat->patchinfo = patchinfo;
	voice_stat->velnum = velnum;
	voice_stat->pabstime = *pcurabstime;

	ptk_stat->voices_active++;
	pmsbase->voices_active++;

	TriggerN64Voice(voice_stat);
}

void N64_voiceparmoff(voice_status *voice_stat) // 80038DE4
{
	static track_status	*ptrack;	//800B6A18
	//PRINTF_D2(WHITE,0,7,"N64_voiceparmoff");

	alSynStopVoice(&alGlobals->drvr, &voice[voice_stat->refindx]);
	alSynFreeVoice(&alGlobals->drvr, &voice[voice_stat->refindx]);

	ptrack = (ptsbase + voice_stat->track);
	pmsbase->voices_active--;

	if (!--ptrack->voices_active)
	{
		if ((ptrack->flags & TRK_STOPPED) && !(ptrack->flags & TRK_MUTE))
		{
			Eng_TrkOff(ptrack);
		}
	}

	voice_stat->flags &= ~(VOICE_ACTIVE | VOICE_RELEASE | VOICE_DECAY);
}

void N64_voicemuterelease(voice_status *voice_stat, int muterelease) // 80038EF8
{
	ALMicroTime deltaTime;

	//PRINTF_D2(WHITE,0,7,"N64_voicemuterelease");

	deltaTime = muterelease * 1000;
	alSynSetPriority(&alGlobals->drvr, &voice[voice_stat->refindx], 0); /* make candidate for stealing */
	alSynSetVol(&alGlobals->drvr, &voice[voice_stat->refindx], 0, deltaTime);

	voice_stat->flags = (voice_stat->flags | VOICE_RELEASE) & ~VOICE_DECAY;
	voice_stat->pabstime = *pcurabstime + muterelease;
}

void N64_voicerelease(voice_status *voice_stat) // 80038FBC
{
	ALMicroTime deltaTime;

	//PRINTF_D2(WHITE,0,7,"N64_voicerelease");

	deltaTime = voice_stat->patchmaps->release_time * 1000;
	alSynSetPriority(&alGlobals->drvr, &voice[voice_stat->refindx], 0); /* make candidate for stealing */
	alSynSetVol(&alGlobals->drvr, &voice[voice_stat->refindx], 0, deltaTime);

	voice_stat->flags = (voice_stat->flags | VOICE_RELEASE) & ~VOICE_DECAY;
	voice_stat->pabstime = *pcurabstime + (unsigned long)voice_stat->patchmaps->release_time;
}

void N64_voicedecay(voice_status *voice_stat) // 80039084
{
	static unsigned long	adjvol;	//800B6A1C
	static track_status		*pts;	//800B6A20

	u32 volume;
	ALMicroTime deltaTime;

	//PRINTF_D2(WHITE,0,7,"N64_voicedecay");

	pts = (ptsbase + voice_stat->track);

	//sndclass
	if (pts->sndclass == SNDFX_CLASS)
	{
		volume = voice_stat->velnum * voice_stat->patchmaps->volume * pts->volume_cntrl * master_sfx_volume;
	}
	else
	{
		volume = voice_stat->velnum * voice_stat->patchmaps->volume * pts->volume_cntrl * master_mus_volume;
	}

	adjvol = voice_stat->patchmaps->decay_level * (volume >> 0xd) >> 7;

	if (enabledecay)
	{
		deltaTime = voice_stat->patchmaps->decay_time * 1000;
		alSynSetVol(&alGlobals->drvr, &voice[voice_stat->refindx], (s16)adjvol, deltaTime);
	}

	voice_stat->flags &= ~VOICE_DECAY;
}

void N64_voicenote(track_status *ptk_stat,
	           patchmaps_header *patchmap, patchinfo_header *patchinfo,
	           unsigned char keynum, unsigned char velnum) // 800391F8
{
	static unsigned long	vi;				//800B6A24
	static unsigned long	found_one;		//800B6A28
	static voice_status		*pvs;			//800B6A2C
	static voice_status		*pvsact;		//800B6A30
	static unsigned long	lowprio;		//800B6A34
	static unsigned long	lowtime;		//800B6A38


	unsigned int pabstime_tmp;
	unsigned int priority_tmp;
	voice_status *voice_tmp;

	//PRINTF_D2(WHITE,0,15,"N64_voicenote");

	found_one = 0;

	pvsact = voice_tmp = (voice_status *)0x0;
	lowprio = priority_tmp = 0x100;
	lowtime = pabstime_tmp = 0xFFFFFFFF;

	pvs = pvsbase;
	vi = nvss;

	while (vi--)
	{
		if (!(pvs->flags & VOICE_ACTIVE))
		{
			pvsact = voice_tmp;
			lowprio = priority_tmp;
			lowtime = pabstime_tmp;
			N64_voiceon(pvs, ptk_stat, patchmap, patchinfo, keynum, velnum);
			found_one = 0;
			break;
		}

		if (pvs->priority <= patchmap->priority)
		{
			if (pvs->priority < lowprio)
			{
				found_one = 1;
                pabstime_tmp = pvs->pabstime;
                priority_tmp = pvs->priority;
                voice_tmp = pvs;
			}
			else
			{
			    if(pvs->flags & VOICE_RELEASE)
                {
                    if(pvsact->flags & VOICE_RELEASE)
                    {
                        if (pvs->pabstime < lowtime)
						{
							found_one = 1;
                            pabstime_tmp = pvs->pabstime;
                            priority_tmp = pvs->priority;
                            voice_tmp = pvs;
						}
                    }
                    else
                    {
                        found_one = 1;
                        pabstime_tmp = pvs->pabstime;
                        priority_tmp = pvs->priority;
                        voice_tmp = pvs;
                    }
                }
                else if(!(pvsact->flags & VOICE_RELEASE))
                {
                    if (pvs->pabstime < lowtime)
                    {
                        found_one = 1;
                        pabstime_tmp = pvs->pabstime;
                        priority_tmp = pvs->priority;
                        voice_tmp = pvs;
                    }
                }
			}
		}

		pvs++;
		pvsact = voice_tmp;
		lowprio = priority_tmp;
		lowtime = pabstime_tmp;
	}

	if (found_one != 0)
	{
		N64_voiceparmoff(pvsact);
		N64_voiceon(pvsact, ptk_stat, patchmap, patchinfo, keynum, velnum);
	}
}

void N64_NoteOn(track_status *ptk_stat) // 80039434
{
	static unsigned long	i;			//800B6A3C
	static unsigned char	note;		//800B6A40
	static unsigned char	vel;		//800B6A41
	static unsigned char	mapcount;	//800B6A42
	static patches_header	*samp_pat;	//800B6A44
	static patchmaps_header	*mapdat;	//800B6A48
	static patchinfo_header	*sampdat;	//800B6A4C

	//PRINTF_D2(WHITE,0,16,"N64_NoteOn");

	note = *(ptk_stat->ppos + 1);
	vel = *(ptk_stat->ppos + 2);

	samp_pat = patchesbase + ptk_stat->patchnum;
	mapcount = samp_pat->patchmap_cnt;

	//PRINTF_D2(WHITE,15,14,"note %d",note);
	//PRINTF_D2(WHITE,15,15,"vel %d",vel);
	//PRINTF_D2(WHITE,15,16,"mapcount %d",mapcount);

	i = 0;
	while (mapcount--)
	{
		mapdat = patchmapsbase + (samp_pat->patchmap_idx + i);
		sampdat = samplesbase + mapdat->sample_id;

		if ((mapdat->note_min <= note) && (note <= mapdat->note_max))
		{
			N64_voicenote(ptk_stat, mapdat, sampdat, note, vel);
		}
		i += 1;
	}
}

void N64_NoteOff(track_status *ptk_stat) // 800395B4
{
	static unsigned long	vi;		//800B6A50
	static voice_status		*pvs;	//800B6A54

	//PRINTF_D2(WHITE,0,7,"N64_NoteOff");
	pvs = pvsbase;
	vi = nvss;

	while (vi--)
	{
		if (((pvs->flags & (VOICE_ACTIVE | VOICE_RELEASE)) == VOICE_ACTIVE) &&
			(pvs->keynum == *(ptk_stat->ppos + 1)) &&
			(pvs->track == ptk_stat->refindx))
		{
			if ((ptk_stat->flags & TRK_OFF) == 0) {
				pvs->sndtype = Snd_Sfx;
			}
			else {
				N64_voicerelease(pvs);
			}
		}

		pvs++;
	}
}

#endif
