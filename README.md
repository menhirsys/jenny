`jenny` is a JSON generator.

`jenny` is built first and foremost for embedded applications where the heap is
limited and you generally want to avoid allocations. malloc() is not used here!

Here's how it works:

First, you define your JSON structure and what variables you want in it. For
example, here is `test.jenny`:

```
{
    "int": "$this_is_an_int, int$",
    "string": "$this_is_a_string, string$"
}
```

(By the way, you can use a JSON validator to check that this is valid JSON.
`jenny` does not check!)

Then, you use `./jenny` to generate a `.c` file:

```
./jenny < test.jenny
{
    { .type = JENNY_CHUNK, .u.chunk = "{\n    \"int\": "},
    { .type = JENNY_VAR, .u.var = { .p = &this_is_an_int, .type = JENNY_INT_TYPE }},
    { .type = JENNY_CHUNK, .u.chunk = ",\n    \"string\": "},
    { .type = JENNY_VAR, .u.var = { .p = &this_is_a_string, .type = JENNY_STRING_TYPE }},
    { .type = JENNY_CHUNK, .u.chunk = "\n}\n"},
    { .type = JENNY_END }
}
```

Then, you `#include` the C file and use the `jenny` API to output the JSON to
any write() -ish function that you want. In this case, I'm wrapping `write`
itself to print to STDOUT:

```
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
```
