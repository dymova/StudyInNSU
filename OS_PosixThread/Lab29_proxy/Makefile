main: main.o Proxy.o Connection.o CacheBucket.o
	g++ -std=c++98  main.o Proxy.o Connection.o CacheBucket.o -o main
Proxy.o: Proxy.cpp
	g++ -std=c++98 -c Proxy.cpp
Connection.o: Connection.cpp
	g++ -std=c++98 -c Connection.cpp
CacheBucket.o: CacheBucket.cpp
	g++ -std=c++98 -c CacheBucket.cpp
main.o: main.cpp
	g++ -std=c++98 -c main.cpp
clean:
	rm -rf *.o main
