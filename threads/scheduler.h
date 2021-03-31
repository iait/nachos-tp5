// scheduler.h 
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "synch.h"
#include "thread.h"

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.

class Scheduler {
  public:
    Scheduler();			// Initialize list of ready threads 
    ~Scheduler();			// De-allocate ready list

    void ReadyToRun(Thread* thread);	// Thread can be dispatched.
    Thread* FindNextToRun();		// Dequeue first thread on the ready 
					// list, if any, and return thread.
    void Run(Thread* nextThread);	// Cause nextThread to start running
    void Print();			// Print contents of ready list


//
// AGREGADO PARA IMPLEMENTAR JOIN Y MULTICOLAS DE PRIORIDAD
//
    bool IsDeath(Thread *t);		// Busca a "t" en la cola de hilos terminados
    List<Thread*> *muertos;		// Hilos que llamaron a Finish, se guerdan para 
					// liberar a los Join
    Lock *lock;
    Condition *join;
    void Move(Thread*);
    void ChangeList(Thread *);// se encaga de cambiar de prioridad un thread
//
//


// ################################################################
// AGREGADO DE LA PRÁCTICA DE USER_PROGRAM
// #################################################################
	int *idStatus;  // Este vector almacena los Status de salida 
			// de los prosesos 
	void AddStatus(int spaceId , int status); 
	int execNum;
	Condition * statusCond;
	Lock * statusLock;
// #################################################################
// FIN DEL AGREGADO
// #################################################################

  private:
    List<Thread*> *readyList;  		// queue of threads that are ready to run,
					// but not running
    List<Thread*> *readyListP[11];	// Colas de prioridad
};

#endif // SCHEDULER_H
