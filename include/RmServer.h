// RmServer.h

#ifndef RM_SERVER_H
#define RM_SERVER_H

#include <string>
#include <winsock.h>


/**
 * Provides a UDP socket server for wireless data transfer.
 * @see RmClient
 */
class RmServer
{
public:

	/**
	 * Initializes a UDP server connection with which a client on the same port may
	 * communicate using RmClient.
	 * @throw an RmExceptions::SocketException if encounters the wrong winsock version
	 */
	RmServer( const short portNumber, const std::string name = "Remote control server" );


	/**
	 * Closes the connection.
	 */
	~RmServer();


	/**
	 * Returns the last message sent from the client, blocking until such message is received.
	 */
	std::string getClientString();


	/**
	 * Sends the given string to the client initialized at construction.
	 * @throw an RmExceptions::SocketException if unable to reach client
	 */
	void sendClientReply( const std::string s );

protected:

	/**
	 * Creates a UDP/IP datagram socket on the given port and binds the server to the socket.
	 * @throw an RmExceptions::SocketException if unable to create or bind the socket
	 */
	int initServer( const short portNumber );

	/**
	 * Provides the means for dynamically creating an exception message that includes
	 * the error followed by the name of the server given during construction.
	 * Memory to accommodate the return string is allocated on each call.
	 */
	const char* message( const char *msg ) const;

private:

	SOCKET m_socket;
	SOCKADDR_IN m_clientAddress;
	SOCKADDR_IN m_serverAddress;
	std::string m_name;
	char m_szBuf[4096]; // this must be class-scope
};

#endif