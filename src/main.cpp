// main.cpp
// The stand-alone app that maps live or pre-recorded sonar data;
// The server side of the client-server application that allows for remote control of robot
// and mapping functionality.


// Disable warning C4786: "identifier was truncated to '255' characters in the debug information
// while compiling class-template member function"
#pragma warning( disable : 4786 )


//////


#include <fstream>
#include <iostream>
#include <string>
#include <string>
#include <ctype.h>

#include "RmSonarMapper.h"
#include "RmGlobalMap.h"
#include "RmUtilityExt.h"
#include "RmExceptions.h"
#include "RmPioneerController.h"
#include "RmServer.h"

using namespace RmUtility;


//////


void mapFromFile( RmSettings &settings, std::ifstream& sonarStream, RmGlobalMap& grid );
void mapFromRobot( RmSettings &settings, std::ofstream &sonarStream, std::string sonarStreamName, 
	RmSonarMap &grid, int remotePort, bool wander );
std::string newLogFile( std::ofstream &sonarStream, std::string sonarStreamName, bool reset = false );


//////


const std::string DataPath( "../data/" );


//////


/**
 * Provides console interface to robot mapping functionality encapsulated within the RmMapper API.
 * Supports direct-connect or wireless simulated or real-world operation and real-time map
 * viewing.
 * Command line arguments allow for specification of
 * <ul>
 * <li>Sonar data input file (pre-recorded)
 * <li>Sonar data output file (live)
 * <li>Grid map output file
 * <li>Robot drive mode (keyboard or wander)
 * <li>Sonar mode (Point of Return, Acoustic Axis, or Field of View)
 * <li>Localization enabled or disabled
 * </ul>
 * Execute this application without any arguments to get specific usage information.
 */
