#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ASSERT(x) \
    do {\
        int b = (x); \
        if (!b) { \
            fflush(stdout); \
            fputs("expected " #x "\n", stderr); \
            exit(1); \
        } \
    } while (0);

#define PRINT_AND_DIE(...) \
    do { \
        fflush(stdout); \
        fprintf(stderr, __VA_ARGS__); \
        exit(1); \
    } while (0);

void print_c_escaped(char *p, size_t len) {
    while (len-- > 0) {
        char c;
        switch (c = *(p++)) {
        case '\0': printf("\\0"); break;
        case '\a': printf("\\a"); break;
        case '\b': printf("\\b"); break;
        case '\f': printf("\\f"); break;
        case '\n': printf("\\n"); break;
        case '\r': printf("\\r"); break;
        case '\t': printf("\\t"); break;
        case '\v': printf("\\v"); break;
        case '\\': printf("\\\\"); break;
        case '\?': printf("\\\?"); break;
        case '\'': printf("\\\'"); break;
        case '\"': printf("\\\""); break;
        default:   printf("%c", c);
        }
    }
}

int main(int argc, char **argv) {
    char last_2[2] = { 0 };
    enum {
        CHUNK,
        VARIABLE_NAME,
        VARIABLE_TYPE
    } state = CHUNK;
    char token[128];
    int token_len = 0;

    puts("{");

    while (1) {
        // Read in the input 128 bytes at a time..
        char buffer[128];
        ssize_t ret = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (ret == -1) {
            if (errno == EINTR) continue;
            perror("read() error");
            return 1;
        }
        if (ret == 0) break;

        for (int i = 0; i < ret; i++) {
            // Check that the current token hasn't gotten pathologically long..
            if (token_len == sizeof(token)) {
                if (state == CHUNK) {
                    // unless it's a chunk, we can cope with that.
                    printf("    { .type = JENNY_CHUNK, .u.chunk = \"");
                    print_c_escaped(token, token_len);
                    printf("\"},\n");
                    token_len = 0;
                } else {
                    PRINT_AND_DIE("token is too long: %.*s\n", token_len, token);
                }
            }

            // A variable starts with '"{', and must be preceeded with CHUNK.
            if (last_2[0] == '"' && last_2[1] == '$') {
                ASSERT(state == CHUNK);
                printf("    { .type = JENNY_CHUNK, .u.chunk = \"");
                print_c_escaped(token, token_len - 2);
                printf("\"},\n");
                state = VARIABLE_NAME;
                token_len = 0;

            // The variable's name and type are separated with ', '.
            } else if (last_2[0] == ',' && last_2[1] == ' ') {
                ASSERT(state == VARIABLE_NAME);
                for (int j = 0; j < token_len - 2; j++) {
                    char c = token[j];
                    ASSERT(c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
                }
                printf("    { .type = JENNY_VAR, .u.var = { .p = &%.*s, ", token_len - 2, token);
                state = VARIABLE_TYPE;
                token_len = 0;

            // The variable type must be 'number' or 'string'.
            } else if (last_2[0] == '$' && last_2[1] == '"') {
                ASSERT(state == VARIABLE_TYPE);
                token_len -= 2;
                char *type = "(internal error)";
                if (!strncmp("int", token, token_len)) {
                    type = "JENNY_INT_TYPE";
                } else if (!strncmp("string", token, token_len)) {
                    type = "JENNY_STRING_TYPE";
                } else {
                    PRINT_AND_DIE("%.*s is not a valid type\n", token_len, token);
                }
                printf(".type = %s }},\n", type);
                state = CHUNK;
                token_len = 0;
            }

            last_2[0] = last_2[1];
            last_2[1] = buffer[i];
            token[token_len++] = buffer[i];
        }
    }

    // If the last bit was 'CHUNK', then let's output what's left.
    if (state == CHUNK) {
        if (token_len != 0) {
            printf("    { .type = JENNY_CHUNK, .u.chunk = \"");
            print_c_escaped(token, token_len);
            printf("\"},\n");
        }
    } else {
        // .. and if it wasn't, that's an error.
        PRINT_AND_DIE("unterminated variable");
    }

    // Output the sentinel and end the struct.
    puts("    { .type = JENNY_END }");
    puts("};");

    return 0;
}
