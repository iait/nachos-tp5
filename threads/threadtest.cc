// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create several threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//
// Parts from Copyright (c) 2007-2009 Universidad de Las Palmas de Gran Canaria
//

#include "copyright.h"
#include "system.h"
#include "port.h"
#include "synch.h"

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 10 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"name" points to a string with a thread name, just for
//      debugging purposes.
//----------------------------------------------------------------------
int asdfasdf;

void lector(void *);
void escritor(void *);
void death(void*);
void malo(void*);
void padre(void*);
void hijo(void*);
void minprio(void *l);
void maxprio(void *l);


void
SimpleThread(void* name)
{
    // Reinterpret arg "name" as a string
    char* threadName = (char*)name;
    
    // If the lines dealing with interrupts are commented,
    // the code will behave incorrectly, because
    // printf execution may cause race conditions.
    for (int num = 0; num < 10; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
	printf("*** thread %s looped %d times, asdf: %d:\n", threadName, num, asdfasdf++);
	//interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s has finished\n", threadName);
    //interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between several threads, by launching
//	ten threads which call SimpleThread, and finally calling 
//	SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

	Port * puertoDePrueba;
	Lock * lock;
	
	puertoDePrueba = new Port("PuertoDePrueba");
	lock = new Lock("LockDeDeath");

      Thread* newThread = new Thread ("Lector");
      Thread* newThread5 = new Thread ("Lector");
      Thread* newThread4 = new Thread ("MaxPrio",6);
      Thread* newThread3 = new Thread ("MaxPrio",5);
      Thread* newThread2 = new Thread ("MaxPrio",6);
      Thread* newThread7 = new Thread ("Death",6);

      newThread7->Fork (death, (void*)lock);
  //    newThread2->Fork (escritor, (void*)puertoDePrueba);
      newThread3->Fork (malo , (void*)lock);
 //     newThread4->Fork (maxprio , (void *)lock);
	

   
/*  
   for ( int k=1; k<=4; k++) {
      char* threadname = new char[100];
      sprintf(threadname, "Hilo %d", k);
      Thread* newThread = new Thread (threadname);
      newThread->Fork (SimpleThread, (void*)threadname);
    }
    
    SimpleThread( (void*)"Hilo 0");
*/
}


//------------------------------------------------------------------
// Funciones de prueba para variables de condición y puertos
//
//------------------------------------------------------------------

void lector(void *puerto)
{
	Port * lectura = (Port *)puerto;
	int buf;
	lectura->Receive(&buf);
	printf("%d\n",buf);
}

void escritor(void *puerto)
{
	Port * escritura = (Port *)puerto;
	escritura->Send(10);
}

void death(void * lock)
{
	((Lock *)lock)->Acquire();
	for(int i = 0 ; i < 100 ; i++){
		printf("Llegué %d \n", i);
	((Lock *)lock)->Release();
	((Lock *)lock)->Acquire();
	}
	((Lock *)lock)->Release();
}

void malo(void * lock)
{
	((Lock *)lock)->Release();
}

void padre(void *hijo)
{
	Thread * h = (Thread *)hijo;
	currentThread->Join(h);
	printf("Al fin te moriste\n");
}

void hijo(void *a)
{
	printf("Me muero...\n");
}

void maxprio(void *l)
{
        printf("Mnxprio tomó el lock con prioridad %d \n", currentThread->getPriority() );
	Lock *lock = (Lock *)l;
	lock->Acquire();
	printf("Maxprio tomó el lock\n");
	lock->Release();
}


void minprio(void *l)
{
        Lock *lock = (Lock *)l;
	
        printf("Minprio tomó el lock con prioridad %d \n", currentThread->getPriority() );
        lock->Acquire();
	Thread * newThread = new Thread("lalala",9);
        newThread->Fork (maxprio , (void *)lock);
	currentThread->Yield();
        printf("Minprio tomó el lock con prioridad %d \n", currentThread->getPriority() );
        lock->Release();
        printf("Minprio tomó el lock con prioridad %d \n", currentThread->getPriority() );
}


void infinito(void *a)
{
}
