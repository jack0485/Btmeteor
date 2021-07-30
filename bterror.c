#include <stdio.h>
#include <stdlib.h>
#include "bterror.h"

void btexit(int errno, char *file, int line) {
    fprintf(stderr, "exit at %s : %d with error number : %d\n", file, line, errno);
    exit(errno);
}
