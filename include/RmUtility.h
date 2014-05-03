// RmUtility.h

#ifndef RM_UTILITY_H
#define RM_UTILITY_H

#define NUM_SONARS 16	// With great consternation, I resort to an ugly define for efficiency's
						// sake; utilization within SonarReading resulted in a gain of ~30ms per 
						// 700 readings.

#include <cmath>
#include <iostream>
#include "Aria.h"

/**
 * Provides a collection of mapping-related structures that are useful throught the mapping
 * application.
 */

namespace RmUtility {

/**
 * A Cartesian coordinate that is defined by an x and y component.
 * This class includes operators for addition, subtraction, comparison, and streaming,
 * as well as a handful of methods for transforming and rotating coordinates.
 */
class Coord
{
public:

	/** The x-component of the coordinate */
	int x;

	/** The y-component of the coordinate */
	int y;

	/** Constructor that provides default initialization to (0,0) */
	Coord( int x_ = 0, int y_ = 0 ) : x( x_ ), y( y_ ) {}

	/**
	 * Scales this coordinate by the given factor (coord / factor).
	 */
	Coord& scale( const double factor );

	/**
	 * Returns a scaled copy of this coordinate (coord / factor).
	 */
	Coord scaled( const double factor ) { 
		Coord c(*this); return c.scale( factor ); 
	}

	/** 
	 * Returns the difference between two coordinates, where the x and y values of the
	 * <code>subtrahend</code> are subtracted from those of this coordinate object. 
	 */
	Coord operator-( const Coord& subtrahend ) const;

	/** 
	 * Returns the sum of the two coordinates, where the x and y values of the
	 * <code>addend</code> are added to those of this coordinate object. 
	 */
	Coord operator+( const Coord& addend ) const;

	/** 
	 * Assignment operator that copies the values of the given Coordinate into this object.
	 */
	Coord& operator=( const Coord& coord ) { 
		this->x = coord.x; this->y = coord.y; return *this; 
	}

	/**
	 * Adds the given addend to this coordinate.
	 */
	Coord& operator+=( const Coord& addend ) { 
		this->x += addend.x; this->y += addend.y; return *this; 
	}

	/**
	 * Subtracts the given subtrahend from this coordinate.
	 */
	Coord& operator-=( const Coord& subtrahend ) { 
		this->x -= subtrahend.x; this->y -= subtrahend.y; return *this; 
	}

	/**
	 * Returns true if the x and y components of this and the given coordinate are identical.
	 */
	bool operator==( const Coord& coord ) const {
		return x == coord.x && y == coord.y;
	}

	/**
	 * Returns true if either of the x or y components of this and the given coordinate are not
	 * identical.
	 */
	bool operator!=( const Coord& coord ) const {
		return x != coord.x || y != coord.y;
	}

	/** 
	 * Returns a copy of this coordinate mapped at distance <code>d</code> and theta <code>th</code>
	 * from this coordinate.
	 */
	Coord mappedTo( double th, double d ) const;

	/** 
	 * Modifies this coordinate such that it is mapped at distance <code>d</code> and theta 
	 * <code>th</code> from itself.
	 */
	void mapTo( double th, double d );

	/** 
	 * Returns a copy of this coordinate rotated <code>th</code> degrees about the given 
	 * <code>pivot</code> point.
	 */
	Coord rotatedBy( double th, Coord pivot = Coord() ) const;

	/** 
	 * Modifies this coordinate such that it is rotated <code>th</code> degrees about the given 
	 * <code>pivot</code> point.
	 */
	void rotateBy( double th, Coord pivot = Coord() );

	/** Returns the distance between this and the given coordinate. */
	double distanceTo( const Coord& coord ) const;

	/** Returns the distance between this and the given coordinate. */
	double distanceFrom( const Coord& coord ) const { return distanceTo( coord ); }

	/** 
	 * Returns the polar angle that extends clockwise from this to the given coordinate. 
	 * The angles of both coordinates are calculated from (0,0).
	 */
	double angleTo( const Coord& coord ) const;

	/** 
	 * Returns the clockwise polar angle from the given coordinate to this coordinate.
	 * The angles of both coordinates are calculated from (0,0).
	 */
	double angleFrom( const Coord& coord ) const { return coord.angleTo( *this ); }

