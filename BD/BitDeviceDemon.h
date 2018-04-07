#ifndef BITDEVICEDEMON_H
#define BITDEVICEDEMON_H

#include "BitDevice.h"

class BitDeviceDemon
{
private:
    BitDevice bd;

    // Preliminary stuff for BD Programs
    bool turingBootstrap();
    void computeAddresses1();
    void computeAddresses2();

    enum OPCODES { OPCLRR, OPLOAD, OPWRDR, OPSYMR, OPMULT,
		   OPADDN, OPWRDW, OPSYMW, OPHALT, OPRTRN,
		   OPTMST=1235};

    bool execOpCode(int opcode, int arg1, int arg2, int arg3);
    
public:
    BitDeviceDemon();
    ~BitDeviceDemon();

    // Accessors
    // Return pointer to tape/registers
    uchar* GetTape(unsigned &buflen);
    int*   GetRegisters();

    // Test for halt condition
    bool  Halted();

    // File I/O
    // Read/Write a tape from/to fname 
    void Read(const char* fname);
    void Write(const char* fname);

    // Execute a single step...
    bool ExecuteS(unsigned opCnt);
};

#endif
