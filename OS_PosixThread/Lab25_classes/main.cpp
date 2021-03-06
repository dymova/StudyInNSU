#include <iostream>
#include <cstdlib>
#include <cstdio>

#include "Forwarder.h"

const char *USAGE = "Usage: Server <listenPort> <remoteHost> <remotePort>\n";

int main(int argc, char **argv) {

    if (argc != 4) {
        fprintf(stderr,"%s" ,USAGE);
        return EXIT_FAILURE;
    }
    try {
        Forwarder* forwarder = new Forwarder(argv[1], argv[2], argv[3]);
        forwarder->start();

    } catch (IllegalArgumentException& e ) {
        std::cout << e.what() << std::endl;
    } catch  (ConstructorForwarderException& e) {
        std::cout << e.what() << std::endl;
    } catch (StartForwarderException& e) {
        std::cout << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
