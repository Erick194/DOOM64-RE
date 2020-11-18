
/* WESS API INCLUDES */
#include "wessapi.h"	// audio stuff...
#include "seqload.h"
#include "soundhw.h"
#include "wessarc.h"
#include "wessseq.h"

#include "funqueue.h"

#include "graph.h" // debug

#ifndef NOUSEWESSCODE
//./seqload.h
//./wessapi.h
//./wessseq.h
//./funqueue.h

#define _ALIGN4_ 1
#define _ALIGN8_ 1

extern void (**CmdFuncArr[10])(track_status *);

module_data				tmp_moddata;		//0x800B4160
patch_group_data		tmp_patgrpdata;		//0x800B4188
pmasterstat tmp_mss;			//0x800B41A8
pmasterstat *pm_stat;			//0x800B41CC

Wess_File_IO_Struct *fp_wmd_file;//8005D8D0
int sysinit;//8005D8D4
int module_loaded;//8005D8D8
int early_exit;//8005D8DC
int wmd_mem_is_mine;//8005D8E0
char *wmd_mem;//8005D8E4
char *wmd_end;//8005D8E8
int wmd_size;//8005D8EC
static int(*Error_func)(int, int) = 0;//8005D8F0
static int Error_module = 0;//8005D8F4

static void err(int code) // 8002F400
{
	if (Error_func) {
		Error_func(Error_module, code);
	}
}

static void zeroset(char *pdest, unsigned long size) // 8002F438
{
	while (size--) *pdest++ = 0;
}

void wess_install_error_handler(int(*error_func)(int, int), int module) // 8002F460
{
	Error_func = error_func;
	Error_module = module;
}

void * wess_get_master_status(void) // 8002F474
{
	return pm_stat;
}

int Is_System_Active(void) // 8002F484
{
	if (!sysinit)
	{
		return 0;
	}
	return 1;
}

int Is_Module_Loaded(void) // 8002F4A8
{
	if (!module_loaded)
	{
		return 0;
	}
	return 1;
}

int Is_Seq_Num_Valid(int seq_num) // 8002F4CC
{
    if ((seq_num >= 0) && (seq_num < wess_seq_loader_count()))
    {
        if ((pm_stat->pmod_info->pseq_info + seq_num)->ptrk_info)
        {
            return 1;
        }
    }

    return 0;

	/*if ((seq_num<0) || (seq_num >= wess_seq_loader_count()))
	{
		return 0;
	}
	else if ((pm_stat->pmod_info->pseq_info + seq_num)->ptrk_info == NULL) {
		return 0;
	}
	return 1;*/
}

void Register_Early_Exit(void) // 8002F540
{
	if (!early_exit)
	{
		early_exit = 1;
	}
}

void wess_install_handler(void) // 8002F564
{
	init_WessTimer();
}

void wess_restore_handler(void) // 8002F584
{
	exit_WessTimer();
}

/* used by wesssys_exit */
enum RestoreFlag {NoRestore,YesRestore};

int wesssys_init(void) // 8002F5A4
{
	int initok;

	initok = 0;

	if (!sysinit)
	{
		wess_engine_off(); /* make sure the SeqEngine is disabled */

		if (!WessTimerActive)
		{
			wess_install_handler();
		}

		wess_low_level_init();
		sysinit = 1;
		initok = 1;
	}
	return (initok);
}

void wesssys_exit(enum RestoreFlag rflag) // 8002F608
{
	if (!Is_System_Active())
	{
		return;
	}

	if (sysinit)
	{
		if (module_loaded)
		{
			wess_unload_module();
		}

		wess_low_level_exit();
		sysinit = 0;
		if (rflag | WessTimerActive)
		{
			wess_restore_handler();
		}
	}
}

char *wess_get_wmd_start(void) // 8002F684
{
	return(wmd_mem);
}

char *wess_get_wmd_end(void) // 8002F694
{
	return(wmd_end);
}

static void free_mem_if_mine(void) // 8002F6A4
{
	if (wmd_mem_is_mine)
	{
		if (wmd_mem != NULL)
		{
			wess_free(wmd_mem);
			wmd_mem = NULL;
		}
		wmd_mem_is_mine = 0;
	}
}

void wess_unload_module(void) // 8002F6F4
{
	if (module_loaded)
	{
		wess_seq_stopall();
		wess_engine_off();

		/* shutdown the loaded drivers and SeqEngine */
		CmdFuncArr[NoSound_ID][DriverExit]((track_status *)pm_stat);
		CmdFuncArr[N64_ID][DriverExit]((track_status *)pm_stat);
		free_mem_if_mine();

		module_loaded = 0;
	}
}

