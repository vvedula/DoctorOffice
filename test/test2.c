
#include "syscall.h"

void newThread(void);

int main()
{
	int j;
	for(j=0;j<110;j++)
	{
	 	Printf1((unsigned int)"Thread:",j);
		Fork(newThread);
	}	
	return 0;
}

void newThread()
{
	Yield();		
}
