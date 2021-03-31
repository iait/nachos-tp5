#ifndef SCONSOLE_H
#define SCONSOLE_H
#include "copyright.h"
#include "utility.h"
#include "console.h"
#include "synch.h"

// ###################################################################
//
// Esta clase es escencialemtne igual a Console, salvo que al leer o 
// escribir los locks de lectura y escritura estarán tomados par
// los procesos que estén usando las llamadas a sistema Read y Write 
// con destino de entrada y salida estandar
//
// El funcionamiento es muy similar al que se ejemplifica en ConsoleTest
// en progtest.cc
//
// ###################################################################

class SynchConsole {
  public:
    SynchConsole(const char *readFile, const char *writeFile);
    ~SynchConsole();			

    void PutChar(char ch);

    char GetChar();	   	

// internal emulation routines -- DO NOT call these. 
    void ReadDone(){readAvail->V();}; 
    void WriteDone(){writeDone->V();};

    Lock * lectura , * escritura;
  private:
 
    Console * console;
    Semaphore * readAvail, * writeDone;
    
};
#endif
