
void main()
{
	int lock1 = CreateLock("lock1",5);
	int cv1 = CreateCV("cv1",3);
	
	Acquire(lock1);
	Wait(cv1,lock1);
	Printf((unsigned int)"Back from Wait\n");
	Release(lock1);
	Exit(0);
}