int main( int argc, char* argv[] )
{
	//////
	// Get command line arguments

	struct InvalidUsageException {
		char* message;
		InvalidUsageException( char* message_ ) : message(message_) {}
	};

	char errMsg[50];

	RmSettings settings;
	bool prerecorded = true;
	bool overwrite = false;
	bool wander = false;
	int remotePort = 0;

	try {
		if ( argc < 3 ) {
			throw InvalidUsageException( "Wrong number of arguments." );
		}

		// Process command line switches
		for ( int i = 1; i < argc; i += 2 ) 
		{
			// Sonar input filename (for processing pre-recorded sonar data)
			if ( strcmp( argv[i], "-si" ) == 0 ) {
				if ( i+1 == argc ) throw InvalidUsageException( "Wrong number of arguments." );
				settings.SonarName = argv[i+1];
				prerecorded = true;
			}

			// Sonar output filename, without overwrite
			else if ( strcmp( argv[i], "-so" ) == 0 ) {
				if ( i+1 == argc ) throw InvalidUsageException( "Wrong number of arguments." );
				settings.SonarName = argv[i+1];
				prerecorded = false;
				overwrite = false;
			}

			// Sonar output filename, with overwrite
			else if ( strcmp( argv[i], "-soo" ) == 0 ) {
				if ( i+1 == argc ) throw InvalidUsageException( "Wrong number of arguments." );
				settings.SonarName = argv[i+1];
				prerecorded = false;
				overwrite = true;
			}

			// Wander mode
			else if ( strcmp( argv[i], "-w" ) == 0 ) {
				wander = true;
			}

			// Remote mode (via wireless UDP connection)
			else if ( strcmp( argv[i], "-r" ) == 0 ) {
				remotePort = atoi( argv[i+1] );
				if ( remotePort <= 0 ) {
					throw InvalidUsageException( "Invalid port specification" );
				}
			}

			// Output filename for grid map
			else if ( strcmp( argv[i], "-g" ) == 0 ) {
				if ( i+1 == argc ) throw InvalidUsageException( "Wrong number of arguments." );
				settings.GridName = argv[i+1];
			}

			// Localize "on" or "off"
			else if ( strcmp( argv[i], "-l" ) == 0 ) {
				if ( i+1 == argc ) throw InvalidUsageException( "Wrong number of arguments." );
				if ( strcmp( argv[i+1], "off" ) == 0 ) settings.Localize = false;
				else if ( strcmp( argv[i+1], "on" ) == 0 ) settings.Localize = true;
			}

			// Sonar model "cell", "axis", or "cone" (point of return, acoustic axis, field of view)
			else if ( strcmp( argv[i], "-m" ) == 0 ) {
				if ( i+1 == argc ) throw InvalidUsageException( "Wrong number of arguments." );
				if ( strcmp( argv[i+1], "cell" ) == 0 ) settings.SonarModel = RmUtility::SingleCell;
				else if ( strcmp( argv[i+1], "axis" ) == 0 ) settings.SonarModel = 
					RmUtility::AcousticAxis;
				else if ( strcmp( argv[i+1], "cone" ) == 0 ) settings.SonarModel = RmUtility::Cone;
			}

			// Unidentified switch
			else {
				sprintf( errMsg, "Invalid switch: %s", argv[i] );
				throw InvalidUsageException( errMsg );
			}
		}

		if ( settings.SonarName == "" ) 
			throw InvalidUsageException( "Missing required switch -si or -so." );
	}
	catch( InvalidUsageException &iue ) {
		std::cout << "\n" << iue.message << "\n";
		std::cout << "Usage:  " << argv[0] << " -si|-so|-soo sonarLogName " <<
			"[-w{ander} -g gridLogName -l{ocalization} on|off -m cell|axis|cone]\n";
		return 1;
	}

	// Update settings file based on command line switches
	settings.write();


	//////
	// Build the map

	RmGlobalMap map( &settings );

	try {

		std::string sonarName( settings.SonarName );
		sonarName.append( ".sd" );

		//////
		// Map from file

		if ( prerecorded ) 
		{
			std::ifstream sonarInStream( sonarName.c_str() );
			if ( sonarInStream.fail() ) {
				throw RmExceptions::IOException( "main()", "Unable to read sonar data file" );
			}
			std::cout << "Mapping sonar data from " << sonarName;
			if ( settings.Localize ) std::cout << " with localization";
			std::cout << "...\n";
			clock_t start = clock();
			mapFromFile( settings, sonarInStream, map );
			std::cout << clock() - start << "\n";
		}


		//////
		// Map from robot

		else 
		{
			// Use ifstream to test for file existence
			std::ifstream ifs( sonarName.c_str() );
			if ( ifs && !overwrite ) {
				std::cout << "The file " << sonarName << " already exists.  Overwrite (y/n)? ";
				char c;
				std::cin >> c;
				if ( tolower( c ) == 'n' ) return 0;
			}
			ifs.close();

			std::cout << "Mapping sonar data from live connection";
			if ( settings.Localize ) std::cout << " with localization";
			std::cout << "...\n";
			std::ofstream sonarOutStream;

			mapFromRobot( settings, sonarOutStream, settings.SonarName, map, remotePort, wander );
		}

		
		//////
		// Save map

		if ( settings.GridName != "" ) 
		{
			std::cout << "Saving global map to " << settings.GridName << ".gd\n";
			std::string gridName( settings.GridName );
			gridName.append( ".gd" );
			static_cast<RmMutableCartesianGrid<float> >(map).put( gridName.c_str(), 4 );
				// using put() rather than operator<<() in order to specify precision
				// cast required even though RmGlobalMap is an RmMutableCartesianGrid
		}

		std::cout << "\n";

		map.empty();
	}
	catch( RmExceptions::Exception e ) {
		std::cout << "\n" << e << "\n";
	}
	catch( ... ) {
		std::cerr << "Uncaught Exception in main().\n";
		throw;
	}

	return 0;
}


/**
 * Builds an occupancy grid using preexisting sonar data in the given file.
 * @param settings the settings to be passed on to the RmSonarMapper
 * @param sonarStream the input sonar data file, already opened for read
 * @param grid the target occupancy grid
 */
