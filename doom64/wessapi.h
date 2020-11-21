    /*------------------------------------------------------------------*/
    /*
                     The Williams Entertainment Sound System
                    Application Programming Interface Routines
                            by Scott Patterson
    */
    /*------------------------------------------------------------------*/

#ifndef _WESSAPI_H
#define _WESSAPI_H

#include <ultra64.h>

    /*------------------------------------------------------------------*/
    /*
        Enumerated types.
    */
    /*------------------------------------------------------------------*/

/* used by wess_seq_pause and wess_seq_pauseall functions */
enum MuteFlag { NoMute, YesMute };

/* used by wess_seq_restartall function */
enum VoiceRestartFlag { NoVoiceRestart, YesVoiceRestart };

/* used by wess_sequence_status function */
enum SequenceStatus { SEQUENCE_INVALID,
		      SEQUENCE_INACTIVE,
		      SEQUENCE_STOPPED,
		      SEQUENCE_PLAYING };

    /*------------------------------------------------------------------*/
    /*------------------------------------------------------------------*/
    /*
        System Setup and Shutdown Functions.
    */
    /*------------------------------------------------------------------*/
    /*------------------------------------------------------------------*/


    /*
        wess_set_error_callback - set a callback for runtime audio errors

        passing NULL removes the callback

        the errstring passed to the callback is guaranteed to be less than 12
        uppercase letter characters only, thus you can print the errors
        with a very simple print routine.

        the errnum1 and errnum2 ints passed to the callback may or may not
        have meaning based on the errstring
    */

typedef void (*WessErrorCallbackProc)( char *errstring, int errnum1, int errnum2 );

extern void wess_set_error_callback( WessErrorCallbackProc errcall );

    /*
        wess_set_decompression_callback - set a callback for runtime decompression

	passing NULL removes the callback

        If the sound system source files are compressed, this callback will happen
        inside the following functions:

        wess_load_module

        wess_seq_loader_init

        wess_seq_load, wess_seq_range_load, or wess_seq_list_load

	when the callback occurs you must run the decompression type
	that matches the decomp_type parameter from the source file at a given
	offset into memory.

	if the decompression is not available or something goes wrong, then return
	a negative number and the sound system call that issued the callback will
	abort.
    */

#define NO_DECOMPRESSION            0  /* you will never actually get this as a decomp type */
#define MARKG_INFLATE_DECOMPRESSION 1

typedef int (*WessDecompCallbackProc)( unsigned char decomp_type,
				       char          *fileref,
				       unsigned long file_offset,
				       char          *ramdest,
				       unsigned long uncompressed_size );

extern void wess_set_decomp_callback( WessDecompCallbackProc decompcall );

    /*
        wess_set_tweak - set the tweak parameters for the audio engine

        If you need to change audio system parameters, wess_tweak allows this.

        If you don't know what these parameters do, don't change them!

        This call must be made before wess_init to have any effect.

        Use the mask to set particlar parameters.
    */

#define TWEAK_DMA_BUFFERS          (0x1L<< 0)
#define TWEAK_DMA_MESSAGES         (0x1L<< 1)
#define TWEAK_DMA_BUFFER_LENGTH    (0x1L<< 2)
#define TWEAK_EXTRA_SAMPLES        (0x1L<< 3)
#define TWEAK_FRAME_LAG            (0x1L<< 4)
#define TWEAK_VOICES               (0x1L<< 5)
#define TWEAK_UPDATES              (0x1L<< 6)
#define TWEAK_SEQUENCES            (0x1L<< 7)
#define TWEAK_TRACKS               (0x1L<< 8)
#define TWEAK_GATES                (0x1L<< 9)
#define TWEAK_ITERS                (0x1L<< 10)
#define TWEAK_CALLBACKS            (0x1L<< 11)
#define TWEAK_MAX_TRKS_PER_SEQ     (0x1L<< 12)
#define TWEAK_MAX_SUBS_PER_TRK     (0x1L<< 13)

