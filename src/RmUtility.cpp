// RmUtility.cpp

#include "RmUtility.h"
#include "RmPioneerController.h"

using namespace RmUtility;

/////////////////////////
//        Coord        //
/////////////////////////


Coord Coord::operator +( const Coord &addend ) const
{
	return Coord( x + addend.x, y + addend.y );
}


Coord Coord::operator -( const Coord &subtrahend ) const
{
	return Coord( x - subtrahend.x, y - subtrahend.y );
}


Coord &Coord::scale( const double factor )
{
	// Convert to grid coordinate scale
	double x = this->x / factor;
	double y = this->y / factor;
	
	// Compute the "ceiling" for negative coordinates such that the proper cell is returned
	// (if a coord falls within the cell between (-382,-383), its y-coord is actually -383;
	// if a coord falls within the cell between (382,383), it's y-coord is 382)
	// For example, (-382.2, -52.89) should map to (-383, -53)
	// whereas (382.2, 52.89) should map to (382, 52)
	// This accounts for coordinate (0,0) being the first cell over, first cell up.
	this->x = static_cast<int>( floor(x) == x ? x : (x > 0 ? x : x - 1) );
	this->y = static_cast<int>( floor(y) == y ? y : (y > 0 ? y : y - 1) );
	
	return *this;
}


Coord Coord::mappedTo( double theta, double distance ) const
{
	Coord coord;
	double sinTh = sin( theta * RadianFactor );
	double cosTh = cos( theta * RadianFactor );
	coord.x = x + static_cast<int>(sinTh * distance + (sinTh < 0 ? -0.5 : 0.5 ));
	coord.y = y + static_cast<int>(cosTh * distance + (cosTh < 0 ? -0.5 : 0.5 ));
	return coord;
}


void Coord::mapTo( double theta, double distance )
{
	*this = mappedTo( theta, distance );
}


Coord Coord::rotatedBy( double rotationTheta, Coord pivot ) const
{
	// Get polar magnitude and direction of existing coord
	double mag = distanceFrom( pivot );
	double th = angleFrom( pivot );

	// Rotate to new coordinate
	th += rotationTheta;
	if ( th >= 360 ) th -= 360;
	return pivot.mappedTo( th, mag );
}


void Coord::rotateBy( double rotationTheta, Coord pivot )
{
	*this = rotatedBy( rotationTheta, pivot );
}


double Coord::distanceTo( const Coord &coord ) const
{
	Coord delta = coord - *this;
	return sqrt( static_cast<double>(delta.x * delta.x + delta.y * delta.y) );
}


double Coord::angleTo( const Coord &coord ) const
{
	Coord delta = coord - *this;
	
	// (this is real ugly, but it works)
	double th = delta.x != 0 ? atan( delta.y / static_cast<double>(delta.x) ) / RadianFactor : 0;
	return (delta.x > 0 ? 90 : delta.x == 0 ? delta.y >= 0 ? 0 : 180 : 270) - th;
}


// Note: Doxygen doesn't like the RmUtility:: qualifier, but the linker requires it
std::ostream &RmUtility::operator<<( std::ostream &os, const Coord &c )
{
	return os << "(" << c.x << ", " << c.y << ")";
}



/////////////////////////
//      BoundBox       //
/////////////////////////


bool BoundBox::contains( const Coord &point ) const
{
	return ul.x <= point.x && lr.x >= point.x && ul.y >= point.y && lr.y <= point.y;
}


BoundBox&  BoundBox::unionWith( const BoundBox &box )
{
	if ( box.ul.x < ul.x ) ul.x = box.ul.x;
	if ( box.ul.y > ul.y ) ul.y = box.ul.y;
	if ( box.lr.x > lr.x ) lr.x = box.lr.x;
	if ( box.lr.y < lr.y ) lr.y = box.lr.y;

	return *this;
}


BoundBox&  BoundBox::intersectWith( const BoundBox &box )
{
	if ( box.ul.x > ul.x ) ul.x = box.ul.x;
	if ( box.ul.y < ul.y ) ul.y = box.ul.y;
	if ( box.lr.x < lr.x ) lr.x = box.lr.x;
	if ( box.lr.y > lr.y ) lr.y = box.lr.y;

	return *this;
}


