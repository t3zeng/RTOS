#ifndef HALF_FIT_H_
#define HALF_FIT_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct node
{
	int val;
}node_t;

typedef struct block
{
	node_t *prev;
	node_t *next;
	node_t *block_size;
	bool *alloc_flag;
}block_t;

void  half_init( void );
void *half_alloc( int );
//void  half_free( void * );

#endif
