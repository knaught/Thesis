// RmClient.h

#include <string>
#include <winsock.h>


/**
 * Provides a UDP socket client for wireless data transfer.
 * @see RmServer
 */
class RmClient
{
public:

	/**
	 * Initializes a UDP client connection with the named server on the given port.
	 * @throw an RmExceptions::SocketException if winsock is unable to make the connection
	 * @see RmServer
	 */
	RmClient( const char *serverName, const short portNumber );


	/**
	 * Closes the connection.
	 */
	~RmClient();


	/**
	 * Returns the last message sent from the server, blocking until such message is received.
	 * @throw an RmExceptions::SocketException if unable to reach server
	 */
	std::string getServerReply();


	/**
	 * Sends the given message to the server.
	 * @throw an RmExceptions::SocketException if unable to reach server
	 */
	void sendServerString( std::string s );

protected:

	/**
	 * Opens a connection to the named server and creates a UDP/IP datagram socket on the
	 * given port.
	 * @throw an RmExceptions::SocketException if unable to reach server or create a socket
	 */
	void initClient( const char *serverName, const short portNumber );

private:

	SOCKET m_socket;
	SOCKADDR_IN m_clientAddress;
	SOCKADDR_IN m_serverAddress;
	char m_szBuf[4096]; // this must be class-scope
};
