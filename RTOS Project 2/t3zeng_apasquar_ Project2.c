//REMINDER TO DOUBLE CHECK THE GETTERS AND SETTERS TO ENSURE THAT THEY PUT EVERYTHING IN THE 4 BYTE HEADER PROPERLY.
//Hours spent: 48
#include "half_fit.h"
#include <stdint.h>
#include <stdio.h>
#include <math.h>

//Create size types for easy use later on in the code
typedef signed char S8;
typedef unsigned char U8;
typedef signed short S16;
typedef unsigned short U16;
typedef signed int S32;
typedef unsigned int U32;
typedef signed long S64;
typedef unsigned long U64;

//Initialize 32K bytes of memory (changed to using U32 and 8192 indices so each one is 32 bits)
U32 my_mem [8192];

//INITIALIZE THE BUCKETS
int bucket[11] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

//my base address of the block

int main(void)
{
	half_init();
	half_alloc(3121);
	half_alloc(512);
	half_alloc(5212);

	int i;
	printf("Buckets:\n");
	for(i=0; i<= 10; i++)
		printf("Address of bucket %d is: %d\n",i, bucket[i]);
	int index = 0;
	int counter = 0;
	while(get_next(index) != index)
	{
		printf("\n\nHeader of the %dth block:\n", counter+1);
		printf("prev: %d ", get_prev(index));
		printf("next: %d ", get_next(index));
		printf("size: %d ", get_block_size(index));
		printf("flag: %d ", get_flag(index));
		if(!get_flag(index))
		{
			printf("prev bucket: %d ", get_prev_bucket(index));
			printf("next bucket: %d ", get_next_bucket(index));
		}

		index = get_next(index);
		counter++;
	}
	printf("\n\nHeader of the %dth block:\n", counter+1);
	printf("prev: %d ", get_prev(index));
	printf("next: %d ", get_next(index));
	printf("size: %d ", get_block_size(index));
	printf("flag: %d ", get_flag(index));
	if(!get_flag(index))
	{
		printf("prev bucket: %d ", get_prev_bucket(index));
		printf("next bucket: %d ", get_next_bucket(index));
	}

	return 0;
}

//Series of get functions to retrieve values from the block of memory
U32 get_prev(int index)
{
	return  (my_mem[index] >> 22) << 3;
}
U32 get_next(int index)
{
	return (my_mem[index] << 10 >> 22) <<3;
}
U32 get_block_size(int index)
{
	if(my_mem[index] << 20 >> 22 == 0)
		return 1024;
	return my_mem[index] << 20 >> 22;
}
U32 get_flag(int index)
{
	return my_mem[index] << 30 >> 31;
}
U32 get_prev_bucket(int index)
{
	return (my_mem[index+1] >> 22) << 3;
}
U32 get_next_bucket(int index)
{
	return (my_mem[index+1] << 10 >> 22) << 3;
}

//Series of set functions to store values to the block of memory
//Takes the index and a U32 value
void set_prev(int index, U32 prev)
{
	my_mem[index] = my_mem[index] & 0x3FFFFF;
	my_mem[index] += prev >> 3 << 22 ;
}
void set_next(int index, U32 next)
{
	my_mem[index] = my_mem[index] & 0xFFC00FFF;
	my_mem[index] += next >> 3 << 12;
}
void set_block_size(int index, U32 size)
{
	my_mem[index] = my_mem[index] & 0xFFFFF003;
	my_mem[index] += size << 2;
}
void set_flag(int index, U32 flag)
{
	my_mem[index] = my_mem[index] & 0xFFFFFFFD;
	my_mem[index] += flag << 1;
}
void set_prev_bucket(int index, U32 prev_bucket)
{
	my_mem[index+1] = my_mem[index+1] & 0x3FFFFF;
	my_mem[index+1] += prev_bucket >> 3 << 22;
}
void set_next_bucket(int index, U32 next_bucket)
{
	my_mem[index+1] = my_mem[index+1] & 0xFFC00FFF;
	my_mem[index+1] += next_bucket >> 3 << 12;
}

//initialize a system with one bin of 32768 bytes of free memory with corresponding pointers
void half_init()
{
	//Initializes the first header block by using bit shifting
	//Puts the prev block as 10 bits at the very front
	set_prev(0, 0);

	//Puts the next block as 10 bits right beside the prev block
	set_next(0, 0);

	//Puts the block_size block as 10 bits right beside the next block
	set_block_size(0, 0);

	//1 bit flag located right beside the block_size block
	set_flag(0, 0);

	//Puts the Previous block in bucket as 10 bits after
	set_prev_bucket(0, 0);

	//Puts the Next block in bucket as 10 bits after
	set_next_bucket(0, 0);

	//makes the last bucket (16384 - 32767) store the address of the first index in the header
	bucket[10] = 0;
}

