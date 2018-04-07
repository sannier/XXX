#include <assert.h>
#include "BitDeviceDemon.h"

BitDeviceDemon::BitDeviceDemon(){}
BitDeviceDemon::~BitDeviceDemon(){}

// A bd Tape looks like this:
//      z     |       p        |   <cmd 0> <cmd 1>...<cmd n-1>
// unsigned       unsigned           a list of n cmds
//   # 32b        # bits             
// wds in tape  to "beginning"
//   (buflen)       (head)
//
// where a <cmd i> is:
//    opCode   |   arg1   |   arg2   |   arg3   |  nxtCmd            | 
//  unsigned       int        int        int      unsigned
//   code for  |   bitAddresses/int values      | bit offset from ap |
//    command  |    as args to command          |   to next Command  |
//
// z@0
// p@32
// opcode@(32+p), arg1@(64+p), arg2@(96+p), arg3@(128+p)
// nxtCmd@(160+p)      
void BitDeviceDemon::computeAddresses1()
{
    // Store the HALTSTATEOFF so we can use to compare for halt test
    bd.LOAD(HALTSTATEOFF, 27); // reg27 (is this 128 now???
    
    // Find z and p
    bd.LOAD( 0, 30); // az =  0 in reg30 
    bd.LOAD(32, 31); // ap = 32 in reg31

    // Fill registers with the values of z and p
    bd.WRDR(30, 32); // z@an/reg32@reg30
    bd.WRDR(31, 33); // p@as/reg33@reg31

    // Find the opcode at p bits past the start of tape
    bd.WRDR(33, 35); // op@aop/reg35@reg33
}

void BitDeviceDemon::computeAddresses2()
{
    // Get the first argument
    bd.ADDN(33, 31, 36); // aarg1=aop+32/reg36=reg33+reg31
    bd.WRDR(36, 37);     //   arg1@aarg1/reg37@reg36

    // Get the second argument
    bd.ADDN(36, 31, 38); // aarg2=aarg1+32/reg38=reg36+reg31
    bd.WRDR(38, 39);     //     arg2@aarg2/reg39@reg38

    // Get the third argument
    bd.ADDN(38, 31, 40); // aarg3=aarg2+32/reg40=reg38+reg31
    bd.WRDR(40, 41);     //     arg3@aarg3/reg41@reg40

    // Get the bit offset to the nxtCmd 
    bd.ADDN(40, 31, 42); // anxtCmd=aarg3+32/reg42=reg40+reg31
    bd.WRDR(42, 43);     //   nxtCmd@anxtCmd/reg43@reg42
}

// Accessors
// Return pointer to tape/registers
uchar* BitDeviceDemon::GetTape(unsigned &buflen) {return bd.GetTape(buflen);}
int*   BitDeviceDemon::GetRegisters()            {return bd.GetRegisters();}

// File I/O
// Read/Write a tape from/to fname 
void BitDeviceDemon::Read(const char* fname)     {bd.Read(fname);}
void BitDeviceDemon::Write(const char* fname)    {bd.Write(fname);}

// Execute a single opCode - returns true if command is "printable"
// TODO: Revisit "printable" hack
bool BitDeviceDemon::execOpCode(int opcode, int arg1, int arg2, int arg3)
{
    // Execute the opCode
    switch(opcode)
    {
    case OPCLRR:    // Clear registers
    {
	bd.CLRR();
	break;
    }
    case OPLOAD:   // Load the value c into the register r
    {
	bd.LOAD(arg1, arg2);
	break;
    }
    case OPWRDR:     // Copy 32 bits from tape@p into register r 
    {
	bd.WRDR(arg1, arg2);
	break;
    }
    case OPSYMR:     // Copy 2 bits from tape@p p into register r 
    {
	bd.SYMR(arg1, arg2);
	break;
    }
    case OPMULT:    // Multiply registers r1 and r2 and place result in r3 
    {
	bd.MULT(arg1, arg2, arg3);
	break;
    }
    case OPADDN:     // Add registers r1 and r2 and place result in r3 
    {
	bd.ADDN(arg1, arg2, arg3);
	break;
    }
    case OPSYMW:   // Copy the 2-bits@p1 to bits@p2) 
    {
	bd.SYMW(arg1, arg2);
	break;
    }
    case OPWRDW:   // Copy 32-bit value v into @p2) 
    {
	bd.WRDW(arg1, arg2);
	break;
    }
    case OPHALT:    // OPCode indicates string has HALTED 
    {
	return true;
    }
    case OPRTRN:    // OPCode to prevent resetting cmd ptr
    {
	bd.WRDW(arg1, arg2);
	return false;
    }
    default:
	assert("Invalid opCode");
	break;
    }

    // Overwrite the bootstrap cmd ptr with next command
    bd.WRDW(43, 31);
    
    return false;    
}

bool BitDeviceDemon::Halted()
{
    assert(bd.Valid());

    //Test for halt by comparing 
    if(!bd.COMP(33, 26))
	return false;
    else
	return true;
}

//Execute a single step...
bool BitDeviceDemon::ExecuteS(unsigned opCnt)
{
    //TODO: Recognize TuringMachine tapes and execute them via
    //        a bootstrap "hardcoded" into the BitDevice.
    assert(bd.Valid());

    // Compute addresses to extract opCode of current command
    computeAddresses1();
    int*     reg    = GetRegisters();
    unsigned opCode = reg[35]; // bd command to invoke

    // If code is a TuringState, test for halt, copy currentState to reg29,
    //  and invoke the bootstrap by setting current comamnd to 1
    if(opCode == OPTMST)
    {
	// Test for halted. If halted, return
	if (Halted()) return true;

	// Not halted? Call bootstrap
	bd.WRDR(31, 29);          // Write currentState to reg29
	bd.LOAD(FIRSTCMDOFF, 49); // Use 49 to hold offset to 1st command in bootstrap)
	bd.WRDW(49, 31);          // Make first command in the bootstrap current

	return true;
    }

    // If its not a turing state, get the other arguments and execute the opCode
    computeAddresses2();
    int      arg1   = reg[37]; 
    int      arg2   = reg[39];
    int      arg3   = reg[41];

    return execOpCode(opCode, arg1, arg2, arg3);
}
