// RmJavaGridModel.cpp

#pragma warning( disable : 4786 )

#include <jni.h>
#include "RmJavaGridModel.h"

#include <fstream>
#include <iostream>
#include "RmSonarMapper.h"
#include "RmSettings.h"
#include "RmUtility.h"
#include "RmPioneerController.h"
#include "RmExceptions.h"
#include "RmGlobalMap.h"

static std::ifstream g_ifStream;
static std::ofstream g_ofStream;
static int g_dataSource;
static int g_sonarNumber = 0;

RmSettings g_settings;
RmGlobalMap g_grid( &g_settings );
std::vector<RmActionHandler*> g_actionHandlers;
RmSonarMapper g_sonarMapper( g_settings, &g_grid );


struct Listener
{
	jobject object;
	jmethodID methodID;
};

static JNIEnv* s_env = NULL;
static Listener* s_listener = NULL;
ArRobot* g_robot;


void fireMapUpdateEvent( std::string eventString )
{
	if ( s_env != NULL && s_listener != NULL ) {
		jstring jstr = s_env->NewStringUTF( eventString.c_str() );
		s_env->CallVoidMethod( s_listener->object, s_listener->methodID, jstr );
	}
}


JNIEXPORT void JNICALL
Java_GridModel_setMapUpdateListener( JNIEnv *env, jobject obj, jobject listener )
{
	s_env = env;
	jclass cls = env->GetObjectClass( listener );
	jmethodID mid = env->GetMethodID( cls, "mapUpdateFired", "(Ljava/lang/String;)V" );
	if ( mid != NULL ) 
	{
		// Install listener
		s_listener = new Listener();
		s_listener->object = obj;
		s_listener->methodID = mid;
	}
	else {
		printf( "Java_GridModel_setMapUpdateListener(): Unable to install listener.\n" );
	}
}


JNIEXPORT jint JNICALL
Java_GridModel_openLiveConnection( JNIEnv *env, jobject obj, jstring filename, jboolean wander )
{
	const char *cfilename = env->GetStringUTFChars( filename, 0 );

	try {
		// Open log file for write
		g_ofStream.open( cfilename );
		if ( !g_ofStream.good() ) {
			throw RmExceptions::IOException( "Java_GridModel_openLiveConnection()", 
				"Unable to open sonar output file." );
		}

		RmSonarMapper* sonarRecorder = new RmSonarMapper( g_settings, g_ofStream, &g_grid );
		g_actionHandlers.empty();
		g_actionHandlers.push_back( sonarRecorder );
		RmPioneerController robot( false, true, &g_actionHandlers );
		if ( robot.getConnectionStatus() == RmPioneerController::FAILED ) {
			throw RmExceptions::Exception( NULL, "Java_GridModel_openLiveConnection()", 
				"Unable to establish connection with simulator or robot." );
		}
	}
	catch( RmExceptions::Exception e ) {
		env->ReleaseStringUTFChars( filename, cfilename );
		std::cerr << e << "\n";
		return 1;
	}

	return 0;
}


JNIEXPORT jint JNICALL  // openConnection( const char* filename )
Java_GridModel_openFileConnection( JNIEnv *env, jobject obj, jstring filename )
{
	const char *cfilename = env->GetStringUTFChars( filename, 0 );
	try {
		if ( g_ifStream.is_open() ) g_ifStream.close();
		g_ifStream.open( cfilename );
		if ( !g_ifStream.good() ) throw RmExceptions::IOException( 
			"Java_GridModel_openFileConnection()", "Unable to open input file." );
	}
	catch( RmExceptions::Exception e ) {
		env->ReleaseStringUTFChars( filename, cfilename );
		std::cerr << e << "\n";
		return 1;
	}
	
	return 0;
}


JNIEXPORT jint JNICALL 
Java_GridModel_resetFileConnection( JNIEnv *env, jobject obj )
{
	try {
		g_ifStream.clear();
		if ( !g_ifStream.is_open() ) throw RmExceptions::IOException( 
			"Java_GridModel_resetFileConnection()", "Connection not open." );
		g_ifStream.seekg( 0 );
		if ( !g_ifStream.good() ) throw RmExceptions::IOException( 
			"Java_GridModel_resetFileConnection()", "Connection reset failed." );
	}
	catch( RmExceptions::Exception e ) {
		std::cerr << e << "\n";
		return 1;
	}

	g_grid.empty();
	g_sonarNumber = 0;

	return 0;
}


JNIEXPORT void JNICALL
Java_GridModel_clearMap( JNIEnv *env, jobject obj )
{
	g_grid.clear();
}


