
#include "syscall.h"

void main()
{
	int input,i,j;
	int newProcess1,newProcess2,newProcess3,newProcess4,newProcess5,newProcess6,newProcess7,newProcess8,newProcess9,newProcess10;
	int newProcess11,newProcess12,newProcess13,newProcess14,newProcess15;

	Printf((unsigned int)"1. Test to check client sends msg Properly\n");
	Printf((unsigned int)"2. Test to check Wait & Signal calls\n");
	Printf((unsigned int)"3. Doctor Office Interaction\n");
	Printf((unsigned int)"Enter the test case to be run:");
	input= Scanf();
	Printf((unsigned int)"\n");

	switch (input)
	{
		case 1:
			newProcess1 = Exec((unsigned int)"../test/project4Test1",21);
			newProcess2 = Exec((unsigned int)"../test/project4Test1",21);
			break;
		case 2:
			newProcess1 = Exec((unsigned int)"../test/project4Test2",21);
			Yield();
			newProcess2 = Exec((unsigned int)"../test/project4Test3",21);
			break;
		case 3:
			Printf((unsigned int)"Note:Please run Wrn First\n");	
			Printf((unsigned int)"1.Wrn\n");
			Printf((unsigned int)"2.Patients(20)\n");
			Printf((unsigned int)"Enter entity to be run on this machine\n");
			input = Scanf();
			if(input == 1)
			{
				newProcess1 = Exec((unsigned int)"../test/wrn",11);
			}
			else if(input == 2)
			{
				newProcess1 = Exec((unsigned int)"../test/patient",15);
				newProcess2 = Exec((unsigned int)"../test/patient",15);
				newProcess3 = Exec((unsigned int)"../test/patient",15);
				newProcess4 = Exec((unsigned int)"../test/patient",15);
				newProcess5 = Exec((unsigned int)"../test/patient",15);
				
				newProcess6 = Exec((unsigned int)"../test/patient",15);
				newProcess7 = Exec((unsigned int)"../test/patient",15);
				newProcess8 = Exec((unsigned int)"../test/patient",15);
				newProcess9 = Exec((unsigned int)"../test/patient",15);
				newProcess10 = Exec((unsigned int)"../test/patient",15);

				newProcess11 = Exec((unsigned int)"../test/patient",15);
				newProcess12 = Exec((unsigned int)"../test/patient",15);
				newProcess13 = Exec((unsigned int)"../test/patient",15);
				newProcess14 = Exec((unsigned int)"../test/patient",15);
				newProcess15 = Exec((unsigned int)"../test/patient",15);
			}
			else
			{
				Printf((unsigned int)"No such case\n");
			}
	}
	for(j=0;j<100000;j++)
	{
		Yield();
	}
}
