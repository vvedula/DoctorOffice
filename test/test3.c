/*
/*one thread waiting on some lock and other thread signals it on some other lock..*/

#include "syscall.h"
void thread1(void);
void thread2(void);


int lock1,lock2,CVForLock1;

int main()
{
	lock1 = CreateLock("lock1",5);
	CVForLock1 = CreateCV("CV1",3);
	lock2 = CreateLock("lock2",5);

	Fork(thread1);
	Fork(thread2);

}

void thread1()
{
	Acquire(lock1);
	Wait(CVForLock1,lock1);
	Release(lock1);
	Exit(0);
}

void thread2()
{
	Yield();
	Yield();

	Acquire(lock1);
	Signal(CVForLock1,lock2);
	Release(lock1);
	Exit(0);
}

