#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>

#include "Proxy.h"

const char *USAGE = "Usage: Proxy <listenPort>";

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "%s", USAGE);
        return EXIT_FAILURE;
    }

    try {
        Proxy* proxy = new Proxy(argv[1]);
        proxy->start();

    } catch (IllegalArgumentException& e ) {
        std::cout << e.what() << std::endl;
    } catch  (ConstructorProxyException e) {
        std::cout << e.what() << std::endl;
    } catch (StartProxyException& e) {
        std::cout << e.what() << std::endl;
    }



    return EXIT_SUCCESS;
}
