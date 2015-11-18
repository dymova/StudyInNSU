#include <iostream>
#include "Forwarder.h"

const char *USAGE = "Usage: Server <listenPort> <remoteHost> <remotePort>";

int main(int argc, char **argv) {

    if (argc != 4) {
        fprintf(stderr, USAGE);
        return EXIT_FAILURE;
    }
    try {
        Forwarder* forwarder = new Forwarder(argv[1], argv[2], argv[3]);
        forwarder->start();

    } catch (IllegalArgumentException e ) {
        std::cout << e.what() << std::endl;
    } catch  (ConstructorForwarderException e) {
        std::cout << e.what() << std::endl;
    } catch (StartForwarderException e) {
        std::cout << e.what() << std::endl;
    }


//    freeaddrinfo(remoteInfo);
    return EXIT_SUCCESS;
}
