#ifndef __JENNY_H__
#define __JENNY_H__

#include <sys/types.h>

typedef struct {
    enum {
        JENNY_CHUNK,
        JENNY_VAR,
        JENNY_END
    } type;
    union {
        char *chunk;
        struct {
            void *p;
            enum {
                JENNY_INT_TYPE,
                JENNY_STRING_TYPE
            } type;
        } var;
    } u;
} jenny_t;

typedef ssize_t (*write_fn)(void *, const void *, size_t);

int jenny(jenny_t *, write_fn, void *);

#endif
