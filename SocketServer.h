#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

/****
 * A Windows socket server 
 * Each client has its own thread
 */

namespace SockServer
{
namespace internal_
{

class SocketClient
{
    public:
    SocketClient(SOCKET in = INVALID_SOCKET) { SetSocket(in); }
    void SetSocket(SOCKET in) { socket = in; if (in >= 0 && in != INVALID_SOCKET) isValid = true; lastError = 0; }
    SOCKET socket = INVALID_SOCKET;
    bool isValid = false;
    size_t lastError = 0;

    /****
     * @brief write to the client
     */
    bool Write(const std::vector<unsigned char>& in);
    void Disconnect();
};

} } // namespace SockServer::internal_

class SocketServer
{
    public:
    SocketServer(int port);
    ~SocketServer();

    StartServer(int port);

    /******
     * @brief Wait on the listen socket until a new connection comes in
     * @NOTE This blocks until a new connection is made
     * @return < 0 on error, 0 on timeout, > 0 on new connection
     */
    void AwaitNewConnections();
    /*****
     * @brief write to all connected clients asynchronously
     */
    void SendToAll(const std::vector<unsigned char>& in);
    void SendToAll(const std::string& in);
    size_t CountOpenConnections() { size_t retval = 0; for(auto& c : clients) if(c.isValid) retval++; return retval; }
    bool shuttingDown = false;

    protected:
    SOCKET listenSocket = INVALID_SOCKET;
    std::vector<SockServer::internal_::SocketClient> clients;
    std::shared_ptr<std::thread> connectionsThread = nullptr;
};

