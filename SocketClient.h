#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

class SocketClient
{
    public:
    SocketClient(const std::string& host, const std::string& port);
    ~SocketClient();
    bool Read(std::vector<unsigned char>& out);
    bool Read(std::string& out);
    bool Write(const std::string& in);
    bool Write(const std::vector<unsigned char>& in);
    int Connect(const std::string& host, const std::string& port);

    bool isConnected = false;
    size_t lastError = 0;
    SOCKET socket;
};
