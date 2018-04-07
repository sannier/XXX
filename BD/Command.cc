// Default constructor (never called because of "casting creation")
BitDeviceMachine::Command::Command()
{assert("Default constructor for Command should never be called");}

// Initialize the opcode and arguments that define this command
void BitDeviceMachine::Command::Init(OPCODE oc,
				     int ar0, int ar1, int ar2, unsigned nxt)
{
    opCode = oc;
    arg[0] = ar0;
    arg[1] = ar1;
    arg[2] = ar2;
    nxtCmd = CMDIDX2OFF(nxt);
}

// Convert CMDIDX into bit offset from tape start and back again
unsigned BitDeviceMachine::Command::CMDIDX2OFF(unsigned idx)
{
    unsigned offset = MT_HEADERSZ+idx*sizeof(BitDeviceMachine::Command);
    return bPB*offset;
}
unsigned BitDeviceMachine::Command::OFF2CMDIDX(unsigned o)
{
    unsigned idx = ((o/bPB)-MT_HEADERSZ)/sizeof(BitDeviceMachine::Command);
    return idx;
}

// Accessors
unsigned BitDeviceMachine::Command::OpCode()        {return opCode;}
int      BitDeviceMachine::Command::Arg(unsigned i) {return arg[i];}
unsigned BitDeviceMachine::Command::Nxto()          {return nxtCmd;}
unsigned BitDeviceMachine::Command::Nxti()          {return OFF2CMDIDX(nxtCmd);}

// Print the values in Command into a debug file
void BitDeviceMachine::Command::DBGPRINT()
{
    switch(opCode)
    {
    case Command::OPTMST: 
    {fprintf(DBGFILE, "OPTMST: ");TMState* s = (TMState*)this;s->DBGPRINT();break;}
    case Command::OPCLRR: 
    {fprintf(DBGFILE, "CLRR: \n"); break;}
    case Command::OPLOAD: 
    {fprintf(DBGFILE, "LOAD(%i, %i)\n", arg[0], arg[1]);break;}
    case Command::OPWRDR: 
    {fprintf(DBGFILE, "WRDR(%i, %i)\n", arg[0], arg[1]);break;}
    case Command::OPSYMR: 
    {fprintf(DBGFILE, "SYMR(%i, %i)\n", arg[0], arg[1]);break;}
    case Command::OPMULT: 
    {fprintf(DBGFILE, "MULT(%i, %i, %i)\n", arg[0], arg[1], arg[2]);break;}
    case Command::OPADDN: 
    {fprintf(DBGFILE, "ADDN(%i, %i, %i)\n", arg[0], arg[1], arg[2]);break;}
    case Command::OPSYMW: 
    {fprintf(DBGFILE, "SYMW(%i, %i)\n", arg[0], arg[1]);break;}
    case Command::OPWRDW: 
    {fprintf(DBGFILE, "WRDW(%i, %i)\n", arg[0], arg[1]);break;}
    case Command::OPHALT: 
    {fprintf(DBGFILE, "HALT()\n");break;}
    case Command::OPRTRN: 
    {fprintf(DBGFILE, "RTRN()\n");break;}
    default:
    {assert("Invalid opCode");break;}
    }
}

// Equality and inequality operators
bool BitDeviceMachine::Command::operator==(const BitDeviceMachine::Command &other)const
{
    if(opCode != other.opCode) return false;
    if(arg[0] != other.arg[0]) return false;
    if(arg[1] != other.arg[1]) return false;
    if(arg[2] != other.arg[2]) return false;
    if(nxtCmd != other.nxtCmd) return false;
    return true;
}
bool BitDeviceMachine::Command::operator!=(const BitDeviceMachine::Command &other)const
{
    return !(*this == other);
}
