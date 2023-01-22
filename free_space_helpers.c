/**************************************************************
* Class:  CSC-415-03 Fall 2021
* Names:  Richard Aguilar
*         Melisa Sever
*         Ryan Scott
*         Jonathan Valadez
*
* Student IDs: 977075554
*              921662115
*              921814228
*              922274961
*
* GitHub Name: raguilar0917
* Group Name: The Beerman Fan Club
* Project: Basic File System
*
* File: free_space_helpers.c
*
* Description: Implements functions from associated header.
    Contains functions useful for manipulating free space, 
    as well as sending free blocks to other functions
*
* 
*
**************************************************************/

#include "free_space_helpers.h"
#include "bit_math.h"
#include "constants.h"
#include <stdio.h>


/*
    Returns first free block starting from param
*/
int GetFreeBlock(int start_pos){
    int block = -1;             // Ret val
    int location_in_byte = 0;   // Will be added to block if free found

    // Loop over bitmap; break if free bit found
        // This can be optimized a bit if we know which bytes are not free
    for(int i = start_pos; i < vcb->bitmap_size_bytes; i++){
		location_in_byte = FindFreeBit(bitmap[i]);
        if(location_in_byte != -1){
            // One byte (i) and offset by location_in_bytes bits
            block = i*8+location_in_byte;
            break;
        }
	}

    return block;
}


/*
    Param: int number of blocks to receive
    Returns: Starting block number of free blocks allocated

    Returns -1 if there are fewer than requested blocks available

    Marks received blocks as used
    If less are used than requested, make sure to free them

*/
int GetNFreeBlocks(int blocks){
    int free_blocks_start = -1;    // Array to hold blocks

    int start_pos = 0;          // Starting read position for bitmap
    int possible_block = -1;    // Set to ret of GetFreeBlock
    int cont_blocks_found = 0;
    int last_free = -1;

    // Find blocks free blocks and add to array

    // 00011101111
        // Find 4 blocks
    
    while(cont_blocks_found < blocks && start_pos < vcb->number_of_blocks){
        for(int i = 0; i < blocks; i++){
            possible_block = GetFreeBlock(start_pos);
            if(last_free == -1){
                last_free = possible_block - 1;
            }

            if(possible_block == -1 || possible_block != last_free+1){   // Did not find N contiguous free

                MarkBlocksFree(free_blocks_start, cont_blocks_found);
                free_blocks_start = -1;
                cont_blocks_found = 0;
                last_free = -1;
                break;
            }
            else{
                if(free_blocks_start == -1){
                    free_blocks_start = possible_block;
                }
                last_free = possible_block;
                cont_blocks_found++;
                start_pos = possible_block/8;   // Start searching from that byte next time
                MarkOneBlockUsed(possible_block);
            }
        }
        start_pos++;

    }

    return free_blocks_start; 
}

/*
    Param: int array of blocks, size of arr
    Returns number of blocks marked as used

int MarkBlocksUsed(int blocks[], int size){
    int marked = 0;
    int block = 0;      // Current working block
    int byte = 0;       // Index into bitmap of the block
    for(int i = 0; i < size; i++){
        block = blocks[i];
        byte = block/8;
        // Set byte to new value after flip
        bitmap[byte] = FlipBitUsed(bitmap[byte], (block % 8));
        marked++;
    }

    return marked;
}

*/

/*
    Param: int array of blocks, size of arr
    Returns number of blocks marked as used
*/

int MarkBlocksUsed(int start, int size){
    int marked = 0;
    int block = start;      // Current working block
    int byte = 0;       // Index into bitmap of the block
    for(int i = 0; i < size; i++){
        byte = block/8;
        // Set byte to new value after flip
        bitmap[byte] = FlipBitUsed(bitmap[byte], (block % 8));
        marked++;
        block++;
    }

    vcb->blocks_available = vcb->blocks_available - marked;

    return marked;
}

/*
    Version of MarkBlocksUsed that marks only a single block
    Param: int block to be marked
    Returns: number of marked blocks
*/
int MarkOneBlockUsed(int block){
    
    return MarkBlocksUsed(block, 1);
}

/*
    Version of MarkBlocksFree that marks only a single block
    Param: int block to be marked free
    Returns: number of marked blocks
*/
int MarkOneBlockFree(int block){    
    return MarkBlocksFree(block, 1);
}


/*
    Param: int array of blocks, size of arr
    Returns number of blocks marked as used
*/

int MarkBlocksFree(int start, int size){
    int marked = 0;
    int block = start;      // Current working block
    int byte = 0;       // Index into bitmap of the block
    for(int i = 0; i < size; i++){
        byte = block/8;
        // Set byte to new value after flip
        bitmap[byte] = FlipBitFree(bitmap[byte], (block % 8));
        marked++;
        block++;
    }

    vcb->blocks_available = vcb->blocks_available + marked;


    return marked;
}