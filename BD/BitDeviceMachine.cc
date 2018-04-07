#include <assert.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "BitDeviceMachine.h"

#include "debugfile.h"

#include "Command.cc"
#include "MachineTape.cc"
#include "RegTape.cc"
#include "WorkingTape.cc"

// Constructor and destructor
BitDeviceMachine::BitDeviceMachine()
{a = 0; b = 0; c = 0;}
BitDeviceMachine::~BitDeviceMachine()
{reset(0, 0, false);}

// Set BitDeviceMachine to work on the given tape
//   of buflen bytes. delTape is true if the tape should
//   be deleted on destruction of the machine
void BitDeviceMachine::reset(uchar* buf, unsigned buflen, bool delTape)
{bd.LoadTape(buf, buflen, delTape);}

// Return a pointer to the machine's register tape
int* BitDeviceMachine::getRegisters()
{return bd.GetRegisters();}

// Write the TuringBootstrap program into MachineTape
#define aHALT()        {a->cmd[m].Init(Command::OPHALT,   0,   0,   0,   0);m++;}
#define aLOAD(i, j)    {a->cmd[m].Init(Command::OPLOAD, (i), (j),   0, m+1);m++;}
#define aWRDR(i, j)    {a->cmd[m].Init(Command::OPWRDR, (i), (j),   0, m+1);m++;}
#define aSYMR(i, j)    {a->cmd[m].Init(Command::OPSYMR, (i), (j),   0, m+1);m++;}
#define aSYMW(i, j)    {a->cmd[m].Init(Command::OPSYMW, (i), (j),   0, m+1);m++;}
#define aWRDW(i, j)    {a->cmd[m].Init(Command::OPWRDW, (i), (j),   0, m+1);m++;}
#define aCOMP(i, j)    {a->cmd[m].Init(Command::OPCOMP, (i), (j),   0, m+1);m++;}
#define aADDN(i, j, k) {a->cmd[m].Init(Command::OPADDN, (i), (j), (k), m+1);m++;}
#define aMULT(i, j, k) {a->cmd[m].Init(Command::OPMULT, (i), (j), (k), m+1);m++;}
#define aRTRN(i, j)    {a->cmd[m].Init(Command::OPRTRN, (i), (j),   0,   0);m++;}
unsigned BitDeviceMachine::turingBootstrap()
{
    unsigned m = 0;
    // HALT is always the 0th command
    aHALT();
    
    // Find address of z and p in the string
    aLOAD(0, 0);         // az =  0 in reg0 
    aLOAD(32, 1);        // ap = 32 in reg1

    // Fill register with the value of z
    aWRDR(0, 2);         // z@an/reg2@reg0

    // Find h: past state header, register section and size of working tape
    //    h = z*32 + MAXREGS*32 + 32 -> h = (z+MAXREGS+1)*32;
    aLOAD(MAXREGS, 4);   //        MAXREGS/reg4
    aADDN(2, 4, 5);      //      z+MAXREGS/reg5=reg2+reg4
    aMULT(5, 1, 6) ;     // (z+MAXREGS)*32/reg6=reg5*reg1
    aADDN(1, 6, 7);      // ah=(z+MR+1)*32/reg7=reg6+reg1
    aWRDR(7, 8);         //           h@ah/reg8@reg7

    // Find x, the symbol under head ax = (z+MAXREGS)*32 + h
    //    bitSizeofA(z*32)+ bitSizeofB(MAXREGS*32) + head
    //     Head is h bits past the start of the working tape
    aADDN(6, 8, 9);      // ax=(z+MAXREGS)*32+h/reg9=reg6+reg8
    aSYMR(9, 10);        //                x@ax/reg10@reg9

    // Find X, the new symbol to be written
    aLOAD(2, 11);        //          2/reg11
    aMULT(11, 10, 12);   //         2x/reg12=reg11*reg10
    aADDN(29, 12, 26);    //      p+2x/reg26=reg29+reg12
    aADDN(26, 1, 13);    // aX=p+2x+32/reg13=reg26+reg1
    aSYMR(13, 24);       //       X@aX/reg24@reg13

    // Find H, the new value of the head 
    //   new head postion is old position + 2*direction
    //   (because symbols take 2 bits)
    aLOAD(8, 14);        //       8/reg14
    aADDN(13, 14, 15);   // aD=aX+8/reg15=reg13+reg14
    aSYMR(15, 19);       //    D@aD/reg19@reg15
    aLOAD(-1, 20);       //      -1/reg20
    aADDN(19, 20, 21);   //  Dd=D-1/reg21=reg19+reg20
    aMULT(11, 21, 22);   //    2*Dd/reg22=reg11*reg21
    aADDN(8, 22,  23);   // H=h+2Dd/reg23=reg8+reg22

    // Find S, the new value of the state
    aADDN(1,29, 16);     //         64+p/reg16=reg1+reg3
    aADDN(1,16, 16);     //         64+p/reg16=reg1+reg3 -- add32 twice for 64
    aMULT(1, 10, 17);    //         32*x/reg17=reg1*reg10
    aADDN(16, 17, 18);   // aS=64+p+32*x/reg18=reg16+reg17
    aWRDR(18, 25);       //         S@aS/reg25@reg18

    // Store the HALTSTATEOFF so we can use to compare for halt test
    aLOAD(HALTSTATEOFF, 27); // 64/reg27 (is this 128 now???

    // Using bits from the registers, modify the buffer to reflect a
    //    single execution step...
    aSYMW(24, 9); // Overwrite "old" symbol with the "new"
    aWRDW(23, 7); // Move tape head L/R or not at all
    aRTRN(25, 1); // Write return address to p and return

    return (m);
}

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
void BitDeviceMachine::computeAddresses1()
{
    // Find z and p
    bd.LOAD( 0, 30); // az =  0 in reg30 
    bd.LOAD(32, 31); // ap = 32 in reg31

    // Fill registers with the values of z and p
    bd.WRDR(30, 32); // z@an/reg32@reg30
    bd.WRDR(31, 33); // p@as/reg33@reg31

    // Find the opcode at p bits past the start of tape
    bd.WRDR(33, 35); // op@aop/reg35@reg33
}

