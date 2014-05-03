// RmSonarMap.h


#ifndef RM_SONAR_MAP_H
#define RM_SONAR_MAP_H

#include <string>
#include <iostream>
#include "RmUtility.h"


/**
 * Provides a virtual superclass that defines a standard interface for a sonar-based 
 * occupancy grid map.
 */

class RmSonarMap
{
public:

	/**
	 * Updates the sonar map using the given sonar range reading data.
	 */
	virtual const std::string update( const RmUtility::SonarReading& reading ) = 0;


	/**
	 * Streams a text representation of the map to the given stream.
	 * A friend <code>operator&lt;&lt;( ostream&, const RmSonarMap& )</code> can call on this 
	 * method for any RmSonarMap-derived object.
	 */
	virtual std::ostream& put( std::ostream& ) const = 0;


	/**
	 * Returns the width of the map as a count of columns.
	 */
	virtual int width() const = 0;


	/**
	 * Returns the height of the map as a count of rows.
	 */
	virtual int height() const = 0;


	/**
	 * Returns the upper-left and lower-right coordinates that bound this map.
	 */
	virtual RmUtility::BoundBox bound() const = 0;


	/**
	 * Reinitializes the map to its initial height, width and cell values.
	 */
	virtual void empty() = 0;


	/**
	 * Clears all map cells to their initial value.
	 */
	virtual void clear() = 0;


	/**
	 * Calls on the RmSonarMap::put()-derived method for streaming a text representation
	 * of the map.
	 */
	friend std::ostream& operator<<( std::ostream& os, const RmSonarMap& map );
};

#endif