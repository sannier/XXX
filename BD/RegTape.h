#ifndef REGTAPE_H
#define REGTAPE_H

// A RegTape holds a set of working registers.
//    It is of fixed size [MAXREGS]*sizeof(int);
class RegTape
{
private:
    friend class BitDeviceMachine;

    // Private data members:
    //     Total size:  MAXREGS*sizeof(int)
    int reg[MAXREGS]; // Registers to hold address arithmetic
	
    //Default Constructor
    RegTape();
	
    // Accessors
    void setReg(unsigned regId, int value);
    int  getReg(unsigned regId);
    unsigned Len();  // Returns length of Regs in bytes

    void DBGPRINT();
	
};


#endif