JNIEXPORT void JNICALL
Java_GridModel_emptyMap( JNIEnv *env, jobject obj )
{
	g_grid.empty();
	g_sonarMapper.reset();
}


JNIEXPORT void JNICALL
Java_GridModel_runMapper( JNIEnv *env, jobject obj )
{
	if ( !g_ofStream.is_open() ) {
		std::cerr << "Java_GridModel_runMapper(): Connection not open.\n";
		return;
	}

	// Start 'er up!
	if ( g_robot ) {
		g_robot->setVel( 40.0 );
		g_robot->runAsync( true );
	}
}


/**
 * Retrieves the first non-commented line of specified length from the given stream
 * into the given buffer.
 */
std::ifstream &getLine( std::ifstream &is, char *buffer, int size )
{
	do if ( !is.getline( buffer, size ) ) break;
	while ( buffer[0] == '%' ); // skip comments

	return is;
}


/**
 * Returns the first non-commented line of <i>unspecified</i> length from the given stream.
 * Terminating newline is not included.
 * This function supports unstructured data, such as camera data, where
 * a line can be of any length.
 */
std::string getLine( std::ifstream &is )
{
	const int BufSize = 120;
	char buff[BufSize];
	std::string line;

	// get next non-comment line
	do 
	{
		is.getline( buff, BufSize );

		// get one newline-terminated line of unspecified length
		while ( !is.eof() ) 
		{
			line.append( buff );

			// bolt if we've read end of line (vice hitting buffer size)
			if ( is.gcount() < BufSize - 2 ) break;

			// for gcount() of 118, 119, or 120, newline is next character
			if ( is.peek() == '\n' ) break;

			// flags are raised when BufSize is reached; ignore them
			is.clear();

			is.getline( buff, BufSize, '\n' );
		}

	} while ( !is.eof() && line[0] == '%' );

	return line;
}


/**
 * Returns log string for next in-range sonar reading, or null if end of file is reached.
 * Also returns null if no valid connection exists.
 */
JNIEXPORT jstring JNICALL
Java_GridModel_stepSonarMapper( JNIEnv *env, jobject obj )
{
	// This routine statically stores a single line of sonar data
	// and then iterates through the range readings each time it is called.

	if ( !g_ifStream.is_open() ) {
		std::cerr << "Java_GridModel_stepMapper(): Connection not open.\n";
		return NULL;
	}

	static RmUtility::SonarReading reading;

	// Repeat until in-range sonar reading is found or end of file is hit
	std::string dataString;
	do {
		// If looking for first sonar reading, process a new line of data
		if ( g_sonarNumber == 0 ) 
		{
			char buffer[120];
			if ( !getLine( g_ifStream, buffer, 120 ) ) return NULL;

			// Extract SonarReading
			reading = RmUtility::SonarReading( buffer );
		}

		// Map single sonar reading
		try {
			dataString = g_sonarMapper.mapReading( &reading, g_sonarNumber );
		} catch ( RmExceptions::Exception e ) {
			std::cerr << "Exception caught in Java_GridModel_stepSonarMapper()\n" << e << "\n";
			throw; // force JVM to crash
		}

		// Move to the next range reading
		if ( ++g_sonarNumber > 15 ) g_sonarNumber = 0;
	} while ( dataString.length() == 0 );

	return env->NewStringUTF( dataString.c_str() );
}


/**
 * Returns log string for next in-range camera reading, or null if end of file is reached.
 * Also returns null if no valid connection exists.
 * Each object detected is treated analagously to a single sonar device.
 */
JNIEXPORT jstring JNICALL
Java_GridModel_stepLogMapper( JNIEnv *env, jobject obj )
{
	if ( !g_ifStream.is_open() ) {
		std::cerr << "Java_GridModel_stepLogMapper(): Connection not open.\n";
		return NULL;
	}

	// Read next log data entry from file
	std::string dataString;
	for ( int i = 0; g_ifStream && i < 3; ++i ) {
		dataString.append( getLine( g_ifStream ) + "\n" );
	}

	return i != 3 ? NULL : env->NewStringUTF( dataString.c_str() );
}


/**
 * No longer supported (10 Feb 05)
 * Returns series of 3-line log entries resulting from localization, each separated by a blank line
 * (which can be split on "\n\n")
 */
JNIEXPORT jstring JNICALL
Java_GridModel_localize( JNIEnv *env, jobject obj )
{
	return env->NewStringUTF( "" );
}


/**
 * No longer supported (10 Feb 05)
 * Returns series of coordinate-value pairs ("-xxx -yyy 0.dddd;") resulting from integration of all 
 * local maps
 */
