#include <fstream>
#include <assert.h>
#include "BitDevice.h"

// #bytes/word, byte offset to start of registers
#define BYTESPERWORD 4
#define REGSTART(b) (int*)((b)+((*(unsigned*)(b))*BYTESPERWORD))

void BitDevice::reset()
{
    if(deleteTape) delete tape;
    tape    = 0;
    tapelen = 0;
    reg     = 0;
    deleteTape = false;
}

void BitDevice::LoadTape(uchar* buf, unsigned buflen, bool oursToDelete)
{
    // Reset BitDevice
    reset();
    
    deleteTape = oursToDelete;
    tape       = buf;
    tapelen    = buflen;
    reg        = (buf ? REGSTART(buf) : 0);
}

// Copy 32 bits beginning at bit p1 into an unsigned and return
//   in an unsigned
unsigned BitDevice::wrd(unsigned p1)
{
    // Find the bytes containing p1 -- expect p1 on a byte boundary
    unsigned b1 = p1/bPB; assert(p1%bPB == 0);

    // Get the value out of the string at the given bytes
    unsigned word = *(unsigned*)(tape+b1);

    // Return word
    return word;
}

// Pull a 2bit symbol from buf at bitAddress and return in a uchar
uchar BitDevice::sym(unsigned bitAddress)
{
    // Get they byte where the bits are -- expect both bits in same byte
    unsigned byteA     = bitAddress/bPB;
    unsigned offsetA   = bitAddress%bPB; assert(offsetA < 7);
    uchar byte = *(tape+byteA);
    
    // Shift byte so that the desired bits are the two least significant
    // Mask out the top six using mask 00000011(3)
    uchar mask = 3; // 00000011 (3)
    byte = byte >> offsetA;
    byte = byte & mask;

    // Return symbol in lowest two bits of uchar
    return byte;
}

// Bit Device has no tape top start with
BitDevice::BitDevice()
{
    LoadTape(0, 0, false);
}

BitDevice::BitDevice(uchar* buf, unsigned buflen)
{
    LoadTape(buf, buflen, false);
}

BitDevice::~BitDevice()
{
    if(deleteTape) delete tape;
}

bool BitDevice::Valid()
{
    return (tape != 0);
}

uchar* BitDevice::GetTape(unsigned &buflen) const
{
    buflen = tapelen;
    return tape;
}

int*  BitDevice::GetRegisters() {return reg;}


// Read length bits in fname into a buffer and return it
void BitDevice::Read(const char* fname)
{
    // Open the file for reading
    std::ifstream tfile(fname, std::ifstream::in | std::ifstream::binary);
    if (!tfile) return;

    // get length of file:
    unsigned buflen;
    tfile.seekg (0, tfile.end);
    buflen = tfile.tellg();
    tfile.seekg (0, tfile.beg);

    // Make a large enough buffer to hold the tape
    uchar *buf = new uchar [buflen];

    // read data as a block:
    int expectedLength = buflen;
    tfile.read ((char*)buf, buflen);
    if (!tfile) buflen = tfile.gcount();
    assert(buflen == expectedLength);

    // Close the file
    tfile.close();

    // Load buffer as a tape -- because we new'd it here, its ours to delete
    LoadTape(buf, buflen, true);
}

//Write length bits from buf into fname
void BitDevice::Write(const char* fname)
{
    assert(tape);
    
    // Open the file for writing
    std::ofstream tfile(fname, std::ofstream::out | std::ofstream::binary);
    if (!tfile)	return;

    // Write the buffer
    tfile.write((char*)tape, tapelen);

    // Close the file
    tfile.close();
}

void BitDevice::CLRR()
{
    for(int i=0; i<MAXREGS; i++)
	reg[i] = 0;
}

// Load the value c into the register r
void BitDevice::LOAD(int c, unsigned r)
{
    assert(r < MAXREGS);
    reg[r] = c;
}

// Subtract the value in register r2 from the register r1
//    (returns value as integer instead of putting it in register)
//     makes it easier to use this as the basis for an if statement
int BitDevice::COMP(unsigned r1, unsigned r2)
{
    assert(r1 < MAXREGS);
    assert(r2 < MAXREGS);
    return reg[r1]-reg[r2];
}

// Read 32 bits from the tape beginning at bit position specified 
//   in register r1 into register r2 
void BitDevice::WRDR(unsigned r1, unsigned r2)
{
    assert(r1 < MAXREGS);
    assert(r2 < MAXREGS);
    reg[r2] = wrd(reg[r1]);
}

// Write 32 bits from register r1 into the tape at bit position specified
//   in register r2 
void BitDevice::WRDW(unsigned r1, unsigned r2)
{
    assert(r1 < MAXREGS);
    assert(r2 < MAXREGS);

    // Find the byte specified by the offset in r2
    //      -- assume even boundaries
    unsigned p = reg[r2];
    assert(p%bPB == 0);
    unsigned b1 = p/bPB; 

    // Write the given value at the given byte
    *(unsigned*)(tape+b1) = reg[r1];
}


// Read 2 bit symbol from the tape beginning at bit position specified
//   in register r1 into register r2 
void BitDevice::SYMR(unsigned r1, unsigned r2)
{
    assert(r1 < MAXREGS);
    assert(r2 < MAXREGS);
    reg[r2] = sym(reg[r1]);
}
    
//  Write 2 bit symbol stored in register r1 into the tape at
//    the bit position specified in register r2
void BitDevice::SYMW(unsigned r1, unsigned r2)
{
    assert(r1 < MAXREGS);
    assert(r2 < MAXREGS);
    STATIC_MASKS;

    // Get the source symbol copied into a 0-padded byte
    //   of the form 000000ss, where ss is in [00/01/10]
    uchar sbyte = reg[r1];

    // Get offset from r2 and use it to find target byte
    // Identify the target byte -- expect both bits to be in same byte
    unsigned p = reg[r2];    assert(p%2 == 0); // Assume even boundaries
    unsigned tbyteA   = p/bPB;
    unsigned toffsetA = p%bPB; assert(toffsetA < 7);              
    uchar tbyte = *(tape+tbyteA);

    // Mask out the target bits and replace them with source bits
    assert(toffsetA == 0 || toffsetA == 2 || toffsetA == 4 || toffsetA == 6);
    tbyte = tbyte & emask[toffsetA/2];
    sbyte = sbyte << toffsetA;
    tbyte = tbyte | sbyte;

    // Replace original target byte with newly edited tbyte
    *(tape+tbyteA) = tbyte;
}

// Multiply/Add the values in register r1 and r2 and place result in r3 
void BitDevice::MULT(unsigned r1, unsigned r2, unsigned r3)
{
    assert(r1 <MAXREGS);
    assert(r2 <MAXREGS);
    assert(r3 <MAXREGS);
    reg[r3] = reg[r1]*reg[r2];
}

void BitDevice::ADDN(unsigned r1, unsigned r2, unsigned r3)
{
    assert(r1 <MAXREGS);
    assert(r2 <MAXREGS);
    assert(r3 <MAXREGS);
    reg[r3] = reg[r1]+reg[r2];
}




