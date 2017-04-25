#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "jenny.h"

void write_all(posix_write_fn write, int fd, const void *p, ssize_t len) {
    while (len > 0) {
        ssize_t ret = write(fd, p, len);
        if (ret == -1) {
            if (errno == EINTR)
                continue;
            // What can I do here?
            return;
        }
        len -= ret;
    }
}

int jenny(jenny_t *j, posix_write_fn write, int fd) {
    for (; j->type != JENNY_END; j++) {
        char buffer[16];
        switch (j->type) {

        case JENNY_CHUNK:
            write_all(write, fd, j->u.chunk, strlen(j->u.chunk));
            break;

        case JENNY_VAR:
            switch (j->u.var.type) {

            case JENNY_INT_TYPE:
                if (snprintf(buffer, sizeof(buffer), "%i", *(int *)j->u.var.p) >= sizeof(buffer)) {
                    fprintf(stderr, "jenny() needed more than %zu bytes for int", sizeof(buffer));
                }
                write_all(write, fd, buffer, strlen(buffer));
                break;

            case JENNY_STRING_TYPE:
                write_all(write, fd, "\"", 1);
                // "[..] characters that must be escaped: quotation mark,
                // reverse solidus, and the control characters (U+0000 through
                // U+001F).
                for (int i = 0; i < strlen(*((char **)j->u.var.p)); i++) {
                    unsigned char c = (*((char **)j->u.var.p))[i];
                    if (c == '"') {
                        write_all(write, fd, "\\\"", 2);
                    } else if (c == '\\') {
                        write_all(write, fd, "\\\\", 2);
                    } else if (c == '\n') {
                        write_all(write, fd, "\\n", 2);
                    } else if (c == '\t') {
                        write_all(write, fd, "\\t", 2);
                    } else if (c <= 0x1f) {
                        write_all(write, fd, "\\u00", 4);
                        snprintf(buffer, sizeof(buffer), "%02x", c);
                        write_all(write, fd, buffer, 2);
                    } else {
                        write_all(write, fd, &c, 1);
                    }
                }
                write_all(write, fd, "\"", 1);
                break;

            }
            break;

        case JENNY_END:
            break;
        }
    }
    return 0;
}
