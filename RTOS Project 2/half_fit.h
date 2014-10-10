#ifndef HALF_FIT_H_
#define HALF_FIT_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef signed char S8;
typedef unsigned char U8;
typedef signed short S16;
typedef unsigned short U16;
typedef signed int S32;
typedef unsigned int U32;
typedef signed long S64;
typedef unsigned long U64;

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
U16 get_prev(int index);
U16 get_next(int index);
U16 get_block_size(int index);
U16 get_flag(int index);
U16 get_prev_bucket(int index);
U16 get_next_bucket(int index);
void set_prev(int index, U16 prev);
void set_next(int index, U16 next);
void set_block_size(int index, U16 size);
void set_flag(int index, U16 flag);
void set_prev_bucket(int index, U16 prev_bucket);
void set_next_bucket(int index, U16 next_bucket);
//void  half_free( void * );

#endif