void mapFromFile( RmSettings &settings, std::ifstream& sonarStream, RmGlobalMap& grid )
{
	char buffer[300];
	RmSonarMapper sonarMapper( settings, &grid );

	while( sonarStream.getline( buffer, 300 ) )
	{
		// Skip comments
		if ( buffer[0] == '%' ) continue;

		// Map entire sonar sweep
		sonarMapper.mapReadings( SonarReading( buffer ) );
	}

	grid.finalize();
}


/**
 * Builds an occupancy grid using live data taken from the real or simulated robot.
 * Supports wireless UDP server operation, receiving commands from a client on port 2000
 * and dispatching to the robot or internally as appropriate.
 * Wireless commands that may be received from the client include those that:
 * <ul>
 * <li>Put the robot in manual keydrive or automatic wander mode
 * <li>Move the robot forward, backward, left, right, or stop its motion altogether
 * <li>Turn the robot off and on, creating a new sonar data file each time its turned on
 * <li>Close and create a new sonar data file
 * <li>Connect to or disconnect from the remote viewer
 * <li>Quit the application
 * </ul>
 * Also supports a remote map viewer connection on port 2100 that allows for live graphical mapping
 * of the robot's environment.
 * @param settings the settings to be passed on to the RmSonarMapper
 * @param sonarStream the output sonar data file, not yet opened for write
 * @param sonarStreamName the path and filename, without extension, of the sonar data file to create
 * @param grid the target occupancy grid
 * @param remotePort if positive non-zero, indicates port for wireless UDP terminal operation
 * @param wander flags keydrive or automatic wander drive
 */