void BitDeviceMachine::computeAddresses2()
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

// Returns size in bytes of a machine with cmdCnt commands and
//   a working tape of tapeLen symbols
unsigned BitDeviceMachine::machineSize(unsigned stateCnt, unsigned tapeSize)
{
    // Well formed machine size is complicated by
    //   two things:
    //    a. Header definition contains 1 state so total
    //       size of header is 
    //         sizeof(Header)+(s-1)*sizeof(state)
    //    b. WorkingTape definition is in "symbols" which are
    //         two bits long and sizeof(WorkingTape) includes
    //         one byte (4 symbols). A tape with tapeSize
    //         symbols takes up 2*tapeSize/8 bytes - the
    //         one byte in the WorkingTape struct
    return (sizeof(BitDeviceMachine::MachineTape) +
	       (stateCnt-1)*sizeof(BitDeviceMachine::Command) +
	    sizeof(BitDeviceMachine::RegTape) +
	    sizeof(BitDeviceMachine::WorkingTape) +
	       ((tapeSize*2)/8)-4);
}

bool BitDeviceMachine::Valid() const
{
    // Start with this: we have to have a tape and three parts
    unsigned buflen;
    uchar* tape = bd.GetTape(buflen);
    return ((tape != 0) && (a != 0) && (b != 0) && (c != 0));
}

bool BitDeviceMachine::WellFormed() const
{
    bool wellformed;
    // Start with this: we have to have a tape and three parts
    //wellformed = Valid();
    //if(!wellformed) return wellformed;
/*
    // n is wellformed if it accurately predicts the size of the state table
    wellformed = ((uchar*)tape + a->z*4) == (uchar*)b;
    if(!wellformed) return wellformed;

    // s is wellformed if it is a state between 0 and n-1
    wellformed = (TMState::OFF2STATE(a->p) < a->getStateCount());
    if(!wellformed) return wellformed;

    // x is wellformed if its a 0/1/2
    uchar x = c->Read();
    wellformed = (x == 0 | x == 1 | x == 2);
    if(!wellformed) return wellformed;

    // X is wellformed if its a 0/1/2 given x
    uchar X = a->Sym(x);
    wellformed = (X == 0 | X == 1 | X == 2);
    if(!wellformed) return wellformed;

    // Ds is wellformed if its a 0/1/2
    uchar Ds = a->Dirs(x);
    wellformed = (Ds == 0 | Ds == 1 | Ds == 2);
    if(!wellformed) return wellformed;

    // Nxt is well formed if its a state in [0, n-1]
    unsigned nxt = a->Nxts(x);
    wellformed = (nxt < a->getStateCount());
    if(!wellformed) return wellformed;
*/
    // TODO: Could do more (check state table, check tape)
    return true;
}

