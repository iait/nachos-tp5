
//############################################################################
//############################################################################
//############################################################################
// IMPLEMENTACIÓN DE PUERTOS
//############################################################################
//############################################################################
//############################################################################

/*
class Port
{
        public:
                Port(const char * debugNAme );
                ~Port();
                int getID(void){ return ID;}
                void Send(int Mensaje);
                void Receive(int * Mensaje);
        private:
                int enviado;
		int r;
		int s;
		int buffer;
                Lock * lock;
                Condition * receive;
                Condition * send;
}
*/
#include "port.h"
Port::Port(const char *debugName)
{
	leido = 1; 
	r = s = 0;
	lock = new Lock(debugName);
	send = new Condition(debugName , lock);
	receive = new Condition(debugName , lock);
}

Port::~Port()
{
	delete lock;
	delete send;
	delete receive;
}
       
void Port::Send(int Mensaje)
{
	lock->Acquire();
	while(!leido)
		receive->Wait();
	leido = 0;
	buffer = Mensaje;
	s = 1;
	send->Signal();
	while(!r)
		receive->Wait();
	r = 0;
        lock->Release();
}

void Port::Receive(int * Mensaje)
{
        lock->Acquire();
        while(!s)
                send->Wait();
        *Mensaje = buffer;
	leido = 1;
        r = 1;
        receive->Broadcast();
	s = 0;
        lock->Release();
}