BoundBox& BoundBox::expandBy( int n, int s, int e, int w )
{
	ul.y += n;
	lr.y += s;
	lr.x += e;
	ul.x += w;

	return *this;
}


BoundBox BoundBox::operator-( const BoundBox &subtrahend ) const
{
	return BoundBox( ul - subtrahend.ul, lr - subtrahend.lr );
}


BoundBox BoundBox::operator+( const BoundBox &addend ) const
{
	return BoundBox( ul + addend.ul, lr + addend.lr );
}


// Note: Doxygen doesn't like the RmUtility:: qualifier, but the linker requires it
std::ostream &RmUtility::operator<<( std::ostream &os, const BoundBox &box )
{
	return os << box.ul << " " << box.lr;
}



/////////////////////////
//    SonarReading     //
/////////////////////////


SonarReading::SonarReading( ArRobot *robot )
	: sonarNumber(0), distance(0)
{
	ArPose arPose = robot->getPose();
	robotPose = RmPioneerController::pose( arPose ); 
	for ( int i = 0; i < RmPioneerController::NumSonars; ++i ) {
		all[i] = static_cast<int>(robot->getSonarRange( i ));
	}
}


SonarReading::SonarReading( char *line )
	: sonarNumber(0), distance(0)
{
	char* token = strtok( line, " " );
	const int x = atoi(token);
	token = strtok( NULL, " " );
	const int y = atoi(token);
	token = strtok( NULL, " " );
	const double th = atof(token);
	robotPose = RmPioneerController::pose( ArPose( x, y, th ) ); // Pose( x, y, th );
		// for backward compatability with thesis data,
		// must retain the assumption that the data file contains raw Aria pose info
		// and needs to be converted
	for ( int i = 0; i < RmPioneerController::NumSonars; ++i )
	{
		token = strtok( NULL, " " );
		if ( token ) all[i] = atoi( token );
		else all[i] = 0;
	}
}


void SonarReading::initRanges( const int *r )
{
	if ( r == NULL ) {
		for ( int i = 0; i < RmPioneerController::NumSonars; ++i ) all[i] = 0;
	}
	else {
		for ( int i = 0; i < RmPioneerController::NumSonars; ++i ) all[i] = r[i];
	}
}


SonarReading::SonarReading( const SonarReading &r )
{
	robotPose = r.robotPose;
	sonarNumber = r.sonarNumber;
	distance = r.distance;
	initRanges( &r.all[0] );
}


SonarReading& SonarReading::operator=( const SonarReading &r )
{
	robotPose = r.robotPose;
	sonarNumber = r.sonarNumber;
	initRanges( &r.all[0] );

	return *this;
}


// Note: Doxygen doesn't like the RmUtility:: qualifier, but the linker requires it
std::ostream &RmUtility::operator<<( std::ostream &os, const SonarReading &r )
{
	os << r.robotPose << " " << r.sonarNumber << " " << r.all[r.sonarNumber] << " [ ";
	for ( int i = 0; i < RmPioneerController::NumSonars; ++i ) os << r.all[i] << " ";
	return os << "]";
}



/////////////////////////
// MappedSonarReading  //
/////////////////////////


// Note: Doxygen doesn't like the RmUtility:: qualifier, but the linker requires it
std::ostream &RmUtility::operator<<( std::ostream &os, const MappedSonarReading &r )
{
	return os 
		<< "Robot: " << r.reading << "\n"
		<< "Sonar: " << r.sonarPose << "\n"
		<< "Object: " << r.objectCoord << "\n";
}



/////////////////////////
//        Pose         //
/////////////////////////


Pose Pose::operator -( const Pose &subtrahend ) const
{
	double th = theta - subtrahend.theta;
	if ( th >= 360 ) th -= 360;
	else if ( th <= -360 ) th += 360;
	return Pose( coord - subtrahend.coord, th ); 
}


Pose Pose::operator +( const Pose &addend ) const
{
	double th = theta + addend.theta;
	if ( th >= 360 ) th -= 360;
	else if ( th <= -360 ) th += 360;
	return Pose( coord + addend.coord, th );
}

// Note: Doxygen doesn't like the RmUtility:: qualifier, but the linker requires it
std::ostream &RmUtility::operator<<( std::ostream &os, const Pose &pose )
{
	return os << pose.coord << ":" << pose.theta;
}
