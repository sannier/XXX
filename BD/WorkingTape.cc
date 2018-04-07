//=============================================================================
// Default constructor
BitDeviceMachine::WorkingTape::WorkingTape(){assert("Should never be called");}

// Set all of a tapes symbols to 0
void BitDeviceMachine::WorkingTape::clear()
{
    for(int i=0; i<tapeLen(); i++)
	assign(2, i);
}

// Bits are fundamental unit
// 1   symbol == 2 bits
// 1    uchar == 4 symbols ==  8 bits
// 1 unsigned == 4 bytes   == 16 symbols == 32 bits
void BitDeviceMachine::WorkingTape::initTape(const char* str,
					     unsigned    strPos,
					     unsigned    nh)
{
    // Make sure string fits on tape and headPos is legal
    assert(strPos < tapeLen() && strPos-strlen(str) >= 0);
    assert(nh >= 0 && nh < tapeLen());

    // Set tape to all blanks
    clear();

    // Position the head at given position and copy each symbol in the string
    // onto successive postions on the tape,  moving the head to the right
    // 2-bits at a time
    setHead(strPos);
    for(int i=0; i<strlen(str); i++)
    {
	assert(str[i] == '0' || str[i] == '1' || str[i] == ' ');
	uchar x = (str[i] == '0' ? 0 : (str[i] == '1' ? 1 : 2));
	Write(x);
	assert(Read() == x);
	setHead(strPos-i);
    }

    // Position the head at given np
    setHead(nh);
}

// Length of the whole structure including: z, p and tape in bytes
unsigned BitDeviceMachine::WorkingTape::Len() const
{
    unsigned sz1 = sizeof(BitDeviceMachine::WorkingTape) +
	           ((BYTESPERWORD*z)-sizeof(BitDeviceMachine::WorkingTape));
    unsigned sz2 = MT_HEADERSZ + 4 + (BYTESPERWORD*z-12);
    unsigned sz3 = z*BYTESPERWORD;
    assert(sz1 == sz2 && sz3 == sz1);
    return sz1;
}

// Length of the (usable) tape in symbols
// TODO: Lets elimintate use of setTapeLen and set in terms of bits/bytes
void BitDeviceMachine::WorkingTape::setTapeLen(unsigned nl)
{
    unsigned nz = ((nl/SYMPERBYTE)/BYTESPERWORD) +2;
    z = nz;
}
unsigned BitDeviceMachine::WorkingTape::tapeLen() const 
{
    return (BYTESPERWORD*(z-2))*SYMPERBYTE;
}

// Set head -- given in symbols, stored in bit offset
//    from start of working tape
void BitDeviceMachine::WorkingTape::setHead(unsigned nh)
{
    //TODO: Should we clamp instead of assert?
    assert(nh < tapeLen());
    h = IND2OFF(nh);
}

// Get head -- stored as bit offset from start of
//   working tape --- return in symbols
unsigned BitDeviceMachine::WorkingTape::getHead() const
{
    assert(OFF2IND(h) < tapeLen());
    return (OFF2IND(h));
}
	
void BitDeviceMachine::WorkingTape::printTapeLine(unsigned opCnt)
{
    // Store the position of the head
    // Move the head from one end of the tape to the other,
    // reading each symbol and printing it out as you go...
    // Write a line with the values on the tape, each seperated by '.'
    // Finally, replace the head to the original positon
    int oldHead = getHead();
    setHead(tapeLen()-1);
    for(int i=tapeLen()-1; i>=0; i--, Move(-1))
    {
	unsigned x = Read();
	assert(x == 0 || x == 1 | x == 2);
	char c = (x == 0 ? '0' : (x == 1 ? '1' : ' '));
	std::cout<<c<< '|';
    }
    std::cout<<"    :" << opCnt << std::endl;
    setHead(oldHead);
}

void BitDeviceMachine::WorkingTape::printHeadLine()
{
    for(int i=tapeLen()-1; i>=0; i--)
	if(i == getHead())
	    std::cout<<'^'<< ' ';
	else 
	    std::cout<<' '<< ' ';
    std::cout<<std::endl;
}

void BitDeviceMachine::WorkingTape::printStateLine(unsigned state)
{
    for(int i=tapeLen()-1; i>=0; i--)
	if(i == getHead())
	    if(state == 0)
		std::cout << '0';
	    else
		std::cout<<state<< ' ';
	else 
	    std::cout<<' '<< ' ';
    std::cout<<std::endl;
}