void BitDeviceMachine::InitToSub1()
{
    // Make a 33+8-cmd machine with an 80 symbol / 160 bit / 20 byte tape
    // TuringBootstrap(including HALT at 0) + 8 TuringStates
    unsigned cmdCount   = 33+8;
    unsigned tapeSize   = 80; // Size in symbols
    Init(cmdCount, tapeSize);

    // Write the bootstrap into the command table: cmdidx is index of next command
    unsigned cmdidx = turingBootstrap();
    
    //Initialize the StateTable
    //State   Symbol     Write          Move          Next State
    // State  0(Halt)      
    //        0         Don't write  Don't move        0   
    //        1         Don't write  Don't move        0   
    //        Blank     Don't write  Don't move        0   
    //State 1	
    //        0	        Write ‘0’    Move tape left    1
    //        1	        Write ‘1’    Move tape left    1
    //        2	        Write ‘2’    Move tape left    2
    //State 2	
    //        0	        Write ‘0’    Move tape left    2
    //        1	        Write ‘1’    Move tape left    2
    //        2	        Write ‘2’    Move tape right   3
    //State 3	
    //        0	        Write ‘0’    Move tape right   3
    //        1	        Write ‘0’    Move tape right   4
    //        2	        Write ‘2’    Move tape left    5
    //State 4	
    //        0	        Write ‘0’    Move tape right   4
    //        1	        Write ‘1’    Move tape right   4
    //        2	        Write ‘2’    Move tape right   8
    //State 5	
    //        0	        Write ‘0’    Move tape left    5
    //        1	        Write ‘1’    Move tape left    5
    //        2	        Write ‘2’    Move tape right   6
    //State 6	
    //        0	        Write ‘2’    Move tape right   6
    //        1	        Write ‘1’    Move tape right   6
    //        2	        Write ‘2’    Move tape right   7
    //State 7	
    //        0	        Write ‘2’    Move tape right   7
    //        1	        Write ‘1’    Move tape right   7
    //        2	        Write ‘2’    Move tape left    0
    //State 8	
    //        0	        Write ‘0’    Move tape right   8
    //        1	        Write ‘0’    Move tape left    1
    //        2	        Write ‘2’    Move tape left    0
    TMState* swiz = (TMState*)a->cmd; //Cast cmd list as state table
    int b = cmdidx; // cmdidx where TuringMachine starts
    swiz[cmdidx].Init(0, 1, 2,  0,  0,  0,   0,   0,   0);cmdidx++; 
    swiz[cmdidx].Init(0, 1, 2, -1, -1, -1, b+1, b+1, b+2);cmdidx++;
    swiz[cmdidx].Init(0, 1, 2, -1, -1, +1, b+2, b+2, b+3);cmdidx++;	
    swiz[cmdidx].Init(0, 0, 2, +1, +1, -1, b+3, b+4, b+5);cmdidx++;
    swiz[cmdidx].Init(0, 1, 2, +1, +1, +1, b+4, b+4, b+8);cmdidx++;
    swiz[cmdidx].Init(0, 1, 2, -1, -1, +1, b+5, b+5, b+6);cmdidx++;
    swiz[cmdidx].Init(2, 1, 2, +1, +1, +1, b+6, b+6, b+7);cmdidx++;
    swiz[cmdidx].Init(2, 1, 2, +1, +1, -1, b+7, b+7,   0);cmdidx++;
    swiz[cmdidx].Init(0, 0, 2, +1, -1, -1, b+8, b+1,   0);cmdidx++;
    
    // The larger number has to be placed to the left of the smaller number
    //    with a blank symbol in between,
    // position the Tape head at any digit of the larger number.
    c->initTape("11111111 111", 20, 13);
 
    //Start in state 1 (of the Turing Machine)
    SetCurrentCommand(b+1);
}

