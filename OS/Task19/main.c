#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#include <string.h>

#define MAX_BUF_SIZE 70


int mymatch(char * expr, char * path) {
    //printf("match('%s' (%d), '%s');\n",expr, (int)expr[0],path);
    for(;;) {
        char c = *expr++;
        //printf("c=%c [%d]\n", c,(int)c);
        switch(c) {
            case '\0':
                return (*path=='\0') ? 0 : -1;
                break;
            case '?':
                if(*path == '\0') {
                    return -1;
                } else {
                    path++;
                }
                break;
            case '*':
                c = *expr;

                while (c == '*') {
                    c = *++expr;
                }
                if(c == '\0') {
                    return 0;
                }
                while(*path != '\0') {
                    if(!mymatch(expr, path)) {
                        return 0;
                    }
                    path++;
                }
                return -1;
                break;
            default:

                if(*path == c) {
                    path++;
                } else {
                    return -1;
                }
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    char pathToFile[MAX_BUF_SIZE];
    if (2 != argc)
    {
        printf("Enter the file name pattern, "
               "similar to that used in the shell.");
        fgets(pathToFile, MAX_BUF_SIZE, stdin);
        pathToFile[strlen(pathToFile) - 1] = '\0';
    }
    else
    {
        strcpy(pathToFile, argv[1]);
    }

    DIR* dir = NULL;
    int files = 0;
    struct dirent *dp;

    if(argc < 2) {
        printf("No args. Use %s expr\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(strchr(argv[1], '/') != NULL) {
        printf("expr contains restricted / symbol\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if((dir = opendir(".")) == NULL) {
        perror("opendir");
        return;
    }

    while((dp = readdir(dir)) != NULL) {
        //if(!fnmatch(argv[1], dp->d_name, 0)) {
        if(strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..")) {
            //printf("===============\n");
            if(!mymatch(argv[1], dp->d_name)) {

                printf("%s\n", dp->d_name);
                files++;
            }
        }
    }

    if(!files) {
        printf("%s\n", argv[1]);
    }

    closedir(dir);

    return 0;
}