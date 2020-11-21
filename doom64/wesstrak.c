
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

#ifndef NOUSEWESSCODE

/*
PatchChg 7
{
	Tuning Program Change
}

PitchMod 9
{
	Pitch Bend Sensitivity
}

ZeroMod 11
{
	MIDI_Controller = 0 (Bank Select)
}

ModuMod 11
{
	MIDI_Controller = 1 (Modulation Wheel or Lever)
}

VolumeMod 12
{
	MIDI_Controller = 7 (Channel Volume(formerly Main Volume))
}

PanMod 13
{
	MIDI_Controller = 10 (Pan)
}

PedalMod 14
{
	MIDI_Controller = 64 (Damper Pedal on / off(Sustain))
}

ReverbMod 15
{
	MIDI_Controller = 91 (Effects 1 Depth->Reverb Send Level)
}

ChorusMod 16
{
	MIDI_Controller = 93 (Effects 3 Depth->Chorus Send Level)
}
*/

extern char scratch_area[32];//800B41E0

extern void patch_chg_action(track_status *ptmp, int patch_num); // 800340E0
extern void pitch_mod_action(track_status *ptmp, int pitch_cntr); // 80034144
extern void volume_mod_action(track_status *ptmp, int volume_cntr); // 800341A8
extern void pan_mod_action(track_status *ptmp, int pan_cntr); // 80034200
extern track_status *gethandletrk(int handle, int track); // 80032EE0

void queue_wess_handle_noteon(int handle, int track, char keynum, char velnum);
void wess_handle_parm_mod(int handle, int track, int value, WessAction function);
void queue_wess_handle_parm_mod(int handle, int track, int value, WessAction function);

void wess_handle_patchchg(int handle, int track, short patchnum) // 80034664
{
	wess_handle_parm_mod(handle, track, patchnum, patch_chg_action);
}

void wess_handle_noteon(int handle, int track, char keynum, char velnum) // 8003469C
{
    int _handle;
	int _track;
	char _keynum;
	char _velnum;

	_handle = handle;
	_track  = track;
	_keynum = keynum;
	_velnum = velnum;

	wess_disable();
	queue_the_function(QUEUE_HANDLE_NOTEON);
	queue_the_data(&_handle, sizeof(int));
	queue_the_data(&_track, sizeof(int));
	queue_the_data(&_keynum, sizeof(char));
	queue_the_data(&_velnum, sizeof(char));
	wess_enable();
}

void run_queue_wess_handle_noteon(void) // 80034708
{
	int handle;
	int track;
	char keynum;
	char velnum;

	unqueue_the_data(&handle, sizeof(int));
	unqueue_the_data(&track, sizeof(int));
	unqueue_the_data(&keynum, sizeof(char));
	unqueue_the_data(&velnum, sizeof(char));
	queue_wess_handle_noteon(handle, track, keynum, velnum);
}

void queue_wess_handle_noteon(int handle, int track, char keynum, char velnum) // 80034768
{
	track_status *ptmp;
	char *ppos;

	int _handle;
	int _track;
	char _keynum;
	char _velnum;

	_handle = handle;
	_track  = track;
	_keynum = keynum;
	_velnum = velnum;

	if (Is_Module_Loaded())
	{
		wess_disable();
		ptmp = gethandletrk(_handle, _track);

		if (ptmp)
		{
			//save
			ppos = ptmp->ppos;

			// set tmp buffer ppos
			ptmp->ppos = scratch_area;
			ptmp->ppos[0] = NoteOn;
			ptmp->ppos[1] = _keynum;
			ptmp->ppos[2] = _velnum;

			CmdFuncArr[ptmp->patchtype][NoteOn](ptmp);

			//restore
			ptmp->ppos = ppos;
		}

		wess_enable();
	}
}

void noteoff_action(track_status *ptmp, int keynum) // 8003483C
{
	ptmp->ppos[0]= NoteOff;
	ptmp->ppos[1]= keynum;
	CmdFuncArr[ptmp->patchtype][NoteOff](ptmp);
}

void wess_handle_noteoff(int handle, int track, char keynum) // 80034894
{
	wess_handle_parm_mod(handle, track, keynum, noteoff_action);
}

