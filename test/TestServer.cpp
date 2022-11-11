#include "../SocketServer.h"

#include <chrono>
#include <thread>
#include <iostream>

bool ctrl_c_pressed = false;

BOOL WINAPI consoleHandler(DWORD signal)
{
	if (signal == CTRL_C_EVENT)
		ctrl_c_pressed = true;
	return TRUE;
}

int main(int argc, char** argv)
{
	if (!SetConsoleCtrlHandler(consoleHandler, TRUE) )
	{
		std::cout << "Error: could not set control handler\n";
		exit(2);
	}

	SocketServer ss(1234);
    std::cerr << "Server started on port 1234\n";

	// process messages until CTRL-C is pressed
    size_t counter = 0;
	while (!ctrl_c_pressed)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ss.SendToAll("Hello " + std::to_string(++counter));
	}
    std::cout << std::string(argv[0]) << " completed. Open connections: " 
                << std::to_string(ss.CountOpenConnections()) << "\n";
    ss.shuttingDown = true;
}

