

#include "syscall.h"

#define Dim 	20	

int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];

int X[Dim][Dim];
int Y[Dim][Dim];
int Z[Dim][Dim];

void matmult1()
{
    int i, j, k;

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     A[i][j] = i;
	     B[i][j] = j;
	     C[i][j] = 0;
	}

    for (i = 0; i < Dim; i++)		/* then multiply them together */
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 C[i][j] += A[i][k] * B[k][j];

	Printf1((unsigned int)"\nMatmult1 Result=>",C[Dim-1][Dim-1]);
	Exit(0);		/* and then we're done */
}


void matmult2()
{
    int a, b, c;

    for (a = 0; a < Dim; a++)		/* first initialize the matrices */
	for (b = 0; b < Dim; b++) {
	     X[a][b] = a;
	     Y[a][b] = b;
	     Z[a][b] = 0;
	}

    for (a = 0; a < Dim; a++)		/* then multiply them together */
	for (b = 0; b < Dim; b++)
            for (c = 0; c < Dim; c++)
		 Z[a][b] += X[a][c] * Y[c][b];

	Printf1((unsigned int)"\nMatmult2 Result=>",Z[Dim-1][Dim-1]);
	Exit(0);		/* and then we're done */
}

int main()
{
	Fork(matmult1);
	Fork(matmult2);

	Exit(0);
}
