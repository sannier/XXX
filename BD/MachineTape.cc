//=====================================================================================
// Default constructor (never called because of "casting creation")
BitDeviceMachine::MachineTape::MachineTape()
{assert("Default constructor for MachineTape should never be called");}

// Get/Set the current cmd as an index 
unsigned BitDeviceMachine::MachineTape::getCurrentCommand()
{return Command::OFF2CMDIDX(p);}

void BitDeviceMachine::MachineTape::setCurrentCommand(unsigned nc)
{p = Command::CMDIDX2OFF(nc);}

// Get/Set Number of commands -- z = f(numOfCommands)
unsigned BitDeviceMachine::MachineTape::getNumberOfCommands() const
{return (z-2)*BYTESPERWORD/sizeof(BitDeviceMachine::Command);}

void BitDeviceMachine::MachineTape::setNumberOfCommands(unsigned cnt)
{z = (cnt*sizeof(BitDeviceMachine::Command)/BYTESPERWORD)+2;}

// Returns the length of the MachineTape in bytes
//    Need this because MachineTape has a variable length array
//    so sizeof(MachineTape) doesn't tell the whole story
unsigned BitDeviceMachine::MachineTape::Len()
{
    unsigned sz1 = sizeof(MachineTape)+
	                 (getNumberOfCommands()-1)*sizeof(BitDeviceMachine::Command);
    unsigned sz2 = MT_HEADERSZ+
	                  getNumberOfCommands()*(sizeof(BitDeviceMachine::Command));
    assert(sz1 == sz2);
    return sz1;
}

void BitDeviceMachine::MachineTape::DBGPRINT()
{
    fprintf(DBGFILE, "z=%5d(%5d), p=%d(%d)\n", z, z*32, p, Command::OFF2CMDIDX(p));
    for(int i=0; i<getNumberOfCommands(); i++)
    {
	fprintf(DBGFILE, "%5i(%5lu): ",
		         i, MT_HEADERSZ+(i*sizeof(BitDeviceMachine::Command)));
	cmd[i].DBGPRINT();
    }
}

// Equality and inequality operators
bool BitDeviceMachine::MachineTape::operator==(const BitDeviceMachine::MachineTape &other) const
{
    if (z != other.z) return false;
    if (p != other.p) return false;
    for(int i=0; i<getNumberOfCommands(); i++)
	if(cmd[i] != other.cmd[i])
	    return false;
    return true;
}
bool BitDeviceMachine::MachineTape::operator!=(const BitDeviceMachine::MachineTape &other) const
{
    return !(*this == other);
}
