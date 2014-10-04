#include "half_fit.h"
#include <stdint.h>
#include <stdio.h>

//Create size types for easy use later on in the code
typedef signed char S8;
typedef unsigned char U8;
typedef signed short S16;
typedef unsigned short U16;
typedef signed int S32;
typedef unsigned int U32;
typedef signed long S64;
typedef unsigned long U64;

//Initialize 32K bytes of memory
U8 my_mem [32768];

//INITIALIZE THE BUCKETS
int bucket[10] = {NULL};

//my base address of the block
// &my_mem;
int main(void)
{
	int i;
	half_init();
	printf("%d\n", my_mem[0]);
	U16 test = "asdffdsa";
	my_mem[0] = test;
	for(i=0;i<10;i++)
		printf("%d\n", my_mem[i]);
	printf("Run Successful!");

	return 0;
}
//Series of get functions to retrieve values from the block of memory
U16 get_prev(int index)
{
	U16 prev = my_mem[index];
	prev << 2;
	prev+=(my_mem[index+1] & 0xC0)>>6;
	return prev;
}
U16 get_next(int index)
{
	U16 next = my_mem[index] & 0x3F;
	next << 4;
	next+=(my_mem[index+1] & 0xF0)>>4;
	return next;
}
U16 get_block_size(int index)
{
	U16 block_size = my_mem[index] & 0xF;
	block_size << 6;
	block_size+=(my_mem[index+1] & 0xFC)>>2;
	return block_size;
}
U16 get_flag(int index)
{
	U16 flag = my_mem[index] & 0x2;
	flag >> 1;
	return flag;
}
U16 get_prev_bucket(int index)
{
	U16 prev_bucket = my_mem[index];
	prev_bucket << 2;
	prev_bucket +=(my_mem[index+1] & 0xC0) >> 6;
	return prev_bucket;
}
U16 get_next_bucket(int index)
{
	U16 next_bucket = my_mem[index] & 0x3F;
	next_bucket << 4;
	next_bucket+=(my_mem[index+1] & 0xC0) >> 4;
	return next_bucket;
}

//Series of set functions to store values to the block of memory
//Takes the index and a U16 value
void set_prev(int index, U16 prev)
{
	U16 temp = prev & 3;
	my_mem[index+1] = temp << 6;
	prev >> 2;
	my_mem[index] += prev;
}
void set_next(int index, U16 next)
{
	U16 temp = next & 0xF;
	my_mem[index+1] = temp << 4;
	next >> 4;
	my_mem[index] += next;
}
void set_block_size(int index, U16 size)
{
	U16 temp = size & 0x3F;
	my_mem[index+1] = temp << 2;
	size >> 6;
	my_mem[index] += size;
}
void set_flag(int index, U16 flag)
{
	flag = flag & 0x2;
	my_mem[index] += my_mem[index] + flag;
}
void set_prev_bucket(int index, U16 prev_bucket)
{
	U16 temp = prev_bucket & 3;
	my_mem[index+1] = temp << 6;
	prev_bucket >> 2;
	my_mem[index] += prev_bucket;
}
void set_next_bucket(int index, U16 next_bucket)
{
	U16 temp = next_bucket & 0xF;
	my_mem[index+1] = temp << 4;
	next_bucket >> 4;
	my_mem[index] += next_bucket;
}

//initialize a system with one bin of 32768 bytes of free memory with corresponding pointers
void half_init()
{
	//Initializes the first header block by using bit shifting
	//Puts the prev block as 10 bits at the very front
	set_prev(0, &my_mem[0]);

	//Puts the next block as 10 bits right beside the prev block
	set_next(1, &my_mem[1]);

	//Puts the block_size block as 10 bits right beside the next block
	set_block_size(2, 1024);

	//1 bit flag located right beside the block_size block
	set_flag(3, 0);

	//Puts the Previous block in bucket as 10 bits after
	set_prev_bucket(4, &my_mem[4]);

	//Puts the Next block in bucket as 10 bits after
	set_next_bucket(5, &my_mem[5]);

	//makes the last bucket (16384 - 32767) store the address of the next_bucket block
	bucket[10] = &my_mem[5];
}

//
void *half_alloc( int n )
{
	//series of if statements to determine which bucket to take memory from (4 bytes are taken out for use with the header)
	if (n + 4 > 0 && n + 4 <= 32 && bucket[0] != NULL)
	{
		set_prev(int index, U16 prev);
	}

	else if(n + 4 <= 64 && bucket[1] != NULL)

	else if(n + 4 <= 128 && bucket[2] != NULL)

	else if(n + 4 <= 256 && bucket[3] != NULL)

	else if(n + 4 <= 512 && bucket[4] != NULL)

	else if(n + 4 <= 1024 && bucket[5] != NULL)

	else if(n + 4 <= 2048 && bucket[6] != NULL)

	else if(n + 4 <= 4096 && bucket[7] != NULL)

	else if(n + 4 <= 8192 && bucket[8] != NULL)

	else if(n + 4 <= 16384 && bucket[9] != NULL)

	else if(n + 4 <= 32768 && bucket[10] != NULL)

	else
		printf("C'EST IMPOSSIBLE!! SACRE BLEU?!?!?");
}
/*
void half_free( void * )
{
}*/
