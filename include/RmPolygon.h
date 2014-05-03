// RmPolygon.h

#ifndef RM_POLYGON_H
#define RM_POLYGON_H

#include <iostream>
#include <vector>
#include "RmUtility.h"
#include "RmMutableMatrix.h"

#define _CPP_EXTERN // ko: turn on function prototypes in gpc.h
#include "../polygon/polygon.h"
#include "../polygon/gpc.h"


/**
 * Provides polygon functions, such as intersection, difference, and union.
 */
class RmPolygon
{
public:

	/**
	 * Creates an RmPolygon using the edges formed by b.
	 */
	RmPolygon( const RmUtility::BoundBox &b );

	
	/**
	 * Creates an RmPolygon using p.
	 */
	RmPolygon( const gpc_polygon &p );


	/**
	 * Creates an RmPolygon as a deep copy of p.
	 */
	RmPolygon( const RmPolygon &p );


	/**
	 * Destroys this RmPolygon.
	 */
	~RmPolygon();


	/**
	 * Replaces the existing RmPolygon with p.
	 */
	RmPolygon& operator=( const RmPolygon &p );


	/**
	 * Converts this RmPolygon to its intersection with p.
	 */
	RmPolygon& intersectWith( const RmPolygon &p );


	/**
	 * Alias for intersectWith.
	 */
	RmPolygon& operator*=( const RmPolygon &p ) { return intersectWith( p ); }


	/**
	 * Returns an RmPolygon that is the intersection of this and p.
	 */
	RmPolygon intersectedWith( const RmPolygon &p ) const;


	/**
	 * Alias for intersectedWith.
	 */
	RmPolygon operator*( const RmPolygon &p ) const { return intersectedWith( p ); }


	/**
	 * Modifies this such that the regions intersecting p are removed.
	 */
	RmPolygon& subtractOut( const RmPolygon &p );


	/**
	 * Alias for subtractOut.
	 */
	RmPolygon& operator-=( const RmPolygon &p ) { return subtractOut( p ); }


	/**
	 * Returns the RmPolygon that results from removing from this its intersection with p.
	 */
	RmPolygon subtractedOut( const RmPolygon &p ) const;


	/**
	 * Alias for subtractedOut.
	 */
	RmPolygon operator-( const RmPolygon &p ) const { return subtractedOut( p ); }


	/**
	 * Modifies this such that its region is unioned with p.
	 */
	RmPolygon& unionWith( const RmPolygon &p );


	/**
	 * Alias for unionWith.
	 */
	RmPolygon& operator+=( const RmPolygon &p ) { return unionWith( p ); }


	/**
	 * Returns the RmPolygon that results from unioning the regions of this and p.
	 */
	RmPolygon unionedWith( const RmPolygon &p ) const;


	/**
	 * Alias for unionedWith.
	 */
	RmPolygon operator+( const RmPolygon &p ) const { return unionedWith( p ); }


	/**
	 * Returns the number of contours (independent polygons) that form this RmPolygon.
	 */
	int numContours() const { return m_polygon.num_contours; }


	/**
	 * Sends a text representation of this RmPolygon to the given stream.
	 */
	std::ostream& put( std::ostream &os ) const;


	friend std::ostream& operator<<( std::ostream &os, const RmPolygon &p );


	/**
	 * Fills the given vector with the coordinates that compose the interior fill of all non-hole
	 * contours within this RmPolygon.
	 * Those cells on the northern and eastern borders of a polygon are excluded in order to allow
	 * for those cases when polygons share borders.
	 */
	void fillInto( std::vector<RmUtility::Coord> *v ) const;


	/**
	 * Returns a vector with the coordinates that compose the interior fill of all non-hole
	 * contours within this RmPolygon.
	 * Those cells on the northern and eastern borders of a polygon are excluded in order to allow
	 * for those cases when polygons share borders.
	 */
	std::vector<RmUtility::Coord> fill() const;


	/**
	 * Returns a vector of coordinates that lie on the line between [from..to).
	 */
	static std::vector<RmUtility::Coord> line( const RmUtility::Coord &from, const RmUtility::Coord &to );


protected:

	/**
	 * Allocates memory and copies the structure of p into this.
	 */
	void createFrom( const gpc_polygon &p );


	/**
	 * Releases all memory allocated during construction.
	 */
	void free();


	/**
	 * Performs the specified operation, defined in gpc.h, on p.
	 */
	RmPolygon& clip( gpc_op op, const RmPolygon &p );

private:

	gpc_polygon m_polygon;

};

#endif