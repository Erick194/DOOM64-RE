// Reconstruido por Erick Vasquez Garcia 16/05/2019 [GEC]
// Update_1 20/10/2019 [GEC]
// Update_2 31/01/2020 [GEC]

#include "wessseq.h"

#include "graph.h"
#ifndef NOUSEWESSCODE
extern void (**CmdFuncArr[10])(track_status *);
extern void(*DrvFunctions[36])(track_status *);

char *Read_Vlq(char *pstart, void *deltatime);
char *Write_Vlq(char *dest, unsigned int value);
int Len_Vlq(unsigned int value);

void Eng_DriverInit (track_status *ptk_stat);//(master_status_structure *pm_stat);
void Eng_DriverExit (track_status *ptk_stat);
void Eng_DriverEntry1 (track_status *ptk_stat);
void Eng_DriverEntry2 (track_status *ptk_stat);
void Eng_DriverEntry3 (track_status *ptk_stat);
void Eng_TrkOff (track_status *ptk_stat);
void Eng_TrkMute (track_status *ptk_stat);
void Eng_PatchChg (track_status *ptk_stat);
void Eng_PatchMod (track_status *ptk_stat);
void Eng_PitchMod (track_status *ptk_stat);
void Eng_ZeroMod (track_status *ptk_stat);
void Eng_ModuMod (track_status *ptk_stat);
void Eng_VolumeMod (track_status *ptk_stat);
void Eng_PanMod (track_status *ptk_stat);
void Eng_PedalMod (track_status *ptk_stat);
void Eng_ReverbMod (track_status *ptk_stat);
void Eng_ChorusMod (track_status *ptk_stat);
void Eng_NoteOn (track_status *ptk_stat);
void Eng_NoteOff (track_status *ptk_stat);
void Eng_StatusMark (track_status *ptk_stat);
void Eng_GateJump (track_status *ptk_stat);
void Eng_IterJump (track_status *ptk_stat);
void Eng_ResetGates (track_status *ptk_stat);
void Eng_ResetIters (track_status *ptk_stat);
void Eng_WriteIterBox (track_status *ptk_stat);
void Eng_SeqTempo (track_status *ptk_stat);
void Eng_SeqGosub (track_status *ptk_stat);
void Eng_SeqJump (track_status *ptk_stat);
void Eng_SeqRet (track_status *ptk_stat);
void Eng_SeqEnd (track_status *ptk_stat);
void Eng_TrkTempo (track_status *ptk_stat);
void Eng_TrkGosub (track_status *ptk_stat);
void Eng_TrkJump (track_status *ptk_stat);
void Eng_TrkRet (track_status *ptk_stat);
void Eng_TrkEnd (track_status *ptk_stat);
void Eng_NullEvent (track_status *ptk_stat);

void(*DrvFunctions[36])(track_status *) =
{
	Eng_DriverInit,
	Eng_DriverExit,
	Eng_DriverEntry1,
	Eng_DriverEntry2,
	Eng_DriverEntry3,
	Eng_TrkOff,
	Eng_TrkMute,
	Eng_PatchChg,
	Eng_PatchMod,
	Eng_PitchMod,
	Eng_ZeroMod,
	Eng_ModuMod,
	Eng_VolumeMod,
	Eng_PanMod,
	Eng_PedalMod,
	Eng_ReverbMod,
	Eng_ChorusMod,
	Eng_NoteOn,
	Eng_NoteOff,
	Eng_StatusMark,     //0x13
	Eng_GateJump,       //0x14
	Eng_IterJump,       //0x15
	Eng_ResetGates,     //0x16
	Eng_ResetIters,     //0x17
	Eng_WriteIterBox,   //0x18
	Eng_SeqTempo,       //0x19
	Eng_SeqGosub,       //0x1A
	Eng_SeqJump,        //0x1B
	Eng_SeqRet,         //0x1C
	//SeqFunctions ??
	Eng_SeqEnd,         //0x1D
	Eng_TrkTempo,       //0x1E
	Eng_TrkGosub,       //0x1F
	Eng_TrkJump,        //0x20
	Eng_TrkRet,         //0x21
	Eng_TrkEnd,         //0x22
	Eng_NullEvent       //0x23
};

