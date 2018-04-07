#ifndef BITDEVICEMACHINE_H
#define BITDEVICEMACHINE_H

#include <stdio.h>

#include "syntactic_sugar.h"
#include "BitDevice.h"

class BitDeviceMachine
{
private:
#include "TMState.h"
#include "Command.h"
#include "MachineTape.h"    
#include "RegTape.h"
#include "WorkingTape.h"    

    // Private Data Members
    MachineTape* a;  // The tape as a whole
    RegTape*     b;  //    subtape with registers
    WorkingTape* c;  //    subtape with working space
    BitDevice    bd; // BitDevice that holds the tape

    // Set BitDeviceMachine to work on the given tape
    //   of buflen bytes. delTape is true if the tape should
    //   be deleted on destruction of the machine
    void reset(uchar* buf, unsigned buflen, bool delTape);

    // Return a pointer to the machine's register tape
    int* getRegisters();

    // Write commands to execute a TuringState into a tape
    unsigned turingBootstrap();

    // Compute the addresses necessary to run a BitDeviceProgram
    //    extract opCode    in computeAddresses1
    //    extract arguments in computerAddresses2
    void computeAddresses1();
    void computeAddresses2();

    // Execute an op code with its arguments
    bool execOpCode(int opcode, int arg1, int arg2, int arg3);

    // Returns size in bytes of a machine with cmdCnt commands and
    //   a working tape of tapeLen symbols
    static unsigned machineSize(unsigned cmdCnt, unsigned tapeLen);
    
public:
    // Constructor and destructor
    BitDeviceMachine();
    ~BitDeviceMachine();

    // TODO: Expand Valid machine to ensure "well-formedness"
    // Valid machine?
    bool Valid() const;
    bool WellFormed() const;

    // Establish the different predefined machines:
    //   Add1, Sub1, BB3, BB4 and PAL
    void InitToAdd1();
    void InitToSub1();
    void InitToBB3();
    void InitToBB4();
    void InitToPAL();

    //TODO: Make Init depend on InitToBuf (trickiness with sizes)
    // Initialize to an empty machine with given cmd count and tapesize
    void Init(unsigned cmdCount, unsigned tapeSize);
    
    // Initialize to a previously established buffer (sizes already in buf)
    //    delTape true means we delete on destruction
    void InitToBuf(uchar* buf, unsigned len, bool delTape);

    // Test for halt condition
    bool  Halted();

    // Read/Write symbol under the head
    uchar Read() const;
    void  Write(uchar x);

    //Returns length of tape in symbols
    unsigned Tapelen();  

    // Set and get Head Position in symbols
    void     SetHead(unsigned p);
    unsigned GetHead() const;

    // Set and get current command
    unsigned GetCurrentCommand() const;
    void     SetCurrentCommand(unsigned idx);
    
    // Execute a step
    // Execute till halt
    bool  ExecuteS();
    void  Execute(bool silent);

    // Initialize machine from a file
    // Write machine to file: as a bit string or member by member
    bool  ReadFile(const char* name);
    bool  WriteFile(const char* name);
    bool  RewriteFile(const char* name);

    // Print machine to standard out
    void Print(unsigned opCnt);

    // Accessors
    unsigned Getz();
    unsigned Getp();
    unsigned Geth();

    // Addressors (useful for testing alignments) :-)
    unsigned zA();
    unsigned pA();
    unsigned hA();

    // Equality and inequality operators
    bool operator==(const BitDeviceMachine &other) const;
    bool operator!=(const BitDeviceMachine &other) const;
};


#endif