// Initialize the machine to one that when fed a tape with a binary number
//   where the head is positioned within the number will add 1 to that
//   number and halt
void BitDeviceMachine::InitToAdd1()
{
    // Make a 33+3-state test machine with an 80 symbol / 160 bit / 20 byte tape
    // TuringBootstrap(including HALT at 0) + 3 TuringStates
    unsigned cmdCount = 33+3;
    unsigned tapeSize = 80; // Size in symbols
    Init(cmdCount, tapeSize);
    
    // Write the bootstrap into the command table: cmdidx is index of next command
    unsigned cmdidx = turingBootstrap();

    //Initialize the StateTable
    // State Table to add 1 to a number
    // State         SymbolRead   WriteInstruction     MoveInstruction    NextState
    // State  0(Halt)      
    //               0            Don't write          Don't move         State 0   
    //               0            Don't write          Don't move         State 0   
    //               0            Don't write          Don't move         State 0   
    // State 1      
    //  	     0            Write ‘0’            Move tape left      State 1
    //               1            Write ‘1’            Move tape left      State 1
    //               Blank        Write ‘Blank’        Move tape right     State 2
    //
    // State 2       
    //               0            Write ‘1’            Move tape right     State 3
    //               1            Write ‘0’            Move tape right     State 2
    //               Blank        Write ‘1’            Move tape left      State 3
    //
    // State 3       
    //               0            Write ‘0’            Move tape left      State 3
    //               1            Write ‘1’            Move tape left      State 3
    //               Blank        Write ‘Blank’        Move tape right     State 0
    TMState* swiz = (TMState*)a->cmd; //Cast cmd list as state table
    int b = cmdidx; // cmdidx where TuringMachine starts
    swiz[cmdidx].Init(0, 0, 0,  0,  0,  0,   0,   0,   0);cmdidx++;
    swiz[cmdidx].Init(0, 1, 2, -1, -1, +1, b+1, b+1, b+2);cmdidx++;
    swiz[cmdidx].Init(1, 0, 1, +1, +1, -1, b+3, b+2, b+3);cmdidx++;
    swiz[cmdidx].Init(0, 1, 2, -1, -1, +1, b+3, b+3,   0);cmdidx++;
    
    // Put "1" beginning at positon 4 on the tape
    //   and proceeing toward 0. Put the head at 4
    c->initTape("1", 4, 4);
    
    //Start in state 1
    SetCurrentCommand(b+1);
}


// 3-State Busy Beaver
// The busy beaver problem is to find the most number of ‘1’s that
// a 2-symbol, n-state Turing machine can print on an initially blank
// tape before halting. The aim of this is to find the smallest
// program which outputs as much data as possible, and can also stop
// eventually.
// Visit http://www.catonmat.net/blog/busy-beaver/
// for more information on the problem.

// For a 3-State machine, the maximum number of ‘1’s that it can
// print is proven to be 6, and it takes 14 steps for the Turing
// machine to do so. The state table for the program is shown below.
// Since only 2 symbols are required, the instructions for the ‘0’
// symbol are left as the default settings.
void BitDeviceMachine::InitToBB3()
{
    // Make a 33+4-state machine with an 80 symbol / 160 bit / 20 byte tape
    // TuringBootstrap(including HALT at 0) + 4 TuringStates
    unsigned cmdCount = 33+4;
    unsigned tapeSize   = 80; // Size in symbols
    Init(cmdCount, tapeSize);

    // Write the bootstrap into the command table: cmdidx is index of next command
    unsigned cmdidx = turingBootstrap();
    
    //Initialize the StateTable
    //State Table
    //State	    Symbol 	Write 	Move 	Next State
    //  0        <Halt State>
    //
    //  1
    //           0	        ‘Blank’	 None	1
    //           1	        ‘1’	 None	0
    //         Blank	        ‘1’	 Left	2
    //  2	   
    //           0	        ‘Blank’	 None	1
    //           1	        ‘1’	 Left	2
    //         Blank	        ‘Blank’	 Left	3
    //  3	   
    //           0	        ‘Blank’	 None	1
    //           1	        ‘1’	 Right	1
    //         Blank	        ‘1’	 Right	3
    TMState* swiz = (TMState*)a->cmd; //Cast cmd list as state table
    int b = cmdidx; // cmdidx where TuringMachine starts
    swiz[cmdidx].Init(0, 1, 2, 0,  0,  0,   0,   0,   0);cmdidx++;
    swiz[cmdidx].Init(2, 1, 1, 0,  0, -1, b+1,   0, b+2);cmdidx++;
    swiz[cmdidx].Init(2, 1, 2, 0, -1, -1, b+1, b+2, b+3);cmdidx++;
    swiz[cmdidx].Init(2, 1, 1, 0, +1, +1, b+1, b+1, b+3);cmdidx++;
    
    // Start with blank tape with the head at 10
    c->initTape(" ", 4, 10);

    //Start in state 1
    SetCurrentCommand(b+1);
}


