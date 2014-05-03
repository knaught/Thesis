// RmMutableCartesianGrid.h

#ifndef RM_MUTABLE_CARTESIAN_GRID_H
#define RM_MUTABLE_CARTESIAN_GRID_H

#include <fstream>
#include "RmMutableMatrix.h"
#include "RmUtility.h"
#include "RmExceptions.h"


/**
 * Builds upon the dynamically resizable two-dimensional matrix functionality of
 * RmMutableMatrix, wrapping around it the notion of a four-quadrant Cartesian coordinate
 * system, or grid.  The grid retains the optional dynamic nature of the matrix, but is now addressed 
 * using Cartesian coordinates, where the origin <code>[0][0]</code> lies in the center of the grid
 * rather than the upper-left corner of the matrix.  This origin may be tied to an external
 * "global" grid coordinate (by providing one during construction)
 * such that all grid accesses are then made using these global coordinates,
 * and they will automatically be translated to the "local" coordinate system.
 * <h3>Usage</h3>
 * The following provides a brief example of how a <code>RmMutableCartesianGrid</code> may be used
 * in a manner peculiar to the Cartesian grid framework; see RmMutableMatrix for an example
 * of its inherent read/write accessibility and dynamic resizability.
 * <pre>
 * 1    #include "RmMutableCartesianGrid.h"
 * 2
 * 3    RmMutableCartesianGrid<int> m1(4,4);
 * 4    ++m1[0][0];
 * 5    m1[-1][2] = 2;
 * 6    m1[2][-1] = 3;
 * 7
 * 8    RmMutableCartesianGrid<float> m2(5,5,RmUtility::Coord(-2,2));
 * 9    m2[0][0] = 1;
 * 10   m2[-2][2] = 2;
 * 11   m2[-4][4] = 3;
 * </pre>
 * Line 3 instantiates a 4 x 4 matrix that is initialized to all zeroes, the C++ default
 * constructor for that primitive data type.
 * The origin of the grid is not mapped to an external global coordinate (the default
 * configuration), thus its
 * origin of [0][0] maps to the center of the grid.  For this evenly-dimensioned grid,
 * that center is one row and column off-center toward the lower-left corner of the grid,
 * as illustrated via line 4 in grid (A) below.
 * Lines 5 and 6 show the bounds of this grid to be (-1,2) and (2,-1).
 * <pre>
 *                    3 0 0 0 0
 *      2 0 0 0       0 0 0 0 0
 *      0 0 0 0       0 0 2 0 0
 *      0 1 0 0       0 0 0 0 0
 *      0 0 0 3       0 0 0 0 1
 *        (A)            (B)
 * </pre>
 * The second grid, <code>m2</code>, has an origin mapped to the global coordinate (-2,2),
 * which means [0][0] is not in the grid center, but in the lower-right corner;
 * the center of the grid is [-2][2].  This is illustrated via lines 9 and 10 in grid (B).
 * Notice also that because this is not an evenly-dimensioned grid, the origin does fall
 * in its exact center.
 * The bounds of this grid are (-4,4) and (0,0), shown by lines 11 and 9, respectively.
 * <h3>Dependencies</h3>
 * RmMutableMatrix provides the underlying dynamic two-dimensional data structure.
 * The utility structures of RmUtility::Coord and RmUtility::BoundBox are used in
 * specifying grid points and boundaries, respectively.
 */

template<class T>
class RmMutableCartesianGrid : public RmMutableMatrix<T>
{
public:

	/** Constants for specifying whether grid expansion is enabled or constrained. */
	enum ExpansionMode { 
		/** The grid cannot be resized */
		Constrain, 
		/** The grid can be resized */
		Expand 
	};

	/** Provides mechanism for enabling double-subscripted access of the grid. */
	friend class Operator2D<T>; 


