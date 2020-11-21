#ifndef _WESSARC_H
#define _WESSARC_H

#include <ultra64.h>
#include <libaudio.h>

//#define NOUSEWESSCODE

enum Wess_Error {

	wess_NO_ERROR,
	wess_FOPEN,
	wess_FREAD,
	wess_FSEEK
};

//LCD FILE Structs
typedef struct
{
	unsigned char	patchmap_cnt;	//r*
	unsigned char	pad1;	        //r*1
	unsigned short	patchmap_idx;	//r*2
}patches_header;

typedef struct
{
	unsigned char priority;			//*0
	unsigned char volume;			//*1----*2
	unsigned char pan;				//*2----*3
	unsigned char reverb;			//*3----*1
	unsigned char root_key;			//*4----*4
	unsigned char fine_adj;			//*5----*5
	unsigned char note_min;			//*6----*6
	unsigned char note_max;			//*7----*7
	unsigned char pitchstep_min;	//*8----*8
	unsigned char pitchstep_max;	//*9----*9
	unsigned short sample_id;		//*10 swap
	unsigned short attack_time;		//*12 swap
	unsigned short decay_time;		//*14 swap
	unsigned short release_time;	//*16 swap
	unsigned char attack_level;		//*18
	unsigned char decay_level;		//*19
}patchmaps_header;

typedef struct {
    s32 order;
    s32 npredictors;
    s16 book[128];        /* Actually variable size. Must be 8-byte aligned */
} ALADPCMBook2;

typedef struct {
    u32         start;
    u32         end;
    u32         count;
    ADPCM_STATE state;
    u32         pad;
} ALADPCMloop2;

typedef struct {
    u32         start;
    u32         end;
    u32         count;
    u32         pad;
} ALRawLoop2;

typedef struct {
    ALADPCMloop2 *loop;
    ALADPCMBook2 *book;
} ALADPCMWaveInfo2;

typedef struct
{
    u8      *base;  /* ptr to start of wave data    */	//r*0
    s32     len;    /* length of data in bytes      */	//r*4
    u8      type;   /* compression type             */	//r*8
    u8      flags;  /* offset/address flags         */	//r*9
    void    *loop;  //r*12
    void    *book;  //r*14
} ALWaveTable2;

typedef struct
{
	//u8          *base;          /* ptr to start of wave data    */	//r*0
	//s32         len;            /* length of data in bytes      */	//r*4
	//u8          type;           /* compression type             */	//r*8
	//u8          flags;          /* offset/address flags         */	//r*9
	//union {
	//	ALADPCMWaveInfo2 adpcmWave;//12
	//	ALRAWWaveInfo   rawWave;//12
	//} waveInfo;//12

	ALWaveTable2 wave;
	s32         pitch;//20
} patchinfo_header; //ALWaveTable custom

typedef struct
{
	unsigned short            nsfx1;			//0 sfx count (124)
	unsigned short            rawcount;			//2
	unsigned short            adpcmcount;		//4 loop data count (23)
	unsigned short            nsfx2;			//6 sfx count again?? (124)
} loopinfo_header;

//*******************************

typedef struct
{
	int				module_id_text;		//*
	unsigned int	module_version;		//*4
	unsigned int	pad1;				//*8
	unsigned short	pad2;				//*12
	unsigned short	sequences;			//*14
	unsigned char	decomp_type;		//*16
	unsigned char	pad3;				//*17
	unsigned char	pad4;				//*18
	unsigned char	pad5;				//*19
	unsigned int	compress_size;		//*20
	unsigned int	data_size;			//*24
	unsigned int	pad6;				//*28
}module_header;//32

typedef struct
{
	unsigned char	voices_type;		//*
	unsigned char	reverb;				//*1
	unsigned short	initpatchnum;		//*2
	unsigned short	initpitch_cntrl;	//*4
	unsigned char	initvolume_cntrl;	//*6
	unsigned char	initpan_cntrl;		//*7
	unsigned char	substack_count;		//*8
	unsigned char	mutebits;			//*9
	unsigned short	initppq;			//*10
	unsigned short	initqpm;			//*12
	unsigned short	labellist_count;	//*14
	unsigned int	data_size;			//*16
}track_header;//size 20 bytes

typedef struct
{
	track_header	*trk_hdr;//*
	unsigned long	*plabellist;//*4
	char			*ptrk_data;//*8
}track_data;

typedef struct
{
	unsigned short	tracks;			//*
	unsigned short	decomp_type;	//*2
	unsigned int	trkinfolength;	//*4
	unsigned int	fileposition;	//*8
}seq_header;

typedef struct
{
	seq_header		seq_hdr;		//*
	track_data		*ptrk_info;		//*12
}sequence_data;

typedef struct
{
	module_header	mod_hdr;			//*
	sequence_data	*pseq_info;			//*32
}module_data;

typedef void (*callfunc_t)(char, short);
typedef struct
{
	unsigned char	active;				//*
	unsigned char	type;				//*1
	unsigned short	curval;				//*2
	callfunc_t      callfunc;			//*4
}callback_status;

