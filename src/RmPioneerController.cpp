// RmPioneerController.cpp

#include <assert.h>

#include <vector>
#include <iostream>
#include "RmPioneerController.h"


const double RmPioneerController::DistToSonar[] = { 194.74, 217.83, 234.09, 241.30, 241.30, 234.09, 
	217.83, 194.74, 194.74, 217.83, 234.09, 241.30, 241.30, 234.09, 217.83, 194.74 }; // millimeters
	// Calculated using x_offset and y_offset, as given in Ken Laviers' HistoGram.cpp code:
	// x_offset[] = {145,185,220,240, 240, 220, 185, 145,-145,-185,-220,-240,-240,-220,-185,-145};
	// y_offset[] = {130,115, 80, 25, -25, -80,-115,-130,-130,-115,-80, -25,  25,   80, 115, 130};
	// Because these are oriented with quadrant 1 being the left half of the forward sonar array,
	// they are converted to an orientation wherein quadrant 4 is the left half of the forward 
	// sonar array:
	// x_offset[] = {-130,-115,-80,-25, 25, 80,115,130, 130, 115,  80,  25, -25, -80,-115,-130};
	// y_offset[] = { 145, 185,220,240,240,220,185,145,-145,-185,-220,-240,-240,-220,-185,-145};  
	// dist = sqrt( x^2 + y^2 ); eg: sqrt( 145^2 + 130^2 ) = 194.74
const double RmPioneerController::ThetaToSonar[] = { 318.12, 328.13, 340.02, 354.05, 5.95, 19.98, 31.87, 
	41.88, 138.12, 148.13, 160.02, 174.05, 185.95, 199.98, 211.87, 221.88 }; // in degrees
	// Calculated using DistToSonar, and the new x_offset and y_offset
	// Quadrant 4 = ASIN(x/dist)+360
	// Quadrant 1 = ASIN(x/dist)
	// Quadrant 2 = ACOS(y/dist)
	// Quadrant 3 = Quadrant 1 + 180
	// eg: for Sonar 0, in quadrant 4: th = asin(-130/194.74) + 360 = -41.88 + 360 = 318.12
const int RmPioneerController::SonarTheta[] = { 270, 310, 330, 350, 10, 30, 50, 90,
	90, 130, 150, 170, 190, 210, 230, 270 };
	// Obtained using figure of sonar array angles in Pioneer Operations Manual:
	// { -90, -50, -30, -10, 10, 30, 50, 90, 90, 130, 150, 170, -170, -150, -130, -90 }
const int RmPioneerController::SonarRange = 2999;
const int RmPioneerController::NumSonars = NUM_SONARS;


RmPioneerController::RmPioneerController( bool wander, bool asynch, 
	std::vector<RmActionHandler*> *actionHandlers, int priority ) : m_connector(NULL)
{
	if ( (m_connectionStatus = initRobot( wander, actionHandlers, priority )) != FAILED )
	{
		if ( asynch ) m_robot.runAsync( true );
		else m_robot.run( true );
	}
}


RmPioneerController::~RmPioneerController()
{
	Aria::shutdown(); 
}


RmPioneerController::ConnectionStatus RmPioneerController::initRobot( bool wander, 
	std::vector<RmActionHandler*> *actionHandlers, int priority )
{
	// Based on Aria 2.1-1 examples/teleop.cpp

	int argc = 1;
	char *argv[] = { "", "" };
	m_connector = new ArSimpleConnector( &argc, argv );
		
	// mandatory init
	Aria::init();
	
	// add the sonar to the robot
	m_robot.addRangeDevice(&m_sonar);
	
	// try to connect, if we fail exit
	if ( !m_connector->connectRobot( &m_robot ) )
	{
		printf("Could not connect to robot... exiting\n");
		Aria::shutdown();
		return FAILED;
	}
	
	// set the robots maximum velocity (sonar don't work at all well if you're
	// going faster)
	m_robot.setAbsoluteMaxTransVel( 400 );
	
	// enable the motors, disable amigobot sounds
	m_robot.comInt( ArCommands::ENABLE, 1 );
	
	// add the cations, put the limiters on top, then have the action,
	// this will keep the action from being able to drive too fast and hit
	// something
	bool avoid = false;
	if ( avoid )
	{
		m_robot.addAction( &m_recover, 100 );
		m_robot.addAction( &m_bumpers, 75 );
		m_robot.addAction( &m_avoidFront, 50 );
		m_robot.addAction( &m_avoidSide, 55 );
	}

	if ( actionHandlers != NULL ) {
		std::vector<RmActionHandler*>::iterator it( actionHandlers->begin() );
		while ( it != actionHandlers->end() ) {
			m_robot.addAction( new RmActionHook( *it ), priority );
			++it;
		}
	}

	setDriveMode( wander ? WANDER : KEYDRIVE );

	return SUCCESS;
}


RmUtility::Pose RmPioneerController::pose()
{	
	return pose( m_robot.getPose() );
}


void RmPioneerController::setDriveMode( DriveMode mode )
{
	if ( mode == WANDER ) {
		m_robot.remAction( &m_keydriveAction );
		m_robot.addAction( &m_constantVelocity, 25 );
		std::cout << "Wandering\n";
	}
	else {
		m_robot.remAction( &m_constantVelocity );
		m_robot.addAction( &m_keydriveAction, 45 );
		std::cout << "Keydrive\n";
	}
}


RmUtility::Pose RmPioneerController::pose( ArPose& arPose )
{
	double th = polarTheta( arPose.getTh() );
	assert( th >= 0 && th < 360 );
	return RmUtility::Pose( -arPose.getY(), arPose.getX(), th );
}


RmUtility::MappedSonarReading RmPioneerController::rangeReading( const RmUtility::SonarReading& reading )
{
	// Sonar pose
	// Location of robot is relative to its center point and heading
	double sth = reading.robotPose.theta + ThetaToSonar[reading.sonarNumber];
	if ( sth >= 360 ) sth -= 360;
	RmUtility::Coord sonarCoord( 
		reading.robotPose.coord.mappedTo( sth, DistToSonar[reading.sonarNumber] ) );
	RmUtility::Pose sonarPose( sonarCoord, sth );

	// Object coord
	// Location of the sensed object is relative to the sonar location and robot heading
	RmUtility::Coord objectCoord( sonarPose.coord.mappedTo( reading.robotPose.theta + 
		SonarTheta[reading.sonarNumber], reading.distance ) );

	return RmUtility::MappedSonarReading( reading, sonarPose, objectCoord );
}


double RmPioneerController::polarTheta( double arTheta )
{
	double theta = arTheta < 0 ? arTheta * -1 : arTheta == 0 ? 0 : 360 - arTheta;
	if ( theta >= 360 ) theta -= 360;

	return theta;
}