//4-State Busy Beaver
//For a 4-state busybeaver, the maximum number
// of ‘1’s that can be printed is 13, and it takes 107 steps.
void BitDeviceMachine::InitToBB4()
{
    // Make a 33+5-cmd machine with an 80 symbol / 160 bit / 20 byte tape
    // TuringBootstrap(including HALT at 0) + 5 TuringStates
    unsigned cmdCount = 33+5;
    unsigned tapeSize = 80; // Size in symbols
    Init(cmdCount, tapeSize);

    // Write the bootstrap into the command table: cmdidx is index of next command
    unsigned cmdidx = turingBootstrap();
    
    //Initialize the StateTable
    //State Symbol 	Write 	Move 	Next State
    //  0         <null state -- halt>
    //  1
    //        0	        ‘Blank’	None    1
    //        1	        ‘1’	Left	2
    //      Blank	‘1’	Right	2
    //  2
    //        0	        ‘Blank’	None	1
    //        1	        ‘Blank’	left	3
    //      Blank	‘1’	left	1
    //  3
    //        0	        ‘Blank’	None	1
    //        1	        ‘1’	left	4
    //       Blank	‘1’	right	0
    //  4
    //        0	        ‘Blank’	None	1
    //        1	        ‘Blank’	right	1
    //       Blank	‘1’	right	4
    TMState* swiz = (TMState*)a->cmd; //Cast cmd list as state table
    int b = cmdidx; // cmdidx where TuringMachine starts
    swiz[cmdidx].Init(0, 1, 2, 0,  0,  0,   0,   0,   0);cmdidx++;
    swiz[cmdidx].Init(2, 1, 1, 0, -1, +1, b+1, b+2, b+2);cmdidx++;
    swiz[cmdidx].Init(2, 2, 1, 0, -1, -1, b+1, b+3, b+1);cmdidx++;
    swiz[cmdidx].Init(2, 1, 1, 0, -1, +1, b+1, b+4,   0);cmdidx++;
    swiz[cmdidx].Init(2, 2, 1, 0, +1, +1, b+1, b+1, b+4);cmdidx++;
    
    // Start with blank tape with the head at 10 
    c->initTape(" ", 4, 10);

    //Start in state 1
    SetCurrentCommand(b+1);
}


//Palindrome Detector
// This program checks a string of symbols on the tape and
//   returns a ‘1’ (green) if it is a palindrome and a ‘0’ (red) if it is not.
void BitDeviceMachine::InitToPAL()
{
    // Make a 33+8-cmd machine with an 80 symbol / 160 bit / 20 byte tape
    // TuringBootstrap(including HALT at 0) + 5 TuringStates
    unsigned cmdCount = 33+8;
    unsigned tapeSize = 80; // Size in symbols
    Init(cmdCount, tapeSize);

    // Write the bootstrap into the command table: cmdidx is index of next command
    unsigned cmdidx = turingBootstrap();
    
    //Initialize the StateTable
    //State Table
    //State	Symbol    Write Move 	Next State
    // 0     <null state -- halt>
    //State 1
    //        0	   ‘0’	  right	 1
    //        1	   ‘1’	  right	 1
    //      Blank Blank   left	 2
    //State 2
    //        0	  Blank  left	 3
    //        1	  Blank  left	 5
    //      Blank  ‘1’	 None    0
    //State 3
    //        0	   ‘0’	 left	 3
    //        1	   ‘1’	 left	 3
    //      Blank Blank  right	 4
    //State 4
    //        0	  Blank  right	 1
    //        1	   ‘1’	 None	 7
    //      Blank Blank  None	 1
    //State 5
    //        0	   ‘0’	 left	 5
    //        1	   ‘1’	 left	 5
    //      Blank Blank  right	 6
    //State 6
    //        0	   ‘0’	 None	 7
    //        1	  Blank  right	 1
    //      Blank Blank  None	 1
    //State 7
    //        0	  Blank  right	 7
    //        1	  Blank  right	 7
    //      Blank  ‘0’	 None	 0
    TMState* swiz = (TMState*)a->cmd; //Cast cmd list as state table
    int b = cmdidx; // cmdidx where TuringMachine starts
    swiz[cmdidx].Init(0, 1, 2,  0,  0,  0,   0,   0,   0);cmdidx++;
    swiz[cmdidx].Init(0, 1, 2, -1, -1, +1, b+1, b+1, b+2);cmdidx++;
    swiz[cmdidx].Init(2, 2, 1, +1, +1,  0, b+3, b+5,   0);cmdidx++;
    swiz[cmdidx].Init(0, 1, 2, +1, +1, -1, b+3, b+3, b+4);cmdidx++;
    swiz[cmdidx].Init(2, 1, 2, -1,  0,  0, b+1, b+7, b+1);cmdidx++;
    swiz[cmdidx].Init(0, 1, 2, +1, +1, -1, b+1, b+1, b+4);cmdidx++;
    swiz[cmdidx].Init(0, 2, 2,  0, -1,  0, b+7, b+1, b+1);cmdidx++;
    swiz[cmdidx].Init(2, 2, 0, -1, -1,  0, b+7, b+7, b+0);cmdidx++;
    
    // Start with legal palindrome between 20-10 the head at 15 
    c->initTape("1111111111 ", 20, 15);

    //Start in state 1
    SetCurrentCommand(b+1);
}


