#ifndef SYNTACTIC_SUGAR_H
#define SYNTACTIC_SUGAR_H

#define REWIND \
    static bool firstTime = true;\
    if(!firstTime)\
    {\
	usleep(500000);\
	std::cout << "\033[1A" << std::flush;\
	std::cout << "\033[1A" << std::flush;\
	std::cout << "\033[1A" << std::flush;\
    }\
    else\
	firstTime = false;
    
#define RANDINIT \
    static bool firstTime = true;\
    if(firstTime)\
	srand (time(NULL));\
    else\
	firstTime = false;


// Experiment with uchar for unsigned char
#define uchar unsigned char


// SOME SYNTACTIC SUGAR FOR MASKING
// iMask to isolate each of the four symbol positions
//              0000011, 00001100, 00110000, 11000000
// eMask to erase each of the four symbol positions in a word
//              11111100, 11110011, 11001111, 00111111
// sMask to write each of the three symbols at each of the four positions
// 00000000, 00000000, 00000000, 00000000
// 00000001, 00000100, 00010000, 01000000
// 00000010, 00001000, 00100000, 10000000
#define STATIC_MASKS \
    static unsigned char xmask[4] = {3, 12, 48, 192};\
    static unsigned char emask[4] = {252, 243, 207, 63};\
    static unsigned char smask[3][4] = {0, 0,  0,   0,  \
					1, 4, 16,  64,	\
					2, 8, 32, 128};

//=============================================================
// Cheats to help with address arithmetic
//TODO: Put these where they belong
#define MAXREGS 50

// Bits per bytes
#define bPB 8

//Bytes per word
#define BYTESPERWORD 4

// Symbols per byte
#define SYMPERBYTE 4

// MachineTape header: z and p
#define MT_HEADERSZ 2*sizeof(unsigned)

// Offset to the halt state
#define HALTSTATEOFF 2*MT_HEADERSZ*bPB

// Offset to the first command (skip header and halt state)(224)
//   using 160 instead of sizeof(Command)
#define FIRSTCMDOFF  MT_HEADERSZ+160



#endif



