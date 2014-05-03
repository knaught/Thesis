// RmBayesCertaintyGrid.cpp

#include <cmath>
#include <iostream>
#include "RmBayesCertaintyGrid.h"
using RmUtility::Coord;
#include "RmExceptions.h"

#define _CPP_EXTERN
#include "../polygon/polygon.h"

const float RmBayesCertaintyGrid::InitVal = 0.5f;


const std::string RmBayesCertaintyGrid::update( const RmUtility::MappedSonarReading& mr )
{
	// Text that goes to the log and map viewer application
	std::string logEntry;


	//////
	// Ignore "disabled" sonars
	if ( !m_settings->EnabledSonars[mr.reading.sonarNumber] ) return "";


	//////
	// Ignore or convert out of range readings

	int rangeReading = mr.reading.distance;
	bool isOutOfRange = false; // for use by cone model

	// Because Region I extends beyond the actual range reading by REGION_I_HALFWIDTH,
	// ensure that its outer boundary is within the range of the sonar because
	// RmBayesSonarModel::prOccupiedGivenSn() expects r <= R
	if ( rangeReading > RmPioneerController::SonarRange - m_settings->RegionIHalfwidth ) 
	{ 
		// Cone model processes 
		if ( m_settings->IgnoreOutOfRange ) { // && m_settings->SonarModel != RmUtility::Cone ) {
			return "";
		}
		isOutOfRange = true;
		rangeReading = m_settings->OutOfRangeConversion;
	}


	//////
	// Build sonar regions
	//
	// Region I is that surrounding the object detected at some distance along the sonar axis.
	// Region II is that which lies between Region I and the origin of the sonar gcSonar.
	//               F
	//         E     O     G      Region I: B-C-D-G-F-E
	//               C            Object (range reading)
	//           B       D
	//                            Region II: A-B-C-D
	//
	//               A            Origin (sonar position)

	// Convert world coordinates to grid coordinates
	const Coord gcRobot = gridCoord( mr.reading.robotPose.coord );
	const Coord gcSonar = gridCoord( mr.sonarPose.coord );
	const Coord gcObject = gridCoord( mr.objectCoord );

	// Find left and right boundaries of cone
	double thAxis = mr.reading.robotPose.theta + 
		RmPioneerController::SonarTheta[mr.reading.sonarNumber];
	if ( thAxis >= 360 ) thAxis -= 360;
	const double thLeft = thAxis - m_settings->Beta;
	const double thRight = thAxis + m_settings->Beta;
	const double d1 = rangeReading - m_settings->RegionIHalfwidth; // dist to region I
	const double d3 = rangeReading + m_settings->RegionIHalfwidth; // dist to region III

	// Create points of polygons defining regions
	const Coord wcSonar( mr.sonarPose.coord );
	const Coord a( gcSonar );
	const Coord b( gridCoord( wcSonar.mappedTo( thLeft, d1 ) ) );
	const Coord c( gridCoord( wcSonar.mappedTo( thAxis, d1 ) ) );
	const Coord d( gridCoord( wcSonar.mappedTo( thRight, d1 ) ) );
	const Coord e( gridCoord( wcSonar.mappedTo( thLeft, d3 ) ) );
	const Coord f( gridCoord( mr.objectCoord.mappedTo( thAxis, d3 - d1 ) ) );
		// See note in updateAxisCell()
	const Coord g( gridCoord( wcSonar.mappedTo( thRight, d3 ) ) );

	// Form the polygons for use by the polygon fill routine
	// NOTE! The order of these points is *extremely* important
	// They are a "connect-the-dots" representation of the two regional polygons
	struct Point regionI[] = 
		{ {b.x, b.y}, {c.x, c.y}, {d.x, d.y}, {g.x, g.y}, {f.x, f.y}, {e.x, e.y} };
	struct Point regionII[] = { {a.x, a.y}, {b.x, b.y}, {c.x, c.y}, {d.x, d.y} };


	//////
	// Build logfile entries

	static char buff[77]; // sprintf buffer that accommodates 19 numbers with at most 3 digits, 
		// each followed by 1 whitespace character, terminated by null (19 * 4 + 1)

	// Pose and range data
	const RmUtility::Pose globalPose = RmUtility::Pose( gcRobot, mr.reading.robotPose.theta );
	sprintf( buff, "%d %d %d %d %d\n", globalPose.coord.x, globalPose.coord.y, 
		static_cast<int>(globalPose.theta), mr.reading.sonarNumber, rangeReading );
	logEntry.append( buff );

	// Region fill data
	std::string regionFill;
	switch( m_settings->SonarModel )
	{
		case RmUtility::SingleCell:

			regionFill.append( updateCell( 
				gcObject, static_cast<double>(rangeReading) / m_settings->CellSize ) );
			break;

		case RmUtility::AcousticAxis:

			regionFill.append( updateAxis( gcSonar, gcObject, f ) );
			break;

		case RmUtility::Cone:

			PointListHeader pointListI = { 6, regionI };
			PointListHeader pointListII = { 4, regionII };

			try {
				if ( !isOutOfRange ) {
					regionFill.append( 
						updateRegion( RmBayesSonarModel::RegionI, &pointListI, gcSonar, thAxis ) );
				}
				regionFill.append( 
					updateRegion( RmBayesSonarModel::RegionII, &pointListII, gcSonar, thAxis ) );
			}
			catch( RmExceptions::Exception e ) {
				std::cerr << "Exception: " << e << "\n";
			}
			break;
	}
	if ( regionFill.length() == 0 ) return "";

	logEntry.append( regionFill );
	logEntry.append( "\n" );

	return logEntry;
}