JNIEXPORT jstring JNICALL
Java_GridModel_integrate( JNIEnv *env, jobject obj )
{
	return env->NewStringUTF( "" );
}



JNIEXPORT jint JNICALL
Java_GridModel_saveMap( JNIEnv *env, jobject obj, jstring filename )
{
	const char *cfilename = env->GetStringUTFChars( filename, 0 );
	try {
		if ( *cfilename == 0 ) throw RmExceptions::IOException( 
			"Java_GridModel_saveMap()", "Filename unspecified." );
		std::ofstream fout( cfilename );
		if( !fout ) throw RmExceptions::IOException( 
			"Java_GridModel_saveMap()", "Unable to open output file." );
		g_grid.put( fout );
		fout.close();
	}
	catch( RmExceptions::Exception e )	{
		env->ReleaseStringUTFChars( filename, cfilename );
		std::cerr << e << "\n";
		return 1;
	}

	return 0;
}


JNIEXPORT void JNICALL
Java_GridModel_setSonarModel( JNIEnv *env, jobject obj, jint model )
{
	g_settings.SonarModel = (RmUtility::SonarModelEnum)model;
}


JNIEXPORT jint JNICALL
Java_GridModel_getSonarModel( JNIEnv *env, jobject obj )
{
	return g_settings.SonarModel;
}


JNIEXPORT void JNICALL
Java_GridModel_setSonarEnabled( JNIEnv *env, jobject obj, jint sonarNumber, jboolean enabled)
{
	g_settings.EnabledSonars[sonarNumber] = enabled;
}


JNIEXPORT jint JNICALL
Java_GridModel_setCellSize( JNIEnv *env, jobject obj, jint v )
{
	return ( v < 1 || v > 500 ) ? -1 : g_settings.CellSize = v;
}


JNIEXPORT jint JNICALL
Java_GridModel_getCellSize( JNIEnv *env, jobject obj )
{
	return g_settings.CellSize;
}


JNIEXPORT jint JNICALL
Java_GridModel_setSonarBeta( JNIEnv *env, jobject obj, jint v )
{
	return ( v < 7 || v > 90 ) ? -1 : g_settings.Beta = v;
}


JNIEXPORT jint JNICALL
Java_GridModel_getSonarBeta( JNIEnv *env, jobject obj )
{
	return g_settings.Beta;
}


JNIEXPORT jfloat JNICALL
Java_GridModel_setMaxOccupied( JNIEnv *env, jobject obj, jfloat v )
{
	return ( v < 0 || v > 1 ) ? -v : g_settings.MaxOccupied = v;
}


JNIEXPORT jfloat JNICALL
Java_GridModel_getMaxOccupied( JNIEnv *env, jobject obj )
{
	return g_settings.MaxOccupied;
}


JNIEXPORT jfloat JNICALL
Java_GridModel_setMaxOccupiedII( JNIEnv *env, jobject obj, jfloat v )
{
	return ( v < 0 || v > 1 ) ? -v : g_settings.MaxEmpty = v;
}


JNIEXPORT jfloat JNICALL
Java_GridModel_getMaxOccupiedII( JNIEnv *env, jobject obj )
{
	return g_settings.MaxEmpty;
}


JNIEXPORT jint JNICALL
Java_GridModel_setRegionIWidth( JNIEnv *env, jobject obj, jint v )
{
	return ( v < 1 || v > 1000 ) ? -v : g_settings.RegionIHalfwidth = v / 2;
}


JNIEXPORT jint JNICALL
Java_GridModel_getRegionIWidth( JNIEnv *env, jobject obj )
{
	return g_settings.RegionIHalfwidth * 2;
}


JNIEXPORT void JNICALL
Java_GridModel_setIgnoreOutOfRange( JNIEnv *env, jobject obj, jboolean v )
{
	g_settings.IgnoreOutOfRange = v;
}


JNIEXPORT jboolean JNICALL
Java_GridModel_getIgnoreOutOfRange( JNIEnv *env, jobject obj )
{
	return g_settings.IgnoreOutOfRange;
}


JNIEXPORT jint JNICALL
Java_GridModel_setOutOfRangeConversion( JNIEnv *env, jobject obj, jint v )
{
	std::cerr << "Java_GridModel_setOutOfRangeConversion(): " << v << "\n";
	return ( v < 1 || v > RmPioneerController::SonarRange ) ? -v : g_settings.OutOfRangeConversion = v;
}