	/**
	 * Constructs a grid with given dimensions, its center point mapping to the given 
	 * external coordinate, and automatic expansion unconstrained.
	 * To change the expansion mode after construction, use setExpansionMode().
	 * @param w the number of columns in the grid
	 * @param h the number of rows in the grid
	 * @param globalOrigin the coordinate in the global frame of reference to which this
	 * grid's origin is linked
	 * @param initVal the value to which all cells in the grid are (re)initialized to
	 */
	RmMutableCartesianGrid( int w = 10, int h = 10, 
		const RmUtility::Coord& globalOrigin = RmUtility::Coord(0,0), const T initVal = T() )
		: RmMutableMatrix<T>( w, h, initVal ),
		  m_globalOrigin( globalOrigin ), m_expandMode( Expand ) { initBounds( w, h ); }


	/**
	 * Assignment operator for the RmMutableCartesianGrid, providing a deep copy of all members.
	 */
	RmMutableCartesianGrid& operator=( const RmMutableCartesianGrid& source );


	/**
	 * Specifies whether to Expand or Constrain the grid when it is indexed using out-of-bound
	 * coordinates.  If constrained, such coordinates will throw an 
	 * IndexOutOfBoundsException.
	 */
	void setExpansionMode( ExpansionMode mode ) { m_expandMode = mode; }


	/**
	 * Sets the global origin to that specified and updates the boundaries accordingly,
	 * leaving the grid center unchanged. See origin().
	 */
	inline void setOrigin( const RmUtility::Coord& origin );


	/**
	 * Expands and/or contracts the matrix by the given amounts without shifting its contents.
	 * See RmMutableMatrix::resizeBy() for details.
	 */
	void resizeBy( int north, int south, int east, int west );


	/**
	 * Rotates this grid about its center by the given theta.
	 * See RmMutableMatrix::rotateBy() for details.
	 * @param theta an angle of rotation within [0..360) degrees
	 */
	void rotateBy( const double theta );


	/**
	 * Trims off all outer rows and columns that contain no non-initializion values.
	 */
	void trim();


	/**
	 * Trims this grid to the given bounds.
	 * If a requested boundary extends beyond the grid's current boundary, no trimming occurs.
	 * For example, given a grid with bounds ( 5, -5, 5, -5 ), a call to trimTo() with
	 * the bound ( 3, -7, 3, -6 ) will trim the grid down to ( 3, -5, 3, -5 ).
	 * Because the requested south and west bounds were beyond the current bounds, no trimming
	 * occurred on those sides.
	 */
	void trimTo( const RmUtility::BoundBox bound );


	/**
	 * Copies the portion of this grid that intersects dest into dest.
	 */
	void copyInto( RmMutableCartesianGrid<T>& dest ) const;


	/**
	 * Copies the values in the given matrix into this matrix.
	 * Overlapping cells are replaced with the values in m.
	 */
	void mergeWith( const RmMutableCartesianGrid<T>& m );


	/**
	 * Provides access to the cell at the given x-y coordinate for read and write operations.  
	 * Indexes correspond to zero-based <code>[x][y]</code> coordinates, with <code>[0][0]</code> 
	 * accessing the <b>center-most cell</b> of the grid.  In the case of
	 * evenly-dimensioned grids, the center cell is the lower-leftmost cell in the center-most
	 * 2x2 quadrant.
	 * <p>
	 * Indexes that reference a cell outside the bounds of the matrix cause it to be
	 * automatically resized, provided that behavior is not disabled using setExpansionMode()
	 * (enabled is the default setting); otherwise, an IndexOutOfBoundsException is thrown.
	 *
	 * @param x the column of the matrix, where a negative value expands westward, and a
	 * positive value beyond its current bounds expands it eastward
	 * @param y the row of the matrix, where a negative value expands it northward, and a
	 * positive value beyond its current bounds expands it southward
	 * @return a reference to the object stored at the specified cell, which may be read from
	 * or written to
	 * @throws an RmExceptions::IndexOutOfBoundsException if the grid is constrained and the 
	 * x or y coordinates are beyond the bounds of the grid
	 */
	T& valueAt( int x, int y );