std::string RmBayesCertaintyGrid::updateCell( const Coord& gcObject, double distance )
{
	// Update grid
	float &pr = valueAt( gcObject.x, gcObject.y );
	pr = m_sonarModel.prOccupiedGivenSn( pr, RmBayesSonarModel::RegionI, distance );

	// Log string
	static char buff[18]; // two signed 3-digit numbers and one 6-digit float, 
		// separated by one whitespace, terminated with null (2*5 + 1*7 + 1)

	sprintf( buff, "%d %d %.4f;", gcObject.x, gcObject.y, pr );
	assert( strlen( buff ) <= 17 );
	return std::string( buff );
}



std::string RmBayesCertaintyGrid::updateAxis( const Coord& gcSonar, const Coord& gcObject, 
	const RmUtility::Coord& gcRegionIII )
{
	// Note: This routine maps from sonar->object->regionIII rather than sonar->regionIII
	// because line sonar->object does not always align with that drawn from sonar->regionIII
	// (due to the low granularity of the grid-world coordinates)
	// It is important that these align for purposes of filtering obstructed readings
	// (see RmGlobalMap::obstructionBetween())

	std::string logEntry;

	// Sonar to object
	for ( struct PointList* linePointList = FillLine( gcSonar.x, gcSonar.y, gcObject.x, gcObject.y ); 
	   linePointList; linePointList = linePointList->next )
	{
		logEntry.append( updateAxisCell( gcSonar, gcObject, 
			Coord( linePointList->point.X, linePointList->point.Y ) ) );
	}

	// Object to Region III
	for ( linePointList = FillLine( gcObject.x, gcObject.y, gcRegionIII.x, gcRegionIII.y ); 
	   linePointList; linePointList = linePointList->next )
	{
		logEntry.append( updateAxisCell( gcSonar, gcObject, 
			Coord( linePointList->point.X, linePointList->point.Y ) ) );
	}

	return logEntry;
}



std::string RmBayesCertaintyGrid::updateAxisCell( const Coord &gcSonar, const Coord &gcObject, 
	const Coord &gcCell )
{
	static char buff[18]; // sprintf buffer that accommodates two 3-digit signed numbers
		// and one 6-digit float, each followed by one character, terminated with null 
		// (2*5 + 1*7 + 1) = 18

	RmBayesSonarModel::Region region = cellRegion( gcSonar, gcObject, gcCell );
	if ( region <= RmBayesSonarModel::RegionI )
	{
		int dx = gcCell.x - gcSonar.x;
		int dy = gcCell.y - gcSonar.y;
		double r = sqrt( pow( dx, 2 ) + pow( dy, 2 ) );
		if ( r > m_sonarModel.R ) r = m_sonarModel.R;
		try 
		{
			float &pr = valueAt( gcCell.x, gcCell.y );
			pr = m_sonarModel.prOccupiedGivenSn( pr, region, r );

			sprintf( buff, "%d %d %.4f;", gcCell.x, gcCell.y, pr );
			assert( strlen( buff ) <= 17 );
		}
		catch( RmExceptions::Exception e ) {
			std::cerr << "Exception caught in RmBayesCertaintyGrid::updateAxisCell(): " << e << "\n";
		}
	}

	return std::string( buff );
}



std::string RmBayesCertaintyGrid::updateRegion( const RmBayesSonarModel::Region region, 
	struct PointListHeader* polygon, const Coord& gcSonar, double thAxis )
{
	if ( region != RmBayesSonarModel::RegionI && region != RmBayesSonarModel::RegionII ) 
		throw RmExceptions::InvalidParameterException( "RmBayesCertaintyGrid::updateRegion", 
			"Invalid region specification" );
	if ( polygon == 0 ) 
		throw RmExceptions::InvalidParameterException( "RmBayesCertaintyGrid::updateRegion", 
			"Null boundary specification" );

	// Allocate buffer for building log string to be returned
	struct PointList* interiorPointList = 
		FillPolygon( polygon, 0, region == RmBayesSonarModel::RegionI ? NONCONVEX : CONVEX, 0, 0 );
	int numPoints = 0;
	for ( struct PointList* ipl = interiorPointList; ipl; ipl = ipl->next ) ++numPoints;
	const unsigned int buffLen = numPoints * 17;	// 17 chars per entry
	char* buff = new char[buffLen + 1];	// plus one terminating null
	buff[0] = 0;						// initialize to zero-length string
	char* buffPtr = buff;

