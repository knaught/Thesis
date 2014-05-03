// RmLocalMap.cpp

#include <cmath>
#include "RmLocalMap.h"


const std::string RmLocalMap::update( const RmUtility::SonarReading& reading )
{
	RmUtility::Pose origin( m_globalOrigin.coord, m_settings->PreRotate ? m_globalOrigin.theta : 0.0 );
	return update( reading, origin, true );
}


const std::string RmLocalMap::update( 
	const RmUtility::SonarReading& reading, const RmUtility::Pose& pivot, bool saveHistory )
{
	// Save sonar reading
	if ( saveHistory ) m_sonarReadings.push_back( reading );

	// Get the localized robot, sonar, and object pose data (only for first sonar of a sweep)
	static RmUtility::Pose lastPose_;
	static RmUtility::Pose localPose_;
	if ( lastPose_ != reading.robotPose ) 
	{
		// If the pose of the first reading is (0,0):0 (such as when the robot starts up),
		// this won't execute, but a rotation wouldn't have any effect anyway, so it's no prob

		// Update cumulative distance traveled and degrees turned
		// (accounting for transition across due north)
		m_cumDist += reading.robotPose.coord.distanceFrom( lastPose_.coord );
		double t = abs( reading.robotPose.theta - lastPose_.theta );
		if ( t > 180 ) t = abs( t - 360 );
		m_cumTurn += t;

		localPose_ = lastPose_ = reading.robotPose;
		if ( pivot.theta != 0 ) 
		{
			// If there were no concern for efficiency, everything in this block could be
			// removed but these two commands, which is where the real work gets done:
			localPose_.coord.rotateBy( pivot.theta, pivot.coord );
			localPose_.theta += pivot.theta;
		}
	}

	// Get map of sonar reading
	// Viewer is expecting blank line as token separating output between calls to update()
	std::string map = RmBayesCertaintyGrid::update( 
		RmUtility::SonarReading( localPose_, &reading.all[0], reading.sonarNumber ) );
	if ( map.length() > 0 ) map.append( "\n" );

	return map;
}


const std::string RmLocalMap::reorientedBy( const RmUtility::Pose& shift ) const
{
	// Start with a clean slate
	RmBayesCertaintyGrid grid( m_settings );

	// Shift all robot poses from global origin and recalculate sonar poses and probabilities
	std::string logString;
	for ( std::vector<RmUtility::SonarReading>::const_iterator reading = m_sonarReadings.begin(); 
		reading != m_sonarReadings.end(); ++reading )
	{
		RmUtility::SonarReading r = *reading;
		r.robotPose += shift;
		r.robotPose.coord.rotateBy( shift.theta, m_globalOrigin.coord + shift.coord );

		std::string map = grid.update( r );
		if ( map.length() > 0 ) {
			logString.append( map + "\n" );
		}
	}

	return logString;
}


const std::string RmLocalMap::reorientBy( const RmUtility::Pose& shift )
{
	// Wipe the slate clean
	empty();

	// Shift the global origin
	m_globalOrigin += shift;

	// Shift all the robot poses and recalculate sonar poses and probabilities
	std::string logString;
	for ( std::vector<RmUtility::SonarReading>::iterator reading = m_sonarReadings.begin(); 
		reading != m_sonarReadings.end(); ++reading )
	{
		(*reading).robotPose += shift;
		(*reading).robotPose.coord.rotateBy( shift.theta, m_globalOrigin.coord );

		std::string map = RmBayesCertaintyGrid::update( *reading );
		if ( map.length() > 0 ) {
			logString.append( map + "\n" );
		}
	}

	return logString;
}