int wess_size_module(char *wmd_filename) // 8002F770
{
	int readrequest, readresult;
	callback_status	*pcalltable;
	int size, i;

	if (!wmd_filename)
		return 0;

	if (!(fp_wmd_file = module_open(wmd_filename)))
	{
		err(wess_FOPEN);
		return (module_loaded);
	}

	pm_stat = (master_status_structure *)&tmp_mss;
	tmp_mss.voices_total = wess_driver_voices;
	tmp_mss.pabstime = &millicount;
	tmp_mss.pmod_info = (module_data *)&tmp_moddata;
	tmp_mss.ppat_info = (patch_group_data *)&tmp_patgrpdata;

	readrequest = sizeof(tmp_moddata.mod_hdr);
	readresult = module_read(&tmp_moddata.mod_hdr, readrequest, fp_wmd_file);
	if (readrequest != readresult)
	{
		err(wess_FREAD);
		return(0);
	}

	if ((pm_stat->pmod_info->mod_hdr.module_id_text != WESS_SSSP_TEXT) ||
		(pm_stat->pmod_info->mod_hdr.module_version != WESS_CORE_VERSION))
	{
		return(0);
	}

	readrequest = sizeof(tmp_patgrpdata.pat_grp_hdr);
	readresult = module_read(&tmp_patgrpdata.pat_grp_hdr, readrequest, fp_wmd_file);
	if (readrequest != readresult)
	{
		err(wess_FREAD);
		return(0);
	}

	module_close(fp_wmd_file);

	size = 8;//start Align 8
	pm_stat->ppat_info->ppat_data = (char *)size;
	size += pm_stat->pmod_info->mod_hdr.data_size;

	pm_stat->pseqstattbl = (sequence_status *)size;
	size += sizeof(*pm_stat->pseqstattbl) * wess_driver_sequences;

	pm_stat->ptrkstattbl = (track_status *)size;
	size += sizeof(*pm_stat->ptrkstattbl) * wess_driver_tracks;

	pm_stat->pvoicestattbl = (voice_status *)size;
	size += sizeof(*pm_stat->pvoicestattbl) * pm_stat->voices_total;

	pm_stat->pcalltable = (callback_status *)size;
	size += sizeof(*pm_stat->pcalltable) * wess_driver_callbacks;

	pm_stat->max_trks_perseq = wess_driver_max_trks_per_seq;

	for (i = 0; i < wess_driver_sequences; i++)
	{
		size += sizeof(char) * wess_driver_gates;
#if _ALIGN4_ == 1
		//force align to word boundary because previous size adjust
		//may wind up with odd address
		size += (unsigned int)size & 1;
		size += (unsigned int)size & 2;
#endif

		size += sizeof(char) * wess_driver_iters;
#if _ALIGN4_ == 1
		//force align to word boundary because previous size adjust
		//may wind up with odd address
		size += (unsigned int)size & 1;
		size += (unsigned int)size & 2;
#endif

		size += sizeof(char) * pm_stat->max_trks_perseq;
#if _ALIGN4_ == 1
		//force align to word boundary because previous size adjust
		//may wind up with odd address
		size += (unsigned int)size & 1;
		size += (unsigned int)size & 2;
#endif
	}

	pm_stat->max_substack_pertrk = wess_driver_max_subs_per_trk;

	for (i = 0; i < wess_driver_tracks; i++)
	{
		size += sizeof(long) * pm_stat->max_substack_pertrk;
	}

#if _ALIGN4_ == 1
	//force align to word boundary because previous size adjust
	//may wind up with odd address
	size += (unsigned int)size & 1;
	size += (unsigned int)size & 2;
#endif

	return size;
}

