all: BD BDM

BD : BDmain.o BitDevice.o BitDeviceDemon.o
	g++ -DDEBUG -g BDmain.o BitDevice.o BitDeviceDemon.o -o BD

BDM : BDMmain.o BitDevice.o BitDeviceMachine.o TMState.o
	g++ -DDEBUG -g BDMmain.o BitDevice.o BitDeviceMachine.o TMState.o -o BDM

BDMmain.o : BDMmain.cc BitDevice.h BitDeviceDemon.h
	g++ -DDEBUG -g -c BDMmain.cc

BDmain.o : BDmain.cc BitDevice.h BitDeviceDemon.h
	g++ -DDEBUG -g -c BDmain.cc

TMState.o : TMState.cc BitDevice.h BitDeviceDemon.h
	g++ -DDEBUG -g -c TMState.cc

BitDeviceDemon.o : BitDeviceDemon.cc BitDeviceDemon.h BitDevice.h 
	g++ -DDEBUG -g -c BitDeviceDemon.cc

BitDevice.o : BitDevice.cc BitDevice.h 
	g++ -DDEBUG -g -c BitDevice.cc

BitDeviceMachine.o : BitDeviceMachine.cc BitDeviceMachine.h BitDevice.h 
	g++ -DDEBUG -g -c BitDeviceMachine.cc

clean :
	rm -f BD *.o 
	rm -f *~

