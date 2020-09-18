/*--------------------------------------------------------------------*/
/* game.c                                                             */
/* Author: Gilron Tsabkevich                                          */
/*--------------------------------------------------------------------*/

#include "board.h"
#include "point.h"
#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/*--------------------------------------------------------------------*/

/* represents Black or White */
enum {FALSE, TRUE};

/*--------------------------------------------------------------------*/

/* Indicates the currentplayer*/
static BoardColor currentPlayer;

/* Indicates if game was initialized */
static int iInitialized = FALSE;

/* The move number in the game */
static int moveNumber;

/* String FIRST constant*/
static const char *FIRST = "FIRST";

/* String SECOND constant*/
static const char *SECOND = "SECOND";

/*--------------------------------------------------------------------*/

/* function gets the color and returns the opposite */

static BoardColor Game_reverseColor(BoardColor color)
{
	if (color == BLACK)
		return WHITE;

	return BLACK;
}

/*--------------------------------------------------------------------*/

/* Gets point p and returns the number of pieces to flip towards
   the upper direction. If none returns 0. */

static int Game_isValidUp(Point p)
{
	int count = 0;
	Point pTemp;
	BoardColor otherColor;

	pTemp.row = p.row - 1;
	pTemp.col = p.col;
	otherColor = Game_reverseColor(currentPlayer);

	while (pTemp.row > 0 && Board_get(pTemp) == (int)otherColor)
	{
		count++;
		pTemp.row--;
	}
	if (count > 0 && Board_get(pTemp) == (int)currentPlayer)
		return count;

	return 0;
}

/*--------------------------------------------------------------------*/

/* Gets point p and returns the number of pieces to flip towards
   the right direction. If none returns 0. */

static int Game_isValidRight(Point p)
{
	int count = 0;
	Point pTemp;
	BoardColor otherColor;

	pTemp.row = p.row;
	pTemp.col = p.col + 1;
	otherColor = Game_reverseColor(currentPlayer);

	while (pTemp.col < BOARD_SIZE - 1 && Board_get(pTemp) == (int)otherColor)
	{
		count++;
		pTemp.col++;
	}

	if (count > 0 && Board_get(pTemp) == (int)currentPlayer)
		return count;
	
	return 0;
}

/*--------------------------------------------------------------------*/

/* Gets point p and returns the number of pieces to flip towards
   the lower direction. If none returns 0. */

static int Game_isValidDown(Point p)
{
	int count = 0;
	Point pTemp;
	BoardColor otherColor;

	pTemp.row = p.row + 1;
	pTemp.col = p.col;
	otherColor = Game_reverseColor(currentPlayer);

	while (pTemp.row < BOARD_SIZE - 1 && Board_get(pTemp) == (int)otherColor)
	{
		count++;
		pTemp.row++;
	}
	if (count > 0 && Board_get(pTemp) == (int)currentPlayer)
		return count;

	return 0;
}

/*--------------------------------------------------------------------*/

/* Gets point p and returns the number of pieces to flip towards
   the left direction. If none returns 0. */

static int Game_isValidLeft(Point p)
{
	int count = 0;
	Point pTemp;
	BoardColor otherColor;

	pTemp.row = p.row;
	pTemp.col = p.col - 1;
	otherColor = Game_reverseColor(currentPlayer);

	while (pTemp.col > 0 && Board_get(pTemp) == (int)otherColor)
	{
		count++;
		pTemp.col--;
	}
	
	if (count > 0 && Board_get(pTemp) == (int)currentPlayer)
		return count;
	
	return 0;
}

/*--------------------------------------------------------------------*/

/* Gets point p and returns the number of pieces to flip towards
   the upper right diagonal direction. If none returns 0. */

static int Game_isValidUpRight(Point p)
{
	int count = 0;
	Point pTemp;
	BoardColor otherColor;

	pTemp.row = p.row - 1;
	pTemp.col = p.col + 1;
	otherColor = Game_reverseColor(currentPlayer);

	while (pTemp.row > 0 && pTemp.col < BOARD_SIZE - 1 
		   && Board_get(pTemp) == (int)otherColor)
	{
		count++;
		pTemp.row--;
		pTemp.col++;
	}

	if (count > 0 && Board_get(pTemp) == (int)currentPlayer)
		return count;

	return 0;
}

