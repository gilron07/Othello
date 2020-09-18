/*--------------------------------------------------------------------*/
/* board.h                                                            */
/* Author: Gilron Tsabkevich                                          */
/*--------------------------------------------------------------------*/

#ifndef BOARD_INCLUDED
#define BOARD_INCLUDED

#include "point.h"
#include <stdio.h>

#define BOARD_SIZE 8

/*--------------------------------------------------------------------*/

/* Type BoardColor that represents board's cell state */
typedef enum BoardColor {EMPTY, WHITE, BLACK} BoardColor;

/*--------------------------------------------------------------------*/

/* Initialize the board */
void Board_init(void);

/*--------------------------------------------------------------------*/
/* Prints borad to psFileHandle File */
void Board_print(FILE *psFileHandle);

/*--------------------------------------------------------------------*/

/* Put appropriate piece on board at pBoard with the corresponding 
   BoardColor color*/
void Board_put(Point pBoard, BoardColor color);

/*--------------------------------------------------------------------*/

/* Returns the content of the plae pBoard on the board */
int Board_get(Point pBoard);

/*--------------------------------------------------------------------*/

/* Returns 1 if the point pBoard is within in board's borders.
   Returns 0 otherwise. */
int Board_isInBorders(Point pBoard);

#endif