typedef struct
{
	unsigned int	load_flags;			//*
	unsigned short	patches;			//*4
	unsigned short	patch_size;			//*6
	unsigned short	patchmaps;			//*8
	unsigned short	patchmap_size;		//*10
	unsigned short	patchinfo;			//*12
	unsigned short	patchinfo_size;		//*14
	unsigned short	drummaps;			//*16
	unsigned short	drummap_size;		//*18
	unsigned int	extra_data_size;	//*20
}patch_group_header;

typedef struct
{
	int hardware_ID;//76
	int flags_load;//80
}hardware_table_list;

typedef struct
{
	patch_group_header pat_grp_hdr;		//*
	char				*ppat_data;		//*24
}patch_group_data;//size 84 bytes

#define DriverInit      0
#define DriverExit      1
#define DriverEntry1    2
#define DriverEntry2    3
#define DriverEntry3    4
#define TrkOff          5
#define TrkMute         6
#define PatchChg        7
#define PatchMod        8
#define PitchMod        9
#define ZeroMod         10
#define ModuMod         11
#define VolumeMod       12
#define PanMod          13
#define PedalMod        14
#define ReverbMod       15
#define ChorusMod       16
#define NoteOn          17
#define NoteOff         18

// Engine Only
#define StatusMark      19
#define GateJump        20
#define IterJump        21
#define ResetGates      22
#define ResetIters      23
#define WriteIterBox    24
#define SeqTempo        25
#define SeqGosub        26
#define SeqJump         27
#define SeqRet          28
#define SeqEnd          29
#define TrkTempo        30
#define TrkGosub        31
#define TrkJump         32
#define TrkRet          33
#define TrkEnd          34
#define NullEvent       35

#define SEQ_STATE_STOPPED 0
#define SEQ_STATE_PLAYING 1

//(val << 0 < 0) 128
//(val << 1 < 0) 64
//(val << 2 < 0) 32
//(val << 3 < 0) 16
//(val << 4 < 0) 8
//(val << 5 < 0) 4
//(val << 6 < 0) 2
//(val << 7 < 0) 1

//New track
#define TRK_OFF		1
#define TRK_SKIP	2
#define TRK_LOOPED	4
#define TRK_TIMED	8
#define TRK_STOPPED	16
#define TRK_HANDLED	32
#define TRK_MUTE	64
#define TRK_ACTIVE	128

//New seq
#define SEQ_STOP	8
#define SEQ_RESTART	16
#define SEQ_PAUSE	32
#define SEQ_HANDLE	64
#define SEQ_ACTIVE	128

//New voice
#define VOICE_DECAY		32
#define VOICE_RELEASE	64
#define VOICE_ACTIVE	128

typedef struct
{
	unsigned char	flags;			//*
	unsigned char	playmode;		//*1
	unsigned short	seq_num;		//*2
	unsigned char	tracks_active;	//*4
	unsigned char	tracks_playing;	//*5
	unsigned char	volume;			//*6
	unsigned char	pan;			//*7
	unsigned int	seq_type;		//*8
	char			*ptrk_indxs;	//*12---
	char			*pgates;		//*16---
	char			*piters;		//*20
}sequence_status;

typedef struct
{
	unsigned char	flags;			//*
	unsigned char	refindx;		//*1
	unsigned char	seq_owner;		//*2
	unsigned char	sndclass;		//*3
	unsigned short	patchnum;		//*4
	short			pitch_cntrl;	//*6
	unsigned int	deltatime;		//*8
	unsigned char	reverb;			//*12
	unsigned char	volume_cntrl;	//*13
	unsigned char	pan_cntrl;		//*14
	unsigned char	mutemask;		//*15
	unsigned char	patchtype;		//*16
	unsigned char	voices_active;	//*17
	unsigned char	voices_max;		//*18
	unsigned char	priority;		//*19
	unsigned short	ppq;			//*20
	unsigned short	qpm;			//*22
	unsigned short	labellist_count;//*24
	unsigned short	labellist_max;	//*26
	unsigned long   ppi;			//*28
	unsigned long	starppi;		//*32
	unsigned int	accppi;			//*36
	unsigned int	totppi;			//*40
	unsigned int	endppi;			//*44
	unsigned char	*pstart;		//*48
	unsigned char	*ppos;			//*52
	unsigned long	*plabellist;	//*56
	unsigned long	*psubstack;		//*60
	unsigned char	*psp;			//*64
	unsigned long	*pstackend;		//*68
	unsigned int	data_size;		//*72
	unsigned int	data_space;		//*76
}track_status;

typedef struct
{
	unsigned char		flags;			//*
	unsigned char		patchtype;		//*1
	unsigned char		refindx;		//*2
	unsigned char		track;			//*3
	unsigned char		priority;		//*4
	unsigned char		keynum;			//*5
	unsigned char		velnum;			//*6
	unsigned char		sndtype;		//*7
	patchmaps_header 	*patchmaps;		//*8
	patchinfo_header 	*patchinfo;		//*12 /* offset to wavetable struct           */
	unsigned long		pabstime;		//*16
}voice_status;//size 24 bytes

