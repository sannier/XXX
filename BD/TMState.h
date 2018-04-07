#ifndef TMSTATE_H
#define TMSTATE_H

#include "syntactic_sugar.h"

// Layout of a Turing Machine State Tape
//
// Let our SA Turing Machine State Tape be defined as a string of 
// bits containing a formatted header
// 
// The header length is a function of the number of states.
//   The total header length is 8 + 20*n bytes, arranged as follows:
//     z, the number of (4-byte)words in the tape,                       4 bytes
//     p, the bit offset from the start of the tape to active state,     4 bytes
//    ST, a complete description of the state table,                   20n bytes
//
// Each state is represented as an opCode indicating this is a TuringMachineState,
//  followed by a 9-tuple of values specifying the next
//  symbol, direction and state that the machine prescribes, each a function
//  of the symbol read.
//                              sym              dir             nxt     
//   State 0: TMSOPCODE |  0     0    0  |   0    0    0  |  0    0    0 |   
//
//   State i: TMSOPCODE | sym0 sym1 sym2 | dir0 dir1 dir2 | nxt0 nxt1 nxt2
//
//     where:
//           TMSOPCODE                                [1 unsigned]
//           sym0, sym1, sym2 are in [00, 01, 10/11]  [1 uchar with 2 waste bits]
//           dir0, dir1, dir2 are in [-1, 0, 1]       [1 uchar with 2 waste bits]
//           PADDING(to help with C++ alignment)      [2 uchar -- unused]
// 	     nxt0, nxt1, nxt2 are in [0, 1, ..., n-1] [3 unsigned]
//
//  The pointers to the nxt states are stored as bit offsets into the state table
//    as counted from the start of the tape ...
//       to represent state i, the offset is  4+16i
//       if the offset is k  , state i = (k-8)/16
//
// The total length of the n state table is:
//   n*(4 uchars + 4 unsigned) = n*20 bytes
//
// We can get n from z (the number of words in the header) as:
//    4*z = 4 + 4 + n*20
//    (4z-8)/20 = n
//    (z-2)/5 = n

//TODO: Make a parent class to indicate that BDM::TMState and BDM::Command are
//        both instances of Commands -- same size, same interface
class TMState
{
private:
    friend class BitDeviceMachine;

    // Private data members
    //   Total size: 5 words/ 20 bytes/ 160 bits
    unsigned  opCode; // OPCODE for TuringMachineState
    uchar     sym;    // 3 2-bit symbols in [00, 01, 10](2 bits unused)
    uchar     dir;    // 3 2-bit symbols in [00, 01, 10](2 bits unused)
    uchar     pad[2]; // 2 uchar pads for alignment
    unsigned  nxt[3]; // 3 bit offsets to next states

    // Default constructor (never called because of "casting creation")
    TMState();
    
    // Init state to given state, direction, symbol triples
    void Init(uchar    s0, uchar    s1, uchar    sb,
	      int      d0, int      d1, int      db,
	      unsigned n0, unsigned n1, unsigned nb);
    
    // Initialize a state to random values consistent with an n state machine
    void Random(int scnt);

    // Accessors
    unsigned OpCode();      // Return the opCode for TuringMachineState
    uchar    Sym (uchar s); // Return sym[s] as uchar in [0,1,2]
    uchar    Dirs(uchar s); // Return dir[s] as uchar in [0,1,2]
    int      Dird(uchar s); // Return dir[s] as int in [-1,0,1]
    unsigned Nxto(uchar s); // Return nxt[s] as offset
    unsigned Nxts(uchar s); // Return nxt[s] as state
    
    // Addressors (useful for testing alignments) :-)
    uchar* SymA();
    uchar* DirA();
    uchar* NxtA();

    // Address conversions
    static int      SYM2Dir(unsigned ds);  // ds in [0,1,2] to int in [-1, 0, 1]
    static uchar    Dir2SYM(int d);        // int in [-1, 0, 1] to uchar in [0, 1, 2]
    static unsigned STATE2OFF(unsigned s); // State index s in [1,..,n] to bit offsetn
    static unsigned OFF2STATE(unsigned o); // Bit offset o to state index in [1, ..,n]

    // Print status of current command to debug file
    void DBGPRINT();
    
    // Equality and inequality operators
    bool operator==(const TMState &other) const;
    bool operator!=(const TMState &other) const;    
};

#endif
