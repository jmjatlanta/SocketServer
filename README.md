A simplistic socket server for Windows

This project was written for Windows on the Linux operating system thanks to mingw64.

The motivation is a project that needs to share information with a small number of clients on an internal network.

The server binds to a port. Once clients are connected, information is shared. Clients do not send messages.
Disconnects are simply handled. Messages are asynchronous.

A small client is also provided, mainly to demonstrate how to write one.

TODO: A disconnection results in retrying to connect.
