#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUF_SIZE 70

int main(int argc, char *argv[]) {
    char buffer[1024];

    char pathToFile[MAX_BUF_SIZE];
    if (argc != 2) {
        printf("usearg: %s <pathToFile>\n", argv[0]);
        fgets(pathToFile, MAX_BUF_SIZE, stdin);
        pathToFile[strlen(pathToFile) - 1] = '\0';
    } else {
        strcpy(pathToFile, argv[1]);
    }

    FILE * fd;
    if ((fd = fopen(pathToFile, "r")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    FILE * fp;
    if ((fp = popen("wc -l", "w")) == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    while(fgets(buffer, sizeof(buffer), fd) != NULL) {
        if(strcmp(buffer, "\n") == 0) {
            fprintf(fp, "\n");
        }
    }

    fclose(fd);
    pclose(fp);
    return 0;
}