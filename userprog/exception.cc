// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synchconsole.h"
#include <vector>
#include "../threads/thread.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

char *ReadStringFromMem(int);
void WriteInMem(int buffer , int bytes, char * VirAddr);
char *ReadFromMem(int string , int NumBytes);
void WriteInSTDOUT(char *Buffer , int NumBytesToWrite);
int ReadFromSTDIN(char *Buffer , int NumBytesToRead);
void NewProses(void *arg)
{
	currentThread->space->InitRegisters();;
	currentThread->space->RestoreState(); 
	machine->Run();
}


void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    int pcAfter = machine->registers[NextPCReg] + 4;
    char *arg;
    if (which == SyscallException)
        switch(type){
		case SC_Halt:
                        DEBUG('U',"%s LLAMA A HALT\n",currentThread->getName());
			DEBUG('U', "Shutdown, initiated by user program.\n");
	   		interrupt->Halt();
			break;
		case SC_Create:
                        DEBUG('U',"%s LLAMA A CREATE\n",currentThread->getName());
			arg = ReadStringFromMem(machine->registers[4]);
			fileSystem->Create(arg, 0);
			DEBUG('U',"El archivo %s fue creado exitosamente\n",arg);
			free(arg);
			break;
		case SC_Open:{
			int fileId;
			DEBUG('U',"%s LLAMA A OPEN\n",currentThread->getName());
                        arg = ReadStringFromMem(machine->registers[4]);
			OpenFile * file = fileSystem->Open(arg);
			if(file == NULL){
				DEBUG('U',"El archivo %s no existe!\n",arg);
				DEBUG('U',"Fin del programa por operación inválida\n");
				currentThread->Finish(-1);
			}
			DEBUG('U',"Se abrió el archivo %s exitosamente\n",arg);
			fileId = currentThread->AgregarDescriptor(file);
			machine->registers[2] = fileId;
                        free(arg);
		}
			break;
		case SC_Read:{
			DEBUG('U',"%s LLAMA A READ\n",currentThread->getName());
			int NumBytesToRead = machine->registers[5];
			int IdRead = machine->registers[6];
			int resultRead;
			arg = new char[NumBytesToRead];
			if(IdRead == 1){ //Se intenta leer de STDOUT
				DEBUG('U',"%s intenta leer de STDOUT\n");
				machine->registers[2] = -1;
				break;
			}
			if(IdRead == 0){ // Lectura desde STDIN
				DEBUG('U',"%s lee de STDIN\n",currentThread->getName());
				resultRead = ReadFromSTDIN(arg,NumBytesToRead);
				WriteInMem(machine->registers[4] , resultRead , arg);
                                DEBUG('U',"%s leyó %d bites exitosamente\n",currentThread->getName(),resultRead);
/* ##########################################################################################
// Solo para debug. BORRAR AL COMPLETAR ReadFromSTDIN
				for(int j = 0 ; j < NumBytesToRead ; j++)
					arg[j] = 'a'+j;
                                WriteInMem(machine->registers[4] , NumBytesToRead , arg);
// ########################################################################################## */
				machine->registers[2] = resultRead;
				break;
			}
			DEBUG('U',"%s intenta leer %d bytes del archivo con id: %d\n",currentThread->getName(),NumBytesToRead,IdRead);
			OpenFile *lectura;
			lectura = currentThread->GetDescriptor(IdRead);
			if(lectura == NULL){
				DEBUG('U',"Se intenta leer de un archivo inválido\n");
                                DEBUG('U',"Fin del programa por error de lectura\n");
				currentThread->Finish(-1);
			}else{
				resultRead = lectura->Read(arg, NumBytesToRead);
				WriteInMem(machine->registers[4] , resultRead , arg);
				DEBUG('U',"%s leyó %d bites exitosamente\n",currentThread->getName(),resultRead);
				machine->registers[2] = resultRead;
			}
			delete [] arg;
			break;
		}
		case SC_Write:{
			DEBUG('U',"%s LLAMA A WRITE\n",currentThread->getName());
			int NumBytesToWrite = machine->registers[5];
			int WriteResult;
			int IdWrite = machine->registers[6];
			arg = ReadFromMem(machine->registers[4] , NumBytesToWrite);
			if(IdWrite == 0){ // Intenta escribir en STDIN
				DEBUG('U',"%s intenta escribir en STDIN\n");
                                machine->registers[2] = -1;
                                break;
			}
			if(IdWrite == 1){
				DEBUG('U' , "%s escibe en STDOUT\n",currentThread->getName());
				WriteInSTDOUT(arg , NumBytesToWrite);
				break;
			}
			DEBUG('U',"%s intenta escribir %d bytes del archivo con id: %d\n",currentThread->getName(),NumBytesToWrite,IdWrite);
			OpenFile *escritura;
			escritura = currentThread->GetDescriptor(IdWrite);
                        if(escritura == NULL){
                                DEBUG('U',"Se intenta escribir en un archivo inválido\n");
                                DEBUG('U',"Fin del programa por error de escritura\n");
                                currentThread->Finish(-1);
                        }else{
				WriteResult = escritura->Write(arg, NumBytesToWrite);
				DEBUG('U', "%s escribió %d bites de forma exitosa\n",currentThread->getName(),WriteResult);
			}
			free(arg);
			break;
		}	
		case SC_Close:{ // Tomamos como implementación que no se puedan cerrar los descriptores estandar (0 y 1)
			DEBUG('U',"%s LLAMA A CLOSE\n", currentThread->getName());
			int argClose;
			argClose = machine->registers[4];
			if(argClose == 0 || argClose == 1){
				DEBUG('U',"No se pueden cerrar los descriptores de entrada y salida estandar\n");
				break;
			}
			OpenFile *cerrar;
			cerrar = currentThread->GetDescriptor(argClose);
			currentThread->BorrarDescriptor(argClose);
			if(cerrar != NULL)
				delete cerrar;
			break;
		}
		case SC_Exec:{
			DEBUG('U',"%s LLAMA A EXEC\n", currentThread->getName());
			arg = ReadStringFromMem(machine->registers[4]);
			OpenFile *executable = fileSystem->Open(arg); 
			AddrSpace *space;
			if (executable == NULL) {
				DEBUG('U', "No se pudo hacer exec del archivo %s\n", arg);
				machine->registers[2] = -1;
				break;
			}
			space = new AddrSpace(executable);
			machine->WriteRegister(2, space->spaceId);
			scheduler->AddStatus(space->spaceId , 128);
			delete executable;
			Thread *thread = new Thread(arg);
			thread->space = space;
			//currentThread->SaveUserState();
			thread->Fork(NewProses, (void *)0);
			//currentThread->RestoreUserState();
			}
			break;
		case SC_Exit:
			DEBUG('U',"%s LLAMA A EXIT\n", currentThread->getName());
			DEBUG('U',"%s spaceId %d\n", currentThread->getName() , currentThread->space->spaceId);
			
			currentThread->Finish(machine->registers[4]);
			break;
		case SC_Join:{
			DEBUG('U',"%s LLAMA A JOIN\n", currentThread->getName());
			int pid = machine->registers[4];
			if(pid >= scheduler->execNum){
				DEBUG('U',"PID inválido\n");
	                        machine->registers[2] = -1;
				break;
			}
			DEBUG('U',"Sobre el proceso de spaceId %d\n", pid);
		        scheduler->statusLock->Acquire();
			currentThread->SaveUserState();
			while(scheduler->idStatus[pid] == 128 )
		                scheduler->statusCond->Wait();
			currentThread->RestoreUserState();
			machine->registers[2] = scheduler->idStatus[pid];
			DEBUG('U',"JOIN DEVOLVIÓ EL ESTATUS %d\n", machine->registers[2]);
		        scheduler->statusLock->Release();
			break;
		}
	    	default:
		   	printf("Unexpected user mode exception %d %d\n", which, type);
                        currentThread->Finish(-1);
//			ASSERT(false);
			break;
	}
    machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
						// are jumping into lala-land
    machine->registers[PCReg] = machine->registers[NextPCReg];
    machine->registers[NextPCReg] = pcAfter;
}

