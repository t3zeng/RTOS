//REMINDER TO DOUBLE CHECK THE GETTERS AND SETTERS TO ENSURE THAT THEY PUT EVERYTHING IN THE 4 BYTE HEADER PROPERLY.
//Hours spent: 25
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
int main(void)
{
	/*
	half_init();
	printf("%d\n", get_prev(0));
	printf("%d\n", get_next(1));
	printf("%d\n", get_block_size(2));
	printf("%d\n", my_mem[2]);
	get_block_size(2);*/

	return 0;
}
//Series of get functions to retrieve values from the block of memory
U16 get_prev(int index)
{
	U16 prev = my_mem[index];
	prev = prev << 2;
	prev+=(my_mem[index+1] & 0xC0)>>6;
	return prev;
}
U16 get_next(int index)
{
	U16 next = my_mem[index] & 0x3F;
	next = next << 4;
	next+=(my_mem[index+1] & 0xF0)>>4;
	return next;
}
U16 get_block_size(int index)
{
	U16 block_size = my_mem[index] & 0xF;
	block_size = block_size << 6;
	block_size+=(my_mem[index+1] & 0xFC)>>2;
	return block_size;
}
U16 get_flag(int index)
{
	U16 flag = my_mem[index] & 0x2;
	flag = flag >> 1;
	return flag;
}
U16 get_prev_bucket(int index)
{
	U16 prev_bucket = my_mem[index];
	prev_bucket = prev_bucket << 2;
	prev_bucket +=(my_mem[index+1] & 0xC0) >> 6;
	return prev_bucket;
}
U16 get_next_bucket(int index)
{
	U16 next_bucket = my_mem[index] & 0x3F;
	next_bucket = next_bucket << 4;
	next_bucket+=(my_mem[index+1] & 0xC0) >> 4;
	return next_bucket;
}

//Series of set functions to store values to the block of memory
//Takes the index and a U16 value
void set_prev(int index, U16 prev)
{
	U16 temp = prev & 0x3;
	my_mem[index+1] += temp << 6;
	prev = prev >> 2;
	my_mem[index] += prev;
}
void set_next(int index, U16 next)
{
	U16 temp = next & 0xF;
	my_mem[index+1] += temp << 4;
	next = next >> 4;
	my_mem[index] += next;
}
void set_block_size(int index, U16 size)
{
	U16 temp = size & 0x3F;
	my_mem[index+1] += temp << 2;
	size = size >> 6;
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
	my_mem[index+1] += temp << 6;
	prev_bucket = prev_bucket >> 2;
	my_mem[index] += prev_bucket;
}
void set_next_bucket(int index, U16 next_bucket)
{
	U16 temp = next_bucket & 0xF;
	my_mem[index+1] += temp << 4;
	next_bucket = next_bucket >> 4;
	my_mem[index] += next_bucket;
}

//initialize a system with one bin of 32768 bytes of free memory with corresponding pointers
void half_init()
{
	//Initializes the first header block by using bit shifting
	//Puts the prev block as 10 bits at the very front
	set_prev(0, 0);

	//Puts the next block as 10 bits right beside the prev block
	set_next(1, 1);

	//Puts the block_size block as 10 bits right beside the next block
	set_block_size(2, 1024);

	//1 bit flag located right beside the block_size block
	set_flag(3, 0);

	//Puts the Previous block in bucket as 10 bits after
	set_prev_bucket(4, 4);

	//Puts the Next block in bucket as 10 bits after
	set_next_bucket(5, 5);

	//makes the last bucket (16384 - 32767) store the address of the first index in the header
	bucket[10] = 0;
}

