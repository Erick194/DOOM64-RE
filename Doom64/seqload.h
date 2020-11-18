    /*------------------------------------------------------------------*/
    /*
                     The Williams Entertainment Sound System
                            by Scott Patterson
    */
    /*------------------------------------------------------------------*/

#ifndef _SEQLOAD_H
#define _SEQLOAD_H

enum Seq_Load_Error {

    SEQLOAD_NO_ERROR,
    SEQLOAD_FOPEN,
    SEQLOAD_FREAD,
    SEQLOAD_FSEEK
};

enum OpenSeqHandleFlag {NoOpenSeqHandle,YesOpenSeqHandle};

    /*
        routine: wess_seq_loader_init()

        - this routine must be called before and seq_loader_routines
        - input_pm_stat is returned by wess_get_master_status()
        - seqfile is the .wmd filename
        - flag specifies if the file handle will be opened during init or
          each time file is accessed
    */

extern int wess_seq_loader_sizeof( void *input_pm_stat,
				   char *seqfile );

extern int wess_seq_loader_init( void *input_pm_stat,
				 char *seqfile, 
				 enum OpenSeqHandleFlag flag,
				 char *memory_pointer,
				 int   memory_allowance);

extern int wess_seq_loader_count( void );

    /*
        routine: wess_seq_loader_exit()

        - closes file handle if not already closed
        - disables sequence loading calls
    */

extern void wess_seq_loader_exit(void);

    /*
        routine: wess_seq_loader_install_error_handler()

        - for installing an error callback to notify file access errors
        - module is your own ID returned as err_module parameter
        - err_enum is the returned Seq_Load_Error enum parameter
    */

extern void wess_seq_loader_install_error_handler(int (*error_func)(int, int),
                                                  int module);

    /*
        general loading guidelines:

        - sizeof functions return the amount of bytes needed for data
          not already loaded, therefore, when sizeof returns 0, this
          means the data referred to is already loaded

        - load functions only load data that is not already loaded
          and return the amount of bytes loaded, memory is not allocated
          internally, you must use the sizeof functions and allocate
          memory yourself

        - free functions mark data as not loaded, memory is not freed
          internally, you must free memory yourself
    */

    /*
        individual sequence loading
    */

extern int wess_seq_sizeof(int seqnum);

extern int wess_seq_load(int seqnum,void *memptr);

extern int wess_seq_free(int seqnum);

    /*
        sequence list loading

        - pass in a list of sequnce numbers to be loaded
        - end this list with the END_SEQ_LIST define
    */

#define END_SEQ_LIST -1

extern int wess_seq_list_sizeof(short *seqlist);

extern int wess_seq_list_load(short *seqlist,void *memptr);

extern int wess_seq_list_free(short *seqlist);

    /*
        sequence range loading

        - specify a number of consecutive sequences to be loaded
    */

extern int wess_seq_range_sizeof(int seqfirst,int numseqs);

extern int wess_seq_range_load(int seqfirst,int numseqs,void *memptr);

extern int wess_seq_range_free(int seqfirst,int numseqs);

#endif

