#ifndef DEBUGFILE
#define DEBUGFILE

#include <stdio.h>
#include <stdlib.h>

extern FILE* DBGFILE;
extern void DBGPRINTS(const char* s);
extern void DBGFLUSH();


/*
FILE* DBGFILE;
void OPENDBGFILE()
{
    DBGFILE = fopen("DBGFILE.txt", "w");
}
void CLOSEDBGFILE()
{
    fclose(DBGFILE);
}
void DBGPRINTS(const char* s)
{
    fprintf(DBGFILE, "%s\n", s);
}
void DBGFLUSH()
{
    fflush(DBGFILE);
}
*/

#endif
