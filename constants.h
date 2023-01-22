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
* File: constants.h
*
* Description: Header that contains global constants and structures
*
* 
*
**************************************************************/


#define MAX_DE_NAME 256
#define MAX_DIRENTRIES 51
#define MAX_PATH_LENGTH 4096
#include <time.h>
#include <stdint.h>

// Volume Control Block. Stores info about the volume to be accessed anywhere
typedef struct VCB{
	uint64_t size_of_block;         //size of a individual block
	uint64_t number_of_blocks;      //counts the number of blocks
	uint64_t blocks_available;      //holds blocks available
	uint64_t bitmap_starting_index; //where the bitmap starts
    uint64_t bitmap_size_bytes;     // Size of bitmap in bytes
    uint64_t bitmap_size_blocks;    // Size of bitmap in blocks

    uint64_t root_starting_index;   // LBA where root starts
    uint64_t root_size;             // Size of root in blocks

	uint64_t signature;             //used to check if own the volume

} VCB;



VCB* vcb;
// Bitmap containing info on free space
unsigned char* bitmap;
