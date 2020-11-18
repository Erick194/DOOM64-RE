#ifndef _WESSSEQ_H
#define _WESSSEQ_H

#include <ultra64.h>
#include <libaudio.h>
#include "wessapi.h"
#include "wessarc.h"

extern unsigned char	master_sfx_volume;
extern unsigned char	master_mus_volume;
extern unsigned char	pan_status;
extern int	            enabledecay;

extern char *Read_Vlq(char *pstart, void *deltatime);
extern char *Write_Vlq(char *dest, unsigned int value);
extern int Len_Vlq(unsigned int value);

extern int SeqOn;
extern void(*drv_cmds[19])(track_status *);

extern void add_music_mute_note(track_status *ptk_stat,
                         unsigned short seq_num,  unsigned short track,
                         unsigned char keynum,  unsigned char velnum,
                         patchmaps_header *patchmap,
                         patchinfo_header *patchinfo);

extern void Eng_TrkOff (track_status *ptk_stat);

extern void(*DrvFunctions[36])(track_status *);
extern void SeqEngine(void);

#endif // _WESSSEQ_H
