/*--------------------------------------------------------------------*/
/* point.c                                                            */
/* Author: Gilron Tsabkevich                                          */
/*--------------------------------------------------------------------*/

#include "point.h"
#include <assert.h>
#include <unistd.h>
/*--------------------------------------------------------------------*/

Point Point_fromText(const char* text)
{
	Point point;

	assert(text != NULL);

	point.col = (int)(text[0] - 'A');
	point.row = (int)(text[1] - '0');

	return point;
}