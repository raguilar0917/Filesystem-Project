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
* File: directory_entry.h
*
* Description: Header containing function prototypes for 
    manipulating individual bits
*
* 
*
**************************************************************/

/*
    Param: A byte
    Returns: Number of "free" bits in the byte
*/
int BitCounter(unsigned char target);


/*
    Param: A byte
    Returns: Index of first free bit in byte
    or -1 if none found
*/
int FindFreeBit(unsigned char target);


/*
    Param: int base and exponent
    Returns: base to the exponent power
*/
int power(int base, int exp);


/*
    Param: A byte and a bit index between 0 and 7
    Turns given bit into a 0

*/
int FlipBitUsed(unsigned char byte, int bit);

/*
    Param: A byte and a bit index between 0 and 7
    Turns given bit into a 1

    Returns: Byte with changes
*/
int FlipBitFree(unsigned char byte, int bit);