JNIEXPORT jint JNICALL
Java_GridModel_getOutOfRangeConversion( JNIEnv *env, jobject obj )
{
	return g_settings.OutOfRangeConversion;
}


JNIEXPORT void JNICALL
Java_GridModel_setIgnoreObstructed( JNIEnv *env, jobject obj, jboolean v )
{
	g_settings.IgnoreObstructed = v;
}


JNIEXPORT jboolean JNICALL
Java_GridModel_getIgnoreObstructed( JNIEnv *env, jobject obj )
{
	return g_settings.IgnoreObstructed;
}


JNIEXPORT jint JNICALL
Java_GridModel_getLocalMapSize( JNIEnv *env, jobject obj )
{
	return g_settings.LocalMapDistance;
}


JNIEXPORT jint JNICALL
Java_GridModel_setLocalMapSize( JNIEnv *env, jobject obj, jint v )
{
	return ( v != 0 && (v < 100 || v > 25000) ) ? -v : g_settings.LocalMapDistance = v;
}


JNIEXPORT jboolean JNICALL
Java_GridModel_getLocalizationEnabled( JNIEnv *env, jobject obj )
{
	return g_settings.Localize;
}


JNIEXPORT void JNICALL
Java_GridModel_setLocalizationEnabled( JNIEnv *env, jobject obj, jboolean v )
{
	g_settings.Localize = v;
}


JNIEXPORT jfloat JNICALL
Java_GridModel_getObstructedCertainty( JNIEnv *env, jobject obj )
{
	return g_settings.ObstructedCertainty;
}


JNIEXPORT jfloat JNICALL
Java_GridModel_setObstructedCertainty( JNIEnv *env, jobject obj, jfloat v )
{
	return ( v < 0 || v > 1 ) ? -v : g_settings.ObstructedCertainty = v;
}


JNIEXPORT jint JNICALL
Java_GridModel_getMotionMinHeight( JNIEnv *env, jobject obj )
{
	return g_settings.MotionModel.MinHeight;
}


JNIEXPORT jint JNICALL
Java_GridModel_setMotionMinHeight( JNIEnv *env, jobject obj, jint v )
{
	return ( v < 0 || v > 50 ) ? -v : g_settings.MotionModel.MinHeight = v;
}


JNIEXPORT jint JNICALL
Java_GridModel_getMotionMinWidth( JNIEnv *env, jobject obj )
{
	return g_settings.MotionModel.MinWidth;
}


JNIEXPORT jint JNICALL
Java_GridModel_setMotionMinWidth( JNIEnv *env, jobject obj, jint v )
{
	return ( v < 0 || v > 50 ) ? -v : g_settings.MotionModel.MinWidth = v;
}


JNIEXPORT jint JNICALL
Java_GridModel_getMotionUnitDistance( JNIEnv *env, jobject obj )
{
	return g_settings.MotionModel.UnitDistance;
}


JNIEXPORT jint JNICALL
Java_GridModel_setMotionUnitDistance( JNIEnv *env, jobject obj, jint v )
{
	return ( v < 1 || v > 1000 ) ? -v : g_settings.MotionModel.UnitDistance = v;
}


JNIEXPORT jint JNICALL
Java_GridModel_getMotionUnitTurn( JNIEnv *env, jobject obj )
{
	return g_settings.MotionModel.UnitTurn;
}


JNIEXPORT jint JNICALL
Java_GridModel_setMotionUnitTurn( JNIEnv *env, jobject obj, jint v )
{
	return ( v < 1 || v > 360 ) ? -v : g_settings.MotionModel.UnitTurn = v;
}


JNIEXPORT jfloat JNICALL
Java_GridModel_getMotionGaussSigma( JNIEnv *env, jobject obj )
{
	return g_settings.MotionModel.GaussianSigma;
}


JNIEXPORT jfloat JNICALL
Java_GridModel_setMotionGaussSigma( JNIEnv *env, jobject obj, jfloat v )
{
	return ( v < 0 || v > 100 ) ? -v : g_settings.MotionModel.GaussianSigma = v;
}


JNIEXPORT jfloat JNICALL
Java_GridModel_getMotionBendFactor( JNIEnv *env, jobject obj )
{
	return g_settings.MotionModel.BendFactor;
}


JNIEXPORT jfloat JNICALL
Java_GridModel_setMotionBendFactor( JNIEnv *env, jobject obj, jfloat v )
{
	return ( v < 0 || v > 100 ) ? -v : g_settings.MotionModel.BendFactor = v;
}


JNIEXPORT void JNICALL
Java_GridModel_saveSettings( JNIEnv *env, jobject obj )
{
	g_settings.write();
}


