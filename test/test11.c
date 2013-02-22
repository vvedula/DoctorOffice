
void main()
{
	int lock1 = CreateLock("lock1",5);
	int cv1 = CreateCV("cv1",3);
	
	Acquire(lock1);
	Broadcast(cv1,lock1);
	Release(lock1);
	
	Exit(0);
}
