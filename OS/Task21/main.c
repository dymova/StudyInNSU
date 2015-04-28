#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

typedef void (*sighandler_t)(int);
sighandler_t sigset(int sig, sighandler_t disp);

sig_atomic_t sigInt = 0;
sig_atomic_t sigQuit = 0;
int count = 0;

void sigcatch(int sig) {

    if(sig == SIGQUIT) {
        sigInt = 1;
    }
    if(sig == SIGINT) {
        sigInt = 1;
    }
//    if (sig == SIGQUIT) {
//        printf("\n^C was pressed %d times\n", count);
//        exit(0);
//    }
//    count++;
//    printf("\a");
}

void handleSigInt() {
    count++;
    printf("\a");
    sigInt = 0;
    signal(SIGINT, sigcatch);
}

void handleSigQuit() {
    printf("\n^C was pressed %d times\n", count);
    exit(EXIT_SUCCESS);
}

int main()
{
//    sigset(SIGINT, sigcatch);
//    sigset(SIGQUIT, sigcatch);
    signal(SIGINT, sigcatch);
    signal(SIGQUIT, sigcatch);

    printf("piii \a");

    for(;;) {
        if(sigInt) {
            handleSigInt();
        }
        if(sigQuit) {
            handleSigQuit();
        }
    }

    return EXIT_SUCCESS;
}