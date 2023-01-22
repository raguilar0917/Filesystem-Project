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
* File: free_space_helpers.h
*
* Description: Header that defines function prototypes for 
    functions that help manipulate and request free space
* 
*
**************************************************************/

/*
    Returns first free block starting from param
*/
int GetFreeBlock(int start_pos);

/*
    Param: int number of blocks to receive
    Returns: Starting position of `blocks` free blocks

    Returns -1 if there are fewer than requested blocks available
*/
int GetNFreeBlocks(int blocks);

/*
    Param: int starting block number, number of blocks to mark used
    Returns number of blocks marked as used
*/
int MarkBlocksUsed(int start, int size);

/*
    Version of MarkBlocksUsed that marks only a single block
    Param: int block to be marked
    Returns: number of marked blocks
*/
int MarkOneBlockUsed(int block);



/*
    Version of MarkBlocksFree that marks only a single block
    Param: int block to be marked free
    Returns: number of marked blocks
*/
int MarkOneBlockFree(int block);


/*
    Param: int array of blocks, size of arr
    Returns number of blocks marked as used
*/

int MarkBlocksFree(int start, int size);