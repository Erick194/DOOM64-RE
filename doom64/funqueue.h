#ifndef _FUNQUEUE_H
#define _FUNQUEUE_H

#define MAX_QUEUE_DATA_SIZE 8192
#define MAX_QUEUE_FUNCTION_SIZE 512

typedef struct
{
	char data[MAX_QUEUE_DATA_SIZE];			//800B4420
	char function[MAX_QUEUE_FUNCTION_SIZE];	//800B6420
} QUEUE_DATA_MEMORY;

extern void queue_memcpy(void *dest, void *src, int size);
extern void queue_the_function(char mode);
extern void queue_the_data(void *src, int size);
extern void unqueue_the_data(void *dest, int size);
extern void process_function_queue(void);
#endif