typedef struct {
                    unsigned long mask;
                    unsigned long dma_buffers;        /* default is 24    */
                    unsigned long dma_messages;       /* default is 32    */
                    unsigned long dma_buffer_length;  /* default is 0x800 */
                    unsigned long extra_samples;      /* default is 80    */
                    unsigned long frame_lag;          /* default is 1     */
                    unsigned long voices;             /* default is 24    */
                    unsigned long updates;            /* default is 48    */
                    unsigned long sequences;          /* default is 26    */
                    unsigned long tracks;             /* default is 25    */
                    unsigned long gates;              /* default is 0     */
                    unsigned long iters;              /* default is 0     */
                    unsigned long callbacks;          /* default is 0     */
                    unsigned long max_trks_per_seq;   /* default is 16    */
                    unsigned long max_subs_per_trk;   /* default is 0     */
               } WessTweakAttr;

extern void wess_set_tweaks(WessTweakAttr *attr);

    /*
        wess_get_tweak - get the tweak parameters for the audio engine

        The given WessTweakAttr structure will updated to the current parameters.

        This function simply fills the WessTweakAttr structure and sets mask = 0.
    */

extern void wess_get_tweaks(WessTweakAttr *attr);


    /*
        wess_init - Initializes the Williams Entertainment Sound System.

        config is a pointer to the WessConfig structure that holds the
        following information:

        audioframerate   - the frequency (number of times per second) the
                           wess_work function will be called. Typically this
                           will be 30 (30fps) or 60 (60fps).

        outputsamplerate - this is the output sample rate of the audio data
                           the higher this number, the higher the possible
                           frequency response of the audio, but the more data
                           that the RSP must calculate.

        maxACMDSize      - the number of commands allowed for the audio
                           command list sent to the RSP.  If this number is
                           too small, memory will be trashed.  Typical values
                           are 3072 (for 30fps) or 2048 (for 60fps)

        heap_ptr         - pointer to an audio heap pointer set up with the
                           alHeapInit call.

        revtbl_ptr       - pointer to a reverb settings table for effects initialization.
                           this MUST be set to zero if no effects are to be turned on.
    */

#define    WESS_REVERB_NONE          0
#define    WESS_REVERB_SMALLROOM     1
#define    WESS_REVERB_BIGROOM       2
#define    WESS_REVERB_CHORUS        3
#define    WESS_REVERB_FLANGE        4
#define    WESS_REVERB_ECHO          5
#define    WESS_REVERB_CUSTOM        6

typedef struct WessConfig {     /* see wess_init function for more info */
    f32       audioframerate;   /* the number of times per second that wesswork is called */
    u32       outputsamplerate; /* the output samplerate to calculate */
    u32       maxACMDSize;      /* the size of the audio command list buffers */
    ALHeap   *heap_ptr;         /* audio heap pointer */
    char     *wdd_location;     /* cart location of wdd file */
    s32       reverb_id;        /* choose from WESS_REVERB defines */
    s32      *revtbl_ptr;       /* reverb table pointer (used if reverb_id == WESS_REVERB_CUSTOM */
} WessConfig;

extern void wess_init(WessConfig *wessconfig);

    /*
        wess_rom_copy - U64 rom copy

        after wess_init has been called, you can call wess_rom_copy

        - this is a blocking rom copy

        - returns 0 and copies nothing if the request was for zero bytes or if
          the wess_init call has not been made
        - returns length copied if length is non zero
    */

extern int wess_rom_copy(char *src, char *dest, int len);

    /*
        wess_exit - call this to shutdown the audio system
    */

extern void wess_exit(void);

    /*
        wess_work - performs ALL CPU audio work for application

        This function must be called at a constant frequency, as set up
        by the WessConfig.audioframerate parameter in the wess_init function.
        It handles all audio work processing and returns a pointer to an RSP
        audio task, or 0 if no RSP processing is required.

        The task returned by wess_work must be given to the RSP as quickly
        as possible so that the RSP audio microcode has completed building
        the audio frame buffer in time for the next call to wess_work.
        Typically this requires that an in-progress RSP graphics task must
        yield, and will resume when the RSP completes the audio task.
    */

extern OSTask * wess_work(void);

    /*
        wess_master_fade - Master volume fade.

        This function smoothly fades the master volume of all sound
        effects and music channels from the current volume to dest_vol.
        The transition will complete in millisec milliseconds. Returns the
        current master volume level prior to the call.

        dest_vol should be 0 - 127
    */

