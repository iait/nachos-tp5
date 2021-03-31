#include "syscall.h"

int main()
{
	OpenFileId file;
	char buf[8];
	Create("Prueba2");
	file = Open("Prueba2");
	Read(buf , 4 , 0);
	Write(buf , 4 , file);
	Write("Hola",4,1);
	Close(file);
	Create("Prueba");
	file = Open("Prueba");
	Write(" mundo!\n",8,file);
	Close(file);
	file = Open("Prueba");
	Read(buf , 8 , file);
	Write(buf,8,1);
	Exit(0);
}
