// RmMutableMatrix.h

#ifndef RM_MUTABLE_MATRIX_H
#define RM_MUTABLE_MATRIX_H

#include <vector>
#include <iostream>
#include <cmath>
#include <cassert>
#include "RmExceptions.h"
#include "RmUtility.h"


/**
 * Overcomes the static nature of built-in C++ two-dimenionsional arrays by
 * providing an <code>stl::vector&lt;T&gt;</code>-based two-dimensional matrix that can be
 * dynamically declared and resized at run-time.
 * Resizing is accomplished explicitly using resizeBy() (which allows for the
 * expansion or truncation of rows or columns on one or more sides of the matrix),
 * or automatically using <code>[][]</code> (which allows for expansion only), 
 * as in <code>myMatrix[3][17]</code>.
 * <h3>Usage</h3>
 * The following provides a brief example of how a <code>RmMutableMatrix</code> may be used.
 * <pre>
 * 1    #include "RmMutableMatrix.h"
 * 2
 * 3    RmMutableMatrix<int> m(4,4);
 * 4    ++m[0][0];
 * 5    m[3][3] = 2;
 * 6    m[-1][-1] = m[1][1] + m[4][4];
 * 7    m[3][3] = 4;
 * 8    if ( m[5][0] == 0 ) m[2][2] = 5;
 * </pre>
 * Line 3 instantiates a 4 x 4 matrix that is initialized to all zeroes, the C++ default
 * constructor for that primitive data type.  The upper-left-most cell is indexed by [0][0],
 * the lower-right-most by [3][3].  Lines 4 and 5 assign values to these two cells,
 * and the resulting matrix is the first shown in (A) below.
 * <pre>
 *                    3 0 0 0 0       3 0 0 0 0 0
 *      1 0 0 0       0 1 0 0 0       0 1 0 0 0 0
 *      0 0 0 0       0 0 0 0 0       0 0 5 0 0 0
 *      0 0 0 0       0 0 0 4 0       0 0 0 4 0 0
 *      0 0 0 2       0 0 0 0 2       0 0 0 0 2 0
 *        (A)            (B)              (C)
 * </pre>
 * Line 6 demonstrates a noteworthy peculiarity.  First, the l-value <code>m[-1][-1]</code>
 * is processed <i>before</i> the r-value, which causes the matrix to be expanded up and to the left
 * by one row and column.  This causes the values that were previously in cells [0][0] and
 * [3][3] to shift to [1][1] and [4][4], which are then being referenced in the
 * r-value. The value of 3 is then placed in the new [0][0], as is shown in the
 * matrix (B).  Line seven illustrates this shift as well.
 * <p>
 * Finally, line 8 and matrix (C) demonstrate that read operations can also cause the matrix to 
 * expand.  
 */

template<class T>
class RmMutableMatrix
{
public:

	/** 
	 * Provides mechanism for achieving a two-dimensional <code>operator[]</code> shorthand,
	 * such as in <code>my2dArray[x][y]</code>.
	 */
	// This is public so that a using class may itself provide this [][] functionality without
	// implementing it directly; see RmBayesCertaintyGrid for an example.
	template<class T> 
	class Operator2D
	{
	public:

		/** 
		 * Stores the reference to the matrix for which the first dimension <code>[x]</code>
		 * is being specified, as well as that <code>x</code>-value, so that an adjacent
		 * <code>[y]</code>, and thus subsequent call to operator[](), will be able to pass 
		 * the request on to RmMutableMatrix::valueAt().
		 */
		Operator2D( RmMutableMatrix<T>* v, int x ) : m_m2v(v), m_m2vc(NULL), m_x(x) {}


		/**
		 * The constant version of Operator2D().
		 */
		Operator2D( const RmMutableMatrix<T>* v, int x ) : m_m2v(NULL), m_m2vc(v), m_x(x) {}


		/**
		 * Makes a call to RmMutableMatrix::valueAt() using the previously retained 
		 * <code>x</code>-value and the newly provided <code>y</code>-value.
		 * @param y the row of the matrix, where a negative value expands it northward, and a
		 * positive value beyond its current bounds expands it southward
		 */
		T& operator[]( int y ) { 
			return m_m2v->valueAt( m_x, y ); }


		/**
		 * The <code>const</code> version of operator[](), which implies the matrix cannot be resized.
		 */
		const T& operator[]( int y ) const { 
			return m_m2vc->valueAt( m_x, y ); }

