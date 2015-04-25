#include <sys/types.h>
#include <stdio.h>
#include <time.h>

extern char *tzname[];

int main(int argc, char *argv[], char *envp[])
{

    time_t now;
    struct tm *sp;

    time (&now);

    putenv("TZ=America/Los_Angeles");

    sp = localtime(&now);

    printf("%d/%d/%d %d:%d\n",
           sp->tm_mday,
           sp->tm_mon + 1,
           sp->tm_year + 1900,
           sp->tm_hour,
           sp->tm_min);

    return 0;
}