// Assign the symbol in s to the symbol location np
void BitDeviceMachine::WorkingTape::assign(uchar sbyte, unsigned nh)
{
    STATIC_MASKS;
    
    assert(nh < tapeLen());

    // Identify the target byte
    // We expect both bits to be in same byte
    unsigned tbyteA   = nh/SYMPERBYTE;
    unsigned toffsetA = nh%SYMPERBYTE*2;
    uchar tbyte = *(T+tbyteA);

    // Mask out the target bits and replace them with source bits
    switch(toffsetA)
    {
    case 0:
	tbyte = tbyte & emask[0];
	break;
    case 2:
	tbyte = tbyte & emask[1];
	sbyte = sbyte << 2;
	break;
    case 4:
	tbyte = tbyte & emask[2];
	sbyte = sbyte << 4;
	break;
    case 6:
	tbyte = tbyte & emask[3];
	sbyte = sbyte << 6;
	break;
    default:
	assert("illegal offset");
    }
    tbyte = tbyte | sbyte;

    // Replace original target byte with newly edited tbyte
    *((uchar*)(T)+tbyteA) = tbyte;
}

// Return the symbol at nh -- in symbols
uchar BitDeviceMachine::WorkingTape::value(unsigned nh)
{
    STATIC_MASKS;
    
    assert(nh < tapeLen());

    // Identify the target byte
    // We expect both bits to be in same byte
    unsigned tbyteA   = nh/SYMPERBYTE;
    unsigned toffsetA = nh%SYMPERBYTE*2;
    uchar tbyte = *(((uchar*)T)+tbyteA);

    // Mask out the target bits and shift them to pos 0 and 1 in the byte
    switch(toffsetA)
    {
    case 0:
	tbyte = tbyte & xmask[0]; //00000011
	break;
    case 2:
	tbyte = tbyte & xmask[1]; //00001100
	tbyte = tbyte >> 2;
	break;
    case 4:
	tbyte = tbyte & xmask[2]; //00110000
	tbyte = tbyte >> 4;
	break;
    case 6:
	tbyte = tbyte & xmask[3]; //11000000
	tbyte = tbyte >> 6;
	break;
    default:
	assert("illegal offset");
    }
    
    return tbyte;
}

// Covert a symbol position in a tape into a bit offset and back again
//   Count from the start of the tape
unsigned BitDeviceMachine::WorkingTape::IND2OFF(unsigned ns)
{return (MT_HEADERSZ+2*(ns));}
unsigned BitDeviceMachine::WorkingTape::OFF2IND(unsigned o)
{return ((o)-MT_HEADERSZ)/2;}

// Move the head left or right (or leave it)
//   clamp between 0 and tapeLen-1 without fault
// TODO: Allow it to increase in size if it moves
//       out of bounds instead of clamping
void BitDeviceMachine::WorkingTape::Move(int d)
{
    assert(d == -1 || d == 0 | d == 1);

    unsigned nh;
    if(h == 0)
	nh = 0;
    else 
	nh = getHead()+d;

    if(nh >= tapeLen())
	nh = tapeLen()-1;

    setHead(nh);
}

// Read the symbol at the head position and return it as an
//   unsigned value in [0|1|2] (2 is blank)
uchar BitDeviceMachine::WorkingTape::Read()
{return value(getHead());}

// Write the symbol s in [0|1|2] on the tape at the head position  
void BitDeviceMachine::WorkingTape::Write(uchar x)
{
    unsigned nh = getHead();
    assign(x, nh);
    assert(value(nh) == x);
}

// Print the tape out to the console -- indicate that the machine
//   working this tape is in the state given
void BitDeviceMachine::WorkingTape::Print(unsigned state, unsigned opCnt)
{
    // If not the first time we called the function, rewind the
    // command line so we print over the lines previously printed
    REWIND;

    // Write a line containing the symbols on the tape
    printTapeLine(opCnt);

    // Write a line with a caret markign the current position of the head
    printHeadLine();
    
    // Write a line with an int for the current state appearing under the caret
    printStateLine(state);
}

void BitDeviceMachine::WorkingTape::DBGPRINT()
{
    char buffer[250];
    // Store the position of the head
    // Move the head from one end of the tape to the other,
    // reading each symbol and printing it out as you go...
    // Write a line with the values on the tape, each seperated by '.'
    // Finally, replace the head to the original positon
    int oldHead = getHead();
    setHead(tapeLen()-1);
    int j=0;
    for(int i=tapeLen()-1; i>=0; i--, Move(-1))
    {
	unsigned x = Read();
	assert(x == 0 || x == 1 | x == 2);
	char c = (x == 0 ? '0' : (x == 1 ? '1' : ' '));
	buffer[j++] = c;
	buffer[j++] = '|';
    }
    setHead(oldHead);
    buffer[j] = '\0';
    fprintf(DBGFILE, "%s\n", buffer);
}


// Equality and inequality operators
bool BitDeviceMachine::WorkingTape::operator==(const BitDeviceMachine::WorkingTape &other) const
{
    if (h != other.h) return false;
    if (z != other.z)  return false;
    for(int i=0; i<tapeLen()/bPB; i+=bPB)
	if(T[i] != other.T[i]) return false;
    return true;
}

bool BitDeviceMachine::WorkingTape::operator!=(const BitDeviceMachine::WorkingTape &other) const
{
    return !(*this == other);
}
