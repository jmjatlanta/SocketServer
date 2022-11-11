#include "SocketClient.h"
#include "winsock2.h"
#include <iostream>

SocketClient::SocketClient(const std::string& host, const std::string& port)
{
    WSADATA wsaData;
    auto iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed with error: " << std::to_string(iResult) << "\n";
    }
    Connect(host, port);
}

SocketClient::~SocketClient()
{
    if (socket != INVALID_SOCKET)
        closesocket(socket);
    WSACleanup();
}

bool SocketClient::Read(std::vector<unsigned char>& out)
{
    char recvbuf[65535];
    auto iResult = recv(socket, recvbuf, 65535, 0);
    if ( iResult > 0 )
    {
        out = std::vector<unsigned char>{ recvbuf, recvbuf + iResult };
        return true;
    }
    else if ( iResult < 0 )
        lastError = WSAGetLastError();
    return false;
}

bool SocketClient::Read(std::string& out)
{
    std::vector<unsigned char> vec;
	auto result = Read( vec );
    if (result)
    {
        out = std::string{vec.begin(), vec.end()};
    }
    return result;
}

bool SocketClient::Write(const std::vector<unsigned char>& in)
{
    auto iResult = send( socket, (const char*)in.data(), (int)in.size(), 0 );
    if (iResult == SOCKET_ERROR) {
        std::cerr << "send failed with error: " << std::to_string(WSAGetLastError()) << "\n";
        isConnected = false;
        return false;
    }
    return true;
}

bool SocketClient::Write(const std::string& in)
{
    return Write( std::vector<unsigned char>{in.begin(), in.end()} );
}

int SocketClient::Connect(const std::string& host, const std::string& port)
{
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    addrinfo *result = nullptr;
    auto iResult = getaddrinfo(NULL, port.c_str(), &hints, &result);
    if ( iResult != 0 ) {
        std::cout << "getaddrinfo failed with error: " << std::to_string(iResult) << "\n";
        WSACleanup();
        return 0;
    }
    
    // Attempt to connect to an address until one succeeds
    for(addrinfo* ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        socket = ::socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (socket == INVALID_SOCKET) {
            std::cerr << "socket failed with error: " << std::to_string(WSAGetLastError()) << "\n";
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(socket);
            socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (socket == INVALID_SOCKET) {
        std::cerr << "Unable to connect to server!\n";
        WSACleanup();
        return 1;
    }

    isConnected = true;

    /*
    // Send an initial buffer
    iResult = send( socket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);
    */

    // shutdown the connection since no more data will be sent
    iResult = shutdown(socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "shutdown failed with error: " << std::to_string(WSAGetLastError()) << "\n";
        closesocket(socket);
        WSACleanup();
        return 1;
    }

    return 0;
}

