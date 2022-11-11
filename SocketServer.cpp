#include "SocketServer.h"
#include "winsock2.h"
#include <iostream>

namespace SockServer
{
namespace internal_
{
bool SocketClient::Write(const std::vector<unsigned char>& in)
{
	if (isValid)
	{
		int flags = 0;
		auto result = send( socket, (const char*)in.data(), in.size(), flags );
		if (result == SOCKET_ERROR)
		{
			lastError = WSAGetLastError();
			closesocket(socket);
			isValid = false;
		}
	}
	return isValid;
}

void SocketClient::Disconnect()
{
	closesocket(socket);
	isValid = false;
}

} } // namespace SockServer::internal_

SocketServer::SocketServer(int port)
{
    StartServer(port);
}

SocketServer::~SocketServer()
{
    if (listenSocket != INVALID_SOCKET)
        closesocket(listenSocket);
	connectionsThread->join();
	std::cerr << "Socket server destructor completed.\n";
}

void SocketServer::SendToAll(const std::vector<unsigned char>& in)
{
	auto cnt = CountOpenConnections();
	if (cnt == 0)
		return;
	std::cerr << "Sending " << std::to_string(in.size()) << " bytes to " 
				<< std::to_string(cnt) << " connections.\n";
	for(auto& client : clients)
	{
		if (client.isValid && !client.Write(in))
		{
			std::cerr << "Write error. Disconnecting client.\n";
			client.Disconnect();
		}
	}
}

void SocketServer::SendToAll(const std::string& in)
{
	return SendToAll( std::vector<unsigned char>{in.begin(), in.end()} );
}

/******
 * Wait on the listen socket until a new connection comes in
 * @NOTE This blocks until shuttingDown == true
 * @return < 0 on error, 0 on timeout, > 0 on new connection
 */
void SocketServer::AwaitNewConnections()
{
	while (!shuttingDown)
	{
		// select() gives us a timeout
		fd_set read_fs;
		fd_set write_fs;
		fd_set except_fs;
		FD_ZERO(&read_fs);
		FD_ZERO(&write_fs);
		FD_ZERO(&except_fs);
		FD_SET(listenSocket, &read_fs);
		FD_SET(listenSocket, &write_fs);
		FD_SET(listenSocket, &except_fs);
		timeval delay;
		delay.tv_sec = 0;
		delay.tv_usec = 30;
		int retVal = select(listenSocket+1, &read_fs, &write_fs, &except_fs, &delay);
		if (retVal < 0) // error
		{
			std::cerr << "AwaitNewConnections: select returned " << std::to_string(WSAGetLastError()) << "\n";
			continue;
		}
		else
		{
			if (retVal > 0) // 0 indicates timeout
			{
				SockServer::internal_::SocketClient client;
				std::cerr << "Awaiting connections.\n";
				client.SetSocket(accept(listenSocket, NULL, NULL));
				if (!client.isValid) {
					std::cerr << "Unable to add a client!\n";
					continue;
				}
				clients.push_back(client);
				std::cerr << "Added a client number " << std::to_string(client.socket) << "\n";
			}
		} 
	}
    return;
}

int SocketServer::StartServer(int port)
{
    static const size_t DEFAULT_BUFLEN = 65535;
    WSADATA wsaData;
    int iResult;

    struct addrinfo *result = NULL;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed with error: " << std::to_string(iResult) << "\n";
        return 0;
    }

    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    std::string s_port = std::to_string(port);
    iResult = getaddrinfo(NULL, s_port.c_str(), &hints, &result);
    if ( iResult != 0 ) {
        std::cout << "getaddrinfo failed with error: " << std::to_string(iResult) << "\n";
        WSACleanup();
        return 0;
    }
    
    // Create a SOCKET for the server to listen for client connections.
    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        std::cout << "socket failed with error: " << std::to_string(WSAGetLastError()) << "\n";
        freeaddrinfo(result);
        WSACleanup();
        return 0;
    }

    // Setup the TCP listening socket
    iResult = bind( listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cout << "bind failed with error: " << std::to_string(WSAGetLastError()) << "\n";
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanup();
        return 0;
    }

    freeaddrinfo(result);

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cout << "listen failed with error: " << std::to_string(WSAGetLastError()) << "\n";
        closesocket(listenSocket);
        WSACleanup();
        return 0;
    }

	// in a new thread, wait for connections
	connectionsThread = std::make_shared<std::thread>(&SocketServer::AwaitNewConnections, this);
	return 1;
}