void BitDeviceMachine::Init(unsigned cmdCount, unsigned tapeSize)
{
    // Make a stateCount-state test machine
    //   with a tapeLen symbol / 2*tapelen bit / 2*tapelen/8 byte tape
    //   TODO::: HACKALERT -- sticking size of MachineTape in first byte
    //                        so BitDevice can find the registers
    unsigned buflen     = machineSize(cmdCount, tapeSize);
    uchar*   tape       = new uchar[buflen];
    *(unsigned*)tape    = (cmdCount*sizeof(BitDeviceMachine::Command)+8)/4;
    reset(tape, buflen, true);
    tape = bd.GetTape(buflen);
    
    // Overlay the three accessor parts and set lengths for MachineTape and WorkingTape
    a = (MachineTape*)tape;
    a->setNumberOfCommands(cmdCount);
    b = (RegTape*)(tape + a->Len());
    c = (WorkingTape*)(tape + a->Len() + b->Len());
    unsigned tlen1 = buflen - a->Len() - b->Len() - MT_HEADERSZ;
    unsigned tlen2 = buflen - ((uchar*)(&c->T)-(uchar*)a);
    assert(tlen1 == tlen2);
    assert(SYMPERBYTE*tlen1 == tapeSize);
    c->setTapeLen(tapeSize);
}

// Init to already created buffer -- if delTape is true, delete on destruction
void BitDeviceMachine::InitToBuf(uchar* buf, unsigned buflen, bool delTape)
{
    // Assign the tape and remember whether its ours to delete on destruction
    // TODO:: HACKALERT -- assuming machinetape size is on this tape
    //                     at the start...makes it possible to find the registers
    reset(buf, buflen, delTape);
    uchar* tape = bd.GetTape(buflen);

    // Overlay the three accessor parts -- assume the lengths are valid
    a = (MachineTape*)tape;
    b = (RegTape*)(tape + a->Len());
    c = (WorkingTape*)(tape + a->Len() + b->Len());
    assert(buflen == a->Len() + b->Len() + c->Len());
}

// Test for halt condition 
bool BitDeviceMachine::Halted()
{assert(Valid()); return (GetCurrentCommand() == 0);}

// Read/Write symbol under the head
uchar BitDeviceMachine::Read() const
{assert(Valid()); return c->Read();}
void BitDeviceMachine::Write(uchar x)
{assert(Valid()); assert(x == 0 || x == 1 || x ==2); return c->Write(x);}

//Returns length of tape in symbols
unsigned BitDeviceMachine::Tapelen()
{assert(Valid()); return c->tapeLen();}

// Set/Get head in symbols
void BitDeviceMachine::SetHead(unsigned np)
{assert(Valid()); assert(np < Tapelen()); c->setHead(np);}
unsigned BitDeviceMachine::GetHead() const
{assert(Valid());return (c->getHead());}

