/*--------------------------------------------------------------------*/
/* game.h                                                             */
/* Author: Gilron Tsabkevich                                          */
/*--------------------------------------------------------------------*/

#ifndef GAME_INCLUDED
#define GAME_INCLUDED

#include "point.h"

/* Initailize new Game */
void Game_init(void);
/*--------------------------------------------------------------------*/

/* Get move text and make the appropriate move on board.
   If move is valid function returns 1. 0 otherwise */
int Game_makeMove(const char* text);

/*--------------------------------------------------------------------*/

/* Returns the score of the game */
int Game_score(void);

/*--------------------------------------------------------------------*/

/* Returns the currentPlayer */
BoardColor Game_currentPlayer(void);

/*--------------------------------------------------------------------*/

/* Skip plater turn */
void Game_skipMove(void);

/*--------------------------------------------------------------------*/

/* Returns 1 if there is at least one posssible move for
   current player. 0 otherwise. */
int Game_isAnyValidMove(void);

/*--------------------------------------------------------------------*/

/* Prints the current game state to psFileHandle. */
void Game_printGameState(FILE *psFileHandle);

/*--------------------------------------------------------------------*/

/* Gets current player's move and prints the line that indicates 
   the move and by whom it was played to psFileHandle. */
void Game_printMove(const char* move, FILE *psFileHandle);


#endif