/*--------------------------------------------------------------------*/
/* point.h                                                            */
/* Author: Gilron Tsabkevich                                          */
/*--------------------------------------------------------------------*/

#ifndef POINT_INCLUDED
#define POINT_INCLUDED

/* A point on the board consists of row and col */
typedef struct Point
{
	/* Point's row representation on the board */
	int row;

	/* Point's row representation on the board  */
	int col;
} Point;

/*--------------------------------------------------------------------*/

/* Get a string text, convert it to Point on board. Return point. */
Point Point_fromText(const char* text);

#endif