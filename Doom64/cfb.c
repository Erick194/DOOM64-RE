
/*
 * RSP view of the frame buffer.  It exists to create an RSP address for
 * the frame buffer, which is remapped on each frame to either of two
 * regions of physical memory that store that actual bits.
 */

#include <ultra64.h>
#include "i_main.h"

u32 cfb[2][SCREEN_WD*SCREEN_HT]; // 8036A000



