#ifndef BITDEVICE_H
#define BITDEVICE_H

#include "syntactic_sugar.h"

// BitDevice is an abstraction of a machine that can
//   read and return 2bits(SYM) or 32bits(WRD)
//   or copy 2bits(SYM) or 32bits(WRD) from
//   any bit location in a given string of bits
//   to any other location*
//     *(some limits on byte boundaries apply)
// Bit Device can also write a given bit string to disk or
//   read in a bitstring previously stored
class BitDevice
{
private:
    uchar*   tape;
    unsigned tapelen;
    int     *reg;
    bool     deleteTape;

    // Dump any exisiting tape
    void reset();

    // Copy the two bits located at the given bit address
    // into an uchar and return
    uchar sym(unsigned bitAddress);
    
    // Copy 32 bits beginning at the given bit address
    //   into an unsigned and return
    unsigned wrd(unsigned bitAddress);

public:
//=====================CONFIGURATION=====================================
    // Constructors/Destructor
    BitDevice();
    BitDevice(uchar* buf, unsigned buflen);
    ~BitDevice();

    // Load the tape we are going to read from and write to
    void LoadTape(uchar* buf, unsigned buflen, bool oursToDelete=false);

    // Accessors
    // Return pointer to tape/registers
    bool   Valid();
    uchar* GetTape(unsigned &buflen) const;
    int*   GetRegisters();

    // File I/O
    // Read/Write a tape from/to fname 
    void Read(const char* fname);
    void Write(const char* fname);

//====================BIT FUNCTIONS=======================================    
    // Subtract the value in register r2 from the register r1
    //    (returns value as integer instead of putting it in register)
    //     makes it easier to use this as the basis for an if statement
    //     TODO: is it somehow cheating not to use register for result?
    //           if we use register, how do we access it?
    int  COMP(unsigned r1, unsigned r2); 
    
    // Clear registers
    void CLRR();

    // Load the (constant) value c into the register r
    void LOAD(int c, unsigned r); 

    // Read 32 bits from the tape beginning at bit position specified 
    //   in register r1 into register r2 
    void WRDR(unsigned r1, unsigned r2);

    // Write 32 bits from register r1 into the tape at bit position specified
    //   in register r2 
    void WRDW(unsigned r1, unsigned r2);

    // Read 2 bit symbol from the tape beginning at bit position specified
    //   in register r1 into register r2 
    void SYMR(unsigned r1, unsigned r2);

    //  Write 2 bit symbol stored in register r1 into the tape at
    //    the bit position specified in register r2
    void SYMW(unsigned r1, unsigned r2);
    
    // Multiply/Add the values in register r1 and r2 and place result in r3 
    void MULT(unsigned r1, unsigned r2, unsigned r3);
    void ADDN(unsigned r1, unsigned r2, unsigned r3);
};

#endif