	/** Sends a text representation of the given object to the given stream. */
	friend std::ostream& operator<<( std::ostream& os, const Coord& coord );
};


/**
 * Specifies an upper-left and lower-right coordinates that identify a rectangular bounding box
 * in a Cartesian coordinate system.
 */
struct BoundBox
{
	/** The upper-left corner of the bounding box */
	Coord ul;

	/** The lower-right corner of the bounding box */
	Coord lr;

	/** Provides means of initialization using specific x-y values */
	BoundBox( int ulX = 0, int ulY = 0, int lrX = 0, int lrY = 0 )
		: ul( ulX, ulY ), lr( lrX, lrY ) {}

	/** Provides means of initialization using Coord values */
	BoundBox( Coord ul_, Coord lr_ )
		: ul( ul_ ), lr( lr_ ) {}

	/** Returns the width of this bounding box. */
	int width() const { return abs( ul.x - lr.x ) + 1; }

	/** Returns the height of this bounding box. */
	int height() const { return abs( ul.y - lr.y ) + 1; }

	/** Returns true if this box contains the given point. */
	bool contains( const Coord& point ) const;

	/** Enlarges this box to the region surrounding it and the given box. */
	BoundBox& unionWith( const BoundBox& box );

	/** Reduces this box to that region encompassing both this and the given box. */
	BoundBox& intersectWith( const BoundBox& box );

	/** Expands the bounds of this box by the given amounts. */
	BoundBox& expandBy( int n, int s, int e, int w );

	/** 
	 * Returns the difference between this and the given BoundBox, where the ul and lr values of the
	 * <code>subtrahend</code> are subtracted from those of this BoundBox object. 
	 */
	BoundBox operator-( const BoundBox& subtrahend ) const;

	/** 
	 * Returns the sum of this and the given BoundBox, where the ul and lr values of the
	 * <code>addend</code> are added to those of this BoundBox object. 
	 */
	BoundBox operator+( const BoundBox& addend ) const;

	/** Sends a text representation of the given object to the given stream. */
	friend std::ostream& operator<<( std::ostream& os, const BoundBox& box );
};


/**
 * Provides a standardized way of representing, accessing, and manipulating an object's pose,
 * that is, its position on a Cartesian coordinate system and its heading.
 */
struct Pose
{
	/** The Cartesian position of the object */
	Coord coord;

	/** The heading of the object */
	double theta;

	/** Provides means of initialization using a Coord value */
	Pose( Coord c = Coord(), double t = 0.0 ) : coord(c), theta(t) {}

	/** Provides means of initialization using specific x-y values */
	Pose( int x, int y, double t = 0.0 ) : coord(x, y), theta(t) {}

	/** 
	 * Scales the pose coordinate by the given factor (coord / factor).
	 */
	Pose& scale( const double factor ) { coord.scale( factor ); return *this; }

	/** 
	 * Returns a copy of this pose scaled by the given factor (see scale()).
	 */
	Pose scaled( const double factor ) const { Pose p(*this); return p.scale( factor ); }

	/** 
	 * Returns the difference between two poses, where the x, y, and theta values of the
	 * <code>subtrahend</code> are subtracted from those of this pose object. 
	 * The return theta will always be between [0..360).
	 */
	Pose operator-( const Pose& subtrahend ) const;

	/** 
	 * Returns the sum of two poses, where the x, y, and theta values of the
	 * <code>addend</code> are added to those of this Pose object. 
	 * The return theta will always be between [0..360).
	 */
	Pose operator+( const Pose& addend ) const;

	/** 
	 * Assignment operator that copies the values of the given Pose into this object.
	 */
	Pose& operator=( const Pose& pose ) { coord = pose.coord; theta = pose.theta; return *this; }

	/** 
	 * Assignment operator that places the sum of this and the given Pose into this object.
	 */
	Pose& operator+=( const Pose& addend ) { *this = *this + addend; return *this; }

	/** 
	 * Assignment operator that places the difference of this and the given Pose into this object.
	 */
	Pose& operator-=( const Pose& addend ) { *this = *this - addend; return *this; }

	/**
	 * Returns true if the x, y, and theta components of this and the given Pose are identical.
	 */
	bool operator==( const Pose& pose ) const { 
		return coord == pose.coord && theta == pose.theta; }

	/**
	 * Returns true if either of the x, y, or theta components of this and the given Pose are not
	 * identical.
	 */
	bool operator!=( const Pose& pose ) const { 	
		return coord != pose.coord || theta != pose.theta; }

