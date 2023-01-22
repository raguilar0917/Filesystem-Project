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
* File: bit_math.c
*
* Description: Implements functions from associated header.
    Contains many math functions that help in finding and 
    manipulating individual bits
*
* 
*
**************************************************************/

#include "bit_math.h"
#include <stdio.h>
/*
    Param: A byte
    Returns: Number of "free" bits in the byte
*/
int BitCounter(unsigned char target){
    int x = 0; // Number of free bits found
    int bit = 0x80; // Bit we are checking; goes high to low

    // Check each bit in target
    while(bit > 0){
        if((target & bit) == bit){
            x++;
        }
        bit = bit/2;
    }

    return x;
}

/*
    Param: A byte
    Returns: Index of first free bit in byte
    or -1 if none found
*/
int FindFreeBit(unsigned char target){
    int x = 0; // Number of free bits found
    int bit = 0x80; // Bit we are checking; goes high to low

    // Check each bit in target. Return bit if free
    while(bit > 0){
        if((target & bit) == bit){
            return x; 
        }
        x++;
        bit = bit/2;
    }

    return -1;
}

/*
    Param: int base and exponent
    Returns: base to the exponent power
*/
int power(int base, int exp){
    int result = 1;
    for(int i = 0; i < exp; i++){
        result = result * base;
    }

    return result;
}


/*
    Param: A byte and a bit index between 0 and 7
    Turns given bit into a 0

    Returns: Byte with changes
*/
int FlipBitUsed(unsigned char byte, int bit){
    bit = power(2, 7-bit);  // 7-bit because index is left to right
    byte = byte & ~bit;

    return byte;
}

/*
    Param: A byte and a bit index between 0 and 7
    Turns given bit into a 1

    Returns: Byte with changes
*/
int FlipBitFree(unsigned char byte, int bit){
    bit = power(2, 7-bit);  // 7-bit because index is left to right
    byte = byte | bit;

    return byte;
}