int wess_load_module(char *wmd_filename,
	char *memory_pointer,
	int   memory_allowance/*,
	int **settings_tag_lists*/) // 8002FA3C
{
	int i, j, k, n, z, types, num, indx, loadit;
	int tracks_toload;
	int readrequest, readresult;
	char max_tracks_inseq, max_voices_intrk, max_substack_intrk;
	char *pdest;
	char *pmem;
	unsigned long patfpos, trkinfosize;
	char *tempwmd;
	int setting, flag, flag2;
	int decomp_type;

	//PRINTF_D(WHITE, "WMD::module_loaded %d", module_loaded);

	if (module_loaded)
	{
		wess_unload_module();
	}

	//num_sd = get_num_Wess_Sound_Drivers(settings_tag_lists);
	//printf("num_sd %d\n", num_sd);

	//PRINTF_D(WHITE, "WMD::memory_pointer %x", &memory_pointer);

	if (memory_pointer == NULL)
	{
		wmd_mem_is_mine = 1;
		wmd_mem = wess_malloc((char *)memory_allowance);
		if (wmd_mem == NULL)
		{
			return(module_loaded);
		}
	}
	else
    {
		wmd_mem_is_mine = 0;
		wmd_mem = memory_pointer;
	}

	wmd_size = memory_allowance;

	//PRINTF_D(WHITE, "WMD::wmd_mem %x", &wmd_mem);
	//PRINTF_D(WHITE, "WMD::wmd_size %d", wmd_size);

	zeroset(wmd_mem, wmd_size);

	if (!Is_System_Active())
	{
	    //PRINTF_D(WHITE, "WMD::Is_System_Active no activo");
		free_mem_if_mine();
		return (module_loaded);
	}

	if (wmd_filename == NULL)
	{
		free_mem_if_mine();
		return (module_loaded);
	}

	if (!(fp_wmd_file = module_open(wmd_filename)))
	{
	    //PRINTF_D(WHITE, "WMD::fp_wmd_file %s Error al abrir", fp_wmd_file);
		err(wess_FOPEN);
		free_mem_if_mine();
		return (module_loaded);
	}

	//PRINTF_D(WHITE, "WMD::fp_wmd_file %s", fp_wmd_file);



	/* loads a related group of patches and sequences */
	/* a module has the information necessary to set up sequencer work
	areas and data areas */
	/*
	The module loading sequence works as follows :
	*/

	pmem = wmd_mem;

	/*
	- allocate space for a master_status_structure
	- update the pmasterstat pointer
	*/

	pm_stat = (master_status_structure *)&tmp_mss;
	tmp_mss.voices_total = wess_driver_voices;
	tmp_mss.pabstime = &millicount;

	/*
	- allocate for the module_data structure
	- update the pmod_info pointer
	*/

	tmp_mss.pmod_info = (module_data *)&tmp_moddata;

	/*
	- allocate for the patch_group_data structure
	- update the ppat_info pointer
	*/
	tmp_mss.ppat_info = (patch_group_data *)&tmp_patgrpdata;

	/*
	- read in sizeof(pm_stat->pmod_info->mod_hdr)
	bytes from the .lmd file into the pm_stat->pmod_info->mod_hdr
	structure.
	*/

	readrequest = sizeof(tmp_moddata.mod_hdr);
	readresult = module_read(&tmp_moddata.mod_hdr, readrequest, fp_wmd_file);
	if (readrequest != readresult)
	{
		err(wess_FREAD);
		free_mem_if_mine();
		return(0);
	}

	//PRINTF_D(WHITE, "WMD::mod_hdr");
	//PRINTF_D(WHITE, "WMD::module_id_text %x",pm_stat->pmod_info->mod_hdr.module_id_text);
	//PRINTF_D(WHITE, "WMD::module_version %d",pm_stat->pmod_info->mod_hdr.module_version);
	//PRINTF_D(WHITE, "WMD::sequences %d",pm_stat->pmod_info->mod_hdr.sequences);
	//PRINTF_D(WHITE, "WMD::decomp_type %d",pm_stat->pmod_info->mod_hdr.decomp_type);
	//PRINTF_D(WHITE, "WMD::compress_size %d",pm_stat->pmod_info->mod_hdr.compress_size);
	//PRINTF_D(WHITE, "WMD::data_size %d",pm_stat->pmod_info->mod_hdr.data_size);

	if ((pm_stat->pmod_info->mod_hdr.module_id_text != WESS_SSSP_TEXT) ||
		(pm_stat->pmod_info->mod_hdr.module_version != WESS_CORE_VERSION))
	{
		free_mem_if_mine();
		return(0);
	}

	readrequest = sizeof(tmp_patgrpdata.pat_grp_hdr);
	readresult = module_read(&tmp_patgrpdata.pat_grp_hdr, readrequest, fp_wmd_file);
	if (readrequest != readresult)
	{
		err(wess_FREAD);
		free_mem_if_mine();
		return(0);
	}

	//PRINTF_D(WHITE, "WMD::pat_grp_hdr");
	//PRINTF_D(WHITE, "WMD::load_flags %d",pm_stat->ppat_info->pat_grp_hdr.load_flags);
	//PRINTF_D(WHITE, "WMD::patches %d",pm_stat->ppat_info->pat_grp_hdr.patches);
	//PRINTF_D(WHITE, "WMD::patch_size %d",pm_stat->ppat_info->pat_grp_hdr.patch_size);
	//PRINTF_D(WHITE, "WMD::patchmaps %d",pm_stat->ppat_info->pat_grp_hdr.patchmaps);
	//PRINTF_D(WHITE, "WMD::patchmap_size %d",pm_stat->ppat_info->pat_grp_hdr.patchmap_size);
	//PRINTF_D(WHITE, "WMD::patchinfo %d",pm_stat->ppat_info->pat_grp_hdr.patchinfo);
	//PRINTF_D(WHITE, "WMD::patchinfo_size %d",pm_stat->ppat_info->pat_grp_hdr.patchinfo_size);
	//PRINTF_D(WHITE, "WMD::drummaps %d",pm_stat->ppat_info->pat_grp_hdr.drummaps);
	//PRINTF_D(WHITE, "WMD::drummap_size %d",pm_stat->ppat_info->pat_grp_hdr.drummap_size);
	//PRINTF_D(WHITE, "WMD::extra_data_size %d",pm_stat->ppat_info->pat_grp_hdr.extra_data_size);

#if _ALIGN8_ == 1
	//force align to word boundary because previous size adjust
	//may wind up with odd address
	pmem += (unsigned int)pmem & 1;
	pmem += (unsigned int)pmem & 2;
	pmem += (unsigned int)pmem & 4;
#endif

	/*
	- allocate and initialize space for
	pm_stat->patch_types patch_group_data structures
	and update the pm_stat->ppat_info pointer.
	*/

	pm_stat->ppat_info->ppat_data = (char*)(patch_group_data *)pmem;
	pmem += pm_stat->pmod_info->mod_hdr.data_size;

	if (pm_stat->pmod_info->mod_hdr.decomp_type)
	{
		module_close(fp_wmd_file);

		decomp_type = pm_stat->pmod_info->mod_hdr.decomp_type;
		readrequest = pm_stat->pmod_info->mod_hdr.data_size;//uncompressed_size
		if (wess_decomp(decomp_type, wmd_filename, 56, pm_stat->ppat_info->ppat_data, readrequest) < 0)
			return(0);
	}
	else
	{
		readrequest = pm_stat->pmod_info->mod_hdr.data_size;
		readresult = module_read(pm_stat->ppat_info->ppat_data, readrequest, fp_wmd_file);

		//printf("WMD::readrequest %d\n",readrequest);
		//printf("WMD::readrequest %d\n",readresult);
		if (readrequest != readresult)
		{
			err(wess_FREAD);
			free_mem_if_mine();
			return(0);
		}

		module_close(fp_wmd_file);
	}

	/*
	--init work structures --------------------------------------------
	- allocate and initialize space for
	pmod_info->mod_hdr.seq_work_areas sequence_status structures
	and update the pmseqstattbl pointer and zero seqs_active.
	*/

	pm_stat->pseqstattbl = (sequence_status *)pmem;
	pmem += sizeof(*pm_stat->pseqstattbl) * wess_driver_sequences;

	//PRINTF_D(WHITE, "WMD::pseqstattbl %d",sizeof(*pm_stat->pseqstattbl) * wess_driver_sequences);

	/*
	- allocate and initialize space for
	pmod_info->mod_hdr.trk_work_areas track_status structures
	and update the pmtrkstattbl pointer and zero trks_active.
	*/

	pm_stat->ptrkstattbl = (track_status *)pmem;
	pmem += sizeof(*pm_stat->ptrkstattbl) * wess_driver_tracks;

	//PRINTF_D(WHITE, "WMD::ptrkstattbl %d",sizeof(*pm_stat->ptrkstattbl) * wess_driver_tracks);

	/*
	- allocate and initialize space for
	voice_total voice_status structures
	and update the pmvoicestattbl pointer and zero voices_active.
	*/

	pm_stat->pvoicestattbl = (voice_status *)pmem;
	pmem += sizeof(*pm_stat->pvoicestattbl) * pm_stat->voices_total;

	//PRINTF_D(WHITE, "WMD::pvoicestattbl %d",sizeof(*pm_stat->pvoicestattbl) * pm_stat->voices_total);

	/*
	- initialize patch_type parameter for each voice work area.
	only the amount of hardware voices possible for each
	patch_type loaded will have voice work areas!!!
	you will run out of voice work areas for a given patch type
	at the same time you have run out of hardware polyphony!!!
	eh,eh,this is cool!,eh,eh
	*/

	for (i = 0; i < pm_stat->voices_total; i++)
	{
		(pm_stat->pvoicestattbl + i)->patchtype = 1;
		(pm_stat->pvoicestattbl + i)->refindx = i;
	}

	/*
	- allocate pm_stat->max_tracks_inseq chars for the ptrk_indxs
	pointers in each sequence_status structure.
	update each pointer to each area.
	initialize indexes to 0xFF.
	*/

	pm_stat->pcalltable = (callback_status *)pmem;
	pmem += sizeof(*pm_stat->pcalltable) * wess_driver_callbacks;

	//PRINTF_D(WHITE, "WMD::pcalltable %d",sizeof(*pm_stat->pcalltable) * wess_driver_callbacks);

	pm_stat->max_trks_perseq = wess_driver_max_trks_per_seq;

	//PRINTF_D(WHITE, "WMD::max_trks_perseq %d",pm_stat->max_trks_perseq);

	for (i = 0; i < wess_driver_sequences; i++)
	{
		(pm_stat->pseqstattbl + i)->pgates = (char *)pmem;

		pmem += sizeof(char) * wess_driver_gates;
#if _ALIGN4_ == 1
		//force align to word boundary because previous pmem adjust
		//may wind up with odd address
		pmem += (unsigned int)pmem & 1;
		pmem += (unsigned int)pmem & 2;
#endif

		(pm_stat->pseqstattbl + i)->piters = (char *)pmem;
		pmem += sizeof(char) * wess_driver_iters;
#if _ALIGN4_ == 1
		//force align to word boundary because previous pmem adjust
		//may wind up with odd address
		pmem += (unsigned int)pmem & 1;
		pmem += (unsigned int)pmem & 2;
#endif

		j = pm_stat->max_trks_perseq;
		pdest = (pm_stat->pseqstattbl + i)->ptrk_indxs = (char *)pmem;
		pmem += sizeof(char) * j;
#if _ALIGN4_ == 1
		//force align to word boundary because previous pmem adjust
		//may wind up with odd address
		pmem += (unsigned int)pmem & 1;
		pmem += (unsigned int)pmem & 2;
#endif

		while (j--)
		{
			*pdest++ = 0xFF;
		}
	}

	/*
	- allocate pm_stat->max_voices_intrk chars for the pvoice_indxs
	pointers in each track_status structure.
	update each pointer to each area.
	initialize indexes to 0xFF.
	*/

	pm_stat->max_substack_pertrk = wess_driver_max_subs_per_trk;

	//PRINTF_D(WHITE, "WMD::max_substack_pertrk %d",wess_driver_max_subs_per_trk);

	for (i = 0; i < wess_driver_tracks; i++)
	{
		(pm_stat->ptrkstattbl + i)->refindx = i;
		(pm_stat->ptrkstattbl + i)->psubstack = (unsigned long *)pmem;
		/* (pm_stat->ptrkstattbl+i)->psp is set when sequence is triggered */
		pmem += sizeof(long) * pm_stat->max_substack_pertrk;
		(pm_stat->ptrkstattbl + i)->pstackend = (unsigned long *)pmem;
	}

#if _ALIGN4_ == 1
	//force align to word boundary because previous pmem adjust
	//may wind up with odd address
	pmem += (unsigned int)pmem & 1;
	pmem += (unsigned int)pmem & 2;
#endif

	CmdFuncArr[NoSound_ID][DriverInit]((track_status *)pm_stat);
	CmdFuncArr[N64_ID][DriverInit]((track_status *)pm_stat);

	wmd_end = pmem;
	module_loaded = 1;
	wess_engine_on();

	return (1);
}