	/** Sends a text representation of the given object to the given stream. */
	friend std::ostream& operator<<( std::ostream& os, const Pose& pose );
};


/**
 * Provides a platform-independent means of representing all pertinent data in a sonar
 * range reading.
 */
struct SonarReading
{
	/** The position and heading of the robot */
	Pose robotPose;

	/** The sonar number from which the reading was taken */
	int sonarNumber;

	/** The sonar range reading, or distance value it returned */
	int distance;

	/** Array of distances returned by a single sweep of all sonars, from which distance is taken. */ 
	int all[NUM_SONARS];

	/**
	 * Initializes all members to their corresponding zero-values.
	 */
	SonarReading()
		: robotPose(), sonarNumber(0), distance(0) { initRanges( NULL ); }


	/** 
	 * Intializes all members to the given values.
	 * @param p robot pose
	 * @param s sonar number
	 * @param a array of all sonar range readings taken at pose p
	 */
	SonarReading( const Pose &p, const int *a = NULL, int s = 0 )
		: robotPose(p), sonarNumber(s), distance(a==NULL?0:a[s]) { initRanges( a ); }


	/**
	 * Initializes with pose and sonar readings as given by ArRobot.
	 * @param robot provides pose and sonar range reading data
	 */
	SonarReading( ArRobot* robot );


	/**
	 * Initializes with the given sonar log data string.
	 * Uses strtok() to parse the line, so line will be modified.
	 * @param line log entry in format <code>x y th.dddd r0 r1 r2 ... r15</code>
	 */
	SonarReading( char *line );


	/**
	 * Initializes this object with the given reading.
	 */
	SonarReading( const SonarReading &r );


	/**
	 * Initializes all sonar range readings to those values in the given array.
	 * If a is NULL, range readings are initialized to 0.
	 */
	void initRanges( const int *a );


	/**
	 * Scales the robot pose and range reading distance by the given factor.
	 */
	SonarReading& scale( const double factor ) { 
		robotPose.scale( factor ); distance = static_cast<int>(distance / factor); return *this; }

	/**
	 * Returns a copy of this SonarReading scaled by the given factor (see scale()).
	 */
	SonarReading scaled( const double factor ) { SonarReading r(*this); return r.scale( factor ); }


	/**
	 * Reinitializes this object with all data values of the given reading.
	 */
	SonarReading& operator=( const SonarReading &r );

	/** Sends a text representation of the given object to the given stream. */
	friend std::ostream& operator<<( std::ostream &os, const SonarReading &pose );
};


/**
 * Provides a means of representing a range reading in the context of a Cartesian coordinate
 * system, where the robot, sonar, and range reading are all mapped to specific cooordinates.
 */
struct MappedSonarReading
{
	/** The sonar reading for which other data is calculated */
	SonarReading reading;

	/** The position and heading of the sonar identified in the #reading */
	Pose sonarPose;

	/** The position of the sensed object as identified in the #reading */
	Coord objectCoord;

	/** 
	 * Provides a means for default initialization of all components to their corresponding
	 * zero-values.
	 */
	MappedSonarReading( SonarReading r = SonarReading(), Pose s = Pose(), Coord o = Coord() )
		: reading(r), sonarPose(s), objectCoord(o) {}

	/**
	 * Scales the reading, pose, and object coordinate by the given factor.
	 */
	MappedSonarReading& scale( const double factor ) { 
		reading.scale( factor ); sonarPose.scale( factor ); objectCoord.scale( factor ); return *this; }

	/**
	 * Returns a copy of this MappedSonarReading scaled by the given factor (see scale()).
	 */
	MappedSonarReading scaled( const double factor ) { 
		MappedSonarReading r(*this); return r.scale( factor ); }

	/** Sends a text representation of the given object to the given stream. */
	friend std::ostream& operator<<( std::ostream& os, const MappedSonarReading& r );
};


/** Identifies methods by which occupancy grid may be updated. */
enum SonarModelEnum { 
	/** The cell corresponding to the object at the intersection of the sonar's acoustic axis and
		range reading */
	SingleCell, 
	/** The cells along the acoustic axis from the sonar device to the sensed object
		as identified by the range reading */
	AcousticAxis, 
	/** All the cells within regions I and II of the sonar cone */
	Cone 
};

/** The value by which degrees must be multiplied in order to approximate them in Radians */
static const double RadianFactor = 0.0174532;

/** Self explanatory */
static const long double Pi = 3.1415926535897932384626433832795028841968;

};
#endif