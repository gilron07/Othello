/*--------------------------------------------------------------------*/
/* referee.c                                                          */
/* Author: Gilron Tsabkevich                                          */
/*--------------------------------------------------------------------*/


#include "board.h"
#include "point.h"
#include "game.h"

#define _POSIX_SOURCE 1
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <sys/resource.h>


/*--------------------------------------------------------------------*/

enum {FALSE, TRUE};

#define MAX_SCORE 64 

/*--------------------------------------------------------------------*/

/* Validates that the executable exed1 is valid and accesible.
   Return 1 if it does. 0 othwerwise */

static int checkExeExistnace(char *exec)
{
	if (access(exec, F_OK) == -1) 
	{
		printf("File %s does not exist or is not executable\n", exec);
		return 0;
	}

	return 1;
}

/*--------------------------------------------------------------------*/

/* Function gets line arguments ,argv, and creates execution names 
   to exec1 and exec2 .Creates the file name to store the game's data, 
   saves the name to fileName. Sets iIsTracking to True if program was 
   executed with -tracking mode, False otherwise. 
   Returns 0 if executables exec1 or exec2 do not exist or not executable 
   Returns 1 otherwise. */

static int parseCommandLineArguments(char *argv[], int* iIsTracking, 
							  char** exec1, char** exec2, char** fileName)
{
    int iArgExesIndex = 1;
	int isTracking = FALSE;

	if (strcmp(argv[1], "-tracking") == 0)
	{
		iArgExesIndex = 2;
		isTracking = TRUE;
	}
	
	*exec1 = (char *) malloc(strlen(argv[iArgExesIndex]) + 3);
	*exec2 = (char *) malloc(strlen(argv[iArgExesIndex + 1]) + 3);
	*fileName = (char *)malloc(strlen(argv[iArgExesIndex]) + 
		                       strlen(argv[iArgExesIndex + 1]) + 4);

	strcpy(*exec1, "./");
	strcpy(*exec2, "./");
	strcat(*exec1, argv[iArgExesIndex]);
	strcat(*exec2, argv[iArgExesIndex + 1]);
	
	*iIsTracking = isTracking;

	strcpy(*fileName, argv[iArgExesIndex]);
	strcat(*fileName,"_vs_");
	strcat(*fileName, argv[iArgExesIndex + 1]);

	if (!checkExeExistnace(argv[iArgExesIndex]) || 
		!checkExeExistnace(argv[iArgExesIndex + 1]))
		return 0;

	return 1;
}

/*--------------------------------------------------------------------*/


/* Function gets the gamescore and prints the game ending */
static void endGame(int gameScore, FILE *psFileHandle, char *argv[], 
			 int iIsTracking)
{
	char *name1;
	char *name2; 
	int iArgExesIndex = 1;

	if (iIsTracking)
		iArgExesIndex++;

	name1 = argv[iArgExesIndex];
	name2 = argv[iArgExesIndex + 1];

	fprintf(psFileHandle,"FIRST (%s) vs SECOND (%s)\n", name1, name2);

	if (gameScore > 0)
		fprintf(psFileHandle,"   Winner FIRST %s\n", name1);

	else if (gameScore < 0)
		fprintf(psFileHandle,"   Winner SECOND %s\n", name2);

	else fprintf(psFileHandle,"   Winner (draw)\n");

	fprintf(psFileHandle,"   Score %d\n", gameScore);
}

/*--------------------------------------------------------------------*/

/* Returns maximum score accoring to player who made the 
   bad move. */
static int determineScoreAbrupt(BoardColor player)
{
	if (player == BLACK)
		return -MAX_SCORE;
	return MAX_SCORE;
}

/*--------------------------------------------------------------------*/

/* player indicates the current color of the player and 
   prints the appropriate early end game. Happens when player 
   does an invalid move. */ 

static void abruptGame(BoardColor player, FILE *psFileHandle, 
	char *argv[], int iIsTracking, int iIsCrashed)
{
	int score; 

	score = determineScoreAbrupt(player);
	endGame(score, psFileHandle, argv, iIsTracking);

	if (iIsCrashed)
		fprintf(psFileHandle,"   Player crahsed\n");
	else fprintf(psFileHandle,"   Bad move\n");
}