unsigned char CmdLength[36] = { // 8005D9F0
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
0x02, 0x03, 0x02, 0x04, 0x05, 0x05, 0x02, 0x02,
0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x03, 0x03,
0x03, 0x01, 0x01, 0x01
};

unsigned char CmdSort[44] = { // 8005DA14
0x00, 0x01, 0x02, 0x03, 0x04, 0x07, 0x06, 0x16,
0x17, 0x18, 0x15, 0x1A, 0x1E, 0x19, 0x1B, 0x1D,
0x1C, 0x1F, 0x05, 0x0F, 0x08, 0x09, 0x12, 0x11,
0x10, 0x14, 0x0D, 0x0B, 0x22, 0x23, 0x13, 0x0C,
0x0A, 0x20, 0x21, 0x0E, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00
};

//-----------------------------------------------------------
// Vlq System
//-----------------------------------------------------------

//extern unsigned int data01;    //L8007F024
//extern unsigned char data02;    //L8007F028

char *Read_Vlq(char *pstart, void *deltatime)//L80035F80()
{
	static int v;//800B6630
	static char c;//800B6634

	v = *pstart++;
	if (v & 0x80)
	{
		v &= 0x7f;
		do
		{
			c = *pstart++;
			v = (c & 0x7f) + (v << 7);
		} while (c & 0x80);
	}
	*(int*)deltatime = v;

	return pstart;
}

char *Write_Vlq(char *dest, unsigned int value)//L8004744C()
{
	char data[10];
	char *ptr;

	data[0] = (value & 0x7f);
	ptr = (char*)&data + 1;

	value >>= 7;
	if (value)
	{
		do
		{
			*ptr++ = ((value & 0x7f) | 0x80);
			value >>= 7;
		} while (value);
	}

	do
	{
		ptr--;
		*dest++ = *ptr;
	} while (*ptr & 0x80);

	return dest;
}

int Len_Vlq(unsigned int value)//L800474AC()
{
    char buff[16];
	char data[10];
	char *ptr;
	char *dest, *start;

	dest = start = (char*)&buff;

	data[0] = (value & 0x7f);
	ptr = (char*)&data + 1;

	value >>= 7;
	if (value)
	{
		do
		{
			*ptr++ = ((value & 0x7f) | 0x80);
			value >>= 7;
		} while (value);
	}

	do
	{
		ptr--;
		*dest++ = *ptr;
	} while (*ptr & 0x80);

	return (int)(dest - start);
}
//-----------------------------------------------------------
// Engine System
//-----------------------------------------------------------

static unsigned char			ntwa = 0;		//8005DAD4
static track_status				*ptsbase = 0;	//8005DAD8
static sequence_status			*pssbase = 0;	//8005DADC
static master_status_structure	*pmsbase = 0;	//8005DAE0

void Eng_DriverInit (track_status *ptk_stat) // 800360C4
{
	master_status_structure *pm_stat;

	pm_stat = (master_status_structure*)ptk_stat;

	//PRINTF_D2(WHITE,0,10,"Eng_DriverInit");
    ntwa = wess_driver_tracks;
	ptsbase = pm_stat->ptrkstattbl;
	pssbase = pm_stat->pseqstattbl;
	pmsbase = pm_stat;

	//PRINTF_D(WHITE,"ntwa %d\n", ntwa);
	//PRINTF_D(WHITE,"ptsbase %x\n", (int)&ptsbase);
	//PRINTF_D(WHITE,"pssbase %x\n", (int)&pssbase);
	//PRINTF_D(WHITE,"pmsbase %x\n", (int)&pmsbase);
}

void Eng_DriverExit (track_status *ptk_stat) // 800360FC
{
    //PRINTF_D2(WHITE,0,10,"Eng_DriverExit");
}

void Eng_DriverEntry1 (track_status *ptk_stat) // 80036104
{
    //PRINTF_D2(WHITE,0,10,"Eng_DriverEntry1");
}

void Eng_DriverEntry2 (track_status *ptk_stat) // 8003610C
{
    //PRINTF_D2(WHITE,0,10,"Eng_DriverEntry2");
}

void Eng_DriverEntry3 (track_status *ptk_stat) // 80036114
{
    //PRINTF_D2(WHITE,0,10,"Eng_DriverEntry3");
}

