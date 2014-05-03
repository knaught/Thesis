
#include "polygon.h"

// draw.c defines:
extern struct PointList* pointListTop;
extern void InitDraw();
extern void DrawPixel(int,int,int);

/**
 * An implementation of the Bresenham Line Algorithm.
 * This is the "optimized" version of drawline2d() from
 * <a href="http://en.wikipedia.org/wiki/Bresenham's_line_algorithm">Wikipedia.org</a>.
 * In this context, this method is used to "fill" the axis along the acoustic axis
 * of the sonar, as defined by the given coordinates, with their probabilities of being
 * occupied.
 *
 * NOTE! FillLine(start, end) does not necessarily return the same result as FillLine(end, start).
 */
struct PointList* FillLine(int x0, int y0, int x1, int y1)
{
    int i;
    int sx, sy;  /* step positive or negative (1 or -1) */
    int dx, dy;  /* delta (difference in X and Y between points) */
    int dx2, dy2;
    int e;
    int temp;

	InitDraw();

    dx = x1 - x0;
    sx = (dx > 0) ? 1 : -1;
    if (dx < 0)
        dx = -dx;

    dy = y1 - y0;
    sy = (dy > 0) ? 1 : -1;
    if (dy < 0)
        dy = -dy;

    dx2 = dx << 1; /* dx2 = 2 * dx */
    dy2 = dy << 1; /* dy2 = 2 * dy */

    if (dy <= dx) { /* steep */
        e = dy2 - dx;

        for (i = 0; i < dx; ++i) {
			DrawPixel( x0, y0, 0 );
			
            while (e >= 0) {
                y0 += sy;
                e -= dx2;
            }

            x0 += sx;
            e += dy2;
        }
    } 
    else {
        // swap x0 <-> y0
        temp = x0;
        x0 = y0;
        y0 = temp;

        // swap dx <-> dy
        temp = dx;
        dx = dy;
        dy = temp;

        // swap dx2 <-> dy2
        temp = dx2;
        dx2 = dy2;
        dy2 = temp;

        // swap sx <-> sy
        temp = sx;
        sx = sy;
        sy = temp;

        e = dy2 - dx;

        for (i = 0; i < dx; ++i) {
			DrawPixel( y0, x0, 0 );

            while (e >= 0) {
                y0 += sy;
                e -= dx2;
            }

            x0 += sx;
            e += dy2;
        }
    }

   return(pointListTop);
}


int LineLength( struct PointList* linePointList )
{
	int i = 0;
	struct PointList* line = linePointList;
	while ( line ) {
	   line = line->next;
		++i;
	}
	return i;
}