	private:

		RmMutableMatrix<T>* m_m2v;
		const RmMutableMatrix<T>* m_m2vc;
		const int m_x;
	};


	/** 
	 * Creates a two-dimensional array, initialized with copies of the given initialization value.
	 * @param w the number of columns in the matrix
	 * @param h the number of rows in the matrix
	 * @param initVal the value to which all cells in the matrix are (re)initialized to
	 * @param colSep the string that separates each cell value on a row
	 * @param rowSep the string that separates each row of cells
	 * @param autoResize whether the matrix will automatically grow to accommodate
	 * out-of-bound indices
	 */
	RmMutableMatrix( int w = 10, int h = 10, const T initVal = T(), const char* colSep = " ",
		const char* rowSep = "\n", bool autoResize = true );


	/**
	 * Assignment operator for the RmMutableMatrix, providing a deep copy of all members.
	 */
	RmMutableMatrix& operator= ( const RmMutableMatrix& source );


	/**
	 * Sets the value that will be used to initialize new cells in future calls to
	 * resizeBy(), clear(), or empty().
	 */
	void setInitValue( const T initVal ) { m_initVal = initVal; }


	/**
	 * Returns a copy of the value that is used to initialize newly created or cleared cells.
	 */
	T initValue( ) const { return m_initVal; }
	

	/**
	 * Expands and/or contracts the matrix by the given amounts without shifting its contents.
	 * Positive values expand; negative values contract.
	 * @param north the amount by which the northern boundary of the matrix is expanded (positive) 
	 * or contracted (negative)
	 * @param south the amount by which the southern boundary of the matrix is expanded (positive) 
	 * or contracted (negative)
	 * @param east the amount by which the eastern boundary of the matrix is expanded (positive) 
	 * or contracted (negative)
	 * @param west the amount by which the western boundary of the matrix is expanded (positive) 
	 * or contracted (negative)
	 * @throws an RmExceptions::InvalidDimensionException if the resulting requested height or width 
	 * of the matrix is less than zero (e.g., given a 10 x 10 matrix, a call to 
	 * <code>resizeBy(-7, -4, 4, -2)</code> would result in a matrix with height of -1 and width of 12, 
	 * which is illegal)
	 */
	virtual void resizeBy( int north, int south, int east, int west );


	/**
	 * Rotates this matrix about its center by the given theta.
	 * The matrix is resized to a square that will accommodate the distance between its opposing
	 * corners.  For example, a 20 x 30 matrix will be resized to 36 x 36, and a 20 x 5 matrix
	 * will be resized to 21 x 21.
	 * Note that the employed algorithm does not handle subsequent rotations well
	 * (rotating first by <i>alpha</i> degrees then again by <i>beta</i>, for example); 
	 * distortions will occur.
	 * @param theta an angle of rotation within [0..360) degrees
	 */
	virtual void rotateBy( const double theta );


	/**
	 * Rotates a copy of this matrix about its center by the given theta.
	 * See rotateBy() for details.
	 * @param theta an angle of rotation within [0..360) degrees
	 */
	virtual RmMutableMatrix<T> rotatedBy( const double theta ) const;


	/**
	 * Resets all values in the matrix to the initialization value provided during construction.
	 */
	virtual void clear();


	/**
	 * Reinitializes the matrix to its initial height, width and cell values.
	 */
	virtual void empty();


	/**
	 * Returns the number of rows in the matrix.
	 */
	int height() const { return m_height; }


	/**
	 * Returns the number of columns in the matrix.
	 */
	int width() const { return m_width; }


