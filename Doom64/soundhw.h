    /*------------------------------------------------------------------*/
    /*
                     The Williams Entertainment Sound System
                            by Scott Patterson
    */
    /*------------------------------------------------------------------*/

#ifndef _SOUNDHW_H
#define _SOUNDHW_H

enum DriverIds {NoSound_ID,N64_ID,GENERIC_ID=50};

enum SoundHardwareTags {
    SNDHW_TAG_END,
    SNDHW_TAG_DRIVER_ID,
    SNDHW_TAG_SOUND_EFFECTS,
    SNDHW_TAG_MUSIC,
    SNDHW_TAG_DRUMS,
    SNDHW_TAG_MAX
};

#endif