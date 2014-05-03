// client.cpp
// The client side of the client-server application that allows for remote control of robot
// and mapping functionality.


#include <conio.h>
#include <string>
#include <iostream>
#include "RmExceptions.h"
#include "RmClient.h"


//////


/**
 * Encapsulates the elements that represent a single user command entered at the client terminal.
 */
struct Command
{
	/** Command string to be sent to server */
	std::string string;
	
	/** Indicates whether client expects a reply from the server before providing another
	    command prompt */
	bool replyExpected;

	/** Indicates whether command requests application termination */
	bool doQuit;

	Command() : string(), replyExpected(false), doQuit(false) {}
};

Command getUserCommand();


//////


/**
 * Provides a UPD/IP client for remote control of the main Mapper application
 * (see main.cpp), which in remote mode serves as the UDP/IP server.
 */
void main( int argc, const char* argv[] )
{
	//
	// Check for the host and port arguments
	//
	if (argc != 3)
	{
		std::cerr << "\nSyntax: " << argv[0] << " ServerName PortNumber\n";
		return;
	}

	short nPort = atoi(argv[2]);
	const std::string handshake( "Hello" );

	try {
		RmClient client( argv[1], nPort );
		client.sendServerString( handshake );
		const std::string reply( client.getServerReply() );
		std::cout << "Remote Control Server says: " << reply << "\n";
		
		std::cout 
			<< "\nPlease use the following keyboard controls:\n"
			<< "  On/off : 'o' and Home key\n"
			<< "  Forward, Backward, Left, Right : Arrow keys, Numkey arrow keys\n"
			<< "  Stop : Spacebary Numkey 5\n"
			<< "  Change data file : d\n"
			<< "  Quit : q\n\n> ";

		Command cmd;
		while ( !cmd.doQuit ) 
		{
			cmd = getUserCommand();
			client.sendServerString( cmd.string );
			if ( cmd.replyExpected ) 
			{
				std::cout << client.getServerReply() << "\n> ";
			}
		}
	}
	catch( RmExceptions::SocketException e ) {
		std::cerr << "\n" << argv[0] << "::main() : " << e;
		return;
	}
}


/**
 * Processes keyboard input and converts it into commands that are to be sent to the
 * mapping server.
 */
Command getUserCommand()
{
	static bool robotIsOn = false;
	char szBuf[256];
	szBuf[0] = '\0';

	Command cmd;
	bool getFilename = false;
	bool getCameraPose = false;

	while ( !kbhit() && szBuf[0] == '\0' ) 
	{
		switch( getch() ) {
			case 'v': // enable remote viewer
				sprintf( szBuf, "viewer" );
				printf( "v\nMode: " );
				gets( szBuf + 1 );
				cmd.replyExpected = true;
				break;
			case '8':  // #keypad arrow up
				sprintf( szBuf, "forward" );
				break;
			case '2':  // #keypad arrow down
				sprintf( szBuf, "back" );
				break;
			case '4':  // #keypad arrow left
				sprintf( szBuf, "left" );
				break;
			case '6':  // #keypad arrow right
				sprintf( szBuf, "right" );
				break;
			case '5':  // #keypad center
			case ' ':  // space bar
				sprintf( szBuf, "stop" );
				break;
			case 'w':
				sprintf( szBuf, "wander" );
				break;
			case 'o':
				sprintf( szBuf, robotIsOn ? "off" : "on" );
				robotIsOn = !robotIsOn;
				cmd.replyExpected = robotIsOn;
				break;
			case 'q':
				sprintf( szBuf, "quit" );
				cmd.doQuit = true;
				break;
			case 'd':
				sprintf( szBuf, "d" );
				printf( "d\nFilename: " );
				gets( szBuf + 1 );
				cmd.replyExpected = true;
				break;
			case 'c':
				sprintf( szBuf, "c" );
				printf( "c\nCamera pose: " );
				gets( szBuf + 1 );
				cmd.replyExpected = true;
				break;
			case 224: // extended keys
				int c = getch();
				switch( c ) {
					case 72: // Arrow up
						sprintf( szBuf, "forward" );
						break;
					case 80: // Arrow down
						sprintf( szBuf, "back" );
						break;
					case 75: // Arrow left
						sprintf( szBuf, "left" );
						break;
					case 77: // Arrow right
						sprintf( szBuf, "right" );
						break;
					case 71: // Home key
						sprintf( szBuf, robotIsOn ? "off" : "on" );
						robotIsOn = !robotIsOn;
						break;
					case 79: // End key
						sprintf( szBuf, "stop" );
						break;
					default:
						printf( "%d", c );
						break;
				}
				break;
		}
	}

	// Clear the buffer
	while ( kbhit() ) getch();
	
	if ( !cmd.replyExpected ) printf( "%s\n> ", szBuf );

	cmd.string = std::string( szBuf );

	return cmd;
}