//-------------------------------------------------------------
// ReadStringFromMem
//
//	Copia una cadena desde mainMemory a un bufer
//
//-------------------------------------------------------------

char *ReadStringFromMem(int string)
{
	std::vector<char> buffer;
	int n;

	ASSERT(machine->ReadMem(string, 1, &n));
	while(n != '\0'){
		buffer.push_back(n);
		ASSERT(machine->ReadMem(++string, 1, &n));
	}
	buffer.push_back(n);
	char *ret = (char *)malloc(buffer.size()*sizeof(char));
	strcpy(ret, &(buffer.at(0)));

	return ret;
}

// ---------------------------------------------------------------------
// WriteInMen
//
// 	Escribe los primeros 'bytes' bytes de un buffer en mainMemory
// 	a partir de la dirección VirAddr
// ------------------------------------------------------------------

void  WriteInMem(int VirAddr , int bytes , char* buffer)
{
	for(int i = 0 ; i < bytes ; i++)
		machine->WriteMem(VirAddr+i,1,(int)buffer[i]);
}

// ------------------------------------------------------------------
// ReadFromMem
//	Devuelve un buffer con lo que apunta la dirección
//	VirAddr[0] ... VirAddr[NumBytes-1]
// ------------------------------------------------------------------

char *ReadFromMem(int VirAddr , int NumBytes)
{
        std::vector<char> buffer;
        int n;
	for(int i = 0 ; i < NumBytes ; i++){
                machine->ReadMem(VirAddr, 1, &n);
		VirAddr++;
                buffer.push_back(n);
        }
        char *ret = (char *)malloc(sizeof(char)*buffer.size());
	for(int i = 0 ; i < NumBytes ; i++)
		ret[i] = buffer.at(i);
        return ret;
}

void WriteInSTDOUT(char *Buffer , int NumBytesToWrite)
{
	synchconsole->escritura->Acquire();
	for(int i = 0 ; i < NumBytesToWrite ; i++)
		synchconsole->PutChar(Buffer[i]);
	synchconsole->escritura->Release();

}

int ReadFromSTDIN(char *Buffer , int NumBytesToRead)
{

	synchconsole->lectura->Acquire();
	for(int i = 0 ; i < NumBytesToRead ; i++)
		Buffer[i] = synchconsole->GetChar();
	synchconsole->lectura->Release();

	return NumBytesToRead;
}