/*--------------------------------------------------------------------*/

/* Gets point p and returns the number of pieces to flip towards
   the lower right diagonal direction. If none returns 0. */

static int Game_isValidDownRight(Point p)
{
	int count = 0;
	Point pTemp;
	BoardColor otherColor;

	pTemp.row = p.row + 1;
	pTemp.col = p.col + 1;
	otherColor = Game_reverseColor(currentPlayer);

	while (pTemp.row < BOARD_SIZE - 1  && pTemp.col < BOARD_SIZE - 1 
		   && Board_get(pTemp) == (int)otherColor)
	{
		count++;
		pTemp.row++;
		pTemp.col++;
	}

	if (count > 0 && Board_get(pTemp) == (int)currentPlayer)
		return count;

	return 0;
}

/*--------------------------------------------------------------------*/

/* Gets point p and returns the number of pieces to flip towards
   the lower left diagonal direction. If none returns 0. */

static int Game_isValidDownLeft(Point p)
{
	int count = 0;
	Point pTemp;
	BoardColor otherColor;

	pTemp.row = p.row + 1;
	pTemp.col = p.col - 1;
	otherColor = Game_reverseColor(currentPlayer);

	while (pTemp.row < BOARD_SIZE - 1  && pTemp.col > 0 
		   && Board_get(pTemp) == (int)otherColor)
	{
		count++;
		pTemp.row++;
		pTemp.col--;
	}

	if (count > 0 && Board_get(pTemp) == (int)currentPlayer)
		return count;

	return 0;
}

/*--------------------------------------------------------------------*/

/* Gets point p and returns the number of pieces to flip towards
   the upper left diagonal direction. If none returns 0. */

static int Game_isValidUpLeft(Point p)
{
	int count = 0;
	Point pTemp;
	BoardColor otherColor;

	pTemp.row = p.row - 1;
	pTemp.col = p.col - 1;
	otherColor = Game_reverseColor(currentPlayer);


	while (pTemp.row > 0  && pTemp.col > 0 
		   && Board_get(pTemp) == (int)otherColor)
	{
		count++;
		pTemp.row--;
		pTemp.col--;
	}

	if (count > 0 && Board_get(pTemp) == (int)currentPlayer)
		return count;

	return 0;
}

/*--------------------------------------------------------------------*/

/* Gets a poing p and checks if the move is valid. Returns 1 if it is
   0 otherwise. */

static int Game_isValidMove(Point p)
{
	if (!Game_isValidUp(p) && !Game_isValidRight(p) &&
		!Game_isValidLeft(p) && !Game_isValidDown(p) &&
		!Game_isValidUpRight(p) && !Game_isValidDownRight(p) &&
		!Game_isValidDownLeft(p) && !Game_isValidUpLeft(p))
		return 0;

	return 1;
}

/*--------------------------------------------------------------------*/

int Game_isAnyValidMove(void)
{
	int count = 0;
	Point pTemp;

	for (pTemp.row = 0; pTemp.row < BOARD_SIZE; pTemp.row++)
	{
		for (pTemp.col = 0; pTemp.col < BOARD_SIZE; pTemp.col++)
		{
			if(Board_get(pTemp) == EMPTY)
			{
				count += Game_isValidMove(pTemp);
			}
		}
	}
	if (count > 0)
		return 1;

	return 0;

}

/*--------------------------------------------------------------------*/

void Game_printGameState(FILE *psFileHandle)
{
	assert(psFileHandle != NULL);

	/* Holds either the FIRST or SECOND */
	if(moveNumber == 0)
		fprintf(psFileHandle, "\nInitial game state:\n");

	else		
		fprintf(psFileHandle, "\nCurrent game state:\n");


	fprintf(psFileHandle, "FIRST = x, SECOND = o\n\n");
	Board_print(psFileHandle);


}

/*--------------------------------------------------------------------*/