void filltrackstat(track_status *ptk_stat, track_data *ptk_info, TriggerPlayAttr *attr) // 8003002C
{
	int tempmask;

	ptk_stat->flags = (ptk_stat->flags | TRK_ACTIVE | TRK_OFF) & ~TRK_TIMED & ~TRK_LOOPED & ~TRK_SKIP;

	ptk_stat->patchtype = 1;
	ptk_stat->voices_active = 0;
	ptk_stat->sndclass = ptk_info->trk_hdr->voices_type;//ptk_info->trk_hdr.voices_class;

	ptk_stat->starppi = 0;
	ptk_stat->accppi = 0;
	ptk_stat->totppi = 0;
	ptk_stat->psp = (unsigned char*)ptk_stat->psubstack;
	ptk_stat->ppq = ptk_info->trk_hdr->initppq;
	ptk_stat->labellist_count = ptk_info->trk_hdr->labellist_count;
	ptk_stat->data_size = ptk_info->trk_hdr->data_size;

	ptk_stat->mutemask = ptk_info->trk_hdr->mutebits;

	if ((attr == NULL) || (!attr->mask))
	{
		tempmask = 0;
	}
	else {
		tempmask = attr->mask;
	}

	if (tempmask & TRIGGER_VOLUME) //0x01
	{
		ptk_stat->volume_cntrl = attr->volume;
	}
	else {
		ptk_stat->volume_cntrl = ptk_info->trk_hdr->initvolume_cntrl;
	}

	if (tempmask & TRIGGER_PAN) //0x02
	{
		ptk_stat->pan_cntrl = attr->pan;
	}
	else {
		ptk_stat->pan_cntrl = ptk_info->trk_hdr->initpan_cntrl;
	}

	if (tempmask & TRIGGER_PATCH) //0x04
	{
		ptk_stat->patchnum = attr->patch;
	}
	else {
		ptk_stat->patchnum = ptk_info->trk_hdr->initpatchnum;
	}

	if (tempmask & TRIGGER_PITCH) //0x08
	{
		ptk_stat->pitch_cntrl = attr->pitch;
	}
	else {
		ptk_stat->pitch_cntrl = ptk_info->trk_hdr->initpitch_cntrl;
	}

	if (tempmask & TRIGGER_MUTEMODE) //0x10
	{
		if (ptk_stat->mutemask & (1 << attr->mutemode))
		{
			ptk_stat->flags |= TRK_MUTE;
		}
		else {
			ptk_stat->flags &= ~TRK_MUTE;
		}
	}
	else {
		ptk_stat->flags &= ~TRK_MUTE;
	}

	if (tempmask & TRIGGER_TEMPO) //0x20
	{
		ptk_stat->qpm = attr->tempo;
	}
	else {
		ptk_stat->qpm = ptk_info->trk_hdr->initqpm;
	}

	ptk_stat->ppi = CalcPartsPerInt(GetIntsPerSec(), ptk_stat->ppq, ptk_stat->qpm);

	if (tempmask & TRIGGER_TIMED) //0x40
	{
		ptk_stat->endppi = ptk_stat->totppi + attr->timeppq;
		ptk_stat->flags |= TRK_TIMED;
	}
	else {
		ptk_stat->flags &= ~TRK_TIMED;
	}

	if (tempmask&TRIGGER_LOOPED) //0x80
	{
		ptk_stat->flags |= TRK_LOOPED;
	}
	else {
		ptk_stat->flags &= ~TRK_LOOPED;
	}

	if (tempmask & TRIGGER_REVERB) //0x100
	{
		ptk_stat->reverb = attr->reverb;
	}
	else {
		ptk_stat->reverb = ptk_info->trk_hdr->reverb;
	}

	/*printf("ptk_stat->ppq %d\n",ptk_stat->ppq);
	printf("ptk_stat->ppq %d\n",ptk_stat->ppq);
	printf("ptk_stat->labellist_count %d\n",ptk_stat->labellist_count);
	printf("ptk_stat->data_size %d\n",ptk_stat->data_size);
	printf("ptk_stat->mutemask %d\n",ptk_stat->mutemask);
	printf("ptk_stat->volume_cntrl %d\n",ptk_stat->volume_cntrl);
	printf("ptk_stat->pan_cntrl %d\n",ptk_stat->pan_cntrl);
	printf("ptk_stat->patchnum %d\n",ptk_stat->patchnum);
	printf("ptk_stat->pitch_cntrl %d\n",ptk_stat->pitch_cntrl);
	printf("ptk_stat->qpm %d\n",ptk_stat->qpm);
	printf("ptk_stat->ppi %d\n",ptk_stat->ppi);
	printf("ptk_stat->flags %d\n",ptk_stat->flags);
	printf("ptk_stat->reverb %d\n",ptk_stat->reverb);*/
}