	/**
	 * Provides access to the cell at the given x-y coordinate for read and write operations.  
	 * Indexes correspond to zero-based <code>[x][y]</code> coordinates, with <code>[0][0]</code> 
	 * accessing what may be considered the upper-left-most cell of a matrix that is built 
	 * top-to-bottom, left-to-right.
	 * <p>
	 * Indexes that reference a cell outside the bounds of the matrix cause it to be
	 * automatically resized, provided that behavior was enabled during construction;
	 * otherwise, an RmExceptions::IndexOutOfBoundsException is thrown.
	 * Positive values expand the matrix eastward and southward;
	 * negative values expand it westward and northward (following the signs of the standard
	 * four-quadrant Cartesian coordinate system).
	 * @param x the column of the matrix, where a negative value expands westward, and a
	 * positive value beyond its current bounds expands it eastward
	 * @param y the row of the matrix, where a negative value expands it northward, and a
	 * positive value beyond its current bounds expands it southward
	 * @return a reference to the object stored at the specified cell, which may be read from
	 * or written to
	 * @throws an RmExceptions::IndexOutOfBoundsException if the x-coordinate is out of bounds and 
	 * automatic grid expansion is disabled
	 */
	virtual T& valueAt( int x, int y );

	
	/**
	 * The <code>const</code> version of valueAt(), which implies the matrix cannot be resized.
	 * @throws an RmExceptions::IndexOutOfBoundsException if the 
	 * x or y coordinates are beyond the bounds of the matrix
	 */
	virtual const T& valueAt( int x, int y ) const;


	/**
	 * Along with a second <code>operator[]</code>, as in <code>myMatrix[3][4]</code>, this operator 
	 * pair provides the same functionality as valueAt().
	 * @param x the column of the matrix, where a negative value expands westward, and a
	 * positive value beyond its current bounds expands it eastward
	 */
	Operator2D<T> operator[]( int x ) { 
		return Operator2D<T>( this, x ); }

	
	/**
	 * The <code>const</code> version of operator[](), which implies the matrix cannot be resized.
	 */
	const Operator2D<T> operator[]( int x ) const { 
		return Operator2D<T>( this, x ); }


	/**
	 * Streams a text representation of the matrix in a top-down row-column format.
	 * The output of each cell is by default, an unformatted value, or optionally,
	 * a string formatted by the given format string.  For example, a format string of "%02d"
	 * when template T is of type int will produce "03", "15", etc.
	 * @param os the output stream
	 * @param format an optional C-style format string
	 */
	virtual std::ostream& put( std::ostream& os, const char *format = NULL ) const;

private:

	int m_initWidth, m_width;
	int m_initHeight, m_height;
	T m_initVal;
	const char* m_colSep;
	const char* m_rowSep;

	std::vector< std::vector<T> > m_2dVector;

	bool m_isAutoResizable;

};


template<class T>
RmMutableMatrix<T>::RmMutableMatrix( int w , int h, const T initVal, const char* colSep, 
	const char* rowSep, bool autoResize )
	: m_initWidth(w), m_width(w), m_initHeight(h), m_height(h),
	  m_2dVector(h, std::vector<T>( w, initVal )), m_initVal(initVal),
	  m_colSep(colSep), m_rowSep(rowSep), m_isAutoResizable(autoResize)
{
}


template<class T>
RmMutableMatrix<T>& RmMutableMatrix<T>::operator=( const RmMutableMatrix& source )
{
	m_initWidth = source.m_initWidth;
	m_initHeight = source.m_initHeight;
	m_initVal = source.m_initVal;
	m_width = source.m_width;
	m_height = source.m_height;
	m_2dVector = source.m_2dVector;
	m_isAutoResizable = source.m_isAutoResizable;
	m_colSep = source.m_colSep;
	m_rowSep = source.m_rowSep;

	return *this;
}