extern int wess_master_fade(char dest_vol, int millisec);

    /*
        routine: wess_load_module()

        - loads master table of sounds and sequences
        - returns 0 for failure, 1 for success
        - NULL for memory_pointer results in internal memory allocation
          of memory_allowance bytes
        - if memory_pointer!=NULL then it is assumed to be a pointer
          to memory_allocation bytes of memory for use by the sound system
        - enables sequencer engine
    */

extern int wess_size_module (char *wmd_filename);

extern int wess_load_module (char *wmd_filename,
                             char *memory_pointer,
                             int   memory_allowance);

    /*
        routine: wess_unload_module()

        - disables sequencer engine
        - frees any allocated memory
    */

extern void wess_unload_module (void);

    /*
        routine: wess_get_wmd_start()

        - after call to wess_load_module is successful, this
          gets the pointer to beginning of module block
    */

extern char *wess_get_wmd_start (void);

    /*
        routine: wess_get_wmd_end()

        - after call to wess_load_module is successful, this
          gets the pointer past end of module block
    */

extern char *wess_get_wmd_end (void);

    /*------------------------------------------------------------------*/
    /*------------------------------------------------------------------*/
    /*
        Sequencer calls.
    */
    /*------------------------------------------------------------------*/
    /*------------------------------------------------------------------*/

    /*
        routine: wess_seq_trigger()

        - a sequence is everything from a single sound to a music sequence.
        - multiple sequences can be called simultaneously
        - dynamic sequence, track, and voice allocation is managed
          by the sequencer
        - tracks have priorty assignments and voices triggered by a
          track inheirit its priority
        - you can trigger multiple instances of a sequence with multiple
          trigger calls
    */

#define TRIGGER_VOLUME   (0x1L<< 0)
#define TRIGGER_PAN      (0x1L<< 1)
#define TRIGGER_PATCH    (0x1L<< 2)
#define TRIGGER_PITCH    (0x1L<< 3)
#define TRIGGER_MUTEMODE (0x1L<< 4)
#define TRIGGER_TEMPO    (0x1L<< 5)
#define TRIGGER_TIMED    (0x1L<< 6)
#define TRIGGER_LOOPED   (0x1L<< 7)
#define TRIGGER_REVERB   (0x1L<< 8)

typedef struct  {
                    unsigned long   mask;
                    unsigned char   volume;   /* 0-127 */
                    unsigned char   pan;      /* 0-127, 64 center */
                    short           patch;    /* 0-32767 */
                    short           pitch;    /* -8192 to 8191 */
                    unsigned char   mutemode; /* 0-7 */
                    unsigned char   reverb;
                    unsigned short  tempo;
                    unsigned long   timeppq;
                } TriggerPlayAttr;

    /* the basic sequence trigger call */

extern void wess_seq_trigger         (int seq_num);

    /* override masked sequence parameters */

extern void wess_seq_trigger_special (int              seq_num,
                                      TriggerPlayAttr *attr);

    /* set your own type number to the sequence */

extern void wess_seq_trigger_type         (int           seq_num,
                                           unsigned long seq_type);

    /* set your own type number to the sequence and
       override masked sequence parameters          */

extern void wess_seq_trigger_type_special (int              seq_num,
                                           unsigned long    seq_type,
                                           TriggerPlayAttr *attr);

extern void wess_seq_update_type_special  (unsigned long    seq_type,
                                           TriggerPlayAttr *attr);

    /*
        routine: wess_seq_status()

        - find out status of any instances of sequences_number
        - returns SEQUENCE_INVALID for no such sequence
                  SEQUENCE_INACTIVE for is not being processed
                  SEQUENCE_STOPPED for sequence is stopped
                  SEQUENCE_PLAYING for sequence is playing
    */

extern int wess_seq_status (int sequence_number);

    /*
        routine: wess_seq_type_status()

        - find out status of any instances of sequence_type
        - returns SEQUENCE_INACTIVE for no sequence of this type is being processed
                  SEQUENCE_STOPPED for a sequence of this type is stopped
                  SEQUENCE_PLAYING for a sequence is this type is playing
    */