//
void *half_alloc( int n )
{
	int i;
	if(n>32768)
		printf("INVALID INPUT");
	else
	{
		//divides n by 32 to quickly place things in a bucket
		int counter = 0;
		while((n > (16*(2<<counter)) && counter <= 10) || (bucket[counter] == NULL && counter <= 10))
		{
			counter++;
		}
		printf("%d\n",counter);

		//No space is available for allocation of a block of that size
		if(counter == 11)
			return;

		if (n + 4 > 0 && n + 4 <= 16*(2<<counter))
		{
			//This block of code creates a copy of the header of free mem and shifts it to the end of the newly alloced mem
			//the size is changed to reflect the new smaller size of the free mem
			//Difference of bucket and &my_mem[0] allows us to get the index in the array to store the new headers
			int index = bucket[counter];

			//sets each part of the header 1 index from each other starting at index+n+4 which is the end of the now allocated n bytes of memory
			set_prev(index+n+4, index);
			set_next(index+n+5, get_next(index+1));
			set_block_size(index+n+6, get_block_size(index+2)-(n/32));
			set_flag(index+n+7, get_flag(index+3));
			set_prev_bucket(index+n+8, NULL); // change NULL to the address of the last block of memory in the bucket
			set_next_bucket(index+n+9, get_next_bucket(index+5));

			//Checks if the remaining memory after allocation is enough to form a new block of free memory with a header and creates them
			if(get_block_size(index+6)-n > 16*(2<<counter) + 4)
			{
				//change bucket[counter]'s address to newly located header
				bucket[counter] += n+4;
			}
			//handles the case where the new the memory must be placed in a smaller bucket
			else if(get_block_size(index+n+6) - n < 16*(2<<counter) + 4)
			{
				int temp = bucket[counter];
				//since the new block size has been confirmed to be smaller, counter is adjusted to reflect the change
				while(get_block_size(index+n+6) < (16*(2<<counter)) && counter >= 0)
				{
					counter--;
				}
				//sets the address of the new bucket to where the new block of memory is
				bucket[counter] = temp;
			}
			//in the event that there isn't enough remaining free memory to create a new block, the bucket will now point to the next free memory block in the bucket
			else if(get_next_bucket(index+5) != NULL)
			{
				bucket[counter] = get_next_bucket(index+5);
			}
			//if there are no more free blocks of memory left, the bucket will be empty and return NULL when called
			else
			{
				bucket[counter] = NULL;
			}
			//changes the size of the new blocks accordingly, set the flags and relocate pointers
			set_block_size(index+2, n/32);
			set_flag(index+3, 1);
			set_next(index+1, &my_mem[0]+index+n+4);
		}
	}
}

void half_free( void * mem)
{
	int index = mem;

	//set flag to unallocated
	set_flag(index+3, 0);
	//checks the block ahead to see if it is empty and can be combined
	if(get_flag(index+3+(get_block_size(index+2)))==0)
	{
		//eliminate headers of adjacent block and adjust the size and pointers
		set_block_size(index+2, get_block_size(index+2)+get_block_size(get_next(index+1)+2));
		set_next(index+1, get_next(get_next(index+1)));
		set_prev(get_next(get_next(index+1)), get_prev(index));
		//manipulate buckets if necessary (this includes changing bucket pointers)

		//divides n by 32 to quickly determine the bucket in which the new block belongs in
		int counter = 0;
		while((get_block_size(index+2) > (16*(2<<counter)) && counter <= 10) || (bucket[counter] == NULL && counter <= 10))
		{
			counter++;
		}
		//new size belongs in the same bucket
		if(bucket[counter])
		{

		}
		//new size belongs in a different bucket
		else if(0)
		{
			//connect the mem on the left and right together
			set_next_bucket(get_prev_bucket(index+4+(get_block_size(index+2)))+5, get_next_bucket(index+5+(get_block_size(index+2))));
			set_prev_bucket(get_next_bucket(index+5+(get_block_size(index+2)))+4, get_prev_bucket(index+4+(get_block_size(index+2))));
		}
	}
	//checks the block behind to see if it is empty and can be combined
	if(get_flag(get_prev(index)+3)==0)
	{
		//eliminate headers of adjacent block and adjust the size and pointers
		set_block_size(get_prev(index)+2, get_block_size(get_prev(index)+2)+get_block_size(index+2));
		set_next(get_prev(index)+1, get_next(get_next(index+1)));
		set_prev(get_next(index+1), get_prev(index));
		//manipulate buckets if necessary (this includes changing bucket pointers)
	}
	//change pointers from the previous and next block so they point to the right blocks
	//add the new block to the bucket
}