void Game_printMove(const char* move, FILE *psFileHandle)
{
	const char *ptrPlayer;

	assert(move != NULL);
	assert(psFileHandle != NULL);

	if(currentPlayer == BLACK)
		ptrPlayer = FIRST;

	else ptrPlayer = SECOND;

	fprintf(psFileHandle, "\nMove #%d (by %s player):  %s\n",
		moveNumber,ptrPlayer, move);
}

/*--------------------------------------------------------------------*/

BoardColor Game_currentPlayer(void)
{
	return currentPlayer;
}

/*--------------------------------------------------------------------*/

int Game_makeMove(const char* text)
{
	int count;
	int i;
	Point pTemp;
	Point pBoard;

	assert(text != NULL);

	pBoard = Point_fromText(text);
	
	if (!Board_isInBorders(pBoard) || !Game_isValidMove(pBoard))
		return 0;

	/* Flip upper pieces if neccessary */
	count = Game_isValidUp(pBoard);
	pTemp.row = pBoard.row - 1;
	pTemp.col = pBoard.col;
	for (i = 0; i < count; i++, pTemp.row--)
		Board_put(pTemp, currentPlayer);

	/* Flip pieces to the right if neccessary */
	count = Game_isValidRight(pBoard);
	pTemp.row = pBoard.row;
	pTemp.col = pBoard.col + 1;
	for (i = 0; i < count; i++, pTemp.col++)
		Board_put(pTemp, currentPlayer);

	/* Flip lower pieces if neccessary */
	count = Game_isValidDown(pBoard);
	pTemp.row = pBoard.row + 1;
	pTemp.col = pBoard.col;
	for (i = 0; i < count; i++, pTemp.row++)
		Board_put(pTemp, currentPlayer);

	/* Flip pieces to the left if neccessary */
	count = Game_isValidLeft(pBoard);
	pTemp.row = pBoard.row;
	pTemp.col = pBoard.col - 1;
	for (i = 0; i < count; i++, pTemp.col--)
		Board_put(pTemp, currentPlayer);


	/* Flip pieces to the up right diagonal direction if neccessary */
	count = Game_isValidUpRight(pBoard);
	pTemp.row = pBoard.row - 1;
	pTemp.col = pBoard.col + 1;
	for (i = 0; i < count; i++, pTemp.col++, pTemp.row--)
		Board_put(pTemp, currentPlayer);

	/* Flip pieces to the down right diagonal direction if neccessary */
	count = Game_isValidDownRight(pBoard);
	pTemp.row = pBoard.row + 1;
	pTemp.col = pBoard.col + 1;
	for (i = 0; i < count; i++, pTemp.col++, pTemp.row++)
		Board_put(pTemp, currentPlayer);

	/* Flip pieces to the down left diagonal direction if neccessary */
	count = Game_isValidDownLeft(pBoard);
	pTemp.row = pBoard.row + 1;
	pTemp.col = pBoard.col - 1;
	for (i = 0; i < count; i++, pTemp.col--, pTemp.row++)
		Board_put(pTemp, currentPlayer);

	/* Flip pieces to the up left diagonal direction if neccessary */
	count = Game_isValidUpLeft(pBoard);
	pTemp.row = pBoard.row - 1;
	pTemp.col = pBoard.col - 1;
	for (i = 0; i < count; i++, pTemp.col--, pTemp.row--)
		Board_put(pTemp, currentPlayer);


	Board_put(pBoard, currentPlayer);

	currentPlayer = Game_reverseColor(currentPlayer);
	moveNumber++;

	return 1;

}

/*--------------------------------------------------------------------*/

int Game_score(void)
{
	Point pTemp;
	int count = 0;

	for(pTemp.row = 0; pTemp.row < BOARD_SIZE; pTemp.row++)
		for(pTemp.col = 0; pTemp.col < BOARD_SIZE; pTemp.col++)
			{
				if(Board_get(pTemp) == BLACK)
					count++;
				if (Board_get(pTemp) == WHITE)
					count--;
			}

	return count;
}

/*--------------------------------------------------------------------*/

void Game_skipMove(void)
{
	currentPlayer = Game_reverseColor(currentPlayer);
}

/*--------------------------------------------------------------------*/

void Game_init(void)
{
	assert(!iInitialized);

	Board_init();
	currentPlayer = BLACK;
	moveNumber = 0;

	iInitialized = TRUE;
}



