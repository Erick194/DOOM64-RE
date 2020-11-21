#ifndef _WESSEDIT_H
#define _WESSEDIT_H

#include "wessarc.h"

extern void wess_handle_free_edit_space(void); // 80034D20
extern char *wess_get_edt_start(void); // 80034D70
extern int wess_get_edt_size(void); // 80034D80
extern void wess_handle_set_edit_tracks(int labels, int soundfx, int music, int drums); // 80034D90
extern int wess_handle_create_edit_space(char *memory_pointer, int data_size, int memory_allowance); // 80034DB4

#endif
