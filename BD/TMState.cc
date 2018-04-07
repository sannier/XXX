#include <fstream>
#include <assert.h>
#include "BitDeviceMachine.h"

#include "debugfile.h"

// Default constructor (never called because of "casting creation")
BitDeviceMachine::TMState::TMState()
{ assert("Default constructor for TMState should never be called");}

// Initialize to given symbol, direction and state triplets
void BitDeviceMachine::TMState::Init(uchar    s0, uchar    s1, uchar    sb,
		   int      d0, int      d1, int      db,
		   unsigned n0, unsigned n1, unsigned nb)
{
    STATIC_MASKS;

    // We dont' get constructors called, because we aren't constructed
    //   per se so ensure opcode is set here:
    opCode = Command::OPTMST;
    
    // Assign the three symbols in order
    sym = smask[s0][0] | smask[s1][1] | smask[sb][2];

    // Assign the three directions in order
    dir = smask[Dir2SYM(d0)][0] |
	  smask[Dir2SYM(d1)][1] |
	  smask[Dir2SYM(db)][2];

    // Assign the three states in order (as bit offsets)
    nxt[0] = STATE2OFF(n0);
    nxt[1] = STATE2OFF(n1);
    nxt[2] = STATE2OFF(nb);
}

// Initialize to a <valid> random state in a table with scnt total states
void BitDeviceMachine::TMState::Random(int scnt)
{
    STATIC_MASKS;
    
    // Initialize random number generator
    RANDINIT;
    
    // Choose random values for the symbols, directions and next states
    //   for each of the three read symbols
    sym  = smask[rand()%3][0]; 
    sym &= smask[rand()%3][1];
    sym &= smask[rand()%3][2];
    
    dir  = smask[rand()%3][0];
    dir &= smask[rand()%3][1];
    dir &= smask[rand()%3][2];
    
    nxt[0] = STATE2OFF(rand()%(scnt+1));
    nxt[1] = STATE2OFF(rand()%(scnt+1));
    nxt[2] = STATE2OFF(rand()%(scnt+1));

    // Validate all values
    assert(Sym(0)  >= 0 && Sym(0)  <= 2);
    assert(Sym(1)  >= 0 && Sym(1)  <= 2);
    assert(Sym(2)  >= 0 && Sym(2)  <= 2);
    assert(Dird(0) >=-1 && Dird(0) <= 1);
    assert(Dird(1) >=-1 && Dird(1) <= 1);
    assert(Dird(2) >=-1 && Dird(2) <= 1);
    assert(Nxts(0) >= 0 && Nxts(0) <= scnt);
    assert(Nxts(1) >= 0 && Nxts(1) <= scnt);
    assert(Nxts(2) >= 0 && Nxts(2) <= scnt);
}

// Accessors
unsigned BitDeviceMachine::TMState::OpCode()
{
    assert(opCode == Command::OPTMST);
    return opCode;
}

uchar BitDeviceMachine::TMState::Sym(uchar s)
{
    STATIC_MASKS;
    assert(s == 0 || s == 1 || s == 2); // Assume valid symbol
    uchar tbyte = sym;

    // Mask out the 2bit symbol corresponding to s position
    //   and shift it into the first two bits
    switch(s)
    {
    case 0:
	tbyte = tbyte & xmask[0]; 
	break;
    case 1:
	tbyte = tbyte & xmask[1];
	tbyte = tbyte >> 2;
	break;
    case 2:
	tbyte = tbyte & xmask[2];
	tbyte = tbyte >> 4;
	break;
    default:
	assert("illegal offset");
    }

    return tbyte;
}

