#include <iostream>
#include <fstream>
#include <assert.h>
#include "BDTests.h"
#include "BitDeviceMachine.h"

void UsageMessage()
{
    std::cout<< "usage: BDM -i <fname1>|-Add1|-Sub1|-PAL|-BB3 [-o <fname2>] [-h][-s][-q]";
    std::cout << std::endl; 
    std::cout << "   h    : print this help message"     << std::endl;
    std::cout << "   i    : set filename for input"      << std::endl;
    std::cout << "   o    : set filename for output"     << std::endl;
    std::cout << "   Add1 : make an Add1 machine"        << std::endl;
    std::cout << "   Sub1 : make a  Sub1 machine"        << std::endl;
    std::cout << "   BB3  : make a 3-state busy beaver"  << std::endl;
    std::cout << "   BB4  : make a 4-state busy beaver"  << std::endl;
    std::cout << "   PAL  : make a palindrom detector"   << std::endl;
    std::cout << "   n    : no execution"                << std::endl;
    std::cout << "   s    : execute a single step"       << std::endl;
    std::cout << "   q    : execute without output"      << std::endl;
}

class CMDOPTIONS
{
public:
    enum mtype {Add1, Sub1, BB3, BB4, PAL};
    char* inname;
    char* outname;
    bool  singleStep;
    bool  silent;
    bool  noExec;
    mtype type;

    CMDOPTIONS(int argc, char* argv[]);
};

CMDOPTIONS::CMDOPTIONS(int argc, char* argv[])
{
    // Initialize options
    inname     = 0;
    outname    = 0;
    singleStep = false;
    silent     = false;
    noExec     = false;
    type       = Sub1;
    
    // Process first required argument
    int i = 1;
    if(argc>1)     // Input filename...
    {
	if(!strcmp(argv[i], "-i"))
	{
	    if(argc>2)
		inname = argv[++i];
	    std::ifstream f(inname, std::ifstream::in | std::ifstream::binary);
	    if(!f) inname = 0;
	}
	else if(!strcmp(argv[i], "-Add1")){type = Add1; i++;}
	else if(!strcmp(argv[i], "-Sub1")){type = Sub1; i++;}
	else if(!strcmp(argv[i], "-BB3")) {type = BB3;  i++;}
	else if(!strcmp(argv[i], "-BB4")) {type = BB4;  i++;}
	else if(!strcmp(argv[i], "-PAL")) {type = PAL;  i++;} 
    }
    // Look through the rest of the arguments...
    for(; i < argc; i++)
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
	else if(!strcmp(argv[i], "-s")) // Single step
	    singleStep = true;
	else if(!strcmp(argv[i], "-q")) // Silent
	    silent = true;
	else if(!strcmp(argv[i], "-n")) // No Execution (overrides s)
	    noExec = true;
	else if(!strcmp(argv[i], "-h")) // Help
	{
	    UsageMessage();
	    exit (0);
	}
	else // Bad option
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

    // Make a BitDeviceMachine
    BitDeviceMachine BDM;

    // Input file or Init to requested type
    if(opt.inname)
	BDM.ReadFile(opt.inname);
    else
    {
	switch(opt.type)
	{
	case CMDOPTIONS::Add1:{BDM.InitToAdd1(); break;}
	case CMDOPTIONS::Sub1:{BDM.InitToSub1(); break;}
	case CMDOPTIONS::BB3: {BDM.InitToBB3();  break;}
	case CMDOPTIONS::BB4: {BDM.InitToBB4();  break;}
	case CMDOPTIONS::PAL: {BDM.InitToPAL();  break;}
	default:  {assert("Invalid machine type");}
	}
    }
	
    // Execute either a single step or until halt
    if(!opt.noExec)
    {
	if(opt.singleStep)
	    BDM.ExecuteS();
	else
	    BDM.Execute(opt.silent);
    }
    
    // Write it out if we have a filename
    if(opt.outname)
	BDM.WriteFile(opt.outname);
}