	assert( strlen( buff ) <= buffLen );

	for ( ; interiorPointList; interiorPointList = interiorPointList->next )
	{
		Coord gcCell( interiorPointList->point.X, interiorPointList->point.Y );

		// Calculate quadrant-relative angle of the cell
		// sin(theta) = dy / r  =>  theta = arcsin( dy / r )
		const int dx = gcCell.x - gcSonar.x;
		const int dy = gcCell.y - gcSonar.y;
		double r = sqrt( pow( dx, 2 ) + pow( dy, 2 ) );
		if ( r > m_sonarModel.R ) r = m_sonarModel.R;
		const double thcos = dy / r;
		const double acosCell = acos( thcos );
		double thCell = acosCell / RmUtility::RadianFactor;
		if ( dx < 0 ) thCell = 360 - thCell;

		// Calculate distance between the two angles (alpha in the sonar model).
		// Note that due to course granularity of grid cells (especially for small range readings),
		// some cells along border of cone will actually be outside the 30 degree cone.
		// Testing showed approximately 0.12% of the cells fall into this category.
		// The options are to treat these like they are on the boundary of the cone or to ignore them.
		double alpha = fabs( thAxis - thCell );
		if ( alpha > m_settings->Beta ) alpha = m_settings->Beta;

		// Update the probability
		try {
			float &pr = valueAt( gcCell.x, gcCell.y );
			pr = m_sonarModel.prOccupiedGivenSn( pr, region, r, alpha );
			sprintf( buffPtr, "%d %d %.4f;", gcCell.x, gcCell.y, pr );
			buffPtr += strlen( buffPtr );

			if ( strlen( buffPtr ) > 17 ) {
				std::cerr << "Error: " << strlen( buff ) << " " << buff << "\n" << gcCell << std::endl;
			}
		}
		catch( RmExceptions::Exception e ) {
			std::cerr << "Exception caught in RmBayesCertaintyGrid::updateRegion(): " << e << "\n";
		}
	}

	if ( strlen( buff ) > buffLen ) {
		std::cerr << "Error: " << strlen( buffPtr ) << " " << buffPtr << "\n" << strlen( buff ) << " " << buff << std::endl;
		assert( strlen( buff ) <= buffLen );
	}
	std::string logString( buff );
	delete[] buff;

	return logString;
}


RmBayesSonarModel::Region RmBayesCertaintyGrid::cellRegion( const Coord& coordSonar,
	const Coord& coordObject, const Coord& coordCell ) const
{
	// Get distance from object to cell
	// sqrt[ (x1 - x2)^2 + (y1 - y2)^2 ]
	double distToObject = sqrt( pow( coordSonar.x - coordObject.x, 2 ) + 
		pow( coordSonar.y - coordObject.y, 2 ) );
	
	// Get distance from robot to cell
	double distToCell = sqrt( pow( coordSonar.x - coordCell.x, 2 ) + 
		pow( coordSonar.y - coordCell.y, 2 ) );
	
	// Region: Out of range
	int outOfRange = RmPioneerController::SonarRange / m_settings->CellSize;
	if ( floor( distToObject ) > outOfRange ) return RmBayesSonarModel::OutOfRange;
	
	// Region: III
	double delta = distToCell - distToObject;
	if ( delta > m_settings->RegionIHalfwidth / m_settings->CellSize ) {
		return RmBayesSonarModel::RegionIII;
	}
	
	// Region: II
	if ( fabs( delta ) > m_settings->RegionIHalfwidth / m_settings->CellSize ) {
		return RmBayesSonarModel::RegionII;
	}
	
	// Region: I
	return RmBayesSonarModel::RegionI;
}


Coord RmBayesCertaintyGrid::gridCoord( Coord worldCoord ) const
{
	// Convert to grid coordinate scale
	double gcX = worldCoord.x / static_cast<double>(m_settings->CellSize);
	double gcY = worldCoord.y / static_cast<double>(m_settings->CellSize);
	
	// Compute the "ceiling" for negative coordinates such that the proper cell is returned
	// (if a wc falls within the cell between (-382,-383), its y-coord is actually -383;
	// if a wc falls within the cell between (382,383), it's y-coord is 382)
	// For example, (-382.2, -52.89) should map to (-383, -53)
	// whereas (382.2, 52.89) should map to (382, 52)
	// This accounts for coordinate (0,0) being the first cell over, first cell up.
	int x = static_cast<int>( floor(gcX) == gcX ? gcX : (gcX > 0 ? gcX : gcX - 1) );
	int y = static_cast<int>( floor(gcY) == gcY ? gcY : (gcY > 0 ? gcY : gcY - 1) );
	
	// Adjust for grid world center
	// (The m_GridCenter adjustment is made after ceiling because the sign of the 
	// original coordinate is significant to the result; for example, 
	// ceiling( -37.1 + 250 ) = 213; whereas, ceiling( -37.1 ) + 250 = 212).
	return Coord( x, y );
}


std::ostream& operator<<( std::ostream& os, const RmBayesCertaintyGrid& grid )
{
	return grid.put( os );
}
