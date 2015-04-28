#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int main(int argc, char** argv) {

    int fd;
    char symbol;
    struct termios ttyNew;
    struct termios ttyCurrent;

    if((fd = open("/dev/tty", O_RDONLY)) == -1) {
        perror("open /dev/tty");
        exit(EXIT_FAILURE);
    }

    if(tcgetattr(fd, &ttyCurrent) == -1) {
        close(fd);
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    ttyNew = ttyCurrent;
    ttyNew.c_lflag &= ~(ECHO | ICANON);
    ttyNew.c_cc[VMIN] = 1;
    ttyNew.c_cc[VTIME] = 0;

    if(tcsetattr(fd, TCSANOW, &ttyNew) == -1) {
        close(fd);
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    printf("Do you like programming? [y/n]\n");
    if(read(fd, &symbol, 1) != 1) {
        close(fd);
        perror("read");
        if(tcsetattr(fd, TCSANOW, &ttyCurrent) == -1) {
            printf("Error. Settings of tty not restore.");
        }
        exit(EXIT_FAILURE);
    }
    printf("Answer: %c\n", symbol);

    if(tcsetattr(fd, TCSANOW, &ttyCurrent) == -1) {
        printf("Error. Settings of tty not restore.");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    return EXIT_SUCCESS;
}