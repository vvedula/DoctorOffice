
#include "syscall.h"

void main()
{
	int input,i,j;
	int newProcess1,newProcess2,newProcess3,newProcess4,newProcess5,newProcess6;
	
	Printf((unsigned int)"\n 1.BASIC TEST CASES FOR LOCK:- Create, Destroy, Acquire Release,Create, Destroy CV, Wait, Signal, Broadcast.\n");
	Printf((unsigned int)"\n 2.FORK test case to not fork more than 100 threads\n");
	Printf((unsigned int)"\n 3.Signal DONE By A Thread But no one waiting.. \n");
	Printf((unsigned int)"\n 4.Lock & CV limit exceeded.. \n");
	Printf((unsigned int)"\n 5.Signal on a CV more than once \n");
	Printf((unsigned int)"\n 6.Exec on two processes and one process acquiring the lock created by the other process\n");
	Printf((unsigned int)"\n 7.Broadcast A Wait CV with a different Lock And Broadcast with a different CV.\n");
	Printf((unsigned int)"\n 8.Execute two doctors office Simulations as two different processes \n");
	Printf((unsigned int)"\n 9. Matmult \n");
	Printf((unsigned int)"\n10. Matmult as two separate processes \n");
	Printf((unsigned int)"\n11.Matmult with 2 forks\n");
	Printf((unsigned int)"\n12. Sort \n");
	Printf((unsigned int)"\n13.Sort as two separate processes \n");
	Printf((unsigned int)"\n14.Sort with 2 forks\n");
	Printf((unsigned int)"\n15.Matmult & Sort simultaneous execution\n");
	Printf((unsigned int)"\n16.Test Case for Wait & Signal\n");
	Printf((unsigned int)"\n17.Test Case for Broadcast\n");
	Printf((unsigned int)"\n18.Test Case for Monitor Variables\n");
	Printf((unsigned int)"\nEnter the test case to be run:");
	input= Scanf();

	switch (input)
	{
		case 1:
			newProcess1 =  Exec((unsigned int)"../test/test1",13);
			break;
		case 2:
			newProcess1 =  Exec((unsigned int)"../test/test2",13);
	   		break;
		case 3:
			newProcess1 =  Exec((unsigned int)"../test/test3",13);
			break;
		case 4: 
			newProcess1 =  Exec((unsigned int)"../test/test4",14);
			break;	   
		case 5: 
			newProcess1 =  Exec((unsigned int)"../test/test5",13);
			break;	 
		case 6:
			newProcess1 = Exec((unsigned int)"../test/test6i",14);
			newProcess2 = Exec((unsigned int)"../test/test6ii",15);
			break;		  
		case 7: 
			newProcess1=  Exec((unsigned int)"../test/test7",13);
			break;	  
		case 8:
			newProcess1= Exec((unsigned int)"../test/doctoroffice",20);
			newProcess2= Exec((unsigned int)"../test/doctoroffice",20);
			break;
		case 9:	
			newProcess1 = Exec((unsigned int)"../test/matmult",15);
			break;		
		case 10:	
			newProcess1 = Exec((unsigned int)"../test/matmult",15);		
			newProcess2 = Exec((unsigned int)"../test/matmult",15);
			break;
		case 11:	
			newProcess1 = Exec((unsigned int)"../test/forkmatmult",19);		
			break;
		case 12:
			newProcess1 = Exec((unsigned int)"../test/sort",12);
			break;
		case 13:
			newProcess1 = Exec((unsigned int)"../test/sort",12);
			newProcess2 = Exec((unsigned int)"../test/sort",12);
			break;		
		case 14:	
			newProcess1 = Exec((unsigned int)"../test/forksort",16);		
			break;			
		case 15:	
			newProcess1 = Exec((unsigned int)"../test/matmult",15);		
			newProcess2 = Exec((unsigned int)"../test/sort",12);					
			break;
		case 16:
			/*newProcess1 = Exec((unsigned int)"../test/test8",13);			
			newProcess2 = Exec((unsigned int)"../test/test9",13);*/
			newProcess3 = Exec((unsigned int)"../test/test10",14);
			newProcess4 = Exec((unsigned int)"../test/test10",14);
			newProcess5 = Exec((unsigned int)"../test/test10",14);
			Yield();
			newProcess6 = Exec((unsigned int)"../test/test11",14);
/*			newProcess5 = Exec((unsigned int)"../test/test12",14);
			newProcess6 = Exec((unsigned int)"../test/test12",14);*/
			break;
		case 17:
			newProcess1 = Exec((unsigned int)"../test/test10",14);			
			newProcess2 = Exec((unsigned int)"../test/test10",14);
			newProcess3 = Exec((unsigned int)"../test/test11",14);
			break;
		case 18:
			newProcess1 = Exec((unsigned int)"../test/test12",14);			
			newProcess2 = Exec((unsigned int)"../test/test13",14);
			break;
		case 19:
			newProcess1 = Exec((unsigned int)"../test/testClient",18);			
	}
	for(j=0;j<100000;j++)
	{
		Yield();
	}
}
