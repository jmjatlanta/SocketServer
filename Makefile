CXX=x86_64-w64-mingw32-g++
LFLAGS=-lws2_32

%.o:%.cpp
	$(CXX) -c $^ -o $@

all: test/TestServer.exe test/TestClient.exe

test/TestServer.exe: test/TestServer.o SocketServer.o
	$(CXX) -static -o $@ $^ $(LFLAGS)

test/TestClient.exe: test/TestClient.o SocketClient.o
	$(CXX) -static -o $@ $^ $(LFLAGS)

clean:
	$(RM) *.o
	$(RM) test/*.o
	$(RM) test/TestClient.exe
	$(RM) test/TestServer.exe