void assigntrackstat(track_status *ptk_stat, track_data *ptk_info) // 800302F8
{
	ptk_stat->data_space = ptk_info->trk_hdr->data_size;
	ptk_stat->labellist_max = ptk_info->trk_hdr->labellist_count;
	ptk_stat->pstart = ptk_info->ptrk_data;
	ptk_stat->ppos = Read_Vlq(ptk_stat->pstart, &ptk_stat->deltatime);
	ptk_stat->plabellist = ptk_info->plabellist;
}

/* used by wess trigger functions */
enum LoopedFlag { NoLooped, YesLooped };

/* used by wess trigger functions */
enum HandleFlag { NoHandle, YesHandle };

int wess_seq_structrig(sequence_data *psq_info,
	int seq_num,
	int seq_type,
	enum HandleFlag gethandle,
	TriggerPlayAttr *attr) // 8003036C
{
    char i, j, limit;
    short n;
	char tracksfilled;
	char *pdest;
	sequence_status *psq_stat;
	track_data *ptk_info;
	track_status *ptk_stat;

	//-------
	sequence_data *_psq_info;
	int _seq_num;
	int _seq_type;
	enum HandleFlag _gethandle;
	TriggerPlayAttr *_attr;

	_psq_info = psq_info;
	_seq_num  = seq_num;
	_seq_type = seq_type;
	_gethandle = gethandle;
	_attr = attr;

	if (!Is_Seq_Num_Valid(_seq_num))
	{
		return (0);
	}

	//printf("wess_seq_structrig %d\n",_seq_num);

	/* runs trigger function and update status structures for api */

	/*
	- save the sequencer information block pointer,
	if a sequence status structure is free:
	mark sequence as active,
	flag to start what tracks can be started,
	for each track started update psq_info->ptrk_indxs
	*/

	wess_disable();

	/* find an open sequence structure */

	limit = wess_driver_sequences;//pm_stat->pmod_info->mod_hdr.seq_work_areas;

	for (i = 0; i<limit; i++)
	{
		if (!((pm_stat->pseqstattbl + i)->flags & SEQ_ACTIVE))
		{
			break;
		}
	}
	if (i == limit)
	{
		wess_enable();
		return (0); /* no sequence work area available */
	}
	else
	{
		tracksfilled = 0; /* used to check if any tracks are started */
		/*
		we found a sequence structure so fill it.
		*/

		psq_stat = pm_stat->pseqstattbl + i;          /* pointer math */

		/*
		for n tracks in the sequence find and open track structure
		and initialize it.
		*/
		n = _psq_info->seq_hdr.tracks;
		limit = wess_driver_tracks;//pm_stat->pmod_info->mod_hdr.trk_work_areas;
		pdest = psq_stat->ptrk_indxs;

		//printf("tracks %d\n",n);
		//printf("limit %d\n",limit);

		for (j = 0; j<limit; j++)
		{
			if (!((pm_stat->ptrkstattbl + j)->flags & TRK_ACTIVE))
			{

				ptk_stat = pm_stat->ptrkstattbl + j;  /* pointer math */
				ptk_info = _psq_info->ptrk_info + tracksfilled;

				/* refindx was filled at init time */
				ptk_stat->seq_owner = i;

				filltrackstat(ptk_stat, ptk_info, _attr);
				assigntrackstat(ptk_stat, ptk_info);

				if (_gethandle)
				{
					ptk_stat->flags |= (TRK_STOPPED | TRK_HANDLED);

				}
				else {
					ptk_stat->flags &= ~(TRK_STOPPED | TRK_HANDLED);
					psq_stat->tracks_playing++;
				}

				psq_stat->tracks_active++;
				pm_stat->trks_active++;
				*pdest++ = j; /* update ptrk_indxs for the sequence */
				tracksfilled++;

				if (!(((--n)<<16)>>16)) break;
			}
		}

		/* if tracks were started, activate the sequence */
		if (tracksfilled)
		{
			psq_stat->seq_num = _seq_num;
			psq_stat->seq_type = _seq_type;
			if (_gethandle)
			{
				psq_stat->flags |= SEQ_HANDLE;
				psq_stat->playmode = SEQ_STATE_STOPPED;
			}
			else {
				psq_stat->flags &= ~SEQ_HANDLE;
				psq_stat->playmode = SEQ_STATE_PLAYING;
			}
			psq_stat->volume = 128;
			psq_stat->pan = 64;
			psq_stat->flags |= SEQ_ACTIVE;
			pm_stat->seqs_active++;
		}
		wess_enable();

		if (tracksfilled)
		{
			return (i + 1);
		}
		else {
			return (0);
		}
	}
}

