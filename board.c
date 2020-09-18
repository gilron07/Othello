/*--------------------------------------------------------------------*/
/* board.c                                                            */
/* Author: Gilron Tsabkevich                                          */
/*--------------------------------------------------------------------*/

#include "board.h"
#include "point.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/*--------------------------------------------------------------------*/

/* In lieu of boolean type. */
enum {FALSE, TRUE};

/*--------------------------------------------------------------------*/

/* holds the board's current state*/
static int board[BOARD_SIZE][BOARD_SIZE];

/* was board initialized */
static int iInitialized = FALSE;

/*--------------------------------------------------------------------*/

int Board_isInBorders(Point pBoard)
{
	if (pBoard.row < 0 || pBoard.row > BOARD_SIZE - 1 ||
		pBoard.col < 0 || pBoard.col > BOARD_SIZE - 1)
		return FALSE;

	return TRUE;
}

/*--------------------------------------------------------------------*/

void Board_init(void)
{
	int row;
	int col;

	assert(!iInitialized);

	for (row = 0; row < BOARD_SIZE; row++)
		for (col = 0; col < BOARD_SIZE; col++)
			board[row][col] = EMPTY;

	/* Initialize the initial game state */
	board[BOARD_SIZE / 2][BOARD_SIZE / 2] = WHITE;
	board[BOARD_SIZE / 2][BOARD_SIZE / 2 - 1] = BLACK;
	board[BOARD_SIZE / 2 - 1][BOARD_SIZE / 2] = BLACK;
	board[BOARD_SIZE / 2 - 1][BOARD_SIZE / 2 - 1] = WHITE;

	iInitialized = TRUE;
}

/*--------------------------------------------------------------------*/

void Board_print(FILE *psFileHandle)
{
	int row;
	int col;

	assert(psFileHandle != NULL);

	fprintf(psFileHandle, "   A B C D E F G H\n");

	for (row = 0; row < BOARD_SIZE; row++)
	{
		fprintf(psFileHandle, "%d ", row);

		for (col = 0; col < BOARD_SIZE; col++)
		{
			if (board[row][col] == EMPTY)
				fprintf(psFileHandle, " .");

			else if (board[row][col] == WHITE)
				fprintf(psFileHandle, " o");

			else fprintf(psFileHandle, " x");

		}
		fprintf(psFileHandle, "\n");
	}
}

/*--------------------------------------------------------------------*/

void Board_put(Point pBoard, BoardColor color)
{
	assert(Board_isInBorders(pBoard));

	board[pBoard.row][pBoard.col] = color;
}

/*--------------------------------------------------------------------*/

int Board_get(Point pBoard)
{
	assert(Board_isInBorders(pBoard));

	return board[pBoard.row][pBoard.col];
}