void wess_handle_pitchmod(int handle, int track, short pitch_cntrl) // 800348C0
{
	wess_handle_parm_mod(handle, track, pitch_cntrl, pitch_mod_action);
}

void zero_mod_action(track_status *ptmp, int value) // 800348F0
{
	ptmp->ppos[0] = ZeroMod;
	ptmp->ppos[1] = value;
	CmdFuncArr[ptmp->patchtype][ZeroMod](ptmp);
}

void wess_handle_zeromod(int handle, int track, char value) // 80034948
{
	wess_handle_parm_mod(handle, track, value, zero_mod_action);
}

void modu_mod_action(track_status *ptmp, int value) // 80034974
{
	ptmp->ppos[0]= ModuMod;
	ptmp->ppos[1] = value;
	CmdFuncArr[ptmp->patchtype][ModuMod](ptmp);
}

void wess_handle_modumod(int handle, int track, char value) // 800349CC
{
	wess_handle_parm_mod(handle, track, value, modu_mod_action);
}

void wess_handle_volumemod(int handle, int track, char volume_cntrl) // 800349F8
{
	wess_handle_parm_mod(handle, track, volume_cntrl, volume_mod_action);
}

void wess_handle_panmod(int handle, int track, char pan_cntr) // 80034A24
{
	wess_handle_parm_mod(handle, track, pan_cntr, pan_mod_action);
}

void pedal_mod_action(track_status *ptmp, int value) // 80034A50
{
	ptmp->ppos[0] = PedalMod;
	ptmp->ppos[1] = value;
	CmdFuncArr[ptmp->patchtype][PedalMod](ptmp);
}

void wess_handle_pedalmod(int handle, int track, char value) // 80034AA8
{
	wess_handle_parm_mod(handle, track, value, pedal_mod_action);
}

void reverb_mod_action(track_status *ptmp, int reverb) // 80034AD4
{
	ptmp->ppos[0] = ReverbMod;
	ptmp->ppos[1] = reverb;
	CmdFuncArr[ptmp->patchtype][ReverbMod](ptmp);
}

void wess_handle_reverbmod(int handle, int track, char reverb) // 80034B2C
{
	wess_handle_parm_mod(handle, track, reverb, reverb_mod_action);
}

void chorus_mod_action(track_status *ptmp, int chorus) // 80034B58
{
	ptmp->ppos[0] = ChorusMod;
	ptmp->ppos[1] = chorus;
	CmdFuncArr[ptmp->patchtype][ChorusMod](ptmp);
}

void wess_handle_chorusmod(int handle, int track, char chorus) // 80034BB0
{
	wess_handle_parm_mod(handle, track, chorus, chorus_mod_action);
}

void wess_handle_parm_mod(int handle, int track, int value, WessAction function) // 80034BDC
{
    int _handle;
	int _track;
	int _value;
	WessAction _function;

	_handle = handle;
	_track  = track;
	_value  = value;
	_function = function;

	wess_disable();
	queue_the_function(QUEUE_HANDLE_PARM_MOD);
	queue_the_data(&_handle, sizeof(int));
	queue_the_data(&_track, sizeof(int));
	queue_the_data(&_value, sizeof(int));
	queue_the_data(&_function, sizeof(WessAction));
	wess_enable();
}

void run_queue_wess_handle_parm_mod(void) // 80034C48
{
	int handle;
	int track;
	int value;
	WessAction function;

	unqueue_the_data(&handle, sizeof(int));
	unqueue_the_data(&track, sizeof(int));
	unqueue_the_data(&value, sizeof(int));
	unqueue_the_data(&function, sizeof(WessAction));
	queue_wess_handle_parm_mod(handle, track, value, function);
}

void queue_wess_handle_parm_mod(int handle, int track, int value, WessAction function) // 80034CA8
{
	track_status *ptmp;
	char *ppos;

	int _handle;
	int _track;
	int _value;
	WessAction _function;

	_handle = handle;
	_track  = track;
	_value  = value;
	_function = function;

	if (Is_Module_Loaded())
	{
		wess_disable();
		ptmp = gethandletrk(_handle, _track);

		if (ptmp)
		{
			wess_track_parm_mod(ptmp, _value, _function);
		}

		wess_enable();
	}
}
#endif // 0