void wess_seq_trigger(int seq_num) // 80030650
{
	wess_seq_trigger_type(seq_num, 0);
}

void wess_seq_trigger_special(int seq_num, TriggerPlayAttr *attr) // 80030670
{
	sequence_data *psq_info;

	psq_info = pm_stat->pmod_info->pseq_info + seq_num; /* pointer math */

	wess_seq_structrig(psq_info, seq_num, 0, NoHandle, attr);
}

int wess_seq_status(int sequence_number) // 800306C0
{
	/* immediate stop of sequence */
	char nt, na;
	sequence_status *psq_stat;
	int status;

	int _sequence_number;

	_sequence_number = sequence_number;

	if (!Is_Seq_Num_Valid(_sequence_number))
	{
		return(SEQUENCE_INVALID);
	}

	status = SEQUENCE_INACTIVE;

	wess_disable();

	/* search for all sequences with this number and turn them off */
	nt = wess_driver_sequences;//pm_stat->pmod_info->mod_hdr.seq_work_areas;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;

	if (na)
	{
		while (nt--)
		{
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				if (psq_stat->seq_num == _sequence_number)
				{
					if (psq_stat->playmode == SEQ_STATE_STOPPED)
					{
						status = SEQUENCE_STOPPED;
					}
					else if (psq_stat->playmode == SEQ_STATE_PLAYING) {
						status = SEQUENCE_PLAYING;
					}
				}

				if (!--na) break;
			}
			psq_stat++;
		}
	}

	wess_enable();

	return(status);
}

