#include "syscall.h"

int main()
{
        OpenFileId file;
        char buf[8];
        file = Open("Prueba");
        Read(buf , 7 , file);
        Write(buf,7,1);
        Write(", hola\n",7,1);
        Exit(0);
}