	/**
	 * The <code>const</code> version of valueAt(), which implies the grid cannot be resized.
	 * @throws an RmExceptions::IndexOutOfBoundsException if the 
	 * x or y coordinates are beyond the bounds of the grid
	 */
	const T& valueAt( int x, int y ) const;


	/**
	 * Returns true if the given coordinate falls within the bounds of this grid.
	 */
	bool inBounds( const RmUtility::Coord &c ) const { return inBounds( c.x, c.y ); }


	/**
	 * Returns true if the given coordinate falls within the bounds of this grid.
	 */
	inline bool inBounds( int x, int y ) const;


	/**
	 * Reinitializes the grid to its initial height, width and cell values.
	 */
	void empty() { RmMutableMatrix<T>::empty(); initBounds( width(), height() ); }


	/** 
	 * Provides upper-left and lower-right coordinates of the grid relative to the global origin
	 * given during construction.  Use this, not width() and height(), to manually
	 * iterate over the matrix.
	 */
	RmUtility::BoundBox bound() const { return m_globalBound; }


	/**
	 * Returns the global origin to which the local origin of this grid is anchored.
	 */
	RmUtility::Coord origin() const { return m_globalOrigin; }


	/**
	 * Streams a text representation of the grid in a top-down row-column format.
	 * @param os the output stream
	 * @param bound provides the option to precede the grid by its boundary coordinates
	 * @param format an optional C-style format string; see RmMutableMatrix<T>::put.
	 */
	std::ostream& put( std::ostream& os, bool bound = true, const char *format = NULL ) const;


	/**
	 * Streams a text representation using put() to a file of the given name.
	 * Numeric data is written with the specified precision.
	 */
	void put( const char* filename, int precision = 4 ) const;

protected:

	/**
	 * Initializes the bounds of the grid such that the global origin maps to the 
	 * physical center of the grid, as defined by the given dimensions.
	 */
	void initBounds( int width, int height );


	/**
	 * Initializes the bounds of the grid such that the global origin maps to the given
	 * center coordinate, rather than the physical center of the grid.
	 */
	void initBounds( RmUtility::Coord center );

private:

	/** The external "global" coordinate that maps to the center of this grid. */
	RmUtility::Coord m_globalOrigin;

	/** The local center of this grid, with [0][0] being the lower-leftmost cell. */
	RmUtility::Coord m_center;

	/** The upper-left and lower-right extremes of this grid in terms of the global frame of
		reference */
	RmUtility::BoundBox m_globalBound;

	/** The current expansion mode that allows or disallows dynamic expansion of the grid. */
	ExpansionMode m_expandMode;
};


template<class T>
RmMutableCartesianGrid<T>& RmMutableCartesianGrid<T>::operator=( const RmMutableCartesianGrid& source )
{
	RmMutableMatrix<T>::operator=( source );
	m_globalOrigin = source.m_globalOrigin;
	m_center = source.m_center;
	m_globalBound = source.m_globalBound;
	m_expandMode = source.m_expandMode;

	return *this;
}



template<class T>
void RmMutableCartesianGrid<T>::initBounds( int width, int height )
{
	int h = height - 1;
	int w = width - 1;
	m_center.x = w / 2;
	m_center.y = h / 2;
	int yt = h - m_center.y + m_globalOrigin.y;
	int xr = w - m_center.x + m_globalOrigin.x;
	int xl = xr - w;
	int yb = yt - h;
	m_globalBound = RmUtility::BoundBox( xl, yt, xr, yb );
}


template<class T>
void RmMutableCartesianGrid<T>::initBounds( RmUtility::Coord center )
{
	int h = height() - 1;
	int w = width() - 1;
	m_center = center;
	int yt = h - m_center.y + m_globalOrigin.y;
	int xr = w - m_center.x + m_globalOrigin.x;
	int xl = xr - w;
	int yb = yt - h;
	m_globalBound = RmUtility::BoundBox( xl, yt, xr, yb );
}