void Eng_TrkOff (track_status *ptk_stat) // 8003611C
{
	static sequence_status	*lpseq;	//800B6638
	static char				*lpdest;//800B663C
	static unsigned long	lj;		//800B6640

	//PRINTF_D2(WHITE,0,10,"Eng_TrkOff");

	lpseq = (pmsbase->pseqstattbl + ptk_stat->seq_owner);

	if (!(ptk_stat->flags & TRK_STOPPED))
	{
		ptk_stat->flags |= TRK_STOPPED;

		if (!--lpseq->tracks_playing)
		{
			lpseq->playmode = SEQ_STATE_STOPPED;
		}
	}

	if (!(ptk_stat->flags & TRK_HANDLED))
	{
		lj = pmsbase->max_trks_perseq;
		lpdest = lpseq->ptrk_indxs;

		while (lj--)
		{
			if (ptk_stat->refindx == *lpdest)
			{
				*lpdest++ = 0xff;
				break;
			}
			lpdest++;
		}

		ptk_stat->flags &= ~TRK_ACTIVE;
		pmsbase->trks_active--;

		if (!--lpseq->tracks_active)
		{
			lpseq->flags &= ~SEQ_ACTIVE;
			pmsbase->seqs_active--;
		}
	}

	ptk_stat->flags &= ~TRK_TIMED;
}

void Eng_TrkMute (track_status *ptk_stat) // 80036298
{
    //PRINTF_D2(WHITE,0,10,"Eng_TrkMute");
}

void Eng_PatchChg (track_status *ptk_stat) // 800362A0
{
	static unsigned char thepatch;//800B6644

	//PRINTF_D2(WHITE,0,10,"Eng_PatchChg");

	thepatch = (*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));
	ptk_stat->patchnum = (unsigned short)thepatch;
}


void Eng_PatchMod (track_status *ptk_stat) // 800362C8
{
    //PRINTF_D2(WHITE,0,10,"Eng_PatchMod");
}

void Eng_PitchMod (track_status *ptk_stat) // 800362D0
{
    static short thepitchmod;//800B6646

	//PRINTF_D2(WHITE,0,10,"Eng_PitchMod");

	thepitchmod = (*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));
	ptk_stat->pitch_cntrl = thepitchmod;
}

void Eng_ZeroMod (track_status *ptk_stat) // 800362F4
{
    //PRINTF_D2(WHITE,0,10,"Eng_ZeroMod");
}

void Eng_ModuMod (track_status *ptk_stat) // 800362FC
{
    //PRINTF_D2(WHITE,0,10,"Eng_ModuMod");
}

void Eng_VolumeMod (track_status *ptk_stat) // 80036304
{
    static unsigned char thevolume; //800B6648

	//PRINTF_D2(WHITE,0,10,"Eng_VolumeMod");

	thevolume = *(ptk_stat->ppos + 1);
	ptk_stat->volume_cntrl = thevolume;
}

void Eng_PanMod (track_status *ptk_stat) // 80036320
{
    static unsigned char thepan; //0x800B6649

	//PRINTF_D2(WHITE,0,10,"Eng_PanMod");
	thepan = *(ptk_stat->ppos + 1);
	ptk_stat->pan_cntrl = thepan;
}

void Eng_PedalMod (track_status *ptk_stat) // 8003633C
{
    //PRINTF_D2(WHITE,0,10,"Eng_PedalMod");
}

void Eng_ReverbMod (track_status *ptk_stat) // 80036344
{
    //PRINTF_D2(WHITE,0,10,"Eng_ReverbMod");
}

void Eng_ChorusMod (track_status *ptk_stat) // 8003634C
{
    //PRINTF_D2(WHITE,0,10,"Eng_ChorusMod");
}

void Eng_NoteOn (track_status *ptk_stat) // 80036354
{
    //PRINTF_D2(WHITE,0,10,"Eng_NoteOn");
}

void Eng_NoteOff (track_status *ptk_stat) // 8003635C
{
    //PRINTF_D2(WHITE,0,10,"Eng_NoteOff");
}

void Eng_StatusMark (track_status *ptk_stat) // 80036364
{
	static char si, sn;				//800B664A, 800B664B
	static callback_status *cbs;	//800B664C

	//PRINTF_D2(WHITE,0,10,"Eng_StatusMark");

	sn = pmsbase->callbacks_active;
	if (sn)
	{
		cbs = pmsbase->pcalltable;
		si = wess_driver_callbacks;
		if(wess_driver_callbacks)
		{
			while (si--)
			{
				if (cbs->active)
				{
					if (cbs->type == *(ptk_stat->ppos + 1))
					{
						cbs->curval = (*(ptk_stat->ppos + 2) | (*(ptk_stat->ppos + 3) << 8));
						cbs->callfunc(cbs->type, cbs->curval);
						break;
					}

					if (!--sn) break;
				}

				cbs++;
			}
		}
	}
}