uchar BitDeviceMachine::TMState::Dirs(uchar s)
{
    STATIC_MASKS;
    assert(s == 0 || s == 1 || s == 2); // Assume valid symbol
    uchar tbyte = dir;

    // Mask out the 2bit symbol corresponding to s position
    //   and shift it into the first two bits
    switch(s)
    {
    case 0:
	tbyte = tbyte & xmask[0];
	break;
    case 1:
	tbyte = tbyte & xmask[1];
	tbyte = tbyte >> 2;
	break;
    case 2:
	tbyte = tbyte & xmask[2];
	tbyte = tbyte >> 4;
	break;
    default:
	assert("illegal offset");
    }
    assert(tbyte == 0 | tbyte == 1 | tbyte == 2); // Assume vaid symbol
    return tbyte;
}

int BitDeviceMachine::TMState::Dird(uchar s)
{
    // Get direction as a symbol
    uchar tbyte = Dirs(s);
    
    // Convert symbol to head direction in [-1, 0, 1]
    switch(tbyte)
    {
    case 0:
	return -1;
    case 1:
	return 0;
    case 2:
	return 1;
    }
    assert("can't get here");
    
    return 0;
}

unsigned BitDeviceMachine::TMState::Nxto(uchar s)
{
    // Return as state offset for state change corresponding to s
    assert(s == 0 || s == 1 || s == 2); // Assume valid symbol
    return (nxt[s]);
}

unsigned BitDeviceMachine::TMState::Nxts(uchar s)
{
    // Return as state offset for state change corresponding to s
    assert(s == 0 || s == 1 || s == 2); // Assume valid symbol
    return OFF2STATE(nxt[s]);
}

// Addressors -- useful in validating alignment
uchar* BitDeviceMachine::TMState::SymA(){return         &sym;}
uchar* BitDeviceMachine::TMState::DirA(){return         &dir;}
uchar* BitDeviceMachine::TMState::NxtA(){return (uchar*)&nxt[0];}

// Equality and inequality operators
bool BitDeviceMachine::TMState::operator==(const TMState &other) const
{
    if (sym    != other.sym)    return false;
    if (dir    != other.dir)    return false;
    if (nxt[0] != other.nxt[0]) return false;
    if (nxt[1] != other.nxt[1]) return false;
    if (nxt[2] != other.nxt[2]) return false;
    return true;
}

bool BitDeviceMachine::TMState::operator!=(const TMState &other) const
{
    return !(*this == other);
}


// Convert the given unsigned in [0,1,2] into
//   an signed integer in [-1, 0, 1]
int BitDeviceMachine::TMState::SYM2Dir(unsigned ds)
{
    if(ds == 00) return -1;
    if(ds == 01) return  0;
    if(ds == 2)  return +1;
    assert("ds is invalid symbol");
    return 0;
}

// Convert the given integer in [-1. 0, 1]
//   into an unsigned char in [0, 1, 2]
uchar BitDeviceMachine::TMState::Dir2SYM(int d)
{
    if(d == -1) return 0;
    if(d ==  0) return 1;
    if(d == +1) return 2;
    assert("d is an invalid direction");
    return 1;
}

// Convert a state index into a bit offset from tape start and back again
unsigned BitDeviceMachine::TMState::STATE2OFF(unsigned s)
{
    //TODO:: This depends on the size/layout of TMMachine::TapeA
    //       Should make that explicit somehow?
    return ((MT_HEADERSZ+(s)*sizeof(TMState))*bPB);
}
unsigned BitDeviceMachine::TMState::OFF2STATE(unsigned o)
{
    //TODO:: This depends on the size/layout of TMMachine::TapeA
    //       Should make that explicit somehow?
    return (((o)/bPB-MT_HEADERSZ)/sizeof(TMState));
}

// Print status of current command to debug file
void BitDeviceMachine::TMState::DBGPRINT()
{
    assert(opCode == Command::OPTMST);
    fprintf(DBGFILE, "Sym:(%u,%u,%u) Dir:(%i,%i,%i) Nxt:(%u,%u,%u)[%u,%u,%u]\n",
	    Sym(0), Sym(1), Sym(2),
	    Dird(0), Dird(1), Dird(2),
	    Nxts(0), Nxts(1), Nxts(2),
	    Nxto(0), Nxto(1), Nxto(2));
	    
}


