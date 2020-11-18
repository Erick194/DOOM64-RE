
#ifndef _WESSINT_S_H
#define _WESSINT_S_H

#include <ultra64.h>

extern unsigned long wesssys_disable_ints(void);
extern void wesssys_restore_ints(unsigned long state);

#endif
