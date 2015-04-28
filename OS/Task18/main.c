#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

void printFileInfo(char* path) {
    struct stat status;
    if(access(path, F_OK) == -1) {
        perror("access");
        return;
    }

    if(stat(path, &status) == -1) {
        perror(path);
        return;
    }

    char type = '?';
    if(S_ISDIR(status.st_mode)) {
        type = 'd';
    }
    else if(S_ISREG(status.st_mode)) {
        type = '-';
    }

    char mode[] = "---------";
    if(status.st_mode & S_IRUSR) {
        mode[0] = 'r';
    }
    if(status.st_mode & S_IWUSR) {
        mode[1] = 'w';
    }
    if(status.st_mode & S_IXUSR) {
        mode[2] = 'x';
    }
    if(status.st_mode & S_IRGRP) {
        mode[3] = 'r';
    }
    if(status.st_mode & S_IWGRP) {
        mode[4] = 'w';
    }
    if(status.st_mode & S_IXGRP) {
        mode[5] = 'x';
    }
    if(status.st_mode & S_IROTH) {
        mode[6] = 'r';
    }
    if(status.st_mode & S_IWOTH) {
        mode[7] = 'w';
    }
    if(status.st_mode & S_IXOTH) {
        mode[8] = 'x';
    }

    struct passwd* password = getpwuid(status.st_uid);
    struct group* group = getgrgid(status.st_gid);
    char* pwName = password->pw_name;
    char* grName = group->gr_name;

    char* mTime = ctime(&status.st_mtime);
    mTime[strlen(mTime) - 1] = '\0';

    printf("%c%s %5d %10s %10s",
        type, mode, status.st_nlink,
        pwName, grName);
    if(type == '-') {
        printf("%6d ", status.st_size);
    } else {
        printf("       ");
    }
    printf("%s %s\n", mTime, basename(path));
}


int main(int argc, char **argv)
{
    for(int i = 1; i < argc; i++)
    {
        printFileInfo(argv[i]);
    }
    return EXIT_SUCCESS;
}