/* used by wess_seq_stop and wess_seq_stop_and_voiceramp functions */
enum MuteRelease { NoMuteRelease, YesMuteRelease};

void __wess_seq_stop(int sequence_number, enum MuteRelease mrelease, int millisec) // 800307AC
{
	/* immediate stop of sequence */

	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	int li, lj;

	int _sequence_number;
	enum MuteRelease _mrelease;
	int _millisec;

	_sequence_number = sequence_number;
	_mrelease = mrelease;
	_millisec = millisec;

	if (!Is_Seq_Num_Valid(_sequence_number))
	{
		return;
	}

	wess_disable();

	/* search for all sequences with this number and turn them off */
	nt = wess_driver_sequences;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;
	if (na)
	{
		while (nt--)
		{
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				if (psq_stat->seq_num == _sequence_number)
				{
					psq_stat->flags = (psq_stat->flags | SEQ_STOP) & ~SEQ_PAUSE & ~SEQ_RESTART;
				}
				if (!--na) break;
			}
			psq_stat++;
		}
	}

	if (_mrelease)
	{
		queue_the_function(QUEUE_SEQ_STOP_AND_VOICERAMP);
		queue_the_data(&_millisec, sizeof(int));
	}
	else
	{
		queue_the_function(QUEUE_SEQ_STOP);
	}
	queue_the_data(&_sequence_number, sizeof(int));
	wess_enable();
}

void wess_seq_stop(int sequence_number) // 800308BC
{
	__wess_seq_stop(sequence_number, NoMuteRelease, 0);
}

void wess_seq_stop_and_voiceramp(int sequence_number, int millisec) // 800308E0
{
	__wess_seq_stop(sequence_number, YesMuteRelease, millisec);
}

void queue_wess_seq_stop(int sequence_number, enum MuteRelease mrelease, int millisec);

void run_queue_wess_seq_stop(void) // 80030904
{
	int sequence_number;

	unqueue_the_data(&sequence_number, sizeof(int));
	queue_wess_seq_stop(sequence_number, NoMuteRelease, 0);
}

void run_queue_wess_seq_stop_and_voiceramp(void) // 80030938
{
	int millisec;
	int sequence_number;

	unqueue_the_data(&millisec, sizeof(int));
	unqueue_the_data(&sequence_number, sizeof(int));
	queue_wess_seq_stop(sequence_number, YesMuteRelease, millisec);
}

