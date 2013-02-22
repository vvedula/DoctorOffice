

void main()
{
	int lock1 = CreateLock("Lock",4);
	Acquire(lock1);
	Printf((unsigned int)"Critical Section\n");
	Release(lock1);
	
	Exit(0);
}