template<class T>
void RmMutableMatrix<T>::resizeBy( int north, int south, int east, int west )
{
	// A negative value removes that number of rows/colums from the corresponding border
	// A positive value adds that number of rows/colums to the corresponding border

	if ( m_height + north + south < 0 || m_width + east + west < 0 ) {
		char buff[100];
		sprintf( buff, "height = %d, width = %d, north = %d, south = %d, east = %d, west = %d",
			m_height, m_width, north, south, east, west );
		assert( false );
		throw RmExceptions::InvalidDimensionException( "RmMutableMatrix<T>::resizeBy()", buff );
	}

	// North
	if ( north < 0 ) {
		m_2dVector.erase( m_2dVector.begin(), m_2dVector.begin() + -north );
		m_height += north;
	}
	else if ( north > 0 ) {
		m_2dVector.insert( m_2dVector.begin(), north, std::vector<T>( m_width, m_initVal ) );
		m_height += north;
	}

	// South
	if ( south < 0 ) {
		m_2dVector.erase( m_2dVector.end() + south, m_2dVector.end() );
		m_height += south;
	}
	else if ( south > 0 ) {
		m_2dVector.insert( m_2dVector.end(), south, std::vector<T>( m_width, m_initVal ) );
		m_height += south;
	}

	// East
	if ( east < 0 ) 
	{
		for ( std::vector< std::vector<T> >::iterator rows = m_2dVector.begin(); 
			rows != m_2dVector.end(); ++rows )
		{
			std::vector<T>& row = *rows;
			row.erase( row.end() + east, row.end() );
		}
		m_width += east;
	}
	else if ( east > 0 ) {
		for ( std::vector< std::vector<T> >::iterator rows = m_2dVector.begin(); 
			rows != m_2dVector.end(); ++rows )
		{
			std::vector<T>& row = *rows;
			row.insert( row.end(), east, m_initVal );
		}
		m_width += east;
	}

	// West
	if ( west < 0 ) 
	{
		for ( std::vector< std::vector<T> >::iterator rows = m_2dVector.begin(); 
			rows != m_2dVector.end(); ++rows )
		{
			std::vector<T>& row = *rows;
			row.erase( row.begin(), row.begin() + -west );
		}
		m_width += west;
	}
	else if ( west > 0 ) {
		for ( std::vector< std::vector<T> >::iterator rows = m_2dVector.begin(); 
			rows != m_2dVector.end(); ++rows )
		{
			std::vector<T>& row = *rows;
			row.insert( row.begin(), west, m_initVal );
		}
		m_width += west;
	}
}


template<class T>
void RmMutableMatrix<T>::clear()
{
	for ( std::vector< std::vector<T> >::iterator rows = m_2dVector.begin(); 
		rows != m_2dVector.end(); ++rows )
	{
		for ( std::vector<T>::iterator cols = (*rows).begin(); cols != (*rows).end(); ++cols )
		{
			*cols = m_initVal;
		}
	}
}


template<class T>
void RmMutableMatrix<T>::empty()
{
	RmMutableMatrix<T>::resizeBy( -(m_height - m_initHeight), 0, -(m_width - m_initWidth), 0 );
	RmMutableMatrix<T>::clear();
}


template<class T>
T& RmMutableMatrix<T>::valueAt( int x, int y )
{
	// This method is cannot be a const because out-of-bounds indexes cause
	// the matrix to be resized.

	if ( m_isAutoResizable ) 
	{
		// Grow height southward (positive) or northward (negative)
		int yDelta = y < 0 ? -y : y >= m_height ? y - m_height + 1: 0;
		if ( yDelta != 0 ) {
			if ( y >= 0 ) RmMutableMatrix<T>::resizeBy( 0, yDelta, 0, 0 );
			else if ( y < 0 ) RmMutableMatrix<T>::resizeBy( yDelta, 0, 0, 0 );
		}

		// Grow width eastward (positive) or westward (negative)
		int xDelta = x < 0 ? -x : x >= m_width ? x - m_width + 1: 0;
		if ( xDelta != 0 ) {
			if ( x >= 0 ) RmMutableMatrix<T>::resizeBy( 0, 0, xDelta, 0 );
			else if ( x < 0 ) RmMutableMatrix<T>::resizeBy( 0, 0, 0, xDelta );
		}
	}
	else
	{
		if ( !(x >= 0 && x < m_width && y >= 0 && y < m_height) ) {
			char buff[50];
			sprintf( buff, "Index constraint: [0..%d)[0..%d); Requested index: [%d][%d]", 
				m_width, m_height, x, y );
			throw RmExceptions::IndexOutOfBoundsException( "RmMutableMatrix<T>::valueAt()", buff );
		}
	}

	return m_2dVector[y < 0 ? 0 : y][x < 0 ? 0 : x];
}


template<class T>
const T& RmMutableMatrix<T>::valueAt( int x, int y ) const
{
	if ( !(x >= 0 && x < m_width && y >= 0 && y < m_height) ) {
		char buff[50];
		sprintf( buff, "Index constraint: [0..%d)[0..%d); Requested index: [%d][%d]", 
			m_width, m_height, x, y );
		throw RmExceptions::IndexOutOfBoundsException( "RmMutableMatrix<T>::valueAt()", buff );
	}

	return m_2dVector[y][x];
}


template<class T>
RmMutableMatrix<T> RmMutableMatrix<T>::rotatedBy( const double theta ) const
{
	RmMutableMatrix<T> mm( *this );
	mm.rotateBy( theta );
	return mm;
}


