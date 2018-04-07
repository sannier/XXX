#ifndef MACHINETAPE_H
#define MACHINETAPE_H

// A BD Machine Tape looks like this:
//      z     |       p        |   <cmd 0> <cmd 1>...<cmd n-1>
// unsigned       unsigned           a list of n cmds
//   # 32b        # bits             
// wds in tape  to curr cmd
//   (buflen)       (head)
//
// where a <cmd i> is:
//    opCode   |   arg1   |   arg2   |   arg3   |  nxtCmd            | 
//  unsigned       int        int        int      unsigned
//   code for  |   bitAddresses/int values      | bit offset from ap |
//    command  |    as args to command          |   to next Command  |
//
// z@0  (Size of Tape in 32bit/4byte words)
// p@32 (Bit offset to current command)
// opcode@(32+p), arg1@(64+p), arg2@(96+p), arg3@(128+p)
// nxtCmd@(160+p)
// n -- number of commands = 4*(z-2)/sizeof(Command);
class MachineTape
{
private:
    friend class BitDeviceMachine;

    // Private data members
    //    Total size: z*sizeof(unsigned)
    unsigned z;     // Size of Tape in 32bit/4byte words)
    unsigned p;     // Bit offset to current command)
    Command  cmd[1];// Variable length array of commands

    // Default constructor (never called because of "casting creation")
    MachineTape();

    // Get/Set the current cmd as an index 
    unsigned getCurrentCommand();
    void     setCurrentCommand(unsigned s);

    // Get/Set Number of commands -- z = f(numOfCommands)
    unsigned getNumberOfCommands() const;
    void     setNumberOfCommands(unsigned nc);

    // Returns the length of the MachineTape in bytes
    //    Need this because MachineTape has a variable length array
    //      so sizeof(MachineTape) doesn't tell the whole story
    unsigned Len();

    // Equality and inequality operators
    bool operator==(const MachineTape &other) const;
    bool operator!=(const MachineTape &other) const;

    void DBGPRINT();
};


#endif
