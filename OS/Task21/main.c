#define _XOPEN_SOURCE  500
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#define DELETE 063

sig_atomic_t sigQuit = 0;

void sigcatch(int sig) {
    if (sig == SIGQUIT) {
        sigQuit = 1;
    }
}

void handleSigQuit(int count) {
    printf("\nSignal sounded %d times\n", count);
    exit(EXIT_SUCCESS);
}

int main() {
    sigset_t mask;
    if(sigfillset(&mask) == -1) {
        perror("sigfillset");
        exit(EXIT_FAILURE);
    }
    if(sigdelset(&mask, SIGQUIT) == -1)
    {
        perror("sigdelset SIGQUIT");
        exit(EXIT_FAILURE);
    }
    if(sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    if(sigset(SIGQUIT, sigcatch) == (__sighandler_t) -1) {
        perror("sigset");
        exit(EXIT_FAILURE);
    }

    struct termios ttyNew;
    struct termios ttyCurrent;
    if(tcgetattr(STDIN_FILENO, &ttyCurrent) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    ttyNew = ttyCurrent;
    ttyNew.c_lflag &= ~(ECHO | ICANON);
    ttyNew.c_cc[VMIN] = 1;
    ttyNew.c_cc[VTIME] = 0;
    if(tcsetattr(STDIN_FILENO, TCSANOW, &ttyNew) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    char c;
    int count = 0;
    for(;;) {
        if(sigQuit) {
            if(tcsetattr(STDIN_FILENO, TCSANOW, &ttyCurrent) == -1) {
                printf("Error. Settings of tty not restore.");
                exit(EXIT_FAILURE);
            }
            handleSigQuit(count);
        }
        if((c = getchar()) == DELETE){
            count++;
            printf("\a%d\n", count);
        }
    }

    return EXIT_SUCCESS;
}
