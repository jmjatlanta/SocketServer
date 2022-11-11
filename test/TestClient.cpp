#include <iostream>
#include <string>
#include "../SocketClient.h"

void print_syntax_and_exit(const char* exe)
{
    std::cerr << "Syntax: " << std::string(exe) << " [host:]port\n";
    exit(2);
}

bool parse_host_port(const std::string& in, std::string& host, std::string& port)
{
    auto pos = in.find(":");
    if (pos != std::string::npos)
    {
        host = in.substr(0, pos);
        port = in.substr(pos+1);
    }
    else
    {
        host = "127.0.0.1";
        port = in;
    }
    return true;
}

int main(int argc, char** argv)
{
    if (argc < 2)
        print_syntax_and_exit(argv[0]);
    std::string host;
    std::string port;
    if(!parse_host_port(argv[1], host, port))
        print_syntax_and_exit(argv[0]);
    std::cerr << "Have host of " << host << " and port of " << port << "\n";
    SocketClient client(host, port);
    while (client.isConnected)
    {
        std::string msg;
        client.Read(msg);
        std::cerr << "Received " << msg << "\n";
    }
    return 1;
}

