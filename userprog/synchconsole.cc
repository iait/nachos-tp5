#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
//Dummy functions
static void SReadAvail(void* arg) { SynchConsole* synchconsole = (SynchConsole*) arg; synchconsole->ReadDone(); }
static void SWriteDone(void* arg) { SynchConsole* synchconsole = (SynchConsole*) arg; synchconsole->WriteDone(); }

//----------------------------------------------------------------------
// SynchConsole::SynchConsole
// 	Initialize the simulation of a hardware console device.
//
//	"readFile" -- UNIX file simulating the keyboard (NULL -> use stdin)
//	"writeFile" -- UNIX file simulating the display (NULL -> use stdout)
//----------------------------------------------------------------------

SynchConsole::SynchConsole(const char *readFile, const char *writeFile)
{
	console = new Console(readFile,writeFile,SReadAvail,SWriteDone,this);
	lectura = new Lock("Lock de lectura de la consola"); 
	escritura = new Lock("Lock de escritura de la consola");
	readAvail = new Semaphore("Semáforo de lectura de la consola",0);
	writeDone = new Semaphore("Semáforo de escritura de la consola",0);
}

//----------------------------------------------------------------------
// SynchConsole::~SynchConsole
// 	Clean up console emulation
//----------------------------------------------------------------------

SynchConsole::~SynchConsole()
{
	delete lectura;
	delete escritura;
	delete readAvail;
	delete writeDone;
	delete console;	
}

//----------------------------------------------------------------------
// SynchConsole::GetChar()
// 	Read a character from the input buffer, if there is any there.
//	Either return the character, or EOF if none buffered.
//----------------------------------------------------------------------

char
SynchConsole::GetChar()
{
	char ch;
	readAvail->P();
	ch = console->GetChar();
	return ch;
}

//----------------------------------------------------------------------
// SynchConsole::PutChar()
// 	Write a character to the simulated display, schedule an interrupt 
//	to occur in the future, and return.
//----------------------------------------------------------------------

void
SynchConsole::PutChar(char ch)
{
	console->PutChar(ch);
	writeDone->P();
}

