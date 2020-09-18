/*--------------------------------------------------------------------*/
/* add2.c                                                             */
/* Author: Gilron Tsabkevich                                          */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/stat.h>

/*--------------------------------------------------------------------*/

/* Program reads number from stdin, adds 2, and prints to std out. 
   Repeats the process while there is an input. Return 0 */

int main(int argc, char *argv[])
{
	int c;
	struct stat fInfo;
	int i;

	for (i = 3; i <= 40; i++)
		if (fstat(i, &fInfo) >= 0){
			fprintf(stderr, "File descriptor %d is not closed", i);
		}

	while (scanf("%d", &c) != EOF)
	{
		c += 2;
		printf("%d ", c);
		fflush(NULL);
	}
	
	return 0;
}