typedef struct
{
	unsigned long		*pabstime;				//*---
	unsigned char		seqs_active;			//*4
	unsigned char		trks_active;			//*5
	unsigned char		voices_active;			//*6
	unsigned char		voices_total;			//*7
	unsigned char		patch_types_loaded;		//*8
	unsigned char		callbacks_active;		//*9
	unsigned char		max_trks_perseq;		//*10
	unsigned char		max_substack_pertrk;	//*11----
	module_data			*pmod_info;				//*12---
	callback_status		*pcalltable;			//*16---
	patch_group_data	*ppat_info;				//*20-----
	sequence_status		*pseqstattbl;			//*24-----
	track_status		*ptrkstattbl;			//*28-----
	voice_status		*pvoicestattbl;			//*32-----
}master_status_structure;

typedef master_status_structure pmasterstat;

#define WESS_SSSP_TEXT		0x534E3634//'SN64'
#define WESS_CORE_VERSION	0x02

#define SNDFX_CLASS		0
#define MUSIC_CLASS		1
#define DRUMS_CLASS		2
#define SFXDRUMS_CLASS	3

#define TAG_SOUND_EFFECTS 1
#define TAG_MUSIC         2
#define TAG_DRUMS         4

#define LOAD_PATCHES	1
#define LOAD_PATCHMAPS	2
#define LOAD_PATCHINFO	4
#define LOAD_DRUMMAPS	8
#define LOAD_EXTRADATA	16

///----------------------------------
extern void (**CmdFuncArr[10])(track_status *);

extern int wess_driver_num_dma_buffers;
extern int wess_driver_num_dma_messages;
extern int wess_driver_dma_buffer_length;
extern int wess_driver_extra_samples;
extern int wess_driver_frame_lag;
extern int wess_driver_voices;
extern int wess_driver_updates;
extern int wess_driver_sequences;
extern int wess_driver_tracks;
extern int wess_driver_gates;
extern int wess_driver_iters;
extern int wess_driver_callbacks;
extern int wess_driver_max_trks_per_seq;
extern int wess_driver_max_subs_per_trk;
extern int enabledecay;

extern int SeqOn;
extern unsigned long millicount;
extern int WessTimerActive;


#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

typedef struct
{
	char *start;		//L800B4200
	char *src;			//L800B4204
} N64_File_IO_Struct;

typedef N64_File_IO_Struct Wess_File_IO_Struct;
typedef N64_File_IO_Struct Wess_Data_IO_Struct;

extern Wess_File_IO_Struct module_fileref;//800B4200
extern Wess_Data_IO_Struct data_fileref;//L800B4208

extern void wess_engine_off(void);//L80035340()
extern void wess_engine_on(void);//L8003534C()

extern void wess_low_level_init(void);//L8003531C()
extern void wess_low_level_exit(void);//L80035324()

extern short GetIntsPerSec(void);
extern unsigned long CalcPartsPerInt(short ips,short ppq,short qpm);

extern void init_WessTimer(void);
extern void exit_WessTimer(void);

extern void wess_low_level_exit(void);
extern char *wess_malloc(char *mem);
extern void wess_free(char *mem);

extern Wess_File_IO_Struct *module_open(char *filename);
extern int module_read(void *destptr, int readbytes, Wess_File_IO_Struct *fileptr);
extern int module_seek(Wess_File_IO_Struct *fileptr, int seekpos, int seekmode);
extern unsigned long module_tell(Wess_File_IO_Struct *fileptr);
extern void module_close(Wess_File_IO_Struct *fileptr);

extern int wess_decomp(unsigned char decomp_type, char *fileref, unsigned long file_offset, char *ramdest, unsigned long uncompressed_size);
extern void wess_enable(void);
extern void wess_disable(void);


extern long WessInterruptHandler(void);

typedef void(*WessAction)(track_status *ptmp, int value);

#define QUEUE_SEQ_STOPALL					0
#define QUEUE_SEQ_STOP						1
#define QUEUE_MASTER_SFX_VOL_SET			2
#define QUEUE_MASTER_MUS_VOL_SET			3
#define QUEUE_HANDLE_PARM_MOD				4
#define QUEUE_HANDLE_NOTEON					5
#define QUEUE_SEQ_PAUSE						6
#define QUEUE_SEQ_RESTART					7
#define QUEUE_SEQ_PAUSEALL					8
#define QUEUE_SEQ_RESTARTALL				9
#define QUEUE_SEQ_STOPTYPE					10
#define QUEUE_SEQ_UPDATE_TYPE_SPECIAL		11
#define QUEUE_SEQ_STOPALL_AND_VOICERAMP		12
#define QUEUE_SEQ_STOP_AND_VOICERAMP		13
#define QUEUE_SEQ_STOPTYPE_AND_VOICERAMP	14

#endif // _WESSARC_H