/*--------------------------------------------------------------------*/

/* Functions gets process ID , playerId, and terminates it*/
static void killPlayer(pid_t iPlayerId, char *argv[])
{
	if (kill(iPlayerId, SIGKILL) == -1)
	{
		perror(argv[0]);
		exit(EXIT_FAILURE); 
	}
}

/*--------------------------------------------------------------------*/

static void runGame(int iIsTracking, FILE *psFilePlayer1ToRef, 
					FILE *psFileRefToPlayer1, FILE *psFilePlayer2ToRef,
					FILE *psFileRefToPlayer2, pid_t iPidFirst, 
					pid_t iPidSecond, char *argv[], char *fileName)
{
	int iIsCrashed = FALSE;
	int iSuccessful;
	FILE *psFileTrackingMode, *psFileReadFrom, *psFileWriteTo;
	BoardColor currentPlayer = BLACK;
	char move[50];

	Game_init();
	if (iIsTracking)
	{
		psFileTrackingMode = fopen(fileName ,"w");
		Game_printGameState(psFileTrackingMode);

	}

	psFileReadFrom = psFilePlayer1ToRef;
	psFileWriteTo = psFileRefToPlayer2;

	while (TRUE)
	{
		currentPlayer = Game_currentPlayer();

		if (fscanf(psFileReadFrom, "%s", move) == EOF)
		{
			iIsCrashed = TRUE;

			if (iIsTracking)
				abruptGame(currentPlayer, psFileTrackingMode, argv,
				 iIsTracking, iIsCrashed);

			/* terminate players' processes */ 
			killPlayer(iPidFirst, argv);
			killPlayer(iPidSecond, argv);

			printf("%d\n", determineScoreAbrupt(currentPlayer));
			break;
		}


		if (iIsTracking)
			Game_printMove(move, psFileTrackingMode);

		iSuccessful = Game_makeMove(move);

		if(!iSuccessful)
		{
			if (iIsTracking)
				abruptGame(currentPlayer, psFileTrackingMode, argv, 
					iIsTracking, iIsCrashed);

			killPlayer(iPidFirst, argv);
			killPlayer(iPidSecond, argv);

			printf("%d\n", determineScoreAbrupt(currentPlayer));
			break;
		}

		if (iIsTracking)
			Game_printGameState(psFileTrackingMode);

		iSuccessful = Game_isAnyValidMove();

		if (!iSuccessful)
		{
			Game_skipMove();
			iSuccessful = Game_isAnyValidMove();
			if (!iSuccessful)
			{
				/* both player don't have valid moves*/
				if (iIsTracking)
					endGame(Game_score(), psFileTrackingMode, argv, iIsTracking);

				fprintf(psFileWriteTo, "%s ", move);
				fflush(NULL);
				printf("%d\n", Game_score());
				break;
			}
			/* Skip the second's player turn */
			fprintf(psFileWriteTo, "%s ", move);
			fflush(NULL);
			continue;
		}

		fprintf(psFileWriteTo, "%s ", move);
		fflush(NULL);

		/* switch the input and the output */
		if (currentPlayer == BLACK)
		{
			psFileReadFrom = psFilePlayer2ToRef;
			psFileWriteTo = psFileRefToPlayer1;	
		}

		else 
		{
			psFileReadFrom = psFilePlayer1ToRef;
			psFileWriteTo = psFileRefToPlayer2;	
		}
		
	}
}
/*--------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	pid_t iPidFirst;
	pid_t iPidSecond;
	int iRet;

	/* arrays for pipes */
	int Player1ToRef[2];
	int RefToPlayer1[2];
	int Player2ToRef[2];
	int RefToPlayer2[2];


	/* strings for executables */
	char *exec1;
	char *exec2;

	/* game varibles */
	char move[50];
	char *fileName;
	int iSuccessful;
	BoardColor currentPlayer = BLACK;
	int iIsCrashed = FALSE;

	FILE *psFilePlayer1ToRef, *psFileRefToPlayer1, *psFilePlayer2ToRef,
		 *psFileRefToPlayer2, *psFileTrackingMode, *psFileReadFrom, 
		 *psFileWriteTo;

	int iIsTracking;
	
	if (!parseCommandLineArguments(argv, &iIsTracking, &exec1, &exec2, &fileName))
		return 0;

	if (pipe(Player1ToRef)) {perror(argv[0]); exit(EXIT_FAILURE);}
	if (pipe(RefToPlayer1)) {perror(argv[0]); exit(EXIT_FAILURE);}
	if (pipe(Player2ToRef)) {perror(argv[0]); exit(EXIT_FAILURE);}
	if (pipe(RefToPlayer2)) {perror(argv[0]); exit(EXIT_FAILURE);}

	iRet = fflush(NULL);
	if (iRet == EOF) {perror(argv[0]); exit(EXIT_FAILURE);}

	iPidFirst = fork();
	if (iPidFirst == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

	/* first child to become exec1 */
	if (iPidFirst == 0)
	{	
		char *apcArgv[3];
		struct rlimit sRlimit;

   		/* Set the process's CPU limit. */
   		sRlimit.rlim_cur = 60;
   		sRlimit.rlim_max = 60;
   		setrlimit(RLIMIT_CPU, &sRlimit);

		/* Redirect stdout and stdin to appropriate pipes */
		iRet = close(0);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = dup(RefToPlayer1[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(1);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = dup(Player1ToRef[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		/* close all unused file descriptors */

		iRet = close(RefToPlayer1[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(RefToPlayer1[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(Player1ToRef[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(Player1ToRef[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(RefToPlayer2[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(RefToPlayer2[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(Player2ToRef[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
		iRet = close(Player2ToRef[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		apcArgv[0] = exec1;
		apcArgv[1] = "FIRST";
		apcArgv[2] = NULL;
		execvp(exec1, apcArgv);
		perror(argv[0]);
		exit(EXIT_FAILURE);

	}

	iRet = fflush(NULL);
	if (iRet == EOF) {perror(argv[0]); exit(EXIT_FAILURE);}

	iPidSecond = fork();
	if (iPidSecond == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

	/* second child to become exec2 */
	if (iPidSecond == 0)
	{	
		char *apcArgv[3];
		struct rlimit sRlimit;

   		/* Set the process's CPU limit. */
   		sRlimit.rlim_cur = 60;
   		sRlimit.rlim_max = 60;
   		setrlimit(RLIMIT_CPU, &sRlimit);

		/* Redirect stdout and stdin to appropriate pipes */
		iRet = close(0);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = dup(RefToPlayer2[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(1);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = dup(Player2ToRef[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		/* close all unused file descriptors */

		iRet = close(RefToPlayer1[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(RefToPlayer1[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(Player1ToRef[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(Player1ToRef[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(RefToPlayer2[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(RefToPlayer2[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(Player2ToRef[0]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		iRet = close(Player2ToRef[1]);
		if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

		apcArgv[0] = exec2;
		apcArgv[1] = "SECOND";
		apcArgv[2] = NULL;
		execvp(exec2, apcArgv);
		perror(argv[0]);
		exit(EXIT_FAILURE);

	}

	/* Parent code: chat */

	iRet = close(Player1ToRef[1]);
	if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

	iRet = close(RefToPlayer1[0]);
	if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

	iRet = close(Player2ToRef[1]);
	if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

	iRet = close(RefToPlayer2[0]);
	if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

	/* Wrap pipes with FILES */

	psFilePlayer1ToRef = fdopen(Player1ToRef[0], "r");
	psFileRefToPlayer1 = fdopen(RefToPlayer1[1], "w");
	psFilePlayer2ToRef = fdopen(Player2ToRef[0], "r");
	psFileRefToPlayer2 = fdopen(RefToPlayer2[1], "w");



	runGame(iIsTracking, psFilePlayer1ToRef, psFileRefToPlayer1,
		psFilePlayer2ToRef, psFileRefToPlayer2, iPidFirst, iPidSecond,
		argv, fileName);
	


	fclose(psFileRefToPlayer1);
	fclose(psFilePlayer1ToRef);
	fclose(psFileRefToPlayer2);
	fclose(psFilePlayer2ToRef);

	if (iIsTracking)
		fclose(psFileTrackingMode);

	/* free allocated memory */
	free(exec1);
	free(exec2);
	free(fileName);

	return 0;
}