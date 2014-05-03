// RmClient.cpp

#include <iostream>
#include "RmClient.h"
#include "RmExceptions.h"


RmClient::RmClient( const char *serverName, const short portNumber )
{
	WORD wVersionRequested = MAKEWORD(1,1);
   	WSADATA wsaData;
    int nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested)
	{	
		throw RmExceptions::SocketException( "RmServer::RmServer()", "Wrong version" );
	}
	
	initClient( serverName, portNumber );
}


RmClient::~RmClient()
{
	closesocket( m_socket );

	WSACleanup(); // release WinSock
}


void RmClient::initClient( const char *serverName, const short portNumber )
{
	//
	// Find the server
	//
    LPHOSTENT lpHostEntry;
	
	lpHostEntry = gethostbyname( serverName );
    if ( lpHostEntry == NULL )
    {
		closesocket( m_socket );
		throw RmExceptions::SocketException( "RmServer::initClient()", "Unable to get host name" );
    }
	
	
	//
	// Create a UDP/IP datagram socket
	//
	m_socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( m_socket == INVALID_SOCKET )
	{
		closesocket(m_socket);
		throw RmExceptions::SocketException( "RmServer::initClient()", "Unable to create socket" );
	}
	
	//
	// Fill in the address structure for the server
	//
	m_serverAddress.sin_family = AF_INET;
	m_serverAddress.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	m_serverAddress.sin_port = htons( portNumber );
}


std::string RmClient::getServerReply()
{
	int nFromLen = sizeof(SOCKADDR);
	memset( m_szBuf, 0, sizeof(m_szBuf) );
	int nRet = recvfrom( m_socket, m_szBuf, sizeof(m_szBuf), 0, (LPSOCKADDR)&m_serverAddress, 
		&nFromLen );

	if ( nRet == SOCKET_ERROR ) {
		throw RmExceptions::SocketException( "RmClient::getReply()", "Socket error" );
	}

	return std::string( m_szBuf );
}


void RmClient::sendServerString( std::string s )
{
	int nRet = sendto( m_socket, s.c_str(), s.length(), 0, (LPSOCKADDR)&m_serverAddress, 
		sizeof(SOCKADDR) );
	if ( nRet == SOCKET_ERROR ) {
		throw RmExceptions::SocketException( "RmClient::sendServerString()", "Socket error" );
	}
}