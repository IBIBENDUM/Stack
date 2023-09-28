#include <stdio.h>

// #define VALUE_TYPE char
// #define ELEM_FORMAT "%c"
// #define POISON_VAL 10
#include "stack.h"
// #undef VALUE_TYPE
// #undef ELEM_FORMAT
// #undef POISON_VAL

int main()
{
    stack stk;
    stack* stk_ptr = &stk;

    FILE* log_ptr = open_html(get_log_file_name());
    if (log_ptr)
    close_html(log_ptr);
    init_stack(stk);
    DEBUG_MSG("INIT_STACK(stk, 0);\n");
    DUMP_STACK(stdout, stk_ptr);

    push_stack(stk_ptr, 63);
    DEBUG_MSG("push_stack(stk_ptr, 3);\n");
    DUMP_STACK(stdout, stk_ptr);


    push_stack(stk_ptr, 64);
    DEBUG_MSG("push_stack(stk_ptr, 4);\n");
    DUMP_STACK(stdout, stk_ptr);
    // for (size_t i = 0; i < sizeof(stack); i++)
    // {
    //     *((char*) &stk + i) = 0;
    // }

    push_stack(stk_ptr, 65);
    DEBUG_MSG("push_stack(stk_ptr, 5);\n");
    DUMP_STACK(stdout, stk_ptr);

    push_stack(stk_ptr, 66);
    DEBUG_MSG("push_stack(stk_ptr, 6);\n");
    DUMP_STACK(stdout, stk_ptr);
//
//     elem_t value = 0;
//     pop_stack(stk_ptr, &value);
//     DUMP_STACK(stdout, stk_ptr);
//     pop_stack(stk_ptr, &value);
//     DUMP_STACK(stdout, stk_ptr);
//     pop_stack(stk_ptr, &value);
//     DUMP_STACK(stdout, stk_ptr);
//     pop_stack(stk_ptr, &value);
//     DUMP_STACK(stdout, stk_ptr);
//     pop_stack(stk_ptr, &value);
//     DUMP_STACK(stdout, stk_ptr);
//
//     destruct_stack(stk_ptr);
//     DUMP_STACK(stdout, stk_ptr);

    return 0;
}