extern int wess_seq_type_status (unsigned long sequence_type);

    /*
        routine: wess_seq_stop()

        - stops all instances of a specified sequence
    */

extern void wess_seq_stop (int sequence_number);

extern void wess_seq_stop_and_voiceramp (int sequence_number,int millisec);

    /*
        routine: wess_seq_stoptype()

        - immediate stop of all sequences that were registered as a certain
          type by wess_seq_trigger_type
    */

extern void wess_seq_stoptype (unsigned long sequence_type);

extern void wess_seq_stoptype_and_voiceramp (unsigned long sequence_type,int millisec);

    /*
        routine: wess_seq_stopall()

        - immediate stop of all sequences
    */

extern void wess_seq_stopall (void);

extern void wess_seq_stopall_and_voiceramp (int millisec);

    /*
        routine: wess_seq_pause()

        - immediately pause a sequence
        - if MuteFlag is YesMute, all playing voices are muted
        - if MuteFlag is NoMute, all playing voices do note decay
    */

extern void wess_seq_pause (int           sequence_number,
                            enum MuteFlag mflag);

    /*
        routine: wess_seq_restart()

        - restarts a paused sequence
    */

extern void wess_seq_restart (int sequence_number);

    /*
        routine: wess_seq_stopall()

        - immediate pause of all sequences
        - if mflag is YesMute, all playing voices are muted
        - if mflag is NoMute, all playing voices remain on
        - if remember bits are set then corrosponding voices
          will restore when wess_seq_restartall is called
    */

#define REMEMBER_MUSIC   (0x1L<< 0)
#define REMEMBER_SNDFX   (0x1L<< 1)

extern void wess_seq_pauseall (enum MuteFlag mflag, int remember);

    /*
        routine: wess_seq_restartall()

        - restart all paused sequences
        - if restart_remembered voices is YesVoiceRestart, all the
          voices remembered at pauseall time will restart
    */

extern void wess_seq_restartall (enum VoiceRestartFlag restart_remembered_voices);

    /*
        routine: wess_register_callback()

        - callbacks are useful for video and program flow synchronization
          with audio.
        - callbacks are called when any track of a playing sequence
          encounters a StatusMark with a registered marker_ID
        - the first parameter of the callback is the marker_ID encountered
        - the second parameter is the StatusMark's data field
    */

extern void wess_register_callback (char   marker_ID,
                                    void (*function_pointer)(char,short));

    /*
        routine: wess_delete_callback()

        - stop any callbacks from a particular marker_ID
    */

extern void wess_delete_callback (char marker_ID);

    /*------------------------------------------------------------------*/
    /*------------------------------------------------------------------*/
    /*
        Master functions get and set global sound parameters.
    */
    /*------------------------------------------------------------------*/
    /*------------------------------------------------------------------*/

    /*
        routine: wess_master_sfx_volume_get()
        routine: wess_master_mus_volume_get()

        - gets the master volume
    */

extern char wess_master_sfx_volume_get (void);
extern char wess_master_mus_volume_get (void);

    /*
        routine: wess_master_sfx_vol_set()
        routine: wess_master_mus_vol_set()

        - sets the master volume
    */

extern void wess_master_sfx_vol_set (char volume);
extern void wess_master_mus_vol_set (char volume);

    /*
        routine: wess_pan_mode_get()

        - gets the pan mode where 0=off, 1=normal, 2=switchLR
    */

extern char wess_pan_mode_get (void);

    /*
        routine: wess_pan_mode_set()

        - sets the pan mode where 0=off, 1=normal, 2=switchLR
    */

extern void wess_pan_mode_set (char mode);

    /*
        routine: wess_set_mute_release(millisec)

        - sets the stop/mute release time given milliseconds
    */

extern void wess_set_mute_release(int millisec);

    /*
        routine: wess_get_mute_release(millisec)

        - gets the stop/mute release time given milliseconds
    */

extern int wess_get_mute_release(void);

    /*
        routine: wess_master_status()

        - this returns a pointer to low level data structures
        - some function calls need this pointer
    */

extern void * wess_get_master_status (void);

#endif




