#include <stdlib.h>
#define __USE_POSIX2
#include <stdio.h>



int main() {
    FILE *fp;

    if ((fp = popen("tr [:lower:] [:upper:]", "w")) == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "Hello, world!\n");

    pclose(fp);

    return EXIT_SUCCESS;
}