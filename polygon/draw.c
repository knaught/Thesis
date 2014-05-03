/* draw.c

   Source:
   Dr. Dobbs Journal, Apr 1991, "grap_pr.asc" [LISTING TWO]
   _GRAPHICS PROGRAMMING COLUMN_
   by Michael Abrash
*/

/* Draws all pixels in the horizontal line segment passed in, from
   (LeftX,Y) to (RightX,Y), in the specified color in mode 13h, the
   VGA's 320x200 256-color mode. Both LeftX and RightX are drawn. No
   drawing will take place if LeftX > RightX. */

#include <stdio.h>
#include <stdlib.h>
#include "polygon.h"

//#define SCREEN_WIDTH    320
//#define SCREEN_SEGMENT  0xA000

struct PointList* pointList = 0;
struct PointList* pointListTop = 0;

void DrawPixel(int, int, int);

void InitDraw() 
{
	if ( pointListTop == 0 ) return;
	pointList = pointListTop;
	while ( pointList )
	{
		struct PointList* nextPointList = pointList->next;
		free( pointList );
		pointList = nextPointList;
	}
	pointList = pointListTop = 0;
}

void DrawHorizontalLineSeg(int Y, int LeftX, int RightX, int Color) {
   int X;

   /* Draw each pixel in the horizontal line segment, starting with
      the leftmost one */
   for (X = LeftX; X <= RightX; X++)
      DrawPixel(X, Y, Color);
}

/* Draws the pixel at (X, Y) in color Color in VGA mode 13h */
void DrawPixel(int X, int Y, int Color) 
{
/*	static int lastY = -1;
	if ( lastY != Y ) printf( "\n" );
	lastY = Y;
	printf( "(%d,%d) ", X, Y );
*/
	if ( pointListTop == 0 )
	{
		if ( (pointList = pointListTop = (struct PointList*)(malloc( sizeof( struct PointList ) ))) == NULL )
			exit(0);
		pointList->next = 0;
		pointList->point.X = X;
		pointList->point.Y = Y;
	}
	else
	{
		struct PointList* nextPointList;
		if ( (nextPointList = (struct PointList*)(malloc( sizeof( struct PointList ) ))) == NULL )
			exit(0);
		nextPointList->next = 0;
		nextPointList->point.X = X;
		nextPointList->point.Y = Y;
		pointList = pointList->next = nextPointList;
	}
}



/* Source:
   Dr. Dobbs Journal, Feb 1991, "graphics.asc" [LISTING TWO]
   _GRAPHICS PROGRAMMING COLUMN_
   by Michael Abrash
*/

void DrawHorizontalLineList(struct HLineList * HLineListPtr,
      int Color)
{
   struct HLine *HLinePtr;
   int Y, X;

   /* Point to the XStart/XEnd descriptor for the first (top)
      horizontal line */
   HLinePtr = HLineListPtr->HLinePtr;
   /* Draw each horizontal line in turn, starting with the top one and
      advancing one line each time */
   for (Y = HLineListPtr->YStart; Y < (HLineListPtr->YStart +
         HLineListPtr->Length); Y++, HLinePtr++) {
      /* Draw each pixel in the current horizontal line in turn,
         starting with the leftmost one */
      for (X = HLinePtr->XStart; X <= HLinePtr->XEnd; X++)
         DrawPixel(X, Y, Color);
   }
}
