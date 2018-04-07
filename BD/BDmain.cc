#include <iostream>
#include <fstream>
#include "BDTests.h"
#include "BitDeviceDemon.h"

void UsageMessage()
{
    std::cout<< "usage: BD <fname1> [-o <fname2>] [-h][-s][-q]";
    std::cout << std::endl; 
    std::cout << "   h    : print this help message"     << std::endl;
    std::cout << "   i    : set filename for input"      << std::endl;
    std::cout << "   o    : set filename for output"     << std::endl;
}

class CMDOPTIONS
{
public:
    char* inname;
    char* outname;
    CMDOPTIONS(int argc, char* argv[]);
};

CMDOPTIONS::CMDOPTIONS(int argc, char* argv[])
{
    // Initialize options
    inname     = 0;
    outname    = 0;

    // Process first required argument
    if(argc>1)     // Input filename...
    {
	inname = argv[1];
	std::ifstream f(inname, std::ifstream::in | std::ifstream::binary);
	if(!f) inname = 0;
    }
    
    // Look through the rest of the arguments...
    for(int i=2; i < argc; i++)
    {
	if(!strcmp(argv[i], "-o")) // Output filename...
	{
	    if(i+1 < argc) outname = argv[++i];
	    if(!outname) 
	    {
		std::cout << "Valid filename must follow -o" << std::endl;
		exit (0);
	    }
	}
	// Help
	else if(!strcmp(argv[i], "-h"))
	{
	    UsageMessage();
	    exit (0);
	}
	// Bad option
	else
	{
	    std::cout << "Invalid option: " << argv[i] << std::endl;
	    UsageMessage();
	    exit(0);
	}
    }
    
}

int main(int argc, char* argv[])
{
    // Run tests just to be sure all is well
    RunTests();

    // Look through the arguments...
    CMDOPTIONS opt(argc, argv);

    // Make a demon
    BitDeviceDemon BDD;

    // Input file or InitToAdd1
    if(opt.inname)
	BDD.Read(opt.inname);
    else
    {
	std::cout << "Valid filename must be first argument" << std::endl;
	UsageMessage();
	exit (0);
    }
	
    // Execute a single step
    BDD.ExecuteS(0);

    // Write it out if we have a filename
    if(opt.outname)
	BDD.Write(opt.outname);
	
}

