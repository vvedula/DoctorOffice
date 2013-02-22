/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

int A[1024];

int X[1024];

void sort1()
{
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++)		
        A[i] = 1024 - i;

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
	   if (A[j] > A[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = A[j];
	      A[j] = A[j + 1];
	      A[j + 1] = tmp;
    	   }
    	   Printf1((unsigned int)"\nSort1 Result=>",A[0]);
	   Exit(0);		/* and then we're done -- should be 0! */
}

void sort2()
{
    int a, b, c;

    for (a = 0; a < 1024; a++)		
        X[a] = 1024 - a;

    for (a = 0; a < 1023; a++)
        for (b = a; b < (1023 - a); b++)
	   if (X[b] > X[b + 1]) {
	      c = X[b];
	      X[b] = X[b + 1];
	      X[b + 1] = c;
    	   }
    	   Printf1((unsigned int)"\nSort2 Result=>",X[0]);
	   Exit(0);		
}

void main()
{
	Fork(sort1);
	Fork(sort2);
	
	Exit(0);
}