void Eng_GateJump (track_status *ptk_stat) // 80036488
{
	static short lindex;	//800B6650
	static char *laboff;	//800B6654
	static char *pgate;		//800B6658

	//PRINTF_D2(WHITE,0,10,"Eng_GateJump");
	pgate = (pssbase + ptk_stat->seq_owner)->pgates + *(ptk_stat->ppos + 1);

	if (*pgate != 0)
	{
		if (*pgate == 0xff)
		{
			*pgate = *(ptk_stat->ppos + 2);
		}

		lindex = (*(ptk_stat->ppos + 3) | (*(ptk_stat->ppos + 4) << 8));

		if (lindex >= 0)
		{
			if (lindex < ptk_stat->labellist_count)
			{
				laboff = ptk_stat->pstart + *(ptk_stat->plabellist + lindex);
				ptk_stat->ppos = laboff;
				ptk_stat->deltatime = 0;
				ptk_stat->flags |= TRK_SKIP;
			}
		}
	}
}

void Eng_IterJump (track_status *ptk_stat) // 80036568
{
	static short lindex;	//800B665C
	static char	*laboff;	//800B6660
	static char	*piter;		//800B6664

	//PRINTF_D2(WHITE,0,10,"Eng_IterJump");
	piter = (pssbase + ptk_stat->seq_owner)->piters + *(ptk_stat->ppos + 1);

	if (*piter != 0)
	{
		if (*piter == 0xFF)
		{
			*piter = *(ptk_stat->ppos + 2);
		}
		else
		{
			*piter = *piter + 0xFF;
		}

		lindex = (*(ptk_stat->ppos + 3) | (*(ptk_stat->ppos + 4) << 8));

		if (lindex >= 0)
		{
			if (lindex < ptk_stat->labellist_count)
			{
				laboff = ptk_stat->pstart + *(ptk_stat->plabellist + lindex);
				ptk_stat->ppos = laboff;
				ptk_stat->deltatime = 0;
				ptk_stat->flags |= TRK_SKIP;
			}
		}
	}
}

void Eng_ResetGates (track_status *ptk_stat) // 80036654
{
	static unsigned char	gi;		//800B6668
	static char				*pgate;	//800B666C

	//PRINTF_D2(WHITE,0,10,"Eng_ResetGates");
	if (*(ptk_stat->ppos + 1) == 0xff)
	{
		gi = wess_driver_gates;
		pgate = (pssbase + ptk_stat->seq_owner)->pgates;

		while (gi--)
		{
			*pgate++ = 0xFF;
		}
	}
	else
	{
		pgate = (pssbase + ptk_stat->seq_owner)->pgates + *(ptk_stat->ppos + 1);
		*pgate = 0xff;
	}
}

void Eng_ResetIters (track_status *ptk_stat) // 80036720
{
	static unsigned char	ii;		//800B6670
	static char				*piter;	//800B6674

	//PRINTF_D2(WHITE,0,10,"Eng_ResetIters");
	if (*(ptk_stat->ppos + 1) == 0xff)
	{
		ii = wess_driver_iters;
		piter = (pssbase + ptk_stat->seq_owner)->piters;

		while (ii--)
		{
			*piter++ = 0xFF;
		}
	}
	else
	{
		piter = (pssbase + ptk_stat->seq_owner)->piters + *(ptk_stat->ppos + 1);
		*piter = 0xff;
	}
}

void Eng_WriteIterBox (track_status *ptk_stat) // 800367EC
{
	static char	*piter;//800B6678

	//PRINTF_D2(WHITE,0,10,"Eng_WriteIterBox");
	piter = (pssbase + ptk_stat->seq_owner)->piters + *(ptk_stat->ppos + 1);
	*piter = *(ptk_stat->ppos + 2);
}

