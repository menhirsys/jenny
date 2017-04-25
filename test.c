#include <unistd.h>

#include "jenny.h"

int this_is_an_int = 20;
char *this_is_a_string = "\"abc\n\t\\123";
jenny_t test[] =
#include "test.jenny.c"

ssize_t write_wrapper(void *p_fd, const void *p, size_t len) {
    return write(*(int *)p_fd, p, len);
}

int main(void) {
    int fd = STDOUT_FILENO;
    jenny(test, write_wrapper, &fd);

    return 0;
}