template<class T>
void RmMutableMatrix<T>::rotateBy( const double theta )
{
	// Adapted from:  
	// Robert D. Grappel, "Rotating a Weather Map"
	// Dr. Dobb's Journal, June 1999
	// http://www.ddj.com/documents/s=902/ddj9906h/9906h.htm (by subscription)
	// Requirements:  input/output matrices MUST BE SQUARE
	//
	// Adaptations:
	// - vectors, derived parameters, point-of-use initialization
	// - removal of negation on radian calculation
	// - boundaries on loops from [1..nij] to [0..nij)
	// - auto-sizing of matrix into square accommodating distance between opposite corners
	// - correction for 1-pixel drift

	// Create squared copy of this matrix as input matrix
	const int w = m_width;
	const int h = m_height;
	const int d = static_cast<int>(sqrt( static_cast<double>(w * w + h * h) ) + 0.5);
	const double wDelta = (d - w) / 2.0;
	const double hDelta = (d - h) / 2.0;
	RmMutableMatrix<T> inmap( *this );
	inmap.resizeBy( hDelta, hDelta, wDelta, wDelta );

	// Square and clear this matrix as the output matrix
	const int nij = inmap.m_width;
	RmMutableMatrix<T>::clear();
	RmMutableMatrix<T>::resizeBy( hDelta, hDelta, wDelta, wDelta );

	// Precalc optimizations
	const int nijh = (nij / 2) + 1;            /* midpoint of the map */
	double rad = theta * RmUtility::Pi / 180.0;          /* convert angle to radians */
	const double s = sin( rad );
	const double c = cos( rad );
	const double n0 = 0.5 - nijh;

	/* precompute sine and cosine tables */
	std::vector<double> s0( nij );
	std::vector<double> c0( nij );
	for ( int i = 0; i < nij; i++ ) 
	{ 
		const double x0 = static_cast<double>(i) + n0;
		c0[i] = x0 * c;  
		s0[i] = x0 * s;
	}

	for ( i = 0; i < nij; i++ )
	{
		const double cc0 = nijh + c0[i];  
		const double ss0 = nijh - s0[i];
		for ( int j = 0; j < nij; j++ )
		{
			const double x = cc0 + s0[j];
			if ((x < 0) || (x > nij)) continue;
			const double y = ss0 + c0[j];
			if ((y < 0) || (y > nij)) continue;
			RmMutableMatrix<T>::valueAt( i, j ) = inmap.valueAt( x, y );
		}
	}

	//////
	// Correct for drift

	// . . . . . . .  Position A is the actual center of the image before rotation
	// . . . . . . .  After rotation by theta, that cell drifts to one of B, C, or D, as follows:
	// . . . . . . .    [315..45) : position A (no drift)
	// . . . A B . .    [45..135) : position B
	// . . . D C . .    [135..225): position C
	// . . . . . . .    [225..315): position D
	// . . . . . . .

	// Shift theta 45 degrees clockwise so that A is [0..90), B is [90..180), etc.
	// And divide into one of four quadrants
	int q = theta + 45;
	if ( q >= 360 ) q -= 360;
	switch( q /= 90 ) 
	{
		case 0: // A
			break;
		case 1: // B
			RmMutableMatrix<T>::resizeBy( 0, 0, 1, -1 );
			break;
		case 2: // C
			RmMutableMatrix<T>::resizeBy( -1, 1, 1, -1 );
			break;
		case 3: // D
			RmMutableMatrix<T>::resizeBy( -1, 1, 0, 0 );
			break;
	}
}



template<class T>
std::ostream& RmMutableMatrix<T>::put( std::ostream& os, const char *format ) const
{
	static char buff[10];

	for ( std::vector< std::vector<T> >::const_iterator rows = m_2dVector.begin(); 
		rows != m_2dVector.end(); ++rows )
	{
		for ( std::vector<T>::const_iterator cols = rows->begin(); cols != rows->end(); ++cols )
		{
			if ( format != NULL ) {
				sprintf( buff, format, *cols );
				os << buff << m_colSep;
			}
			else {
				os << *cols << m_colSep;
			}
		}
		os << m_rowSep;
	}
	return os;
}


template<class T>
std::ostream& operator<< ( std::ostream& os, const RmMutableMatrix<T>& v )
{
	return v.put( os );
}



#endif