void queue_wess_seq_stop(int sequence_number, enum MuteRelease mrelease, int millisec) // 80030978
{
	/* immediate stop of sequence */

	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;
	int get_millisec;

	int _sequence_number;
	enum MuteRelease _mrelease;
	int _millisec;

	_sequence_number = sequence_number;
	_mrelease = mrelease;
	_millisec = millisec;

	if (!Is_Seq_Num_Valid(_sequence_number))
	{
		return;
	}

	wess_disable();

	/* search for all sequences with this number and turn them off */
	nt = wess_driver_sequences;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;
	if (na)
	{
		if (_mrelease)
		{
			get_millisec = wess_get_mute_release();
			wess_set_mute_release(_millisec);
		}

		while (nt--)
		{
			//if (psq_stat->active)
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				if (psq_stat->seq_num == _sequence_number)
				{
					if (psq_stat->flags & SEQ_STOP)
					{
						psq_stat->flags &= ~SEQ_STOP;

						li = psq_stat->tracks_active;
						lj = pm_stat->max_trks_perseq;
						/* *lpdest refers to an active track if not 0xFF */
						lpdest = psq_stat->ptrk_indxs;
						while (lj--)
						{
							if (*lpdest != 0xFF)
							{
								ptmp = (pm_stat->ptrkstattbl + (*lpdest));
								CmdFuncArr[ptmp->patchtype][TrkOff](ptmp);
								if (!--li) break;
							}
							lpdest++;
						}
					}
				}
				if (!--na) break;
			}
			psq_stat++;
		}

		if (_mrelease)
		{
			wess_set_mute_release(get_millisec);
		}
	}

	wess_enable();
}

void __wess_seq_stopall(enum MuteRelease mrelease, int millisec) // 80030B78
{
	/* immediate stop of all sequences */

	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	int li, lj;

	enum MuteRelease _mrelease;
	int _millisec;

	_mrelease = mrelease;
	_millisec = millisec;

	if (!Is_Module_Loaded())
	{
		return;
	}

	wess_disable();

	/* search for all sequences with this number and turn them off */
	nt = wess_driver_sequences;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;
	if (na)
	{
		while (nt--)
		{
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				psq_stat->flags = (psq_stat->flags | SEQ_STOP) & ~SEQ_PAUSE & ~SEQ_RESTART;
				if (!--na) break;
			}
			psq_stat++;
		}
	}

	if (_mrelease)
	{
		queue_the_function(QUEUE_SEQ_STOPALL_AND_VOICERAMP);
		queue_the_data(&_millisec, sizeof(int));
	}
	else
	{
		queue_the_function(QUEUE_SEQ_STOPALL);
	}

	wess_enable();
}

void wess_seq_stopall(void) // 80030C68
{
	__wess_seq_stopall(NoMuteRelease, 0);
}

void wess_seq_stopall_and_voiceramp(int millisec) // 80030C8C
{
	__wess_seq_stopall(YesMuteRelease, millisec);
}

void queue_wess_seq_stopall(enum MuteRelease mrelease, int millisec);

void run_queue_wess_seq_stopall(void) // 80030CB0
{
	queue_wess_seq_stopall(NoMuteRelease, 0);
}

void run_queue_wess_seq_stopall_and_voiceramp(void) // 80030CD4
{
	int millisec;

	unqueue_the_data(&millisec, sizeof(int));
	queue_wess_seq_stopall(YesMuteRelease, millisec);
}

void queue_wess_seq_stopall(enum MuteRelease mrelease, int millisec) // 80030D04
{
	char nt, na;
	sequence_status *psq_stat;
	track_status *ptmp;
	char *lpdest;
	int li, lj;
	int get_millisec;

	enum MuteRelease _mrelease;
	int _millisec;

	_mrelease = mrelease;
	_millisec = millisec;

	if (!Is_Module_Loaded())
	{
		return;
	}

	/* immediate stop of all sequences */
	wess_disable();

	/* search for all sequences and turn them off */
	nt = wess_driver_sequences;
	na = pm_stat->seqs_active;
	psq_stat = pm_stat->pseqstattbl;
	if (na)
	{
		if (_mrelease)
		{
			get_millisec = wess_get_mute_release();
			wess_set_mute_release(_millisec);
		}

		while (nt--)
		{
			if (psq_stat->flags & SEQ_ACTIVE)
			{
				if (psq_stat->flags & SEQ_STOP)
				{
					psq_stat->flags &= ~SEQ_STOP;

					li = psq_stat->tracks_active;
					lj = pm_stat->max_trks_perseq;
					/* *lpdest refers to an active track if not 0xFF */
					lpdest = psq_stat->ptrk_indxs;
					while (lj--)
					{
						if (*lpdest != 0xFF)
						{
							ptmp = (pm_stat->ptrkstattbl + (*lpdest));
							CmdFuncArr[ptmp->patchtype][TrkOff](ptmp);
							if (!--li) break;
						}
						lpdest++;
					}
				}
				if (!--na) break;
			}

			psq_stat++;
		}

		if (_mrelease)
		{
			wess_set_mute_release(get_millisec);
		}
	}

	wess_enable();
}
#endif // 0
