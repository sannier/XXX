#ifndef COMMAND_H
#define COMMAND_H

// a <cmd i> is:
//    opCode   |   arg1   |   arg2   |   arg3   |  nxtCmd      | 
//   unsigned  |   int    |   int    |   int    | unsigned     |
//   code for  |   bitAddresses/int values      | bit offset   |
//    command  |    as args to command          | to next cmd  |
//     32b     |    32b   +    32b   +    32b   |   32b
//             Total size: 5 words/ 20 bytes/ 160 bits
class Command
{
private:
    friend class BitDeviceMachine;

    // Private data members
    //   Total size: 5 words/ 20 bytes/ 160 bits
    unsigned opCode;
    int      arg[3];
    unsigned nxtCmd;

    // Default constructor (never called because of "casting creation")
    Command();

    enum OPCODE { OPCLRR, OPLOAD, OPWRDR, OPSYMR, OPMULT,
		  OPADDN, OPWRDW, OPSYMW, OPHALT, OPRTRN,
		  OPTMST=1235};
    void Init(OPCODE oc, int ar1, int ar2, int ar3, unsigned nxt);

    // Convert CMDIDX into bit offset from tape start and back again
    static unsigned CMDIDX2OFF(unsigned idx);
    static unsigned OFF2CMDIDX(unsigned o);

    // Accessors
    unsigned OpCode();      
    int      Arg(unsigned i);
    unsigned Nxto();
    unsigned Nxti();

    // Equality and inequality operators
    bool operator==(const Command &other) const;
    bool operator!=(const Command &other) const;    
	
    void DBGPRINT();
};

#endif