// Set/Get the current command
unsigned BitDeviceMachine::GetCurrentCommand() const
{assert(Valid()); return a->getCurrentCommand();}
void BitDeviceMachine::SetCurrentCommand(unsigned nc)
{assert(Valid()); a->setCurrentCommand(nc);}

// Execute a single opCode - returns true if command is "printable"
// TODO: Revisit "printable" hack
bool BitDeviceMachine::execOpCode(int opcode, int arg1, int arg2, int arg3)
{
    // Execute the opCode
    switch(opcode)
    {
    case Command::OPCLRR:    // Clear registers
    {
	fprintf(DBGFILE, "CLRR(%d, %d)\n", arg1, arg2);
	bd.CLRR();
	break;
    }
    case Command::OPLOAD:   // Load the value c into the register r
    {
	fprintf(DBGFILE, "LOAD(%d, %d)\n", arg1, arg2);
	bd.LOAD(arg1, arg2);
	break;
    }
    case Command::OPWRDR:     // Copy 32 bits from tape@p into register r 
    {
	fprintf(DBGFILE, "WRDR(%d, %d)\n", arg1, arg2);
	bd.WRDR(arg1, arg2);
	break;
    }
    case Command::OPSYMR:     // Copy 2 bits from tape@p p into register r 
    {
	fprintf(DBGFILE, "SYMR(%d, %d)\n", arg1, arg2);
	bd.SYMR(arg1, arg2);
	break;
    }
    case Command::OPMULT:    // Multiply registers r1 and r2 and place result in r3 
    {
	fprintf(DBGFILE, "MULT(%d, %d, %d)\n", arg1, arg2, arg3);
	bd.MULT(arg1, arg2, arg3);
	break;
    }
    case Command::OPADDN:     // Add registers r1 and r2 and place result in r3 
    {
	fprintf(DBGFILE, "ADDN(%d, %d, %d)\n", arg1, arg2, arg3);
	bd.ADDN(arg1, arg2, arg3);
	break;
    }
    case Command::OPSYMW:   // Copy the 2-bits@p1 to bits@p2) 
    {
	fprintf(DBGFILE, "SYMW(%d, %d)\n", arg1, arg2);
	bd.SYMW(arg1, arg2);
	break;
    }
    case Command::OPWRDW:   // Copy 32-bit value v into @p2) 
    {
	fprintf(DBGFILE, "WRDW(%d, %d)\n", arg1, arg2);
	bd.WRDW(arg1, arg2);
	break;
    }
    case Command::OPHALT:    // OPCode indicates string has HALTED 
    {
	fprintf(DBGFILE, "HALT()\n");
	return true;
    }
    case Command::OPRTRN:    // OPCode to prevent resetting cmd ptr
    {
	fprintf(DBGFILE, "WRDW(%d, %d)\n", arg1, arg2);
	bd.WRDW(arg1, arg2);
	fprintf(DBGFILE, "RTRN()\n");
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

// Execute a single step
bool BitDeviceMachine::ExecuteS()
{
    assert(bd.Valid());
    
    // Compute addresses to extract opCode of current command
    computeAddresses1();
    int*     reg    = getRegisters();
    unsigned opCode = reg[35];

    // If code is a TuringState, test for halt, copy currentState to reg29,
    //  and invoke the bootstrap by setting current comamnd to 1
    if(opCode == Command::OPTMST)
    {
	// Test for halted. If halted, return
	if (Halted()) return true;

	// Not halted? Call bootstrap
	bd.WRDR(31, 29);          // Write currentState to reg29
	bd.LOAD(FIRSTCMDOFF, 49); // Use 49 to hold offset to 1st command in bootstrap)
	bd.WRDW(49, 31);          // Make first command in the bootstrap current

	return true;
    }

    // If its not a turing state, get the other arguments and execute the opcode
    computeAddresses2();
    int arg1 = reg[37]; 
    int arg2 = reg[39];
    int arg3 = reg[41];
    return execOpCode(opCode, arg1, arg2, arg3);
}

// Run the current machine -- print each state transition unless silent
void BitDeviceMachine::Execute(bool silent)
{
    // Don't let a machine run more than 10000 steps    
    static int MAXSTEPS = 10000; 

    // Print the tape before first step if not silent
    if(!silent) Print(0);  

    // Run the machine till the stop state is reached
    for(int i=0; !Halted() && i<MAXSTEPS; i++)
    {
	// Execute a step	
	bool printable = ExecuteS();

	// Print the tape after this step(if printable)
	if(!silent && printable) Print(i);
    }
}

// Initialize machine from a file
bool BitDeviceMachine::ReadFile(const char* fname)
{
    // Open the file for reading
    std::ifstream tfile(fname, std::ifstream::in | std::ifstream::binary);
    if (!tfile) return false;

    // get length of file:
    tfile.seekg (0, tfile.end);
    int length = tfile.tellg();
    tfile.seekg (0, tfile.beg);

    // Make a large enough buffer to hold the tape
    uchar *buf = new uchar [length];

    // read data as a block:
    tfile.read ((char*)buf, length);
    int bytesRead = length;
    if (!tfile) bytesRead = tfile.gcount();
    assert(bytesRead == length);

    // Close the file
    tfile.close();

    // Overlay the three accessor parts
    InitToBuf(buf, length, true);

    return true;
}

// Write machine to file: as a bit string
bool BitDeviceMachine::WriteFile(const char* fname)
{
    assert(Valid());

    // Open the file for writing
    std::ofstream tfile(fname, std::ofstream::out | std::ofstream::binary);
    if (!tfile) return false;

    // Write the buffer
    unsigned len = a->Len() + b->Len() + c->Len();
    unsigned buflen;
    uchar* tape = bd.GetTape(buflen);
    assert(len == buflen);
    tfile.write((char*)(tape), len);

    // Close the file
    tfile.close();
    
    return true;
}

// Write machine to file member by member
bool BitDeviceMachine::RewriteFile(const char* fname)
{
    assert(Valid());

    // Open the file for writing
    std::ofstream tfile(fname, std::ofstream::out | std::ofstream::binary);
    if (!tfile) return false;

    // Write size as 32bit unsigned
    tfile.write((char*)&(a->z), sizeof(unsigned));

    // Write CurrentCommand as a 32 bit unsigned
    tfile.write((char*)&(a->p), sizeof(unsigned));

    // Write Command Table as sizeof(Command)*n bytes
    tfile.write((char*)&(a->cmd),
		(sizeof(BitDeviceMachine::Command)*a->getNumberOfCommands()));

    // Write registers as sizeof(RegTape) bytes
    tfile.write((char*)b, sizeof(RegTape));

    // Write Length as 32 bit unsigned
    tfile.write((char*)&(c->z), sizeof(unsigned));

    // Write HeadPos as 32 bit unsigned
    tfile.write((char*)&(c->h), sizeof(unsigned));

    // Write the rest of the tape 
    tfile.write((char*)&(c->T), c->tapeLen()/SYMPERBYTE);
    
    // Close the file
    tfile.close();

    return true;
}

void BitDeviceMachine::Print(unsigned opCnt)
{
    assert(Valid());
    // c->Print(GetCurrentCommand(), opCnt);
    //TODO: Get rid of HACK to make state right
    //      (current command is start of bootstrap after TuringState execed
    int*     reg    = getRegisters();
    unsigned cs     = TMState::OFF2STATE(reg[29]);
    c->Print(cs, opCnt);
}

// Accessors
unsigned BitDeviceMachine::Getz()   {assert(Valid()); return a->z;}
unsigned BitDeviceMachine::Getp()   {assert(Valid()); return a->p;}
unsigned BitDeviceMachine::Geth()   {assert(Valid()); return c->h;}

// Addressors (useful for testing alignments) :-)
unsigned BitDeviceMachine::zA()
{assert(Valid()); return (((uchar*)&a->z             -(uchar*)a)*8);}
unsigned BitDeviceMachine::pA()
{assert(Valid()); return (((uchar*)&a->p             -(uchar*)a)*8);}
unsigned BitDeviceMachine::hA()
{assert(Valid()); return (((uchar*)&c->h             -(uchar*)a)*8);}


// Equality and inequality operators
bool BitDeviceMachine::operator==(const BitDeviceMachine &other) const
{
    // The resgister parts may differ and Machines are still considered ==
    if(!Valid() || !other.Valid()) return false;
    
    return (*a == *other.a && *c == *other.c);
}

bool BitDeviceMachine::operator!=(const BitDeviceMachine &other) const
{
    return !(*this == other);
}
