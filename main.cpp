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


    init_stack(stk);
    DEBUG_MSG("INIT_STACK(stk, 0);\n");
    dump_stack(stdout, stk_ptr);

    push_stack(stk_ptr, 63);
    DEBUG_MSG("push_stack(stk_ptr, 3);\n");
    dump_stack(stdout, stk_ptr);

    log_stack_to_html(stk_ptr);

    push_stack(stk_ptr, 64);
    DEBUG_MSG("push_stack(stk_ptr, 4);\n");
    dump_stack(stdout, stk_ptr);

    for (size_t i = 8; i < sizeof(stack); i++)
    {
        dump_stack(stdout, stk_ptr);
        // log_stack_to_html(stk_ptr);
        *((char*) stk_ptr + i) = 0;
        close_html();
    }

    DEBUG_MSG("ABOBA\n");


    push_stack(stk_ptr, 65);
    DEBUG_MSG("push_stack(stk_ptr, 5);\n");
    dump_stack(stdout, stk_ptr);

    push_stack(stk_ptr, 66);
    DEBUG_MSG("push_stack(stk_ptr, 6);\n");
    dump_stack(stdout, stk_ptr);
//
//     elem_t value = 0;
//     pop_stack(stk_ptr, &value);
//     dump_stack(stdout, stk_ptr);
//     pop_stack(stk_ptr, &value);
//     dump_stack(stdout, stk_ptr);
//     pop_stack(stk_ptr, &value);
//     dump_stack(stdout, stk_ptr);
//     pop_stack(stk_ptr, &value);
//     dump_stack(stdout, stk_ptr);
//     pop_stack(stk_ptr, &value);
//     dump_stack(stdout, stk_ptr);
//
//     destruct_stack(stk_ptr);
//     dump_stack(stdout, stk_ptr);

    return 0;
}
