//=============================================================================
// Default constructor (never called because of "casting creation")
BitDeviceMachine::RegTape::RegTape(){assert("Should never be called");}

// Set/get the value of a given register
void BitDeviceMachine::RegTape::setReg(unsigned regId, int value)
{
    assert(regId < MAXREGS);
    reg[regId] = value;
}
int  BitDeviceMachine::RegTape::getReg(unsigned regId)
{
    assert(regId < MAXREGS);
    return reg[regId];
}

// Returns the length of a RegTape in bytes
unsigned BitDeviceMachine::RegTape::Len() {return sizeof(BitDeviceMachine::RegTape);}

// Prints the contents of the registers to a debug file
void BitDeviceMachine::RegTape::DBGPRINT()
{
    char Mng[MAXREGS][100] = {"az", "ap", "z", "3", "MAXREGS", "z+MAXREGS",
			      "@WT", "ah",
			      "h", "ax", "x", "2", "2x", "aX", "8",
			      "aD", "64+p", "32*x", "aP", "D", "-1", "Dd","2*Dd",
			      "H", "X", "P", "p+2x", "HSTOFF", "28", "currentState",
			      "az", "ap", "z", "p", "34", "op", "aarg1",
			      "arg1", "aarg2", "arg2", "aarg3", "arg3", "anxtCmd",
			      "nxtCmd", "44", "45", "46", "47", "48", "1stcmdoff"};
	
    for(int i=0; i<MAXREGS; i++)
    {
	fprintf(DBGFILE, "reg[%2i]=%10i (%10s)\n", i, reg[i], Mng[i]);
    }
}
