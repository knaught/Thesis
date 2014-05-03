// RmSonarMapper.cpp


#include <iostream>
#include <cstdio>
#include <string>
#include <assert.h>

#include "RmSonarMapper.h"
using namespace RmUtility;

void RmSonarMapper::handleAction( ArRobot* robot )
{
	SonarReading readings( robot );
	mapReadings( readings );
	saveReadings( robot->getPose(), readings );
}


std::string RmSonarMapper::mapReading( SonarReading *reading, const int sonarNumber )
{
	// Method is called once for each individual reading
	// sonarNumber and distance specify the specific reading within the sweep

	// Buffer entire reading sweep on first call until an update is triggered
	// When an update is triggered, return one representative reading per call

	// Note the passed reading is used to test for update...not to make an update
	// It is added to the current or new collection after the test/update

	assert( sonarNumber >= 0 && sonarNumber <= RmPioneerController::NumSonars );

	static std::vector<SonarReading> collection_;
	static SonarReading *collectionReading_ = NULL;

	// Reset state and return if no sonar reading provided
	if ( reading == NULL ) {
		collection_.clear();
		collectionReading_ = NULL;
		return "";
	}

	// Make requested reading active
	reading->sonarNumber = sonarNumber;
	reading->distance = reading->all[sonarNumber];

	// Test for max distance/turn each time first sonar in sweep is being processed
	if ( reading->sonarNumber == 0 ) 
	{
		switch ( updateTriggered( reading->robotPose ) )
		{
			case Turn:
				collectionReading_ = &collection_.back();
				collection_.clear();
				break;

			case Distance:
				collectionReading_ = readingFrom( collection_ );
				collection_.clear();
				break;

			case None:
				collectionReading_ = NULL;
				break;
		}

		collection_.push_back( *reading );
	}

	return collectionReading_ == NULL ? "" : updateUsing( collectionReading_, reading->sonarNumber );
}


bool RmSonarMapper::mapReadings( SonarReading &readings )
{
	// Note the passed reading is used to test for update...not to make an update
	// It is added to the current or new collection after the test/update

	static std::vector<SonarReading> collection_;
	bool update;

	switch ( updateTriggered( readings.robotPose ) ) 
	{
		case Turn:
			updateUsing( &collection_.back() );
			collection_.clear();
			update = true;
			break;

		case Distance:
			updateUsing( readingFrom( collection_ ) );
			collection_.clear();
			update = true;
			break;

		case None:
			update = false;
			break;
	}

	collection_.push_back( readings );

	return update;
}


void RmSonarMapper::saveReadings( const ArPose &arPose, const SonarReading &readings )
{
	if ( m_sonarOut == NULL ) return;

	static char buff_[3*4 + 1];
	const Pose pose( arPose.getX(), arPose.getY(), arPose.getTh() );
	sprintf( buff_, "%d %d %.6f ", pose.coord.x, pose.coord.y, pose.theta );
		// for backward compatability with thesis data,
		// must retain the assumption that the data file contains raw Aria pose info
	std::string readingString( buff_ );

	for ( int i = 0; i < RmPioneerController::NumSonars; ++i )
	{
		sprintf( buff_, "%d ", readings.all[i] );
		readingString += buff_;
	}
	readingString += "\n";

	*m_sonarOut << readingString;
}


RmSonarMapper::UpdateType RmSonarMapper::updateTriggered( const RmUtility::Pose &pose )
{
	// Update/test distance of travel and degree of turn
	static int startX_ = 0; // trigger update on first run
	static int startY_ = 0;
	static double startTh_ = 0.0;

	const bool xMax = abs( pose.coord.x - startX_ ) >= m_settings.MaxCollectionDistance;
	const bool yMax = abs( pose.coord.y - startY_ ) >= m_settings.MaxCollectionDistance;
	double deltaTh = fabs( pose.theta - startTh_ );
	if ( deltaTh > 180.0 ) deltaTh = fabs( deltaTh - 360.0 );
	const bool thMax = deltaTh >= m_settings.MaxCollectionDegrees;

	UpdateType update = None;
	if ( xMax || yMax ) update = Distance;
	if ( thMax ) update = Turn; // takes precedence over distance

	if ( update != None )
	{
		startX_ = pose.coord.x / m_settings.MaxCollectionDistance * m_settings.MaxCollectionDistance;
		startY_ = pose.coord.y / m_settings.MaxCollectionDistance * m_settings.MaxCollectionDistance;
		startTh_ = pose.theta / m_settings.MaxCollectionDegrees * m_settings.MaxCollectionDegrees;
	}

	return update;
}


SonarReading* RmSonarMapper::readingFrom( std::vector<SonarReading> &collection )
{
	// NOTE: updates triggered by degree of turn should not be convolving multiple readings

	if ( collection.size() == 0 ) {
		std::cerr << "RmSonarMapper::readingFrom() : collection.size() == 0\n";
		return NULL;
	}

	// Init return value
	static SonarReading reading_; // static so can return pointer without using new
	reading_.robotPose = collection.back().robotPose;
	for ( int i = 0; i < RmPioneerController::NumSonars; ++i ) {
		reading_.all[i] = RmPioneerController::SonarRange + 1;
	}

	// Update return value to shortest readings in collection
	std::vector<SonarReading>::const_iterator ri; // map iterator
	for ( ri = collection.begin(); ri != collection.end(); ++ri ) {
		for ( int i = 0; i < RmPioneerController::NumSonars; ++i ) {
			if ( reading_.all[i] > ri->all[i] ) {
				reading_.all[i] = ri->all[i];
			}
		}
	}

	return &reading_;
}


std::string RmSonarMapper::updateUsing( SonarReading *reading, const int sonarNumber )
{
	if ( reading == NULL || sonarNumber < -1 || sonarNumber > RmPioneerController::NumSonars ) {
		return "";
	}

	std::string viewerString;

	// If sonarNumber is -1, loop over all sonars 
	int i = sonarNumber == -1 ? 0 : sonarNumber; 
	int j = sonarNumber == -1 ? RmPioneerController::NumSonars : sonarNumber + 1;
	while ( i < j )
	{
		reading->sonarNumber = i;
		reading->distance = reading->all[i];

		viewerString = m_bayesianGrid->update( *reading );

		if ( m_remoteViewServer && viewerString.length() > 0 ) {
			m_remoteViewServer->sendClientReply( viewerString );
		}

		++i;
	}

	return viewerString;
}