//for all the index = (n/4), must make sure that it will always take pieces of 32 even if it means it is more than what is required
void *half_alloc( int n )
{
	int i;
	if(n>32764)
		return NULL;
	else
	{
		//moves the number up to the nearest 32 as memory can only be stored in 32s
		if(n%32 !=0)
		{
			n /= 32;
			n++;
			n *= 32;
		}
		//divides n by 32 to quickly place things in a bucket
		int counter = 0;
		while((n > (16*(2<<counter)) && counter <= 10) || (bucket[counter] == -1 && counter <= 10))
			counter++;
		//No space is available for allocation of a block of that size
		if(counter == 11)
			return NULL;
		else
		{
			if (n > 0 && n <= 16*(2<<counter))
			{
				//This block of code creates a copy of the header of free mem and shifts it to the end of the newly alloced mem
				//the size is changed to reflect the new smaller size of the free mem
				int index = bucket[counter];

				//sets each part of the header 1 index from each other starting at index+n+4 which is the end of the now allocated n bytes of memory
				set_prev(index+n/4, index);
				//check if the block was the last one and make it point to itself it is
				if(get_next(index) == index)
					set_next(index+n/4, index+n/4);
				else
					set_next(index+n/4, get_next(index));
				set_block_size(index+n/4, get_block_size(index)-n/32);
				set_flag(index+n/4, 0);
				set_prev_bucket(index+n/4, index+n/4);
				//check if the block is the last in the bucket
				if(get_next_bucket(index) == index)
					set_next_bucket(index+n/4, index+n/4);
				else
					set_next_bucket(index+n/4, get_next_bucket(index));

				//Checks if the remaining memory after allocation is enough to form a new block of free memory with a header and creates them
				if((int)(get_block_size(index+n/4)-n/32) >= (8*(2<<counter))/32)
				{
					//change bucket[counter]'s address to newly located header
					bucket[counter] += n/4;
				}
				//handles the case where the new the memory must be placed in a smaller bucket
				else if((int)(get_block_size(index+n/4) - n/32) < (8*(2<<counter))/32)
				{
					int temp = bucket[counter];
					//remove the memory slot from the previous bucket
					if(get_next_bucket(bucket[counter]) != bucket[counter])
					{
						bucket[counter] = get_next_bucket(bucket[counter]);
						set_prev_bucket(bucket[counter], bucket[counter]);
					}
					else
						bucket[counter] = -1;
					//since the new block size has been confirmed to be smaller, counter is adjusted to reflect the change
					while(get_block_size(index+n/4)*32 < (8*(2<<counter)) && counter >= 0)
						counter--;
					//sets the address of the new bucket to where the new block of memory is
					bucket[counter] = temp;
					bucket[counter] += n/4;
				}
				//in the event that there isn't enough remaining free memory to create a new block, the bucket will now point to the next free memory block in the bucket
				else if(get_next_bucket(index) != -1)
				{
					bucket[counter] = get_next_bucket(index);
				}
				//if there are no more free blocks of memory left, the bucket will be empty and return -1 when called
				else
				{
					bucket[counter] = -1;
				}
				//changes the size of the new blocks accordingly, set the flags and relocate pointers
				set_block_size(index, n/32);
				set_flag(index, 1);
				set_next(index, index+n/4);
				return &my_mem[0]+(index*4);
			}
			else
				return NULL;
		}
	}
}