void mapFromRobot( RmSettings &settings, std::ofstream  &sonarStream, std::string sonarStreamName, 
	RmSonarMap &grid, int remotePort, bool wander )
{
	settings.Localize = false; // just get sonar data; localization causes delays

	std::vector<RmActionHandler*> actionHandlers;

	RmSonarMapper sonarMapper( settings, sonarStream, &grid );
	actionHandlers.push_back( &sonarMapper );

	RmPioneerController *robot = NULL;
	ArActionKeydrive *keydriveAction = NULL;

	if ( remotePort > 0 ) 
	{
		RmServer remoteControlServer( remotePort, "Robot control server" ); // for the robot
		RmServer remoteViewServer( remotePort + 1, "Map viewer server" ); // for the Java map viewer

		// Establish communication with control client
		std::cout << "Remote client says: " << remoteControlServer.getClientString() << "\n";
		remoteControlServer.sendClientReply( "Welcome" );

		std::string cmdString;
		std::string replyString;
		bool quit = false;
		bool remoteViewer = false;
		while ( (robot == NULL || robot->isRunning()) && !quit ) 
		{
			// Get remote control command
			cmdString = remoteControlServer.getClientString();
			std::cout << cmdString << "\n";

			switch( cmdString.c_str()[0] )
			{
				case 'v': // connect to or disconnect from remote viewer
					if ( !remoteViewer ) {
						std::cout << "Awaiting handshake from Remote Control Viewer\n" ;
						std::cout << "Remote Control Viewer says: " << 
							remoteViewServer.getClientString() << "\n";
						remoteViewServer.sendClientReply( "Welcome" );
						sonarMapper.setRemoteViewServer( &remoteViewServer );
						remoteViewer = true;
						replyString = "Remote viewer connection established";
					}

					if ( cmdString.length() >= 3 ) {
						replyString = "Viewer sonar mode set to ";
						switch( cmdString.c_str()[2] )
						{
							case 'e': // cEll
								settings.SonarModel = RmUtility::SingleCell;
								replyString.append( "Single Cell" );
								break;
							case 'x': // aXis
								settings.SonarModel = RmUtility::AcousticAxis;
								replyString.append( "Acoustic Axis" );
								break;
							case 'o': // cOne
								settings.SonarModel = RmUtility::Cone;
								replyString.append( "Cone" );
								break;
							case 'l': // cLose
								replyString = 
									"Remote viewer connection closed. Rerun viewer application.";
								remoteViewer = false;
								remoteViewServer.sendClientReply( "reset" );
								break;
						}	
					}
					else replyString = "Unrecognized mode";

					remoteControlServer.sendClientReply( replyString );
					break;

				case 'f': // forward
					if ( keydriveAction ) keydriveAction->up();
					break;

				case 'b': // backward
					if ( keydriveAction ) keydriveAction->down();
					break;

				case 'l': // left
					if ( keydriveAction ) keydriveAction->left();
					break;

				case 'r': // right
					if ( keydriveAction ) keydriveAction->right();
					break;

				case 's': // stop
					if ( keydriveAction ) keydriveAction->space();
					break;

				case 'w': // wander/keydrive toggle
					if ( robot ) {
						wander = !wander;
						robot->setDriveMode( 
							wander ? RmPioneerController::WANDER : RmPioneerController::KEYDRIVE );
					}
					break;

				case 'o': // toggle robot on and off, creating new log file each time turned on
					if ( robot == NULL ) {
						std::cerr << "robot == NULL\n";
						remoteControlServer.sendClientReply( 
							newLogFile( sonarStream, sonarStreamName ) );
						robot = new RmPioneerController( wander, !wander, &actionHandlers );
						keydriveAction = robot->arKeydriveAction();
					}
					else {
						std::cerr << "robot != NULL\n";
						sonarStream.close();

						delete robot;
						robot = NULL;
						keydriveAction = NULL;

						try {
							remoteViewServer.sendClientReply( "reset" );
						}
						catch ( RmExceptions::SocketException ) {
							// Unable to communicate with remote viewer
							// Ignore
						}
					}
					break;

				case 'd': // close current log file and create new one 
					sonarStreamName = DataPath + cmdString.substr(1);
					remoteControlServer.sendClientReply( 
						newLogFile( sonarStream, sonarStreamName, true ) );
					break;

				case 'q': // quit the application
					sonarStream.close();
					quit = true;
					if ( robot ) delete robot;
					robot = NULL;
					keydriveAction = NULL;
					try {
						remoteViewServer.sendClientReply( "quit" );
					}
					catch ( RmExceptions::SocketException ) {
						// Unable to communicate with remote viewer
						// Ignore
					}
					break;
			}
 		}
	}
	else
	{
		std::cout << newLogFile( sonarStream, sonarStreamName ) << "\n";
		robot = new RmPioneerController( wander, false, &actionHandlers );
			// this blocks, handling all robot motion control, until user presses Escape
			// meanwhile, the actionHandlers are being called as part of the robot event cycle
	}

	if ( robot ) delete robot;

	return;
}


/**
 * Opens an sonar data output file of type ".sd".
 * Each call will increment a sequence number that is appended to the end of the filename.
 * The first call does not append a sequence number.
 * For example, given a sonarStreamName of "mydata", the output files 
 * will be "mydata.sd", "mydata2.sd", "mydata3.sd", and so on.
 * @param sonarStream the stream used to open the file; if a file is already open on the stream,
 * it is first closed
 * @param sonarStreamName the path and filename, without extension, of the file to create
 * @param reset resets the sequence number to 1
 */
std::string newLogFile( std::ofstream &sonarStream, std::string sonarStreamName, bool reset )
{
	static const char *dataFail = "Unable to open sonar file '%s' for write.";
	static const char *dataOkay = "Sonar file '%s' successfully opened for write.";
	static char sonarName[255];
	static int segment = 0;

	if ( reset ) segment = 0;

	if ( sonarStream.is_open() ) sonarStream.close();

	if ( ++segment == 1 ) sprintf( sonarName, "%s.sd", sonarStreamName.c_str() );
	else sprintf( sonarName, "%s%d.sd", sonarStreamName.c_str(), segment );
	sonarStream.open( sonarName );

	char buff[255];
	sprintf( buff, sonarStream ? dataOkay : dataFail, sonarName );

	return std::string( buff );
}
