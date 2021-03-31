// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "scheduler.h"
#include "system.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads to empty.
//----------------------------------------------------------------------

Scheduler::Scheduler()
{
    for(int i=0;i<=10;i++){ 
    	readyListP[i] = new List<Thread*>; 
    }
    readyList = readyListP[5]; //Para mantener compatibilidad con el código original.
    muertos = new List<Thread*>;
    lock = new Lock("Scheduler Lock");
    join = new Condition("Scheduler Join Condition",lock);
	

// ################################################################
// AGREGADO DE LA PRÁCTICA DE USER_PROGRAM
// #################################################################
        idStatus = NULL;  // Este vector almacena los Status de salida 
                          // de los prosesos 
        execNum = 0;
        statusLock = new Lock("Lock para Join de UP");
        statusCond = new Condition("Variabled e condición para Join de UP", statusLock);
// #################################################################
// FIN DEL AGREGADO
// #################################################################
} 


// ################################################################
// AGREGADO DE LA PRÁCTICA DE USER_PROGRAM
// #################################################################
//----------------------------------------------------------------------
// Scheduler::AddStatus
//
//	Agrega un hilo a la lista de status si no existe o actualiza su estado
//	si terminó
//
//----------------------------------------------------------------------
void 
Scheduler::AddStatus(int spaceId , int status)
{
	if(execNum == 0){
		idStatus = (int *)malloc(sizeof(int));
		idStatus[0] = status;
		execNum++;
		return;
	}

	if(spaceId < execNum){
		idStatus[spaceId] = status;
	}else{
		execNum++;
		idStatus = (int *)realloc(idStatus , execNum * sizeof(int));
		idStatus[spaceId] = status;
	}
	return;
}

// #################################################################
// FIN DEL AGREGADO
// #################################################################


//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    delete readyList; 
    delete muertos;
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    DEBUG('t', "Putting thread %s on ready list.\n", thread->getName());

    thread->setStatus(READY);

//
// MODIFICADO PARA QUE SE AGREGUE A SU COLA CORRESPONDIENTE SEGÚN PRIORIDAD
//
    readyListP[thread->getPriority()]->Append(thread);
// FIN DE AGREGADO
//

}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------


//
// MODIFICADA PARA QUE BUSQUE EL SIGUIENTE POR COLA DE PRIORIDAD
//
//

Thread *
Scheduler::FindNextToRun ()
{
    for(int i=10;i>=1;i--)
        if(!(readyListP[i]->IsEmpty()))
            return readyListP[i]->Remove();
    
    return readyListP[0]->Remove();
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread)
{
    Thread *oldThread = currentThread;
    
#ifdef USER_PROGRAM			// ignore until running user programs 
    if (currentThread->space != NULL) {	// if this thread is a user program,
        currentThread->SaveUserState(); // save the user's CPU registers
	currentThread->space->SaveState();
    }
#endif
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    currentThread = nextThread;		    // switch to the next thread
    currentThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG('t', "Switching from thread \"%s\" to thread \"%s\"\n",
	  oldThread->getName(), nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".
    SWITCH(oldThread, nextThread);
    DEBUG('t', "Now in thread \"%s\"\n", currentThread->getName());

    // If the old thread gave up the processor because it was finishing,
    // we need to delete its carcass.  Note we cannot delete the thread
    // before now (for example, in Thread::Finish()), because up to this
    // point, we were still running on the old thread's stack!
    if (threadToBeDestroyed != NULL) {
        delete threadToBeDestroyed;
	threadToBeDestroyed = NULL;
    }
    
#ifdef USER_PROGRAM
    if (currentThread->space != NULL) {		// if there is an address space
        currentThread->RestoreUserState();     // to restore, do it.
	currentThread->space->RestoreState();
    }
#endif
}

//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------

static void
ThreadPrint (Thread* t) {
  t->Print();
}

void
Scheduler::Print()
{
    printf("Ready lists contents:\n");
    for(int i=10;i<=0;i--){
        printf("Imprimiendo lista %d:\n", i);
        readyListP[i]->Apply(ThreadPrint);
        printf("\n");
    }
}

//----------------------------------------------------------------------
// Scheduler::IsDeath
//	Busca un hilo en la cola de hilos terminados y dice si está o no
//----------------------------------------------------------------------

bool Scheduler::IsDeath(Thread *t)
{
    if( muertos->IsEmpty() )
	return false;
    Thread * first, *actual;
    first = muertos->Remove();
    muertos->Append(first);
    if(first == t)
	return true;
    actual = muertos->Remove();
    while( actual != first ){   // Si en la primer comparación son iguales
				// es porque la cola tenía un sólo elemento
	muertos->Append(actual);
	if( actual == t )
	   return true;
        actual = muertos->Remove();
    }
    muertos->Append(actual);
    return false;
}

//------------------------------------------------------------------------
// Scheduler::ChangeList
// 	Toma un hilo "thread" y cambia su prioridad por la del currentThread
//	Luego se busca en su cola de prioridad, si aparece (no estaba en BLOCKED)
//	lo extrae y llama a readyToRun (que lo coloca en su nueva cola)
//------------------------------------------------------------------------

void Scheduler::ChangeList(Thread *thread)
{
    Thread *first, *actual;
    List<Thread *> *list = readyListP[thread->getPriority()];
    thread->SetPriority( currentThread->getPriority() );

    if(list->IsEmpty())
	return;

    first = list->Remove();
    if(first == thread){
        this->ReadyToRun(thread);
        return;
    }
    list->Append(first);
    actual = list->Remove();
    while(actual != first){
        if(actual == thread){
            ReadyToRun(thread);
            return;
        }
        list->Append(actual);
        actual = list->Remove();
    }
    list->Append(actual);

    return;
}

//------------------------------------------------------------------------
// Scheduler::Move
// 	Hace lo mismo que ChangeList, sólo que mueve el hilo a su cola 
//	original en lugar de ir a la del currentThread
//------------------------------------------------------------------------
void Scheduler::Move(Thread *thread)
{
    Thread *first, *actual;
    List<Thread *> *list = readyListP[thread->getPriority()];
    thread->SetPriority( thread->GetOriginalPriority() );

    if(list->IsEmpty())
        return;

    first = list->Remove();
    if(first == thread){
        ReadyToRun(thread);
        return;
    }
    list->Append(first);
    actual = list->Remove();
    while(actual != first){
        if(actual == thread){
            ReadyToRun(thread);
            return;
        }
        list->Append(actual);
        actual = list->Remove();
    }
    list->Append(actual);

    return;
}
