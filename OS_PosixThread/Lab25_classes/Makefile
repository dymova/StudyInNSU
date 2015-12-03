main: main.o Forwarder.o Connection.o 
	g++ -std=c++98 -lsocket -lnsl main.o Forwarder.o Connection.o -o main
Forwarder.o: Forwarder.cpp
	g++ -std=c++98 -c Forwarder.cpp
Connection.o: Connection.cpp
	g++ -std=c++98 -c Connection.cpp
main.o: main.cpp
	g++ -std=c++98 -c main.cpp
