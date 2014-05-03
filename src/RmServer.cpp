// RmServer.cpp

#include <iostream>
#include "RmServer.h"
#include "RmExceptions.h"


RmServer::RmServer( const short portNumber, const std::string name )
: m_name(name)
{
	WORD wVersionRequested = MAKEWORD(1,1);
   	WSADATA wsaData;
    int nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested)
	{	
		throw RmExceptions::SocketException( "RmServer::RmServer()", "Wrong version" );
	}
	
	initServer( portNumber );
}


RmServer::~RmServer()
{
	closesocket( m_socket );
}


int RmServer::initServer( const short portNumber )
{
	int nRet;
	char temp[15] = "";
	int stopflag = 1;
	
	//
	// Create a UDP/IP datagram socket
	//
	m_socket = socket(AF_INET,		// Address family
					   SOCK_DGRAM,  // Socket type
					   IPPROTO_UDP);// Protocol
	if (m_socket == INVALID_SOCKET)
	{
		throw RmExceptions::SocketException( "RmServer::initServer()", "Invalid socket" );
	}
	
	
	//
	// Fill in the address structure
	//
	m_serverAddress.sin_family = AF_INET;
	m_serverAddress.sin_addr.s_addr = INADDR_ANY;		// Let WinSock assign address
	m_serverAddress.sin_port = htons(portNumber);		// Use port passed from user
	
	//
	// Bind the server to the socket
	//
	nRet = bind(m_socket, (LPSOCKADDR)&m_serverAddress, sizeof(SOCKADDR) );
	if (nRet == SOCKET_ERROR)
	{
		closesocket(m_socket);
		throw RmExceptions::SocketException( "RmServer::initServer()", "Unable to bind socket" );
	}
	
	
	//
	// Verify connection
	//
	nRet = gethostname(m_szBuf, sizeof(m_szBuf));
	if (nRet == SOCKET_ERROR)
	{
		closesocket(m_socket);
		throw RmExceptions::SocketException( "RmServer::initServer()", "Unable to get host name" );
	}
	
	std::cout << m_name << " '" << m_szBuf << "' waiting on port " << portNumber << "\n";
	
	return nRet;
}


std::string RmServer::getClientString() 
{
	int nLen = sizeof(SOCKADDR);
	memset( m_szBuf, 0, sizeof(m_szBuf) );
	int nRet = recvfrom( m_socket, m_szBuf, sizeof(m_szBuf), 0, (LPSOCKADDR)&m_clientAddress, &nLen );
	return std::string( m_szBuf );
}


void RmServer::sendClientReply( const std::string s )
{
	int nRet = sendto( m_socket, s.c_str(), s.length(), 0, (LPSOCKADDR)&m_clientAddress, 
		sizeof(SOCKADDR) );
	if ( nRet == SOCKET_ERROR ) {
		const char *m = message( "Socket error" );
		throw RmExceptions::SocketException( "RmServer::sendReply()", m );
	}
}

const char* RmServer::message( const char *msg ) const
{
	static const std::string pre( " [" );
	static const std::string suf( "]" );
	std::string m = msg + pre + m_name + suf;
	char *cmsg = new char[m.length() + 1];
	strcpy( cmsg, m.c_str() );
	return cmsg;
}