template<class T>
void RmMutableCartesianGrid<T>::setOrigin( const RmUtility::Coord& origin )
{
	m_globalOrigin = origin;
	initBounds( m_center );
}


template<class T>
T& RmMutableCartesianGrid<T>::valueAt( int x, int y )
{
	// Note: The m_center is anchored to m_gobalOrigin
	// Growing the grid up or to the left leaves that center unchanged
	// Growing the grid down or to the right by distance delta shifts the center up or down by the
	// same delta

	bool expanded = false;

	// Convert local/global coords to matrix indices
	int localX = m_center.x + x - m_globalOrigin.x;
	int localY = m_center.y + y - m_globalOrigin.y;

	// Expand grid as necessary and update bounds
	// ...left
	if ( localX < 0 ) {
		m_center.x += -localX;
		m_globalBound.ul.x += localX;
		expanded = true;
	}

	// ...right
	else if ( localX >= width() ) {
		m_globalBound.lr.x += localX - width() + 1;
		expanded = true;
	}

	// ...up
	if ( localY >= height() ) {
		m_globalBound.ul.y += localY - height() + 1;
		expanded = true;
	}

	// ...down
	else if ( localY < 0 ) {
		m_center.y += -localY;
		m_globalBound.lr.y += localY;
		expanded = true;
	}

	if ( expanded && m_expandMode == Constrain ) {
		char buff[50];
		sprintf( buff, "Index constraint: [%d..%d)[%d..%d); Requested index: [%d][%d]", 
			m_globalBound.ul.x, m_globalBound.lr.x, m_globalBound.ul.y, m_globalBound.lr.y, x, y );
		throw RmExceptions::IndexOutOfBoundsException( 
			"RmMutableCartesianGrid<T>::valueAt()", buff );
	}

	// Access grid using transposed coords
	// (from origin-lower-left to origin-upper-left)
	return RmMutableMatrix<T>::valueAt( localX, height() - localY - 1 );
}


template<class T>
const T& RmMutableCartesianGrid<T>::valueAt( int x, int y ) const
{
	// Note: The m_center is anchored to m_gobalOrigin
	// Growing the grid up or to the left leaves that center unchanged
	// Growing the grid down or to the right by distance delta shifts the center up or down by the 
	// same delta

	bool expanded = false;

	// Convert local/global coords to matrix indices
	int localX = m_center.x + x - m_globalOrigin.x;
	int localY = m_center.y + y - m_globalOrigin.y;

	// Expand grid as necessary and update bounds
	// ...left
	if ( localX < 0 || localX >= width() || localY >= height() || localY < 0 ) {
		char buff[50];
		sprintf( buff, "Index constraint: [0..%d)[0..%d); Requested index: [%d][%d]", 
			width(), height(), x, y );
		throw RmExceptions::IndexOutOfBoundsException( "RmMutableCartesianGrid<T>::valueAt() const", buff );
	}

	// Access grid using transposed coords
	// (from origin-lower-left to origin-upper-left)
	return RmMutableMatrix<T>::valueAt(localX, height() - localY - 1);
}


template<class T>
bool RmMutableCartesianGrid<T>::inBounds( int x, int y ) const
{
	return x >= m_globalBound.ul.x && x <= m_globalBound.lr.x &&
		y <= m_globalBound.ul.y && y >= m_globalBound.lr.y;
}


template<class T>
void RmMutableCartesianGrid<T>::rotateBy( const double theta )
{
	/////
	// Resize grid such that origin is in physical center

	// find furthest boundary from origin
	int temp;
	int horizMax = (temp = width()-1 - m_center.x) > m_center.x ? temp : m_center.x;
	int vertMax = (temp = height()-1 - m_center.y) > m_center.y ? temp : m_center.y;
		// Note: use the max of these two to square the grid about its center

	// punch out boundaries by that distance from the center
	valueAt( m_globalOrigin.x - horizMax, m_globalOrigin.y + vertMax );
	valueAt( m_globalOrigin.x + horizMax, m_globalOrigin.y - vertMax );


	//////
	// Rotate

	RmMutableMatrix<T>::rotateBy( theta );

	// accommodate resizing that resulted from rotation
	initBounds( width(), height() );
}