void half_free( void * address)
{
	int index = ((int)address - (int)&my_mem[0])/32;
	if(!(get_flag(index) == 0 || index > 8192 || index < 0))
	{
		//handle case in which there are no free blocks adjacent to the block about to be freed
		if(get_flag(get_next(index))!=0 && get_flag(get_prev(index))!=0)
		{
			//creates counter which continuously iterates to find what bucket this free block belongs to
			int original_bucket = 0;
			while((get_block_size(index)*32 > (16*(2<<original_bucket)) && original_bucket <= 10))
				original_bucket++;
			//find where to place the newest member of the bucket
			int current_index = bucket[original_bucket];

			//if the bucket isn't empty, it will search for the right place to put the new memory
			if(current_index != -1)
				while(index > current_index && (get_next_bucket(current_index) != current_index))
					current_index = get_next_bucket(current_index);
			//handle case where current_index was the first thing in the bucket
			if(bucket[original_bucket] == current_index || current_index == -1)
			{
				//set the prev bucket pointer to itself
				set_prev_bucket(index, index);
				if(current_index == -1)
					set_next_bucket(index, index);
				else
				{
					set_next_bucket(index, bucket[original_bucket]);
					set_prev_bucket(bucket[original_bucket], index);
				}
				//makes the bucket point to our new memory block
				bucket[original_bucket] = index;
			}
			//handle case where index is further than the last memory in the bucket
			else if(index > current_index)
			{
				set_prev_bucket(index, current_index);
				set_next_bucket(current_index, index);
				set_next_bucket(index, index);
			}
			//handle case where index is somewhere in between
			else
			{
				//reroute the pointers that pointed to current_index to index
				set_prev_bucket(index, get_prev_bucket(current_index));
				set_next_bucket(get_prev_bucket(current_index), index);
				//put pointers to point the new memory to the bucket
				set_prev_bucket(current_index, index);
				set_next_bucket(index, current_index);
			}

		}
		//checks the block ahead to see if it is empty and can be combined
		if(get_flag(get_next(index))==0)
		{
			//creates counter which continuously iterates to find what bucket this free block belongs to
			int original_bucket = 0;
			while((get_block_size(get_next(index))*32 > (16*(2<<original_bucket)) && original_bucket <= 10))
				original_bucket++;
			//eliminate headers of adjacent block and adjust the size and pointers
			set_block_size(index, get_block_size(index)+get_block_size(get_next(index)));
			if(get_next(get_next(index)) != get_next(index))
			{
				set_next(index, get_next(get_next(index)));
				set_prev(get_next(get_next(index)), index);
			}
			else
				set_next(index, index);

			//manipulate buckets if necessary (this includes changing bucket pointers)
			//divides n by 32 once more to see if the memory should still go in the same bucket
			int new_bucket = 0;
			while((get_block_size(index)*32 > (16*(2<<new_bucket)) && new_bucket <= 10))
				new_bucket++;
			//new size belongs in the same bucket
			if(new_bucket == original_bucket)
			{
				//check if the pointer is not the only thing in the bucket
				if(get_next_bucket(get_next(index)) != get_next(index))
				{
					//replace pointers in header of second block and put them in first block as it will be located in the same spot in the bucket
					set_next_bucket(index, get_next_bucket(get_next(index)));
					set_prev_bucket(index, get_prev_bucket(get_next(index)));
					//need to add pointers from other free mem to point back
					set_next_bucket(get_prev_bucket(get_next(index)), index);
					set_prev_bucket(get_next_bucket(get_next(index)), index);
				}
				else
				{
					set_next_bucket(index, index);
					set_prev_bucket(index, index);
				}
				bucket[original_bucket] = index;
			}
			//new size belongs in a different bucket
			else
			{
				//series of if statements that are used to remove the old memory from the bucket
				//check if the memory is the first in its bucket
				if(get_prev_bucket(get_next(index)) == get_next(index))
				{
					//if there are more blocks of memory in the bucket
					if(get_next_bucket(get_next(index)) != get_next(index))
					{
						//make the next thing in that bucket point to itself on prev
						set_prev_bucket(get_next_bucket(get_next(index)), get_next_bucket(get_next(index)));
						//set the bucket to point to that free memory
						bucket[original_bucket] = get_next_bucket(get_next(index));
					}
					//only thing in the bucket
					else
						bucket[original_bucket] = -1;
				}
				//check if the memory is the last in its bucket
				else if(get_next_bucket(get_next(index)) == get_next(index))
				{
					//sets the second last memory as the last by making it point to itself
					set_next_bucket(get_prev_bucket(get_next(index)), get_prev_bucket(get_next(index)));
				}
				//connect the mem on the left and right together which would eliminate the memory from the bucket unless it was the first or last
				else
				{
					set_next_bucket(get_prev_bucket(get_next(index)), get_next_bucket(get_next(index)));
					set_prev_bucket(get_next_bucket(get_next(index)), get_prev_bucket(get_next(index)));
				}

				//places the new bigger block of memory in a new bucket
				//creates an integer to track the index of the block of memory in the bucket that comes after the index of the newly allocated block
				int current_index = bucket[new_bucket];
				if(current_index != -1)
					while(index > get_next_bucket(current_index) && (get_next_bucket(current_index) != current_index))
						current_index = get_next_bucket(current_index);
				//handle the case in which current_index is the first block of memory in the bucket and index comes before it
				if((current_index == bucket[new_bucket] || current_index == -1) && current_index > index)
				{
					//since the index is located before the first block in the bucket, it becomes the new first block (prev points to itself and next points to old first mem in bucket)
					bucket[new_bucket] = index;
					set_prev_bucket(index, index);
					if(current_index == -1)
						set_next_bucket(index, index);
					else
						set_next_bucket(index, current_index);
				}
				//handle case where current_index is the last block of memory in the bucket and index comes after
				else if(current_index < index)
				{
					//This happens when the loop reached the end and index is still greater
					set_next_bucket(current_index, index);
					set_next_bucket(index, index);
					set_prev_bucket(index, current_index);
				}
				//handles case where current_index is somewhere in the middle
				else
				{
					//current_index at this point will point to the free mem after the one at index
					set_next_bucket(index, current_index);
					set_prev_bucket(index, get_prev_bucket(current_index));
					set_next_bucket(get_prev_bucket(index), index);
					set_prev_bucket(current_index, index);
				}
			}
		}
		//checks the block behind to see if it is empty and can be combined
		if(get_flag(get_prev(index))==0)
		{
			//creates counter which continuously iterates to find what bucket this free block belongs to
			int original_bucket = 0;
			while((get_block_size(get_prev(index))*32 > (16*(2<<original_bucket)) && original_bucket <= 10))
				original_bucket++;
			//eliminate headers of adjacent block and adjust the size and pointers
			set_block_size(get_prev(index), get_block_size(get_prev(index))+get_block_size(index));
			if(get_next(index) != index)
			{
				set_next(get_prev(index), get_next(index));
				set_prev(get_next(index), get_prev(index));
			}
			else
				set_next(get_prev(index), get_prev(index));
			//manipulate buckets if necessary (this includes changing bucket pointers)
			//divides n by 32 once more to see if the memory should still go in the same bucket
			int new_bucket = 0;
			while((get_block_size(get_prev(index))*32 > (16*(2<<new_bucket)) && new_bucket <= 10))
				new_bucket++;
			//unlike the case where there is memory free after the memory inputed, this case does not require adjustments to the pointers if a bucket change is not required
			if(new_bucket == original_bucket)
				bucket[original_bucket] = get_prev(index);
			else
			{
				//check if the memory is the first in its bucket
				if(get_prev_bucket(get_prev(index)) == get_prev(index))
				{
					//check to make sure its not the only thing in the bucket
					if(get_next_bucket(get_prev(index)) != get_prev(index) && get_next_bucket(get_prev(index)) != index)
					{
						//make the next thing in that bucket point to itself on prev
						set_prev_bucket(get_next_bucket(get_prev(index)), get_next_bucket(get_prev(index)));
						bucket[original_bucket] = get_next_bucket(get_prev(index));
					}
					else
						bucket[original_bucket] = -1;
				}
				//check if the memory is the last in its bucket
				else if(get_next_bucket(get_prev(index)) == get_prev(index))
				{
					set_next_bucket(get_prev_bucket(get_prev(index)), get_prev(index));
				}
				//connect the mem on the left and right together which would eliminate the memory from the bucket unless it was the first or last
				else
				{
					set_next_bucket(get_prev_bucket(get_prev(index)), get_next_bucket(get_prev(index)));
					set_prev_bucket(get_next_bucket(get_prev(index)), get_prev_bucket(get_prev(index)));
				}

				//creates an integer to track the index of the block of memory in the bucket that comes after the index of the newly allocated block
				int current_index = bucket[new_bucket];
				if(current_index != -1)
					while(index > get_next_bucket(current_index) && (get_next_bucket(current_index) != current_index))
						current_index = get_next_bucket(current_index);
				//handle the case in which current_index is the first block of memory in the bucket and index comes before it
				if((current_index == bucket[new_bucket] || current_index == -1) && current_index > index)
				{
					//since the index is located before the first block in the bucket, it becomes the new first block (prev points to itself and next points to old first mem in bucket)
					bucket[new_bucket] = get_prev(index);
					set_prev_bucket(get_prev(index), get_prev(index));
					if(current_index == -1)
						set_next_bucket(get_prev(index), get_prev(index));
					else
						set_next_bucket(get_prev(index), current_index);
				}
				//handle case where current_index is the last block of memory in the bucket and index comes after
				else if(current_index < index)
				{
					//This happens when the loop reached the end and index is still greater
					set_next_bucket(current_index, get_prev(index));
					set_next_bucket(get_prev(index), get_prev(index));
					set_prev_bucket(get_prev(index), current_index);
				}
				//handles case where current_index is somewhere in the middle
				else
				{
					//current_index at this point will point to the free mem after the one at index
					set_next_bucket(get_prev(index), current_index);
					set_prev_bucket(get_prev(index), get_prev_bucket(current_index));
					set_next_bucket(get_prev_bucket(get_prev(index)), get_prev(index));
					set_prev_bucket(current_index, get_prev(index));
				}
			}
		}
	}
	//set flag to unallocated
	set_flag(index, 0);
}

