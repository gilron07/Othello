/*--------------------------------------------------------------------*/
/* referee.c                                                          */
/* Author: Gilron Tsabkevich                                          */
/*--------------------------------------------------------------------*/

#define _POSIX_SOURCE 1

#include "point.h"
#include "board.h"
#include "game.h"
#include <sys/resource.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


/*--------------------------------------------------------------------*/

enum {FALSE, TRUE};

#define MAX_SCORE 64 
#define ONE_MINUTE 60
#define EXEC_ADDITION 3
#define VS_ADDITION 4

/*--------------------------------------------------------------------*/

/* Validates that the executable exec is valid and accesible.
   Return 1 if it does. 0 othwerwise */

static int checkExeExistnace(char *exec)
{
	if (access(exec, F_OK) == -1) 
	{
		fprintf(stderr, "File %s does not exist or is not executable\n", exec);
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

	assert(iIsTracking != NULL);

	if (strcmp(argv[1], "-tracking") == 0)
	{
		iArgExesIndex = 2;
		isTracking = TRUE;
	}
	
	*exec1 = (char *) malloc(strlen(argv[iArgExesIndex]) + EXEC_ADDITION);
	*exec2 = (char *) malloc(strlen(argv[iArgExesIndex + 1]) + EXEC_ADDITION);
	*fileName = (char *)malloc(strlen(argv[iArgExesIndex]) + 
		                       strlen(argv[iArgExesIndex + 1]) + VS_ADDITION);

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

/* Function gets the gameScore and prints the game ending to appropriate
   file handler, psFileHandle. Functions gets the name of the players 
   location from argv and iIsTracking.*/

static void endGame(int gameScore, FILE *psFileHandle, char *argv[], 
			 int iIsTracking)
{
	char *name1;
	char *name2; 
	int iArgExesIndex = 1;

	assert(psFileHandle != NULL);

	if (iIsTracking)
		iArgExesIndex++;

	/* gets players' names */
	name1 = argv[iArgExesIndex];
	name2 = argv[iArgExesIndex + 1];

	fprintf(psFileHandle,"FIRST (%s) vs SECOND (%s)\n", name1, name2);

	if (gameScore > 0)
		fprintf(psFileHandle,"   Winner FIRST %s\n", name1);

	else if (gameScore < 0)
		fprintf(psFileHandle,"   Winner SECOND %s\n", name2);

	else fprintf(psFileHandle,"   Winner (draw)\n");

	/* print score*/
	fprintf(psFileHandle,"   Score %d\n", gameScore);
}

/*--------------------------------------------------------------------*/

/* Returns maximum score according to player who made the 
   bad move. */

static int determineScoreAbrupt(BoardColor player)
{
	if (player == BLACK)
		return -MAX_SCORE;
	return MAX_SCORE;
}

/*--------------------------------------------------------------------*/

/* Function gets the player who made an invalid move or has crashed 
   which is indicated by iIsCrashed. Prints the appropriate game state
   to psFileHandle while using iIsTracking and the command-line-arguments
   argv information. */

static void abruptGame(BoardColor player, FILE *psFileHandle, 
	char *argv[], int iIsTracking, int iIsCrashed)
{
	int score; 

	assert(psFileHandle != NULL);

	/* determine what player gets max score */
	score = determineScoreAbrupt(player);
	endGame(score, psFileHandle, argv, iIsTracking);

	if (iIsCrashed)
		fprintf(psFileHandle,"   Player crahsed\n");

	else fprintf(psFileHandle,"   Bad move\n");
}

/*--------------------------------------------------------------------*/

/* terminate process iPid. In case there is an error argv[0] would 
   be used. */

static void terminateProcess(pid_t iPid, char *argv[])
{
	if (kill(iPid, SIGKILL) == -1)
	{
		perror(argv[0]);
		exit(EXIT_FAILURE); 
	}
}

/*--------------------------------------------------------------------*/

/* Function runs two players concurrently and runs an othello game 
   between the two, as it reads move from player1 to player2 and vice verca.
   argc is the command-line argument count, and argv contains
   the command-line arguements. Return 0. */

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
	char *fileName;
	/* read move buffer */
	char move[10];
	int iSuccessful;
	BoardColor currentPlayer = BLACK;
	int iIsCrashed = FALSE;

	FILE *psFilePlayer1ToRef, *psFileRefToPlayer1, *psFilePlayer2ToRef,
		 *psFileRefToPlayer2, *psFileTrackingMode, *psFileReadFrom, 
		 *psFileWriteTo;

	int iIsTracking;
	
	if (!parseCommandLineArguments(argv, &iIsTracking, &exec1, &exec2, &fileName))
		return 0;

	/* creates pipes */
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
   		sRlimit.rlim_cur = ONE_MINUTE;
   		sRlimit.rlim_max = ONE_MINUTE;
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

		/* execute player1 as FIRST player */
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
   		sRlimit.rlim_cur = ONE_MINUTE;
   		sRlimit.rlim_max = ONE_MINUTE;
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

		/* execute player2 as SECOND player */
		apcArgv[0] = exec2;
		apcArgv[1] = "SECOND";
		apcArgv[2] = NULL;
		execvp(exec2, apcArgv);
		perror(argv[0]);
		exit(EXIT_FAILURE);

	}

	/* Parent code */

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

	/* run game */
	Game_init();

	if (iIsTracking)
	{
		/* open the file in order to record the game state */
		psFileTrackingMode = fopen(fileName ,"w");
		Game_printGameState(psFileTrackingMode);

	}

	/* set current file inputs and outputs */
	psFileReadFrom = psFilePlayer1ToRef;
	psFileWriteTo = psFileRefToPlayer2;

	/* game loop, breaks whenever the game is concluded in any way */
	while (TRUE)
	{
		/* get current player */
		currentPlayer = Game_currentPlayer();

		/* get move from player */
		if (fscanf(psFileReadFrom, "%s", move) == EOF)
		{
			iIsCrashed = TRUE;

			if (iIsTracking)
				abruptGame(currentPlayer, psFileTrackingMode, argv,
				 iIsTracking, iIsCrashed);

			/* terminate both players */
			terminateProcess(iPidFirst, argv);
			terminateProcess(iPidSecond, argv);

			printf("%d\n", determineScoreAbrupt(currentPlayer));
			break;
		}


		if (iIsTracking)
			Game_printMove(move, psFileTrackingMode);

		iSuccessful = Game_makeMove(move);

		/* if move was not successful */
		if(!iSuccessful)
		{
			if (iIsTracking)
				abruptGame(currentPlayer, psFileTrackingMode, argv, 
					iIsTracking, iIsCrashed);

			/* terminate both players */
			terminateProcess(iPidFirst, argv);
			terminateProcess(iPidSecond, argv);

			printf("%d\n", determineScoreAbrupt(currentPlayer));
			break;
		}

		if (iIsTracking)
			Game_printGameState(psFileTrackingMode);

		iSuccessful = Game_isAnyValidMove();

		/* if no valid moves for the current player */
		if (!iSuccessful)
		{
			Game_skipMove();
			iSuccessful = Game_isAnyValidMove();

			/* if no valid moves for both players */
			if (!iSuccessful)
			{
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

		/* write move to other player */
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

	/* close all files */
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