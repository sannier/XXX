#ifndef WORKINGTAPE_H
#define WORKINGTAPE_H

// A WorkingTape looks like this:
// A BD Machine Working Tape looks like this:
//      z     |       h        |   <2-bit symbols>
// unsigned       unsigned       a string of 2-bit symbols in [00,01,10]
//   # 32b        # bits             
// wds in tape  to current symbol
//   (buflen)       (head)
//
class WorkingTape
{
private:
    friend class BitDeviceMachine;

    // Private Data Members
    //    Total size: z*sizeof(unsigned)
    unsigned z; // Length of tape in 4-byte words
    unsigned h; // Position of head in bits
    uchar T[4]; // Variable length array containing 4(z-2)uchars

    // Default constructor (never called because of "casting creation")
    WorkingTape();

    // Set all of a tapes symbols to 0
    void clear();
	
    // Initialize the tape to a given string
    void initTape(const char* str, unsigned strPos, unsigned p);

    // Set the length of the tape in symbols
    // Get the length of the tape in symbols
    void     setTapeLen(unsigned ln);
    unsigned tapeLen() const;
    
    // Set the head to position p (in symbols)
    // Get the head (in symbols)
    void     setHead(unsigned p);
    unsigned getHead() const;

    // Worker functions for print
    void printTapeLine(unsigned opCnt);
    void printHeadLine();
    void printStateLine(unsigned state);
	
    // Write s at p 
    void assign(uchar s, unsigned p);

    // Return the symbol at p
    uchar value(unsigned p);
	
    // Some syntactic sugar to convert from head index (symbols) to offset(bits)
    static unsigned IND2OFF(unsigned s);
    static unsigned OFF2IND(unsigned o);
    
    // Length of the whole structure including: l, p and tape in bytes    
    unsigned Len() const;
	
    // Move the head in the given direction d in [-1|0|+1]
    void Move(int d);// head += d;

    // Read the symbol at the head position and return it as an
    //   unsigned value in [0|1|2] (2 is blank)
    uchar Read();

    // Write the symbol s in [0|1|2] on the tape at the head position  
    void Write(uchar s);

    // Print the tape out to the console -- indicate that the machine
    //   working this tape is in the state given
    void Print(unsigned state, unsigned opCnt);

    // Equality and inequality operators
    bool operator==(const WorkingTape &other) const;
    bool operator!=(const WorkingTape &other) const;

    void DBGPRINT();
	
};


#endif