void Eng_SeqTempo (track_status *ptk_stat) // 8003682C
{
	static unsigned short	ntracks;		//800B667C
	static unsigned char	nactive;		//800B667E
	static unsigned char	*ptindxs;					//800B6680
	static track_status		*ptstemp;		//800B6684
	static sequence_status	*psstemp;		//800B6688

	//PRINTF_D2(WHITE,0,10,"Eng_SeqTempo");

	psstemp = (pssbase + ptk_stat->seq_owner);
	ptindxs = (unsigned char *)psstemp->ptrk_indxs;
	nactive = psstemp->tracks_active;
	ntracks = (pmsbase->pmod_info->pseq_info + psstemp->seq_num)->seq_hdr.tracks;

	while (ntracks--)
	{
		if (*ptindxs != 0xff)
		{
			ptstemp = (ptsbase + (*ptindxs));
			ptstemp->qpm = (unsigned short)(*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));
			ptstemp->ppi = CalcPartsPerInt(GetIntsPerSec(), ptstemp->ppq, ptstemp->qpm);

			if (!--nactive) break;
		}
		ptindxs++;
	}
}

void Eng_SeqGosub (track_status *ptk_stat) // 800369C8
{
	static short			lindex;			//800B668C
	static unsigned short	ntracks;		//800B668E
	static char				*laboff;		//800B6690
	static unsigned char	nactive;		//800B6694
	static unsigned char	*ptindxs;		//800B6698
	static track_status		*ptstemp;		//800B669C
	static sequence_status	*psstemp;		//800B66A0

	//PRINTF_D2(WHITE,0,10,"Eng_SeqGosub");

	lindex = (*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));

	if ((lindex >= 0) && (lindex < ptk_stat->labellist_count))
	{
		psstemp = (pssbase + ptk_stat->seq_owner);
		ptindxs = (unsigned char *)psstemp->ptrk_indxs;
		nactive = psstemp->tracks_active;
		ntracks = (pmsbase->pmod_info->pseq_info + psstemp->seq_num)->seq_hdr.tracks;

		while (ntracks--)
		{
			if (*ptindxs != 0xFF)
			{
				ptstemp = (ptsbase + (*ptindxs));

				ptstemp->psp = (unsigned char*)ptstemp->ppos + CmdLength[26];
				ptstemp->psp++;
				laboff = ptstemp->pstart + *(ptstemp->plabellist + lindex);
				ptstemp->ppos = laboff;
				ptstemp->deltatime = 0;
				ptstemp->flags |= TRK_SKIP;

				if (!--nactive) break;
			}
			ptindxs++;
		}
	}
}

void Eng_SeqJump (track_status *ptk_stat) // 80036B80
{
	static short			lindex;			//800B66A4
	static unsigned short	ntracks;		//800B66A6
	static char				*laboff;		//800B66A8
	static unsigned char	nactive;		//800B66AC
	static unsigned char	*ptindxs;		//800B66B0
	static track_status		*ptstemp;		//800B66B4
	static sequence_status	*psstemp;		//800B66B8

	//PRINTF_D2(WHITE,0,10,"Eng_SeqJump");

	lindex = (*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));

	if ((lindex >= 0) && (lindex < ptk_stat->labellist_count))
	{
		psstemp = (pssbase + ptk_stat->seq_owner);
		ptindxs = (unsigned char *)psstemp->ptrk_indxs;
		nactive = psstemp->tracks_active;
		ntracks = (pmsbase->pmod_info->pseq_info + psstemp->seq_num)->seq_hdr.tracks;

		while (ntracks--)
		{
			if (*ptindxs != 0xFF)
			{
				ptstemp = (ptsbase + (*ptindxs));
				laboff = ptstemp->pstart + *(ptstemp->plabellist + lindex);
				ptstemp->ppos = laboff;
				ptstemp->deltatime = 0;
				ptstemp->flags |= TRK_SKIP;

				if (!--nactive) break;
			}
			ptindxs++;
		}
	}
}

