#include "syscall.h"

int main()
{
	char buf[4];
	OpenFileId WriteId,ReadWriteId;
	//Create("File1");
	//Create("File2");
	Create("ReadAndWriteFile");
	Create("WriteFile");
	ReadWriteId = Open("ReadAndWriteFile");
	WriteId = Open("WriteFile");
	Write("Hola\n" , 5 , ReadWriteId);
	Close(ReadWriteId);
	ReadWriteId = Open("ReadAndWriteFile");
	Read(buf , 4 , ReadWriteId);
	Close(ReadWriteId);
	Write(buf , 4 , WriteId);
	Write("\n" , 1 , WriteId);
	Close(WriteId);
	Exit(0);
}
