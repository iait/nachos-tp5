#include "syscall.h"

int main()
{
	int pid;
	pid = Exec("../test/consola1");
	Join(pid);
	pid = Exec("../test/consola2");
	Join(pid);
	Exit(0);
}