void Eng_SeqRet (track_status *ptk_stat) // 80036D10
{
	static unsigned short	ntracks;		//800B66BC
	static unsigned char	nactive;		//800B66BE
	static unsigned char	*ptindxs;		//800B66C0
	static track_status		*ptstemp;		//800B66C4
	static sequence_status	*psstemp;		//800B66C8

	//PRINTF_D2(WHITE,0,10,"Eng_SeqRet");

	psstemp = (pssbase + ptk_stat->seq_owner);
	ptindxs = (unsigned char *)psstemp->ptrk_indxs;
	nactive = psstemp->tracks_active;
	ntracks = (pmsbase->pmod_info->pseq_info + psstemp->seq_num)->seq_hdr.tracks;

	while (ntracks--)
	{
		if (*ptindxs != 0xFF)
		{
			ptstemp = (ptsbase + (*ptindxs));
			ptstemp->psp--;
			ptstemp->deltatime = 0;
			ptstemp->flags |= TRK_SKIP;
			ptstemp->ppos = (unsigned char*)ptstemp->psp;

			if (!--nactive) break;
		}
		ptindxs++;
	}
}

void Eng_SeqEnd (track_status *ptk_stat) // 80036E48
{
	static sequence_status	*lpseq;		//800B66CC
	static track_status		*ptmp;		//800B66D0
	static unsigned char	*lpdest;	//800B66D4
	static unsigned char	li;			//800B66D8
	static unsigned long	lj;			//800B66DC

	//PRINTF_D2(WHITE,0,10,"Eng_SeqEnd");

	if (ptk_stat->flags & TRK_HANDLED)
	{
		lpseq = (pmsbase->pseqstattbl + ptk_stat->seq_owner);
		lpdest = (unsigned char	*)lpseq->ptrk_indxs;
		li = lpseq->tracks_active;
		lj = pmsbase->max_trks_perseq;

		while (lj--)
		{
			if (*lpdest != 0xFF)
			{
				ptmp = (ptsbase + *lpdest);
				CmdFuncArr[ptmp->patchtype][TrkOff]((track_status *)ptmp);

				if (!--li) break;
			}

			lpdest++;
		}

		ptk_stat->flags |= TRK_SKIP;
	}
	else
	{
		lpseq = (pmsbase->pseqstattbl + ptk_stat->seq_owner);
		lpdest = (unsigned char	*)lpseq->ptrk_indxs;
		li = lpseq->tracks_active;
		lj = pmsbase->max_trks_perseq;

		while (lj--)
		{
			if (*lpdest != 0xFF)
			{
				ptmp = (ptsbase + *lpdest);
				CmdFuncArr[ptmp->patchtype][TrkOff]((track_status *)ptmp);

				if (!--li) break;
			}

			lpdest++;
		}
	}
}

void Eng_TrkTempo (track_status *ptk_stat) // 800370D8
{
	//PRINTF_D2(WHITE,0,10,"Eng_TrkTempo");
	ptk_stat->qpm = *(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8);
	ptk_stat->ppi = CalcPartsPerInt(GetIntsPerSec(), ptk_stat->ppq, ptk_stat->qpm);
}

void Eng_TrkGosub (track_status *ptk_stat) // 8003713C
{
	unsigned int position;

	//PRINTF_D2(WHITE,0,10,"Eng_TrkGosub");

	position = (unsigned int)(*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));
	if ((position >= 0) && (position < ptk_stat->labellist_count))
	{
		ptk_stat->psp = (unsigned char*)ptk_stat->ppos + CmdLength[26];
		ptk_stat->psp++;
		ptk_stat->ppos = ptk_stat->pstart + *(ptk_stat->plabellist + position);
		ptk_stat->deltatime = 0;
		ptk_stat->flags |= TRK_SKIP;
	}
}

void Eng_TrkJump (track_status *ptk_stat) // 800371C4
{
	unsigned int position;

	//PRINTF_D2(WHITE,0,10,"Eng_TrkJump");

	position = (unsigned int)(*(ptk_stat->ppos + 1) | (*(ptk_stat->ppos + 2) << 8));
	if ((position >= 0) && (position < ptk_stat->labellist_count))
	{
		ptk_stat->ppos = ptk_stat->pstart + *(ptk_stat->plabellist + position);
		ptk_stat->deltatime = 0;
		ptk_stat->flags |= TRK_SKIP;
	}
}

void Eng_TrkRet (track_status *ptk_stat) // 80037230
{
    //PRINTF_D2(WHITE,0,10,"Eng_TrkRet");
	ptk_stat->psp--;
	ptk_stat->ppos = (unsigned char*)ptk_stat->psp;
	ptk_stat->ppos = Read_Vlq(ptk_stat->ppos, &ptk_stat->deltatime);
	ptk_stat->deltatime = 0;
	ptk_stat->flags |= TRK_SKIP;
}

