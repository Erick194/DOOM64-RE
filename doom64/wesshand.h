#ifndef _WESSTRAK_H
#define _WESSTRAK_H

#include "wessapi.h"
#include "wessarc.h"

extern sequence_status *gethandleseq(int handle); // 80032E80
extern sequence_status *gethandleseq(int handle); // 80032E80
extern int wess_handle_get(int seq_num); // 80032F80
extern int wess_handle_status(int handle); // 80032FD8
extern void wess_handle_return(int handle); // 80033048
extern void wess_handle_play_special(int handle, TriggerPlayAttr *attr); // 80033180
extern void wess_handle_play(int handle); // 80033298()
extern void wess_handle_play_track_special(int handle, int track, TriggerPlayAttr *attr); // 800332B8
extern void wess_handle_play_track(int handle, int track); // 80033344
extern void wess_handle_get_special(int handle, TriggerPlayAttr *attr); // 80033364
extern void wess_handle_set_special(int handle, TriggerPlayAttr *attr); // 80033454
extern void wess_handle_stop(int handle); // 80033544
extern void wess_handle_fastsettempo(int handle, short tempo); // 80033658
extern int wess_handle_fade(void); // 80033788
extern void wess_handle_resetposition(int handle); // 800337B0
extern int wess_track_gotoposition(track_status *ptmp, int position, char *ppos); // 800338F0
extern int wess_track_setposition(track_status *ptmp, int position, char *ppos); // 80033B24
extern int wess_handle_setposition(int handle, int position); // 80033EB8
extern int wess_handle_getposition(int handle); // 80034010
extern void patch_chg_action(track_status *ptmp, short patch_num); // 800340E0
extern void pitch_mod_action(track_status *ptmp, int pitch_cntr); // 80034144
extern void volume_mod_action(track_status *ptmp, int volume_cntr); // 800341A8
extern void pan_mod_action(track_status *ptmp, int pan_cntr); // 80034200
extern void wess_track_parm_mod(track_status *ptmp, int value, WessAction funcion); // 80034258

#endif
