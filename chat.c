/*--------------------------------------------------------------------*/
/* char.c                                                             */
/* Author: Gilron Tsabkevich                                          */
/*--------------------------------------------------------------------*/

#define _POSIX_SOURCE 1
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/*--------------------------------------------------------------------*/

#define EXEC_ADDITION 3

/*--------------------------------------------------------------------*/

/* Program pipes between add1 and add2. Sends an intial 0 to add1 and
   pipes the output to add2. Return 0. */

int main(int argc, char *argv[])
{
	pid_t iPid;
	int iRet;
	int i;
	/* arrays for pipes */
	int Add1ToChat[2];
	int ChatToAdd1[2];
	int Add2ToChat[2];
	int ChatToAdd2[2];

	/* obtained values from pipes */
	int resAdd1;
	int resAdd2;

	/* strings for executables */
	char *exec1;
	char *exec2;

	FILE *psFileAdd1ToChat, *psFileChatToAdd1, *psFileAdd2ToChat,
		 *psFileChatToAdd2;

	/* adding ./ to executable name */
	exec1 = (char *) malloc(strlen(argv[1]) + EXEC_ADDITION);
	exec2 = (char *) malloc(strlen(argv[2]) + EXEC_ADDITION);
	strcpy(exec1, "./");
	strcpy(exec2, "./");
	strcat(exec1, argv[1]);
	strcat(exec2, argv[2]);

	if (pipe(Add1ToChat)) {perror(argv[0]); exit(EXIT_FAILURE);}
	if (pipe(ChatToAdd1)) {perror(argv[0]); exit(EXIT_FAILURE);}
	if (pipe(Add2ToChat)) {perror(argv[0]); exit(EXIT_FAILURE);}
	if (pipe(ChatToAdd2)) {perror(argv[0]); exit(EXIT_FAILURE);}

	iRet = fflush(NULL);
	if (iRet == EOF) {perror(argv[0]); exit(EXIT_FAILURE);}

	iPid = fork();
	if (iPid == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

	/* first child to become exec1 */
	if (iPid == 0)
	{	
		char *apcArgv[2];

		/* Redirect stdout and stdin to appropriate pipes */
		iRet = close(0);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = dup(ChatToAdd1[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(1);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = dup(Add1ToChat[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		/* close all unused file descriptors */

		iRet = close(ChatToAdd1[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(ChatToAdd1[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(Add1ToChat[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(Add1ToChat[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(ChatToAdd2[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(ChatToAdd2[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(Add2ToChat[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(Add2ToChat[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		apcArgv[0] = exec1;
		apcArgv[1] = NULL;
		execvp(exec1, apcArgv);
		perror(argv[0]);
		exit(EXIT_FAILURE);

	}

	iRet = fflush(NULL);
	if (iRet == EOF) {perror(argv[0]); exit(EXIT_FAILURE);}

	iPid = fork();
	if (iPid == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

	/* second child to become exec2 */
	if (iPid == 0)
	{	
		char *apcArgv[2];

		/* Redirect stdout and stdin to appropriate pipes */
		iRet = close(0);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = dup(ChatToAdd2[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(1);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = dup(Add2ToChat[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		/* close all unused file descriptors */

		iRet = close(ChatToAdd1[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(ChatToAdd1[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(Add1ToChat[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(Add1ToChat[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(ChatToAdd2[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(ChatToAdd2[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(Add2ToChat[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(Add2ToChat[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		apcArgv[0] = exec2;
		apcArgv[1] = NULL;
		execvp(exec2, apcArgv);
		perror(argv[0]);
		exit(EXIT_FAILURE);

	}

	/* Parent code: chat */

	iRet = close(Add1ToChat[1]);
	if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

	iRet = close(ChatToAdd1[0]);
	if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

	iRet = close(Add2ToChat[1]);
	if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

	iRet = close(ChatToAdd2[0]);
	if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

	/* Wrap pipes with FILES */

	psFileAdd1ToChat = fdopen(Add1ToChat[0], "r");
	psFileChatToAdd1 = fdopen(ChatToAdd1[1], "w");
	psFileAdd2ToChat = fdopen(Add2ToChat[0], "r");
	psFileChatToAdd2 = fdopen(ChatToAdd2[1], "w");

	fprintf(psFileChatToAdd1, "%d ",0);
	fflush(NULL);

	for(i = 0; i < 10; i++)
	{
		fscanf(psFileAdd1ToChat, "%d", &resAdd1);
		printf("%d\n", resAdd1);
		fprintf(psFileChatToAdd2, "%d ", resAdd1);
		fflush(NULL);
		fscanf(psFileAdd2ToChat, "%d", &resAdd2);
		printf("%d\n", resAdd2);
		fprintf(psFileChatToAdd1, "%d ", resAdd2);
		fflush(NULL);

	}

	fclose(psFileChatToAdd1);
	fclose(psFileAdd1ToChat);
	fclose(psFileChatToAdd2);
	fclose(psFileAdd2ToChat);

	/* free allocated memory */
	free(exec1);
	free(exec2);

	return 0;
}