void Eng_TrkEnd (track_status *ptk_stat) // 8003728C
{
	//PRINTF_D2(WHITE,0,10,"Eng_TrkEnd");

	if (ptk_stat->flags & TRK_HANDLED)
	{
		if ((ptk_stat->flags & TRK_LOOPED) && (ptk_stat->totppi > 15))
		{
			ptk_stat->flags |= TRK_SKIP;
			ptk_stat->ppos = ptk_stat->pstart;
			ptk_stat->ppos = Read_Vlq(ptk_stat->pstart, &ptk_stat->deltatime);
		}
		else
		{
			CmdFuncArr[ptk_stat->patchtype][TrkOff](ptk_stat);
			ptk_stat->flags |= TRK_SKIP;
		}
	}
	else
	{
		if ((ptk_stat->flags & TRK_LOOPED) && (ptk_stat->totppi > 15))
		{
			ptk_stat->flags |= TRK_SKIP;
			ptk_stat->ppos = ptk_stat->pstart;
			ptk_stat->ppos = Read_Vlq(ptk_stat->pstart, &ptk_stat->deltatime);
		}
		else
		{
			CmdFuncArr[ptk_stat->patchtype][TrkOff](ptk_stat);
		}
	}
}

void Eng_NullEvent (track_status *ptk_stat) // 800373A4
{
    //PRINTF_D2(WHITE,0,10,"Eng_NullEvent");
}

void SeqEngine(void) // 800373AC
{
	static track_status		*pts;	//800B66E0
	static unsigned char	na;		//800B66E4
	static unsigned int		ni;		//800B66E8
	static char				*nn;	//800B66EC

	track_status *ptrkstattbl;
	//PRINTF_D2(WHITE,0,9,"SeqEngine %d\n",SeqOn);

	na = pmsbase->trks_active;
	//PRINTF_D2(WHITE,0,4,"trks_active %d",na);
	//PRINTF_D2(WHITE,0,5,"ptsbase->patchtype %d",ptsbase->patchtype);

	if (na)
	{
		//printf("trks_active %d\n",na);
		pts = ptsbase;
		ni = ntwa;

		while (ni--)
		{
			if ((pts->flags & TRK_ACTIVE))
			{
				if (!(pts->flags & TRK_STOPPED))
				{
					pts->starppi += pts->ppi;
					pts->totppi += (pts->starppi >> 16);
					pts->accppi += (pts->starppi >> 16);
					pts->starppi &= 65535;
					//printf("totppi %d\n",pts->totppi);
					//printf("accppi %d\n",pts->accppi);
					//printf("starppi %d\n",pts->starppi);

					if (!(pts->flags & TRK_TIMED) || (pts->totppi < pts->endppi))
					{
						/*printf("totppi %d\n",pts->totppi);
						printf("accppi %d\n",pts->accppi);
						printf("starppi %d\n",pts->starppi);
						printf("deltatime %d\n",pts->deltatime);*/

						while (pts->deltatime <= pts->accppi &&
							((pts->flags & (TRK_STOPPED | TRK_ACTIVE)) == TRK_ACTIVE))
						{
							pts->accppi -= pts->deltatime;
							nn = (pts->ppos);

							if (!(*nn < 7) && (*nn < 19))
							{
								CmdFuncArr[pts->patchtype][*nn]((track_status *)pts);

								pts->ppos += CmdLength[*nn];
								pts->ppos = Read_Vlq(pts->ppos, &pts->deltatime);
							}
							else if (!(*nn < 19) && (*nn < 36))
							{
								DrvFunctions[*nn]((track_status *)pts);

								if ((pts->flags & (TRK_ACTIVE | TRK_SKIP)) == TRK_ACTIVE)
								{
									pts->ppos += CmdLength[*nn];
									pts->ppos = Read_Vlq(pts->ppos, &pts->deltatime);
								}
								else
								{
									pts->flags &= ~TRK_SKIP;
								}
							}
							else
							{
								Eng_SeqEnd((track_status *)pts);
							}
						}
					}
					else
					{
						CmdFuncArr[pts->patchtype][TrkOff]((track_status *)pts);
					}
				}

				if (!--na) break;
			}

			pts++;
		}
	}

	CmdFuncArr[ptsbase->patchtype][2](ptsbase);
}

#endif
