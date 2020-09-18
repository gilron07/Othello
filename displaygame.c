/*--------------------------------------------------------------------*/
/* displaygame.c                                                      */
/* Author: Gilron Tsabkevich                                          */
/*--------------------------------------------------------------------*/

#include "board.h"
#include "point.h"
#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/*--------------------------------------------------------------------*/

#define MAX_SCORE 64

/*--------------------------------------------------------------------*/
/* Function gets the gameScore and prints the game ending to 
   psFileHandle. */

static void endGame(int gameScore, FILE *psFileHandle)
{
	assert(psFileHandle != NULL);

	fprintf(psFileHandle,"FIRST vs SECOND\n");

	if (gameScore > 0)
		fprintf(psFileHandle,"   Winner FIRST\n");

	else if (gameScore < 0)
		fprintf(psFileHandle,"   Winner SECOND\n");

	else fprintf(psFileHandle,"   Winner (draw)\n");

	fprintf(psFileHandle,"   Score %d\n", gameScore);
}

/*--------------------------------------------------------------------*/

/* player indicates the current color of the player. Function 
   prints the appropriate early end game to psFileHandle. 
   Happens when player plays an invalid move. */ 

static void abruptGame(BoardColor player, FILE *psFileHandle)
{
	assert(psFileHandle != NULL);

	if (player == BLACK)
		endGame(-MAX_SCORE, psFileHandle);
	else endGame(MAX_SCORE, psFileHandle);

	fprintf(psFileHandle,"   Bad move\n");
}

/*--------------------------------------------------------------------*/

/* Function reads moves fron stdin and feeds the game  with the moves.
   Return 0. argc is the command-line argument count, and argv contains
   the command-line arguements. */

int main(int argc, char const *argv[])
{
	/* Buffer */
	char move[10];
	int iSuccessful;
	BoardColor currentPlayer = BLACK;

	Game_init();
	Game_printGameState(stdout);

	/* scan moves from input */
	while(fscanf(stdin, "%s", move) != EOF)
	{
		currentPlayer = Game_currentPlayer();
		Game_printMove(move, stdout);

		iSuccessful = Game_makeMove(move);
		if(!iSuccessful) 
		{
			abruptGame(currentPlayer, stdout);
			break;
		}
		
		/* prints the game state */
		Game_printGameState(stdout);

		/* check if any valid moves exists, if not
	 	   pass the turn to the other player */
		iSuccessful = Game_isAnyValidMove();
		if (!iSuccessful)
		{
			Game_skipMove();
			iSuccessful = Game_isAnyValidMove();
			if (!iSuccessful)
			{
				/* both player don't have valid moves */
				endGame(Game_score(), stdout);
				break;
			}

		}

	}

	return 0;
}