#include <stdio.h>

#define VALUE_TYPE int
#define ELEM_FORMAT "%d"
#include "stack.h"
#undef VALUE_TYPE
#undef ELEM_FORMAT

int main()
{
    stack stk;
    stack* stk_ptr = &stk;

    init_stack(stk);
    // init_stack_with_capacity(stk, 1);
    DEBUG_MSG("INIT_STACK(stk, 0);\n");
    DUMP_STACK(stdout, stk_ptr);

    push_stack(stk_ptr, 3);
    DEBUG_MSG("push_stack(stk_ptr, 3);\n");
    DUMP_STACK(stdout, stk_ptr);
//
//
//     push_stack(stk_ptr, 4);
//     DEBUG_MSG("push_stack(stk_ptr, 4);\n");
//     DUMP_STACK(stdout, stk_ptr);
    write_stack_log(stk_ptr);
    // for (size_t i = 0; i < sizeof(stack); i++)
    // {
    //     *((char*) &stk + i) = 0;
    // }
//
//     push_stack(stk_ptr, 5);
//     DEBUG_MSG("push_stack(stk_ptr, 5);\n");
//     DUMP_STACK(stdout, stk_ptr);
//
//     push_stack(stk_ptr, 6);
//     DEBUG_MSG("push_stack(stk_ptr, 6);\n");
//     DUMP_STACK(stdout, stk_ptr);
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