template<class T>
void RmMutableCartesianGrid<T>::trim()
{
	const T empty = initValue();
	RmUtility::BoundBox inner( m_globalBound.lr, m_globalBound.ul ); 
		// reversed intentionally to force update

	for ( int y = m_globalBound.ul.y; y > m_globalBound.lr.y; --y ) {
		for ( int x = m_globalBound.ul.x; x < m_globalBound.lr.x; ++x ) {
			if ( valueAt( x, y ) != empty ) {
				if ( x < inner.ul.x ) inner.ul.x = x;
				else if ( x > inner.lr.x ) inner.lr.x = x;
				if ( y > inner.ul.y ) inner.ul.y = y;
				else if ( y < inner.lr.y ) inner.lr.y = y;
			}
		}
	}

	resizeBy( inner.ul.y - m_globalBound.ul.y, m_globalBound.lr.y - inner.lr.y, 
		inner.lr.x - m_globalBound.lr.x, m_globalBound.ul.x - inner.ul.x );
}


template<class T>
void RmMutableCartesianGrid<T>::trimTo( const RmUtility::BoundBox bound )
{
	int n = bound.ul.y - m_globalBound.ul.y;
	if ( n > 0 ) n = 0;
	int s = m_globalBound.lr.y - bound.lr.y;
	if ( s > 0 ) s = 0;
	int e = bound.lr.x - m_globalBound.lr.x;
	if ( e > 0 ) e = 0;
	int w = m_globalBound.ul.x - bound.ul.x;
	if ( w > 0 ) w = 0;

	resizeBy( n, s, e, w );
}


template<class T>
void RmMutableCartesianGrid<T>::copyInto( RmMutableCartesianGrid<T>& dest ) const
{
	// Get requested bounds as fit within this grid
	RmUtility::BoundBox bound( dest.bound() );
	bound.intersectWith( m_globalBound );

	for ( int y = bound.ul.y; y >= bound.lr.y; --y ) {
		for ( int x = bound.ul.x; x <= bound.lr.x; ++x ) {
			const T v = valueAt( x, y ); // make copy
			dest.valueAt( x, y ) = v;
		}
	}
}


template<class T>
void RmMutableCartesianGrid<T>::mergeWith( const RmMutableCartesianGrid<T>& m )
{
	const T empty = initValue();
	const RmUtility::BoundBox bound( m.bound() );
	for ( int y = bound.ul.y; y >= bound.lr.y; --y ) {
		for ( int x = bound.ul.x; x <= bound.lr.x; ++x ) {
			const T v = m.valueAt( x, y );
			if ( v != empty ) valueAt( x, y ) = v;
		}
	}
}


template<class T>
void RmMutableCartesianGrid<T>::resizeBy( int north, int south, int east, int west )
{
	RmMutableMatrix<T>::resizeBy( north, south, east, west );

	initBounds( RmUtility::Coord( m_center.x + west, m_center.y + south ) );
}


template<class T>
std::ostream& RmMutableCartesianGrid<T>::put( 
	std::ostream& os, bool bound, const char *format ) const
{
	if ( bound ) os << m_globalBound << "\n";
	return RmMutableMatrix<T>::put( os, format ); 
}


template<class T>
void RmMutableCartesianGrid<T>::put( const char* filename, int precision ) const
{
	std::ofstream ofs( filename );
	ofs.setf( std::ios_base::fixed, std::ios_base::floatfield );
	ofs.precision( precision );
	put( ofs, true );
	ofs.close();
}

#endif