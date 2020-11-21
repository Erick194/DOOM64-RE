#ifndef _WESSTRAK_H
#define _WESSTRAK_H

#include "wessarc.h"

extern void wess_handle_patchchg(int handle, int track, short patchnum); // 80034664
extern void wess_handle_noteon(int handle, int track, char keynum, char velnum); // 8003469C
extern void run_queue_wess_handle_noteon(void); // 80034708
extern void run_queue_wess_handle_noteon(void); // 80034708
extern void noteoff_action(track_status *ptmp, int keynum); // 8003483C
extern void wess_handle_noteoff(int handle, int track, char keynum); // 80034894
extern void wess_handle_noteoff(int handle, int track, char keynum); // 80034894
extern void zero_mod_action(track_status *ptmp, int value); // 800348F0
extern void wess_handle_zeromod(int handle, int track, char value); // 80034948
extern void wess_handle_zeromod(int handle, int track, char value); // 80034948
extern void wess_handle_zeromod(int handle, int track, char value); // 80034948
extern void wess_handle_zeromod(int handle, int track, char value); // 80034948
extern void wess_handle_panmod(int handle, int track, char pan_cntr); // 80034A24
extern void pedal_mod_action(track_status *ptmp, int value); // 80034A50
extern void wess_handle_pedalmod(int handle, int track, char value); // 80034AA8
extern void wess_handle_pedalmod(int handle, int track, char value); // 80034AA8
extern void wess_handle_reverbmod(int handle, int track, char reverb); // 80034B2C
extern void chorus_mod_action(track_status *ptmp, int chorus); // 80034B58
extern void wess_handle_chorusmod(int handle, int track, char chorus); // 80034BB0
extern void wess_handle_chorusmod(int handle, int track, char chorus); // 80034BB0
extern void run_queue_wess_handle_parm_mod(void); // 80034C48
extern void run_queue_wess_handle_parm_mod(void); // 80034C48

#endif
