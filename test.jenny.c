{
    { .type = JENNY_CHUNK, .u.chunk = "{\n    \"int\": "},
    { .type = JENNY_VAR, .u.var = { .p = &this_is_an_int, .type = JENNY_INT_TYPE }},
    { .type = JENNY_CHUNK, .u.chunk = ",\n    \"string\": "},
    { .type = JENNY_VAR, .u.var = { .p = &this_is_a_string, .type = JENNY_STRING_TYPE }},
    { .type = JENNY_CHUNK, .u.chunk = "\n}\n"},
    { .type = JENNY_END }
};
